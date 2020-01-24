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

#ifndef ABSTRACT_GRADIENT_DESCENT_TSNE
#define ABSTRACT_GRADIENT_DESCENT_TSNE

#include <cstdint>
#include <vector>
#include "hdi/utils/abstract_log.h"
#include "hdi/utils/math_utils.h"
#include "hdi/data/embedding.h"
#include "hdi/data/map_mem_eff.h"
#include "hdi/dimensionality_reduction/tsne_parameters.h"

namespace hdi::dr {
   //! T-SNE gradient descent abstract class
  /*!
    T-SNE gradient descent abstract class
    \author Mark van de Ruit
  */
  class AbstractGradientDescentTSNE {
  public:
    // Internal types
    typedef float scalar_t;
    typedef std::vector<scalar_t> scalar_vector_t;
    typedef uint32_t data_handle_t;
    typedef std::vector<data::MapMemEff<data_handle_t, scalar_t>> sparse_scalar_matrix_t;
    
    AbstractGradientDescentTSNE()
    : _initialized(false),
      _logger(nullptr),
      _exaggeration_baseline(1.0),
      _iteration(0) { }
    
    virtual ~AbstractGradientDescentTSNE() { }
    
    // Reset the internal state of the iterator but keep embedding
    virtual void reset() {
      _initialized = false;
      _iteration = 0;
    }

    // Reset the internal state of the iterator and erase embedding
    virtual void clear() {
      reset();
      _embedding->clear();
    }

    // Initialize the class with a set of distributions and parameters
    virtual void initialize(const sparse_scalar_matrix_t& probabilities,
                    data::Embedding<scalar_t>* embedding, 
                    TsneParameters params = TsneParameters()) = 0;

    // Perform one iteration of t-SNE
    virtual void iterate(double mult = 1.0) = 0;

    // Get the current logger
    utils::AbstractLog* logger() const {
      return _logger;
    }

    // Set the current logger
    virtual void setLogger(utils::AbstractLog* logger) {
      _logger = logger; 
    }

    // Get the current iteration
    unsigned int iteration() const {
      return _iteration;
    }

    // Compute KL divergence over the current embedding
    double computeKullbackLeiblerDivergence() const {
      const int n = _embedding->numDataPoints();

      double sum_Q = 0.0;
      #pragma omp parallel for reduction(+: sum_Q)
      for (int j = 0; j < n; ++j) {
        double _sum_Q = 0.0;
        for (int i = j + 1; i < n; ++i) {
          const double euclidean_dist_sq(
            utils::euclideanDistanceSquared<float>(
              _embedding->getContainer().begin() + j * _params._embedding_dimensionality,
              _embedding->getContainer().begin() + (j + 1) * _params._embedding_dimensionality,
              _embedding->getContainer().begin() + i * _params._embedding_dimensionality,
              _embedding->getContainer().begin() + (i + 1) * _params._embedding_dimensionality
              )
          );
          const double v = 1. / (1. + euclidean_dist_sq);
          _sum_Q += v * 2;
        }
        sum_Q += _sum_Q;
      }

      double kl = 0.0;
      #pragma omp parallel for reduction(+: kl)
      for (int i = 0; i < n; ++i) {
        double _kl = 0.0;
        for (const auto& pij : _P[i]) {
          uint32_t j = pij.first;
          // Calculate Qij
          const double euclidean_dist_sq(
            utils::euclideanDistanceSquared<float>(
              _embedding->getContainer().begin() + j*_params._embedding_dimensionality,
              _embedding->getContainer().begin() + (j + 1)*_params._embedding_dimensionality,
              _embedding->getContainer().begin() + i*_params._embedding_dimensionality,
              _embedding->getContainer().begin() + (i + 1)*_params._embedding_dimensionality
              )
          );
          const double v = 1. / (1. + euclidean_dist_sq);
          double p = pij.second / (2 * n);
          double klc = p * std::log(p / (v / sum_Q));
          _kl += klc;
        }
        kl += _kl;
      }
      return kl;
    }

  protected:
    bool _initialized;
    unsigned int _iteration;
    double _exaggeration_baseline;
    data::Embedding<scalar_t>* _embedding;
    sparse_scalar_matrix_t _P;
    TsneParameters _params;
    utils::AbstractLog* _logger;
  };
}

#endif // ABSTRACT_GRADIENT_DESCENT_TSNE