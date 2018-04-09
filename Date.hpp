// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

#pragma once
#include <string>

class Date
{
public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static const std::string compiledTime;
  static const std::string prettyRunTime;
  static const std::string compactRunTime;
  virtual inline ~Date(void) {;}
private:
  static const time_t rawTime;
  static const tm* const gimmeADecentName;
  static std::string getPrettyRunTime(void);
  static std::string getCompactRunTime(void);
};
