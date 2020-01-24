#pragma once

namespace hdi::dr {
  // Simple 2D unsigned point representation
  struct alignas(8) Point2Dui {
    unsigned x, y;
  };

  // Simple 2D floating point representation
  struct alignas(8) Point2D {
    float x, y;
  };

  // Simple 2D bounds representation
  struct Bounds2D {
    Point2D min, max;

    Point2D range() {
      return Point2D {
        max.x - min.x,
        max.y - min.y
      };
    }

    Point2D center() {
      return Point2D {
        0.5f * (min.x + max.x),
        0.5f * (min.y + max.y)
      };
    }
  };
};