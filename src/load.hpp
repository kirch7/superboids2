// Copyright (C) 2016-2018 Cássio Kirch.
// License specified in LICENSE file.

#pragma once
#include <fstream>
#include <vector>
#include "Superboid.hpp"
#include "parameters.hpp"

class InitialPositions
{
public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static inline bool load(void) { return _load; }
  static inline std::ifstream& file(void) { return _file; }
  static inline step_int startStep(void)   { return _startStep; }
  static inline super_int initialActivatedCellNo(void) { return _initialActivatedCellNo; }
  friend int setInitialPositionsFile(const void* const);
  virtual inline ~InitialPositions() {;}
private:
  static super_int _initialActivatedCellNo;
  static bool _load;
  static std::ifstream _file;
  static step_int _startStep;
};

extern void loadPositions(std::vector<Superboid>&);

