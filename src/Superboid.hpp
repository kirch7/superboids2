// Copyright (C) 2016-2018 Cássio Kirch.
// License specified in LICENSE file.

#pragma once
#include <iostream> // operator<< .
#include <vector>
#include <valarray>
#include <random>
#include <sstream>
#include "Miniboid.hpp"
#include "CellNeighbors.hpp"

class Superboid
{
public:
  bool activated;
  std::vector<Miniboid> virtualMiniboids;
  std::vector<Miniboid> miniboids;
  const super_int ID;
  const type_int type;
  void setGamma(std::vector<Superboid>&);
  real gamma;
  bool doUseGamma;

  void setShape(const step_int);
  real area;
  real perimeter;
  real meanRadius;
  real meanRadius2;
  
  inline void reset(void)
  {
    for (auto& mini : this->miniboids)
    {
      mini.reset();
      ////mini._tangentNeighbors.clear();
    }
    this->cellNeighbors = CellNeighbors();
    this->infiniteVectors.clear();
    this->infinite2Vectors.clear();
  }
  Superboid(void);
  real get0to2piRandom(void);
  void checkVirtual(const bool export_);
  void setNextPosition(const step_int);
  bool divide(const super_int, Superboid&, std::vector<Box>&, const step_int);
  Distance getBiggestAxis() const;
  
  CellNeighbors cellNeighbors;
  void checkWrongNeighbors(const std::vector<Superboid>&);
  
  std::vector<std::valarray<real>> infiniteVectors;
  std::vector<std::valarray<real>> infinite2Vectors;
  std::ostringstream virtualsInfo;
  step_int getLastDivisionStep(void) const { return this-> lastDivisionStep; }
  
protected:
  static super_int _totalSuperboids;
  std::default_random_engine _randomEngine;
  step_int _shapeStep;
  step_int lastDivisionStep;
  inline Superboid(Superboid&);
};

extern std::ostream& operator<< (std::ostream& os, const Superboid& super);
inline bool operator!=   (const Superboid& s1, const Superboid& s2)
{
  return (s1.ID != s2.ID);
}
inline bool operator==   (const Superboid& s1, const Superboid& s2)
{
  return (s1.ID == s2.ID);
}
