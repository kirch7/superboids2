// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#pragma once

#include <valarray>
#include "Distance.hpp"
#include "parameters.hpp"

std::valarray<real> getFiniteForce(const Distance &, const real beta,
                                   const real rEq);
std::valarray<real> getFiniteForce(Distance, const real beta, const real rEq,
                                   const std::vector<real> &transitions);
