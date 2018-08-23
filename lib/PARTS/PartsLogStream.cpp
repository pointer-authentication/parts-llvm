//
// Created by ishkamiel on 17/08/18.
//

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
