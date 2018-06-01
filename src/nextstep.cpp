// Copyright (C) 2016-2018 Cássio Kirch.
// License specified in LICENSE file.

#include <thread>
#include <valarray>
#include <set>
#include "parameters.hpp"
#include "Superboid.hpp"
#include "nextstep.hpp"
#include "export.hpp"
#include "Date.hpp" ////
#include <fstream> ////
#include <random>

static void
divide(std::vector<Box>& boxes,
       std::vector<Superboid>& superboids,
       const step_int step)
{
  const step_int nonDivisionInterval = parameters().NON_DIVISION_INTERVAL > step ? parameters().NON_DIVISION_INTERVAL : step;

  std::set<super_int> eligibleCells;

  for (const auto& super : superboids)
  {
    if (super.activated == false)
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
      if (super.activated == false)
      {
	if (superboids[chosen].divide(2, super, boxes, step) == true)
	  return;
	else
	  break;
      }
  }
  
  return;
}

static void
nextVelocity(const thread_int THREAD_ID, std::vector<Superboid>& superboids, const step_int STEP)
{
  for (auto& superboid : superboids)
  {
    if (superboid.ID % parameters().THREADS != THREAD_ID)
      continue;
    if (superboid.activated == false)
      continue;
    for (auto& mini : superboid.miniboids)
      mini.setNextVelocity(STEP);
  }
  
  return;
}

static void
nextPosition(const thread_int THREAD_ID, std::vector<Superboid>& superboids, const step_int step)
{
  for (auto& superboid : superboids)
  {
    if (superboid.ID % parameters().THREADS != THREAD_ID)
      continue;
    if (superboid.activated == false)
      continue;
    superboid.setNextPosition(step);
  }
  
  return;
}

static void
nextNeighbors(const thread_int THREAD_ID, std::vector<Superboid>& superboids, const step_int step)
{
  for (auto& superboid : superboids)
  {
    if (superboid.ID % parameters().THREADS != THREAD_ID)
      continue;
    if (superboid.activated == false)
      continue;
    for (auto& mini : superboid.miniboids)
      mini.setNeighbors(step); // Search for neighbors.
  }
  
  return;
}

static void
nextCheckNeighbors(const thread_int THREAD_ID, std::vector<Superboid>& superboids)
{
  for (auto& superboid : superboids)
  {
    if (superboid.ID % parameters().THREADS != THREAD_ID)
      continue;
    if (superboid.activated == false)
      continue;
    superboid.checkWrongNeighbors(superboids);
  }
  
  return;
}

static void
nextVirtuals(const thread_int THREAD_ID, std::vector<Superboid>& superboids, const bool export_, const step_int step)
{
  for (auto& superboid : superboids)
  {
    if (superboid.ID % parameters().THREADS != THREAD_ID)
      continue;
    if (superboid.activated == false)
      continue;
    superboid.checkVirtual(export_, step);
  }
  
  return;
}

static void
nextReset(const thread_int THREAD_ID, std::vector<Superboid>& superboids, const bool shape, const step_int STEP)
{
  for (auto& superboid : superboids)
  {
    if (superboid.ID % parameters().THREADS != THREAD_ID)
      continue;
    if (superboid.activated == false)
      continue;
    superboid.reset();
    if (shape)
      superboid.setShape(STEP);
  }
  
  return;
}

static void
nextBoxes(std::vector<Box>& boxes, std::vector<Superboid>& superboids, const step_int step)
{
  for (auto& super : superboids)
    if (super.activated == true)
    {
      for (auto& mini : super.miniboids)
      {
	mini.checkLimits(step);
	const box_int newBoxID = Box::getBoxID(mini.position);
	const box_int oldBoxID = mini.boxID();
	if (oldBoxID != newBoxID)
	{
	  boxes[oldBoxID].remove(mini);
	  boxes[newBoxID].append(mini);
	}
      }
    }
  for (auto& box : boxes)
  {
    const std::list<const Miniboid*> miniboids = box.miniboids;
    for (const auto mini : miniboids)
      if (mini->isVirtual)
	box.remove(*mini);
  }

  for (auto& super : superboids)
    super.virtualMiniboids.clear();
  
  return;
}

