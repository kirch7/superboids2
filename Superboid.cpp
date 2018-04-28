// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

////#include <cmath>
#include <random>
#include <vector>
#include <ctime>
#include <iostream> ////
#include <algorithm> // sort
#include "parameters.hpp"
#include "Miniboid.hpp"
#include "Box.hpp"
////#include "SuperboidContainer.hpp"
#include "Superboid.hpp"
#include "export.hpp"
#include "nextstep.hpp"
#include "Stokes.hpp"

unsigned long long int
getSeed(const super_int id)
{
  return std::time(NULL) + id;
}

super_int Superboid::_totalSuperboids(0u);

std::ostream& operator<< (std::ostream& os, const Superboid& super)
{
  for (const auto& mini : super.miniboids)
    os << mini << std::endl;
  os << std::endl;
  return os;
}

static std::vector<super_int>
getVectorWithNoOfSuperboidsPerType(void)
{
  std::vector<super_int> vec(TYPES_NO);
  super_int total = 0u;

  if (TYPES_NO == SUPERBOIDS)                     //// GAMBIARRA
    return   std::vector<super_int>(TYPES_NO, 1); //// GAMBIARRA
  
  for (type_int type = 0u; type < vec.size(); ++type)
  {
    if (PROPORTIONS[type] < 1.0e-6f)
      vec[type] = 0u;
    else
    {
      super_int s = static_cast<super_int>(std::floor(PROPORTIONS[type] * real(SUPERBOIDS)));
      vec[type] = s;
      total += s;
    }
  }
  vec[0u] += SUPERBOIDS - total;
  return vec;
}

static type_int
getType(void)
{
  static const std::vector<super_int> TOTAL = getVectorWithNoOfSuperboidsPerType();
  unsigned long int seed(0u);
  static bool firstTime = true;
  if (firstTime)
  {
    firstTime = false;
    seed = std::time(NULL);
  }

  static std::mt19937 mtEngine(seed);
  static std::uniform_int_distribution<super_int>
    uniform(0u, SUPERBOIDS-1u);
  static std::vector<super_int> already(TYPES_NO);

  while (true)
  {
    super_int intRand = uniform(mtEngine);
    super_int typeSum(0);
    for (type_int type = 0u; type < TYPES_NO; ++type)
    {
      if (PROPORTIONS[type] >= 0.00001f)
      {
        typeSum += std::ceil(PROPORTIONS[type] * real(SUPERBOIDS));
        if (intRand < typeSum)
          if (already[type] < TOTAL[type])
          {
            ++(already[type]);
            return type;
          }
      }
    }
  }
}

static type_int
getType(const super_int id)
{
  static std::default_random_engine defaultEngine(std::time(NULL));
  static std::mt19937 mtEngine(defaultEngine());
  static std::uniform_int_distribution<super_int> uni(0, SUPERBOIDS - 1);
  
  static auto amounts = getVectorWithNoOfSuperboidsPerType();
  static std::vector<bool> alreadySet(SUPERBOIDS, false);
  static std::vector<type_int> types(SUPERBOIDS, 123);
  
  static bool firstTime = true;
  if (firstTime)
  {
    firstTime = false;
    for (type_int t = 0; t < TYPES_NO; ++t)
    {
      super_int supersWithTypeT = 0;
      while (supersWithTypeT < amounts[t])
      {
	super_int chosen = uni(mtEngine);
	if (alreadySet[chosen] == false)
	{
	  alreadySet[chosen] = true;
	  ++supersWithTypeT;
	  types[chosen] = t;
	}
      }
    }
  }

  return types[id];
}

static std::valarray<real>
initialNoise(const real radius)
{
  static std::default_random_engine defaultEngine(std::time(NULL));
  static std::mt19937 mtEngine(defaultEngine());
  std::uniform_real_distribution<real> uniDistribution2pi(0.0, TWO_PI);
  std::uniform_real_distribution<real> uniDistributionRadius(0.0, radius);
  std::valarray<real> _noise(DIMENSIONS);
  if (DIMENSIONS == 2)
  {
    const real r = std::sqrt(uniDistributionRadius(mtEngine));
    const real a = uniDistribution2pi(mtEngine);
    _noise[X] = r * std::cos(a);
    _noise[Y] = r * std::sin(a);
  }
  else {
    std::cerr << "Not yet implemented." << std::endl << std::endl;
  }
  return _noise;
}

