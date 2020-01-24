/*
 * Copyright (c) 2014, Nicola Pezzotti (Delft University of Technology)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *  must display the following acknowledgement:
 *  This product includes software developed by the Delft University of Technology.
 * 4. Neither the name of the Delft University of Technology nor the names of
 *  its contributors may be used to endorse or promote products derived from
 *  this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY NICOLA PEZZOTTI ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL NICOLA PEZZOTTI BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

#pragma once
#define GLSL(name, version, shader) \
  static const char * name = \
  "#version " #version "\n" #shader

// Expand density texture to stencil texture
GLSL(stencil_src, 430,
  layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
  layout(std430, binding = 0) buffer AtlasDataBlock { 
    struct {
      uvec2 offset; 
      uvec2 size;
    } atlasLevelData[];
  };
  layout(r8ui, binding = 0) writeonly uniform uimage2D stencil_texture;
  
  uniform sampler2D density_atlas;

  // Shared memory is made slightly larger than group size
  const ivec2 groupSize = ivec2(gl_WorkGroupSize);
  const ivec2 xGroupSize = ivec2(groupSize.x, 0);
  const ivec2 yGroupSize = ivec2(0, groupSize.y);
  const ivec2 sharedBorder = ivec2(1);
  const ivec2 sharedDims = groupSize + 2 * sharedBorder;
  shared float sharedReads[sharedDims.x * sharedDims.y];

  const uint numOffsets = 9;
  const ivec2[] offsets = ivec2[](
    ivec2(-1, -1), ivec2(0, -1), ivec2(1, -1),
    ivec2(-1, 0), ivec2(0, 0), ivec2(1, 0),
    ivec2(-1, 1), ivec2(0, 1), ivec2(1, 1)
  );

  int sharedMemoryIdx(ivec2 lxy) {
    return sharedDims.x * lxy.y + lxy.x;
  }

  void main() {
    const ivec2 xy = ivec2(gl_WorkGroupID * gl_WorkGroupSize + gl_LocalInvocationID);
    const ivec2 lxy = ivec2(gl_LocalInvocationID);
 
    // Fetch texel value at current invocation into shared memory
    const ivec2 _xy = xy - sharedBorder;
    sharedReads[sharedMemoryIdx(lxy)] = texelFetch(density_atlas, _xy, 0).x;
    // Account for borders as well
    if (lxy.x < 2) {
      sharedReads[sharedMemoryIdx(lxy + xGroupSize)] 
        = texelFetch(density_atlas, _xy + xGroupSize, 0).x;
    }
    if (lxy.y < 2) {
      sharedReads[sharedMemoryIdx(lxy + yGroupSize)] 
        = texelFetch(density_atlas, _xy + yGroupSize, 0).x;
    }
    if (lessThan(lxy, ivec2(2)) == bvec2(true)) {
      sharedReads[sharedMemoryIdx(lxy + groupSize)] 
        = texelFetch(density_atlas, _xy + groupSize, 0).x;
    }

    // Ensure shared memory is synchronized across group
    barrier();

    // Accumulate values inside kernel, reading from shared memory only
    float stencilValue = 0.f;
    const ivec2 _lxy = sharedBorder + lxy;
    for (int i = 0; i < numOffsets; i++) {
      stencilValue += sharedReads[sharedMemoryIdx(_lxy + offsets[i])];
    }
    
    // Don't allow stores beyond texture size
    // const ivec2 textureSize = ivec2(atlasLevelData[0].size);
    // if (lessThan(xy, textureSize) == bvec2(true)) {
      imageStore(stencil_texture, xy, uvec4(uint(stencilValue > 0.f)));
    // }
  }
);

// Compute shader for field computation
GLSL(field_src, 430,
  layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;
  layout(std430, binding = 0) buffer PositionsBlock { 
    vec2 Positions[]; 
  };
  layout(std430, binding = 1) buffer BoundsBlock { 
    vec2 minBounds;
    vec2 maxBounds;
    vec2 range;
    vec2 invRange;
  };
  layout(std430, binding = 2) buffer AtlasDataBlock { 
    struct {
      uvec2 offset; 
      uvec2 size;
    } atlasLevelData[];
  };
  layout(rgba32f, binding = 0) writeonly uniform image2D fields_texture;
  
  uniform uint num_points;
  uniform uvec2 texture_size;
  uniform usampler2D stencil_texture;
  uniform sampler2D density_atlas;

  // Reduction components
  const uint groupSize = gl_WorkGroupSize.x;
  const uint halfGroupSize = groupSize / 2;
  shared vec3 reductionArray[halfGroupSize];

  void main() {
    // Location of current workgroup
    ivec2 xyFixed = ivec2(gl_WorkGroupID.xy);
    uint lid = gl_LocalInvocationIndex.x;

    // Skip pixel if stencil is empty
    if (texelFetch(stencil_texture, xyFixed, 0).x == 0u) {
      if (lid < 1) {
        imageStore(fields_texture, xyFixed, vec4(0));
      }
      return;
    } 

    // Map to domain pos
    vec2 domain_pos = (vec2(xyFixed) + vec2(0.5)) / vec2(texture_size);
    domain_pos = domain_pos * range + minBounds;

    // Iterate over points to obtain density/gradient
    vec3 v = vec3(0);
    for (uint i = lid; i < num_points; i += groupSize) {
      vec2 t = domain_pos - Positions[i];
      float t_stud = 1.f / (1.f + dot(t, t));
      vec2 t_stud_2 = t * (t_stud * t_stud);

      // Field layout is: S, V.x, V.y
      v += vec3(t_stud, t_stud_2);
    }
    
    // Perform reduce add over all computed points for this pixel
    if (lid >= halfGroupSize) {
      reductionArray[lid - halfGroupSize] = v;
    }
    barrier();
    if (lid < halfGroupSize) {
      reductionArray[lid] += v;
    }
    for (uint i = halfGroupSize / 2; i > 1; i /= 2) {
      barrier();
      if (lid < i) {
        reductionArray[lid] += reductionArray[lid + i];
      }
    }
    barrier();
    if (lid < 1) {
      vec3 reducedArray = reductionArray[0] + reductionArray[1];
      imageStore(fields_texture, xyFixed, vec4(reducedArray, 0));
    }
  }
);

// Compute shader for point sampling from field
GLSL(interp_src, 430,
  layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
  layout(std430, binding = 0) buffer Pos{ vec2 Positions[]; };
  layout(std430, binding = 1) buffer Val { vec3 Values[]; };
  layout(std430, binding = 2) buffer BoundsInterface { 
    vec2 minBounds;
    vec2 maxBounds;
    vec2 range;
    vec2 invRange;
  };

  uniform uint num_points;
  uniform uvec2 texture_size;
  uniform sampler2D fields_texture;

  void main() {
    uint i = gl_WorkGroupID.x * gl_WorkGroupSize.x + gl_LocalInvocationIndex.x;
    if (i >= num_points) {
      return;
    }
    
    // Map position of point to [0, 1]
    vec2 position = (Positions[i] - minBounds) * invRange;

    // Sample texture at mapped position
    Values[i] = texture(fields_texture, position).xyz;
  }
);