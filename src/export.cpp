// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#include "export.hpp"
#include "Date.hpp"
#include "Distance.hpp"
#include "Superboid.hpp"
#include "load.hpp"
#include "parameters.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <valarray>
#include <vector>

static void writeMSDHead(std::ofstream &myFile) {
  for (const auto i : std::vector<unsigned long int>(
           {parameters().STEPS, parameters().EXIT_INTERVAL,
            parameters().DIMENSIONS}))
    myFile << i << std::endl;

  for (const auto i : std::vector<real>({parameters().DT, parameters().RANGE}))
    myFile << i << std::endl;

  return;
}

static void writeBinPrintHead(std::ofstream &myFile) {
  for (const auto i : std::vector<unsigned long int>(
           {parameters().STEPS, parameters().EXIT_INTERVAL,
            parameters().DIMENSIONS}))
    myFile << i << std::endl;

  for (const auto i : std::vector<real>({parameters().DT, parameters().RANGE}))
    myFile << i << std::endl;

  return;
}

void exportMSD(std::vector<Superboid> &superboids) {
  exportMSD(MSD::file(), superboids);
  return;
}

void exportMSD(std::ofstream &myFile, std::vector<Superboid> &superboids) {
  static bool firstTime = true;
  if (firstTime) {
    firstTime = false;
    writeMSDHead(myFile);
  }

  super_int activatedNo = 0u;
  for (auto &super : superboids)
    if (super.isActivated() == true)
      ++activatedNo;

  uint16_t activated = static_cast<uint16_t>(activatedNo);
  myFile.write(reinterpret_cast<char *>(&activated), sizeof(activated));

  for (auto &super : superboids) {
    if (super.isActivated() == false)
      continue;
    const std::valarray<real> &position = super.miniboids[0u].position;
    for (dimension_int dim = 0u; dim < parameters().DIMENSIONS; ++dim) {
      float dComp = static_cast<float>(position[dim]);
      myFile.write(reinterpret_cast<char *>(&dComp), sizeof(dComp));
    }

    uint16_t type = static_cast<uint16_t>(super.type);
    myFile.write(reinterpret_cast<char *>(&type), sizeof(type));
    uint16_t neiNo = static_cast<uint16_t>(super.cellNeighbors().size());
    myFile.write(reinterpret_cast<char *>(&neiNo), sizeof(neiNo));
    float coreSize = static_cast<float>(parameters().PRINT_CORE);
    myFile.write(reinterpret_cast<char *>(&coreSize), sizeof(coreSize));
  }
  return;
}

void binPrint(std::vector<Superboid> &superboids) {
  binPrint(BinPrint::file(), superboids);
  return;
}

void binPrint(std::ofstream &myFile, std::vector<Superboid> &superboids) {
  static bool firstTime = true;
  if (firstTime) {
    firstTime = false;
    writeBinPrintHead(myFile);
  }

  super_int activatedNo = 0u;
  for (auto &super : superboids)
    if (super.isActivated() == true)
      ++activatedNo;

  uint16_t activated =
      static_cast<uint16_t>(activatedNo * parameters().MINIBOIDS_PER_SUPERBOID);
  myFile.write(reinterpret_cast<char *>(&activated), sizeof(activated));

  for (auto &super : superboids) {
    if (super.isActivated() == false)
      continue;

    for (const auto &mini : super.miniboids) {
      const std::valarray<real> &position = mini.position;
      for (dimension_int dim = 0u; dim < parameters().DIMENSIONS; ++dim) {
        float dComp = static_cast<float>(position[dim]);
        myFile.write(reinterpret_cast<char *>(&dComp), sizeof(dComp));
      }

      uint16_t type = static_cast<uint16_t>(super.type);
      myFile.write(reinterpret_cast<char *>(&type), sizeof(type));
      uint16_t neiNo = static_cast<uint16_t>(super.cellNeighbors().size());
      myFile.write(reinterpret_cast<char *>(&neiNo), sizeof(neiNo));
      float coreSize =
          static_cast<float>(mini.ID == 0 ? (3.0 * parameters().PRINT_CORE)
                                          : parameters().PRINT_CORE);
      myFile.write(reinterpret_cast<char *>(&coreSize), sizeof(coreSize));
    }
  }

  myFile.flush();

  return;
}

