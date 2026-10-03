#pragma once
#include "interval.hpp"
#include "ray.hpp"
#include "vec3.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>

class aabb {
public:
  aabb() = default;
  aabb(interval a, interval b, interval c) : intervals{a, b, c} {}
  aabb(const point3 &a, const point3 &b) {
    for (int n = 0; n < 3; n++) {
      intervals[n] = interval(std::min(a[n], b[n]), std::max(a[n], b[n]));
    }
  }
  aabb(const aabb &a, const aabb &b) {
    for (int n = 0; n < 3; n++) {
      intervals[n] = interval(a.axis_interval(n), b.axis_interval(n));
    }
  }

  const interval &axis_interval(int n) const {
    assert(0 <= n && n < 3);
    return intervals[n];
  }

  bool hit(const ray &r, interval ray_t) const {
    for (int n = 0; n < 3; n++) {
      const interval &i = axis_interval(n);
      double di_inv = 1 / r.d()[n];
      double t0 = (i.min - r.o()[n]) * di_inv;
      double t1 = (i.max - r.o()[n]) * di_inv;

      if (t0 > t1) {
        ray_t.min = std::max(ray_t.min, t1);
        ray_t.max = std::min(ray_t.max, t0);
      } else {
        ray_t.min = std::max(ray_t.min, t0);
        ray_t.max = std::min(ray_t.max, t1);
      }
      if (ray_t.max <= ray_t.min) {
        return false;
      }
    }
    return true;
  }

  int longest_axis() const {
    int axis = 0;
    double biggest_so_far = 0;
    for (int n = 0; n < 3; n++) {
      double s = intervals[n].size();
      if (s > axis) {
        axis = s;
        biggest_so_far = n;
      }
    }
    return biggest_so_far;
  }

private:
  std::array<interval, 3> intervals;
};