void
nextBoxes(std::vector<Box>& boxes, Superboid& super, const step_int step)
{
  if (super.activated == true)
    for (auto& mini : super.miniboids)
    {
      mini.checkLimits(step);
      const box_int newBoxID = Box::getBoxID(mini.position);
      const box_int oldBoxID = mini.boxID();
      if (oldBoxID != newBoxID)
      {
	boxes[oldBoxID].remove(mini);
	boxes[newBoxID].append(mini);
      }
    }
  
  super.virtualMiniboids.clear();
  
  return;
}

static void
nextBoxes_putVirtuals(std::vector<Box>& boxes, std::vector<Superboid>& superboids, const step_int step)
{
  for (auto& super : superboids)
  {
    for (auto& mini : super.virtualMiniboids)
    {
      mini.checkLimits(step);
      const box_int newBoxID = Box::getBoxID(mini.position);
      boxes[newBoxID].append(mini);
    }
  }
  
  return;
}

static void
nextGamma(const thread_int THREAD_ID, std::vector<Superboid>& superboids)
{
  for (auto& superboid : superboids)
  {
    if (superboid.ID % parameters().THREADS != THREAD_ID)
      continue;
    if (superboid.activated == false)
      continue;
    superboid.setGamma(superboids);
  }
  
  return;
}

static std::valarray<real>
getMeanPosition(const std::vector<Superboid>& superboids)
{
  std::valarray<real> mean(parameters().DIMENSIONS);
  super_int divideBy = 0u;
  for (const auto& super : superboids)
    if (super.activated == true)
    {
      ++divideBy;
      mean += super.miniboids[0u].position;
    }
  mean /= divideBy;

  return mean;
}

// static real
// getMeanAngle(const std::vector<Superboid>& superboids)
// {
//   real rot = 0.0f;
//   const super_int size = SUPERBOIDS * MINIBOIDS_PER_SUPERBOID;
//   for (const auto& super : superboids)
//     for (const auto& mini : super.miniboids)
//     {
//       const std::valarray<real>& position = mini.position;
//       rot += std::atan2(position[Y], position[X]) / size;
//     }
//   return rot;
// }

void
correctPositionAndRotation(std::vector<Superboid>& superboids)
{
  {
    const std::valarray<real> meanPosition = getMeanPosition(superboids);
    for (auto& super : superboids)
      if (super.activated == true)
	for (auto& mini : super.miniboids)
	  mini.position -= meanPosition;
  }
  // if (false) ////
  // {
  //   static real lastAngle = 0.0f;
  //   const real meanAngle = getMeanAngle(superboids);
  //   const real deltaAngle = meanAngle - lastAngle;
  //   lastAngle = meanAngle;
  //   for (auto& super : superboids)
  //     for (auto& mini : super.miniboids)
  //     {
  //       std::valarray<real> oldR  = mini.position;
  //       std::valarray<real>& newR = mini.position;
  //       //// não genérico em dimensões
  //       newR[X] = oldR[X] * std::cos(deltaAngle) - oldR[Y] * sin(deltaAngle);
  //       newR[Y] = oldR[Y] * std::cos(deltaAngle) + oldR[X] * sin(deltaAngle);

  //       std::valarray<real> oldV  = mini.velocity;
  //       std::valarray<real>& newV = mini.velocity;
  //       newV[X] = oldV[X] * std::cos(deltaAngle) - oldV[Y] * sin(deltaAngle);
  //       newV[Y] = oldV[Y] * std::cos(deltaAngle) + oldV[X] * sin(deltaAngle);
  //     }
  // }
}

