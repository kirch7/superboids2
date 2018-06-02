// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#include <cmath>
#include "lines.hpp"

real
getAngularCoefficient(const std::valarray<real> pointA, const std::valarray<real> pointB)
{
  const real deltaX = pointB[X] - pointA[X];
  const real deltaY = pointB[Y] - pointA[Y];

  if (std::abs(deltaX) <= parameters().REAL_TOLERANCE)
    return sign(deltaX) * sign(deltaY) * INFINITY;
  else
    return deltaY / deltaX;
}

std::valarray<real>
getClosestPoint(const real m, const std::valarray<real> pointA, const std::valarray<real> pointC)
{
  const real x1 = pointA[X];
  const real x3 = pointC[X];
  const real y1 = pointA[Y];
  const real y3 = pointC[Y];

  if (std::abs(m) <= parameters().REAL_TOLERANCE)
    return std::valarray<real>({x3, y1});
  else if (!std::isnormal(m))
    return std::valarray<real>({x1, y3});
  else
  {
    const real div = m + 1.0f / m;
    const real commonX = ((x3 / m) + y3 + (m * x1) - y1) / div;
    const real commonY = m * (commonX - x1) + y1;
    return std::valarray<real>({commonX, commonY});
  }
}
