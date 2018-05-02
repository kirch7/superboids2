// Copyright (C) 2016-2018 Cássio Kirch.
// License specified in LICENSE file.

#include <iostream>
#include <map>
#include <tuple>
#include <sstream>
#include <exception>
#include "Parameter.hpp"

class SuperParameter
{
public:
  SuperParameter(const std::string&, const bool);
  bool isSet(void) const { return this->_modifiedCount > 0; }
  void pushDependency(const std::string&);
  std::size_t getElementsNo(void);
  
  const std::string name;
  bool areDependenciesSet(void);
protected:
  std::size_t _modifiedCount;
  bool        _canChange;
  std::vector<std::string> _dependencies;
};

template <typename P> class Parameter : public SuperParameter
{
public:
  static std::vector<Parameter<P>> map;
  static bool has(const std::string&);
  static Parameter<P> get(const std::string&);
  
  Parameter(const std::string&, const bool);
  const P& operator()(void) const;
  void operator=(const P&);
  void set(const P&);
private:
  P _parameter;
};

template<> std::vector<Parameter<std::string>>                    Parameter<std::string>::map  = std::vector<Parameter<std::string>>();
template<> std::vector<Parameter<unsigned long int>>              Parameter<unsigned long int>::map = std::vector<Parameter<unsigned long int>>();
template<> std::vector<Parameter<real>>                           Parameter<real>::map = std::vector<Parameter<real>>();
template<> std::vector<Parameter<std::vector<real>>>              Parameter<std::vector<real>>::map = std::vector<Parameter<std::vector<real>>>();
template<> std::vector<Parameter<std::vector<std::vector<real>>>> Parameter<std::vector<std::vector<real>>>::map = std::vector<Parameter<std::vector<std::vector<real>>>>();

template<typename P>
bool
Parameter<P>::has(const std::string& n)
{
  for (const auto& elem : map)
    if (elem.name == n)
      return true;

  return false;
}

template<typename P>
Parameter<P>
Parameter<P>::get(const std::string& n)
{
  for (const auto& elem : map)
    if (elem.name == n)
      return elem;

  std::cerr << "BLAME IT TO THE PROGRAMMER" << std::endl;
  std::exit(3);
}

void
setParameters(void)
{
  if (Parameter<std::string>::map.size() +
      Parameter<unsigned long int>::map.size() +
      Parameter<real>::map.size() +
      Parameter<std::vector<real>>::map.size() +
      Parameter<std::vector<std::vector<real>>>::map.size() != 0)
    return;
  
  auto& string_set = Parameter<std::string>::map;
  string_set.emplace_back("boundary", false);
  string_set.emplace_back("initial", false);
  string_set.emplace_back("kill", false);
    
  auto& uint_set = Parameter<unsigned long int>::map;
  uint_set.emplace_back("dimensions", false);
  uint_set.emplace_back("types", false);
  uint_set.emplace_back("cells", false);
  uint_set.emplace_back("max_cells", false);
  uint_set.emplace_back("particles_per_cell", false);
  uint_set.emplace_back("division", true);
  uint_set.emplace_back("non_division", true);
  uint_set.emplace_back("steps", true);
  uint_set.emplace_back("exit_interval", true);
  uint_set.emplace_back("threads", true);
  
  auto& real_set = Parameter<real>::map;
  real_set.emplace_back("tolerable_p0", true);
  real_set.emplace_back("domain", false);
  real_set.back().pushDependency("dimensions");
  real_set.emplace_back("neighbor_distance", false);
  real_set.emplace_back("initial_distance", true);
  real_set.emplace_back("core_diameter", true);
  real_set.emplace_back("core_intensity", true);
  real_set.emplace_back("print_core", true);
  real_set.emplace_back("eta", true);
  real_set.emplace_back("dt", true);
  real_set.emplace_back("exit_factor", true);
  real_set.emplace_back("real_tolerance", true);
  
  auto& vector_set = Parameter<std::vector<real>>::map;
  vector_set.emplace_back("rectangle", true);
  vector_set.back().pushDependency("domain");
  vector_set.emplace_back("stokes", true);
  vector_set.back().pushDependency("rectangle");
  vector_set.emplace_back("radial_plastic_begin", true);
  vector_set.back().pushDependency("types");
  vector_set.emplace_back("radial_plastic_end", true);
  vector_set.back().pushDependency("types");
  vector_set.emplace_back("proportions", true);
  vector_set.back().pushDependency("types");
  vector_set.emplace_back("target_area", true);
  vector_set.back().pushDependency("types");
  vector_set.emplace_back("radial_eq", true);
  vector_set.back().pushDependency("types");
  vector_set.emplace_back("kapa", true);
  vector_set.back().pushDependency("types");
  vector_set.emplace_back("auto_alpha", true);
  vector_set.back().pushDependency("types");
  vector_set.emplace_back("speed", true);
  vector_set.back().pushDependency("types");

  auto& matrix_set = Parameter<std::vector<std::vector<real>>>::map;
  matrix_set.emplace_back("inter_eq", true);
  matrix_set.back().pushDependency("types");
  matrix_set.emplace_back("inter_beta", true);
  matrix_set.back().pushDependency("types");
  matrix_set.emplace_back("radial_beta", true);
  matrix_set.back().pushDependency("types");
  matrix_set.emplace_back("inter_alpha", true);
  matrix_set.back().pushDependency("types");
}

