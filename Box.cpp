// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

#include <valarray>
#include <vector>
#include "Miniboid.hpp"
#include "Superboid.hpp"
#include "Box.hpp"

namespace boxID
{
  static box_int
  south (const box_int centralBoxID)
  {
    if (centralBoxID >= BOXES_IN_EDGE)
      return (centralBoxID - BOXES_IN_EDGE);
    else
      return (BOXES - BOXES_IN_EDGE + centralBoxID);
  }
  
  static box_int
  north (const box_int centralBoxID)
  {
    if (centralBoxID < BOXES - BOXES_IN_EDGE)
      return (centralBoxID + BOXES_IN_EDGE);
    else
      return (centralBoxID + BOXES_IN_EDGE - BOXES);
  }
  
  static box_int
  west (const box_int centralBoxID)
  {
    if (centralBoxID % BOXES_IN_EDGE != 0)
      return (centralBoxID - 1);
    else
      return (centralBoxID + BOXES_IN_EDGE - 1);
  }
  
  static box_int
  east (const box_int centralBoxID)
  {
    if (centralBoxID % BOXES_IN_EDGE != BOXES_IN_EDGE - 1u)
      return (centralBoxID + 1);
    else
      return (centralBoxID - BOXES_IN_EDGE + 1u);
  }
  
  static box_int
  northeast (const box_int centralBoxID)
  {
    return (north(east(centralBoxID)));
  }
  
  static box_int
  northwest (const box_int centralBoxID)
  {
    return (north(west(centralBoxID)));
  }
  
  static box_int
  southeast (const box_int centralBoxID)
  {
    return (south(east(centralBoxID)));
  }
  
  static box_int
  southwest (const box_int centralBoxID)
  {
    return (south(west(centralBoxID)));
  }
} /* End of boxID namespace. */

box_int Box::_totalBoxesCount(0u);

/******************
 *  Box methods:  *
 ******************/

/* Box unique constructor: */
Box::Box(void):
  ID(_totalBoxesCount++),
  inEdge(Box::getIsInEdge(ID)), /* Construct boolean. */
  neighbors (9u, nullptr), /* Construct a vector with 9 null pointer elements. */
  miniboids()  /* Construct superboids as a empty list. */
{
  #ifdef DEBUG
  if (_totalBoxesCount > BOXES)
  {
    std::cerr << "There must be only " << BOXES << " boxes." << std::endl;
    exit(30);
  }
  #endif
  
  return;
}

box_int
Box::getBoxID (std::valarray<real> position) /* Parameter by copy. */
{
  static const real BOX_SIZE_INVERSE = static_cast<real>(BOXES_IN_EDGE)/RANGE; /* 1/BOX_SIZE */
  
  position += 0.5f * RANGE;     /* Translate */
  position *= BOX_SIZE_INVERSE; /* Normalize */

  std::vector<box_int> truncated;
  for (dimension_int dimension = 0u; dimension < DIMENSIONS; ++dimension)
    truncated.push_back(static_cast<box_int>(position[dimension]));
  
  box_int tmpBoxID(0u);
  for (dimension_int dim = 0u; dim < DIMENSIONS; ++dim)
  {
    box_int sum = truncated[dim];
    for (dimension_int power = 0u; power < dim; ++power)
      sum *= BOXES_IN_EDGE;
    tmpBoxID += sum;
  }

  for (dimension_int dim = 0u; dim < DIMENSIONS; ++dim)
    if (position[dim] == BOXES_IN_EDGE)
    {
      box_int sum = 1;
      for (dimension_int power = 0u; power < dim; ++power)
        sum *= BOXES_IN_EDGE;
      tmpBoxID -= sum;
    }

  return tmpBoxID;
}

bool Box::getIsInEdge(const box_int boxID)
{
  if      (boxID < BOXES_IN_EDGE)
    return true;
  
  else if (boxID >= BOXES - BOXES_IN_EDGE)
    return true;
  
  else if (boxID % BOXES_IN_EDGE == 0)
    return true;
  
  else if (boxID % BOXES_IN_EDGE == BOXES_IN_EDGE - 1)
    return true;
  
  else
    return false;
}

void Box::setNeighbors(std::vector<Box>& boxes)
{
  using namespace boxID;
  
  neighbors[static_cast<uint16_t>(CardinalPoint::ACTUAL)] = &boxes[ID];
  
  neighbors[static_cast<uint16_t>(CardinalPoint::NORTH)] = &boxes[north(ID)];
  neighbors[static_cast<uint16_t>(CardinalPoint::SOUTH)] = &boxes[south(ID)];
  neighbors[static_cast<uint16_t>(CardinalPoint::EAST)]  = &boxes[east(ID)];
  neighbors[static_cast<uint16_t>(CardinalPoint::WEST)]  = &boxes[west(ID)];

  neighbors[static_cast<uint16_t>(CardinalPoint::NORTHWEST)] = &boxes[northwest(ID)];
  neighbors[static_cast<uint16_t>(CardinalPoint::NORTHEAST)] = &boxes[northeast(ID)];
  neighbors[static_cast<uint16_t>(CardinalPoint::SOUTHWEST)] = &boxes[southwest(ID)];
  neighbors[static_cast<uint16_t>(CardinalPoint::SOUTHEAST)] = &boxes[southeast(ID)];

  return;
}

void
Box::append (Miniboid& mini)
{
  if (mini.superboid.activated == true)
  {
    this->miniboids.push_front(&mini);
    mini.setBox(this);
  }
  
  return;
}
