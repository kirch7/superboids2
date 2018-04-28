// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

#include <algorithm>
#include "Stokes.hpp"
#include "Box.hpp"
#include "Distance.hpp"

static std::vector<box_int>
getBoxIDs(const std::valarray<real>& center, const real radius)
{
  std::vector<box_int> boxes;

  real tmpRadius = radius;
  const real delta = parameters().RANGE / parameters().BOXES_IN_EDGE;
  while (tmpRadius > 0.0f)
  {
    for (unsigned i = 0u; i < 3141u; ++i)
    {
      std::valarray<real> point = center;
      const real angle = i * TWO_PI / 3141.0f;
      point[X] += radius * std::cos(angle);
      point[Y] += radius * std::sin(angle);
      boxes.emplace_back(Box::getBoxID(point));
    }
    tmpRadius -= delta;
  }

  std::sort(boxes.begin(), boxes.end());
  boxes.resize(std::distance(boxes.begin(), std::unique(boxes.begin(), boxes.end())));
  
  return boxes;
}


Stokes::Stokes(std::valarray<real> _center, real _radius):
  center(_center),
  radius(_radius),
  boxIDs(getBoxIDs(_center, _radius))
{
  return;
}

bool
Stokes::isInside(const std::valarray<real>& position) const
{
  const real TOLERABLE = this->radius + parameters().REAL_TOLERANCE;
  const Distance d(this->center, position);
  return d.module < TOLERABLE;
}
