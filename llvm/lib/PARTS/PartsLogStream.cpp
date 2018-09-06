//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <llvm/PARTS/PartsLogStream.h>

#include "llvm/PARTS/PartsLogStream.h"

namespace llvm {

namespace PARTS {

PartsLogStream::PartsLogStream(llvm::raw_ostream &ostream)
    : m_ostream(&ostream) {}

PartsLogStream::~PartsLogStream() {
  resetColor();
}

PartsLogStream &PartsLogStream::operator<<(const std::string &str) {
  *m_ostream << str;
  return *this;
}

PartsLogStream &PartsLogStream::operator<<(unsigned long &str) {
  *m_ostream << str;
  return *this;
}

PartsLogStream &PartsLogStream::operator<<(long &str) {
  *m_ostream << std::to_string(str);
  return *this;
}

PartsLogStream &PartsLogStream::operator<<(unsigned &str) {
  *m_ostream << std::to_string(str);
  return *this;
}

PartsLogStream &PartsLogStream::operator<<(Instruction &I) {
  I.print(*m_ostream, true);
  return *this;
}

PartsLogStream &PartsLogStream::operator<<(const char *str) {
  *m_ostream << str;
  return *this;
}

PartsLogStream &PartsLogStream::operator<<(Module::global_iterator &GV) {
  GV->print(*m_ostream, true);
  return *this;
}

PartsLogStream &PartsLogStream::operator<<(MachineBasicBlock::instr_iterator &MI) {
  MI->print(*m_ostream);
  return *this;
}

PartsLogStream &PartsLogStream::resetColor() {
  m_ostream->resetColor();
  return *this;
}

PartsLogStream &PartsLogStream::changeColor(enum raw_ostream::Colors color, bool bold, bool bg) {
  m_ostream->changeColor(color, bold, bg);
  return *this;
}



} // namespace PARTS

} // namespace llvm
