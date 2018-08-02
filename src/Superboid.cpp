// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#include "Superboid.hpp"

#include <ctime>
#include <random>
#include <vector>

#include "Box.hpp"
#include "Miniboid.hpp"
#include "Stokes.hpp"
#include "export.hpp"
#include "initial.hpp"
#include "nextstep.hpp"
#include "parameters.hpp"

unsigned long long int
    getSeed(const super_int id) {
  return std::time(NULL) + id;
}

super_int Superboid::_totalSuperboids(0u);

std::ostream &
    operator<<(std::ostream &os, const Superboid &super) {
  for (const auto &mini : super.miniboids)
    os << mini << std::endl;
  os << std::endl;
  return os;
}

static std::vector<super_int>
    getVectorWithNoOfSuperboidsPerType(void) {
  std::vector<super_int> vec(parameters().TYPES_NO);
  super_int total = 0u;

  for (type_int type = 0u; type < vec.size(); ++type) {
    if (parameters().PROPORTIONS[type] < 1.0e-6f)
      vec[type] = 0u;
    else {
      super_int s = static_cast<super_int>(std::floor(
          parameters().PROPORTIONS[type] * real(parameters().SUPERBOIDS)));
      vec[type]   = s;
      total += s;
    }
  }
  vec[0u] += parameters().SUPERBOIDS - total;
  return vec;
}

static type_int
    getType(const super_int id) {
  static std::default_random_engine defaultEngine(std::time(NULL));
  static std::mt19937 mtEngine(defaultEngine());
  static std::uniform_int_distribution<super_int> uni(
      0, parameters().SUPERBOIDS - 1);

  static auto amounts = getVectorWithNoOfSuperboidsPerType();
  static std::vector<bool> alreadySet(parameters().SUPERBOIDS, false);
  static std::vector<type_int> types(parameters().SUPERBOIDS, 123);

  static bool firstTime = true;
  if (firstTime) {
    firstTime = false;
    for (type_int t = 0; t < parameters().TYPES_NO; ++t) {
      super_int supersWithTypeT = 0;
      while (supersWithTypeT < amounts[t]) {
        super_int chosen = uni(mtEngine);
        if (alreadySet[chosen] == false) {
          alreadySet[chosen] = true;
          ++supersWithTypeT;
          types[chosen] = t;
        }
      }
    }
  }

  return types[id];
}

// Get a peripheral miniboid position considering the central miniboid
// position is its origin.
static std::valarray<real>
    getPeripheralMiniboidPosition(const type_int TYPE, const real ANGLE) {
  const real distance = parameters().RADIAL_REQ[TYPE] * 0.5f;

  std::valarray<real> position(parameters().DIMENSIONS);
  position[X] = distance * std::cos(ANGLE);
  position[Y] = distance * std::sin(ANGLE);
  //// position += initialNoise(parameters().CORE_DIAMETER / 10.0f);

  return position;
}

static real
    getDeltaAngle(void) {
  if (parameters().MINIBOIDS_PER_SUPERBOID > 3u)
    return TWO_PI / (parameters().MINIBOIDS_PER_SUPERBOID - 1u);
  else
    return parameters().INITIAL_ANGLE_BETWEEN;
}

