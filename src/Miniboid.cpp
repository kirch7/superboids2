// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#include <iostream> // operator<<
#include <list>
#include <tuple>
#include <algorithm>
#include "elastic_plastic.hpp"
#include "Miniboid.hpp"
#include "Box.hpp"
#include "Distance.hpp"
#include "lines.hpp"
#include "Superboid.hpp"
#include "export.hpp"
#include "Stokes.hpp"

std::ostream& operator<<(std::ostream& os, const Miniboid& mini)
{
  const real diameter = (mini.ID == 0u) ? 4.0f * parameters().PRINT_CORE : parameters().PRINT_CORE;
  os << mini.position << diameter << '\t' << mini.superboid.cellNeighbors().size();
  return os;
}

std::valarray<real> Miniboid::getAngles(const mini_int id)
{
  static const real DELTA_ANGLE = (TWO_PI / (parameters().MINIBOIDS_PER_SUPERBOID - 1u));
  std::valarray<real> angles(parameters().DIMENSIONS - 1u);
  if (id != 0u)
    angles[0u] = DELTA_ANGLE * (id - 1u);
  return angles;
}

void
Miniboid::checkLimits(void)
{
  if (parameters().BC == BoundaryCondition::PERIODIC)
    this->checkPeriodicLimits();
  else if (parameters().BC == BoundaryCondition::RECTANGLE || parameters().BC == BoundaryCondition::STOKES)
    this->checkRectangularLimits();

  if (parameters().BC == BoundaryCondition::STOKES && this->isVirtual == false)
    this->checkStokesLimits();
  
  return;
}

void
Miniboid::checkLimits(const step_int step)
{
  this->checkLimits();
  
  if (step != 0)
    this->checkKillCondition(step);
  
  return;
}

static void
checkPeriodicLimits(std::valarray<real>& position)
{
  static const real HALF_RANGE(parameters().RANGE / 2.0f);
  
  for (auto& component : position)
  {
    while (component <= -HALF_RANGE)
      component += parameters().RANGE;
    while (component >   HALF_RANGE)
      component -= parameters().RANGE;
  }
  
  return;
}

void
Miniboid::checkPeriodicLimits()
{
  ::checkPeriodicLimits(this->position);
  
  return;
}

void
Miniboid::checkKillCondition(const step_int step)
{
  if (this->superboid.isActivated() == false || this->superboid.willDie())
    return;
  
  if (parameters().KILL_CONDITION == KillCondition::RIGHT_EDGE || parameters().KILL_CONDITION == KillCondition::RIGHT_EDGE_OR_P0)
    if (this->position[X] > parameters().RECTANGLE_SIZE[X] / 2.0f)
    {
      this->superboid.deathMessage = "right egde";
      this->superboid.setDeactivation();
      std::cerr << "Death (rightEdge) at position " << this->superboid.miniboids[0].position << std::endl;
      return;
    }
  
  if (parameters().KILL_CONDITION == KillCondition::P0 || parameters().KILL_CONDITION == KillCondition::RIGHT_EDGE_OR_P0)
  {
    this->superboid.setShape(step);
    const real P0 = this->superboid.perimeter / std::sqrt(this->superboid.area);
    if (P0 > parameters().P0_LIMIT)
    {
      this->superboid.deathMessage = "P0";
      this->superboid.setDeactivation();
      std::cerr << "Death with p0\t" << P0 << "\tat position " << this->superboid.miniboids[0].position << std::endl;
      return;
    }
  }
  
  return;
}

void
Miniboid::checkRectangularLimits()
{
  static const real THREE_HALFS_RANGE(1.5f * parameters().RANGE);

  for (auto& component : this->position)
  {
    while (component <= -THREE_HALFS_RANGE)
      component += parameters().RANGE;
    while (component >   THREE_HALFS_RANGE)
      component -= parameters().RANGE;
  }
  for (dimension_int dim = 0u; dim < parameters().DIMENSIONS; ++dim)
  {
    const real HALF_RECTANGLE_SIZE = 0.5f * parameters().RECTANGLE_SIZE[dim];
    if (this->position[dim] < -HALF_RECTANGLE_SIZE)
    {
      real delta = this->position[dim] + HALF_RECTANGLE_SIZE;
      if (-2.0f * delta < parameters().DT * parameters().SPEED[this->superboid.type])
        this->position[dim] -= 2.0f * delta;
      else
        this->position[dim] = -HALF_RECTANGLE_SIZE + parameters().REAL_TOLERANCE;
      this->velocity[dim]    *= -1.0f;
      this->newVelocity[dim] *= -1.0f;
    }
    else if (this->position[dim] > HALF_RECTANGLE_SIZE)
    {
      real delta = this->position[dim] - HALF_RECTANGLE_SIZE;
      if (2.0f * delta < parameters().DT * parameters().SPEED[this->superboid.type])
        this->position[dim] -= 2.0f * delta;
      else
        this->position[dim] = HALF_RECTANGLE_SIZE - parameters().REAL_TOLERANCE;
      this->velocity[dim]    *= -1.0f;
      this->newVelocity[dim] *= -1.0f;
    }
  }
  return;
}

