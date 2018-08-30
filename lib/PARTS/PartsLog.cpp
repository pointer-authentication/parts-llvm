
#include <unordered_map>
#include "llvm/PARTS/PartsLog.h"

namespace llvm {

namespace PARTS {

PartsLog::PartsLog(const std::string &name)
    : m_name(name) {}

PartsLogStream &PartsLog::warn() {
  static PartsLogStream log = PartsLogStream("warning: ");
  return log;
}

PartsLogStream &PartsLog::info() {
  static PartsLogStream log = PartsLogStream("info: ");
  return log;
}

PartsLogStream &PartsLog::error() {
  static PartsLogStream log = PartsLogStream("error: ");
  return log;
}

PartsLogStream &PartsLog::red() {
  static PartsLogStream log = PartsLogStream("red: ");
  return log;
}

PartsLogStream &PartsLog::green() {
  static PartsLogStream log = PartsLogStream("green: ");
  return log;
}

PartsLog_ptr PartsLog::getLogger(const std::string &name)
{
  static auto loggers = std::unordered_map<std::string, std::shared_ptr<PartsLog>>();

  auto found = loggers.find(name);
  if (found != loggers.end()) {
    return found->second;
  } else {
    auto newLogger = std::make_shared<PartsLog>(name);
    loggers.insert({name, newLogger});
    return newLogger;
  }
}

} // PARTS

} // llvm

