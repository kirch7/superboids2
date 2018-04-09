#pragma once
#include <list>
#include "Miniboid.hpp"
#include "parameters.hpp"

class CellNeighbors
{
public:
  /*const std::list<super_int>& operator()(void);
  void append(const super_int id);
  inline CellNeighbors(const std::vector<Superboid>& supers):
    _duplicates(true),
    superboids(supers)
  {
    return;
  }
  const std::vector<Superboid>& superboids;*/
  const std::list<super_int>& operator()(void);
  void append(const super_int id);
  inline CellNeighbors(void): _duplicates(true) {return;}

protected:
  bool _duplicates;
  std::list<super_int> _list;
};
