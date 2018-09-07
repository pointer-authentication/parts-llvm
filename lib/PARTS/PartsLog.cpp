//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/PARTS/PartsLog.h"

#include <mutex>
#include <unordered_map>
#include <unistd.h>
#include <llvm/PARTS/PartsLog.h>

#include "llvm/Support/raw_ostream.h"


namespace llvm {

namespace PARTS {

PartsLog::PartsLog(const std::string &name)
    : m_name(name) {}

PartsLog::~PartsLog() {}

void PartsLog::restrictToFunc(const std::string func) {
  m_onlyFunc = true;
  m_onlyFuncName = func;
}

PARTS::PartsLogStream PartsLog::debug() {
  return PartsLogStream(get_ostream(m_enabled, raw_ostream::CYAN));
}

PARTS::PartsLogStream PartsLog::debug(const std::string &F) {
  return PartsLogStream(get_ostream(m_enabled, F, raw_ostream::CYAN));
}

PARTS::PartsLogStream PartsLog::info() {
  return PartsLogStream(get_ostream(m_enabled, raw_ostream::WHITE));
}

PARTS::PartsLogStream PartsLog::info(const std::string &F) {
  return PartsLogStream(get_ostream(m_enabled, F, raw_ostream::WHITE));
}

PARTS::PartsLogStream PartsLog::red() {
  return PartsLogStream(get_ostream(m_enabled, raw_ostream::RED));
}

PARTS::PartsLogStream PartsLog::green() {
  return PartsLogStream(get_ostream(m_enabled, raw_ostream::GREEN));
}

PARTS::PartsLogStream PartsLog::warn() {
  return PartsLogStream(get_ostream(true, raw_ostream::YELLOW));
}

PARTS::PartsLogStream PartsLog::error() {
  return PartsLogStream(get_ostream(true, raw_ostream::MAGENTA));
}

PARTS::PartsLogStream PartsLog::error(const std::string &F) {
  return PartsLogStream(get_ostream(true, F, raw_ostream::MAGENTA));
}

std::mutex _mutex;

PartsLog_ptr PartsLog::getLogger(const std::string &name)
{
  static auto loggers = std::unordered_map<std::string, std::shared_ptr<PartsLog>>();

  std::unique_lock<std::mutex> lock(_mutex);

  auto found = loggers.find(name);
  if (found != loggers.end()) {
    return found->second;
  } else {
    auto newLogger = std::make_shared<PartsLog>(name);
    loggers.insert({name, newLogger});
    return newLogger;
  }
}

PartsLogStream PartsLog::inc(const std::string &var, bool b, const std::string &F) {
  return b ? inc(var, raw_ostream::GREEN, F) : inc(var, raw_ostream::RED, F);
}

PartsLogStream PartsLog::dec(const std::string &var, bool b, const std::string &F) {
  return b ? dec(var, raw_ostream::GREEN, F) : dec(var, raw_ostream::RED, F);
}

PartsLogStream PartsLog::inc(const std::string &var, const raw_ostream::Colors c, const std::string &F) {
  auto &s = getStats();
  s.inc(var);
  return PartsLogStream(get_ostream(m_enabled, F, c));
}

PartsLogStream PartsLog::dec(const std::string &var, const raw_ostream::Colors c, const std::string &F) {
  auto &s = getStats();
  s.inc(var);
  return PartsLogStream(get_ostream(m_enabled, F, c));
}


PartsLog::Stats &PartsLog::getStats() {
  static Stats stats;
  return stats;
}

PartsLog::Stats::Stats() {
  m_stats = std::map<std::string,int>();
}

PartsLog::Stats::~Stats() {
  dump();
}

void PartsLog::Stats::dump() const {
  errs() << "\n//---------------- Dumping PartsLog::Stats --------------------//\n";

  if (m_stats.empty())
    errs() << "EMPTY!!!\n";

  for (auto p : m_stats)
    errs() << p.first << ": " << p.second << "\n";

  errs() << "//---------------- done ---------------------------------------//\n\n";
}

void PartsLog::Stats::inc(const std::string var) {
  auto found = m_stats.find(var);
  if (found == m_stats.end()) {
    m_stats.insert({var, 1});
  } else {
    found->second++;
  }
}

void PartsLog::Stats::dec(const std::string var) {
  auto found = m_stats.find(var);
  if (found == m_stats.end()) {
    m_stats.insert({var, -1});
  } else {
    found->second--;
  }
}



} // PARTS

} // llvm

