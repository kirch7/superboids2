// Copyright (C) 2016-2018 Cássio Kirch.
// License specified in LICENSE file.

#pragma once
#include <iostream>
#include <string>
#include <list>

typedef int (*ArgFunction)(const void* const);

class Argument
{
public:
  inline static size_t getBiggestSize(void);
  const std::string argument;
  const std::string secondArgument;
  const std::string help;
  const ArgFunction function;
  inline Argument(const char* const arg, const char* const helppie,            \
                  const ArgFunction funcPointer=nullptr, const char* const secArg=""):
    argument(arg),
    secondArgument(secArg),
    help(helppie),
    function(funcPointer) { return; }

protected:
  static std::size_t _biggestSize;
};

extern std::list<Argument>& getDontRunList(void);
extern std::list<Argument>& getDoRunList(void);
extern std::ostream& operator<< (std::ostream& os, const Argument& arg);
