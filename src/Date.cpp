// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

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
  char cString[1024u];
  strftime(cString, 1024u, "%b %d %Y  %T", gimmeADecentName);

  std::string banana(cString);
  return banana;
}

std::string
Date::getCompactRunTime(void)
{
  char cString[1024u];
  strftime(cString, 1024u, "%Y_%m_%d_%Hh%Mmin_%Ssec", gimmeADecentName);

  std::string papaya(cString);
  return papaya;
}
