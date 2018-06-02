// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#pragma once
#include "Distance.hpp"
class Miniboid;

class Neighbor
{
public:
  const Miniboid& miniNeighbor;
  const Distance  distance;
  Neighbor(const Miniboid&, const Distance&);
  Neighbor(const Distance&, const Miniboid&);
};
