//
// Created by ishkamiel on 16/08/18.
//

#ifndef LLVM_PARTS_PARTSLOG_H
#define LLVM_PARTS_PARTSLOG_H

#include <string>
#include <memory>
#include "llvm/PARTS/PartsLogStream.h"

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