static std::valarray<real>
getCentralMiniboidPosition(void)
{
  static super_int superboidCount = 0u;
  static super_int superboidsOnLayer = 1u;
  static uint16_t layerCount = 0u;
  static std::valarray<real> nextPosition (0.0f, DIMENSIONS);
  static real angle = 0.0f;
  static const real DISTANCE = INITIAL_DISTANCE;
  static const real DELTA_ANGLE = PI / 3.0f;

  const std::valarray<real> position(nextPosition + initialNoise(CORE_DIAMETER / 2.0f));
  
  if (superboidCount == (superboidsOnLayer - 1u))
  {
    angle = 0.0;
    superboidCount = 0u;
    ++layerCount;
    superboidsOnLayer = layerCount * 6u;
    nextPosition[Y] = DISTANCE * (layerCount - 1);
    nextPosition[X] = 0.0;
  }
  else
  {
    if (superboidCount == 0u)
      angle = 2.0 * DELTA_ANGLE;
    else if (superboidCount % layerCount == 0)
      angle += DELTA_ANGLE;
    ++superboidCount;
  }

  nextPosition[Y] += DISTANCE * std::cos(angle);
  nextPosition[X] += DISTANCE * std::sin(angle);
  return position;
}

// Get a peripheral miniboid position considering the central miniboid
// position is its origin.
static std::valarray<real>
getPeripheralMiniboidPosition(const type_int TYPE, const real ANGLE)
{
  //// const real distance = INITIAL_DISTANCE * RADIAL_REQ[TYPE] / 3.0f; //////
  const real distance = RADIAL_REQ[TYPE] * 0.5f;
  
  std::valarray<real> position(DIMENSIONS);
  position[X] = distance * std::cos(ANGLE);
  position[Y] = distance * std::sin(ANGLE);
  position += initialNoise(CORE_DIAMETER / 10.0f);
  
  return position;
}

static real
getDeltaAngle(void)
{
  if (MINIBOIDS_PER_SUPERBOID > 3u)
    return TWO_PI / (MINIBOIDS_PER_SUPERBOID - 1u);
  else
    return INITIAL_ANGLE_BETWEEN;
}

