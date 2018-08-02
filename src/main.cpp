// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#include <fstream>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>

#include "Argument.hpp"
#include "Box.hpp"
#include "Date.hpp"
#include "Parameter.hpp"
#include "Stokes.hpp"
#include "Superboid.hpp"
#include "export.hpp"
#include "load.hpp"
#include "nextstep.hpp"
#include "parameters.hpp"

static void
    shapeIt(const std::vector<Superboid> &superboids, std::ofstream &shapeFile,
            const step_int step) {
  const Parameters &p = parameters();

  const real smallestFloat = std::numeric_limits<float>().min();
  const real biggestFloat  = std::numeric_limits<float>().max();
  real ratioSum            = -0.0f;
  real maxRatio            = smallestFloat;
  real minRatio            = biggestFloat;
  real perimeterSum        = -0.0f;
  real maxPerimeter        = smallestFloat;
  real minPerimeter        = biggestFloat;
  real areaSum             = -0.0f;
  real maxArea             = smallestFloat;
  real minArea             = biggestFloat;
  real radiusSum           = -0.0f;
  real maxRadius           = smallestFloat;
  real minRadius           = biggestFloat;
  real radius2Sum          = -0.0f;

  std::vector<real> ratioSumVec(p.TYPES_NO, -0.0f);
  std::vector<real> maxRatioVec(p.TYPES_NO, smallestFloat);
  std::vector<real> minRatioVec(p.TYPES_NO, biggestFloat);
  std::vector<real> perimeterSumVec(p.TYPES_NO, -0.0f);
  std::vector<real> maxPerimeterVec(p.TYPES_NO, smallestFloat);
  std::vector<real> minPerimeterVec(p.TYPES_NO, biggestFloat);
  std::vector<real> areaSumVec(p.TYPES_NO, -0.0f);
  std::vector<real> maxAreaVec(p.TYPES_NO, smallestFloat);
  std::vector<real> minAreaVec(p.TYPES_NO, biggestFloat);
  std::vector<real> radiusSumVec(p.TYPES_NO, -0.0f);
  std::vector<real> maxRadiusVec(p.TYPES_NO, smallestFloat);
  std::vector<real> minRadiusVec(p.TYPES_NO, biggestFloat);
  std::vector<real> radius2SumVec(p.TYPES_NO, -0.0f);

  super_int cellsActivatedNo = 0;
  std::vector<super_int> activatedPerType(p.TYPES_NO, 0);
  for (const auto &super : superboids) {
    if (super.isActivated() == false)
      continue;
    ++activatedPerType[super.type];
    ++cellsActivatedNo;

    areaSum += super.area;
    perimeterSum += super.perimeter;
    const real ratio = super.perimeter / std::sqrt(super.area);
    ratioSum += ratio;
    radiusSum += super.meanRadius;
    radius2Sum += super.meanRadius2;

    areaSumVec[super.type] += super.area;
    perimeterSumVec[super.type] += super.perimeter;
    ratioSumVec[super.type] += ratio;
    radiusSumVec[super.type] += super.meanRadius;
    radius2SumVec[super.type] += super.meanRadius2;

    if (ratio < minRatio)
      minRatio = ratio;
    if (ratio > maxRatio)
      maxRatio = ratio;
    if (super.area < minArea)
      minArea = super.area;
    if (super.area > maxArea)
      maxArea = super.area;
    if (super.perimeter < minPerimeter)
      minPerimeter = super.perimeter;
    if (super.perimeter > maxPerimeter)
      maxPerimeter = super.perimeter;
    if (super.meanRadius < minRadius)
      minRadius = super.meanRadius;
    if (super.meanRadius > maxRadius)
      maxRadius = super.meanRadius;

    if (ratio < minRatioVec[super.type])
      minRatioVec[super.type] = ratio;
    if (ratio > maxRatioVec[super.type])
      maxRatioVec[super.type] = ratio;
    if (super.area < minAreaVec[super.type])
      minAreaVec[super.type] = super.area;
    if (super.area > maxAreaVec[super.type])
      maxAreaVec[super.type] = super.area;
    if (super.perimeter < minPerimeterVec[super.type])
      minPerimeterVec[super.type] = super.perimeter;
    if (super.perimeter > maxPerimeterVec[super.type])
      maxPerimeterVec[super.type] = super.perimeter;
    if (super.meanRadius < minRadiusVec[super.type])
      minRadiusVec[super.type] = super.meanRadius;
    if (super.meanRadius > maxRadiusVec[super.type])
      maxRadiusVec[super.type] = super.meanRadius;
  }
  const real meanArea      = areaSum / cellsActivatedNo;
  const real meanPerimeter = perimeterSum / cellsActivatedNo;
  const real meanRatio     = ratioSum / cellsActivatedNo;
  const real meanRadius    = radiusSum / cellsActivatedNo;

  real msdPerimeter = -0.0f;
  real msdRatio     = -0.0f;
  real msdArea      = -0.0f;

  std::vector<real> meanAreaVec(p.TYPES_NO);
  std::vector<real> meanPerimeterVec(p.TYPES_NO);
  std::vector<real> meanRatioVec(p.TYPES_NO);
  std::vector<real> meanRadiusVec(p.TYPES_NO);
  for (type_int type = 0u; type < p.TYPES_NO; ++type) {
    meanAreaVec[type]      = areaSumVec[type] / activatedPerType[type];
    meanPerimeterVec[type] = perimeterSumVec[type] / activatedPerType[type];
    meanRatioVec[type]     = ratioSumVec[type] / activatedPerType[type];
    meanRadiusVec[type]    = radiusSumVec[type] / activatedPerType[type];
  }

  std::vector<real> msdPerimeterVec(p.TYPES_NO, -0.0f);
  std::vector<real> msdRatioVec(p.TYPES_NO, -0.0f);
  std::vector<real> msdAreaVec(p.TYPES_NO, -0.0f);

  for (const auto &super : superboids) {
    if (super.isActivated() == false)
      continue;

    msdPerimeter += square(super.perimeter - meanPerimeter);
    msdArea += square(super.area - meanArea);
    msdRatio += square(super.perimeter / std::sqrt(super.area) - meanRatio);

    msdPerimeterVec[super.type]
        += square(super.perimeter - meanPerimeterVec[super.type]);
    msdAreaVec[super.type] += square(super.area - meanAreaVec[super.type]);
    msdRatioVec[super.type] += square(super.perimeter / std::sqrt(super.area)
                                      - meanRatioVec[super.type]);
  }

  msdArea /= cellsActivatedNo;
  msdArea = std::sqrt(msdArea);
  msdPerimeter /= cellsActivatedNo;
  msdPerimeter = std::sqrt(msdPerimeter);
  msdRatio /= cellsActivatedNo;
  msdRatio             = std::sqrt(msdRatio);
  real meanMeanRadius2 = radius2Sum / cellsActivatedNo;

  std::vector<real> meanMeanRadius2Vec(p.TYPES_NO);
  for (type_int type = 0u; type < p.TYPES_NO; ++type) {
    msdAreaVec[type] /= activatedPerType[type];
    msdAreaVec[type] = std::sqrt(msdAreaVec[type]);
    msdPerimeterVec[type] /= activatedPerType[type];
    msdPerimeterVec[type] = std::sqrt(msdPerimeterVec[type]);
    msdRatioVec[type] /= activatedPerType[type];
    msdRatioVec[type]        = std::sqrt(msdRatioVec[type]);
    meanMeanRadius2Vec[type] = radius2SumVec[type] / activatedPerType[type];
  }

  shapeFile << std::fixed << step << '\t' << meanRatio << '\t' << msdRatio
            << '\t' << minRatio << '\t' << maxRatio << '\t' << meanArea << '\t'
            << msdArea << '\t' << minArea << '\t' << maxArea << '\t'
            << meanPerimeter << '\t' << msdPerimeter << '\t' << minPerimeter
            << '\t' << maxPerimeter << '\t' << meanRadius << '\t'
            << meanMeanRadius2 << '\t';
  for (type_int type = 0u; type < p.TYPES_NO; ++type) {
    shapeFile << std::fixed << step << '\t' << meanRatioVec[type] << '\t'
              << msdRatioVec[type] << '\t' << minRatioVec[type] << '\t'
              << maxRatioVec[type] << '\t' << meanAreaVec[type] << '\t'
              << msdAreaVec[type] << '\t' << minAreaVec[type] << '\t'
              << maxAreaVec[type] << '\t' << meanPerimeterVec[type] << '\t'
              << msdPerimeterVec[type] << '\t' << minPerimeterVec[type] << '\t'
              << maxPerimeterVec[type] << '\t' << meanRadiusVec[type] << '\t'
              << meanMeanRadius2Vec[type] << '\t';
  }
  shapeFile << std::endl;
  return;
}