SuperParameter::SuperParameter(const std::string& n, const bool a):
  name(n),
  _modifiedCount(0),
  _canChange(a) { }

template <typename P>
Parameter<P>::Parameter(const std::string& n, const bool a):
  SuperParameter(n, a) {}

template <typename P>
const P&
Parameter<P>::operator()(void) const {
  return this->_parameter;
}

std::size_t
SuperParameter::getElementsNo(void)
{
  if (this->_dependencies.size() == 0)
    throw "Eita, giovana!";

  std::size_t elemNo = 0;
  for (const auto& depName : this->_dependencies)
  {
    for (const auto& dep : Parameter<unsigned long int>::map)
      if (dep.name == depName)
	elemNo = dep();
  }

  return elemNo;
}

template <typename P>
void
Parameter<P>::operator=(const P& p)
{
  if (!this->_canChange && this->_modifiedCount != 0)
    throw  "redefinition of final parameter";
  
  this->_parameter = p;
  ++this->_modifiedCount;
}

bool
SuperParameter::areDependenciesSet(void)
{
  if (this->_dependencies.empty())
    return true;
  
  std::vector<std::size_t> counter(this->_dependencies.size(), 0);
  for (std::size_t depIndex = 0; depIndex < this->_dependencies.size(); ++depIndex)
  {
    if (Parameter<std::string>::has(this->_dependencies[depIndex]))
      if (!Parameter<std::string>::get(this->_dependencies[depIndex]).isSet())
	++counter[depIndex];
    
    if (Parameter<unsigned long int>::has(this->_dependencies[depIndex]))
      if (!Parameter<unsigned long int>::get(this->_dependencies[depIndex]).isSet())
	++counter[depIndex];

    if (Parameter<real>::has(this->_dependencies[depIndex]))
      if (!Parameter<real>::get(this->_dependencies[depIndex]).isSet())
      ++counter[depIndex];
    
    if (Parameter<std::vector<real>>::has(this->_dependencies[depIndex]))
      if (!Parameter<std::vector<real>>::get(this->_dependencies[depIndex]).isSet())
      ++counter[depIndex];
    
    if (Parameter<std::vector<std::vector<real>>>::has(this->_dependencies[depIndex]))
      if (!Parameter<std::vector<std::vector<real>>>::get(this->_dependencies[depIndex]).isSet())
      ++counter[depIndex];
  }
  
  for (const auto& count : counter)
    if (count != 0)
      return false;

  return true;
}

void
SuperParameter::pushDependency(const std::string& dep)
{
  this->_dependencies.push_back(dep);
}

template<> std::string getParameter(const std::string& name)
{
  for (const auto& comp : Parameter<std::string>::map)
    if (comp.name == name)
      return comp();

  std::cerr << "parameter " << name << " not found" << std::endl;
  std::exit(2);
}

