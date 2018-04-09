// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

#include <iostream> ////
#include <sstream>
#include "Date.hpp"
#include "parameters.hpp"

const real REAL_TOLERANCE = 1.0e-9;

const super_int SUPERBOIDS = 12u;                   // Cells number.
const super_int MAX_SUPERBOIDS = 1024u;
const type_int  TYPES_NO   = 12u;            // Cell types quantity.
const mini_int  MINIBOIDS_PER_SUPERBOID = 12u;      // Particles in a single cell. All cells have the same particles number.

const box_int   BOXES_IN_EDGE = 45u;                // sqrt(total boxes quantity).
const box_int   BOXES = BOXES_IN_EDGE*BOXES_IN_EDGE; // Total boxes quantity.
const real      RANGE = 50.0f;                      // Domain length.

const BoundaryCondition BC = BoundaryCondition::RECTANGLE;
//const BoundaryCondition BC = BoundaryCondition::PERIODIC;
const InitialCondition  INITIAL_CONDITION = InitialCondition::LEFT_EDGE;
//const InitialCondition  INITIAL_CONDITION = InitialCondition::HEX_CENTER;
const std::vector<real> RECTANGLE_SIZE = { 0.9f * RANGE, 0.5f * RANGE };

const step_int DIVISION_INTERVAL = 10000u; // Every DIVISION_INTERVAL a division will occur.
const step_int NON_DIVISION_INTERVAL = 50000u; // A cell can not divide if it is product of a division in the last NON_DIVISION_INTERVAL.

#ifndef _RADIAL_PLASTIC_END // Plastic starts at 1.2 and ends at elastic_again
  #define _RADIAL_PLASTIC_END 2.0f
#endif

#ifndef _NEIDIST
  #define _NEIDIST 1.1f                             // Max range of intercell miniboids force
#endif

#ifndef _INTER_ELASTIC_UP_LIMIT
  #define _INTER_ELASTIC_UP_LIMIT RANGE          //Beginning of domain range for plastic intercellular force
#endif

#ifndef _RADIAL_PLASTIC_BEGIN
  #define _RADIAL_PLASTIC_BEGIN 1.2f
#endif

////const std::vector<real> RADIAL_PLASTIC_END     = {_RADIAL_PLASTIC_END, _RADIAL_PLASTIC_END};
//const std::vector<real> RADIAL_PLASTIC_END     = {RANGE, RANGE}; //{1.5f, RANGE};
const std::vector<real> RADIAL_PLASTIC_END     = {RANGE, RANGE, RANGE, RANGE, RANGE, RANGE, RANGE, RANGE, RANGE, RANGE, RANGE, RANGE}; //{1.5f, RANGE};
const real      NEIGHBOR_DISTANCE       = _NEIDIST;  // Intercell forces maximum reach distance.
const real      INTER_ELASTIC_UP_LIMIT  = _INTER_ELASTIC_UP_LIMIT;   /////////////////////////////////////////////
//const std::vector<real> RADIAL_PLASTIC_BEGIN   = {RANGE, RANGE}; //{1.2f, RANGE};
const std::vector<real> RADIAL_PLASTIC_BEGIN   = {RANGE, RANGE, RANGE, RANGE, RANGE, RANGE, RANGE, RANGE, RANGE, RANGE, RANGE, RANGE}; //{1.2f, RANGE};
const real      INITIAL_DISTANCE        = 2.0f; //2.75f;      // Cell-center-to-cell-center distance in STEP=0 (initial condition).
const real      INITIAL_ANGLE_BETWEEN   = HALF_PI; // rad  // Nowadays it is unused. It was used in twist spring tests.
const real      INITIAL_ANGLE_ID1       = 0.0f;    // rad  // Nowadays it is unused. It was used in twist spring tests.
const real      TWIST_EQ_ANGLE          = 2.0f;    // rad  // Nowadays it is unused. It was used in twist spring tests.
const real      CORE_DIAMETER           = 0.2f;    // "Infinite" force maximum reach distance.
const real      PRINT_CORE              = 0.2f; //0.175f; // Unit to GNUplot use in circles plotting.
const real      INFINITE_FORCE          = 1000.0f; // Simple "Infinite" magnitude.

