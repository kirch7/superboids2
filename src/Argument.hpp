// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#pragma once
#include <iostream>
#include <string>
#include <vector>

typedef int (*ArgFunction)(const std::string &);

class Argument {
 public:
  inline static size_t getBiggestSize(void);
  static std::vector<Argument> &args(void) { return arguments; }
  static bool has(const std::string &);
  static Argument &get(const std::string &);
  static bool areAllMandatorySet(void);
  const std::string argument;
  const std::string secondArgument;
  const std::string help;
  const ArgFunction function;
  const bool mandatory;
  const bool preventRunning;
  const bool skipMandatory;
  inline Argument(const char *const arg, const char *const helppie,
                  const bool mand, const bool prev, const bool skipMand,
                  const ArgFunction funcPointer = nullptr,
                  const char *const secArg = "")
      : argument(arg),
        secondArgument(secArg),
        help(helppie),
        function(funcPointer),
        mandatory(mand),
        preventRunning(prev),
        skipMandatory(skipMand),
        isSet(false),
        valueSet("") {
    return;
  }
  friend int main(int, char **);

 protected:
  bool isSet;
  std::string valueSet;
  static std::size_t _biggestSize;
  static std::vector<Argument> arguments;
};

extern std::ostream &operator<<(std::ostream &os, const Argument &arg);
