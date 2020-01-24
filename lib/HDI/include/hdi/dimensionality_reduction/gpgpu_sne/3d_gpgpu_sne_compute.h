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

#include <cstdlib>
#include "hdi/utils/abstract_log.h"
#include "hdi/data/shader.h"
#include "hdi/dimensionality_reduction/tsne_parameters.h"
#include "gpgpu_utils.h"
#include "3d_utils.h"

#define ASSERT_SUM_Q // Enable SumQ != 0 assertion
#define USE_DEPTH_FIELD // Field computation leverages depth maps
// #define USE_MIPMAP // Field computation leverages mipmaps

#ifdef USE_DEPTH_FIELD
#include "depth_field_computation.h"
#elif defined USE_MIPMAP
#include "mipmap_field_computation.h"
#else
#include "3d_field_computation.h"
#endif 

namespace hdi::dr {
  class Gpgpu3dSneCompute {
  public:
    // Base constr., destr.
    Gpgpu3dSneCompute();
    ~Gpgpu3dSneCompute();

    // Initialize gpu components for computation
    void initialize(const embedding_t* embedding, 
                    const TsneParameters& params, 
                    const sparse_matrix_t& P);

    // Remove gpu components
    void clean();

    // Perform computation for specified iteration
    void compute(embedding_t* embedding, 
                 float exaggeration, 
                 unsigned iteration, 
                 float mult);
    
    Bounds3D bounds() const {
      return _bounds;
    }

    void setLogger(utils::AbstractLog* logger) {
      _logger = logger; 
      _fieldComputation.setLogger(logger);
    }
    
  private:
    void computeBounds(unsigned n, float padding);
    void sumQ(unsigned n);
    void computeGradients(unsigned n, float exaggeration);
    void updatePoints(unsigned n, float iteration, float mult);
    void updateEmbedding(unsigned n, float exaggeration, float iteration);

    bool _initialized;
    bool _adaptive_resolution;
    float _resolution_scaling;

    enum BufferType {
      // Enums matching to storage buffers in _buffers array
      BUFFER_POSITION,
      BUFFER_INTERP_FIELDS,
      BUFFER_SUM_Q,
      BUFFER_SUM_Q_REDUCE_ADD,
      BUFFER_NEIGHBOUR,
      BUFFER_PROBABILITIES,
      BUFFER_INDEX,
      BUFFER_POSITIVE_FORCES,
      BUFFER_GRADIENTS,
      BUFFER_PREV_GRADIENTS,
      BUFFER_GAIN,
      BUFFER_BOUNDS_REDUCE_ADD,
      BUFFER_BOUNDS,
      
      // Static enum length
      BufferTypeLength 
    };

    enum ProgramType {
      // Enums matching to shader programs in _programs array
      PROGRAM_SUM_Q,
      PROGRAM_POSITIVE_FORCES,
      PROGRAM_GRADIENTS,
      PROGRAM_UPDATE,
      PROGRAM_BOUNDS,
      PROGRAM_CENTERING,
      
      // Static enum length
      ProgramTypeLength 
    };

    std::array<GLuint, BufferTypeLength> _buffers;
    std::array<ShaderProgram, ProgramTypeLength> _programs;
#ifdef USE_DEPTH_FIELD
    DepthFieldComputation _fieldComputation;
#elif defined USE_MIPMAP
    MipmapFieldComputation _fieldComputation;
#else
    BaselineFieldComputation _fieldComputation;
#endif
    TsneParameters _params;
    Bounds3D _bounds;
    utils::AbstractLog* _logger;

    TIMERS_DECLARE(
      TIMER_SUM_Q,
      TIMER_GRADIENTS,
      TIMER_UPDATE,
      TIMER_BOUNDS,
      TIMER_CENTERING
    )
  };
}