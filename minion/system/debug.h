// Minion https://github.com/minion/minion
// SPDX-License-Identifier: MPL-2.0

#ifndef DEBUG_H
#define DEBUG_H

#include "basic_headers.h"

void outputFatalError(string s) DOM_NORETURN;

template <typename T>
inline void CheckNotBound(const T& t, std::string s, std::string s2 = "") {
  for(SysInt i = 0; i < (SysInt)t.size(); ++i) {
    if(t[i].isBound()) {
      ostringstream oss;
      oss << "Cannot use '" << s << "' with BOUND or SPARSEBOUND variables.\n";
      if(s2 != "")
        oss << "Please use '" << s2 << "' as a replacement or";
      oss << "Please use DISCRETE variables instead.\n";
      outputFatalError(oss.str());
    }
  }
}

template <typename T>
inline void CheckNotBoundSingle(const T& t, std::string s, std::string s2 = "") {
  if(t.isBound()) {
    ostringstream oss;
    oss << "Cannot use " << s << " with BOUND or SPARSEBOUND variables.\n";
    if(s2 != "")
      oss << "Please use " << s2 << " as a replacement or ";
    oss << "Please use DISCRETE variables instead.\n";
    outputFatalError(oss.str());
  }
}

#ifndef MINION_DEBUG
#ifndef NO_DEBUG
#define NO_DEBUG
#endif
#endif

#define D_FATAL_ERROR(s)                                                                           \
  {                                                                                                \
    D_FATAL_ERROR2(s, __FILE__, tostring(__LINE__));                                               \
    throw 0;                                                                                       \
  }

#define INPUT_ERROR(s)                                                                             \
  {                                                                                                \
    cout << "There was a problem in your input file:\n" << s << endl;                              \
    exit(1);                                                                                       \
  }
// These functions are defined in debug_functions.cpp

void D_FATAL_ERROR2(string s, string file, string line);
void DOM_NORETURN FAIL_EXIT(string s = "");

struct assert_fail {};

void errorPrintingFunction(std::string a, std::string f, SysInt line) DOM_NORETURN;
void userErrorPrintingFunction(std::string a, std::string f, SysInt line) DOM_NORETURN;

void FATAL_REPORTABLE_ERROR() DOM_NORETURN;

#define CHECK(x, y)                                                                                \
  {                                                                                                \
    if(!(x)) {                                                                                     \
      userErrorPrintingFunction(y, __FILE__, __LINE__);                                         \
    }                                                                                              \
  }

// Check a value doesn't overflow, to be used in ctor of cts
#define CHECKSIZE(x, message)                                                                      \
  CHECK(x <= ((BigInt)checked_cast<SysInt>(DomainInt_Max)) &&                                      \
            x >= ((BigInt)checked_cast<SysInt>(DomainInt_Min)),                                    \
        message)

#ifdef MINION_DEBUG

enum DebugTypes {
  DI_SOLVER,
  DI_SUMCON,
  DI_BOOLCON,
  DI_ANDCON,
  DI_ARRAYAND,
  DI_QUEUE,
  DI_REIFY,
  DI_LEXCON,
  DI_TABLECON,
  DI_TEST,
  DI_DYSUMCON,
  DI_DYNAMICTRIG,
  DI_DYELEMENT,
  DI_INTCON,
  DI_LONGINTCON,
  DI_INTCONTAINER,
  DI_BOUNDCONTAINER,
  DI_GACELEMENT,
  DI_CHECKCON,
  DI_VECNEQ,
  DI_MEMBLOCK,
  DI_POINTER,
  DI_OR,
  DI_GADGET
};

#define DEBUG_CASE(x)                                                                              \
  case x:                                                                                          \
    std::cerr << #x;                                                                               \
    break;

#else

#endif

inline bool DOMAIN_CHECK(BigInt v) {
  return v < DomainInt_Max && v > DomainInt_Min;
}

// These are just to catch cases where the user didn't cast to BigInt
// themselves, which makes the function useless.
inline void DOMAIN_CHECK(DomainInt);
inline void DOMAIN_CHECK(SysInt);
inline void DOMAIN_CHECK(UnsignedSysInt);

#endif // DEBUG_H
