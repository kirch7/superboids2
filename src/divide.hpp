// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#pragma once

#include "Box.hpp"
#include "Superboid.hpp"
#include "parameters.hpp"
#include <vector>

extern void divide(std::vector<Box> &, std::vector<Superboid> &,
                   const step_int);
