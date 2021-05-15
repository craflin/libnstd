
#include <cstdlib>
#include <cmath>

#include <nstd/Math.hpp>

uint Math::random() {return rand();}
uint Math::random(uint seed) {return srand(seed), rand();}
const usize Math::randomMax = RAND_MAX;
double Math::floor(double v) {return std::floor(v);}
float Math::floor(float v) {return std::floor(v);}
double Math::ceil(double v) {return std::ceil(v);}
float Math::ceil(float v) {return std::ceil(v);}
double Math::exp(double v) {return std::exp(v);}
float Math::exp(float v) {return std::exp(v);}

