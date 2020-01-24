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

#include <iostream>
#include <vector>
#include "hdi/data/embedding.h"
#include "hdi/data/map_mem_eff.h"
#include "hdi/utils/scoped_timers.h"
#include "hdi/utils/glad.h"

// Toggle to globally enable/disable GL_ASSERT()
// #define UTILS_GL_ASSERTS
#ifdef UTILS_GL_ASSERTS
#include <sstream>
#endif // UTILS_GL_ASSERTS

// Toggle to globally enable/disable GL_TICK GL_TOCK
// #define UTILS_QUERY_TIMERS 
#ifdef UTILS_QUERY_TIMERS
#include <array>
#include <deque>
#include <numeric>
#include <stdexcept>
#endif // UTILS_QUERY_TIMERS

namespace hdi::dr {
  // Internal types
  typedef hdi::data::Embedding<float> embedding_t;
  typedef std::vector<hdi::data::MapMemEff<uint32_t, float>> sparse_matrix_t;

  // Linearized representation of sparse probability matrix
  struct LinearProbabilityMatrix {
    std::vector<uint32_t> neighbours;
    std::vector<float> probabilities;
    std::vector<int> indices;
  };

  inline LinearProbabilityMatrix linearizeProbabilityMatrix(const embedding_t* embedding, const sparse_matrix_t& P) {
    LinearProbabilityMatrix LP;
    LP.indices.reserve(2 * embedding->numDataPoints());
    LP.neighbours.reserve(500 * embedding->numDataPoints());
    LP.probabilities.reserve(500 * embedding->numDataPoints());

    for (size_t i = 0; i < embedding->numDataPoints(); i++) {
      LP.indices.push_back(static_cast<int>(LP.neighbours.size()));\
      for (const auto& p_ij : P[i]) {
        LP.neighbours.push_back(p_ij.first);
        LP.probabilities.push_back(p_ij.second);
      }
      LP.indices.push_back(static_cast<int>(P[i].size()));
    }
    return LP;
  }

  // For aggressive debugging
#ifdef UTILS_GL_ASSERTS
 #define GL_ASSERT(msg) \
    { \
      GLenum err; \
      while ((err = glGetError()) != GL_NO_ERROR) { \
        std::stringstream ss; \
        ss << "glAssert failed at: " << msg << ", with " << err; \
        throw std::runtime_error(ss.str()); \
      } \
    }
#else
  #define GL_ASSERT(msg)
#endif // UTILS_GL_ASSERTS

#ifdef UTILS_QUERY_TIMERS
  // GL timer query wrapper class
  class glTimerQuery {
  public:
    glTimerQuery()
    : _initialized(false) { }

    ~glTimerQuery() {
      if (_initialized) {
        destroy();
      }
    }
    
    void create() {
      if (!_initialized) {
        glGenQueries(1, &_handle);
        _values.fill(0);
        _iteration = 0;
        _initialized = true;
      }
    }

    void destroy() {
      if (_initialized) {
        glDeleteQueries(1, &_handle);
        _values.fill(0);
        _iteration = 0;
        _initialized = false;
      }
    }

    void tick() {
      assertInitialized();
      glBeginQuery(GL_TIME_ELAPSED, _handle);
    }

    void tock() {
      assertInitialized();
      glEndQuery(GL_TIME_ELAPSED);
    }

    bool poll() {
      assertInitialized();
      int i = 0;
      glGetQueryObjectiv(_handle, GL_QUERY_RESULT_AVAILABLE, &i);
      if (!i) {
        return GL_FALSE;
      }
      
      glGetQueryObjecti64v(_handle, GL_QUERY_RESULT, &_values[TIMER_LAST_QUERY]);
      _values[TIMER_TOTAL] += _values[TIMER_LAST_QUERY];
      _values[TIMER_AVERAGE] 
        = _values[TIMER_AVERAGE] 
        + (_values[TIMER_LAST_QUERY] - _values[TIMER_AVERAGE]) 
        / (++_iteration);

      return GL_TRUE;
    }

    double averageMillis() const {
      return static_cast<double>(_values[TIMER_AVERAGE]) / 1000000.0;
    }

    double averageMicros() const {
      return static_cast<double>(_values[TIMER_AVERAGE]) / 1000.0;
    }

    double totalMillis() const {
      return static_cast<double>(_values[TIMER_TOTAL]) / 1000000.0;
    }

  private:
    enum ValueType {
      TIMER_LAST_QUERY,
      TIMER_AVERAGE,
      TIMER_TOTAL,

      // Static enum length
      ValueTypeLength 
    };

    bool _initialized;
    GLuint _handle;
    unsigned _iteration;
    std::array<GLint64, ValueTypeLength> _values;

    void assertInitialized() {
      if (!_initialized) {
        throw std::runtime_error("glTimerQuery not initialized");
      }
    }
  };

  #define TIMERS_DECLARE(...) \
    enum glTimerQueryType { \
      __VA_ARGS__,\
      glTimerQueryTypeLength \
    }; \
    std::array<glTimerQuery, glTimerQueryTypeLength> glTimerQueries;

  #define TIMERS_CREATE() \
    for (auto& t : glTimerQueries) t.create();
  
  #define TIMERS_DESTROY() \
    for (auto& t : glTimerQueries) t.destroy();

  #define TIMERS_UPDATE() \
    { \
      std::deque<int> types(glTimerQueryTypeLength); \
      std::iota(types.begin(), types.end(), 0); \
      while (!types.empty()) { \
        const int t = types.front(); \
        types.pop_front(); \
        if (!glTimerQueries[t].poll()) { \
          types.push_back(t); \
        } \
      } \
    }
  
  #define TIMER_TICK(name) \
    glTimerQueries[name].tick();

  #define TIMER_TOCK(name) \
    glTimerQueries[name].tock();

  #ifdef _WIN32
    #define TIMER_LOG(logger, name, str) \
      utils::secureLogValue(logger, \
        std::string(str) + " average (\xE6s)", \
        std::to_string(glTimerQueries[name].averageMicros()) \
        );
  #else
    #define TIMER_LOG(logger, name, str) \
        utils::secureLogValue(logger, \
          std::string(str) + " average (\xC2\xB5s)", \
          std::to_string(glTimerQueries[name].averageMicros()) \
          );
  #endif
  
  #define TIMERS_LOG_ENABLE true
#else
  #define TIMERS_DECLARE(...)
  #define TIMERS_CREATE()
  #define TIMERS_DESTROY()
  #define TIMERS_UPDATE()
  #define TIMER_TICK(name)
  #define TIMER_TOCK(name)
  #define TIMER_LOG(logger, name, str)
  #define TIMERS_LOG_ENABLE false
#endif // UTILS_QUERY_TIMERS
}