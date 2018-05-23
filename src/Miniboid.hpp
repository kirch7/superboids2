// Copyright (C) 2016-2018 Cássio Kirch.
// License specified in LICENSE file.

#pragma once
#include <iostream>
#include <valarray>
#include <list>
#include <tuple>
#include "Neighbor.hpp"
#include "TwistNeighbor.hpp"
#include "Distance.hpp"
#include "CellNeighbors.hpp"
#include "parameters.hpp"

class Superboid;
bool operator==   (const Superboid& s1, const Superboid& s2);
class Box;

class Miniboid
{
public:
  const bool isVirtual;
  const mini_int ID;
  Superboid& superboid;
  std::valarray<real> position;
  std::valarray<real> velocity;
  std::valarray<real> newVelocity;
  Distance radialDistance;
  real radialAngle;

  //  real angleCM;
  box_int boxID() const;   // ID of box where *this is.
  bool    inEdge() const;  // True if box where *this is is in edge.

  inline Miniboid(const mini_int _id, Superboid& super, const bool);
  void checkLimits(void);
  void setNextVelocity(const step_int);
  void setNextPosition(void);
  inline void setBox(Box* const b) { _box = b; }
  inline Box& getBox()  const {return *(this->_box);}
  void reset(void);
  real getAreaBetween(const Miniboid&) const;
  friend void exportPositions(const std::vector<Superboid>&, const step_int);
  void setNeighbors(void);
  std::list<TwistNeighbor> _twistNeighbors;
  //std::list<const Miniboid*> _tangentNeighbors;
  //const Miniboid* leftN;
  //const Miniboid* rightN;
  std::list<std::tuple<step_int, std::vector<const Miniboid*>>> history;
  bool isInSomeTriangle(const Superboid& super);
  bool fatInteractions(const step_int, const std::list<Neighbor>&, const bool interact);
  std::list<std::list<Neighbor> > _neighbors; // From different superboid.
protected:
  std::valarray<real> _noiseSum;    // Related to ETA.
  std::valarray<real> _velocitySum; // Related to ALPHA;
  std::valarray<real> _forceSum;    // Related to BETA.
  Box* _box;
  std::valarray<mini_int> _neighborsPerTypeNos;
  void noise(void);
  void checkNeighbor(Miniboid& neighbor);
  static std::valarray<real> getAngles(const mini_int id);
  inline Miniboid(void); /* Declared but intentionally not defined. */
  void setNewVelocity(void);
  void interInteractions(const step_int);
  void checkPeriodicLimits();
  void checkRectangularLimits();
  void checkStokesLimits();
  void checkKillCondition();
  real getHarrisParameter(const std::vector<std::vector<real>>&, const std::vector<real>& medium) const;
};

inline Miniboid::Miniboid(const mini_int _id, Superboid& super, const bool isVirt = false):
  isVirtual(isVirt),
  ID(_id),
  superboid(super),
  position(parameters().DIMENSIONS),
  velocity(parameters().DIMENSIONS),
  newVelocity(parameters().DIMENSIONS),
  radialDistance(Distance()),
  ////equilibriumAngles(getAngles(_id)),
  _noiseSum(parameters().DIMENSIONS),
  _velocitySum(parameters().DIMENSIONS),
  _forceSum(parameters().DIMENSIONS),
  _box(nullptr),
  _neighborsPerTypeNos(parameters().TYPES_NO)
{
  if (!this->isVirtual)
  {
    this->setNewVelocity();
    if (_id != 0u)
    {
      if (parameters().MINIBOIDS_PER_SUPERBOID > 3u || _id != parameters().MINIBOIDS_PER_SUPERBOID - 1u)
	this->_twistNeighbors.push_front(TwistNeighbor(_id, getTangentNeighborID(_id,  1)));
      if (parameters().MINIBOIDS_PER_SUPERBOID > 3u || _id != 1u)
	this->_twistNeighbors.push_front(TwistNeighbor(_id, getTangentNeighborID(_id, -1)));
    }
  }
  
  return;
}

extern std::ostream& operator<<(std::ostream& os, const Miniboid& mini);

inline bool operator==(const Miniboid& m1, const Miniboid& m2)
{
  return (m1.ID == m2.ID && m1.isVirtual == m2.isVirtual && m1.superboid  == m2.superboid);
}

inline bool operator!=(const Miniboid& m1, const Miniboid& m2)
{
  return !(m1 == m2);
}

bool isPointInTriangle(const std::valarray<real>& p_test, const std::valarray<real>& p0, const std::valarray<real>& p1, const std::valarray<real>& p2);
bool isPointInSomeTriangle(const std::valarray<real>& point, const Superboid& super);