const step_int  STEPS         = 2000000llu;        // Last step number.
const step_int  EXIT_INTERVAL = 100u; //100llu;           // Initial interval between outputs to files. In case EXIT_FACTOR=0, this interval is homogeneous.
const real      EXIT_FACTOR   = 0.0f; //0.8f;

const real      ETA       = 1.0f;                  // Noise weight
#define         V0          0.007f                 // Speed each one of all particles has in a single time step. For now, there is a single V0 to an entire simulation.
const real      DT        = 1.0f;                  // Delta time. Time step jump value.

static std::vector<real>
getUniformProportions()
{
  std::vector<real> vec(TYPES_NO, 1.0f/TYPES_NO);
  real sum = -0.0f;
  for (const auto& i : vec)
    sum += i;
  if (sum < 1.0f)
    vec[0] += (1.0f - sum);
  return vec;
}
// Proportions: array with elements having to sum 1. int(SUPERBOIDS*(nth element)) = (type n cells no).
//const std::vector<real> PROPORTIONS = {0.333333f, 0.666667f};
const std::vector<real> PROPORTIONS = getUniformProportions();

// Equilibrium distance is associated with elastic forces.
// Radial equilibrium distance (if it is radial, it is INTRAcellular):
#define REQ_1R      1.0f
#define REQ_2R      1.0f
// Intercell equilibrium distance (between two cells PERIPHERAL particles (there is a mechanism for intercell fatboid repulsion)):
#define _REQ_I      0.75f
#define REQ_1I1     _REQ_I
#define REQ_1I2     _REQ_I
#define REQ_2I1     REQ_1I2
#define REQ_2I2     _REQ_I

// Kapa is twist spring weight.
// Twist:
#ifdef _KAPA
  #define KAPA_1    _KAPA
#else
  #define KAPA_1    2.0f
#endif
#define KAPA_2    KAPA_1

// Beta is elastic and constant forces weight (remember Beta is not associated with twist springs).
// Radial betas (if it is radial, it is INTRAcellular):
#ifndef _RBETA
#define BETA_1R     0.10f  //beta_radial endo
#else
  #define BETA_1R     _RBETA
#endif
#define BETA_2R     0.10f  //BETA_1R  //beta_radial ecto
// Intercell betas (between two cells PERIPHERAL particles (there is a mechanism for intercell fatboid repulsion)):
#ifdef _IBETA
  #define _BETA_I _IBETA
#else
  #define _BETA_I 0.20f
#endif
// #define BETA_1I1    0.1f
// #define BETA_1I2    0.12247448713915890490f
// #define BETA_2I1    BETA_1I2
// #define BETA_2I2    0.15f
#define BETA_1I1    _BETA_I // Beta inter endo-endo
#define BETA_1I2    _BETA_I // Beta inter endo-ecto
#define BETA_2I1    BETA_1I2 // Beta inter ecto-endo
#define BETA_2I2    _BETA_I // Beta inter ecto-ecto

// Alpha is velocity allignment weight.
// AutoAlpha is alpha for same-cell particles.
// InterAlpha is an intercellular alpha.
#ifdef _ALPHA
  #define _AUTO_ALPHA     _ALPHA
  #define _INTER_ALPHA    _ALPHA
#else
  #ifndef _AUTO_ALPHA
    #define _AUTO_ALPHA     13.0f ////
  #endif
  #ifndef _INTER_ALPHA
    #define _INTER_ALPHA    13.0f
  #endif
#endif

#define AUTO_ALPHA_1      _AUTO_ALPHA // endo
#define AUTO_ALPHA_2      _AUTO_ALPHA // ecto

#define INTER_ALPHA_11    _INTER_ALPHA // endo-endo
#define INTER_ALPHA_12    _INTER_ALPHA // endo-ecto
#define INTER_ALPHA_21    _INTER_ALPHA // ecto-endo
#define INTER_ALPHA_22    _INTER_ALPHA // ecto-ecto

static std::vector<real>
getNElementsVector(const real& V1, const real& V2)
{
  std::vector<real> vec(TYPES_NO, V2);
  vec[0] = V1;
  return vec;
}

static std::vector<std::vector<real>>
getNElementsMatrix(const real& V11, const real& V22, const real& V12, const real& V21)
{
  std::vector<std::vector<real>> matrix(TYPES_NO, std::vector<real>(TYPES_NO, V22));
  if (TYPES_NO > 1)
  {
    matrix[0][0] = V11;
    matrix[0][1] = V12;
    matrix[1][0] = V21;
  }
  return matrix;
}

