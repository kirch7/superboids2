// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#pragma once
#include <string>

class Date {
 public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static const std::string compiledTime;
  static const std::string prettyRunTime;
  static const std::string compactRunTime;
  virtual inline ~Date(void) { ; }

 private:
  static const time_t rawTime;
  static const tm *const gimmeADecentName;
  static std::string getPrettyRunTime(void);
  static std::string getCompactRunTime(void);
};
