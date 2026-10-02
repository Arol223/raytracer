#pragma once
#include "vec3.hpp"
#include <cassert>

class ray {
public:
  ray(point3 o, vec3 dir, double t) : origin(o), tm(t) {
    double l = dir.length();
    assert(l > 1e-8);
    direction = dir / l;
  }
  point3 o() const { return origin; }
  vec3 d() const { return direction; }
  double time() const { return tm; }

  point3 at(double t) const { return origin + t * direction; }

private:
  point3 origin;
  vec3 direction;
  double tm;
};