static std::vector<std::string>
getColours()
{
  const std::string s = "unset";
  std::vector<std::string> v (TYPES_NO, s);
  return v;
}

static std::vector<real>
getTargetAreas(void)
{
  std::vector<real> v(TYPES_NO);
  for (type_int t = 0; t < TYPES_NO; ++t)
    v[t] = square(RADIAL_REQ[t]) * PI;
  return v;
}

const real RADIAL_SPRING_EXP = 1.0f; //// Deixa-me harmônico. O código tá "otimizado" (não genérico (embora haja código genérico comentado)) para molas lineares.
const std::vector<real> RADIAL_REQ = getNElementsVector(REQ_1R, REQ_2R);

const std::vector<std::vector<real>> INTER_REQ   = getNElementsMatrix(REQ_1I1, REQ_2I2, REQ_1I2, REQ_2I1);

const std::vector<real> RADIAL_BETA              = getNElementsVector(BETA_1R, BETA_2R);

const std::vector<std::vector<real>> INTER_BETA   = getNElementsMatrix(BETA_1I1, BETA_2I2, BETA_1I2, BETA_2I1);

const std::vector<real> KAPA = getNElementsVector(KAPA_1, KAPA_2);

const std::vector<std::vector<real>> INTER_ALPHA = getNElementsMatrix(INTER_ALPHA_11, INTER_ALPHA_22, INTER_ALPHA_12, INTER_ALPHA_21);
const std::vector<real> AUTO_ALPHA = getNElementsVector(AUTO_ALPHA_1, AUTO_ALPHA_2);

const std::vector<real> VELOCITY = getNElementsVector(V0, V0);

const std::vector<std::string> COLOURS = getColours();
//{"red", "blue", "dark-green", "magenta", "green", "black", "orange", "purple", "dark-blue", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""}; // GNUplot rgb color names.
const uint16_t PLOT_SIZE = 1080U; // GNUplot image size (1-D).

const real    HALF_PI = 1.57079633f;
const real         PI = 3.14159265359f;
const real     TWO_PI = 6.283185307179586f;
const dimension_int X = 0u;
const dimension_int Y = 1u;
const dimension_int Z = 2u;
const dimension_int DIMENSIONS = 2u;


const unsigned int DATE_SIZE = 48u;
#define THREADS_MAX_NO 16u      // Threads maximum quantity.
const thread_int   THREADS   = SUPERBOIDS > THREADS_MAX_NO ? THREADS_MAX_NO : SUPERBOIDS; // Threads quantity.

const std::vector<real> TARGET_AREA = getTargetAreas();


template <typename T>
static void
printMatrix(std::ostringstream& st, const T matrix[9u][9u])
{
  for (type_int line = 0u; line < TYPES_NO; ++line)
  {
    st << '#';
    for (type_int column = 0u; column < TYPES_NO; ++column)
      st << '\t' << matrix[line][column];
    st << std::endl;
  }
  
  return;
}

template <typename T>
static void
printMatrix(std::ostringstream& st, const std::vector<std::vector<T>>& matrix)
{
  for (type_int line = 0u; line < matrix.size(); ++line)
  {
    st << '#';
    for (type_int column = 0u; column < matrix[line].size(); ++column)
      st << '\t' << matrix[line][column];
    st << std::endl;
  }
  
  return;
}

template<typename S, typename T>
static void
printVector(S& st, const std::vector<T>& matrix)
{
  st << '#';
  const std::size_t SIZE  = matrix.size();
  for (type_int column = 0u; column < SIZE; ++column)
    st << '\t' << matrix[column];
  st << std::endl;
  
  return;
}

template <typename S, typename T>
static void
printVector(S& st, const T matrix[9u])
{
  st << '#';
  for (type_int column = 0u; column < TYPES_NO; ++column)
    st << '\t' << matrix[column];
  st << std::endl;
  
  return;
}

