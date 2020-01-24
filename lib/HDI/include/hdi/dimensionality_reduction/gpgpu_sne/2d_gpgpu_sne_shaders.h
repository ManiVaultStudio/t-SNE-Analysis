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

GLSL(sumq_src, 430,
  layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
  layout(std430, binding = 0) buffer Val { vec3 Values[]; };
  layout(std430, binding = 1) buffer SumQReduceAddInterface { float SumReduceAdd[128]; };
  layout(std430, binding = 2) buffer SumQInterface { 
    float sumQ;
    float invSumQ;
  };

  uniform uint num_points;
  uniform uint iteration;
  const uint groupSize = gl_WorkGroupSize.x;
  const uint halfGroupSize = groupSize / 2;
  shared float reduction_array[halfGroupSize];

  void main() {
    uint lid = gl_LocalInvocationIndex.x;
    float sum = 0.f;
    if (iteration == 0) {
      // First iteration adds all values
      for (uint i = gl_WorkGroupID.x * gl_WorkGroupSize.x + lid;
          i < num_points;
          i += gl_WorkGroupSize.x * gl_NumWorkGroups.x) {
        sum += max(Values[i].x - 1.f, 0f);
      }
    } else if (iteration == 1) {
      // Second iteration adds resulting 128 values
      sum = SumReduceAdd[lid];      
    }

    // Reduce add to a single value
    if (lid >= halfGroupSize) {
      reduction_array[lid - halfGroupSize] = sum;
    }
    barrier();
    if (lid < halfGroupSize) {
      reduction_array[lid] += sum;
    }
    for (uint i = halfGroupSize / 2; i > 1; i /= 2) {
      barrier();
      if (lid < i) {
        reduction_array[lid] += reduction_array[lid + i];
      }
    }
    barrier();
    if (lid < 1) {
      if (iteration == 0) {
        SumReduceAdd[gl_WorkGroupID.x] = reduction_array[0] + reduction_array[1];
      } else if (iteration == 1) {
        sumQ = reduction_array[0] + reduction_array[1];
        invSumQ = 1.f / sumQ;
      }
    }
  }
);

GLSL(positive_forces_src, 430,
  layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;
  layout(std430, binding = 0) buffer Pos{ vec2 Positions[]; };
  layout(std430, binding = 1) buffer Neigh { uint Neighbours[]; };
  layout(std430, binding = 2) buffer Prob { float Probabilities[]; };
  layout(std430, binding = 3) buffer Ind { int Indices[]; };
  layout(std430, binding = 4) buffer Posit { vec2 PositiveForces[]; };

  const uint groupSize = gl_WorkGroupSize.x;
  const uint halfGroupSize = groupSize / 2;
  uniform uint num_points;
  uniform float inv_num_points;
  shared vec2 reduction_array[halfGroupSize];

  void main () {
    const uint i = gl_WorkGroupID.x;
    const uint lid = gl_LocalInvocationID.x;
    if (i >= num_points) {
      return;
    }

    // Computing positive forces using nearest neighbors
    vec2 point_i = Positions[i];
    int index = Indices[i * 2 + 0];
    int size = Indices[i * 2 + 1];
    vec2 positive_force = vec2(0);
    for (uint j = lid; j < size; j += groupSize) {
      // Get other point coordinates
      vec2 point_j = Positions[Neighbours[index + j]];

      // Calculate 2D distance between the two points
      vec2 dist = point_i - point_j;

      // Similarity measure of the two points
      float qij = 1.f + dot(dist, dist);

      // Calculate the attractive force
      positive_force += Probabilities[index + j] * dist / qij;
    }
    positive_force *= inv_num_points;

    // Reduce add positive_force to a single value
    if (lid >= halfGroupSize) {
      reduction_array[lid - halfGroupSize] = positive_force;
    }
    barrier();
    if (lid < halfGroupSize) {
      reduction_array[lid] += positive_force;
    }
    for (uint reduceSize = halfGroupSize / 2; reduceSize > 1; reduceSize /= 2) {
      barrier();
      if (lid < reduceSize) {
        reduction_array[lid] += reduction_array[lid + reduceSize];
      }
    }
    barrier();
    if (lid < 1) {
      PositiveForces[i] = reduction_array[0] + reduction_array[1];
    } 
  }
);

// Copied from compute_shaders.glsl, adapted for 3d
GLSL(gradients_src, 430,
  layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
  layout(std430, binding = 0) buffer Posit { vec2 PositiveForces[]; };
  layout(std430, binding = 1) buffer Fiel { vec3 Fields[]; };
  layout(std430, binding = 2) buffer SumQInterface { 
    float sumQ;
    float invSumQ;
  };
  layout(std430, binding = 3) buffer Grad { vec2 Gradients[]; };

  uniform uint num_points;
  uniform float exaggeration;

  void main() {
    uint i = gl_WorkGroupID.x * gl_WorkGroupSize.x + gl_LocalInvocationIndex.x;
    if (i >= num_points) {
      return;
    }

    vec2 positive = PositiveForces[i];
    vec2 negative = Fields[i].yz * invSumQ;
    Gradients[i] = 4.f * (exaggeration * positive - negative);
  }
);

