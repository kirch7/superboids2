// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

#include <cmath>
#include <stdexcept>
#include "parameters.hpp"
#include "Miniboid.hpp"
#include "Superboid.hpp"
#include "Distance.hpp"

Distance::Distance(const std::valarray<real>& va):
  miniboid1(nullptr),
  miniboid2(nullptr)
{
  if (va.size() != 2)
    throw std::length_error("Only 2D supported.");

  this->module = -0.0;
  for (auto& comp : va)
    this->module += square(comp);
  this->module = std::sqrt(this->module);
  this->cosine = va[X] / this->module;
  this->sine   = va[Y] / this->module;
}

Distance::Distance(const Miniboid& m1, const Miniboid& m2):
  miniboid1(&m1),
  miniboid2(&m2),
  module(0.0f),
  sine(0.0f),
  cosine(0.0f)
{
  const Distance d = Distance(m1.position, m2.position);
  this->module = d.module;
  this->sine   = d.sine;
  this->cosine = d.cosine;

  return;
}

Distance::Distance(const std::valarray<real>& position1, const std::valarray<real>& position2):
  miniboid1(nullptr),
  miniboid2(nullptr),
  module(0.0f),
  sine(0.0f),
  cosine(0.0f)
{
  //std::valarray<real> delta(position2 - position1);
  std::valarray<real> delta(2);
  delta[X] = position2[X] - position1[X];
  delta[Y] = position2[Y] - position1[Y];
  const real HALF_RANGE = RANGE / 2.0f;
  real sumOfSquares = -0.0f;
  for (real& component : delta)
  {
    if (std::fabs(component) >= HALF_RANGE)
      component -= sign(component) * RANGE;
    sumOfSquares += square(component);
  }

  this->module = std::sqrt(sumOfSquares);
  if (this->module > 1.0e-6)
  {
    this->cosine = delta[X] / this->module;
    this->sine   = delta[Y] / this->module;
  }
  
  return;
}

std::valarray<real>
Distance::getDirectionArray(void) const
{
  std::valarray<real> va(DIMENSIONS);
  va[X] = this->cosine;
  va[Y] = this->sine;
  return va;
}
  
real
Distance::getAngle(void) const
{
  real angle = std::atan2(this->sine, this->cosine);

  if (angle < -PI)
    angle += TWO_PI;
  
  return angle;
}

std::valarray<real>
Distance::getTangentArray(void) const
{
  real newAngle = this->getAngle() + HALF_PI;
  std::valarray<real> a = {std::cos(newAngle), std::sin(newAngle)};
  return a;
}

real
assertAngle(real anglie)
{
  while (anglie < -PI)
    anglie += TWO_PI;
  while (anglie > PI)
    anglie -= TWO_PI;
  return anglie;
}

real
angleBetween (const real phi1, const real phi2)
{
  const real difference = phi2 - phi1;
  const real absolute = std::fabs(difference);
  
  if (absolute <= PI)
    return assertAngle(difference);
  else
    return assertAngle(sign(difference) * (absolute - TWO_PI));
};
