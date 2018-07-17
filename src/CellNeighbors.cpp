// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#include "CellNeighbors.hpp"

const std::list<super_int> &CellNeighbors::operator()(void) {
  if (this->_duplicates) {
    this->_list.unique();
    this->_list.sort();
    this->_list.unique();
    this->_duplicates = false;
  }
  return this->_list;
}

void CellNeighbors::append(const super_int id) {
  this->_list.push_back(id);
  this->_duplicates = true;
  return;
}