// Copied from compute_shaders.glsl, adapted for 3d
GLSL(update_src, 430,
  layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
  layout(std430, binding = 0) buffer Pos{ vec2 Positions[]; };
  layout(std430, binding = 1) buffer GradientLayout { vec2 Gradients[]; };
  layout(std430, binding = 2) buffer PrevGradientLayout { vec2 PrevGradients[]; };
  layout(std430, binding = 3) buffer GainLayout { vec2 Gain[]; };

  uniform uint num_points;
  uniform float eta;
  uniform float minGain;
  uniform float mult;
  uniform float iter_mult;

  vec2 dir(vec2 v) {
    return vec2(
      v.x > 0 ? 1 : -1, 
      v.y > 0 ? 1 : -1
    );
  }

  vec2 matches(vec2 a, vec2 b) {
    return vec2(
      sign(a.x) != sign(b.x) ? 1 : 0, 
      sign(a.y) != sign(b.y) ? 1 : 0
    );
  }

  void main() {
    uint i = gl_WorkGroupID.x * gl_WorkGroupSize.x + gl_LocalInvocationIndex.x;
    if (i >= num_points) {
      return;
    }
    
    vec2 grad = Gradients[i];
    vec2 pgrad = PrevGradients[i];
    vec2 gain = Gain[i];

    gain = mix(gain * 0.8, gain + 0.2, matches(grad, pgrad));
    gain = max(gain, vec2(minGain));

    vec2 etaGain = eta * gain;
    grad = dir(grad) * abs(grad * etaGain) / etaGain;
    pgrad = iter_mult * pgrad - etaGain * grad;

    Gain[i] = gain;
    PrevGradients[i] = pgrad;
    Positions[i] += pgrad * mult;
  }
);

// Copied from compute_shaders.glsl, adapted for 3d
GLSL(bounds_src, 430,
  layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
  layout(std430, binding = 0) buffer Pos{ vec2 positions[]; };
  layout(std430, binding = 1) buffer BoundsReduceAddInterface {
    vec2 minBoundsReduceAdd[128];
    vec2 maxBoundsReduceAdd[128];
  };
  layout(std430, binding = 2) buffer BoundsInterface { 
    vec2 minBounds;
    vec2 maxBounds;
    vec2 range;
    vec2 invRange;
  };

  uniform uint iteration;
  uniform uint num_points;
  uniform float padding;
  const uint halfGroupSize = gl_WorkGroupSize.x / 2;
  shared vec2 min_reduction[halfGroupSize];
  shared vec2 max_reduction[halfGroupSize];

  vec2 removeZero(in vec2 v) {
    // This can be done a lot faster
    if (v.x == 0.f) v.x = 1.f;
    if (v.y == 0.f) v.y = 1.f;
    return v;
  }

  void main() {
    const uint lid = gl_LocalInvocationIndex.x;
    vec2 min_local = vec2(1e38);
    vec2 max_local = vec2(-1e38);

    if (iteration == 0) {
      // First iteration adds all values
      for (uint i = gl_WorkGroupID.x * gl_WorkGroupSize.x + lid;
          i < num_points;
          i += gl_WorkGroupSize.x * gl_NumWorkGroups.x) {
        vec2 pos = positions[i];
        min_local = min(pos, min_local);
        max_local = max(pos, max_local);
      }
    } else if (iteration == 1) {
      // Second iteration adds resulting 128 values
      min_local = minBoundsReduceAdd[lid];
      max_local = maxBoundsReduceAdd[lid];
    }

    // Perform reduce-add over all points
    if (lid >= halfGroupSize) {
      min_reduction[lid - halfGroupSize] = min_local;
      max_reduction[lid - halfGroupSize] = max_local;
    }
    barrier();
    if (lid < halfGroupSize) {
      min_reduction[lid] = min(min_local, min_reduction[lid]);
      max_reduction[lid] = max(max_local, max_reduction[lid]);
    }
    for (uint i = halfGroupSize / 2; i > 1; i /= 2) {
      barrier();
      if (lid < i) {
        min_reduction[lid] = min(min_reduction[lid], min_reduction[lid + i]);
        max_reduction[lid] = max(max_reduction[lid], max_reduction[lid + i]);
      }
    }
    barrier();
    // perform store in starting unit
    if (lid < 1) {
      min_local = min(min_reduction[0], min_reduction[1]);
      max_local = max(max_reduction[0], max_reduction[1]);
      if (iteration == 0) {
        minBoundsReduceAdd[gl_WorkGroupID.x] = min_local;
        maxBoundsReduceAdd[gl_WorkGroupID.x] = max_local;
      } else if (iteration == 1) {
        vec2 padding = (max_local - min_local) * 0.5f * padding;
        minBounds = min_local - padding;
        maxBounds = max_local + padding;
        range = (maxBounds - minBounds);
        invRange = 1.f / removeZero(range);
      }
    }
  }
);

// Copied from compute_shader.glsl, adapted for 3d
GLSL(centering_src, 430,
  layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
  layout(std430, binding = 0) buffer Pos{ vec2 Positions[]; };
  layout(std430, binding = 1) buffer BoundsInterface { 
    vec2 minBounds;
    vec2 maxBounds;
    vec2 range;
    vec2 invRange;
  };

  uniform uint num_points;
  uniform vec2 center;
  uniform float scaling;

  void main() {
    uint i = gl_WorkGroupID.x * gl_WorkGroupSize.x + gl_LocalInvocationIndex.x;
    if (i >= num_points) {
      return;
    }
    
    Positions[i] = scaling * (Positions[i] - center); 
  }
);