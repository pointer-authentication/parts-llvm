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
#include "llvm/Support/raw_ostream.h"
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
private:
  class Stats {
  private:
    std::map<std::string, int> m_stats;

  public:
    Stats();
    Stats(const Stats &) = delete;
    ~Stats();

    void dump() const;
    void inc(const std::string var);
    void dec(const std::string var);
  };

  const std::string m_name;
  bool m_enabled = true;
  bool m_onlyFunc = false;
  std::string m_onlyFuncName = "";

  static Stats &getStats();
  inline raw_ostream &get_ostream(const raw_ostream::Colors c) const;
  inline raw_ostream &get_ostream(const std::string &F, const raw_ostream::Colors c) const;

protected:
public:
  explicit PartsLog(const std::string &name);
  ~PartsLog();

  inline PartsLog &enable();
  inline PartsLog &disable();

  void restrictToFunc(const std::string func);

  PARTS::PartsLogStream inc(const std::string &var, bool b, const std::string &F = "");
  PARTS::PartsLogStream inc(const std::string &var, const raw_ostream::Colors c = raw_ostream::BLUE, const std::string &F = "");
  PARTS::PartsLogStream dec(const std::string &var, bool b, const std::string &F = "");
  PARTS::PartsLogStream dec(const std::string &var, const raw_ostream::Colors c = raw_ostream::BLUE, const std::string &F = "");

  PARTS::PartsLogStream debug();
  PARTS::PartsLogStream info();
  PARTS::PartsLogStream warn();
  PARTS::PartsLogStream error();
  PARTS::PartsLogStream red();
  PARTS::PartsLogStream green();

  PARTS::PartsLogStream debug(const std::string &F);
  PARTS::PartsLogStream info(const std::string &F);
  PARTS::PartsLogStream warn(const std::string &F);
  PARTS::PartsLogStream error(const std::string &F);
  PARTS::PartsLogStream red(const std::string &F);
  PARTS::PartsLogStream green(const std::string &F);

  static PartsLog_ptr getLogger(const std::string &name);
};

inline raw_ostream &PartsLog::get_ostream(const raw_ostream::Colors c) const {
  if (m_enabled)
    return (errs().changeColor(c, false, false) << m_name << ": ");

  return nulls();
}

inline raw_ostream &PartsLog::get_ostream(const std::string &F, const raw_ostream::Colors c) const {
  if (m_enabled && (!m_onlyFunc || F.compare(m_onlyFuncName) == 0)) {
    return (errs().changeColor(c, false, false) << m_name << ": ");
  }

  return nulls();
}

inline PartsLog &PartsLog::enable() {
  m_enabled = true;
  return *this;
}

inline PartsLog &PartsLog::disable() {
  m_enabled = false;
  return *this;
}

} // PARTS

} // llvm

#endif //LLVM_PARTS_PARTSLOG_H
