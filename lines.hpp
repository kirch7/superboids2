#include <valarray>
#include "parameters.hpp"

extern real getAngularCoefficient(const std::valarray<real> pointA, const std::valarray<real> pointB);
extern std::valarray<real> getClosestPoint(const real m, const std::valarray<real> pointA, const std::valarray<real> pointC);
