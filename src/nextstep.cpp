// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#include "nextstep.hpp"
#include "Superboid.hpp"
#include "divide.hpp"
#include "export.hpp"
#include "parameters.hpp"
#include <thread>
#include <valarray>

static void nextVelocity(const thread_int THREAD_ID,
                         std::vector<Superboid> &superboids,
                         const step_int STEP) {
  for (auto &superboid : superboids) {
    if (superboid.ID % parameters().THREADS != THREAD_ID)
      continue;
    if (superboid.isActivated() == false)
      continue;
    for (auto &mini : superboid.miniboids)
      mini.setNextVelocity(STEP);
  }

  return;
}

static void nextPosition(const thread_int THREAD_ID,
                         std::vector<Superboid> &superboids,
                         const step_int step) {
  for (auto &superboid : superboids) {
    if (superboid.ID % parameters().THREADS != THREAD_ID)
      continue;
    if (superboid.isActivated() == false)
      continue;
    superboid.setNextPosition(step);
  }

  return;
}

static void nextBackInTime(const thread_int THREAD_ID,
                           std::vector<Superboid> &superboids,
                           const step_int step) {
  for (auto &superboid : superboids) {
    if (superboid.ID % parameters().THREADS != THREAD_ID)
      continue;
    if (superboid.isActivated() == false)
      continue;
    superboid.checkBackInTime(step);
  }

  return;
}

static void nextNeighbors(const thread_int THREAD_ID,
                          std::vector<Superboid> &superboids,
                          const step_int step) {
  for (auto &superboid : superboids) {
    if (superboid.ID % parameters().THREADS != THREAD_ID)
      continue;
    if (superboid.isActivated() == false)
      continue;
    for (auto &mini : superboid.miniboids)
      mini.setNeighbors(step); // Search for neighbors.
  }

  return;
}

static void nextCheckNeighbors(const thread_int THREAD_ID,
                               std::vector<Superboid> &superboids) {
  for (auto &superboid : superboids) {
    if (superboid.ID % parameters().THREADS != THREAD_ID)
      continue;
    if (superboid.isActivated() == false)
      continue;
    superboid.checkWrongNeighbors(superboids);
  }

  return;
}

static void nextVirtuals(const thread_int THREAD_ID,
                         std::vector<Superboid> &superboids, const bool export_,
                         const step_int step) {
  for (auto &superboid : superboids) {
    if (superboid.ID % parameters().THREADS != THREAD_ID)
      continue;
    if (superboid.isActivated() == false)
      continue;
    superboid.checkVirtual(export_, step);
  }

  return;
}

static void nextReset(const thread_int THREAD_ID,
                      std::vector<Superboid> &superboids, const bool shape,
                      const step_int STEP) {
  for (auto &superboid : superboids) {
    if (superboid.ID % parameters().THREADS != THREAD_ID)
      continue;
    if (superboid.isActivated() == false)
      continue;
    superboid.reset();
    if (shape)
      superboid.setShape(STEP);
  }

  return;
}

static void nextBoxes(std::vector<Box> &boxes,
                      std::vector<Superboid> &superboids, const step_int step) {
  for (auto &super : superboids) {
    if (super.isActivated() == true) {
      for (auto &mini : super.miniboids) {
        mini.checkLimits(step);
        const box_int newBoxID = Box::getBoxID(mini.position);
        const box_int oldBoxID = mini.boxID();
        if (oldBoxID != newBoxID) {
          boxes[oldBoxID].remove(mini);
          boxes[newBoxID].append(mini);
        }
      }
    }
  }

  for (auto &box : boxes) {
    const std::list<const Miniboid *> miniboids = box.miniboids;
    for (const auto &mini : miniboids)
      if (mini->isVirtual)
        box.remove(*mini);
  }

  for (auto &super : superboids)
    super.virtualMiniboids.clear();

  return;
}

void nextBoxes(std::vector<Box> &boxes, Superboid &super, const step_int step) {
  if (super.isActivated() == true)
    for (auto &mini : super.miniboids) {
      mini.checkLimits(step);
      const box_int newBoxID = Box::getBoxID(mini.position);
      const box_int oldBoxID = mini.boxID();
      if (oldBoxID != newBoxID) {
        boxes[oldBoxID].remove(mini);
        boxes[newBoxID].append(mini);
      }
    }

  super.clearVirtualMiniboids();

  return;
}

static void nextBoxes_putVirtuals(std::vector<Box> &boxes,
                                  std::vector<Superboid> &superboids,
                                  const step_int step) {
  for (auto &super : superboids) {
    for (auto &mini : super.virtualMiniboids) {
      mini.checkLimits(step);
      const box_int newBoxID = Box::getBoxID(mini.position);
      boxes[newBoxID].append(mini);
    }
  }

  return;
}

static void nextGamma(const thread_int THREAD_ID,
                      std::vector<Superboid> &superboids) {
  for (auto &superboid : superboids) {
    if (superboid.ID % parameters().THREADS != THREAD_ID)
      continue;
    if (superboid.isActivated() == false)
      continue;
    superboid.setGamma(superboids);
  }

  return;
}

