#pragma once
#include "rtweekend.hpp"
#include <cassert>
#include <cmath>

struct interval {

  interval(double min, double max) : min(min), max(max) {}
  interval() : min(infinity), max(-infinity) {}
  interval(const interval &i1, const interval &i2) {
    min = std::min(i1.min, i2.min);
    max = std::max(i1.max, i2.max);
  }
  static const interval empty, universe;
  bool contains(double x) const { return (min <= x && x <= max); }
  bool surrounds(double x) const { return (min < x && x < max); }
  double min;
  double max;

  double clamp(const double x) const {
    if (x < min) {
      return min;
    } else if (x > max) {
      return max;
    } else {
      return x;
    }
  }

  double size() const { return max - min; }

  interval expand(double delta) const {
    return interval(min - delta / 2, max + delta / 2);
  }
};

inline const interval interval::empty = interval(+infinity, -infinity);
inline const interval interval::universe = interval(-infinity, +infinity);
