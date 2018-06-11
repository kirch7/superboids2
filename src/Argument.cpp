// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

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
#include "Parameter.hpp"

std::vector<Argument> getArguments(void);

std::size_t Argument::_biggestSize(0u);
std::vector<Argument> Argument::arguments = getArguments();

bool
Argument::has(const std::string& s)
{
  for (const auto& arg : Argument::args())
    if (arg.argument == s)
      return true;
  return false;
}

Argument&
Argument::get(const std::string& s)
{
  for (auto& arg : Argument::args())
    if (arg.argument == s)
      return arg;
  
  throw "Cacaca";
}

bool
Argument::areAllMandatorySet(void)
{
  for (const auto& arg : Argument::args())
    if (arg.mandatory)
      if (!arg.isSet)
	return false;
  
  return true;
}

std::ostream&
operator<< (std::ostream& os, const Argument& arg)
{
  size_t tabsNo = Argument::getBiggestSize() + 1u -    \
    (arg.argument.size() + arg.secondArgument.size());
  std::string tab = "";
  for (size_t tabCount = tabsNo; tabCount > 0u; --tabCount)
    tab += " ";
  os << arg.argument << ' ' << arg.secondArgument << tab << arg.help;
  
  return os;
}

size_t
Argument::getBiggestSize(void)
{
  if (Argument::_biggestSize == 0u)
  {
    auto lambdaExp =[&](const std::vector<Argument>& argList)
      {
        for (const auto& arg : argList)
        {
          size_t size = arg.argument.size() + arg.secondArgument.size() + 1u;
          if (size > Argument::_biggestSize)
            Argument::_biggestSize = size;
        }
        return;
      };
    lambdaExp(Argument::args());
  }
  return Argument::_biggestSize;
}

static int
printHelp(const std::string&)
{
  {
    const std::vector<Argument>& list(getArguments());
    for (auto& arg : list)
      std::cout << arg << std::endl;
  }

  std::cout.flush();
  return 0;
}

int
setMSD(const std::string&)
{
  MSD::_export = true;
  MSD::_file.open((Date::compactRunTime + "_msd_v4.bin").c_str());
  return 0;
}

int
setBinPrint(const std::string&)
{
  BinPrint::_export = true;
  BinPrint::_file.open((Date::compactRunTime + "_binprint_v4.bin").c_str());
  return 0;
}

static int
printRadius(const std::string&)
{
  auto getTangentRadius =[](real rEq)
  {
    return                                                              \
    static_cast<real>(rEq * std::sqrt(2.0 * (1.0 - cos(2.0*PI /         \
                                                     (parameters().MINIBOIDS_PER_SUPERBOID - 1u)))));
  };
    
  for (type_int type = 0u; type < parameters().TYPES_NO; ++type)
    std::cout << getTangentRadius(parameters().RADIAL_REQ[type]) << std::endl;

  return 0;
}

static int
printParameters(const std::string&)
{
  std::cout << getParameters() << std::endl;
  std::cout.flush();
  return 0;
}

static int
printParametersSample(const std::string&)
{
  std::cout << getParametersSample() << std::endl;
  std::cout.flush();
  return 0;
}

static int
printExitSteps(const std::string&)
{
  std::cout << static_cast<unsigned>(static_cast<real>(parameters().STEPS)/static_cast<real>(parameters().EXIT_INTERVAL)) << std::endl;
  std::cout.flush();
  return 0;
}

static int
printThreadsNo(const std::string&)
{
  std::cout << parameters().THREADS << std::endl;
  std::cout.flush();
  return 0;
}

static int
printHalfRange(const std::string&)
{
  std::cout << parameters().RANGE/2.0f << std::endl;
  std::cout.flush();
  return 0;
}

int
setShapeExportation(const std::string&)
{
  Shape::_export = true;
  return 0;
}

int
setGammaExportation(const std::string&)
{
  Gamma::_export = true;
  return 0;
}

int
setPhi(const std::string&)
{
  Phi::_export = true;
  return 0;
}

