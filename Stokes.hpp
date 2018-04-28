// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

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
