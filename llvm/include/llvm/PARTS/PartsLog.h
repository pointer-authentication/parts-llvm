//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_PARTS_PARTSLOG_H
#define LLVM_PARTS_PARTSLOG_H

#include <string>
#include <memory>
#include "llvm/PARTS/PartsLogStream.h"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"


//#define DISABLE_PA_DEBUG 1
//#define DEBUG_ONLY_FUNC "check_user"
//#define DEBUG_ONLY_FUNC "DivideInternalFPF"
#define DEBUG_PA_OPT(F,x) do { \
  x;\
  errs() << KNRM; \
} while(0);
#define DEBUG_PA_MIR(F,x) do { \
  x;\
  errs() << KNRM; \
} while(0);
#define DEBUG_PA_LOW(F,x) do { \
  x;\
  errs() << KNRM; \
} while(0);
#define DEBUG_PA_FUNC(F, name, x) do { \
  if (((F)->getName() == (name)) { \
    errs() << KBLU; \
    x; \
    errs() << KNRM; \
  } \
} while(0);


#ifdef DEBUG_ONLY_FUNC
#undef DEBUG_PA_OPT
#undef DEBUG_PA_MIR
#undef DEBUG_PA_LOW
#undef DEBUG_PA_FUNC
#define DEBUG_PA_OPT(F,x) do { \
  if (F != nullptr && DEBUG_ONLY_FUNC == (F)->getName()) { \
    x;\
    errs() << KNRM; \
  } \
} while(0);
#define DEBUG_PA_MIR(F,x) do { \
  if (F != nullptr && DEBUG_ONLY_FUNC == (F)->getName()) { \
    x;\
    errs() << KNRM; \
  } \
} while(0);
#define DEBUG_PA_LOW(F,x) do { \
  if (F != nullptr && DEBUG_ONLY_FUNC == (F)->getName()) { \
    x;\
    errs() << KNRM; \
  } \
} while(0);
#define DEBUG_PA_FUNC(F, name, x) do { \
  if (DEBUG_ONLY_FUNC == (F)->getName()) { \
    errs() << KBLU; \
    x; \
    errs() << KNRM; \
  } \
} while(0);
#endif

#ifdef DISABLE_PA_DEBUG
#undef DEBUG_PA_OPT
#define DEBUG_PA_OPT(F,x)
#undef DEBUG_PA_MIR
#define DEBUG_PA_MIR(F,x)
#undef DEBUG_PA_LOW
#define DEBUG_PA_LOW(F,x)
#undef DEBUG_PA_FUNC
#define DEBUG_PA_FUNC(F,x)
#endif


namespace llvm {

namespace PARTS {

class PartsLog;
typedef std::shared_ptr<PartsLog> PartsLog_ptr;

class PartsLog {
public:

private:
  const std::string m_name;

protected:
public:
  PartsLog(const std::string &name);

  PartsLogStream &warn();
  PartsLogStream &info();
  PartsLogStream &error();
  PartsLogStream &red();
  PartsLogStream &green();

  static PartsLog_ptr getLogger(const std::string &name);
};

} // PARTS

} // llvm

#endif //LLVM_PARTS_PARTSLOG_H