int
setInitialPositionsFile(const std::string& filename)
{
  InitialPositions::_load = true;
  InitialPositions::_file.open(filename, std::ifstream::in | std::ifstream::binary);
  std::string line;
  std::getline(InitialPositions::file(), line);
  std::string cellsNo;
  std::getline(InitialPositions::file(), cellsNo);
  InitialPositions::_startStep = std::stoul(line);
  *const_cast<super_int* const>(&parameters().SUPERBOIDS) = std::stoul(cellsNo);
  if (parameters().MAX_SUPERBOIDS < parameters().SUPERBOIDS)
    *const_cast<super_int* const>(&parameters().MAX_SUPERBOIDS) = std::stoul(cellsNo);
  if (InitialPositions::_startStep >= parameters().STEPS)
    throw(std::range_error("Invalid step in binary file or invalid STEPS parameter."));
  
  return 0;
}

int
setSCS(const std::string&)
{
  SCS::_export = true;
  SCS::_file.open((Date::compactRunTime + "_scs.dat").c_str(), std::ios::out);
  return 0;
}

int
setInfinite(const std::string&)
{
  Infinite::_export = true;
  Infinite::_infFile.open(Date::compactRunTime + "_inf.dat", std::ios::out);
  Infinite::_inf2File.open(Date::compactRunTime + "_inf2.dat", std::ios::out);
  Infinite::_virtFile.open(Date::compactRunTime + "_virt.dat", std::ios::out);
  if (!BinPrint::write())
    return setBinPrint(std::string());
  return 0;
}

int
setLastStep(const std::string& stepString)
{
  *const_cast<step_int*>(&parameters().STEPS) = std::stol(stepString);

  return 0;
}

int
setParameters(const std::string& filename)
{
  std::ifstream file(filename);
  if (!file.is_open())
  {
    std::cerr << "could not open " << filename << std::endl;
    std::exit(11);
  }
  std::string content( (std::istreambuf_iterator<char>(file) ),
                       (std::istreambuf_iterator<char>()    ) );

  loadParametersFromString(content);

  const_cast<Parameters* const>(&parameters())->set();
  
  return 0;
}

std::vector<Argument>&
getMandatoryList(void)
{
  static std::vector<Argument> list;
  static bool firstTime = true;
  if (firstTime)
  {
    firstTime = false;
  }

  return list;
}

std::vector<Argument>
getArguments(void)
{
  std::vector<Argument> list;
  list.push_back(Argument("-h", "Show this help message.", false, true, true, printHelp));
  list.push_back(Argument("-p", "Show parameters.", false, true, false, printParameters));
  list.push_back(Argument("-sample", "Show parameters sample.", false, true, true, printParametersSample));
  list.push_back(Argument("-r", "Show ideal tangent radius.", false, true, false, printRadius));
  list.push_back(Argument("-v", "Show number of exit times.", false, true, false, printExitSteps));
  list.push_back(Argument("-range", "Show HALF of range value.", false, true, false, printHalfRange));
  list.push_back(Argument("-t", "Show number of worker threads.", false, true, false, printThreadsNo));
  list.push_back(Argument("-shape", "Export area and perimeter information.", false, false, false, setShapeExportation));
  list.push_back(Argument("-gamma", "Export segregation information.", false, false, false, setGammaExportation));
  list.push_back(Argument("-msd", "Export position of central particle.", false, false, false, setMSD));
  list.push_back(Argument("-binprint", "Export position of particles.", false, false, false, setBinPrint));
  list.push_back(Argument("-virtual", "Export position of virtual particles and infinite forces.", false, false, false, setInfinite));
  list.emplace_back("-phi", "Export velocity alignment.", false, false, false, setPhi);
  list.emplace_back("-initial", "Load initial positions from file.", false, false, false, setInitialPositionsFile, "[file]");
  list.emplace_back("-scs", "Single cell stability.", false, false, false, setSCS);
  list.emplace_back("-laststep", "Override last step.", false, false, false, setLastStep, "[naturalnumber]");
  list.push_back(Argument("-param", "Specify file with parameters", true, false, false, setParameters, "[file]"));
  
  return list;
}