Superboid::Superboid(void)
    : ID(this->_totalSuperboids)
    , type(getType(this->_totalSuperboids))
    , area(-0.0f)
    , perimeter(-0.0f)
    , meanRadius(-0.0f)
    , meanRadius2(-0.0f)
    , virtualsInfo(std::ios_base::out)
    , _deathState(DeathState::WillDie)
    , _randomEngine(getSeed(ID))
    , _lastDivisionStep(0) {
  this->miniboids.reserve(parameters().MINIBOIDS_PER_SUPERBOID);
  this->virtualMiniboids.reserve(64u * parameters().MINIBOIDS_PER_SUPERBOID);

  if (this->_totalSuperboids >= parameters().SUPERBOIDS) {
    for (mini_int miniCount = 0u;
         miniCount < parameters().MINIBOIDS_PER_SUPERBOID; ++miniCount)
      this->miniboids.emplace_back(miniCount, *this);
  } else {
    if (parameters().INITIAL_CONDITION == InitialCondition::HEX_CENTER) {
      for (mini_int miniCount = 0u;
           miniCount < parameters().MINIBOIDS_PER_SUPERBOID; ++miniCount) {
        static const real DELTA_ANGLE = getDeltaAngle();
        this->miniboids.emplace_back(miniCount, *this);
        if (miniCount == 0u) {
          bool ready = false;
          while (!ready) {
            this->miniboids[0].position = getCentralMiniboidPosition();
            ready = tryArrangeCell(this->miniboids[0]);
          }
        } else
          this->miniboids[miniCount].position
              = this->miniboids[0u].position
                + getPeripheralMiniboidPosition(
                      this->type, DELTA_ANGLE * (miniCount - 1u)
                                      + parameters().INITIAL_ANGLE_ID1);
      }
    } else if (parameters().INITIAL_CONDITION == InitialCondition::LEFT_EDGE) {
      static const super_int AMOUNT_IN_A_COLUMN
          = parameters().RECTANGLE_SIZE[Y] / parameters().INITIAL_DISTANCE;
      static super_int positionCount = 0;
      bool ready = false;

      while (!ready) {
        this->miniboids.emplace_back(0u, *this);
        std::valarray<real> &centralPosition = this->miniboids[0u].position;
        const super_int COLUMN
            = static_cast<super_int>(positionCount / AMOUNT_IN_A_COLUMN);
        const super_int ROW
            = static_cast<super_int>(positionCount % AMOUNT_IN_A_COLUMN);
        centralPosition[X] = (COLUMN + 0.5f) * parameters().INITIAL_DISTANCE
                            - parameters().RECTANGLE_SIZE[X] / 2.0f;
        centralPosition[Y] = (ROW + 0.5f) * parameters().INITIAL_DISTANCE
                            - parameters().RECTANGLE_SIZE[Y] / 2.0f;
        ready = tryArrangeCell(this->miniboids[0]);
        ++positionCount;
      }

      if (parameters().DIMENSIONS != 2u)
        std::cerr << "Unimplemented!" << std::endl;

      for (mini_int miniCount = 1u;
           miniCount < parameters().MINIBOIDS_PER_SUPERBOID; ++miniCount) {
        this->miniboids.emplace_back(miniCount, *this);
        this->miniboids[miniCount].position
            = this->miniboids[0u].position
              + getPeripheralMiniboidPosition(
                    this->type, getDeltaAngle() * (miniCount - 1u)
                                    + parameters().INITIAL_ANGLE_ID1);
      }
    } else {
      std::cerr << "Unimplemented!" << std::endl;
    }
  }

  this->deactivate();
  ++(this->_totalSuperboids);

  return;
}

real
    Superboid::get0to2piRandom(void) {
  static std::uniform_real_distribution<real> uniDistribution(0.0f, TWO_PI);
  return uniDistribution(this->_randomEngine);
}

void
    Superboid::setGamma(std::vector<Superboid> &superboids) {
  super_int sameTypeNeighborsNo    = 0u;
  super_int anotherTypeNeighborsNo = 0u;

  for (const auto &superNeighborID : this->cellNeighbors()) {
    if (this->type == superboids[superNeighborID].type)
      ++sameTypeNeighborsNo;
    else
      ++anotherTypeNeighborsNo;
  }

  if (sameTypeNeighborsNo + anotherTypeNeighborsNo != 0) {
    this->gamma = static_cast<real>(anotherTypeNeighborsNo)
                  / (anotherTypeNeighborsNo + sameTypeNeighborsNo);
    this->doUseGamma = true;
  } else {
    this->gamma      = -0.0f;
    this->doUseGamma = false;
  }

  return;
}

void
    Superboid::setShape(const step_int STEP) {
  if (STEP <= this->_shapeStep && STEP != 0)
    return;
  if (this->isActivated() == false)
    std::cerr << "eita, giovana" << std::endl;

  this->_shapeStep  = STEP;
  this->area        = -0.0f;
  this->perimeter   = -0.0f;
  this->meanRadius  = -0.0f;
  this->meanRadius2 = -0.0f;

  for (const auto &mini1 : this->miniboids) {
    if (mini1.ID == 0u)
      // If central miniboid, then continue loop in the next miniboid.
      continue;
    else if (mini1.ID
             == parameters().MINIBOIDS_PER_SUPERBOID - 1u)  // Last miniboid.
    {
      this->perimeter += Distance(mini1, this->miniboids[1u]).module;
      this->area += mini1.getAreaBetween(this->miniboids[1u]);
    } else  // Another peripheral but the last.
    {
      this->perimeter += Distance(mini1, this->miniboids[mini1.ID + 1u]).module;
      this->area += mini1.getAreaBetween(this->miniboids[mini1.ID + 1u]);
    }

    real radialDistanceModule = Distance(mini1, this->miniboids[0]).module;
    this->meanRadius += radialDistanceModule;
    this->meanRadius2 += square(radialDistanceModule);
  }

  return;
}

