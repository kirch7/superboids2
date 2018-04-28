// Copyright (C) 2016-2018 Cássio Kirch.
// License specified in LICENSE file.

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
