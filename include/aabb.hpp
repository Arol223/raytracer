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
  aabb(interval a, interval b, interval c) : intervals{a, b, c} {
    pad_to_minimums();
  }
  aabb(const point3 &a, const point3 &b) {
    for (int n = 0; n < 3; n++) {
      intervals[n] = interval(std::min(a[n], b[n]), std::max(a[n], b[n]));
    }
    pad_to_minimums();
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
      if (s > biggest_so_far) {
        axis = n;
        biggest_so_far = s;
      }
    }
    return axis;
  }

private:
  std::array<interval, 3> intervals;
  void pad_to_minimums() {
    for (interval &i : intervals) {
      if (i.size() < 0.0001) {
        i = i.expand(0.0001);
      }
    }
  }
};

inline aabb operator+(const aabb &box, const vec3 &shift) {
  return aabb(box.axis_interval(0) + shift.x(),
              box.axis_interval(1) + shift.y(),
              box.axis_interval(2) + shift.z());
}

inline aabb operator+(const vec3 &shift, const aabb &box) {
  return box + shift;
}

inline std::array<vec3, 8> get_corners(const aabb &box) {
  std::array<interval, 3> intervals;
  for (int i = 0; i < 3; i++) {
    intervals[i] = box.axis_interval(i);
  }
  std::array<vec3, 8> corners;
  corners[0] = vec3(intervals[0].min, intervals[1].min, intervals[2].min);
  corners[1] = vec3(intervals[0].min, intervals[1].min, intervals[2].max);
  corners[2] = vec3(intervals[0].min, intervals[1].max, intervals[2].min);
  corners[3] = vec3(intervals[0].max, intervals[1].min, intervals[2].min);
  corners[4] = vec3(intervals[0].max, intervals[1].max, intervals[2].min);
  corners[5] = vec3(intervals[0].min, intervals[1].max, intervals[2].max);
  corners[6] = vec3(intervals[0].max, intervals[1].min, intervals[2].max);
  corners[7] = vec3(intervals[0].max, intervals[1].max, intervals[2].max);
  return corners;
}
