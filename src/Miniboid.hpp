// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#pragma once
#include "CellNeighbors.hpp"
#include "Distance.hpp"
#include "Neighbor.hpp"
#include "TwistNeighbor.hpp"
#include "parameters.hpp"
#include <iostream>
#include <list>
#include <map>
#include <tuple>
#include <valarray>

class Superboid;
bool operator==(const Superboid &s1, const Superboid &s2);
class Box;

class Miniboid {
public:
  const bool isVirtual;
  const mini_int ID;
  Superboid &superboid;
  std::valarray<real> position;
  std::valarray<real> velocity;
  std::valarray<real> newVelocity;
  Distance radialDistance;
  real radialAngle;

  //  real angleCM;
  box_int boxID() const; // ID of box where *this is.
  bool inEdge() const;   // True if box where *this is is in edge.

  inline Miniboid(const mini_int _id, Superboid &super, const bool);
  void checkLimits(const step_int);
  void checkLimits(void);
  void setNextVelocity(const step_int);
  void setNextPosition(const step_int);
  void checkFatOut(void);
  void checkBackInTime(const step_int);
  inline void setBox(Box *const b) { _box = b; }
  inline Box &getBox(void) const { return *(this->_box); }
  inline Box *getBoxPtr(void) const { return this->_box; }
  void reset(void);
  real getAreaBetween(const Miniboid &) const;
  void setNeighbors(const step_int);
  std::list<TwistNeighbor> _twistNeighbors;
  bool isInSomeNthTriangle(const mini_int nth, const Superboid &super);
  bool fatInteractions(const step_int, const std::list<Neighbor> &,
                       const bool interact);
  std::map<super_int, std::list<Neighbor>>
      _neighbors; // From different superboid.
  friend void exportPositions(const std::vector<Superboid> &, const step_int);

protected:
  std::list<std::tuple<step_int, std::vector<const Miniboid *>>> history;
  std::valarray<real> _oldPosition;
  step_int _lastInvasionStep;
  std::valarray<real> _noiseSum;    // Related to ETA.
  std::valarray<real> _velocitySum; // Related to ALPHA;
  std::valarray<real> _forceSum;    // Related to BETA.
  Box *_box;
  std::valarray<mini_int> _neighborsPerTypeNos;
  void noise(void);
  void checkNeighbor(Miniboid &neighbor);
  static std::valarray<real> getAngles(const mini_int id);
  inline Miniboid(void); /* Declared but intentionally not defined. */
  void setNewVelocity(void);
  void interInteractions(const step_int);
  void interInteractions(const Neighbor &);
  void checkPeriodicLimits(void);
  void checkRectangularLimits(void);
  void checkStokesLimits(void);
  void checkKillCondition(const step_int);
  real getHarrisParameter(const std::vector<std::vector<real>> &,
                          const std::vector<real> &medium) const;
};

inline Miniboid::Miniboid(const mini_int _id, Superboid &super,
                          const bool isVirt = false)
    : isVirtual(isVirt), ID(_id), superboid(super),
      position(parameters().DIMENSIONS), velocity(parameters().DIMENSIONS),
      newVelocity(parameters().DIMENSIONS), radialDistance(Distance()),
      _oldPosition(parameters().DIMENSIONS), _lastInvasionStep(0),
      _noiseSum(parameters().DIMENSIONS), _velocitySum(parameters().DIMENSIONS),
      _forceSum(parameters().DIMENSIONS), _box(nullptr),
      _neighborsPerTypeNos(parameters().TYPES_NO) {
  if (!this->isVirtual) {
    this->setNewVelocity();
    if (_id != 0u) {
      if (parameters().MINIBOIDS_PER_SUPERBOID > 3u ||
          _id != parameters().MINIBOIDS_PER_SUPERBOID - 1u)
        this->_twistNeighbors.push_front(
            TwistNeighbor(_id, getTangentNeighborID(_id, 1)));
      if (parameters().MINIBOIDS_PER_SUPERBOID > 3u || _id != 1u)
        this->_twistNeighbors.push_front(
            TwistNeighbor(_id, getTangentNeighborID(_id, -1)));
    }
  }

  return;
}

extern std::ostream &operator<<(std::ostream &os, const Miniboid &mini);

inline bool operator==(const Miniboid &m1, const Miniboid &m2) {
  return (m1.ID == m2.ID && m1.isVirtual == m2.isVirtual &&
          m1.superboid == m2.superboid);
}

inline bool operator!=(const Miniboid &m1, const Miniboid &m2) {
  return !(m1 == m2);
}

bool isPointInTriangle(const std::valarray<real> &p_test,
                       const std::valarray<real> &p0,
                       const std::valarray<real> &p1,
                       const std::valarray<real> &p2);
bool isPointInSomeNthTriangle(const mini_int nth,
                              const std::valarray<real> &point,
                              const Superboid &super);
