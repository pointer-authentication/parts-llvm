//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_PARTSLOGSTREAM_H
#define LLVM_PARTSLOGSTREAM_H

#include <string>
#include <memory>
#include "llvm/PARTS/PartsLogStream.h"
#include "llvm/IR/Instruction.h"

namespace llvm {

namespace PARTS {

class PartsLogStream {
private:
  const std::string m_preamble;

public:
  PartsLogStream() = delete;

  PartsLogStream(const std::string &preamble);

  PartsLogStream &operator<<(std::string &str);

  PartsLogStream &operator<<(unsigned long &str);

  PartsLogStream &operator<<(long &str);

  PartsLogStream &operator<<(Instruction &I);
};

} // namesapce PARTS

} // namespace llvm

#endif //LLVM_PARTSLOGSTREAM_H
