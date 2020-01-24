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

// Vertex shader for voxel grid computation
GLSL(grid_vert_src, 430,
  layout(location = 0) in vec4 point;
  layout(std430, binding = 0) buffer BoundsInterface { 
    vec3 minBounds;
    vec3 maxBounds;
    vec3 range;
    vec3 invRange;
  };

  void main() {
    // Transform point into [0, 1] space
    vec3 position = (point.xyz - minBounds) * invRange;
    
    // Transform point into [-1, 1] clip space
    gl_Position =  vec4(position * 2.f - 1.f, 1.f);
  }
);

// Fragment shader for voxel grid computation
GLSL(grid_fragment_src, 430,
  layout (location = 0) out uvec4 color;

  uniform usampler1D cellMap;
  uniform float zPadding;

  void main() {
    // Z value to look up cell values at
    float zFixed = gl_FragCoord.z * textureSize(cellMap, 0).x - 0.5f;
    
    // Store OR of the two nearest cells as result
    uvec4 lowerCell = texelFetch(cellMap, int(floor(zFixed)), 0);
    uvec4 greaterCell = texelFetch(cellMap, int(ceil(zFixed)), 0);
    color = lowerCell | greaterCell;
  }
);

// Compute shader for field computation
GLSL(field_src, 430,
  layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;
  layout(std430, binding = 0) buffer PosInterface { vec3 Positions[]; };
  layout(std430, binding = 1) buffer BoundsInterface { 
    vec3 minBounds;
    vec3 maxBounds;
    vec3 range;
    vec3 invRange;
  };
  layout(rgba32f, binding = 0) writeonly uniform image3D fields_texture;
  
  uniform uint num_points;
  uniform uint grid_depth;
  uniform uvec3 texture_size;
  uniform usampler2D grid_texture;

  // Reduction components
  const uint groupSize = gl_WorkGroupSize.x;
  const uint halfGroupSize = groupSize / 2;
  shared vec4 reductionArray[halfGroupSize];

  void main() {
    // Location of current workgroup
    ivec2 xyFixed = ivec2(gl_WorkGroupID.xy);
    vec2 xyUnfixed = (vec2(xyFixed) + vec2(0.5)) / vec2(texture_size.xy);
    uint lid = gl_LocalInvocationIndex.x;

    // Query grid map for entire z axis
    // Skip all pixels if empty
    uvec4 gridVec;
    if ((gridVec = texture(grid_texture, xyUnfixed)) == uvec4(0)) {
      uint z = gl_WorkGroupID.z * gl_WorkGroupSize.z + lid;
      if (z < texture_size.z) {
        imageStore(fields_texture, ivec3(xyFixed, z), vec4(0));
      }
      return;
    }
    
    // Workgroups stride over z-dimension
    for (uint z = gl_WorkGroupID.z; 
          z < texture_size.z; 
          z += gl_NumWorkGroups.z) {
      // Map xyz to [0, 1]
      ivec3 xyzFixed = ivec3(xyFixed, z);
      vec3 domain_pos = (vec3(xyzFixed) + vec3(0.5)) / vec3(texture_size);

      // Query grid map sample or skip pixel
      int _z = int(domain_pos.z * grid_depth);
      if (bitfieldExtract(gridVec[_z / 32], 31 - (_z % 32) , 1) == 0) {
        if (lid < 1) {
          imageStore(fields_texture, xyzFixed, vec4(0));
        }
        continue;
      }

      // Map to domain bounds
      domain_pos = domain_pos * range + minBounds;

      // Iterate over points to obtain density/gradient
      vec4 v = vec4(0);
      for (uint i = lid; i < num_points; i += groupSize) {
        vec3 t = domain_pos - Positions[i];
        float t_stud = 1.f / (1.f + dot(t, t));
        vec3 t_stud_2 = t * (t_stud * t_stud);

        // Field layout is: S, V.x, V.y, V.z
        v += vec4(t_stud, t_stud_2);
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
        vec4 reducedArray = reductionArray[0] + reductionArray[1];
        imageStore(fields_texture, xyzFixed, reducedArray);
      }
      barrier();
    }
  }
);

// Compute shader for point sampling from field
GLSL(interp_src, 430,
  layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
  layout(std430, binding = 0) buffer Pos{ vec3 Positions[]; };
  layout(std430, binding = 1) buffer Val { vec4 Values[]; };
  layout(std430, binding = 2) buffer BoundsInterface { 
    vec3 minBounds;
    vec3 maxBounds;
    vec3 range;
    vec3 invRange;
  };

  uniform uint num_points;
  uniform uvec3 texture_size;
  uniform sampler3D fields_texture;

  void main() {
    // Grid stride loop, straight from CUDA, scales better for very large N
    for (uint i = gl_WorkGroupID.x * gl_WorkGroupSize.x + gl_LocalInvocationIndex.x;
        i < num_points;
        i += gl_WorkGroupSize.x * gl_NumWorkGroups.x) {
      // Map position of point to [0, 1]
      vec3 position = (Positions[i] - minBounds) * invRange;

      // Sample texture at mapped position
      Values[i] = texture(fields_texture, position);
    }
  }
);