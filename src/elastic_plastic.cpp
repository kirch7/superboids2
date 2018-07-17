// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#include "elastic_plastic.hpp"

std::valarray<real> getFiniteForce(Distance dist, const real beta,
                                   const real rEq,
                                   const std::vector<real> &transitions) {
  real &module = dist.module;
  std::valarray<real> force(0.0f, parameters().DIMENSIONS);

  if (transitions.size() == 0 || module <= transitions[0u] + 1.0e-6f) {
    const real scalar = (1.0f - module / rEq) * beta;
    if (std::isfinite(scalar)) {
      force = dist.getDirectionArray();
      force *= scalar;
      return force;
    }
  } else
    for (std::size_t index = 1u; index < transitions.size(); ++index) {
      if (index % 2 == 1)  // odd index
      {
        if (module <= transitions[index]) {
          module = transitions[index - 1];
          force = getFiniteForce(dist, beta, rEq, transitions);
          return force;
        }
      } else  // even index
      {
        if (module <= transitions[index] + 1.0e-6f) {
          const real d = transitions[index - 1] - transitions[index];
          const real dif = 1.0f - (module - d) / rEq;
          if (std::isfinite(dif)) {
            force = dist.getDirectionArray();
            force *= dif;
          }
          return force;
        }
      }
    }

  dist.module = transitions.back();

  return getFiniteForce(dist, beta, rEq, transitions);
}

std::valarray<real> getFiniteForce(const Distance &d, const real beta,
                                   const real rEq) {
  Distance dist(d);
  const real &module = dist.module;
  std::valarray<real> force(0.0f, parameters().DIMENSIONS);

  if (module <= (parameters().INTER_ELASTIC_UP_LIMIT +
                 1.0e-6))  // Elastic regime or called by recursion.
  {
    const real scalar = (1.0f - module / rEq) * beta;
    if (std::isfinite(scalar)) {
      force = dist.getDirectionArray();
      force *= scalar;
    }
    return force;
  } else  // Constant regime; call recursion.
  {
    dist.module = parameters().INTER_ELASTIC_UP_LIMIT;
    return getFiniteForce(dist, beta, rEq);
  }
}
