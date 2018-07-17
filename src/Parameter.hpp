// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#pragma once
#include "parameters.hpp"

// extern unsigned long int getParameter_uint(const std::string&);
// extern real getParameter_real(const std::string&);
// extern std::vector<real> getParameter_vector(const std::string&);
// extern std::vector<std::vector<real>> getParameter_matrix(const
// std::string&);

template <typename Eita> Eita getParameter(const std::string &);

template <> std::string getParameter(const std::string &);
template <> real getParameter(const std::string &);
template <> unsigned long int getParameter(const std::string &);
template <> std::vector<real> getParameter(const std::string &);
template <> std::vector<std::vector<real>> getParameter(const std::string &);

extern void loadParametersFromString(const std::string &);
extern std::string getParametersSample();
extern void setParameters(void);
