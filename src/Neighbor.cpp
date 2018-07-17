// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#include "Neighbor.hpp"
#include "Miniboid.hpp"

Neighbor::Neighbor(const Miniboid &m, const Distance &d)
    : miniNeighbor(m), distance(d) {
  return;
}

Neighbor::Neighbor(const Distance &d, const Miniboid &m)
    : miniNeighbor(m), distance(d) {
  return;
}

bool operator<(const Neighbor &n1, const Neighbor &n2) {
  return n1.distance.module < n2.distance.module;
}