Superboid::Superboid(void):
  activated(false),
  ID(this->_totalSuperboids),
  type(getType(this->_totalSuperboids)),
  area(-0.0f),
  perimeter(-0.0f),
  meanRadius(-0.0f),
  meanRadius2(-0.0f),
  virtualsInfo(std::ios_base::out),
  _randomEngine(/*SuperboidContainer::*/getSeed(ID)),
  lastDivisionStep(0)
{
  this->miniboids.reserve(MINIBOIDS_PER_SUPERBOID);
  this->virtualMiniboids.reserve(64u * MINIBOIDS_PER_SUPERBOID);

  if (this->_totalSuperboids >= SUPERBOIDS)
  {
    for (mini_int miniCount = 0u; miniCount < MINIBOIDS_PER_SUPERBOID; ++miniCount)
      this->miniboids.emplace_back(miniCount, *this);
  }
  else
  {
    if (INITIAL_CONDITION == InitialCondition::HEX_CENTER)
    {
      for (mini_int miniCount = 0u; miniCount < MINIBOIDS_PER_SUPERBOID; ++miniCount)
      {
	static const real DELTA_ANGLE = getDeltaAngle();
	this->miniboids.emplace_back(miniCount, *this);
	if (miniCount == 0u)
	{
	  bool ready = false;
	  while (!ready)
	  {
	    this->miniboids[0u].position = getCentralMiniboidPosition();
	    ready = true;
	    for (dimension_int dim = 0u; ready && dim < DIMENSIONS; ++dim)
	    {
	      const real comp = this->miniboids[0u].position[dim];
	      const real HALF_RANGE = RANGE / 2;
	      if (/*std::fabs(HALF_RANGE - std::fabs(comp)) <= INITIAL_DISTANCE / 2 ||*/
		HALF_RANGE - std::fabs(comp) < INITIAL_DISTANCE / 2.0f - 0.1f)
		ready = false;
	      if (BC == BoundaryCondition::RECTANGLE || BC == BoundaryCondition::STOKES)
		if (RECTANGLE_SIZE[dim] / 2.0f - std::fabs(comp) < INITIAL_DISTANCE / 2.0f - 0.1f)
		  ready = false;
	    }

	    if (BC == BoundaryCondition::STOKES)
	      for (const auto& hole : STOKES_HOLES)
	      {
		const Distance d(hole.center, this->miniboids[0u].position);
		if (d.module < hole.radius + INITIAL_DISTANCE / 2.0f - 0.1f)
		  ready = false;
	      }
	  }
	}
	else
	  this->miniboids[miniCount].position = this->miniboids[0u].position + \
	    getPeripheralMiniboidPosition(this->type, DELTA_ANGLE * (miniCount - 1u) + INITIAL_ANGLE_ID1);
    
      }
    }
    else if (INITIAL_CONDITION == InitialCondition::LEFT_EDGE)
    {
      static const super_int AMOUNT_IN_A_COLUMN = RECTANGLE_SIZE[Y] / INITIAL_DISTANCE;

      this->miniboids.emplace_back(0u, *this);
      std::valarray<real>& centralPosition = this->miniboids[0u].position;
      const super_int COLUMN = static_cast<super_int>(this->ID / AMOUNT_IN_A_COLUMN);
      const super_int ROW    = static_cast<super_int>(this->ID % AMOUNT_IN_A_COLUMN);
      centralPosition[X] = (COLUMN + 0.5f) * INITIAL_DISTANCE - RECTANGLE_SIZE[X] / 2.0f;
      centralPosition[Y] = (ROW    + 0.5f) * INITIAL_DISTANCE - RECTANGLE_SIZE[Y] / 2.0f;
      if (DIMENSIONS != 2u)
	std::cerr << "Unimplemented!" << std::endl;
      
      for (mini_int miniCount = 1u; miniCount < MINIBOIDS_PER_SUPERBOID; ++miniCount)
      {
	this->miniboids.emplace_back(miniCount, *this);
	this->miniboids[miniCount].position = this->miniboids[0u].position + \
	  getPeripheralMiniboidPosition(this->type, getDeltaAngle() * (miniCount - 1u) + INITIAL_ANGLE_ID1);
      }
    }
    else
    {
      std::cerr << "Unimplemented!" << std::endl;
    }
  }

  ++(this -> _totalSuperboids);

  return;
}

real
Superboid::get0to2piRandom(void)
{
  static std::uniform_real_distribution<real> uniDistribution(0.0f, TWO_PI);
  return uniDistribution(this->_randomEngine);
}

void
Superboid::setGamma(std::vector<Superboid>& superboids)
{
  super_int sameTypeNeighborsNo = 0u;
  super_int anotherTypeNeighborsNo = 0u;
  
  for (const auto& superNeighborID : this->cellNeighbors())
  {
    if (this->type == superboids[superNeighborID].type)
    {
      ++sameTypeNeighborsNo;
      //// std::cerr << "Same\t"  << this->ID << '\t' << superNeighborID << '\t' << this->type << '\t' << superboids[superNeighborID].type << std::endl;
    }
    else
    {
      ++anotherTypeNeighborsNo;
      //// std::cerr << "ñSame\t"  << this->ID << '\t' << superNeighborID << '\t' << this->type << '\t' << superboids[superNeighborID].type << std::endl;
    }
  }
  
  if (sameTypeNeighborsNo + anotherTypeNeighborsNo != 0)
  {
    this->gamma = static_cast<real>(anotherTypeNeighborsNo) / (anotherTypeNeighborsNo + sameTypeNeighborsNo);
    this->doUseGamma = true;
  }
  else
  {
    this->gamma = -0.0f;
    this->doUseGamma = false;
  }

  //// std::cerr << "###### " << this->ID << '\t' << this->gamma << std::endl;
  
  return;
}

