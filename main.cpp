// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

#include <iostream>
#include <vector>
#include <thread>
#include <fstream>
#include <limits>
#include "export.hpp"
#include "load.hpp"
#include "Date.hpp"
#include "Superboid.hpp"
#include "Argument.hpp"
#include "parameters.hpp"
#include "Box.hpp"
#include "nextstep.hpp"

static void
oneSystem()
{
  std::ofstream parametersFile((Date::compactRunTime + ".dat").c_str());
  parametersFile << getParameters() << std::endl;
  parametersFile.close();
  
  std::vector<Superboid> superboids(MAX_SUPERBOIDS);
  for (super_int index = 0; index < SUPERBOIDS; ++index)
    superboids[index].activated = true;

  if (InitialPositions::load())
    loadPositions(superboids);
  
  std::vector<Box>       boxes(BOXES);
  for (auto& box: boxes)
    box.setNeighbors(boxes);

  for (auto& super : superboids)
    if (super.activated == true)
      for (auto& mini : super.miniboids)
	mini.checkLimits();
  
  for (auto& super : superboids)
  {
    if (super.activated == false)
      continue;
    for (auto& mini : super.miniboids)
      boxes[Box::getBoxID(mini.position)].append(mini);
  }

  if (BC == BoundaryCondition::PERIODIC)
    correctPositionAndRotation(superboids);

  for (auto& super : superboids)
  {
    if (super.activated == false)
      continue;
    
    for (auto& mini : super.miniboids)
      mini.reset();
    if (Shape::write())
      super.setShape(0u);
  }
  
  step_int continuousStep = 0llu;
  step_int nextExitStep = InitialPositions::startStep();

  std::ofstream phiFile;
  if (Phi::write())
    phiFile.open((Date::compactRunTime + "_phi.dat").c_str(), std::ios::out);

  std::ofstream scsFile;

  std::fstream gammaFile;
  if (Gamma::write())
    gammaFile.open((Date::compactRunTime + "_gamma.dat").c_str(), std::ios::out);

  std::ofstream shapeFile;
  if (Shape::write())
  {
    shapeFile.open((Date::compactRunTime + "_shape.dat").c_str());
    shapeFile << "#step \t" <<
      "meanRatio\t" << "msdRatio\t" << "minRatio\t" << "maxRatio\t" <<
      "meanArea\t" << "msdArea\t\t" << "minArea\t\t" << "maxArea\t\t" <<
      "meanPerimeter\t" << "msdPerimeter\t" << "minPerimeter\t" << "maxPerimeter\t" <<
      "meanRadius\t" << "meanRadius2\t" << 
      std::endl;
  }
  
  bool keepStepLoop = true;
  for (step_int step = InitialPositions::startStep(); step <= STEPS; ++step)
  {
    if (keepStepLoop == false)
      break;
    
    bool gamma = false;
    bool shape = false;
    bool checkVirtuals  = false;
    checkVirtuals = true; //// 
    bool exportVirtuals = false;
    if (Infinite::write())
      if (step + 1 == nextExitStep || step + 1 == STEPS)
	exportVirtuals = true;
    if (step == nextExitStep || step == STEPS || step == 0u)
    {
      std::cerr << "Step: " << step << std::endl; ////
      exportLastPositionsAndVelocities(superboids, step);
      if (false) // count cell neighbors.
      {
        super_int countNeighbors = 0u;
        for (auto& super : superboids)
          countNeighbors += super.cellNeighbors().size();
        std::cout << step << '\t' << static_cast<real>(countNeighbors) / static_cast<real>(SUPERBOIDS) << std::endl;
      }

      if (MSD::write())
        exportMSD(superboids);
      
      if (BinPrint::write())
        binPrint(superboids);

      if (Infinite::write())
	Infinite::write(superboids);
      
      if (Phi::write())
        exportPhi(phiFile, superboids);
      
      if (Shape::write())
        shape = true;
 
      if (SCS::write())
        SCS::write(step, superboids);
      
      if(Gamma::write() && step != 0u)
        gamma = true;

      //// checkVirtuals = true;
      ++continuousStep;

      if (EXIT_FACTOR < REAL_TOLERANCE)
      {
	nextExitStep += EXIT_INTERVAL;
      }
      else
      {
	step_int deltaExit = 1;
	if (step != 0)
	  deltaExit = static_cast<step_int>(std::pow(step, EXIT_FACTOR));
	nextExitStep = step + deltaExit;
      }
    }
    
    if (nextStepOK(boxes, superboids, step, shape, gamma, checkVirtuals, exportVirtuals) == false)
      keepStepLoop = false;
    
    // Mean gamma measure.
    if (gamma == true)
    {
      real meanGamma = -0.0f;
      super_int divideBy = 0u;
      for (auto& super : superboids)
        if (super.type == 0)
          if (super.doUseGamma == true)
          {
            meanGamma += super.gamma;
            ++divideBy;
          }
      meanGamma /= divideBy;
      
      gammaFile << std::fixed << step << '\t' << meanGamma << std::endl;
    }

    // Shape measures.
    if (shape == true)
    {
      const real smallestFloat = static_cast<real>(std::numeric_limits<float>().min());
      const real biggestFloat  = static_cast<real>(std::numeric_limits<float>().max());
      real ratioSum      = -0.0f;
      real maxRatio      = smallestFloat;
      real minRatio      = biggestFloat;
      real perimeterSum  = -0.0f;
      real maxPerimeter  = smallestFloat;
      real minPerimeter  = biggestFloat;
      real areaSum       = -0.0f;
      real maxArea       = smallestFloat;
      real minArea       = biggestFloat;
      real radiusSum     = -0.0f;
      real maxRadius     = smallestFloat;
      real minRadius     = biggestFloat;
      real radius2Sum    = -0.0f;

      std::vector<real> ratioSumVec(TYPES_NO, -0.0f);
      std::vector<real> maxRatioVec(TYPES_NO, smallestFloat);
      std::vector<real> minRatioVec(TYPES_NO, biggestFloat);
      std::vector<real> perimeterSumVec(TYPES_NO, -0.0f);
      std::vector<real> maxPerimeterVec(TYPES_NO, smallestFloat);
      std::vector<real> minPerimeterVec(TYPES_NO, biggestFloat);
      std::vector<real> areaSumVec(TYPES_NO, -0.0f);
      std::vector<real> maxAreaVec(TYPES_NO, smallestFloat);
      std::vector<real> minAreaVec(TYPES_NO, biggestFloat);
      std::vector<real> radiusSumVec(TYPES_NO, -0.0f);
      std::vector<real> maxRadiusVec(TYPES_NO, smallestFloat);
      std::vector<real> minRadiusVec(TYPES_NO, biggestFloat);
      std::vector<real> radius2SumVec(TYPES_NO, -0.0f);

      super_int cellsActivatedNo = 0;
      for (const auto& super : superboids)
      {
	if (super.activated == false)
	  continue;

	++cellsActivatedNo;
	areaSum                     += super.area;
	perimeterSum                += super.perimeter;
	const real ratio             = super.perimeter / std::sqrt(super.area);
        ratioSum                    += ratio;
        radiusSum                   += super.meanRadius;
        radius2Sum                  += super.meanRadius2;

	areaSumVec[super.type]      += super.area;
        perimeterSumVec[super.type] += super.perimeter;
        ratioSumVec[super.type]     += ratio;
        radiusSumVec[super.type]    += super.meanRadius;
        radius2SumVec[super.type]   += super.meanRadius2;

        if (ratio < minRatio)                              minRatio     = ratio;
        if (ratio > maxRatio)                              maxRatio     = ratio;
        if (super.area < minArea)                          minArea      = super.area;
        if (super.area > maxArea)                          maxArea      = super.area;
        if (super.perimeter < minPerimeter)                minPerimeter = super.perimeter;
        if (super.perimeter > maxPerimeter)                maxPerimeter = super.perimeter;
        if (super.meanRadius < minRadius)                  minRadius    = super.meanRadius;
        if (super.meanRadius > maxRadius)                  maxRadius    = super.meanRadius;

	if (ratio < minRatioVec[super.type])               minRatio     = ratio;
        if (ratio > maxRatioVec[super.type])               maxRatio     = ratio;
        if (super.area < minAreaVec[super.type])           minArea      = super.area;
        if (super.area > maxAreaVec[super.type])           maxArea      = super.area;
        if (super.perimeter < minPerimeterVec[super.type]) minPerimeter = super.perimeter;
        if (super.perimeter > maxPerimeterVec[super.type]) maxPerimeter = super.perimeter;
        if (super.meanRadius < minRadiusVec[super.type])   minRadius    = super.meanRadius;
        if (super.meanRadius > maxRadiusVec[super.type])   maxRadius    = super.meanRadius;
      }
      const real meanArea = areaSum / cellsActivatedNo;
      const real meanPerimeter = perimeterSum / cellsActivatedNo;
      const real meanRatio = ratioSum / cellsActivatedNo;
      const real meanRadius = radiusSum / (cellsActivatedNo /** MINIBOID_PER_SUPERBOID*/);

      real msdPerimeter = -0.0f;
      real msdRatio = -0.0f;
      real msdArea = -0.0f;
      
      std::vector<real> meanAreaVec(TYPES_NO);
      std::vector<real> meanPerimeterVec(TYPES_NO);
      std::vector<real> meanRatioVec(TYPES_NO);
      std::vector<real> meanRadiusVec(TYPES_NO);
      for (type_int type = 0u; type < TYPES_NO; ++type)
      {
	meanAreaVec[type]      = areaSumVec[type] / cellsActivatedNo;
	meanPerimeterVec[type] = perimeterSumVec[type] / cellsActivatedNo;
	meanRatioVec[type]     = ratioSumVec[type] / cellsActivatedNo;
	meanRadiusVec[type]    = radiusSumVec[type] / (cellsActivatedNo /** MINIBOID_PER_SUPERBOID*/);
      }
      
      std::vector<real> msdPerimeterVec(TYPES_NO, -0.0f);
      std::vector<real> msdRatioVec(TYPES_NO, -0.0f);
      std::vector<real> msdAreaVec(TYPES_NO, -0.0f);
      
      for (const auto& super : superboids)
      {
	if (super.activated == false)
	  continue;
	
        msdPerimeter += square(super.perimeter - meanPerimeter);
        msdArea += square(super.area - meanArea);
        msdRatio += square(super.perimeter / std::sqrt(super.area) - meanRatio);

        msdPerimeterVec[super.type] += square(super.perimeter - meanPerimeterVec[super.type]);
        msdAreaVec[super.type]      += square(super.area - meanAreaVec[super.type]);
        msdRatioVec[super.type]     += square(super.perimeter / std::sqrt(super.area) - meanRatioVec[super.type]);
      }

      msdArea /= cellsActivatedNo;
      msdArea = std::sqrt(msdArea);
      msdPerimeter /= cellsActivatedNo;
      msdPerimeter = std::sqrt(msdPerimeter);
      msdRatio /= cellsActivatedNo;
      msdRatio = std::sqrt(msdRatio);
      real meanMeanRadius2 = radius2Sum / cellsActivatedNo;

      std::vector<real> meanMeanRadius2Vec(TYPES_NO);
      for (type_int type = 0u; type < TYPES_NO; ++type)
      {
	msdAreaVec[type] /= cellsActivatedNo;
	msdAreaVec[type] = std::sqrt(msdAreaVec[type]);
	msdPerimeterVec[type] /= cellsActivatedNo;
	msdPerimeterVec[type] = std::sqrt(msdPerimeterVec[type]);
	msdRatioVec[type] /= cellsActivatedNo;
	msdRatioVec[type] = std::sqrt(msdRatioVec[type]);
	meanMeanRadius2Vec[type] = radius2SumVec[type] / cellsActivatedNo;
      }
      
      shapeFile << std::fixed << step << '\t' <<
        meanRatio << '\t' <<
	msdRatio << '\t' <<
	minRatio << '\t' <<
	maxRatio << '\t' <<
        meanArea << '\t' <<
	msdArea << '\t' <<
	minArea << '\t' <<
	maxArea << '\t' <<
        meanPerimeter << '\t' <<
	msdPerimeter << '\t' <<
	minPerimeter << '\t' <<
	maxPerimeter << '\t' <<
        meanRadius << '\t' <<
	meanMeanRadius2 << '\t';
      for (type_int type = 0u; type < TYPES_NO; ++type)
      {
	shapeFile << std::fixed << step << '\t' <<
	  meanRatioVec[type]       << '\t' <<
	  msdRatioVec[type]        << '\t' <<
	  minRatioVec[type]        << '\t' <<
	  maxRatioVec[type]        << '\t' <<
	  meanAreaVec[type]        << '\t' <<
	  msdAreaVec[type]         << '\t' <<
	  minAreaVec[type]         << '\t' <<
	  maxAreaVec[type]         << '\t' <<
	  meanPerimeterVec[type]   << '\t' <<
	  msdPerimeterVec[type]    << '\t' <<
	  minPerimeterVec[type]    << '\t' <<
	  maxPerimeterVec[type]    << '\t' <<
	  meanRadiusVec[type]      << '\t' <<
	  meanMeanRadius2Vec[type] << '\t';
      }	
      shapeFile << std::endl;
    }
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
  /*for (const auto& box : boxes)
  {
    real density = box.getDensity();
    if (density > 0.2)
      std::cout << box.ID << '\t' << box.getDensity() << std::endl;
  }*/
  
  return;
}

int
main(int argc, char** argv)
{
  int problem(0);
  
  if (argc > 1)
  {
    std::list<std::list<Argument> > listOfLists;
    listOfLists.push_back(getDontRunList());
    listOfLists.push_back(getDoRunList());

    const uint16_t possibleArgumentsNo(getDoRunList().size() + getDontRunList().size());

    // Check if there is any invalid argument.
    // If there is any, the program ends.
    for (int argvCount = 1; argvCount < argc; ++argvCount)
    {
      uint16_t wrongArgumentsNo(0u);
      for (const auto& list : listOfLists)
        for (const auto& arg : list)
        {
          if (argv[argvCount] != arg.argument)
            if (argv[argvCount-1] != std::string("-initial"))
              ++wrongArgumentsNo;
        }
      if (wrongArgumentsNo == possibleArgumentsNo)
      {
        std::cerr << "Invalid argument: " << argv[argvCount] << std::endl;
        return 1;
      }
    }

    // Check if there is some parameter getter argument.
    for (int argvCount = 1; argvCount < argc; ++argvCount)
      for (const auto& arg : getDontRunList())
        if (arg.argument == argv[argvCount])
        {
          arg.function(nullptr);
          return 0;
        }

    // Check if there is some 'execute about' argument.
    for (int argvCount = 1; argvCount < argc; ++ argvCount)
      for (const auto& arg : getDoRunList())
        if (arg.argument == argv[argvCount])
        {
          if (argvCount >= (argc - 1) && (argv[argvCount] == std::string("-initial")))
          {
            std::cerr << "Please, enter with a valid path." << std::endl;
            return (2);
          }
          else
            problem += arg.function(argv[argvCount + 1]);
        }
  }
  
  if (argc != 1 && problem)
    return problem;
  else
  {
    ////distances();
    oneSystem();
    return (0);
  }
}
