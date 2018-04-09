// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

#include <fstream>
#include <sstream>
#include "parameters.hpp"
#include "Date.hpp"

const std::string Date::compiledTime(__DATE__ "  " __TIME__);
const time_t Date::rawTime = time(NULL);
const tm* const Date::gimmeADecentName = localtime(&rawTime);
const std::string Date::prettyRunTime(Date::getPrettyRunTime());
const std::string Date::compactRunTime(Date::getCompactRunTime());

std::string
Date::getPrettyRunTime(void)
{
  char cString[DATE_SIZE];
  strftime(cString, DATE_SIZE, "%b %d %Y  %T", gimmeADecentName);

  std::string banana(cString);
  return banana;
}

std::string
Date::getCompactRunTime(void)
{
  char cString[DATE_SIZE];
  strftime(cString, DATE_SIZE, "%Y_%m_%d_%Hh%Mmin_%Ssec", gimmeADecentName);

  std::string papaya(cString);
  return papaya;
}
