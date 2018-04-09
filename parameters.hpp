// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

#pragma once
#include <string>
#include <cstdint>
#include <cmath>
#include <vector>

#define SEED            (time(NULL))
typedef uint16_t type_int;
typedef uint16_t mini_int;
typedef uint16_t super_int;
typedef uint32_t box_int;
typedef uint64_t step_int;
typedef uint16_t dimension_int;
typedef uint16_t thread_int;
typedef float    real;

enum class BoundaryCondition { PERIODIC, RECTANGLE, STOKES };
enum class InitialCondition { HEX_CENTER, LEFT_EDGE };

extern const BoundaryCondition BC;
extern const InitialCondition  INITIAL_CONDITION;
extern const std::vector<real> RECTANGLE_SIZE;

extern const step_int DIVISION_INTERVAL;
extern const step_int NON_DIVISION_INTERVAL;

extern const real    HALF_PI;
extern const real         PI;
extern const real     TWO_PI;
extern const dimension_int X;
extern const dimension_int Y;
extern const dimension_int Z;
extern const dimension_int DIMENSIONS;
extern const type_int TYPES_NO;

extern const super_int SUPERBOIDS;
extern const super_int MAX_SUPERBOIDS;
extern const mini_int  MINIBOIDS_PER_SUPERBOID;

extern const box_int BOXES_IN_EDGE;
extern const box_int BOXES;
extern const real    RANGE;

extern const real    NEIGHBOR_DISTANCE;
extern const real    INTER_ELASTIC_UP_LIMIT;
extern const std::vector<real> RADIAL_PLASTIC_BEGIN;
extern const std::vector<real> RADIAL_PLASTIC_END;
extern const real    INITIAL_DISTANCE;
extern const real    INITIAL_ANGLE_BETWEEN;
extern const real    INITIAL_ANGLE_ID1;
extern const real    TWIST_EQ_ANGLE;
extern const real    CORE_DIAMETER;
extern const real    PRINT_CORE;
extern const real    INFINITE_FORCE;

extern const std::vector<real> PROPORTIONS;

extern const std::vector<real> TARGET_AREA;
extern const std::vector<real> RADIAL_REQ;
extern const std::vector<std::vector<real>> INTER_REQ;

extern const std::vector<real> RADIAL_BETA;
extern const real RADIAL_SPRING_EXP;
extern const std::vector<std::vector<real>> INTER_BETA;

extern const std::vector<real> KAPA;

extern const std::vector<std::vector<real>> INTER_ALPHA;
extern const std::vector<real> AUTO_ALPHA;

extern const std::vector<real> VELOCITY;

extern const std::vector<std::string> COLOURS;
extern const uint16_t PLOT_SIZE;

extern const real ETA;
extern const real DT;

extern const step_int STEPS;
extern const step_int EXIT_INTERVAL;
extern const real     EXIT_FACTOR;

extern const unsigned int DATE_SIZE;
extern const thread_int   THREADS;

extern const real REAL_TOLERANCE;

extern std::string& getParameters(void);

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

