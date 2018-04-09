// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

#include <thread>
#include <valarray>
#include "parameters.hpp"
#include "Superboid.hpp"
#include "nextstep.hpp"
#include "export.hpp"
#include "Date.hpp" ////
#include <fstream> ////
#include <random>

// inline static super_int
// getFirstID(const thread_int THREAD_ID)
// {
//   return ((SUPERBOIDS / THREADS) * THREAD_ID);
// }

// inline static super_int
// getLastID(const thread_int THREAD_ID)
// {
//   super_int last = (SUPERBOIDS / THREADS) * (1u + THREAD_ID) - 1u;
//   if (THREAD_ID == (THREADS -1u))
//     last += SUPERBOIDS % THREADS;
//   return last;
// }

static void
divide(std::vector<Box>& boxes,
       std::vector<Superboid>& superboids,
       const step_int step)
{
  const step_int nonDivisionInterval = NON_DIVISION_INTERVAL > step ? NON_DIVISION_INTERVAL : step;
      
  // super_int cellWithBiggestArea = 0u;
  // real      biggestArea = -0.0f;
  // for (const auto& super : superboids)
  // 	if (super.activated == true)
  // 	  if (super.area > biggestArea)
  // 	  {
  // 	    cellWithBiggestArea = super.ID;
  // 	    biggestArea = super.area;
  // 	  }

  //if (INITIAL_CONDITION == InitialCondition::LEFT_EDGE)
  {
    std::vector<super_int> cellsOnTheEdge;
    for (const auto& super : superboids)
      if (super.activated == true)
	if (super.getLastDivisionStep() + NON_DIVISION_INTERVAL <= nonDivisionInterval)
	  //if (super.miniboids[0u].position[X] < (-RECTANGLE_SIZE[X] / 2.0f + 2.0f * RADIAL_REQ[super.type]))
	  cellsOnTheEdge.emplace_back(super.ID);

    if (cellsOnTheEdge.size() == 0)
    {
      super_int closestCellID = 0u;
      real closestDistance = RANGE;
      for (const auto& super : superboids)
	if (super.activated == true)
	  if (super.miniboids[0u].position[X] + RECTANGLE_SIZE[X] / 2 > closestDistance)
	  {
	    closestCellID = super.ID;
	    closestDistance = super.miniboids[0u].position[X] + RECTANGLE_SIZE[X] / 2;
	  }
      cellsOnTheEdge.emplace_back(closestCellID);
    }
	
    static std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, cellsOnTheEdge.size() - 1);
    const super_int chosen = cellsOnTheEdge[distribution(generator)];
    // const super_int chosen = cellWithBiggestArea;
	
    for (auto& super : superboids)
      if (super.activated == false)
      {
	superboids[chosen].divide(2, super, boxes, step);
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
    if (superboid.ID % THREADS != THREAD_ID)
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
    if (superboid.ID % THREADS != THREAD_ID)
      continue;
    if (superboid.activated == false)
      continue;
    superboid.setNextPosition(step);
  }
  
  return;
}

static void
nextNeighbors(const thread_int THREAD_ID, std::vector<Superboid>& superboids)
{
  for (auto& superboid : superboids)
  {
    if (superboid.ID % THREADS != THREAD_ID)
      continue;
    if (superboid.activated == false)
      continue;
    for (auto& mini : superboid.miniboids)
      mini.setNeighbors(); // Search for neighbors.      
  }
  
  return;
}

static void
nextVirtuals(const thread_int THREAD_ID, std::vector<Superboid>& superboids, const bool export_)
{
  for (auto& superboid : superboids)
  {
    if (superboid.ID % THREADS != THREAD_ID)
      continue;
    if (superboid.activated == false)
      continue;
    superboid.checkVirtual(export_);
  }
  
  return;
}

