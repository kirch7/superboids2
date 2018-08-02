// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#include <random>
#include <ctime>

#include "initial.hpp"
#include "Stokes.hpp"

static std::valarray<real>
    initialNoise(const real radius) {
  static std::default_random_engine defaultEngine(std::time(NULL));
  static std::mt19937 mtEngine(defaultEngine());
  std::uniform_real_distribution<real> uniDistribution2pi(0.0, TWO_PI);
  std::uniform_real_distribution<real> uniDistributionRadius(0.0, radius);
  std::valarray<real> _noise(parameters().DIMENSIONS);
  if (parameters().DIMENSIONS == 2) {
    const real r = std::sqrt(uniDistributionRadius(mtEngine));
    const real a = uniDistribution2pi(mtEngine);
    _noise[X]    = r * std::cos(a);
    _noise[Y]    = r * std::sin(a);
  } else {
    std::cerr << "Not yet implemented." << std::endl << std::endl;
  }
  return _noise;
}

std::valarray<real>
    getCentralMiniboidPosition(void) {
  static super_int superboidCount    = 0u;
  static super_int superboidsOnLayer = 1u;
  static uint16_t layerCount         = 0u;
  static std::valarray<real> nextPosition(0.0f, parameters().DIMENSIONS);
  static real angle             = 0.0f;
  static const real DISTANCE    = parameters().INITIAL_DISTANCE;
  static const real DELTA_ANGLE = PI / 3.0f;

  const std::valarray<real> position(
      nextPosition + initialNoise(parameters().CORE_DIAMETER / 2.0f));

  if (superboidCount == (superboidsOnLayer - 1u)) {
    angle          = 0.0;
    superboidCount = 0u;
    ++layerCount;
    superboidsOnLayer = layerCount * 6u;
    nextPosition[Y]   = DISTANCE * (layerCount - 1);
    nextPosition[X]   = 0.0;
  } else {
    if (superboidCount == 0u)
      angle = 2.0 * DELTA_ANGLE;
    else if (superboidCount % layerCount == 0)
      angle += DELTA_ANGLE;
    ++superboidCount;
  }

  nextPosition[Y] += DISTANCE * std::cos(angle);
  nextPosition[X] += DISTANCE * std::sin(angle);
  return position;
}

bool
    tryArrangeCell(Miniboid& mini) {
  bool ready    = true;
  for (dimension_int dim = 0u; ready && dim < parameters().DIMENSIONS; ++dim) {
    const real comp       = mini.position[dim];
    const real HALF_RANGE = parameters().RANGE / 2.0f;
    if (HALF_RANGE - std::fabs(comp)
        < parameters().INITIAL_DISTANCE / 2.0f - parameters().REAL_TOLERANCE)
      ready = false;
    if (parameters().BC == BoundaryCondition::RECTANGLE
        || parameters().BC == BoundaryCondition::STOKES)
      if (parameters().RECTANGLE_SIZE[dim] / 2.0f - std::fabs(comp)
          < parameters().INITIAL_DISTANCE / 2.0f - 0.1f)
        ready = false;
  }

  if (parameters().BC == BoundaryCondition::STOKES)
    for (const auto& hole : parameters().STOKES_HOLES) {
      const Distance d(hole.center, mini.position);
      if (d.module
          < hole.radius + parameters().RADIAL_REQ[mini.superboid.type] - 0.1f)
        ready = false;
    }

  return ready;
}