void
Miniboid::checkStokesLimits()
{
  for (const auto& hole : parameters().STOKES_HOLES)
  {
    if (this->_box)
    {
      bool skip = true;
      const box_int myBoxID = Box::getBoxID(this->position);
      for (const auto boxID : hole.boxIDs)
      {
        if (boxID == myBoxID)
        {
          skip = false;
          break;
        }
      }
      
      if (skip)
        continue;
    }
      
    const real TOLERABLE = (this->ID == 0u && !this->isVirtual)
      ? (hole.radius + parameters().RADIAL_REQ[this->superboid.type] / 2.0f)
      : hole.radius;
    const Distance d(hole.center, this->position);

    if (d.module < TOLERABLE)
    {
      std::valarray<real> direction = d.getDirectionArray();
      real delta = d.module - TOLERABLE;
      if (2.0f * delta < parameters().DT * parameters().SPEED[this->superboid.type])
        this->position -= direction * (2.0f * delta);
      else
        this->position = hole.center + (direction * TOLERABLE);
    }
  }

  return;
}

void
Miniboid::checkFatOut(void)
{
  if (this->ID != 0 || this->isVirtual)
    return;
  else
    return;
  
  bool isFatOut = true;
  for (mini_int miniID = 2; miniID < parameters().MINIBOIDS_PER_SUPERBOID - 1; ++miniID)
    if (isPointInTriangle(this->position,
                          this->superboid.miniboids[1u].position,
                          this->superboid.miniboids[miniID].position,
                          this->superboid.miniboids[miniID + 1].position))
    {
      isFatOut = false;
      break;
    }

  if (isFatOut)
  {
    std::valarray<real> cm(-0.0f, parameters().DIMENSIONS);
    for (const auto& mini : this->superboid.miniboids)
      if (mini.ID > 1)
      {
        Distance d(mini, this->superboid.miniboids[1]);
        cm += d.module * d.getDirectionArray();
      }
    cm /= static_cast<real>(parameters().MINIBOIDS_PER_SUPERBOID - 2);
    cm = this->superboid.miniboids[1].position - cm;
    ::checkPeriodicLimits(cm);
    this->position = cm;
  }
  
  return;
}

void
Miniboid::noise(void)
{
  real angle = this->superboid.get0to2piRandom();
  this->_noiseSum = std::valarray<real>({std::cos(angle), std::sin(angle)});
  this->_noiseSum *= parameters().ETA;

  return;
}

bool
isPointInTriangle(const std::valarray<real>& p_test, const std::valarray<real>& p0, const std::valarray<real>& p1, const std::valarray<real>& p2)
{
  const Distance d(p_test, p0);
  const real dX = d.module * d.cosine;
  const real dY = d.module * d.sine;

  const Distance d20(p2, p0);
  const real dX20 = d20.module * d20.cosine;
  const real dY20 = d20.module * d20.sine;

  const Distance d10(p1, p0);
  const real dX10 = d10.module * d10.cosine;
  const real dY10 = d10.module * d10.sine;
  
  const real s_p = (dY20*dX) - (dX20*dY);
  const real t_p = (dX10*dY) - (dY10*dX);
  const real D = (dX10*dY20) - (dY10*dX20);

  if (D > 0.0f)
    return ( (s_p >= 0.0f) && (t_p >= 0.0f) && (s_p + t_p) <= D );
  else
    return ( (s_p <= 0.0f) && (t_p <= 0.0f) && (s_p + t_p) >= D );
}