void
    Superboid::checkVirtual(const bool export_, const step_int step) {
  const real _maxDistance
      = 2.0f * parameters().RADIAL_REQ[this->type]
        * std::sqrt(
              2.0f
              * (1.0f
                 - std::cos(2.0 * PI
                            / (parameters().MINIBOIDS_PER_SUPERBOID - 1u))));
  if (export_)
    this->virtualsInfo.str(std::string(""));
  for (auto &mini1 : this->miniboids) {
    if (mini1.ID == 0u)
      continue;
    std::vector<real> vec;
    for (auto &tn : mini1._twistNeighbors)
      if (tn._distance.module > parameters().CORE_DIAMETER)
        vec.emplace_back(tn._distance.module);
    vec.emplace_back(_maxDistance);
    std::sort(vec.begin(), vec.end());
    const real maxDistance = vec.front();
    const Distance &dist   = mini1._twistNeighbors.front()._distance;

    if (dist.module > maxDistance) {
      const mini_int VIRTUAL_NO
          = static_cast<mini_int>(std::floor(dist.module / maxDistance));
      for (mini_int virtID = 0; virtID < VIRTUAL_NO; ++virtID) {
        const mini_int newID = this->virtualMiniboids.size();
        this->virtualMiniboids.emplace_back(newID, *this, true);
        Miniboid &virtualMini                = this->virtualMiniboids[newID];
        std::valarray<real> differenceVector = dist.getDirectionArray();
        differenceVector *= (virtID + 1u) * (dist.module / (VIRTUAL_NO + 1));
        virtualMini.position = mini1.position + differenceVector;
        if (export_)
          this->virtualsInfo << virtualMini.position << '\t' << this->type
                             << std::endl;
        virtualMini.checkLimits(step);
        virtualMini.reset();
      }
    }
  }

  return;
}

void
    Superboid::setNextPosition(const step_int step) {
  this->setShape(step);

  for (auto &mini : this->miniboids)
    mini.setNextPosition(step);

  this->miniboids[0].checkFatOut();

  return;
}

static std::vector<std::valarray<real>>
    getOriginalPositions(const std::vector<Miniboid> &miniboids) {
  std::vector<std::valarray<real>> v;
  v.reserve(miniboids.size());

  for (const auto &mini : miniboids)
    v.push_back(mini.position);

  return v;
}

static void
    setOriginalPositions(std::vector<Miniboid> &miniboids,
                         const std::vector<std::valarray<real>> &original) {
  for (auto &mini : miniboids)
    mini.position = original[mini.ID];

  return;
}

static void
    rearrangePeripherals(Superboid &superboid, const real distance) {
  superboid.miniboids[0].checkLimits();
  superboid.miniboids[0].reset();
  std::vector<Distance> distances;

  for (mini_int miniID = 1u; miniID < parameters().MINIBOIDS_PER_SUPERBOID;
       ++miniID) {
    const real angle
        = miniID * TWO_PI / (parameters().MINIBOIDS_PER_SUPERBOID - 1);
    std::valarray<real> dist({std::cos(angle), std::sin(angle)});
    dist *= distance + parameters().REAL_TOLERANCE;
    superboid.miniboids[miniID].position
        = superboid.miniboids[0u].position + dist;
    superboid.miniboids[miniID].reset();
    superboid.miniboids[miniID].checkLimits();
  }

  return;
}

