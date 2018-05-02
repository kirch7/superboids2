// Copyright (C) 2016-2018 Cássio Kirch.
// License specified in LICENSE file.

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
#include "Stokes.hpp" ////
#include "Parameter.hpp"

static void
oneSystem()
{
  const Parameters& p = parameters();
  
  std::ofstream parametersFile((Date::compactRunTime + ".dat").c_str());
  parametersFile << getParameters() << std::endl;
  parametersFile.close();

  std::vector<Superboid> superboids(p.MAX_SUPERBOIDS);
  for (super_int index = 0u;
       index < p.SUPERBOIDS;
       ++index)
    superboids[index].activated = true;

  //// std::vector<Superboid> superboids(1);
  //// superboids[0].activated = true;
  //// for (auto& mini : superboids[0].miniboids)
  ////   mini.position[X] += 5.5f;
  
  if (InitialPositions::load())
    loadPositions(superboids);
  
  std::vector<Box>       boxes(p.BOXES);
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

  if (p.BC == BoundaryCondition::PERIODIC)
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
  for (step_int step = InitialPositions::startStep(); step <= p.STEPS; ++step)
  {
    if (keepStepLoop == false)
      break;
    
    bool gamma = false;
    bool shape = false;
    bool checkVirtuals  = false;
    checkVirtuals = true; //// 
    bool exportVirtuals = false;
    if (Infinite::write())
      if (step + 1 == nextExitStep || step + 1 == p.STEPS)
	exportVirtuals = true;
    if (step == nextExitStep || step == p.STEPS || step == 0u)
    {
      std::cerr << "Step: " << step << std::endl; ////
      exportLastPositionsAndVelocities(superboids, step);
      if (false) // count cell neighbors.
      {
        super_int countNeighbors = 0u;
        for (auto& super : superboids)
          countNeighbors += super.cellNeighbors().size();
        std::cout << step << '\t' << static_cast<real>(countNeighbors) / static_cast<real>(p.SUPERBOIDS) << std::endl;
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

      if (p.EXIT_FACTOR < p.REAL_TOLERANCE)
      {
	nextExitStep += p.EXIT_INTERVAL;
      }
      else
      {
	step_int deltaExit = 1;
	if (step != 0)
	  deltaExit = static_cast<step_int>(std::pow(step, p.EXIT_FACTOR));
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
      
      std::vector<real> meanAreaVec(p.TYPES_NO);
      std::vector<real> meanPerimeterVec(p.TYPES_NO);
      std::vector<real> meanRatioVec(p.TYPES_NO);
      std::vector<real> meanRadiusVec(p.TYPES_NO);
      for (type_int type = 0u; type < p.TYPES_NO; ++type)
      {
	meanAreaVec[type]      = areaSumVec[type] / cellsActivatedNo;
	meanPerimeterVec[type] = perimeterSumVec[type] / cellsActivatedNo;
	meanRatioVec[type]     = ratioSumVec[type] / cellsActivatedNo;
	meanRadiusVec[type]    = radiusSumVec[type] / (cellsActivatedNo /** MINIBOID_PER_SUPERBOID*/);
      }
      
      std::vector<real> msdPerimeterVec(p.TYPES_NO, -0.0f);
      std::vector<real> msdRatioVec(p.TYPES_NO, -0.0f);
      std::vector<real> msdAreaVec(p.TYPES_NO, -0.0f);
      
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

      std::vector<real> meanMeanRadius2Vec(p.TYPES_NO);
      for (type_int type = 0u; type < p.TYPES_NO; ++type)
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
      for (type_int type = 0u; type < p.TYPES_NO; ++type)
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

// int
// main(int argc, char** argv)
// {
//   setParameters();
//   int problem(0);

//   if (argc > 1)
//   {
//     std::list<std::vector<Argument> > listOfLists;
//     listOfLists.push_back(getDontRunList());
//     listOfLists.push_back(getDoRunList());
//     listOfLists.push_back(getMandatoryList());
    
//     const uint16_t possibleArgumentsNo(getDoRunList().size() + getDontRunList().size() + getMandatoryList().size());

//     // Check if there is any invalid argument.
//     // If there is any, the program ends.
//     for (int argvCount = 1; argvCount < argc; ++argvCount)
//     {
//       uint16_t wrongArgumentsNo(0u);
//       for (const auto& list : listOfLists)
//         for (const auto& arg : list)
//         {
//           if (argv[argvCount] != arg.argument)
//             //if (std::string("-initial") != argv[argvCount-1] && std::string("-param") != argv[argvCount-1])
//             if (argv[argvCount-1] != std::string("-initial") && argv[argvCount-1] != std::string("-param"))
//               ++wrongArgumentsNo;
//         }
      
//       if (wrongArgumentsNo == possibleArgumentsNo)
//       {
//         std::cerr << "Invalid argument: " << argv[argvCount] << std::endl;
//         return 1;
//       }
//     }

//     // Check if there is some parameter getter argument.
//     for (int argvCount = 1; argvCount < argc; ++argvCount)
//       for (const auto& arg : getDontRunList())
//         if (arg.argument == argv[argvCount])
//         {
//           arg.function(nullptr);
//           return 0;
//         }
//   }

//   {
//     const auto mandatory = getMandatoryList();
//     const std::vector<unsigned> OK(mandatory.size(), 1);
//     std::vector<unsigned> ok(mandatory.size(), 0);
//     for (int argvCount = 1; argvCount < argc; ++argvCount)
//       for (std::size_t mandCount = 0; mandCount < mandatory.size(); ++mandCount)
// 	if (mandatory[mandCount].argument == argv[argvCount])
// 	  ++ok[mandCount];
//     if (ok != OK)
//     {
//       std::cerr << "mandatory argment not present or duplicated." << std::endl;
//       return 1;
//     }
//     //// Must be generalized.
//     for (int argvCount = 1; argvCount < argc; ++argvCount)
//       for (const auto& arg : getMandatoryList())
// 	if (argv[argvCount] == arg.argument)
// 	  arg.function(argv[argvCount + 1]);
//   }
  
//   if (argc > 1)
//   {
//     // Check if there is some 'execute about' argument.
//     for (int argvCount = 1; argvCount < argc; ++ argvCount)
//       for (const auto& arg : getDoRunList())
//         if (arg.argument == argv[argvCount])
//         {
//           //if (argvCount >= (argc - 1) &&
// 	  // (argv[argvCount] == std::string("-initial") || argv[argvCount-1] == std::string("-param")))
//           if (argvCount >= (argc - 1) && (argv[argvCount] == std::string("-initial")))
//           {
//             std::cerr << "Please, enter with a valid path." << std::endl;
//             return (2);
//           }
//           else
//             problem += arg.function(argv[argvCount + 1]);
//         }
//   }
  
//   if (argc != 1 && problem)
//     return problem;
//   else
//   {
//     ////distances();
//     oneSystem();
//     return (0);
//   }
// }

int
main(int argc, char** argv)
{
  setParameters();

  for (int aCount = 1; aCount < argc; ++aCount)
  {
    if (Argument::has(argv[aCount]))
    {
      auto& arg = Argument::get(argv[aCount]);
      arg.isSet = true;
      if (arg.secondArgument != "")
      {
	++aCount;
	if (aCount == argc)
	{
	  std::cerr << "expects one value for " << arg.argument << std::endl;
	  return 5;
	}
	arg.valueSet = argv[aCount];
      }
    }
    else
    {
      std::cerr << argv[aCount] << " not recognized" << std::endl;
      return 6;
    }
  }

  for (const auto& arg : Argument::args())
    if (arg.skipMandatory)
      if (arg.isSet)
	return arg.function(arg.valueSet);
  
  if (!Argument::areAllMandatorySet())
  {
    std::cerr << "not all mandatory arguments set." << std::endl;
    return 10;
  }
  for (const auto& arg : Argument::args())
    if (!arg.skipMandatory)
      if (arg.mandatory)
	if (arg.preventRunning)
	  if (arg.isSet)
	  {
	    std::cerr << "impossible" << std::endl;
	    return arg.function(arg.valueSet);
	  }
  
  for (const auto& arg : Argument::args())
    if (!arg.skipMandatory)
      if (arg.mandatory)
	if (!arg.preventRunning)
	  if (arg.isSet)
	    arg.function(arg.valueSet);
  
  for (const auto& arg : Argument::args())
    if (!arg.skipMandatory)
      if (!arg.mandatory)
	if (arg.preventRunning)
	  if (arg.isSet)
	    return arg.function(arg.valueSet);
  
  for (const auto& arg : Argument::args())
    if (!arg.skipMandatory)
      if (!arg.mandatory)
	if (!arg.preventRunning)
	  if (arg.isSet)
	    arg.function(arg.valueSet);

  oneSystem();
  
  return 0;
}