template<> unsigned long int getParameter(const std::string& name)
{
  for (const auto& comp : Parameter<unsigned long int>::map)
    if (comp.name == name)
      return comp();

  std::cerr << "parameter " << name << " not found" << std::endl;
  std::exit(2);
}

template<> real getParameter(const std::string& name)
{
  for (const auto& comp : Parameter<real>::map)
    if (comp.name == name)
      return comp();
 
  std::cerr << "parameter " << name << " not found" << std::endl;
  std::exit(2);
}

template<> std::vector<real> getParameter(const std::string& name)
{
  for (const auto& comp : Parameter<std::vector<real>>::map)
    if (comp.name == name)
      return comp();

    std::cerr << "parameter " << name << " not found" << std::endl;
    std::exit(2);
}

template<> std::vector<std::vector<real>> getParameter(const std::string& name)
{
  for (const auto& comp : Parameter<std::vector<std::vector<real>>>::map)
    if (comp.name == name)
      return comp();

  std::cerr << "parameter " << name << " not found" << std::endl;
  std::exit(2);
}

static void
panic(const std::string& m, std::size_t lineNo)
{
  std::cerr << "Error (line " << lineNo << "): " << m << std::endl;
  std::exit(1);
}

static void
panic(const std::string& m, const std::tuple<std::string, std::size_t>& t)
{
  std::cerr << "Error (line " << std::get<1>(t) << "): " << m << std::endl;
  std::cerr << std::get<0>(t) << std::endl;
  std::exit(1);
}

static std::string
stripFront(const std::string s)
{
  if (s.size() == 0)
    return std::string();

  const char c = s.front();
  if (c == '\n' || c == ' ' || c == '\t' || c == '\r')
    return stripFront(s.substr(1));
  return s;
}

static std::string
stripBack(const std::string s)
{
  if (s.size() == 0)
    return std::string();

  const char c = s.back();
  if (c == '\n' || c == ' ' || c == '\t' || c == '\r')
    return stripBack(s.substr(0, s.size() - 1));
  else
    return s;
}

static std::string
strip(const std::string s)
{
  return stripFront(stripBack(s));
}

static std::vector<std::tuple<std::string, std::size_t>>
splitLines(const std::string& s)
{
  std::vector<std::tuple<std::string, std::size_t>> vec;
  std::size_t p0 = 0;
  std::size_t lineNo = 1;
  
  for (std::size_t index = 0; index < s.size(); ++index)
  {
    if (index != p0)
      if (s[index] == '\n' || index == s.size() - 1)
      {
	std::string subs = s.substr(p0, index - p0);
	subs = strip(subs);
	if (subs.size() > 0)
	  vec.push_back(std::tuple<std::string, std::size_t>(subs, lineNo));
	p0 = index;
      }
    if (s[index] == '\n')
      ++lineNo;
  }
  
  return vec;
}


static std::tuple<std::string, std::string>
splitTwo(std::string s, std::size_t lineNo)
{
  s = strip(s);

  // Check if s starts with equals.
  if (s.front() == '=')
    panic("starts with equals", lineNo);

  std::size_t end = s.size();
  
  // Count occurences of '='
  std::size_t occurrencesOfEquals =  0;
  for (std::size_t index = 0; index < s.size(); ++index)
    if (s[index] == '#')
    {
      end = index + 1;
      break;
    }
    else if (s[index] == '=')
      ++occurrencesOfEquals;

  
  if (occurrencesOfEquals != 1)
    panic("amount of equals", lineNo);

  std::string key("");
  std::string value("");
  
  for (std::size_t index = 0; index < end; ++index)
    if (s[index] == '=')
    {
      key = s.substr(0, index - 1);
      value = s.substr(index + 1, end);
      break;
    }

  key = strip(key);
  value = strip(value);
  
  return std::tuple<std::string, std::string>(key, value);
}