bool
    Superboid::divide(const super_int divide_by, Superboid &newSuperboid,
                      std::vector<Box> &boxes, const step_int step) {
  if (divide_by < 2u) {
    std::cerr << "Cannot divide by n | n < 2" << std::endl;
    return false;
  }
  if (divide_by != 2u) {
    std::cerr << "Unimplemented." << std::endl;
    return false;
  }

  static std::random_device devRand;
  static std::default_random_engine generator(devRand());
  std::uniform_int_distribution<int> distribution(0, parameters().TYPES_NO - 1);
  const type_int newType = distribution(generator);

  *const_cast<type_int *>(&(newSuperboid.type)) = newType;

  const std::vector<std::valarray<real>> originalPositions
      = getOriginalPositions(this->miniboids);
  newSuperboid.activate();
  newSuperboid.clearVirtualMiniboids();
  this->clearVirtualMiniboids();

  for (mini_int miniID = 0u; miniID < parameters().MINIBOIDS_PER_SUPERBOID;
       ++miniID) {
    newSuperboid.miniboids[miniID].position = this->miniboids[miniID].position;
    newSuperboid.miniboids[miniID].velocity = this->miniboids[miniID].velocity;
    newSuperboid.miniboids[miniID].newVelocity
        = this->miniboids[miniID].newVelocity;

    const box_int newBoxID
        = Box::getBoxID(newSuperboid.miniboids[miniID].position);
    boxes[newBoxID].append(newSuperboid.miniboids[miniID]);
  }

  step_int atempts = 0;
  while (true) {
    ++atempts;
    setOriginalPositions(this->miniboids, originalPositions);

    if (atempts > 6) {
      nextBoxes(boxes, *this, step);
      ;
      newSuperboid.setDeactivation(
          "newSuperboid could not be child cell (division)");
      return false;
    }

    bool someInvasion = false;

    real divisionAngle = this->get0to2piRandom();
    std::valarray<real> nucleusNucleusDistance(
        {std::cos(divisionAngle), std::sin(divisionAngle)});
    nucleusNucleusDistance *= parameters().DIVISION_DISTANCE;

    newSuperboid.miniboids[0u].position
        = this->miniboids[0u].position - nucleusNucleusDistance;
    this->miniboids[0u].position
        = this->miniboids[0u].position + nucleusNucleusDistance;

    const real radius
        = parameters().DIVISION_DISTANCE - parameters().CORE_DIAMETER * 2.0f;
    this->_shapeStep        = 0;
    newSuperboid._shapeStep = 0;
    rearrangePeripherals(*this, radius);
    rearrangePeripherals(newSuperboid, radius);

    std::vector<Superboid *> twoSupers({this, &newSuperboid});
    bool insideBox = true;
    for (auto superPtr : twoSupers)
      if (insideBox)
        for (const auto &mini : superPtr->miniboids)
          if (insideBox)
            for (dimension_int dim = 0u; dim < parameters().DIMENSIONS; ++dim) {
              const real HALF_RECTANGLE_SIZE
                  = 0.5f * parameters().RECTANGLE_SIZE[dim];
              if (std::abs(mini.position[dim]) > HALF_RECTANGLE_SIZE)
                insideBox = false;
            }
    if (insideBox == false)
      continue;

    for (auto &mini : newSuperboid.miniboids)
      mini.setBox(&boxes[Box::getBoxID(mini.position)]);

    nextBoxes(boxes, *this, step);

    for (auto super : twoSupers) {
      for (auto &mini : super->miniboids)
        mini.setNeighbors(step);
      for (auto &mini : super->virtualMiniboids)
        mini.setNeighbors(step);
    }

    for (auto super : twoSupers) {
      for (auto &mini : super->miniboids) {
        if (!someInvasion)
          for (const auto &hole : parameters().STOKES_HOLES)
            if (hole.isInside(mini.position)) {
              someInvasion = true;
              break;
            }

        if (!someInvasion) {
          for (const auto &pair : mini._neighbors)
            if (mini.fatInteractions(0, std::get<1>(pair), false)) {
              someInvasion = true;
              break;
            }
        }
      }
    }
    if (!someInvasion)
      break;
  }

  this->_lastDivisionStep        = step;
  newSuperboid._lastDivisionStep = step;

  this->reset();
  newSuperboid.reset();

  this->clearVirtualMiniboids();
  newSuperboid.clearVirtualMiniboids();

  return true;
}

Distance
    Superboid::getBiggestAxis() const {
  std::vector<Distance> distances;
  const mini_int peripheralNo = parameters().MINIBOIDS_PER_SUPERBOID - 1;
  for (mini_int miniID1 = 1u; miniID1 < parameters().MINIBOIDS_PER_SUPERBOID;
       ++miniID1) {
    std::vector<mini_int> miniIDs2;
    mini_int opposite1 = miniID1;
    opposite1 += peripheralNo / 2;
    if (parameters().MINIBOIDS_PER_SUPERBOID % 2 == 0) {
      miniIDs2.emplace_back(opposite1 - 1);
      miniIDs2.emplace_back(opposite1);
      miniIDs2.emplace_back(opposite1 + 1);
      miniIDs2.emplace_back(opposite1 + 2);
    } else  // MINIBOIDS_PER_SUPERBOID % 2 == 1
    {
      miniIDs2.emplace_back(opposite1 - 1);
      miniIDs2.emplace_back(opposite1);
      miniIDs2.emplace_back(opposite1 + 1);
    }

    for (auto &miniID2 : miniIDs2)  // Reference.
      if (miniID2 >= parameters().MINIBOIDS_PER_SUPERBOID)
        miniID2 -= peripheralNo;
    for (auto miniID2 : miniIDs2)  // Copy.
      if (miniID2 > miniID1)
        distances.emplace_back(this->miniboids[miniID1],
                               this->miniboids[miniID2]);
  }

  std::sort(distances.begin(), distances.end());

  return distances.back();
}

