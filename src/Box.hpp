// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#pragma once
#include "parameters.hpp"
#include <cstdint>
#include <list>
#include <valarray>
#include <vector>

class Miniboid;
class Superboid;

enum class CardinalPoint : uint16_t {
  ACTUAL,
  NORTH,
  SOUTH,
  EAST,
  WEST,
  NORTHEAST,
  NORTHWEST,
  SOUTHEAST,
  SOUTHWEST
};

class Box {
public:
  Box(void);         /* Constructor. */
  const box_int ID;  /* Unique ID. */
  const bool inEdge; /* True if box is in edge. */

  void setNeighbors(std::vector<Box> &boxes);
  std::vector<const Box *> neighbors;

  std::list<const Miniboid *> miniboids;
  /* Append a Superboid object to the list in this class. */
  void append(Miniboid &mini);
  /* Remove a Superboid object from the list in this class. */
  inline void remove(const Miniboid &mini) {
    this->miniboids.remove(&mini);
    return;
  }
  /* Return density: cells per box. */
  inline real getDensity(void) const {
    real raspberry = static_cast<real>(this->miniboids.size());
    ;

    return raspberry / parameters().MINIBOIDS_PER_SUPERBOID;
  }

  static bool getIsInEdge(const box_int boxID);
  static box_int getBoxID(const std::valarray<real> position);
  static inline void setNeighborBoxes(std::vector<Box> &boxes) {
    for (auto &box : boxes)
      box.setNeighbors(boxes);
  }

private:
  static box_int _totalBoxesCount;
  Box(const Box &) = delete; /* Invalidate use of copy constructor. */
};