bool
nextStepOK(std::vector<Box>& boxes,
	   std::vector<Superboid>& superboids,
	   const step_int step,
	   const bool shape,
	   const bool gamma,
	   const bool checkVirt,
	   const bool exportVirt)
{
#if 1
  if (parameters().BC == BoundaryCondition::PERIODIC)
    correctPositionAndRotation(superboids);
#else
#warning "You should enable correctPositionAndRotation."
#endif

  if (gamma)
  {
    static std::vector<std::thread> gammaThreads(parameters().THREADS);
    for (thread_int threadCount = 0u; threadCount < parameters().THREADS; ++threadCount)
      gammaThreads[threadCount] = std::thread(nextGamma, threadCount,   \
                                              std::ref(superboids));
    for (auto& thread : gammaThreads)
      thread.join();
  }

  /////std::cerr << "reset" << std::endl;
  {
    static std::vector<std::thread> resetThreads(parameters().THREADS);
    for (thread_int threadCount = 0u; threadCount < parameters().THREADS; ++threadCount)
      resetThreads[threadCount] = std::thread(nextReset, threadCount,   \
                                              std::ref(superboids),	\
					      shape,			\
					      step);
    for (auto& thread : resetThreads)
      thread.join();
  }
  /////std::cerr << "reset" << std::endl;

  /////std::cerr << "virtual" << std::endl;
  {
    static std::vector<std::thread> virtualThreads(parameters().THREADS);
    for (thread_int threadCount = 0u; threadCount < parameters().THREADS; ++threadCount)
      virtualThreads[threadCount] = std::thread(nextVirtuals, threadCount, \
						std::ref(superboids),	\
						exportVirt,
						step);
    for (auto& thread : virtualThreads)
      thread.join();
  }
  /////std::cerr << "virtual" << std::endl;

  /////std::cerr << "virtual boxes" << std::endl;
  nextBoxes_putVirtuals(boxes, superboids, step);
  /////std::cerr << "virtual boxes" << std::endl;
  
  /////std::cerr << "neighbors" << std::endl;
  {
    static std::vector<std::thread> neighborsThreads(parameters().THREADS);
    for (thread_int threadCount = 0u; threadCount < parameters().THREADS; ++threadCount)
      neighborsThreads[threadCount] = std::thread(nextNeighbors,	\
						  threadCount,		\
						  std::ref(superboids),
						  step);
    for (auto& thread : neighborsThreads)
      thread.join();
  }
  /////std::cerr << "neighbors" << std::endl;

  {
    static std::vector<std::thread> checkNeighborsThreads(parameters().THREADS);
    for (thread_int threadCount = 0u; threadCount < parameters().THREADS; ++threadCount)
      checkNeighborsThreads[threadCount] = std::thread(nextCheckNeighbors, \
						       threadCount,	\
						       std::ref(superboids));
    for (auto& thread : checkNeighborsThreads)
      thread.join();
  }

  
  /////std::cerr << "velocity" << std::endl;
  {
    static std::vector<std::thread> velocityThreads(parameters().THREADS);
    for (thread_int threadCount = 0u; threadCount < parameters().THREADS; ++threadCount)
      velocityThreads[threadCount] = std::thread(nextVelocity,		\
						 threadCount,		\
                                                 std::ref(superboids),	\
						 step);
    for (auto& thread : velocityThreads)
      thread.join();
  }
  /////std::cerr << "velocity" << std::endl;
  
  if (checkVirt)
    for (const auto& super : superboids)
    {
      if (super.activated == false)
	continue;
      const size_t s = super.virtualMiniboids.size();
      if (s > 4 * parameters().MINIBOIDS_PER_SUPERBOID)
        return false;
    }
  
  /////std::cerr << "position" << std::endl;
  {
    static std::vector<std::thread> positionThreads(parameters().THREADS);
    for (thread_int threadCount = 0u; threadCount < parameters().THREADS; ++threadCount)
      positionThreads[threadCount] = std::thread(nextPosition, threadCount, \
                                                 std::ref(superboids),	\
						 step);
    for (auto& thread : positionThreads)
      thread.join();
  }
  /////std::cerr << "position" << std::endl;
  
  if (parameters().DIVISION_INTERVAL != 0u)
    if (step % parameters().DIVISION_INTERVAL == parameters().DIVISION_INTERVAL - 1)
      divide(boxes, superboids, step);

  /////std::cerr << "nextBoxes" << std::endl;
  nextBoxes(boxes, superboids, step);
  /////std::cerr << "nextBoxes" << std::endl;
  
  return true;
}
