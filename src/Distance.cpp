// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#include "Distance.hpp"

#include <cmath>
#include <stdexcept>

#include "Miniboid.hpp"
#include "Superboid.hpp"
#include "parameters.hpp"

Distance::Distance(const std::valarray<real> &va)
    : miniboid1(nullptr), miniboid2(nullptr) {
  if (va.size() != 2)
    throw std::length_error("Only 2D supported.");

  this->module = -0.0;
  for (auto &comp : va)
    this->module += square(comp);
  this->module = std::sqrt(this->module);
  this->cosine = va[X] / this->module;
  this->sine   = va[Y] / this->module;
}

Distance::Distance(const Miniboid &m1, const Miniboid &m2)
    : miniboid1(&m1)
    , miniboid2(&m2)
    , module(-0.0f)
    , sine(-0.0f)
    , cosine(-0.0f) {
  const Distance d = Distance(m1.position, m2.position);
  this->module     = d.module;
  this->sine       = d.sine;
  this->cosine     = d.cosine;

  return;
}

Distance::Distance(const std::valarray<real> &position1,
                   const std::valarray<real> &position2)
    : miniboid1(nullptr)
    , miniboid2(nullptr)
    , module(-0.0f)
    , sine(-0.0f)
    , cosine(-0.0f) {
  std::valarray<real> delta(position2 - position1);
  // if (std::fabs(position1[X]) < 1.0e-6f || std::fabs(position2[X]) < 1.0e-6f
  // || std::fabs(position1[Y]) < 1.0e-6f || std::fabs(position2[Y]) < 1.0e-6f)
  //   std::cerr << "Distance::Distance: delta = " << delta << std::endl;

  const real HALF_RANGE = parameters().RANGE / 2.0f;
  real sumOfSquares     = -0.0f;
  for (real &component : delta) {
    if (std::fabs(component) >= HALF_RANGE)
      component -= sign(component) * parameters().RANGE;
    sumOfSquares += square(component);
  }

  this->module = std::sqrt(sumOfSquares);
  if (this->module > 1.0e-6) {
    this->cosine = delta[X] / this->module;
    this->sine   = delta[Y] / this->module;
  }

  return;
}

std::valarray<real>
    Distance::getDirectionArray(void) const {
  std::valarray<real> va(parameters().DIMENSIONS);
  va[X] = this->cosine;
  va[Y] = this->sine;
  return va;
}

real
    Distance::getAngle(void) const {
  real angle = std::atan2(this->sine, this->cosine);

  if (angle < -PI)
    angle += TWO_PI;

  return angle;
}

std::valarray<real>
    Distance::getTangentArray(void) const {
  real newAngle         = this->getAngle() + HALF_PI;
  std::valarray<real> a = {std::cos(newAngle), std::sin(newAngle)};
  return a;
}

real
    assertAngle(real anglie) {
  while (anglie < -PI)
    anglie += TWO_PI;
  while (anglie > PI)
    anglie -= TWO_PI;
  return anglie;
}

real
    angleBetween(const real phi1, const real phi2) {
  const real difference = phi2 - phi1;
  const real absolute   = std::fabs(difference);

  if (absolute <= PI)
    return assertAngle(difference);
  else
    return assertAngle(sign(difference) * (absolute - TWO_PI));
};
