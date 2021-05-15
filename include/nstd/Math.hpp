
#pragma once

#include <nstd/Base.hpp>

class Math
{
public:
  template<typename T> static const T& max(const T& a, const T& b) {return a > b ? a : b;}
  template<typename T> static const T& min(const T& a, const T& b) {return a < b ? a : b;}
  template<typename T> static const T abs(const T& v) {return v < 0 ? -v : v;} // TODO: fast abs for double/float

  static double floor(double v); // TODO: inline this
  static float floor(float v); // TODO: inline this
  static double ceil(double v); // TODO: inline this
  static float ceil(float v); // TODO: inline this
  static double exp(double v); // TODO: inline this
  static float exp(float v); // TODO: inline this

  static uint random(); // TODO: inline this
  static uint random(uint seed);

  static const usize randomMax;
};
