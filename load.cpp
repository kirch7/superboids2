// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

#include <iostream> ////
#include "parameters.hpp"
#include "load.hpp"

bool InitialPositions::_load(false);
std::ifstream InitialPositions::_file;
step_int InitialPositions::_startStep(0u);
super_int InitialPositions::_initialActivatedCellNo(SUPERBOIDS);

void loadPositions(std::vector<Superboid>& superboids)
{
  std::ifstream& file = InitialPositions::file();
  
  for (super_int superID = 0; superID < SUPERBOIDS; ++superID)
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