std::string&
getParameters(void)
{
  static bool firstTime = true;
  static std::string parameters;
  if (firstTime)
  {
    firstTime = false;
    std::ostringstream stream;
    stream << "# Compiled at" << '\t' << Date::compiledTime << std::endl;
    stream << "# Runned at"   << '\t' << Date::prettyRunTime << std::endl;
    stream << "# Superboids"  << "\t\t" << SUPERBOIDS << std::endl;
    stream << "# Miniboids per superboid" << "\t" << MINIBOIDS_PER_SUPERBOID << std::endl;
    stream << "# Boxes in edge"     << "\t\t" << BOXES_IN_EDGE << std::endl;
    stream << "# Boxes"           << "\t\t\t" << BOXES << std::endl;
    stream << "# Range (domain size)" << "\t" << RANGE << std::endl;

    stream << "# Neighbor distance"   << "\t" << NEIGHBOR_DISTANCE << std::endl;
    stream << "# Inter-cell elastic up limit" << "\t" << INTER_ELASTIC_UP_LIMIT << std::endl;
    stream << "# Radial plastic begin limit"     << std::endl;
    printVector(stream, RADIAL_PLASTIC_BEGIN);
    stream << std::endl;
    stream << "# Radial plastic end limit"     << std::endl;
    printVector(stream, RADIAL_PLASTIC_END);
    stream << std::endl;
    stream << "# Initial distance"  << "\t\t" << INITIAL_DISTANCE << std::endl;
    stream << "# Initial angle ID 1"  << "\t" << INITIAL_ANGLE_ID1 << std::endl;
    stream << "# Core diameter"     << "\t\t" << CORE_DIAMETER << std::endl;
    stream << "# \"Infinite\" force"  << "\t" << INFINITE_FORCE << std::endl;

    stream << std::endl;
    std::string bc;
    if (BC == BoundaryCondition::PERIODIC)
      bc = "PERIODIC";
    else if (BC == BoundaryCondition::RECTANGLE)
      bc = "RECTANGLE";
    else if (BC == BoundaryCondition::STOKES)
      bc = "STOKES";
    else
      std::cerr << std::endl << "UGLY CACACA" << std::endl << std::endl;
    stream << "# BC"       << "\t\t\t" << bc << std::endl;
    stream << "# RECTANGLE:" << std::endl;
    printVector(stream, RECTANGLE_SIZE);
    stream << std::endl;
    
    stream << "# STEPS"       << "\t\t\t" << STEPS << std::endl;
    stream << "# EXIT_INTERVAL" << "\t\t" << EXIT_INTERVAL << std::endl;
    stream << "# EXIT_FACTOR" << "\t\t" << EXIT_FACTOR << std::endl;

    stream << "# Types no." << "\t\t" << TYPES_NO << std::endl;
    stream << std::endl << "# Proportions" << std::endl;
    printVector(stream, PROPORTIONS);
    stream << std::endl;

    stream << std::endl << "# Radial spring exp.:" << RADIAL_SPRING_EXP << std::endl;
    stream << "# Radial R_Eq:" << std::endl;
    printVector(stream, RADIAL_REQ);
    stream << std::endl;

    stream << std::endl << "# Radial beta:" << std::endl;
    printVector(stream, RADIAL_BETA);
    stream << std::endl;

    stream << std::endl << "# Inter R_Eq:" << std::endl;
    printMatrix(stream, INTER_REQ);
    stream << std::endl;

    stream << std::endl << "#Inter Beta: " << std::endl;
    printMatrix(stream, INTER_BETA);
    stream << std::endl;

    stream << std::endl << "# Inter Alpha:" << std::endl;
    printMatrix(stream, INTER_ALPHA);
    stream << std::endl;

    stream << std::endl << "# Auto Alpha:" << std::endl;
    printVector(stream, AUTO_ALPHA);
    stream << std::endl;

    stream << std::endl << "# Kapa:" << std::endl;
    printVector(stream, KAPA);
    stream << std::endl;

    stream << std::endl << "# Velocity:" << std::endl;
    printVector(stream, VELOCITY);
    stream << std::endl;

    stream << std::endl << "# Colors:" << std::endl;
    printVector(stream, COLOURS);
    stream << std::endl;

    stream << "# ETA" << "\t\t\t" << ETA << std::endl;
    stream << "# DT" << "\t\t\t" << DT << std::endl;
    stream << "# NEIGHBOR_DISTANCE" << "\t" << NEIGHBOR_DISTANCE << std::endl;

    stream << "# THREADS"       << "\t\t" << THREADS << std::endl;

    parameters = stream.str();
  }
  return parameters;
}