static std::valarray<real>
getMeanPosition(const std::vector<Superboid> &superboids) {
  std::valarray<real> mean(parameters().DIMENSIONS);
  super_int divideBy = 0u;
  for (const auto &super : superboids)
    if (super.isActivated() == true) {
      ++divideBy;
      mean += super.miniboids[0u].position;
    }
  mean /= divideBy;

  return mean;
}

void correctPositionAndRotation(std::vector<Superboid> &superboids) {
  const std::valarray<real> meanPosition = getMeanPosition(superboids);
  for (auto &super : superboids)
    if (super.isActivated() == true)
      for (auto &mini : super.miniboids)
        mini.position -= meanPosition;
}

error::NextStepError nextStep(std::vector<Box> &boxes,
                              std::vector<Superboid> &superboids,
                              const step_int step, const bool shape,
                              const bool gamma, const bool checkVirt,
                              const bool exportVirt) {
#if 1
  if (parameters().BC == BoundaryCondition::PERIODIC)
    correctPositionAndRotation(superboids);
#else
#warning "You should enable correctPositionAndRotation."
#endif

  {
    for (auto &super : superboids)
      if (super.willDie())
        super.deactivate();
  }

  if (gamma) {
    static std::vector<std::thread> gammaThreads(parameters().THREADS);
    for (thread_int threadCount = 0u; threadCount < parameters().THREADS;
         ++threadCount)
      gammaThreads[threadCount] =
          std::thread(nextGamma, threadCount, std::ref(superboids));
    for (auto &thread : gammaThreads)
      thread.join();
  }

  {
    static std::vector<std::thread> resetThreads(parameters().THREADS);
    for (thread_int threadCount = 0u; threadCount < parameters().THREADS;
         ++threadCount)
      resetThreads[threadCount] = std::thread(
          nextReset, threadCount, std::ref(superboids), shape, step);
    for (auto &thread : resetThreads)
      thread.join();
  }

  {
    static std::vector<std::thread> virtualThreads(parameters().THREADS);
    for (thread_int threadCount = 0u; threadCount < parameters().THREADS;
         ++threadCount)
      virtualThreads[threadCount] = std::thread(
          nextVirtuals, threadCount, std::ref(superboids), exportVirt, step);
    for (auto &thread : virtualThreads)
      thread.join();
  }

  nextBoxes_putVirtuals(boxes, superboids, step);

  {
    static std::vector<std::thread> neighborsThreads(parameters().THREADS);
    for (thread_int threadCount = 0u; threadCount < parameters().THREADS;
         ++threadCount)
      neighborsThreads[threadCount] =
          std::thread(nextNeighbors, threadCount, std::ref(superboids), step);
    for (auto &thread : neighborsThreads)
      thread.join();
  }

  {
    static std::vector<std::thread> checkNeighborsThreads(parameters().THREADS);
    for (thread_int threadCount = 0u; threadCount < parameters().THREADS;
         ++threadCount)
      checkNeighborsThreads[threadCount] =
          std::thread(nextCheckNeighbors, threadCount, std::ref(superboids));
    for (auto &thread : checkNeighborsThreads)
      thread.join();
  }

  {
    static std::vector<std::thread> velocityThreads(parameters().THREADS);
    for (thread_int threadCount = 0u; threadCount < parameters().THREADS;
         ++threadCount)
      velocityThreads[threadCount] =
          std::thread(nextVelocity, threadCount, std::ref(superboids), step);
    for (auto &thread : velocityThreads)
      thread.join();
  }

  if (checkVirt)
    for (const auto &super : superboids) {
      if (super.isActivated() == false)
        continue;
      const size_t s = super.virtualMiniboids.size();
      if (s > 4 * parameters().MINIBOIDS_PER_SUPERBOID)
        return error::NextStepError::TOO_MANY_VIRTUALS_SINGLE_CELL;
    }

  {
    static std::vector<std::thread> positionThreads(parameters().THREADS);
    for (thread_int threadCount = 0u; threadCount < parameters().THREADS;
         ++threadCount)
      positionThreads[threadCount] =
          std::thread(nextPosition, threadCount, std::ref(superboids), step);
    for (auto &thread : positionThreads)
      thread.join();
  }

  {
    static std::vector<std::thread> backThreads(parameters().THREADS);
    for (thread_int threadCount = 0u; threadCount < parameters().THREADS;
         ++threadCount)
      backThreads[threadCount] =
          std::thread(nextBackInTime, threadCount, std::ref(superboids), step);
    for (auto &thread : backThreads)
      thread.join();
  }

  if (parameters().DIVISION_INTERVAL != 0u)
    if (step % parameters().DIVISION_INTERVAL ==
        parameters().DIVISION_INTERVAL - 1)
      divide(boxes, superboids, step);

  nextBoxes(boxes, superboids, step);

  return error::NextStepError::OK;
}
