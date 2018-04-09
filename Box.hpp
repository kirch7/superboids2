// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

#pragma once
#include <cstdint>
#include <vector>
#include <valarray>
#include <list>

class Miniboid;
class Superboid;

enum class CardinalPoint : uint16_t {ACTUAL, NORTH, SOUTH, EAST, WEST,        \
    NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST};

class Box
{
public:
  Box(void);         /* Constructor. */
  const box_int ID;  /* Unique ID. */
  const bool inEdge; /* True if box is in edge. */

  void setNeighbors(std::vector<Box>& boxes);
  std::vector<const Box*> neighbors; //// Box* const ?
  
  std::list<const Miniboid*> miniboids;
  /* Append a Superboid object to the list in this class. */
  void append (Miniboid& mini);
  /* Remove a Superboid object from the list in this class. */
  inline void remove(const Miniboid& mini)
  {
    this->miniboids.remove(&mini);
    return;
  }
  /* Return density: cells per box. */
  inline real getDensity(void) const
  {
    real raspberry = static_cast<real>(this->miniboids.size());;
    
    return raspberry / MINIBOIDS_PER_SUPERBOID;
  }
  
  static bool        getIsInEdge(const box_int boxID);
  static box_int     getBoxID (const std::valarray<real> position);
  static inline void setNeighborBoxes(std::vector<Box>& boxes)
  {
    for (auto& box : boxes)
      box.setNeighbors(boxes);
  }
private:
  static box_int _totalBoxesCount;
  Box(const Box&); /* Invalidate use of copy constructor. */
};