bool
isPointInSomeNthTriangle(const mini_int nth, const std::valarray<real>& point, const Superboid& super)
{
  const Miniboid& fatboid = super.miniboids[0u];
  bool inSomeTriangle = false;
  const Miniboid* auxMini = &super.miniboids.back();

  for (auto& realMini : super.miniboids)
  {
    if (realMini.ID == 0)
      continue;
    if (inSomeTriangle)
      break;
    mini_int nextID = realMini.ID % (parameters().MINIBOIDS_PER_SUPERBOID - 1) + nth;
    nextID %= parameters().MINIBOIDS_PER_SUPERBOID - 1;
    auxMini = &super.miniboids[nextID];
    inSomeTriangle = isPointInTriangle(point, fatboid.position, auxMini->position, realMini.position);
  }

  return inSomeTriangle;
}

bool
Miniboid::isInSomeNthTriangle(const mini_int nth, const Superboid& super)
{
  const Miniboid& fatboid = super.miniboids[0u];
  bool inSomeTriangle = false;
  const Miniboid* auxMini = &super.miniboids.back();

  for (auto& realMini : super.miniboids)
  {
    if (realMini.ID == 0)
      continue;
    if (inSomeTriangle)
      break;
    mini_int nextID = realMini.ID % (parameters().MINIBOIDS_PER_SUPERBOID - 1) + nth;
    nextID %= parameters().MINIBOIDS_PER_SUPERBOID - 1;
    auxMini = &super.miniboids[nextID];
    inSomeTriangle = isPointInTriangle(this->position, fatboid.position, auxMini->position, realMini.position);
  }

  return inSomeTriangle;
}

bool
Miniboid::fatInteractions(const step_int STEP,
			  const std::list<Neighbor>& list,
			  const bool interact)
{
  bool inSomeTriangle = false;

  if (list.empty())
    return inSomeTriangle;
  
  const Superboid& super = list.front().miniNeighbor.superboid;
  const Miniboid& fatboid = super.miniboids[0u];
    
  if (super.ID != this->superboid.ID)
  {
    for (mini_int nth = 1; nth <= 2; ++nth)
    {
      std::valarray<real> tangent(parameters().DIMENSIONS);
      
      const Miniboid* auxMini = nullptr;
      for (auto& realMini : super.miniboids)
      {
        if (realMini.ID == 0)
          continue;
        if (inSomeTriangle)
          break;
        mini_int nextID = realMini.ID % (parameters().MINIBOIDS_PER_SUPERBOID - 1) + nth;
        nextID %= parameters().MINIBOIDS_PER_SUPERBOID - 1;
        auxMini = &super.miniboids[nextID];
        
        inSomeTriangle = isPointInTriangle(this->position, fatboid.position, realMini.position, auxMini->position);
        if (inSomeTriangle && interact)
        {
          std::tuple<step_int, std::vector<const Miniboid*>>* h = nullptr;
          for (auto& c : this->history)
            if (std::get<1>(c).front()->superboid.ID == super.ID)
            {
              h = &c;
              break;
            }
          if (h)
          {
            const std::vector<const Miniboid*>& v = std::get<1>(*h);
            std::get<0>(*h) = STEP;
            tangent = Distance(*v[1], *v[0]).getTangentArray();
          }
          else {
            this->history.push_back(std::tuple<step_int, std::vector<const Miniboid*>>(STEP, std::vector<const Miniboid*>({auxMini, &realMini})));
            tangent = Distance(realMini, *auxMini).getTangentArray();
          }
        }
      }
      auxMini = nullptr;

      if (inSomeTriangle)
        this->_lastInvasionStep = STEP;
      
      if (inSomeTriangle && interact)
      {
        //// const std::valarray<real> tangent = tangentSignal * Distance(r1, r2).getTangentArray();
        //// const std::valarray<real> tangent = Distance(fatboid.position, r0 - tangent).module > Distance(fatboid.position, r0 + tangent).module ? -tangent : tangent;
        const std::valarray<real> force1 = (0.9f * parameters().INFINITE_FORCE) * tangent;
        const std::valarray<real> radial = this->radialDistance.getDirectionArray();
        const std::valarray<real> force2 = radial * (0.9f * parameters().INFINITE_FORCE);
	if (Infinite::write())
        {
	  std::valarray<real> direction = tangent + radial;
	  direction /= getModule(direction);
          std::valarray<real> infThing(parameters().DIMENSIONS * 2u);
          for (std::size_t i = 0; i < parameters().DIMENSIONS; ++i)
            infThing[i] = this->position[i];
          for (std::size_t i = 0; i < parameters().DIMENSIONS; ++i)
            infThing[i + parameters().DIMENSIONS] = direction[i];
          this->superboid.infinite2Vectors.push_back(infThing);
        }
        
        this->_forceSum += force1;
        this->_forceSum += force2;
      }
    }
  }

  return inSomeTriangle;
}

