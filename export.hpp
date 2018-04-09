// Copyright (C) 2016-2017 Cássio Kirch.
// Permission is granted to copy, distribute and/or modify this document
// under the terms of the GNU Free Documentation License, Version 1.3
// or any later version published by the Free Software Foundation;
// with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
// A copy of the license is included in the section entitled "GNU
// Free Documentation License".

#pragma once
#include <vector>
#include "parameters.hpp"

class Superboid;

class MSD
{
public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static inline bool write(void) { return _export; }
  static inline std::ofstream& file(void) { return _file; }
  virtual inline ~MSD() {;}
  friend int setMSD(const void* const);
private:
  static std::ofstream _file;
  static bool _export;
};

class BinPrint
{
public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static inline bool write(void) { return _export; }
  static inline std::ofstream& file(void) { return _file; }
  virtual inline ~BinPrint() {;}
  friend int setBinPrint(const void* const);
private:
  static std::ofstream _file;
  static bool _export;
};

class Shape
{
public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static inline bool write(void) { return _export; }
  friend int setShapeExportation(const void* const);
private:
  static bool _export;
};

class Gamma
{
public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static inline bool write(void) { return _export; }
  friend int setGammaExportation(const void* const);
private:
  static bool _export;
};

class SCS
{
public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static inline bool write(void) { return _export; }
  friend int setSCS(const void* const);
  static void write(const step_int step, const std::vector<Superboid>& superboids);
  static inline std::ofstream& file(void) { return _file; }
private:
  static std::ofstream _file;
  static bool _export;
};

class Infinite
{
public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static inline bool write(void) { return _export; }
  friend int setInfinite(const void* const);
  static void write(std::vector<Superboid>& superboids);
  static inline std::ofstream& infFile(void) { return _infFile; }
  static inline std::ofstream& inf2File(void) { return _inf2File; }
  static inline std::ofstream& virtualsFile(void) { return _virtFile; }
  static void close(void);
private:
  static std::ofstream _infFile;
  static std::ofstream _inf2File;
  static std::ofstream _virtFile;
  static bool _export;
};

class Phi
{
public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static inline bool write(void) { return _export; }
  friend int setPhi(const void* const);
private:
  static bool _export;
};

extern void exportMSD(std::ofstream&, std::vector<Superboid>&);
extern void exportMSD(std::vector<Superboid>&);
extern void binPrint(std::vector<Superboid>&);
extern void binPrint(std::ofstream&, std::vector<Superboid>&);
//// extern void exportPhi(std::vector<Superboid>&);
extern void exportPhi(std::ofstream&, const std::vector<Superboid>&);
extern void exportLastPositionsAndVelocities(const std::vector<Superboid>&, const step_int);
