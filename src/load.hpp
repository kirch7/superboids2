// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#pragma once
#include <fstream>
#include <vector>
#include "Superboid.hpp"
#include "parameters.hpp"

class InitialPositions {
 public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static inline bool load(void) { return _load; }
  static inline std::ifstream &file(void) { return _file; }
  static inline step_int startStep(void) { return _startStep; }
  friend int setInitialPositionsFile(const std::string &);
  virtual inline ~InitialPositions() { ; }

 private:
  static bool _load;
  static std::ifstream _file;
  static step_int _startStep;
};

extern void loadPositions(std::vector<Superboid> &);
