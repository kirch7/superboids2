// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

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
