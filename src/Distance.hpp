// Copyright (C) 2016-2018 Cássio Kirch.
// License specified in LICENSE file.

#pragma once

#include <iostream>
#include <valarray>
#include "parameters.hpp"

class Miniboid;

class Distance
{
public:
  const Miniboid* miniboid1;
  const Miniboid* miniboid2;
  real module;
  real sine;
  real cosine;
  
  Distance(const Miniboid& miniA, const Miniboid& miniB);
  inline Distance(void): miniboid1(nullptr), miniboid2(nullptr), module(0), sine(0), cosine(0) {return;}
  inline Distance(const Distance& d): miniboid1(d.miniboid1), miniboid2(d.miniboid2), 
                                      module(d.module), sine(d.sine), cosine(d.cosine) {return;}
  Distance(const std::valarray<real>&);
  Distance(const std::valarray<real>&, const std::valarray<real>&D);
  //inline bool operator()() const { return this->module != 0.0f; }
  std::valarray<real> getDirectionArray(void) const;
  real getAngle(void) const;
  std::valarray<real> getTangentArray(void) const;
};

inline std::ostream& operator<<(std::ostream& os, const Distance& d)
{
  os << d.module << '\t' << d.cosine << '\t' << d.sine;
  return os;
}


inline Distance
operator-(const Distance& d)
{
  Distance dist = d;
  dist.sine = -dist.sine;
  dist.cosine = -dist.cosine;
  return dist;
}

inline Distance
operator+(const Distance& d1, const Distance& d2)
{
  Distance r;
  r.module = std::sqrt(square(d1.module * d1.cosine) + square(d1.module * d1.sine) +
                       square(d2.module * d2.cosine) + square(d2.module * d2.sine) +
                       d1.module * d2.module * (d1.cosine * d2.cosine + d1.sine * d2.sine));
  r.cosine = (d1.module * d1.cosine + d2.module * d2.cosine) / r.module;
  r.sine = (d1.module * d1.sine + d2.module * d2.sine) / r.module;
  return r;
}

inline Distance
operator-(const Distance& d1, const Distance& d2)
{
  return d1 + -d2;
}

inline Distance
operator*(const real r, const Distance& d)
{
  Distance dist = d;
  dist.module *= r;
  if (r < 0.0)
  {
    dist.module *= -1;
    dist.cosine *= -1;
    dist.sine *= -1;
  }
  return dist;
}

inline Distance
operator*(const Distance& d, const real r)
{
  return r*d;
}

inline bool
operator<(const Distance& d1, const Distance& d2)
{
  return d1.module < d2.module;
}

inline bool
operator>(const Distance& d1, const Distance& d2)
{
  return d1.module > d2.module;
}

inline bool
operator<(const Distance& d1, const real& d2)
{
  return d1.module < d2;
}

inline bool
operator>(const Distance& d1, const real& d2)
{
  return d1.module > d2;
}


extern real angleBetween (const real phi1, const real phi2);
extern real assertAngle  (real anglie);

template<typename Array>
inline real
getModule(const Array& va1, const Array& va2)
{
  real _module = -0.0;
  const size_t size = va1.size() < va2.size() ? va1.size() : va2.size();
  for (uint16_t count = 0u; count < size; ++count)
    _module += square(va1[count] - va2[count]);
  _module = std::sqrt(_module);
  return _module;
}

