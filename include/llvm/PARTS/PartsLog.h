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

#define DISABLE_PA_DEBUG 1
//#define PA_DEBUG_ONLY_FUNC "printer1"

#ifdef DISABLE_PA_DEBUG
#define DEBUG_PA(x)
#else
#define DEBUG_PA(x) x
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
    void inc(const std::string &var, unsigned num=1);
    void dec(const std::string &var, unsigned num=1);
  };

  const std::string m_name;
  bool m_enabled = false;
#ifndef PA_DEBUG_ONLY_FUNC
  bool m_onlyFunc = false;
  std::string m_onlyFuncName = "";
#else
  bool m_onlyFunc = true;
  std::string m_onlyFuncName = PA_DEBUG_ONLY_FUNC;
#endif

  static Stats &getStats();
  inline raw_ostream &get_ostream(bool enabled, const raw_ostream::Colors c) const;
  inline raw_ostream &get_ostream(bool enabled, const std::string &F, const raw_ostream::Colors c) const;

protected:
public:
  explicit PartsLog(const std::string &name);
  ~PartsLog();

  inline PartsLog &enable();
  inline PartsLog &disable();

  void restrictToFunc(const std::string func);

  PARTS::PartsLogStream inc(const std::string &var, unsigned num) { return inc(var, raw_ostream::BLUE, "", num); }
  PARTS::PartsLogStream inc(const std::string &var, const std::string &F, unsigned num = 1) { return inc(var, raw_ostream::BLUE, F, num); }
  PARTS::PartsLogStream inc(const std::string &var, bool b, const std::string &F = "", unsigned num = 1);
  PARTS::PartsLogStream inc(const std::string &var,
                            const raw_ostream::Colors c = raw_ostream::BLUE,
                            const std::string &F = "",
                            const unsigned num  = 1);
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

inline raw_ostream &PartsLog::get_ostream(bool enabled, const raw_ostream::Colors c) const {
  if (enabled)
    return (errs().changeColor(c, false, false) << m_name << ": ");

  return nulls();
}

inline raw_ostream &PartsLog::get_ostream(bool enabled, const std::string &F, const raw_ostream::Colors c) const {
  if (enabled && (!m_onlyFunc || F.compare(m_onlyFuncName) == 0)) {
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
