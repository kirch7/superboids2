// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#include <valarray>
#include "parameters.hpp"

extern real getAngularCoefficient(const std::valarray<real> pointA, const std::valarray<real> pointB);
extern std::valarray<real> getClosestPoint(const real m, const std::valarray<real> pointA, const std::valarray<real> pointC);
