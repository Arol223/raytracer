#pragma once
#include <cassert>
#include <cmath>
#include <ostream>

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
  f << int(c[0] * scale) << " " << int(c[1] * scale) << " " << int(c[2] * scale)
    << "\n";
}

inline std::ostream &operator<<(std::ostream &out, const vec3 &v) {
  return out << v[0] << ' ' << v[1] << ' ' << v[2];
}