void
Miniboid::interInteractions(const Neighbor& neighbor)
{
  const Miniboid& miniNeighbor = neighbor.miniNeighbor;
  if (this->superboid.ID != miniNeighbor.superboid.ID)
  {
    const type_int MY_TYPE = this->superboid.type;
    const type_int NEIGHBOR_TYPE = miniNeighbor.superboid.type;
    
    // Sum velocity (ALPHA):
    this->_velocitySum += miniNeighbor.velocity * parameters().INTER_ALPHA[MY_TYPE][NEIGHBOR_TYPE];
    
    // Sum force (BETA):
    const real beta = parameters().INTER_BETA[MY_TYPE][NEIGHBOR_TYPE];
    const real rEq  = parameters().INTER_REQ[MY_TYPE][NEIGHBOR_TYPE];
    if (neighbor.distance.module <= parameters().CORE_DIAMETER)
    {
      const std::valarray<real> d = neighbor.distance.getDirectionArray();
      this->_forceSum += -parameters().INFINITE_FORCE * d;
      
      if (Infinite::write())
      {              
	std::valarray<real> infThing(parameters().DIMENSIONS * 2u);
	for (std::size_t i = 0; i < parameters().DIMENSIONS; ++i)
	  infThing[i] = this->position[i];
	for (std::size_t i = 0; i < parameters().DIMENSIONS; ++i)
	  infThing[i + parameters().DIMENSIONS] = -d[i];
	this->superboid.infiniteVectors.push_back(infThing);
      }
    }
    else
    {
     std::valarray<real> force = -getFiniteForce(neighbor.distance, beta, rEq);
      this->_forceSum += force;
    }
  }

  return;
}

void
Miniboid::interInteractions(const step_int STEP)
{
  for (auto& c : this->history)
    if (std::get<0>(c) + 1 != STEP)
    {
      this->history.remove(c);
      break;
    }

  for (const auto& pair : this->_neighbors)
  {
    const auto& list = std::get<1>(pair);
    // Interaction with peripheral miniboids:
    if (true)
    {
      if (list.size() == 1)
	this->interInteractions(list.front());
      else if (list.size() > 1)
      {
	super_int count = 0;
	const Miniboid* firstMini = nullptr;
	const Distance* firstDistance = nullptr;
	const Miniboid* secondMini = nullptr;
	//// const Distance* secondDistance = nullptr;
	
	for (const auto& particleNei : list)
	{
	  if (count == 0)
	  {
	    firstMini = &particleNei.miniNeighbor;
	    firstDistance = &particleNei.distance;
	  }
	  else if (count == 1)
	  {
	    secondMini = &particleNei.miniNeighbor;
	    //// secondDistance = &particleNei.distance;
	  }
	  else
	    break;
	  ++count;
	}
	
	Miniboid mini(0, firstMini->superboid, true);
	mini.velocity = firstMini->velocity;
	const real angularCoeff = getAngularCoefficient(firstMini->position, secondMini->position);
	mini.position = getClosestPoint(angularCoeff, firstMini->position, this->position);
	const Distance dist(*this, mini);
	const Neighbor miniN(mini, dist);
	this->interInteractions(miniN);
      }
    }
    else
      for (const auto& particleNei : list)
	this->interInteractions(particleNei);

    // Check if interacts with fatboid:
    this->fatInteractions(STEP, list, true);
    
  } // end of for pair in _neighbors.
  
  return;
}

real
Miniboid::getHarrisParameter(const std::vector<std::vector<real>>& matrix, const std::vector<real>& medium) const
{
  const type_int MY_TYPE = this->superboid.type;
  const mini_int TOTAL = parameters().HARRIS_AMOUNT[this->superboid.type];
  mini_int total = 0;
  std::vector<mini_int> counts(parameters().TYPES_NO, 0);

  for (const auto& pair : this->_neighbors)
  {
    const auto& superList = std::get<1>(pair);
    for (const auto& neighbor : superList)
    {
      ++counts[neighbor
               .miniNeighbor
               .superboid
               .type];
      ++total;
    }
  }

  real harris = -0.0f;

  for (type_int t = 0u; t < parameters().TYPES_NO; ++t)
  {
    harris += matrix[MY_TYPE][t] * counts[t];
  }

  if (total >= TOTAL)
    harris /= total;
  else
  {
    harris += (TOTAL - total) * medium[MY_TYPE];
    harris /= TOTAL;
  }
  
  return harris;
}

