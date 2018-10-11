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
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/PARTS/PartsLogStream.h"
#include "llvm/IR/Instruction.h"
#include "llvm/CodeGen/MachineBasicBlock.h"

namespace llvm {

namespace PARTS {

class PartsLogStream {
private:
  llvm::raw_ostream *m_ostream;

public:
  explicit PartsLogStream(llvm::raw_ostream &ostream);
  ~PartsLogStream();

  PartsLogStream &resetColor();
  PartsLogStream &changeColor(enum raw_ostream::Colors colors, bool bold,bool bg);

  PartsLogStream &operator<<(const std::string &str);
  PartsLogStream &operator<<(const unsigned long &str);
  PartsLogStream &operator<<(const char *str);
  PartsLogStream &operator<<(const long &str);
  PartsLogStream &operator<<(const int &str);
  PartsLogStream &operator<<(const unsigned &str);
  PartsLogStream &operator<<(const Value *I);
  PartsLogStream &operator<<(const Type *T);
  PartsLogStream &operator<<(const Instruction &I);
  PartsLogStream &operator<<(const Module::global_iterator &GV);
  PartsLogStream &operator<<(const GlobalVariable &GV);
  PartsLogStream &operator<<(const MachineBasicBlock::instr_iterator &MI);
};

} // namespace PARTS

} // namespace llvm

#endif //LLVM_PARTSLOGSTREAM_H
