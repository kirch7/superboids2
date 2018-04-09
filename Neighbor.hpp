// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

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
