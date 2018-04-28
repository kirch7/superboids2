// Copyright (C) 2016-2018 Cássio Kirch.
// License specified in LICENSE file.

#include <iostream> ////
#include <sstream>
#include "Stokes.hpp"
#include "Date.hpp"
#include "parameters.hpp"

Parameters _p;

const Parameters&
parameters()
{
  return _p;
}
const dimension_int X = 0u;
const dimension_int Y = 1u;
const dimension_int Z = 2u;
const real    HALF_PI = 1.57079633f;
const real         PI = 3.14159265359f;
const real     TWO_PI = 6.283185307179586f;


#define THREADS_MAX_NO 16u      // Threads maximum quantity.
#define         V0          0.007f                 // Speed each one of all particles has in a single time step. For now, there is a single V0 to an entire simulation.

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
#define BETA_1R1     0.02f  //beta_radial endo
#else
  #define BETA_1R1     _RBETA
#endif
#define BETA_2R2     0.20f  //BETA_1R  //beta_radial ecto
#define BETA_1R2     0.40f
#define BETA_2R1     BETA_1R2
// Intercell betas (between two cells PERIPHERAL particles (there is a mechanism for intercell fatboid repulsion)):
#ifdef _IBETA
  #define _BETA_I _IBETA
#else
  #define _BETA_I 0.04f
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
  #define _AUTO_ALPHA     13.0f // 13
  #endif
  #ifndef _INTER_ALPHA
  #define _INTER_ALPHA    13.0f // 13
  #endif
#endif

#define AUTO_ALPHA_1      _AUTO_ALPHA // endo
#define AUTO_ALPHA_2      _AUTO_ALPHA // ecto

#define INTER_ALPHA_11    _INTER_ALPHA // endo-endo
#define INTER_ALPHA_12    _INTER_ALPHA // endo-ecto
#define INTER_ALPHA_21    _INTER_ALPHA // ecto-endo
#define INTER_ALPHA_22    _INTER_ALPHA // ecto-ecto



