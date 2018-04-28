// Copyright (C) 2016-2018 Cássio Kirch.
// License specified in LICENSE file.

#pragma once
#include <valarray>
#include "parameters.hpp"

class Stokes {
public:
  Stokes(std::valarray<real> _center, real _radius);
  const std::valarray<real> center;
  const real radius;
  const std::vector<box_int> boxIDs;
  bool isInside(const std::valarray<real>& position) const;
};