void
    oneSystem(void) {
  const Parameters &p = parameters();

  std::ofstream parametersFile((Date::compactRunTime + ".dat").c_str());
  parametersFile << getParameters() << std::endl;
  parametersFile.close();

  std::vector<Superboid> superboids(p.MAX_SUPERBOIDS);
  for (super_int index = 0u; index < p.SUPERBOIDS; ++index)
    superboids[index].activate();
  for (const auto &hole : parameters().STOKES_HOLES)
    for (auto &super : superboids)
      if (super.isActivated())
        for (const auto &mini : super.miniboids)
          if (hole.isInside(mini.position)) {
            super.setDeactivation("Began inside hole");
            super.deactivate();
            break;
          }

  if (InitialPositions::load())
    loadPositions(superboids);

  std::vector<Box> boxes(p.BOXES);
  for (auto &box : boxes)
    box.setNeighbors(boxes);

  for (auto &super : superboids)
    if (super.isActivated() == true)
      for (auto &mini : super.miniboids)
        mini.checkLimits();

  for (auto &super : superboids) {
    if (super.isActivated() == false)
      continue;
    for (auto &mini : super.miniboids)
      boxes[Box::getBoxID(mini.position)].append(mini);
  }

  if (p.BC == BoundaryCondition::PERIODIC)
    correctPositionAndRotation(superboids);

  for (auto &super : superboids) {
    if (super.isActivated() == false)
      continue;

    for (auto &mini : super.miniboids)
      mini.reset();

    super.setShape(0u);
  }

  step_int continuousStep = 0llu;
  step_int nextExitStep   = InitialPositions::startStep();

  std::ofstream phiFile;
  if (Phi::write())
    phiFile.open((Date::compactRunTime + "_phi.dat").c_str(), std::ios::out);

  std::ofstream scsFile;

  std::fstream gammaFile;
  if (Gamma::write())
    gammaFile.open((Date::compactRunTime + "_gamma.dat").c_str(),
                   std::ios::out);

  std::ofstream shapeFile;
  if (Shape::write()) {
    shapeFile.open((Date::compactRunTime + "_shape.dat").c_str());
    shapeFile << "#step \t"
              << "meanRatio\t"
              << "msdRatio\t"
              << "minRatio\t"
              << "maxRatio\t"
              << "meanArea\t"
              << "msdArea\t\t"
              << "minArea\t\t"
              << "maxArea\t\t"
              << "meanPerimeter\t"
              << "msdPerimeter\t"
              << "minPerimeter\t"
              << "maxPerimeter\t"
              << "meanRadius\t"
              << "meanRadius2\t" << std::endl;
  }

  bool keepStepLoop = true;
  for (step_int step = InitialPositions::startStep(); step <= p.STEPS; ++step) {
    if (keepStepLoop == false)
      break;

    bool gamma          = false;
    bool shape          = false;
    bool checkVirtuals  = false;
    checkVirtuals       = true;  ////
    bool exportVirtuals = false;
    if (Infinite::write())
      if (step + 1 == nextExitStep || step + 1 == p.STEPS)
        exportVirtuals = true;
    if (step == nextExitStep || step == p.STEPS || step == 0u) {
      std::cerr << "Step: " << step << std::endl;  ////
      exportLastPositionsAndVelocities(superboids, step);
      if (false)  // count cell neighbors.
      {
        super_int countNeighbors = 0u;
        for (auto &super : superboids)
          countNeighbors += super.cellNeighbors().size();
        std::cout << step << '\t'
                  << static_cast<real>(countNeighbors)
                         / static_cast<real>(p.SUPERBOIDS)
                  << std::endl;
      }

      if (MSD::write())
        exportMSD(superboids);

      if (BinPrint::write())
        binPrint(superboids);

      if (PlainPrint::write())
        plainPrint(superboids);

      if (Infinite::write())
        Infinite::write(superboids);

      if (Phi::write())
        exportPhi(phiFile, superboids);

      if (Shape::write())
        shape = true;

      if (SCS::write())
        SCS::write(step, superboids);

      if (Gamma::write() && step != 0u)
        gamma = true;

      ++continuousStep;

      if (p.EXIT_FACTOR < p.REAL_TOLERANCE) {
        nextExitStep += p.EXIT_INTERVAL;
      } else {
        step_int deltaExit = 1;
        if (step != 0)
          deltaExit = static_cast<step_int>(std::pow(step, p.EXIT_FACTOR));
        nextExitStep = step + deltaExit;
      }
    }

    auto error = nextStep(boxes, superboids, step, shape, gamma, checkVirtuals,
                          exportVirtuals);
    if (error != error::NextStepError::OK) {
      keepStepLoop = false;
      std::cerr << "this program will die soon. ";
      if (error == error::NextStepError::TOO_MANY_VIRTUALS_SINGLE_CELL)
        std::cerr << "TOO_MANY_VIRTUALS_SINGLE_CELL" << std::endl;
      else if (error == error::NextStepError::TOO_MANY_VIRTUALS_AVERAGE)
        std::cerr << "TOO_MANY_VIRTUALS_AVERAGE" << std::endl;
    }

    // Mean gamma measure.
    if (gamma == true) {
      real meanGamma     = -0.0f;
      super_int divideBy = 0u;
      for (auto &super : superboids)
        if (super.type == 0)
          if (super.doUseGamma == true) {
            meanGamma += super.gamma;
            ++divideBy;
          }
      meanGamma /= divideBy;

      gammaFile << std::fixed << step << '\t' << meanGamma << std::endl;
    }

    // Shape measures.
    if (shape == true)
      shapeIt(superboids, shapeFile, step);
  }

  gammaFile.close();
  shapeFile.close();
  if (InitialPositions::load())
    InitialPositions::file().close();
  if (MSD::write())
    MSD::file().close();
  if (SCS::write())
    SCS::file().close();
  if (Infinite::write())
    Infinite::close();

  return;
}

