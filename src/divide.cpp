// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#include <set>
#include "divide.hpp"

void
divide(std::vector<Box>& boxes,
       std::vector<Superboid>& superboids,
       const step_int step)
{
  const step_int nonDivisionInterval = parameters().NON_DIVISION_INTERVAL > step ? parameters().NON_DIVISION_INTERVAL : step;

  std::set<super_int> eligibleCells;

  for (const auto& super : superboids)
  {
    if (super.isActivated() == false)
      continue;
    if (super.getLastDivisionStep() + parameters().NON_DIVISION_INTERVAL <= nonDivisionInterval)
    {
      if (super
	  .miniboids[0]
	  .position[X] < parameters().DIVISION_REGION_X)
	eligibleCells.insert(super.ID);
    }
  }
  
  step_int atempts = 0;
  while (true)
  {
    ++atempts;

    if (atempts > 16)
      return;
    
    if (eligibleCells.size() == 0)
      return;

    static std::random_device deviceEngine;
    static std::default_random_engine generator(deviceEngine());
    std::uniform_int_distribution<int> distribution(0, eligibleCells.size() - 1);

    const super_int chosenOrd = distribution(generator);
    super_int chosenCount = 0;
    super_int chosen = 42424;
    for (const auto superID : eligibleCells)
    {
      if (chosenCount == chosenOrd)
      {
	chosen = superID;
	break;
      }
      else
	++chosenCount;
    }
    
    superboids[chosen].setShape(step);
    const real p0 = superboids[chosen].perimeter / std::sqrt(superboids[chosen].area);
    if (p0 > parameters().TOLERABLE_P0)
    {
      eligibleCells.erase(chosen);
      continue;
    }
  
    for (auto& super : superboids)
      if (super.isActivated() == false)
      {
	if (superboids[chosen].divide(2, super, boxes, step) == true)
	  return;
	else
	  break;
      }
  }
  
  return;
}

