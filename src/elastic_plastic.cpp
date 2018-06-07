// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#include "elastic_plastic.hpp"

std::valarray<real>
getFiniteForce(Distance dist, const real beta, const real rEq, const std::vector<real>& transitions)
{
  real& module = dist.module;
  std::valarray<real> force(0.0f, parameters().DIMENSIONS);

  if (transitions.size() == 0 || module <= transitions[0u] + 1.0e-6f)
  {
    const real scalar = (1.0f - module / rEq) * beta;
    if (std::isfinite(scalar))
    {
      force = dist.getDirectionArray();
      force *= scalar;
      return force;
    }
  }
  else
    for (std::size_t index = 1u; index < transitions.size(); ++index)
    {
      if (index % 2 == 1) // odd index
      {
	if (module <= transitions[index])
	{
	  module = transitions[index - 1];
	  force = getFiniteForce(dist, beta, rEq, transitions);
	  return force;
	}
      }
      else // even index
      {
	if (module <= transitions[index] + 1.0e-6f)
	{
	  const real d = transitions[index - 1] - transitions[index];
	  const real dif = 1.0f - (module - d) / rEq;
	  if (std::isfinite(dif))
	  {
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

std::valarray<real>
getFiniteForce(Distance dist, const real beta, const real rEq)
{
  const real& module = dist.module;
  std::valarray<real> force(0.0f, parameters().DIMENSIONS);

  if (module <= (parameters().INTER_ELASTIC_UP_LIMIT + 1.0e-6)) // Elastic regime or called by recursion.
  {
    const real scalar = (1.0f - module / rEq) * beta;
    if (std::isfinite(scalar))
    {
      force = dist.getDirectionArray();
      force *= scalar;
    }
    return force;
  }
  else // Constant regime; call recursion.
  {
    dist.module = parameters().INTER_ELASTIC_UP_LIMIT;
    return getFiniteForce(dist, beta, rEq);
  }
}

std::valarray<real>
getFiniteRadialForceWithoutBeta(Distance dist, const real rEq, const type_int TYPE)
{
  const real& module = dist.module;
  std::valarray<real> force(-0.0f, parameters().DIMENSIONS);
  
  if (module <= parameters().RADIAL_PLASTIC_BEGIN[TYPE] + 1.0e-6)
  {
    const real dif = 1.0f - dist.module / rEq;
    //// const real difCubicRoot = std::pow(std::fabs(dif), parameters().RADIAL_SPRING_EXP);
    const real scalar = sign(dif) * std::fabs(dif);
    if (std::isfinite(scalar))
    {
      force = dist.getDirectionArray();
      force *= scalar;
    }
  }
  else if (module <= parameters().RADIAL_PLASTIC_END[TYPE] + 1.0e06) // Constant regime; call recursion.
  {
    dist.module = parameters().RADIAL_PLASTIC_BEGIN[TYPE];
    force = getFiniteRadialForceWithoutBeta(dist, rEq, TYPE);
  }
  else
  {
    const real d = parameters().RADIAL_PLASTIC_END[TYPE] - parameters().RADIAL_PLASTIC_BEGIN[TYPE];
    const real dif = 1.0f - (dist.module - d) / rEq;
    if (std::isfinite(dif))
    {
      force = dist.getDirectionArray();
      force *= dif;
    }
  }
  return force;
}