void
    Superboid::checkWrongNeighbors(const std::vector<Superboid> &superboids) {
  const std::list<super_int> neighbors
      = this->cellNeighbors();  // Value, not referece.
  for (const auto cellID1 : neighbors) {
    for (const auto cellID2 : neighbors) {
      if (cellID1 == cellID2)
        continue;
      if (this->ID == cellID1)
        continue;
      if (this->ID == cellID2)
        continue;

      const Superboid &super1 = superboids[cellID1];
      const Superboid &super2 = superboids[cellID2];

      const Distance dist(this->miniboids[0u], super2.miniboids[0u]);
      const std::vector<real> portions
          = {0.5, 0.53, 0.47, 0.56, 0.44, 0.6, 0.4};
      for (const auto portion : portions) {
        const Distance halfDist = dist * portion;

        //// checar BC periódica depois!!!!!!!!!!!!!!!!!!!!
        if (isPointInSomeNthTriangle(
                1,
                this->miniboids[0u].position
                    + (halfDist.getDirectionArray() * halfDist.module),
                super1)) {
          this->cellNeighbors.remove(cellID2);
          for (auto &mini : this->miniboids)
            for (auto &pair : mini._neighbors) {
              auto &cell = std::get<1>(pair);
              if (!cell.empty()) {
                const super_int superID
                    = cell.front().miniNeighbor.superboid.ID;
                if (superID == super2.ID)
                  cell.clear();
              }
            }
          break;
        }
      }
    }
  }
}

void
    Superboid::deactivate(void) {
  std::cerr << "death " << this->ID << ": " << this->_deathMessage << std::endl;

  for (auto &mini : this->miniboids) {
    if (mini.getBoxPtr())
      mini.getBoxPtr()->remove(mini);
    mini.setBox(nullptr);
  }

  this->_deathState = DeathState::Dead;

  return;
}

void
    Superboid::activate(void) {
  this->_deathState = DeathState::Live;

  return;
}

void
    Superboid::clearVirtualMiniboids(void) {
  for (auto &mini : this->virtualMiniboids) {
    if (mini.getBoxPtr())
      mini.getBoxPtr()->remove(mini);
  }
  this->virtualMiniboids.clear();

  return;
}

void
    Superboid::setDeactivation(const std::string &message) {
  this->_deathState   = DeathState::WillDie;
  this->_deathMessage = message;

  return;
}

bool
    Superboid::willDie(void) const {
  return this->_deathState == DeathState::WillDie;
}

bool
    Superboid::isActivated(void) const {
  return this->_deathState != DeathState::Dead;
}

void
    Superboid::checkBackInTime(const step_int step) {
  for (auto &mini : this->miniboids)
    mini.checkBackInTime(step);

  return;
}

real
    Superboid::getRadialReq(const step_int step) const {
  if (parameters().DIVISION_INTERVAL == 0)
    return parameters().RADIAL_REQ[this->type];
  const step_int deltaT = step - this->_lastDivisionStep;
  if (deltaT > parameters().NON_DIVISION_INTERVAL)
    return parameters().RADIAL_REQ[this->type];

  real angular
      = parameters().RADIAL_REQ[this->type] - parameters().DIVISION_DISTANCE;
  angular /= parameters().NON_DIVISION_INTERVAL;

  return parameters().DIVISION_DISTANCE + angular * deltaT;
}

real
    Superboid::getTangentReq(const step_int step) const {
  if (parameters().DIVISION_INTERVAL == 0)
    return parameters().TANGENT_REQ[this->type];
  const step_int deltaT = step - this->_lastDivisionStep;
  if (deltaT > parameters().NON_DIVISION_INTERVAL)
    return parameters().TANGENT_REQ[this->type];

  const real r = this->getRadialReq(step);
  return parameters().TANGENT_REQ[this->type] * r
         / parameters().RADIAL_REQ[this->type];
}
