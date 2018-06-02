// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#include "Miniboid.hpp"
#include "Neighbor.hpp"

Neighbor::Neighbor(const Miniboid& m, const Distance& d):
  miniNeighbor(m),
  distance(d)
{ return; }

Neighbor::Neighbor(const Distance& d, const Miniboid& m):
  miniNeighbor(m),
  distance(d)
{ return; }
