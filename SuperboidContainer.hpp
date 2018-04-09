#pragma once
#include <vector>
#include "Superboid.hpp"

class SuperboidContainer
{
public:
  std::vector<Superboid> superboids;

  SuperboidContainer(void);
protected:
  static unsigned long long int getSeed(const super_int id);
  
};
