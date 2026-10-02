#pragma once
#include <limits>
#include <numbers>
#include <random>

inline constexpr double infinity = std::numeric_limits<double>::infinity();
inline constexpr double pi = std::numbers::pi;
inline double random_double(std::mt19937 &rng) {
  auto dist = std::uniform_real_distribution<double>();
  return dist(rng);
}
inline double random_double(std::mt19937 &rng, double min, double max) {
  auto dist = std::uniform_real_distribution<double>(min, max);
  return dist(rng);
}

inline double deg_to_rad(double deg) { return deg * pi / 180; }