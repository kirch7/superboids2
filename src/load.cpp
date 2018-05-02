// Copyright (C) 2016-2018 Cássio Kirch.
// License specified in LICENSE file.

#include <iostream> ////
#include "parameters.hpp"
#include "load.hpp"

bool InitialPositions::_load(false);
std::ifstream InitialPositions::_file;
step_int InitialPositions::_startStep(0u);

void loadPositions(std::vector<Superboid>& superboids)
{
  std::ifstream& file = InitialPositions::file();
  
  for (super_int superID = 0; superID < parameters().SUPERBOIDS; ++superID)
  {
    Superboid& super = superboids[superID];
    // Get cell type:
    std::string line;
    getline(file, line);
    *const_cast<type_int*>(&(super.type)) = std::stoul(line);

    // Get miniboids positions:
    for (auto& mini : super.miniboids)
    {
      for (auto& posComponent : mini.position)
        file.read(reinterpret_cast<char*>(&posComponent), sizeof(real));
      for (auto& velComponent : mini.velocity)
        file.read(reinterpret_cast<char*>(&velComponent), sizeof(real));
    }
  }
  
  return;
}
