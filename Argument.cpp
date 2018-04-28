// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <string>
#include <stdexcept>
#include "Date.hpp"
#include "parameters.hpp"
#include "export.hpp"
#include "load.hpp"
#include "Argument.hpp"

std::size_t Argument::_biggestSize(0u);

std::ostream& operator<< (std::ostream& os, const Argument& arg)
{
  size_t tabsNo = Argument::getBiggestSize() + 1u -    \
    (arg.argument.size() + arg.secondArgument.size());
  std::string tab = "";
  for (size_t tabCount = tabsNo; tabCount > 0u; --tabCount)
    tab += " ";
  os << arg.argument << ' ' << arg.secondArgument << tab << arg.help;
  
  return os;
}

size_t Argument::getBiggestSize(void)
{
  if (Argument::_biggestSize == 0u)
  {
    auto lambdaExp =[&](std::list<Argument>& argList)
      {
        for (const auto& arg : argList)
        {
          size_t size = arg.argument.size() + arg.secondArgument.size() + 1u;
          if (size > Argument::_biggestSize)
            Argument::_biggestSize = size;
        }
        return;
      };
    lambdaExp(getDontRunList());
    lambdaExp(getDoRunList());
  }
  return Argument::_biggestSize;
}

static int
printHelp(const void* const)
{
  {
    const std::list<Argument>& list(getDontRunList());
    for (auto& arg : list)
      std::cout << arg << std::endl;
  }
  {
    const std::list<Argument>& list(getDoRunList());
    for (auto& arg : list)
      std::cout << arg << std::endl;
  }
  std::cout.flush();
  return 0;
}

int
setMSD(const void* const)
{
  MSD::_export = true;
  MSD::_file.open((Date::compactRunTime + "_msd_v4.bin").c_str());
  return 0;
}

int
setBinPrint(const void* const)
{
  BinPrint::_export = true;
  BinPrint::_file.open((Date::compactRunTime + "_binprint_v4.bin").c_str());
  return 0;
}

static int
printRadius(const void* const)
{
  auto getTangentRadius =[](real rEq)
  {
    return                                                              \
    static_cast<real>(rEq * std::sqrt(2.0 * (1.0 - cos(2.0*PI /         \
                                                     (MINIBOIDS_PER_SUPERBOID - 1u)))));
  };
    
  for (type_int type = 0u; type < TYPES_NO; ++type)
    std::cout << getTangentRadius(RADIAL_REQ[type]) << std::endl;

  return 0;
}

static int
printParameters(const void* const)
{
  std::cout << getParameters() << std::endl;
  std::cout.flush();
  return 0;
}

static int
printExitSteps(const void* const)
{
  std::cout << static_cast<unsigned>(static_cast<real>(STEPS)/static_cast<real>(EXIT_INTERVAL)) << std::endl;
  std::cout.flush();
  return 0;
}

static int
printThreadsNo(const void* const)
{
  std::cout << THREADS << std::endl;
  std::cout.flush();
  return 0;
}

static int
printHalfRange(const void* const)
{
  std::cout << RANGE/2.0f << std::endl;
  std::cout.flush();
  return 0;
}

int
setShapeExportation(const void* const)
{
  Shape::_export = true;
  return 0;
}

int
setGammaExportation(const void* const)
{
  Gamma::_export = true;
  return 0;
}

int
setPhi(const void* const)
{
  Phi::_export = true;
  return 0;
}

int
setInitialPositionsFile(const void* const filename)
{
  InitialPositions::_load = true;
  InitialPositions::_file.open(static_cast<const char* const>(filename),
                               std::ifstream::in | std::ifstream::binary);
  std::string line;
  std::getline(InitialPositions::file(), line);
  std::string cellsNo;
  std::getline(InitialPositions::file(), cellsNo);
  InitialPositions::_startStep = std::stoul(line);
  InitialPositions::_initialActivatedCellNo = std::stoul(cellsNo);
  if (InitialPositions::_startStep >= STEPS)
    throw(std::range_error("Invalid step in binary file or invalid STEPS parameter."));
  
  return 0;
}

int
setSCS(const void* const)
{
  SCS::_export = true;
  SCS::_file.open((Date::compactRunTime + "_scs.dat").c_str(), std::ios::out);
  return 0;
}

int
setInfinite(const void* const)
{
  Infinite::_export = true;
  Infinite::_infFile.open(Date::compactRunTime + "_inf.dat", std::ios::out);
  Infinite::_inf2File.open(Date::compactRunTime + "_inf2.dat", std::ios::out);
  Infinite::_virtFile.open(Date::compactRunTime + "_virt.dat", std::ios::out);
  if (!BinPrint::write())
    return setBinPrint(nullptr);
  return 0;
}

int
setLastStep(const void* const stepVoid)
{
  const char* const stepCString = static_cast<const char*>(stepVoid);
  const std::string stepString(stepCString);
  STEPS = std::stol(stepString);

  return 0;
}

std::list<Argument>&
getDontRunList(void)
{
  static std::list<Argument> list;
  static bool firstTime = true;
  if (firstTime)
  {
    firstTime = false;
    list.push_back(Argument("-h", "Show this help message.", printHelp));
    list.push_back(Argument("-p", "Show parameters.", printParameters));
    ////list.push_back(Argument("-check", "Only check parameters."));
    list.push_back(Argument("-r", "Show ideal tangent radius.", printRadius));
    list.push_back(Argument("-v", "Show number of exit times.", printExitSteps));
    list.push_back(Argument("-range", "Show HALF of range value.", printHalfRange));
    list.push_back(Argument("-t", "Show number of worker threads.", printThreadsNo));
  }

  return list;
}

std::list<Argument>&
getDoRunList(void)
{
  static std::list<Argument> list;
  static bool firstTime = true;
  if (firstTime)
  {
    firstTime = false;
    ////list.push_back(Argument("-flex", "Run anyway."));
    list.push_back(Argument("-shape", "Export area and perimeter information.", setShapeExportation));
    list.push_back(Argument("-gamma", "Export segregation information.", setGammaExportation));
    list.push_back(Argument("-msd", "Export position of central particle.", setMSD));
    list.push_back(Argument("-binprint", "Export position of particles.", setBinPrint));
    list.push_back(Argument("-virtual", "Export position of virtual particles and infinite forces.", setInfinite));
    list.emplace_back("-phi", "Export velocity alignment.", setPhi);
    list.emplace_back("-initial", "Load initial positions from file.", setInitialPositionsFile, "[file]");
    list.emplace_back("-scs", "Single cell stability.", setSCS);
    list.emplace_back("-laststep", "Override last step.", setLastStep, "[naturalnumber]");
    ////list.push_back(Argument("-progress", "Show progress bar."));
  }

  return list;
}
