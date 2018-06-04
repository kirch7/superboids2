// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#pragma once
#include <string>
#include <cstdint>
#include <cmath>
#include <vector>
#include <valarray>
#include <iostream>

#define SEED            (time(NULL))
typedef uint16_t type_int;
typedef uint16_t mini_int;
typedef uint16_t super_int;
typedef uint32_t box_int;
typedef uint64_t step_int;
typedef uint16_t dimension_int;
typedef uint16_t thread_int;
typedef float    real;

class Stokes;

enum class BoundaryCondition { PERIODIC, RECTANGLE, STOKES };
enum class InitialCondition { HEX_CENTER, LEFT_EDGE };
enum class KillCondition { NONE, RIGHT_EDGE, P0, RIGHT_EDGE_OR_P0 };

extern const real    HALF_PI;
extern const real         PI;
extern const real     TWO_PI;
extern const dimension_int X;
extern const dimension_int Y;
extern const dimension_int Z;

class Parameters
{
public:
  void set(void);
  
  BoundaryCondition BC;
  InitialCondition  INITIAL_CONDITION;
  KillCondition     KILL_CONDITION;
  real P0_LIMIT;
  std::vector<real> RECTANGLE_SIZE;
  
  std::vector<Stokes> STOKES_HOLES;
  
  step_int DIVISION_INTERVAL;
  step_int NON_DIVISION_INTERVAL;
  real TOLERABLE_P0;
  real DIVISION_REGION_X;

  dimension_int DIMENSIONS;
  type_int TYPES_NO;

  super_int SUPERBOIDS;
  super_int MAX_SUPERBOIDS;
  mini_int  MINIBOIDS_PER_SUPERBOID;
  std::vector<mini_int>  HARRIS_AMOUNT;
  
  box_int BOXES_IN_EDGE;
  box_int BOXES;
  real    RANGE;

  real    NEIGHBOR_DISTANCE;
  real    INTER_ELASTIC_UP_LIMIT;
  std::vector<real> RADIAL_PLASTIC_BEGIN;
  std::vector<real> RADIAL_PLASTIC_END;
  real    INITIAL_DISTANCE;
  real    INITIAL_ANGLE_BETWEEN;
  real    INITIAL_ANGLE_ID1;
  real    TWIST_EQ_ANGLE;
  real    CORE_DIAMETER;
  real    PRINT_CORE;
  real    INFINITE_FORCE;

  std::vector<real> PROPORTIONS;

  std::vector<real> TARGET_AREA;
  std::vector<real> RADIAL_REQ;
  std::vector<std::vector<real>> INTER_REQ;

  std::vector<std::vector<real>> RADIAL_BETA;
  std::vector<real> RADIAL_BETA_MEDIUM;
  real RADIAL_SPRING_EXP;
  std::vector<std::vector<real>> INTER_BETA;

  std::vector<std::vector<real>> KAPA;
  std::vector<real> KAPA_MEDIUM;

  std::vector<real>              TANGENT_REQ;
  std::vector<std::vector<real>> TANGENT_BETA;
  std::vector<real>              TANGENT_BETA_MEDIUM;

  std::vector<std::vector<real>> INTER_ALPHA;
  std::vector<real> AUTO_ALPHA;

  std::vector<real> SPEED;

  real ETA;
  real DT;

  step_int STEPS;
  step_int EXIT_INTERVAL;
  real     EXIT_FACTOR;

  thread_int   THREADS;

  real REAL_TOLERANCE;

  real getDivisionDistance(void) const;
protected:
  void set1(void);
  void setCells(void);
  void setDomain(void);
  void setDivision(void);
  void setRadial(void);
  void setInter(void);
  void setStokes(void);
};

template<typename T>
inline T
square(T t)
{
  return t*t;
}

template<typename T>
inline T
cube(T t)
{
  return t*t*t;
}

template<typename T>
inline int sign(T& v)
{
  if (v < static_cast<T>(0))
    return -1;
  else
    return +1;
}

template<typename Array>
real
getModule(const Array& array)
{
  real squareSum = 0.0f;
  for (auto component : array)
    squareSum += square(component);
  return std::sqrt(squareSum);
};

inline std::ostream& operator<<(std::ostream& os,               \
                                const std::valarray<real>& va)
{
  for (auto& component : va)
    os << std::fixed << component << '\t';
  
  return os;
}

extern const Parameters& parameters();
extern std::string& getParameters(void);
