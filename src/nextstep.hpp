// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#pragma once

#include <vector>
#include "Superboid.hpp"
#include "Box.hpp"

namespace error {
  enum class NextStepError
  {
    OK,
    TOO_MANY_VIRTUALS_AVERAGE,
    TOO_MANY_VIRTUALS_SINGLE_CELL
  };
}

extern error::NextStepError nextStep(std::vector<Box>&, std::vector<Superboid>&, const step_int, const bool shape, const bool gamma, const bool checkVirt, const bool exportVirt);
extern void correctPositionAndRotation(std::vector<Superboid>& superboids);
extern void nextBoxes(std::vector<Box>& boxes, Superboid&, const step_int);
