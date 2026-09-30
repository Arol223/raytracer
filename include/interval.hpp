#include "rtweekend.hpp"
#include <cassert>

struct interval {

  interval(double min, double max) : min(min), max(max) {}
  interval() : min(infinity), max(-infinity) {}
  static const interval empty, universe;
  bool contains(double x) const { return (min <= x && x <= max); }
  bool surrounds(double x) const { return (min < x && x < max); }
  double min;
  double max;
};

inline const interval interval::empty = interval(+infinity, -infinity);
inline const interval interval::universe = interval(-infinity, +infinity);