void
Superboid::setShape(const step_int STEP)
{
  if (STEP <= this->_shapeStep && STEP != 0)
    return;

  this->_shapeStep = STEP;
  this->area        = -0.0f;
  this->perimeter   = -0.0f;
  this->meanRadius  = -0.0f;
  this->meanRadius2 = -0.0f;
  
  for (const auto& mini1 : this->miniboids)
  {
    if (mini1.ID == 0u)
      // If central miniboid, then continue loop in the next miniboid.
      continue;
    else if (mini1.ID == MINIBOIDS_PER_SUPERBOID - 1u) // Last miniboid.
    {
      this->perimeter += Distance(mini1, this->miniboids[1u]).module;
      this->area += mini1.getAreaBetween(this->miniboids[1u]);
    }
    else // Another peripheral but the last.
    {
      this->perimeter += Distance(mini1, this->miniboids[mini1.ID + 1u]).module;
      this->area += mini1.getAreaBetween(this->miniboids[mini1.ID + 1u]);
    }

    real radialDistanceModule = Distance(mini1, this->miniboids[0]).module;
    this->meanRadius  += radialDistanceModule;
    this->meanRadius2 += square(radialDistanceModule);
  }
  
  return;
}

void
Superboid::checkVirtual(const bool export_)
{
  //this->virtualMiniboids.clear();
  const real _maxDistance = 2.0f * RADIAL_REQ[this->type] *			\
    std::sqrt(2.0f * (1.0f - std::cos(2.0*PI / (MINIBOIDS_PER_SUPERBOID - 1u))));
  //Miniboid* thisMini = nullptr;
  if (export_)
    this->virtualsInfo.str(std::string(""));
  for (auto& mini1 : this->miniboids)
  {
    if (mini1.ID == 0u)
      continue;
    std::vector<real> vec;
    for (auto& tn : mini1._twistNeighbors)
      if (tn._distance.module > CORE_DIAMETER)
	vec.emplace_back(tn._distance.module);
    vec.emplace_back(_maxDistance);
    std::sort(vec.begin(), vec.end());
    const real maxDistance = vec.front();
    const Distance& dist = mini1._twistNeighbors.front()._distance;
    // mini1.rightN = thisMini;
    // if (thisMini)
    //   thisMini->leftN = &mini1;
    // thisMini = &mini1;
    if (dist.module > maxDistance)
    {
      const mini_int VIRTUAL_NO = static_cast<mini_int>(std::floor(dist.module / maxDistance));
      for (mini_int virtID = 0; virtID < VIRTUAL_NO; ++virtID)
      {
	const mini_int newID = this->virtualMiniboids.size();
	this->virtualMiniboids.emplace_back(newID, *this, true);
	Miniboid& virtualMini = this->virtualMiniboids[newID];
	std::valarray<real> differenceVector = dist.getDirectionArray();
	differenceVector *= (virtID + 1u) * (dist.module / (VIRTUAL_NO + 1));
	virtualMini.position = mini1.position + differenceVector;
	if (export_)
	  this->virtualsInfo << virtualMini.position << '\t' << this->type << std::endl;
	virtualMini.checkLimits();
	virtualMini.reset();
	//thisMini->leftN = &virtualMini;
	//virtualMini.rightN = thisMini;
	//thisMini = &virtualMini;
      }
    }
  }
  //this->miniboids[1].rightN = thisMini;
  //thisMini->leftN = &this->miniboids[1];
  
  return;
}

void
Superboid::setNextPosition(const step_int step)
{
  this->setShape(step);
  for (auto& mini : this->miniboids)
    mini.setNextPosition();
  
  return;
}

static std::vector<std::valarray<real>>
getOriginalPositions(const std::vector<Miniboid>& miniboids)
{
  std::vector<std::valarray<real>> v;
  v.reserve(miniboids.size());
  
  for (const auto& mini : miniboids)
    v.push_back(mini.position);

  return v;
}

static void
setOriginalPositions(std::vector<Miniboid>& miniboids, const std::vector<std::valarray<real>>& original)
{
  for (auto& mini : miniboids)
    mini.position = original[mini.ID];
  
  return;
}