static std::vector<std::string>
splitWords(const std::string& s)
{
  std::vector<std::string> vec;
  std::size_t p0 = 0;
  
  for (std::size_t index = 0; index < s.size(); ++index)
  {
    if (((s[index] < 'a' || s[index] > 'z') && (s[index] < 'A' || s[index] > 'Z') &&
	 (s[index] < '0' || s[index] > '9') && s[index] != '.' && s[index] != '-'  && s[index] != '_') ||
	 index + 1 == s.size())
    {
      std::string subs = s.substr(p0, index - p0 + 1);
      subs = strip(subs);
      if (subs.size() > 0)
	vec.push_back(subs);
      p0 = index;
    }
  }

  return vec;
  
}

static bool
isNumeric(const std::string& s)
{
  for (const char c : s)
    if (!std::isdigit(c) && c != '.' && c != '-')
      return false;
  
  return true;
}

void
loadParametersFromString(const std::string& raw)
{
  setParameters();
  /*std::cerr << Parameter<std::string>::map.size() << std::endl;
  std::cerr << Parameter<unsigned long int>::map.size() << std::endl;
  std::cerr << Parameter<real>::map.size() << std::endl;
  std::cerr << Parameter<std::vector<real>>::map.size() << std::endl;
  std::cerr << Parameter<std::vector<std::vector<real>>>::map.size() << std::endl;*/

  auto vec = splitLines(raw);

  for (const auto& t : vec)
  {
    const std::string& s = std::get<0>(t);
    const std::size_t lineNo = std::get<1>(t);
    if (strip(s).front() == '#')
      continue;
    const auto tuple2 = splitTwo(s, lineNo);
    const std::string _key   = strip(std::get<0>(tuple2));
    const std::string _value = strip(std::get<1>(tuple2));
    
    const auto keys = splitWords(_key);    
    const auto values = splitWords(_value);

    std::size_t matchesNo = 0;
    
    // Non numeric parameters:
    for (auto& parameter : Parameter<std::string>::map)
      if (parameter.name == keys[0])
      {
	if (!parameter.areDependenciesSet())
	  panic("dependencies unset", t);
	++matchesNo;
	if (values.size() != 1)
	  panic("invalid value", t);
	parameter = values[0];
      }

    // Integer number parameters:
    for (auto& parameter : Parameter<unsigned long int>::map)
      if (parameter.name == keys[0])
      {
	if (!parameter.areDependenciesSet())
	  panic("dependencies unset", t);
	++matchesNo;
	if (values.size() != 1)
	  panic("invalid value", t);
	else if (!isNumeric(values[0]))
	  panic("non numeric", t);
	else
	  parameter = std::stoul(values[0]);
      }

    // Real number parameters:
    for (auto& parameter : Parameter<real>::map)
      if (parameter.name == keys[0])
      {
	if (!parameter.areDependenciesSet())
	  panic("dependencies unset", t);
	++matchesNo;
	bool ok = false;
	if (values.size() != 1)
	  panic("invalid values", t);
	for (const auto r : Parameter<real>::map)
	  if (values[0] == r.name)
	  {
	    if (r.isSet())
	    {
	      parameter = r();
	      ok = true;
	    }
	    else
	      panic("dependency unset", t);
	  }
	if (ok)
	  continue;
	if (isNumeric(values[0]))
	  parameter = std::stof(values[0]);
	else
	  panic("invalid value", t);
      }

    // Vector parameters:
    for (auto& parameter : Parameter<std::vector<real>>::map)
      if (parameter.name == keys[0])
      {
	if (!parameter.areDependenciesSet())
	  panic("dependencies unset", t);
	++matchesNo;
	const std::size_t elementsNo = parameter.getElementsNo();
	
	if (keys.size() == 1)
	{
	  if (values.size() == 1)
	  {
	    if (isNumeric(values[0]))
	      parameter = std::vector<real>(elementsNo, std::stof(values[0]));
	    else
	    {
	      if (Parameter<real>::has(values[0]) && Parameter<real>::get(values[0]).isSet())
		parameter = std::vector<real>(elementsNo, Parameter<real>::get(values[0])());
	      else
		panic("invalid value", t);
	    }
	  }
	  else if (values.size() == elementsNo)
	  {
	    auto newVec = std::vector<real>();
	    for (const auto& valueStr : values)
	    {
	      if (!isNumeric(valueStr))
		panic("non numeric", t);
	      newVec.push_back(std::stof(valueStr));
	      parameter = newVec;
	    }
	  }
	  else
	    panic("wrong number of elements", t);
	}
	else if (keys.size() == 2)
	{
	  if (!isNumeric(keys[1]))
	    panic("invalid key", t);
	  else
	  {
	    if (values.size() != 1)
	      panic("invalid value", t);
	    else if (isNumeric(values[0]))
	    {
	      auto oldVec = parameter();
	      oldVec[std::stoul(keys[1])] = std::stof(values[0]);
	      parameter = oldVec;
	    }
	    else
	    {
	      if (Parameter<real>::has(values[0]) && Parameter<real>::get(values[0]).isSet())
		parameter = std::vector<real>(elementsNo, Parameter<real>::get(values[0])());
	      else
		panic("invalid value", t);
	      
	    }
	  }
	}
	else
	  panic("invalid key", t);
      }
    
    // Matrix parameters:
    for (auto& parameter : Parameter<std::vector<std::vector<real>>>::map)
      if (parameter.name == keys[0])
      {
	if (!parameter.areDependenciesSet())
	  panic("dependencies unset", t);
	++matchesNo;
	const std::size_t elementsNo = parameter.getElementsNo();
	if (keys.size() == 1)
	{
	  if (values.size() == 1 && isNumeric(values[0]))
	    parameter = std::vector<std::vector<real>>(elementsNo, std::vector<real>(elementsNo, std::stof(values[0])));
	  else if (values.size() == 1 && !isNumeric(values[0]))
	  {
	    if (values[0] == "mirror")
	    {
	      auto matrix = parameter();
	      for (std::size_t line = 0; line < matrix.size(); ++line)
	      {
		if (matrix[line].size() != elementsNo)
		  panic("algorithm problem", t);
		for (std::size_t column = 0; column < elementsNo; ++column)
		  if (line < column)
		    matrix[column][line] = matrix[line][column];
	      }
	      parameter = matrix;
	    }
	  }
	  else if (values.size() == square(elementsNo))
	  {
	    auto newMatrix = std::vector<std::vector<real>>();
	    auto newVec = std::vector<real>();
	    for (const auto& valueStr : values)
	    {
	      if (!isNumeric(valueStr))
		panic("non numeric", t);
	      newVec.push_back(std::stof(valueStr));
	      if (newVec.size() == elementsNo)
	      {
		newMatrix.push_back(newVec);
		newVec.clear();
	      }
	    }
	    parameter = newMatrix;
	  }
	  else
	    panic("wrong number of elements", t);
	}
	else if (keys.size() == 2)
	  panic("unimplemented", t);
	else if (keys.size() == 3)
	  panic("unimplemented", t);
	else
	  panic("invalid key", t);
      }

    if (matchesNo == 0)
    {
      std::cerr << keys.size() << std::endl;
      panic("key not found", t);
    }
    else if (matchesNo > 1)
      panic("algorithm problem", t);
  }
}

std::string
getParametersSample()
{
  std::stringstream stream;

  for (const auto& param : Parameter<std::string>::map)
    stream << param.name << '\t' << '=' << '\t' << "sometext" << std::endl;

  for (const auto& param : Parameter<unsigned long int>::map)
    stream << param.name << '\t' << '=' << '\t' << "7" << std::endl;
  
  for (const auto& param : Parameter<real>::map)
    stream << param.name << '\t' << '=' << '\t' << "-0.0" << std::endl;
  
  for (const auto& param : Parameter<std::vector<real>>::map)
    stream << param.name << '\t' << '=' << '\t' << "-0.0" << std::endl;

  for (const auto& param : Parameter<std::vector<std::vector<real>>>::map)
    stream << param.name << '\t' << '=' << '\t' << "-0.0" << std::endl;

  return stream.str();
}
