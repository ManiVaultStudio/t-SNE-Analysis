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

GLSL(density_lod_bottom_vertex, 430,
  layout(location = 0) in vec2 point;
  layout(std430, binding = 0) buffer BoundsInterface { 
    vec2 minBounds;
    vec2 maxBounds;
    vec2 range;
    vec2 invRange;
  };

  out vec2 position;

  void main() {
    // Transform point into [0, 1] space
    position = (point.xy - minBounds) * invRange;

    // Transform point into [-1, 1] clip space
    gl_Position =  vec4(position * 2.f - 1.f, 0.f, 1.f);
  }
);

GLSL(density_lod_bottom_fragment, 430,
  layout (location = 0) out vec4 color;
  
  in vec2 position;

  void main() {
    // Blend together density and center of mass
    // by adding 1 for every point, and summing positions
    color = vec4(1.f, position, 0.f);
  }
);

GLSL(density_lod_bottom_divide_comp, 430,
  layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
  layout(std430, binding = 0) buffer AtlasDataBlock { 
    struct {
      uvec2 offset; 
      uvec2 size;
    } atlasLevelData[];
  };
  layout (rgba32f, binding = 0) writeonly uniform image2D level32f;

  uniform sampler2D texture32f;

  void main() {
    // Location of current workgroup unit
    ivec2 xy = ivec2(gl_WorkGroupID * gl_WorkGroupSize + gl_LocalInvocationID);
    if (min(xy, ivec2(atlasLevelData[0].size) - 1) != xy) {
      return;
    }

    // Divide the center of mass by n
    vec4 v = texelFetch(texture32f, xy, 0);
    v.yz /= max(1.f, v.x); // Prevent divide-by-0 for empty spaces
    imageStore(level32f, xy, v);
  }
);

// Density computation for upper LOD levels, 2 steps
GLSL(density_lod_upper_src, 430,
  layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
  layout(std430, binding = 0) buffer AtlasDataBlock { 
    struct {
      uvec2 offset; 
      uvec2 size;
    } atlasLevelData[];
  };
  layout(rgba32f, binding = 0) writeonly uniform image2D level32f;
  
  uniform sampler2D texture32f;
  uniform int l;
  uniform int maxl;
  const ivec2 groupSize = ivec2(gl_WorkGroupSize.xy);
  shared vec4 reductionArray[64];

  void addTo(inout vec4 v, in vec4 add) {
    vec2 weights = vec2(v.x, add.x);
    float n = weights.x + weights.y;
    weights /= max(vec2(1), n); // Prevent divide-by-0

    vec2 xCenters = weights * vec2(v.y, add.y);
    vec2 yCenters = weights * vec2(v.z, add.z);

    float xv = xCenters.x + xCenters.y;
    float yv = yCenters.x + yCenters.y;
    
    v = vec4(n, xv, yv, 0);
  }

  int toLid(ivec2 lxy) {
    return int(gl_WorkGroupSize.x) * lxy.y + lxy.x;
  }

  bool isInside(ivec2 xy, ivec2 minBounds, ivec2 maxBounds) {
    return clamp(xy, minBounds, maxBounds) == xy;
  }

  bool isBelow(ivec2 xy, ivec2 maxBounds) {
    return min(xy, maxBounds) == xy;
  }

  vec4 sampleToplayer(ivec2 xy) {
    ivec2 prevMin = ivec2(atlasLevelData[l - 1].offset);
    ivec2 prevMax = prevMin + ivec2(atlasLevelData[l - 1].size) - 1;
    ivec2 prevxy = prevMin + 2 * xy;

    vec4 v = vec4(0);

    if (isInside(prevxy, prevMin, prevMax)) {
      v += texelFetch(texture32f, prevxy, 0);
    }
    if (isInside(prevxy + ivec2(1, 0), prevMin, prevMax)) {
      addTo(v, texelFetch(texture32f, prevxy + ivec2(1, 0), 0));
    }
    if (isInside(prevxy + ivec2(0, 1), prevMin, prevMax)) {
      addTo(v, texelFetch(texture32f, prevxy + ivec2(0, 1), 0));
    }
    if (isInside(prevxy + ivec2(1, 1), prevMin, prevMax)) {
      addTo(v, texelFetch(texture32f, prevxy + ivec2(1, 1), 0));
    }

    return v;
  }

  void main() {
    // Location of work unit
    const ivec2 xy = ivec2(gl_WorkGroupID * gl_WorkGroupSize + gl_LocalInvocationID);
    const ivec2 lxy = ivec2(gl_LocalInvocationID.xy);
    const uint lid = gl_LocalInvocationIndex;

    // Perform sample in 2x2 pixel area in l - 1, and store result in l
    vec4 v = sampleToplayer(xy);
    if (isBelow(xy, ivec2(atlasLevelData[l].size) - 1)) {
      const ivec2 _xy = ivec2(atlasLevelData[l].offset) + xy;
      imageStore(level32f, _xy, v);
    }
    reductionArray[lid] = v;
    barrier();

    // Reduce add using modulo, dropping 3/4th of remaining pixels every l + 1
    int modOffset = 1;
    int modFactor = 2;
    for (int _l = l + 1; _l < maxl; _l++) {
      const ivec2 lxyMod = lxy % modFactor;
      ivec2 xyHor = lxy + ivec2(modOffset, 0);
      ivec2 xyVer = lxy + ivec2(0, modOffset);

      // Horizontal shift
      if (lxyMod.x == 0) {
        addTo(reductionArray[lid], reductionArray[toLid(xyHor)]);
      }
      barrier();

      // Vertical shift
      if (lxyMod.y == 0) {
        addTo(reductionArray[lid], reductionArray[toLid(xyVer)]);
      }
      barrier();

      // Store result in atlas layer _l
      ivec2 _xy = xy / modFactor;
      if (lxyMod == ivec2(0) && isBelow(_xy, ivec2(atlasLevelData[_l].size) - 1)) {
        _xy += ivec2(atlasLevelData[_l].offset);
        imageStore(level32f, _xy, reductionArray[lid]);
      }

      modOffset *= 2;
      modFactor *= 2;
    }
  }
);