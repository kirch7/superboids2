// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

#include <cmath>
#include <vector>
#include "TwistNeighbor.hpp"

static std::vector<real>
getAngles(const mini_int myID, const mini_int neighborID)
{
  std::vector<real> angles;
  
  static const mini_int M = static_cast<mini_int>(std::ceil((parameters().MINIBOIDS_PER_SUPERBOID - 1u)/2u));

  if (parameters().MINIBOIDS_PER_SUPERBOID > 3u)
  {
    if (myID != 0u)
    {
      std::vector<mini_int> ms(2u, myID); // vector of two components filled with myID value.
      std::vector<std::vector<int>> orientationMatrix;
      {
        std::vector<int> line;
        line.push_back(0);  // position
        line.push_back(-1); // orientation
        orientationMatrix.push_back(line);
      }
      {
        std::vector<int> line;
        line.push_back(1);  // position
        line.push_back(1);  // orientation
        orientationMatrix.push_back(line);
      }
      
      bool keep = true;
      for (mini_int m = 1u; m <= M && keep; ++m)
      {
        for (const auto& line : orientationMatrix)
        {
          ms[line[0u]] = getTangentNeighborID(ms[line[0u]], line[1u]);
          if (neighborID == ms[line[0u]])
          {
            ////angles.push_back(real(m * TWO_PI / (MINIBOIDS_PER_SUPERBOID - 1u)));
            angles.push_back(line[1] * m * parameters().TWIST_EQ_ANGLE);
            keep = false;
            break;
          }
        }
      }
    }
  }
  else
  {
    if (myID == 1u && neighborID == 2u)
      angles.push_back(parameters().TWIST_EQ_ANGLE);
    else if (myID == 2u && neighborID == 1u)
      angles.push_back(-parameters().TWIST_EQ_ANGLE);
  }
  
  //std::cout << angles[0u] << std::endl;
  return angles;
}



TwistNeighbor::TwistNeighbor (const mini_int myID, const mini_int neighborID):
  ID(neighborID),
  ANGLES(getAngles(myID, neighborID)),
  _distance(Distance())
{;
  return;
}

mini_int
getTangentNeighborID(const mini_int MY_ID, const int SIDE)
{
  if (SIDE == 0u)
    return 0u;
  else if (SIDE > 0)
  {
    if (MY_ID < parameters().MINIBOIDS_PER_SUPERBOID - 1u)
      return (MY_ID + 1u);
    else
      return 1u;
  }
  else
  {
    if (MY_ID > 1u)
      return (MY_ID - 1u);
    else
      return (parameters().MINIBOIDS_PER_SUPERBOID - 1u);
  }
}