void exportLastPositionsAndVelocities(const std::vector<Superboid> &superboids,
                                      const step_int step) {
  const std::string fileNameBase = Date::compactRunTime;
  const std::string fileName = fileNameBase + std::string("_last.bin");

  if (step != InitialPositions::startStep() &&
      rename(fileName.c_str(), (fileNameBase + "_lastButOne.bin").c_str()) != 0)
    throw std::runtime_error("Could not move last file to last but one.");

  std::ofstream binaryOutFile(fileName.c_str(),
                              std::ofstream::out | std::ofstream::binary);
  binaryOutFile << step << std::endl;
  super_int activatedCellsNo = 0;
  for (const auto &super : superboids)
    if (super.isActivated() == true)
      ++activatedCellsNo;

  binaryOutFile << activatedCellsNo << std::endl;
  for (auto &super : superboids) {
    if (super.isActivated() == false)
      continue;

    binaryOutFile << super.type << std::endl;
    for (auto &mini : super.miniboids) {
      for (auto &component : mini.position)
        binaryOutFile.write(reinterpret_cast<const char *>(&component),
                            sizeof(real));
      for (auto &component : mini.velocity)
        binaryOutFile.write(reinterpret_cast<const char *>(&component),
                            sizeof(real));
    }
  }
  binaryOutFile.close();
}

void exportPhi(std::ofstream &file, const std::vector<Superboid> &superboids) {
  std::valarray<real> meanArray(-0.0, parameters().DIMENSIONS);
  for (const auto &super : superboids) {
    if (super.isActivated() == false)
      continue;

    meanArray += super.miniboids[0u].velocity /
                 (parameters().SUPERBOIDS * parameters().SPEED[super.type]);
  }

  real arraySum = -0.0;
  for (const auto &i : meanArray)
    arraySum += square(i);

  file << std::sqrt(arraySum) << std::endl;

  return;
}

void SCS::write(const step_int step, const std::vector<Superboid> &superboids) {
  for (auto &super : superboids) {
    if (super.isActivated() == false)
      continue;

    std::valarray<real> peripheralsCM(-0.0, parameters().DIMENSIONS);
    const mini_int PERIPHERAL_NO = parameters().MINIBOIDS_PER_SUPERBOID - 1;
    for (auto &mini : super.miniboids) {
      if (mini.ID == 0)
        continue;
      peripheralsCM += mini.position;
    }
    peripheralsCM /= static_cast<real>(PERIPHERAL_NO);

    SCS::file() << std::fixed << step << '\t' << super.ID << '\t'
                << getModule(super.miniboids[0u].position, peripheralsCM)
                << std::endl;
  }
  return;
}

void Infinite::write(std::vector<Superboid> &superboids) {
  std::ofstream &infinite2File = inf2File();
  std::ofstream &infiniteFile = infFile();
  std::ofstream &virtFile = virtualsFile();

  infiniteFile << '#' << std::endl;
  infinite2File << '#' << std::endl;
  virtFile << '#' << std::endl;
  for (auto &super : superboids) {
    if (super.isActivated() == false)
      continue;

    for (const auto &va : super.infiniteVectors)
      infiniteFile << va << std::endl;
    for (const auto &va : super.infinite2Vectors)
      infinite2File << va << std::endl;
    virtFile << super.virtualsInfo.str();
  }

  infinite2File << std::endl << std::endl << std::endl;
  infiniteFile << std::endl << std::endl << std::endl;
  virtFile << std::endl << std::endl << std::endl;
}

void Infinite::close() {
  _infFile.close();
  _inf2File.close();
  _virtFile.close();
}

bool MSD::_export(false);
std::ofstream MSD::_file;

bool BinPrint::_export(false);
std::ofstream BinPrint::_file;

bool Infinite::_export(false);
std::ofstream Infinite::_infFile;
std::ofstream Infinite::_inf2File;
std::ofstream Infinite::_virtFile;

bool Shape::_export(false);
bool Gamma::_export(false);
bool Phi::_export(false);
bool SCS::_export(false);
std::ofstream SCS::_file;
