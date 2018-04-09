// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

#pragma once

#include <vector>
#include "Superboid.hpp"
#include "Box.hpp"

extern bool nextStepOK(std::vector<Box>& boxes, std::vector<Superboid>&, const step_int, const bool shape, const bool gamma, const bool checkVirt, const bool exportVirt);
extern void correctPositionAndRotation(std::vector<Superboid>& superboids);
extern void nextBoxes(std::vector<Box>& boxes, Superboid&);