static void
rearrangePeripherals(Superboid& superboid, const real distance)
{
  superboid.miniboids[0].checkLimits();
  std::vector<Distance> distances;
  
  /*for (const auto& mini : superboid.miniboids)
  {
    if (mini.ID == 0u)
      distances.emplace_back();
    else
      distances.emplace_back(mini, superboid.miniboids[0u]);
      }*/

  // Fild smallest distance.
  real smallest = RANGE;
  /*for (mini_int miniID = 1u; miniID < MINIBOIDS_PER_SUPERBOID; ++miniID)
    if (distances[miniID].module < smallest)
      smallest = distances[miniID].module;

  smallest /= 2.0f; ////
  */

  smallest = distance + REAL_TOLERANCE;
  for (mini_int miniID = 1u; miniID < MINIBOIDS_PER_SUPERBOID; ++miniID)
  {
    //std::valarray<real> dist = distances[miniID].getDirectionArray();
    const real angle = miniID * TWO_PI / (MINIBOIDS_PER_SUPERBOID - 1);
    std::valarray<real> dist({std::cos(angle), std::sin(angle)});
    dist *= smallest;
    superboid.miniboids[miniID].position = superboid.miniboids[0u].position + dist;
    superboid.miniboids[miniID].checkLimits();
  }
  
  return;
}

bool
Superboid::divide(const super_int divide_by, Superboid& newSuperboid, std::vector<Box>& boxes, const step_int step)
{
  if (divide_by < 2u)
  {
    std::cerr << "Cannot divide by n | n < 2" << std::endl;
    return false;
  }
  if (divide_by != 2u)
  {
    std::cerr << "Unimplemented." << std::endl;
    return false;
  }

  this->setShape(step);
  const real p0 = this->perimeter / std::sqrt(this->area);
  if (p0 > TOLERABLE_P0)
  {
    std::cerr << "p0:\t" << p0 << std::endl;
    return false;
  }
    
  static std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(0, TYPES_NO - 1);
  const type_int newType = distribution(generator);
  
  *const_cast<type_int*>(&(newSuperboid.type)) = newType;

  real divisionAngle = this->get0to2piRandom();

  const std::vector<std::valarray<real>> originalPositions = getOriginalPositions(this->miniboids);
  newSuperboid.activated = true;
  for (mini_int miniID = 0u; miniID < MINIBOIDS_PER_SUPERBOID; ++miniID)
  {
    newSuperboid.miniboids[miniID].position = this->miniboids[miniID].position;
    newSuperboid.miniboids[miniID].velocity = this->miniboids[miniID].velocity;
    newSuperboid.miniboids[miniID].newVelocity = this->miniboids[miniID].newVelocity;

    const box_int newBoxID = Box::getBoxID(newSuperboid.miniboids[miniID].position);
    boxes[newBoxID].append(newSuperboid.miniboids[miniID]);
  }

  step_int atempts = 0;
  while (true)
  {
    ++atempts;
    setOriginalPositions(this->miniboids, originalPositions);

    if (atempts > 16)
      return false;
    
    bool someInvasion = false;

    //const Distance peripheralDistance = this->getBiggestAxis();
    divisionAngle = this->get0to2piRandom();
    std::valarray<real> peripheralVA({std::cos(divisionAngle), std::sin(divisionAngle)}); //// Arranja um nome melhor!
    peripheralVA *= static_cast<real>(10.0f / std::sqrt(atempts)) * CORE_DIAMETER;
    //const Distance peripheralDistance = Distance(peripheralVA);
    //const Miniboid& peripheral1 = *peripheralDistance.miniboid1;
    //const Miniboid& peripheral2 = *peripheralDistance.miniboid2;

    //const std::valarray<real> peripheralDistanceArray = peripheralDistance.getDirectionArray() * peripheralDistance.module;
    //const std::valarray<real> differenceArray = static_cast<real>(0.333333f) * peripheralDistanceArray;
    const std::valarray<real> differenceArray = peripheralVA;
  
    this->miniboids[0u].position = this->miniboids[0u].position + differenceArray;
    newSuperboid.miniboids[0u].position = this->miniboids[0u].position - differenceArray;
    // this->miniboids[0u].position = peripheral1.position + differenceArray;
    // newSuperboid.miniboids[0u].position = peripheral2.position - differenceArray;

    const real radius = 3.0f * CORE_DIAMETER / std::sqrt(atempts);
    rearrangePeripherals(*this, radius);
    rearrangePeripherals(newSuperboid, radius);

    std::vector<Superboid*> twoSupers({this, &newSuperboid});
    bool insideBox = true;
    for (auto superPtr : twoSupers)
      if (insideBox)
	for (const auto& mini : superPtr->miniboids)
	  if (insideBox)
	    for (dimension_int dim = 0u; dim < DIMENSIONS; ++dim)
	    {
	      const real HALF_RECTANGLE_SIZE = 0.5f * RECTANGLE_SIZE[dim];
	      if (mini.position[dim] < -HALF_RECTANGLE_SIZE)
		insideBox = false;
	    }
    if (insideBox == false)
      continue;
    
    nextBoxes(boxes, *this);
    nextBoxes(boxes, newSuperboid);
    
    for (auto super : twoSupers)
    {
      for (auto& mini : super->miniboids)
      {
	if (!someInvasion)
	  for (const auto& hole : STOKES_HOLES)
	    if (hole.isInside(mini.position))
	    {
	      someInvasion = true;
	      break;
	    }

	if (!someInvasion)
	{
	  mini.setNeighbors();
	  for (const auto& list : mini._neighbors)
	    if (mini.fatInteractions(0, list, false))
	    {
	      someInvasion = true;
	      break;
	    }
	}
      }
    }
    if (!someInvasion)
      break;
  }
  
  this->lastDivisionStep = step;
  newSuperboid.lastDivisionStep = step;
    
  this->reset();
  newSuperboid.reset();
  
  this->virtualMiniboids.clear();
  newSuperboid.virtualMiniboids.clear();

  return true;
}