static void
nextReset(const thread_int THREAD_ID, std::vector<Superboid>& superboids, const bool shape, const step_int STEP)
{
  for (auto& superboid : superboids)
  {
    if (superboid.ID % THREADS != THREAD_ID)
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
nextBoxes(std::vector<Box>& boxes, std::vector<Superboid>& superboids)
{
  for (auto& super : superboids)
    if (super.activated == true)
    {
      for (auto& mini : super.miniboids)
      {
	mini.checkLimits();
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
nextBoxes(std::vector<Box>& boxes, Superboid& super)
{
  if (super.activated == true)
    for (auto& mini : super.miniboids)
    {
      mini.checkLimits();
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
nextBoxes_putVirtuals(std::vector<Box>& boxes, std::vector<Superboid>& superboids)
{
  for (auto& super : superboids)
  {
    for (auto& mini : super.virtualMiniboids)
    {
      mini.checkLimits();
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
    if (superboid.ID % THREADS != THREAD_ID)
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
  std::valarray<real> mean(DIMENSIONS);
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
  if (BC == BoundaryCondition::PERIODIC)
    correctPositionAndRotation(superboids);
#else
#warning "You should enable correctPositionAndRotation."
#endif

  if (gamma)
  {
    static std::vector<std::thread> gammaThreads(THREADS);
    for (thread_int threadCount = 0u; threadCount < THREADS; ++threadCount)
      gammaThreads[threadCount] = std::thread(nextGamma, threadCount,   \
                                              std::ref(superboids));
    for (auto& thread : gammaThreads)
      thread.join();
  }

  /////std::cerr << "reset" << std::endl;
  {
    static std::vector<std::thread> resetThreads(THREADS);
    for (thread_int threadCount = 0u; threadCount < THREADS; ++threadCount)
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
    static std::vector<std::thread> virtualThreads(THREADS);
    for (thread_int threadCount = 0u; threadCount < THREADS; ++threadCount)
      virtualThreads[threadCount] = std::thread(nextVirtuals, threadCount, \
						std::ref(superboids),	\
						exportVirt);
    for (auto& thread : virtualThreads)
      thread.join();
  }
  /////std::cerr << "virtual" << std::endl;

  /////std::cerr << "virtual boxes" << std::endl;
  nextBoxes_putVirtuals(boxes, superboids);
  /////std::cerr << "virtual boxes" << std::endl;
  
  /////std::cerr << "neighbors" << std::endl;
  {
    static std::vector<std::thread> neighborsThreads(THREADS);
    for (thread_int threadCount = 0u; threadCount < THREADS; ++threadCount)
      neighborsThreads[threadCount] = std::thread(nextNeighbors,	\
						  threadCount,		\
						  std::ref(superboids));
    for (auto& thread : neighborsThreads)
      thread.join();
  }
  /////std::cerr << "neighbors" << std::endl;

  /////std::cerr << "velocity" << std::endl;
  {
    static std::vector<std::thread> velocityThreads(THREADS);
    for (thread_int threadCount = 0u; threadCount < THREADS; ++threadCount)
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
      if (s > 4 * MINIBOIDS_PER_SUPERBOID)
        return false;
    }
  
  /////std::cerr << "position" << std::endl;
  {
    static std::vector<std::thread> positionThreads(THREADS);
    for (thread_int threadCount = 0u; threadCount < THREADS; ++threadCount)
      positionThreads[threadCount] = std::thread(nextPosition, threadCount, \
                                                 std::ref(superboids),	\
						 step);
    for (auto& thread : positionThreads)
      thread.join();
  }
  /////std::cerr << "position" << std::endl;
  
  if (DIVISION_INTERVAL != 0u)
    if (step % DIVISION_INTERVAL == DIVISION_INTERVAL - 1)
      divide(boxes, superboids, step);

  /////std::cerr << "nextBoxes" << std::endl;
  nextBoxes(boxes, superboids);
  /////std::cerr << "nextBoxes" << std::endl;
  
  return true;
}
