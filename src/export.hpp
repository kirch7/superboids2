// Copyright (C) 2016-2018 Cássio Kirch.
// Copyright (C) 2018 Leonardo Gregory Brunnet.
// License specified in LICENSE file.

#pragma once
#include <vector>
#include "parameters.hpp"

class Superboid;

class MSD {
 public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static inline bool write(void) { return _export; }
  static inline std::ofstream &file(void) { return _file; }
  virtual inline ~MSD() { ; }
  friend int setMSD(const std::string &);

 private:
  static std::ofstream _file;
  static bool _export;
};

class BinPrint {
 public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static inline bool write(void) { return _export; }
  static inline std::ofstream &file(void) { return _file; }
  virtual inline ~BinPrint() { ; }
  friend int setBinPrint(const std::string &);

 private:
  static std::ofstream _file;
  static bool _export;
};

class Shape {
 public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static inline bool write(void) { return _export; }
  friend int setShapeExportation(const std::string &);

 private:
  static bool _export;
};

class Gamma {
 public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static inline bool write(void) { return _export; }
  friend int setGammaExportation(const std::string &);

 private:
  static bool _export;
};

class SCS {
 public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static inline bool write(void) { return _export; }
  friend int setSCS(const std::string &);
  static void write(const step_int step,
                    const std::vector<Superboid> &superboids);
  static inline std::ofstream &file(void) { return _file; }

 private:
  static std::ofstream _file;
  static bool _export;
};

class Infinite {
 public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static inline bool write(void) { return _export; }
  friend int setInfinite(const std::string &);
  static void write(std::vector<Superboid> &superboids);
  static inline std::ofstream &infFile(void) { return _infFile; }
  static inline std::ofstream &inf2File(void) { return _inf2File; }
  static inline std::ofstream &virtualsFile(void) { return _virtFile; }
  static void close(void);

 private:
  static std::ofstream _infFile;
  static std::ofstream _inf2File;
  static std::ofstream _virtFile;
  static bool _export;
};

class Phi {
 public:
  virtual void youCannotMakeAInstanceOfMe(void) = 0;
  static inline bool write(void) { return _export; }
  friend int setPhi(const std::string &);

 private:
  static bool _export;
};

extern void exportMSD(std::ofstream &, std::vector<Superboid> &);
extern void exportMSD(std::vector<Superboid> &);
extern void binPrint(std::vector<Superboid> &);
extern void binPrint(std::ofstream &, std::vector<Superboid> &);
extern void exportPhi(std::ofstream &, const std::vector<Superboid> &);
extern void exportLastPositionsAndVelocities(const std::vector<Superboid> &,
                                             const step_int);
