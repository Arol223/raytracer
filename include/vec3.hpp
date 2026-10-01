#pragma once
#include "interval.hpp"
#include <cassert>
#include <cmath>
#include <ostream>
#include <random>

namespace {
double linear_to_gamma(double x) { return x > 0 ? std::sqrt(x) : 0.0; }
} // namespace
class vec3 {
public:
  double e[3];

  vec3() : e{0, 0, 0} {}
  vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

  double x() const { return e[0]; }
  double y() const { return e[1]; }
  double z() const { return e[2]; }

  double operator[](int i) const {
    assert(0 <= i && i <= 2);
    return e[i];
  }
  double &operator[](int i) {
    assert(0 <= i && i <= 2);
    return e[i];
  }
  vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }

  vec3 &operator+=(const vec3 &other) {
    e[0] += other.e[0];
    e[1] += other.e[1];
    e[2] += other.e[2];
    return *this;
  }

  vec3 &operator*=(const double t) {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
  }

  vec3 &operator/=(const double t) {
    e[0] /= t;
    e[1] /= t;
    e[2] /= t;
    return *this;
  }

  double length_squared() const {
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
  }

  double length() const { return std::sqrt(length_squared()); }
};

inline vec3
operator+(const vec3 &x,
          const vec3 &y) { // could refactor to vec3 x, const vec3& y and use +=
  return vec3(x.x() + y.x(), x.y() + y.y(), x.z() + y.z());
}

inline vec3 operator-(const vec3 &x, const vec3 &y) {
  return vec3(x.x() - y.x(), x.y() - y.y(), x.z() - y.z());
}

inline vec3 operator*(const vec3 &x, const vec3 &y) {
  return vec3(x.x() * y.x(), x.y() * y.y(), x.z() * y.z());
}

inline vec3 operator*(const vec3 &x, double t) {
  return vec3(x.x() * t, x.y() * t, x.z() * t);
}

inline vec3 operator*(double t, const vec3 &x) {
  return vec3(x.x() * t, x.y() * t, x.z() * t);
}

inline vec3 operator/(const vec3 &x, double t) {
  return vec3(x.x() / t, x.y() / t, x.z() / t);
}

inline double dot(const vec3 &x, const vec3 &y) {
  return x.x() * y.x() + x.y() * y.y() + x.z() * y.z();
}

inline vec3 cross(const vec3 &x, const vec3 &y) {
  return vec3(x[1] * y[2] - x[2] * y[1], -(x[0] * y[2] - x[2] * y[0]),
              x[0] * y[1] - x[1] * y[0]);
}

inline vec3 unit_vector(const vec3 &x) { return x / x.length(); }

using point3 = vec3;
using color = vec3;

inline void write_color(std::ostream &f, const color &c) {
  const double scale = 255.999;
  interval bounds = interval(0.000, 0.999);
  f << int(bounds.clamp(linear_to_gamma(c[0])) * scale) << " "
    << int(bounds.clamp(linear_to_gamma(c[1])) * scale) << " "
    << int(bounds.clamp(linear_to_gamma(c[2])) * scale) << "\n";
}

inline std::ostream &operator<<(std::ostream &out, const vec3 &v) {
  return out << v[0] << ' ' << v[1] << ' ' << v[2];
}

inline vec3 random_vec(std::mt19937 &rng, double min, double max) {
  auto dist = std::uniform_real_distribution<double>(min, max);
  double x = dist(rng);
  double y = dist(rng);
  double z = dist(rng);
  return vec3(x, y, z);
}

inline vec3 random_vec(std::mt19937 &rng) { return random_vec(rng, 0, 1); }

inline vec3 random_unit_vector(std::mt19937 &rng) {
  while (true) {
    vec3 vec = random_vec(rng, -1, 1);
    double l2 = vec.length_squared();
    if (1e-160 < l2 && l2 <= 1)
      return vec / std::sqrt(l2);
  }
}

inline vec3 random_on_hemisphere(std::mt19937 &rng, const vec3 &normal) {
  vec3 vec = random_unit_vector(rng);

  return dot(vec, normal) < 0 ? -vec : vec;
}