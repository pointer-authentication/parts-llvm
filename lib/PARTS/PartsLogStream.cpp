//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/PARTS/PartsLogStream.h"

namespace llvm {

namespace PARTS {

PartsLogStream::PartsLogStream(const std::string &preamble)
    : m_preamble(preamble) {}

PartsLogStream &PartsLogStream::operator<<(std::string &str) {
  return *this;
}

PartsLogStream &PartsLogStream::operator<<(unsigned long &str) {
  return *this;
}

PartsLogStream &PartsLogStream::operator<<(long &str) {
  return *this;
}

PartsLogStream &PartsLogStream::operator<<(Instruction &I) {
  return *this;
}

} // namespace PARTS

} // namespace llvm
