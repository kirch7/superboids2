// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#pragma once
#include <iostream>  // operator<< .
#include <random>
#include <sstream>
#include <valarray>
#include <vector>
#include "CellNeighbors.hpp"
#include "Miniboid.hpp"

enum class DeathState { Live, WillDie, Dead };

class Superboid {
 public:
  std::vector<Miniboid> virtualMiniboids;
  void clearVirtualMiniboids(void);

  std::vector<Miniboid> miniboids;
  const super_int ID;
  const type_int type;
  void setGamma(std::vector<Superboid> &);
  real gamma;
  bool doUseGamma;

  void setShape(const step_int);
  real area;
  real perimeter;
  real meanRadius;
  real meanRadius2;

  inline void reset(void) {
    for (auto &mini : this->miniboids) {
      mini.reset();
      ////mini._tangentNeighbors.clear();
    }
    this->cellNeighbors = CellNeighbors();
    this->infiniteVectors.clear();
    this->infinite2Vectors.clear();
    this->_deathMessage = "";
  }
  Superboid(void);
  real get0to2piRandom(void);
  void checkVirtual(const bool export_, const step_int);
  void setNextPosition(const step_int);
  bool divide(const super_int, Superboid &, std::vector<Box> &, const step_int);
  Distance getBiggestAxis() const;

  CellNeighbors cellNeighbors;
  void checkWrongNeighbors(const std::vector<Superboid> &);

  std::vector<std::valarray<real>> infiniteVectors;
  std::vector<std::valarray<real>> infinite2Vectors;
  std::ostringstream virtualsInfo;
  step_int getLastDivisionStep(void) const { return this->_lastDivisionStep; }

  void setDeactivation(const std::string&);
  bool willDie(void) const;
  void deactivate(void);
  bool isActivated(void) const;
  void activate(void);  // Ignoring boxes.
  void checkBackInTime(const step_int);
  real getRadialReq(const step_int) const;
  real getTangentReq(const step_int) const;

 protected:
  static super_int _totalSuperboids;
  std::string _deathMessage;
  DeathState _deathState;
  std::default_random_engine _randomEngine;
  step_int _shapeStep;
  step_int _lastDivisionStep;
  Superboid(Superboid &) = delete;
};

extern std::ostream &operator<<(std::ostream &os, const Superboid &super);

inline bool operator!=(const Superboid &s1, const Superboid &s2) {
  return (s1.ID != s2.ID);
}

inline bool operator==(const Superboid &s1, const Superboid &s2) {
  return (s1.ID == s2.ID);
}