int
    main(int argc, char **argv) {
  setParameters();

  for (int aCount = 1; aCount < argc; ++aCount) {
    if (Argument::has(argv[aCount])) {
      auto &arg = Argument::get(argv[aCount]);
      arg.isSet = true;
      if (arg.secondArgument != "") {
        ++aCount;
        if (aCount == argc) {
          std::cerr << "expects one value for " << arg.argument << std::endl;
          return 5;
        }
        arg.valueSet = argv[aCount];
      }
    } else {
      std::cerr << argv[aCount] << " not recognized" << std::endl;
      return 6;
    }
  }

  for (const auto &arg : Argument::args())
    if (arg.skipMandatory)
      if (arg.isSet)
        return arg.function(arg.valueSet);

  if (!Argument::areAllMandatorySet()) {
    std::cerr << "not all mandatory arguments set." << std::endl;
    return 10;
  }
  for (const auto &arg : Argument::args())
    if (!arg.skipMandatory)
      if (arg.mandatory)
        if (arg.preventRunning)
          if (arg.isSet) {
            std::cerr << "impossible" << std::endl;
            return arg.function(arg.valueSet);
          }

  for (const auto &arg : Argument::args())
    if (!arg.skipMandatory)
      if (arg.mandatory)
        if (!arg.preventRunning)
          if (arg.isSet)
            arg.function(arg.valueSet);

  for (const auto &arg : Argument::args())
    if (!arg.skipMandatory)
      if (!arg.mandatory)
        if (arg.preventRunning)
          if (arg.isSet)
            return arg.function(arg.valueSet);

  for (const auto &arg : Argument::args())
    if (!arg.skipMandatory)
      if (!arg.mandatory)
        if (!arg.preventRunning)
          if (arg.isSet)
            arg.function(arg.valueSet);

  oneSystem();

  return 0;
}