Distance
Superboid::getBiggestAxis() const
{
  std::vector<Distance> distances;
  const mini_int peripheralNo = MINIBOIDS_PER_SUPERBOID - 1;
  for (mini_int miniID1 = 1u;
       miniID1 < MINIBOIDS_PER_SUPERBOID;
       ++miniID1)
  {
    std::vector<mini_int> miniIDs2;
    mini_int opposite1 = miniID1;
    opposite1 += peripheralNo / 2;
    if (MINIBOIDS_PER_SUPERBOID % 2 == 0)
    {
      miniIDs2.emplace_back(opposite1 - 1);
      miniIDs2.emplace_back(opposite1);
      miniIDs2.emplace_back(opposite1 + 1);
      miniIDs2.emplace_back(opposite1 + 2);
    }
    else // MINIBOIDS_PER_SUPERBOID % 2 == 1
    {
      miniIDs2.emplace_back(opposite1 - 1);
      miniIDs2.emplace_back(opposite1);
      miniIDs2.emplace_back(opposite1 + 1);
    }
    
    for (auto& miniID2 : miniIDs2) // Reference.
    {
      if (miniID2 >= MINIBOIDS_PER_SUPERBOID)
	miniID2 -= peripheralNo;
    }
    for (auto miniID2 : miniIDs2)  // Copy.
      if (miniID2 > miniID1)
	distances
	  .emplace_back(this->miniboids[miniID1],
			this->miniboids[miniID2]);
  }

  std::sort(distances.begin(), distances.end());
  
  return distances.back();
}

void
Superboid::checkWrongNeighbors(const std::vector<Superboid>& superboids)
{
  const std::list<super_int> neighbors = this->cellNeighbors(); // Value, not referece.
  for (const auto cellID1 : neighbors)
  {
    for (const auto cellID2 : neighbors)
    {
      if (cellID1 == cellID2)
	continue;
      if (this->ID == cellID1)
	continue;
      if (this->ID == cellID2)
	continue;
      
      const Superboid& super1 = superboids[cellID1];
      const Superboid& super2 = superboids[cellID2];
      
      const Distance dist(this->miniboids[0u], super2.miniboids[0u]);
      const Distance halfDist = dist * 0.5f;

      //// checar BC periódica depois!!!!!!!!!!!!!!!!!!!!
      if (isPointInSomeTriangle(this->miniboids[0u].position + (halfDist.getDirectionArray() * halfDist.module), super1))
      {
	this->cellNeighbors.remove(cellID2);
	for (auto& mini : this->miniboids)
	  for (auto& cell : mini._neighbors)
	  {
	    if (!cell.empty())
	    {
	      const super_int superID = cell
		.front()
		.miniNeighbor
		.superboid
		.ID;
	      if (superID == super2.ID)
		cell.clear();
	    }
	  }
      }
    }
  }
}
