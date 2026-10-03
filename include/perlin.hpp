#include "interval.hpp"
#include "vec3.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <vector>

class perlin {
public:
  perlin(std::mt19937 &rng) {
    perm_x.resize(256);
    perm_y.resize(256);
    perm_z.resize(256);
    for (int i = 0; i < 256; i++) {
      values[i] = random_unit_vector(rng);
      perm_x[i] = i;
      perm_y[i] = i;
      perm_z[i] = i;
    }
    std::shuffle(perm_x.begin(), perm_x.end(), rng);
    std::shuffle(perm_y.begin(), perm_y.end(), rng);
    std::shuffle(perm_z.begin(), perm_z.end(), rng);
  }

  double noise(const point3 &p) const {
    point3 fractional;
    point3 integers;
    for (int i = 0; i < 3; i++) {
      integers[i] = static_cast<int>(std::floor(p[i]));
      fractional[i] = p[i] - integers[i];
    }
    std::array<vec3, 8> c_abc =
        corner_values(integers[0], integers[1], integers[2]);
    return perlin_interpolation(c_abc, fractional[0], fractional[1],
                                fractional[2]);
  }

private:
  std::array<vec3, 256> values;
  std::vector<int> perm_x;
  std::vector<int> perm_y;
  std::vector<int> perm_z;

  std::array<vec3, 8> corner_values(int i, int j, int k) const {
    std::array<vec3, 8> c;
    // 2x2x2 array, strides 2^2, 2^1, 2^0
    for (int n = 0; n < 2; n++) {
      for (int m = 0; m < 2; m++) {
        for (int l = 0; l < 2; l++) {
          c[n * 4 + m * 2 + l] =
              values[perm_x[(i + n) & 255] ^ perm_y[(j + m) & 255] ^
                     perm_z[(k + l) & 255]];
        }
      }
    }
    return c;
  }
  static double omega(int a, double u) {
    assert(a == 0 || a == 1);
    return a * u + (1 - a) * (1 - u);
  }

  static double trilerp(const std::array<double, 8> &corners, double u,
                        double v, double w) {
    double sum = 0;
    for (int a = 0; a < 2; a++) {
      for (int b = 0; b < 2; b++) {
        for (int c = 0; c < 2; c++) {
          sum += omega(a, u) * omega(b, v) * omega(c, w) *
                 corners[a * 4 + b * 2 + c];
        }
      }
    }
    return sum;
  }
  static double perlin_interpolation(const std::array<vec3, 8> &corners,
                                     double u, double v, double w) {
    double sum = 0;
    for (int a = 0; a < 2; a++) {
      for (int b = 0; b < 2; b++) {
        for (int c = 0; c < 2; c++) {
          sum += omega(a, hermite3(u)) * omega(b, hermite3(v)) *
                 omega(c, hermite3(w)) *
                 dot(corners[a * 4 + b * 2 + c], vec3(u - a, v - b, w - c));
        }
      }
    }
    return sum;
  }

  static double hermite3(double x) { return x * x * (3 - 2 * x); }
};

inline double turbulence(const perlin &gen, point3 p, int depth) {
  double sum = 0;
  double weight = 1;
  for (int k = 0; k < depth; k++) {
    sum += weight * gen.noise(p);
    weight *= 0.5;
    p *= 2;
  }
  return std::abs(sum);
}