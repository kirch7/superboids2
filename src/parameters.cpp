// Copyright (C) 2016-2018 Cássio Kirch.
// License specified in LICENSE file.

#include <iostream> ////
#include <sstream>
#include "Stokes.hpp"
#include "Date.hpp"
#include "Parameter.hpp"
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

// static real
// getMaxValue(const std::vector<std::vector<real>>& matrix)
// {
//   if (matrix.size() == 0)
//     return -0.0f;
  
//   real max = -1.0e10f;
//   for (const auto& v : matrix)
//     for (const auto elem : v)
//       if (elem > max)
// 	max = elem;

//   return max;
// }


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
getTargetAreas(const std::vector<real>& reqs)
{
  std::vector<real> v(reqs.size());
  for (type_int t = 0; t < v.size(); ++t)
    v[t] = square(reqs[t]) * PI;
  return v;
}

static std::vector<real>
normalize(const std::vector<real>& v)
{
  std::vector<real> vec = v;
  real sum = -0.0f;
  for (const auto& i : vec)
    sum += i;
  for (auto& i : vec)
    i /= sum;

  sum = -0.0f;
  for (const auto& i : vec)
    sum += i;
  
  if (sum < 1.0f)
    vec[0] += (1.0f - sum);
  
  return vec;
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
    stream << "# Harris amount" << "\t\t" << p.HARRIS_AMOUNT << std::endl;
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
    stream << "# DIVISION_REGION_X" << p.DIVISION_REGION_X << std::endl;
    stream << std::endl;
    
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

    stream << std::endl << "# Radial beta medium:" << std::endl;
    printVector(stream, p.RADIAL_BETA_MEDIUM);
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
    printMatrix(stream, p.KAPA);
    stream << std::endl;

    stream << std::endl << "# Kapa medium:" << std::endl;
    printVector(stream, p.KAPA_MEDIUM);
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

static box_int
getBoxesInEdge()
{
  const box_int b = getParameter<real>("domain") / getParameter<real>("neighbor_distance");
  if (b < 3)
    return 3;
  else
    return b;
}

static void panic(const std::string& m) __attribute__((noreturn));

static void
panic(const std::string& m)
{
  std::cerr << m << std::endl;
  std::exit(2);
}

static void panic(const std::string& m, const real value) __attribute__((noreturn));

static void
panic(const std::string& m, const real value) {
  std::cerr << m << std::endl;
  std::cerr << value << " was found" << std::endl;
  std::exit(2);
}

void
Parameters::set1(void)
{
  this->REAL_TOLERANCE = getParameter<real>("real_tolerance");
  if (this->REAL_TOLERANCE <= 0.0)
    panic("real_tolerance is supposed to be positive.", this->REAL_TOLERANCE);
  else if (this->REAL_TOLERANCE > 1.0e-3)
    panic("real_tolerance is to big", this->REAL_TOLERANCE);

  this->DIMENSIONS = getParameter<unsigned long int>("dimensions");;
  if (this->DIMENSIONS != 2)
    panic("only 2D for now");

  // Last step number:
  this->STEPS         = getParameter<unsigned long int>("steps");
  // Initial interval between outputs to files. In case EXIT_FACTOR=0, this interval is homogeneous:
  this->EXIT_INTERVAL = getParameter<unsigned long int>("exit_interval");
  this->EXIT_FACTOR   = getParameter<real>("exit_factor");;
  if (this->EXIT_FACTOR < 0.0f)
    panic("exit_factor must be positive or nule.", this->EXIT_FACTOR);

  // Cell-center-to-cell-center distance in STEP=0 (initial condition):
  this->INITIAL_DISTANCE        = getParameter<real>("initial_distance");
  if (this->INITIAL_DISTANCE <= this->REAL_TOLERANCE)
    panic("initial_distance too short", this->INITIAL_DISTANCE);
  else if (this->INITIAL_DISTANCE > this->RANGE)
    panic("initial_distance too big", this->INITIAL_DISTANCE);
  
  this->INITIAL_ANGLE_BETWEEN   = HALF_PI; // rad  // Nowadays it is unused. It was used in twist spring tests.
  this->INITIAL_ANGLE_ID1       = 0.0f;    // rad  // Nowadays it is unused. It was used in twist spring tests.
  this->TWIST_EQ_ANGLE          = 2.0f;    // rad  // Nowadays it is unused. It was used in twist spring tests.
  // "Infinite" force maximum reach distance:
  this->CORE_DIAMETER           = getParameter<real>("core_diameter");
  if (this->CORE_DIAMETER < 0.0)
    panic("core_diameter must be positive or nule", this->CORE_DIAMETER);
  
  // Unit to GNUplot use in circles plotting:
  this->PRINT_CORE              = getParameter<real>("print_core");
  if (this->PRINT_CORE <= 0.0)
    panic("print_core must be positive", this->CORE_DIAMETER);
  // Simple "Infinite" magnitude:
  this->INFINITE_FORCE          = getParameter<real>("core_intensity");
  if (this->INFINITE_FORCE < 0.0)
    panic("infitite force must be positive or nule", this->CORE_DIAMETER);

  // Noise weight:
  this->ETA       = getParameter<real>("eta");
  // Delta time. Time step jump value:
  this->DT        = getParameter<real>("dt");
  if (this->DT <= 0.0)
    panic("dt must be positive", this->CORE_DIAMETER);


  
  return;
}

void
Parameters::setCells(void)
{
  // Cells number:
  this->SUPERBOIDS = getParameter<unsigned long>("cells");
  if (this->SUPERBOIDS < 1)
    panic("it is supposed to some cell to exist", this->SUPERBOIDS);
  
  this->MAX_SUPERBOIDS = getParameter<unsigned long>("max_cells");
  if (this->MAX_SUPERBOIDS < this->SUPERBOIDS)
    panic("you must increase max_cells", this->MAX_SUPERBOIDS);
  
  // Cell types quantity:
  this->TYPES_NO   = getParameter<unsigned long>("types");
  if (this->TYPES_NO < 1)
    panic("it is supposed to some cell type to exist", this->TYPES_NO);

  // Particles in a single cell. All cells have the same particles number:
  this->MINIBOIDS_PER_SUPERBOID = getParameter<unsigned long>("particles_per_cell");
  if (this->MINIBOIDS_PER_SUPERBOID < 4)
    panic("it is supposed to a cell to have at least 4 particles", MINIBOIDS_PER_SUPERBOID);

  return;
}

static BoundaryCondition
getBC(const std::string& b)
{
  if (b == "stokes")
    return BoundaryCondition::STOKES;
  else if (b == "rectangle")
    return BoundaryCondition::RECTANGLE;
  else if (b == "periodic")
    return BoundaryCondition::PERIODIC;
  else
    panic("boundary must be either stokes, rectangle or periodic");
}

static InitialCondition
getIC(const std::string& ic)
{
  if (ic == "left")
    return InitialCondition::LEFT_EDGE;
  else if (ic == "hex_center")
    return InitialCondition::HEX_CENTER;
  else
    panic("initial must be either left or hex_center");
}

static KillCondition
getKC(const std::string& kc)
{
  if (kc == "right")
    return KillCondition::RIGHT_EDGE;
  else if (kc == "none")
    return KillCondition::NONE;
  else if (kc == "p0")
    return KillCondition::P0;
  else if (kc == "right_edge_or_p0")
    return KillCondition::RIGHT_EDGE_OR_P0;
  else
    panic("kill must be either right, none, right_edge_or_p0, or p0");
}

void
Parameters::setDomain(void)
{
  // Domain length:
  this->RANGE = getParameter<real>("domain");
  if (this->RANGE <= 0.0f)
    panic("domain is supposed to be positive", this->RANGE);

  this->RECTANGLE_SIZE = getParameter<std::vector<real>>("rectangle");
  for (const auto& comp : this->RECTANGLE_SIZE)
  {
    if (comp <= 0.0)
      panic("rectangle length supposed to be positive", comp);
    else if (comp > this->RANGE)
      panic("rectangle length supposed to smaller than domain", comp);
  }
  
  // Intercell forces maximum reach distance:
  this->NEIGHBOR_DISTANCE       = getParameter<real>("neighbor_distance");
  if (this->NEIGHBOR_DISTANCE <= 0.0)
    panic("neighbor_distance supposed to be positive", this->NEIGHBOR_DISTANCE);
  else if (this->NEIGHBOR_DISTANCE < this->REAL_TOLERANCE)
    panic("neighbor_distance supposed to be bigger", this->NEIGHBOR_DISTANCE);

  // sqrt(total boxes quantity):
  this->BOXES_IN_EDGE = getBoxesInEdge();
  // Total boxes quantity:
  this->BOXES = square(this->BOXES_IN_EDGE);

  this->BC = getBC(getParameter<std::string>("boundary"));
  this->INITIAL_CONDITION = getIC(getParameter<std::string>("initial"));
  this->KILL_CONDITION    = getKC(getParameter<std::string>("kill"));
  this->P0_LIMIT          = getParameter<real>("p0_limit");
  if(this->P0_LIMIT < 3.545 )
    panic("p0_limit should be greater than 3.545");
  return;
}

void
Parameters::setDivision(void)
{
  // Every DIVISION_INTERVAL a division will occur:
  this->DIVISION_INTERVAL = getParameter<unsigned long int>("division");
  // A cell can not divide if it is product of a division in the last NON_DIVISION_INTERVAL:
  this->NON_DIVISION_INTERVAL = getParameter<unsigned long int>("non_division");
  // A cell can not divide if its P0 is greater than TOLERABLE_P0:
  this->TOLERABLE_P0 = getParameter<real>("tolerable_p0");
  if (this->TOLERABLE_P0 < 1.0f)
    panic("tolerable_p0 is too small", this->TOLERABLE_P0);
  else if (this->TOLERABLE_P0 >= 5.0f)
    panic("tolerable_p0 is too big", this->TOLERABLE_P0);

  return;
}

void
Parameters::setInter(void)
{
  this->INTER_ELASTIC_UP_LIMIT  = getParameter<real>("domain");
  
  this->INTER_REQ   = getParameter<std::vector<std::vector<real>>>("inter_eq");
  for (const auto& vec : this->INTER_REQ)
    for (const auto& comp : vec)
      if (comp < this->REAL_TOLERANCE)
	panic("inter_eq must be bigger than real_tolerance", comp);
  
  this->INTER_BETA   = getParameter<std::vector<std::vector<real>>>("inter_beta");
  for (const auto& vec : this->INTER_BETA)
    for (const auto& comp : vec)
      if (comp < 0.0f)
	panic("inter_eq must be bigger positive or nule", comp);
  
  return;
}

void
Parameters::setRadial(void)
{
  this->RADIAL_PLASTIC_BEGIN = getParameter<std::vector<real>>("radial_plastic_begin");
  for (const auto comp : this->RADIAL_PLASTIC_BEGIN)
    if (comp < this->REAL_TOLERANCE)
      panic("radial_plastic_begin must be bigger than real_tolerance", comp);
  this->RADIAL_PLASTIC_END = getParameter<std::vector<real>>("radial_plastic_end");
  for (const auto comp : this->RADIAL_PLASTIC_END)
    if (comp < this->REAL_TOLERANCE)
      panic("radial_plastic_begin must be bigger than real_tolerance", comp);

  this->RADIAL_SPRING_EXP = 1.0f; //// Deixa-me harmônico. O código tá "otimizado" (não genérico (embora haja código genérico comentado)) para molas lineares.
  this->RADIAL_REQ = getParameter<std::vector<real>>("radial_eq");
  for (const auto& comp : this->RADIAL_REQ)
    if (comp < this->REAL_TOLERANCE)
      panic("radial_eq must be bigger than real_tolerance", comp);
  
  this->RADIAL_BETA = getParameter<std::vector<std::vector<real>>>("radial_beta");
  for (const auto& vec : this->RADIAL_BETA)
    for (const auto& comp : vec)
      if (comp < 0.0f)
	panic("radial_eq must be bigger positive or nule", comp);

  this->RADIAL_BETA_MEDIUM = getParameter<std::vector<real>>("radial_beta_medium");
  for (const auto& comp : this->RADIAL_BETA_MEDIUM)
    if (comp < this->REAL_TOLERANCE)
      panic("radial_beta_medium must be bigger than real_tolerance", comp);

  this->HARRIS_AMOUNT = getParameter<unsigned long int>("harris_amount");
  if (this->HARRIS_AMOUNT == 0)
    panic("harris_amount must be positive");
  
  return;
}

void
Parameters::setStokes(void)
{
  this->STOKES_HOLES = std::vector<Stokes>();
  const auto& vec = getParameter<std::vector<real>>("stokes");
  if (vec.size() % (this->DIMENSIONS + 1) != 0)
    panic("algorithm problem");
  const std::size_t stokesNo = vec.size() / (this->DIMENSIONS + 1);
  for (std::size_t stokesID = 0; stokesID < stokesNo; ++stokesID)
  {
    std::valarray<real> position(this->DIMENSIONS);
    for (dimension_int dim = 0; dim < this->DIMENSIONS; ++dim)
      position[dim] = vec[stokesID * (this->DIMENSIONS + 1) + 1 + dim];
    this->STOKES_HOLES.emplace_back(position, vec[stokesID * (this->DIMENSIONS + 1)]);
  }

  return;
}

void
Parameters::set(void)
{
  setParameters();

  this->setDomain();
  this->set1();
  this->setCells();
  this->setDivision();
  this->setRadial();
  this->setInter();
  this->setStokes();
  
  // Proportions: array with elements having to sum 1. int(SUPERBOIDS*(nth element)) = (type n cells no).
  this->PROPORTIONS = getParameter<std::vector<real>>("proportions");
  for (const auto& comp : this->PROPORTIONS)
    if (comp < 0.0f)
      panic("proportion component must be positive or nule", comp);
  this->PROPORTIONS = normalize(this->PROPORTIONS);

  this->KAPA = getParameter<std::vector<std::vector<real>>>("kapa");
  this->KAPA_MEDIUM = getParameter<std::vector<real>>("kapa_medium");

  this->INTER_ALPHA = getParameter<std::vector<std::vector<real>>>("inter_alpha");
  this->AUTO_ALPHA = getParameter<std::vector<real>>("auto_alpha");

  this->SPEED = getParameter<std::vector<real>>("speed");

  
  this->THREADS    = getParameter<unsigned long int>("threads"); // Threads quantity.

  this->TARGET_AREA = getTargetAreas(this->RADIAL_REQ);
};
