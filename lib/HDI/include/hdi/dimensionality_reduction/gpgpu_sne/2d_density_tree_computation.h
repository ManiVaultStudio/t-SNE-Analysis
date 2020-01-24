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

#include <array>
#include "hdi/utils/abstract_log.h"
#include "hdi/data/shader.h"
#include "hdi/dimensionality_reduction/tsne_parameters.h"
#include "gpgpu_utils.h"
#include "2d_utils.h"

namespace hdi::dr {
  struct AtlasLevelBlock {
    Point2Dui offset; // Offset of pixel block in atlas texture
    Point2Dui size; // Size of pixel block in atlas texture
  };

  class Density2dTreeComputation {
  public:
    Density2dTreeComputation();
    ~Density2dTreeComputation();

    // Initialize gpu components for computation
    void initialize(const TsneParameters& params,
                    unsigned n);

    // Remove gpu components
    void clean();

    // Compute field and depth textures
    void compute(unsigned w, unsigned h, unsigned n,
                 GLuint position_buff, GLuint bounds_buff,
                 Bounds2D bounds);

    // Assign logger
    void setLogger(utils::AbstractLog* logger) {
      _logger = logger; 
    }

    // Obtain density atlas texture
    GLuint getDensityAtlasTexture() const {
      return _textures[TEXTURE_DENSITY_ATLAS_32F];
    }

    // Obtain density atlas buffer data
    GLuint getDensityAtlasBuffer() const {
      return _buffers[BUFFER_ATLAS_DATA];
    }

    // Obtain density atlas layout
    std::vector<AtlasLevelBlock> getDensityAtlasLevels() const {
      return _atlasLevels;
    }

    // Obtain density atlas size
    Point2Dui getDensityAtlasSize() const {
      return _atlasDimensions;
    }

  private:
    enum TextureType {
      TEXTURE_DENSITY_ATLAS_32F,
      TextureTypeLength
    };

    enum BufferType {
      BUFFER_ATLAS_DATA,
      BufferTypeLength
    };

    enum ProgramType {
      PROGRAM_DENSITY_LOD_BOTTOM,
      PROGRAM_DENSITY_LOD_BOTTOM_DIVIDE,
      PROGRAM_DENSITY_LOD_UPPER,
      ProgramTypeLength
    };

    enum FramebufferType {
      FBO_DENSITY_LOD_BOTTOM,
      FramebufferTypeLength
    };

    bool _initialized;
    int _iteration;
    unsigned _w, _h;

    std::array<ShaderProgram, ProgramTypeLength> _programs;
    std::array<GLuint, TextureTypeLength> _textures;
    std::array<GLuint, BufferTypeLength> _buffers;
    std::array<GLuint, FramebufferTypeLength> _framebuffers;

    GLuint _point_vao;
    std::vector<AtlasLevelBlock> _atlasLevels;
    Point2Dui _atlasDimensions;
    TsneParameters _params;
    utils::AbstractLog* _logger;

    TIMERS_DECLARE (
      TIMER_DENSITY_LOD_BOTTOM,
      TIMER_DENSITY_LOD_BOTTOM_DIVIDE,
      TIMER_DENSITY_LOD_UPPER
    )
  };
}