void
Miniboid::setNextVelocity(const step_int STEP)
{
  if (this->isVirtual)
    return;

  this->noise();        // Find a random noise (ETA).
  const type_int MY_TYPE = this->superboid.type;

  // Interact with miniboids of another superboid:
  if (this->ID != 0 && !this->isVirtual)
    this->interInteractions(STEP);
  
  // Same superboid:
  const auto& miniboidsInThisCell = this->superboid.miniboids;
  for (const auto& mini : miniboidsInThisCell)
    this->_velocitySum += parameters().AUTO_ALPHA[MY_TYPE] * mini.velocity;

  // Radial:
  if (this->ID == 0u)
  {
    for (auto& mini : miniboidsInThisCell)
      if (mini.ID != 0u)
      {
        if (mini.radialDistance.module <= parameters().CORE_DIAMETER)
          this->_forceSum += parameters().INFINITE_FORCE * mini.radialDistance.getDirectionArray();
        else
        {
          const real beta = mini.getHarrisParameter(parameters().RADIAL_BETA, parameters().RADIAL_BETA_MEDIUM);
          std::valarray<real> force = beta * getFiniteRadialForceWithoutBeta(mini.radialDistance, parameters().RADIAL_REQ[MY_TYPE], MY_TYPE);
          this->_forceSum += force;
        }
      }
  }
  else
    if (this->ID != 0u)
    {
      Distance distance = (this->radialDistance);
      // lest cost in -real than -Distance.
      if (distance.module <= parameters().CORE_DIAMETER)
        this->_forceSum += -parameters().INFINITE_FORCE * distance.getDirectionArray();
      else
      {
        // lest cost in -real than -Distance.
        const real beta = this->getHarrisParameter(parameters().RADIAL_BETA, parameters().RADIAL_BETA_MEDIUM);
        std::valarray<real> force = -beta * getFiniteRadialForceWithoutBeta(distance, parameters().RADIAL_REQ[MY_TYPE], MY_TYPE);
        this->_forceSum += force;
      }
    }
  
  // Twist
  for (const auto& tn : this->_twistNeighbors)
  {
    const short int signal = sign(tn.ANGLES[0u]);
    const Miniboid& miniNeighbor = miniboidsInThisCell[tn.ID];
    const real distanceBetween = tn._distance.module;
    std::valarray<real> tangent = (this->radialDistance.getTangentArray());
    if (distanceBetween <= parameters().CORE_DIAMETER)
    {
      const std::valarray<real> f = signal * parameters().INFINITE_FORCE * tangent;
      this->_forceSum += f;
    }
    else
    {
      const real ANGLE_BETWEEN = angleBetween(this->radialAngle, miniNeighbor.radialAngle);
      const real SUBTRACTION = assertAngle(ANGLE_BETWEEN - tn.ANGLES[0u]);
      const real kapa1 = this->getHarrisParameter(parameters().KAPA, parameters().KAPA_MEDIUM);
      const real kapa2 = miniNeighbor.getHarrisParameter(parameters().KAPA, parameters().KAPA_MEDIUM);
      const real kapa = (kapa1 + kapa2) / 2.0f;
      const real beta = this->getHarrisParameter(parameters().TANGENT_BETA, parameters().TANGENT_BETA_MEDIUM);
      const std::valarray<real> f1 = -kapa * SUBTRACTION * parameters().RADIAL_REQ[MY_TYPE] * tangent;
      const std::vector<real> limits({parameters().TANGENT_PLASTIC_BEGIN[MY_TYPE], parameters().TANGENT_PLASTIC_END[MY_TYPE]});
      const std::valarray<real> f2 = static_cast<real>(signal) * getFiniteForce(tn._distance, beta, parameters().TANGENT_REQ[MY_TYPE], limits);
      this->_forceSum += f1;
      this->_forceSum += f2;
    }
  }

  // Finalizing
  const std::valarray<real> sum = this->_noiseSum + this->_velocitySum + this->_forceSum;
  const real module = getModule(sum);
  if (module > 1.0e-6f)
    this->newVelocity = (parameters().SPEED[MY_TYPE] / module) * sum;

  return;
}

