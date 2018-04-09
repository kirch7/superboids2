// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

#pragma once
#include <iostream>
#include <vector>
#include "Distance.hpp"
#include "parameters.hpp"

class TwistNeighbor
{
public:
  const mini_int ID;
  const std::vector<real> ANGLES;
  TwistNeighbor(const mini_int myID, const mini_int neighborID);
  Distance _distance;
};    

inline std::ostream& operator<<(std::ostream& os, const TwistNeighbor& tn)
{
  if (tn.ANGLES.size() > 0)
    os << "ID: " << tn.ID << '\t' << tn.ANGLES.front();
  return os;
}

extern mini_int getTangentNeighborID(const mini_int MY_ID, const int SIDE);
