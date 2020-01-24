/*
*
* Copyright (c) 2014, Nicola Pezzotti (Delft University of Technology)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. All advertising materials mentioning features or use of this software
*    must display the following acknowledgement:
*    This product includes software developed by the Delft University of Technology.
* 4. Neither the name of the Delft University of Technology nor the names of
*    its contributors may be used to endorse or promote products derived from
*    this software without specific prior written permission.
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
*
*/

#ifndef GRADIENT_DESCENT_TSNE_TEXTURE_H
#define GRADIENT_DESCENT_TSNE_TEXTURE_H

#include <map>
#include <unordered_map>
#include "hdi/utils/assert_by_exception.h"
#include "hdi/dimensionality_reduction/gpgpu_sne/gpgpu_sne_compute.h"
#include "hdi/dimensionality_reduction/gpgpu_sne/gpgpu_sne_raster.h"
#include "hdi/dimensionality_reduction/abstract_gradient_descent_tsne.h"

namespace hdi::dr {
  //! tSNE with sparse and user-defined probabilities
  /*!
  Implementation of the tSNE algorithm with sparse and user-defined probabilities
  \author Nicola Pezzotti
  */
  class GradientDescentTSNETexture : public AbstractGradientDescentTSNE {
  public:
    GradientDescentTSNETexture();
    ~GradientDescentTSNETexture();
    
    void initialize(const sparse_scalar_matrix_t& probabilities,
                    data::Embedding<scalar_t>* embedding, 
                    TsneParameters params = TsneParameters()) override;
    
    //! Initialize the class with a joint-probability distribution. Note that it must be provided non initialized and with the weight of each row equal to 2.
    void initializeWithJointProbabilityDistribution(const sparse_scalar_matrix_t& distribution, 
                                                    data::Embedding<scalar_t>* embedding, 
                                                    TsneParameters params = TsneParameters());

    void iterate(double mult = 1.0) override;

    //! Get the position in the embedding for a data point
    void getEmbeddingPosition(scalar_vector_t& embedding_position, 
                              data_handle_t handle) const;

    //! Get the number of data points
    unsigned int getNumberOfDataPoints() { 
      return _P.size(); 
    }

    //! Get P
    const sparse_scalar_matrix_t& getDistributionP() const { 
      return _P; 
    }

    //! Get Q
    const scalar_vector_t& getDistributionQ() const { 
      return _Q; 
    }

    //! Set the adaptive texture scaling
    void setResolutionFactor(float factor) {
#ifndef __APPLE__
      if (GLAD_GL_VERSION_4_3)
      {
        _gpgpu_compute_tsne.setScalingFactor(factor);
      }
  else if (GLAD_GL_VERSION_3_3)
#endif // __APPLE__
      {
        _gpgpu_raster_tsne.setScalingFactor(factor);
      }
    }

    Bounds2D bounds() const { 
      return _gpgpu_compute_tsne.bounds(); 
    }

  private:
    //! Compute High-dimensional distribution
    void computeHighDimensionalDistribution(const sparse_scalar_matrix_t& probabilities);
    //! Initialize the point in the embedding
    void initializeEmbeddingPosition(int seed, double multiplier = .1);
    //! Do an iteration of the gradient descent
    void doAnIterationExact(double mult = 1);
    //! Do an iteration of the gradient descent
    void doAnIterationBarnesHut(double mult = 1);
    //! Compute Low-dimensional distribution
    void computeLowDimensionalDistribution();
    //! Compute tSNE gradient with the texture based algorithm
    void doAnIterationImpl(double exaggeration);

    //! Compute the exaggeration factor based on the current iteration
    double exaggerationFactor() const;

  private:
    scalar_vector_t* _embedding_container;
    scalar_vector_t _Q; //! Conditional probalility distribution in the Low-dimensional space
    scalar_t _normalization_Q; //! Normalization factor of Q - Z in the original paper

    // Underlying implementation
#ifndef __APPLE__
    GpgpuSneCompute _gpgpu_compute_tsne;
#endif // __APPLE__
    GpgpuSneRaster _gpgpu_raster_tsne;
  };
}
#endif