void
Miniboid::setNextPosition(const step_int step)
{
  // const step_int invasionCounter = step - this->_lastInvasionStep;;
  this->velocity = this->newVelocity;
  // if (invasionCounter > 0u && this->ID != 0u)
  //   this->position = this->superboid.miniboids[0u].position - (1.1f * parameters().CORE_DIAMETER * this->radialDistance.getDirectionArray());
  // if (invasionCounter > 15u && this->ID != 0u)
  //   this->position = this->superboid.miniboids[0u].position - (1.1f * parameters().CORE_DIAMETER * this->radialDistance.getDirectionArray());
  // else if (invasionCounter > 10u && this->ID != 0u)
  //   this->position = this->superboid.miniboids[0u].position - (2.0f * parameters().CORE_DIAMETER * this->radialDistance.getDirectionArray());
  // else if (invasionCounter > 5u && this->ID != 0u)
  //   this->position = this->superboid.miniboids[0u].position - (0.5f * this->radialDistance.module * this->radialDistance.getDirectionArray());
  // else
  {
    this->position += parameters().DT * this->velocity; // Velocity is already normalized to V0.
    // if (this->ID != 0 && !this->isVirtual)
    //   this->position += this->radialDistance.getDirectionArray() *
    //         ((this->superboid.area - parameters().TARGET_AREA[this->superboid.type]) / (20.0f * TWO_PI * this->superboid.meanRadius));
  }
  
  this->checkLimits(step);
  
  return;
}

void
Miniboid::checkBackInTime(const step_int step)
{
  return;
  
  if (step == 0)
    this->_oldPosition = this->position;
  else if (this->_lastInvasionStep == step)
  {
    this->position = this->_oldPosition;
    this->checkLimits(step);
  }
  else
    this->_oldPosition = this->position;
  
  return;
}

bool
Miniboid::inEdge(void) const
{
  return this->_box->inEdge;
}

void
Miniboid::setNeighbors(const step_int step)
{
  this->checkLimits(step);
  this->_neighbors.clear(); // Removes all elements.

  // Search for neighbors:
  for (auto box : this->_box->neighbors)
    for (auto miniPointer : box->miniboids)
      if (miniPointer->superboid.ID != this->superboid.ID)
        if (miniPointer->ID != 0u)
        {
          const Miniboid& mini = *miniPointer;
          Distance distance(*this, mini);
          if (distance.module <= parameters().NEIGHBOR_DISTANCE)
          {
            this->superboid.cellNeighbors.append(mini.superboid.ID); // Potential data race!!!!!!!!
            ++(this->_neighborsPerTypeNos[mini.superboid.type]); //// ERRADO?
	    std::list<Neighbor>& c = this->_neighbors[mini.superboid.ID];
	    c.emplace_back(mini, distance);
	  }
        }
  
  // Sort each list in terms of distance:
  for (auto& pair : this->_neighbors)
    std::get<1>(pair).sort();
  
  return;
}

box_int
Miniboid::boxID(void) const
{
  return _box->ID;
}

void
Miniboid::setNewVelocity(void)
{
  const real angle = this->superboid.get0to2piRandom();
  this->newVelocity[X] = std::cos(angle);
  this->newVelocity[Y] = std::sin(angle);
  return;
}

void
Miniboid::reset(void)
{
  this->_noiseSum = 0.0f;
  this->_velocitySum = 0.0f;
  this->_forceSum = 0.0f;
  this->_neighborsPerTypeNos = 0u;
  
  if (this->ID != 0u || this->isVirtual)
  {
    this->radialDistance = Distance(*this, this->superboid.miniboids[0u]);
    this->radialAngle = this->radialDistance.getAngle();
  }

  if (!this->isVirtual)
    for (auto& tn : this->_twistNeighbors)
      tn._distance = Distance(*this, this->superboid.miniboids[tn.ID]);
  
  return;
}

real
Miniboid::getAreaBetween(const Miniboid& miniNeighbor) const
{
#ifdef DEBUG
  if (this->superboid != miniNeighbor.superboid)
    std::cerr << "cacaca" << std::endl;
#endif

  const real DELTA_ANGLE = std::fabs(
    angleBetween(this->radialDistance.getAngle(), miniNeighbor.radialDistance.getAngle()));
  const real area = std::sin(DELTA_ANGLE) * this->radialDistance.module * \
    miniNeighbor.radialDistance.module * 0.5f;

  return area;
}