static real
getMaxValue(const std::vector<std::vector<real>>& matrix)
{
  if (matrix.size() == 0)
    return -0.0f;
  
  real max = -1.0e10f;
  for (const auto& v : matrix)
    for (const auto elem : v)
      if (elem > max)
	max = elem;

  return max;
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

static std::vector<real>
getNElementsVector(const real& V1, const real& V2, const type_int size)
{
  std::vector<real> vec(size, V2);
  vec[0] = V1;
  return vec;
}

static std::vector<std::vector<real>>
getNElementsMatrix(const real& V11, const real& V22, const real& V12, const real& V21, const type_int size)
{
  std::vector<std::vector<real>> matrix(size, std::vector<real>(size, V22));
  if (size > 1)
  {
    matrix[0][0] = V11;
    matrix[0][1] = V12;
    matrix[1][0] = V21;
  }
  return matrix;
}

static std::vector<real>
getTargetAreas(const Parameters& p)
{
  std::vector<real> v(p.TYPES_NO);
  for (type_int t = 0; t < v.size(); ++t)
    v[t] = square(p.RADIAL_REQ[t]) * PI;
  return v;
}

std::string&
getParameters(void)
{
  static bool firstTime = true;
  static std::string s;
  if (firstTime)
  {
    const Parameters& p = parameters();
    firstTime = false;
    std::ostringstream stream;
    stream << "# Compiled at" << '\t' << Date::compiledTime << std::endl;
    stream << "# Runned at"   << '\t' << Date::prettyRunTime << std::endl;
    stream << "# Superboids"  << "\t\t" << p.SUPERBOIDS << std::endl;
    stream << "# Superboids Max"  << "\t" <<  p.MAX_SUPERBOIDS << std::endl;
    stream << "# Miniboids per superboid" << "\t" << p.MINIBOIDS_PER_SUPERBOID << std::endl;
    stream << "# Boxes in edge"     << "\t\t" << p.BOXES_IN_EDGE << std::endl;
    stream << "# Boxes"           << "\t\t\t" << p.BOXES << std::endl;
    stream << "# Range (domain size)" << "\t" << p.RANGE << std::endl;

    stream << "# Neighbor distance"   << "\t" << p.NEIGHBOR_DISTANCE << std::endl;
    stream << "# Inter-cell elastic up limit" << "\t" << p.INTER_ELASTIC_UP_LIMIT << std::endl;
    stream << "# Radial plastic begin limit"     << std::endl;
    printVector(stream, p.RADIAL_PLASTIC_BEGIN);
    stream << std::endl;
    stream << "# Radial plastic end limit"     << std::endl;
    printVector(stream, p.RADIAL_PLASTIC_END);
    stream << std::endl;
    stream << "# Initial distance"  << "\t\t" << p.INITIAL_DISTANCE << std::endl;
    stream << "# Initial angle ID 1"  << "\t" << p.INITIAL_ANGLE_ID1 << std::endl;
    stream << "# Core diameter"     << "\t\t" << p.CORE_DIAMETER << std::endl;
    stream << "# \"Infinite\" force"  << "\t" << p.INFINITE_FORCE << std::endl;

    stream << std::endl;
    std::string bc;
    if (p.BC == BoundaryCondition::PERIODIC)
      bc = "PERIODIC";
    else if (p.BC == BoundaryCondition::RECTANGLE)
      bc = "RECTANGLE";
    else if (p.BC == BoundaryCondition::STOKES)
      bc = "STOKES";
    else
      std::cerr << std::endl << "UGLY CACACA" << std::endl << std::endl;
    stream << "# BC"       << "\t\t\t" << bc << std::endl;
    stream << "# RECTANGLE:" << std::endl;
    printVector(stream, p.RECTANGLE_SIZE);
    stream << std::endl << std::endl;

    stream << "# Stokes holes:" << std::endl;
    for (const auto& hole : p.STOKES_HOLES)
      stream << "#radius: " << hole.radius << "\tcenter: " << hole.center << std::endl;
    stream << std::endl;
    
    stream << "# STEPS"       << "\t\t\t" << p.STEPS << std::endl;
    stream << "# EXIT_INTERVAL" << "\t\t" << p.EXIT_INTERVAL << std::endl;
    stream << "# EXIT_FACTOR" << "\t\t" << p.EXIT_FACTOR << std::endl;

    stream << "# DIVISION_INTERVAL" << "\t\t" << p.DIVISION_INTERVAL << std::endl;
    stream << "# NON_DIVISION_INTERVAL" << "\t\t" << p.NON_DIVISION_INTERVAL << std::endl;
    stream << "# TOLERABLE_P0" << "\t\t" << p.TOLERABLE_P0 << std::endl;
    
    stream << "# Types no." << "\t\t" << p.TYPES_NO << std::endl;
    stream << std::endl << "# Proportions" << std::endl;
    printVector(stream, p.PROPORTIONS);
    stream << std::endl;

    stream << std::endl << "# Radial spring exp.:" << p.RADIAL_SPRING_EXP << std::endl;
    stream << "# Radial R_Eq:" << std::endl;
    printVector(stream, p.RADIAL_REQ);
    stream << std::endl;

    stream << std::endl << "# Radial beta:" << std::endl;
    printMatrix(stream, p.RADIAL_BETA);
    stream << std::endl;

    stream << std::endl << "# Inter R_Eq:" << std::endl;
    printMatrix(stream, p.INTER_REQ);
    stream << std::endl;

    stream << std::endl << "#Inter Beta: " << std::endl;
    printMatrix(stream, p.INTER_BETA);
    stream << std::endl;

    stream << std::endl << "# Inter Alpha:" << std::endl;
    printMatrix(stream, p.INTER_ALPHA);
    stream << std::endl;

    stream << std::endl << "# Auto Alpha:" << std::endl;
    printVector(stream, p.AUTO_ALPHA);
    stream << std::endl;

    stream << std::endl << "# Kapa:" << std::endl;
    printVector(stream, p.KAPA);
    stream << std::endl;

    stream << std::endl << "# Velocity:" << std::endl;
    printVector(stream, p.SPEED);
    stream << std::endl;

    stream << "# ETA" << "\t\t\t" << p.ETA << std::endl;
    stream << "# DT" << "\t\t\t" << p.DT << std::endl;
    stream << "# NEIGHBOR_DISTANCE" << "\t" << p.NEIGHBOR_DISTANCE << std::endl;

    stream << "# THREADS"       << "\t\t" << p.THREADS << std::endl;

    s= stream.str();
  }
  return s;
}

static std::vector<real>
getUniformProportions(const type_int size)
{
  std::vector<real> vec(size, 1.0f/size);
  real sum = -0.0f;
  for (const auto& i : vec)
    sum += i;
  if (sum < 1.0f)
    vec[0] += (1.0f - sum);
  return vec;
}

Parameters::Parameters()
{
this->REAL_TOLERANCE = 1.0e-9;

this->SUPERBOIDS = 37u;                   // Cells number.
this->MAX_SUPERBOIDS = 4096u;
this->TYPES_NO   = 2u;            // Cell types quantity.
this->MINIBOIDS_PER_SUPERBOID = 12u;      // Particles in a single cell. All cells have the same particles number.

this->BOXES_IN_EDGE = RANGE / NEIGHBOR_DISTANCE;                // sqrt(total boxes quantity).
this->BOXES = BOXES_IN_EDGE*BOXES_IN_EDGE; // Total boxes quantity.
this->RANGE = 50.0f;                      // Domain length.

//this->BC = BoundaryCondition::STOKES;
//this->BC = BoundaryCondition::RECTANGLE;
this->BC = BoundaryCondition::PERIODIC;
//this->INITIAL_CONDITION = InitialCondition::LEFT_EDGE;
this->INITIAL_CONDITION = InitialCondition::HEX_CENTER;
//this->KILL_CONDITION    = KillCondition::RIGHT_EDGE;
this->KILL_CONDITION    = KillCondition::NONE;
//this->RECTANGLE_SIZE = std::vector<real>({ 0.9f * RANGE, 0.5f * RANGE });
this->RECTANGLE_SIZE = std::vector<real>({ 0.5f * RANGE, 0.5f * RANGE });
//this->RECTANGLE_SIZE = std::vector<real>({ 240.0f, 60.0f });

/*this->STOKES_HOLES = std::vector<Stokes>(
  {Stokes(std::valarray<real>(0.0f, DIMENSIONS), 9.0f)}
  );*/
this->STOKES_HOLES = std::vector<Stokes>();

this->DIVISION_INTERVAL = 0u; // Every DIVISION_INTERVAL a division will occur.
this->NON_DIVISION_INTERVAL = 50000u; // A cell can not divide if it is product of a division in the last NON_DIVISION_INTERVAL.
this->TOLERABLE_P0 = 3.9f; // A cell can not divide if its P0 is greater than TOLERABLE_P0.

////this->RADIAL_PLASTIC_END     = {_RADIAL_PLASTIC_END, _RADIAL_PLASTIC_END};
//this->RADIAL_PLASTIC_BEGIN   = {RANGE, RANGE}; //{1.2f, RANGE};
this->RADIAL_PLASTIC_BEGIN = std::vector<real>(TYPES_NO, RANGE);
//this->RADIAL_PLASTIC_END = {1.5f, RANGE};
this->RADIAL_PLASTIC_END = std::vector<real>(TYPES_NO, RANGE); //{1.5f, RANGE};
this->NEIGHBOR_DISTANCE       = _NEIDIST;  // Intercell forces maximum reach distance.
this->INTER_ELASTIC_UP_LIMIT  = _INTER_ELASTIC_UP_LIMIT;   /////////////////////////////////////////////
this->INITIAL_DISTANCE        = 2.0f; //1.8f; //2.75f;      // Cell-center-to-cell-center distance in STEP=0 (initial condition).
this->INITIAL_ANGLE_BETWEEN   = HALF_PI; // rad  // Nowadays it is unused. It was used in twist spring tests.
this->INITIAL_ANGLE_ID1       = 0.0f;    // rad  // Nowadays it is unused. It was used in twist spring tests.
this->TWIST_EQ_ANGLE          = 2.0f;    // rad  // Nowadays it is unused. It was used in twist spring tests.
this->CORE_DIAMETER           = 0.2f;    // "Infinite" force maximum reach distance.
this->PRINT_CORE              = 0.2f; //0.175f; // Unit to GNUplot use in circles plotting.
this->INFINITE_FORCE          = 1000.0f; // Simple "Infinite" magnitude.

this->STEPS         = 500000llu;        // Last step number.
this->EXIT_INTERVAL = 1000llu;           // Initial interval between outputs to files. In case EXIT_FACTOR=0, this interval is homogeneous.
this->EXIT_FACTOR   = 0.0f; //0.8f;

this->ETA       = 1.0f;                  // Noise weight
this->DT        = 1.0f;                  // Delta time. Time step jump value.

// Proportions: array with elements having to sum 1. int(SUPERBOIDS*(nth element)) = (type n cells no).
//this->PROPORTIONS = {0.25f, 0.75f};
this->PROPORTIONS = getUniformProportions(this->TYPES_NO);

this->RADIAL_SPRING_EXP = 1.0f; //// Deixa-me harmônico. O código tá "otimizado" (não genérico (embora haja código genérico comentado)) para molas lineares.
this->RADIAL_REQ = getNElementsVector(REQ_1R, REQ_2R, this->TYPES_NO);

this->INTER_REQ   = getNElementsMatrix(REQ_1I1, REQ_2I2, REQ_1I2, REQ_2I1, this->TYPES_NO);

this->RADIAL_BETA = getNElementsMatrix(BETA_1R1, BETA_2R2, BETA_1R2, BETA_2R1, this->TYPES_NO);

this->INTER_BETA   = getNElementsMatrix(BETA_1I1, BETA_2I2, BETA_1I2, BETA_2I1, this->TYPES_NO);

this->KAPA = getNElementsVector(KAPA_1, KAPA_2, this->TYPES_NO);

this->INTER_ALPHA = getNElementsMatrix(INTER_ALPHA_11, INTER_ALPHA_22, INTER_ALPHA_12, INTER_ALPHA_21, this->TYPES_NO);
this->AUTO_ALPHA = getNElementsVector(AUTO_ALPHA_1, AUTO_ALPHA_2, this->TYPES_NO);

this->SPEED = getNElementsVector(V0, V0, this->TYPES_NO);

this->DIMENSIONS = 2u;


this->THREADS   = SUPERBOIDS > THREADS_MAX_NO ? THREADS_MAX_NO : SUPERBOIDS; // Threads quantity.

this->TARGET_AREA = getTargetAreas(*this);
this->MAX_RADIAL_BETA = getMaxValue(RADIAL_BETA);
};
