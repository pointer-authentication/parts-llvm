//
// Created by ishkamiel on 30/08/18.
//

#ifndef LLVM_PARTSUTILS_H
#define LLVM_PARTSUTILS_H

#include "AArch64.h"
#include "AArch64RegisterInfo.h"
#include <memory>

namespace llvm {

namespace PARTS {

class PartsUtils;

typedef std::shared_ptr<PartsUtils> PartsUtils_ptr;

class PartsUtils {

  const AArch64RegisterInfo *TRI;

public:
  PartsUtils(const AArch64RegisterInfo *TRI) : TRI(TRI) {};

  static inline PartsUtils_ptr get(const AArch64RegisterInfo *TRI) {
    return std::make_shared<PartsUtils>(TRI);
  };

  inline bool registerFitsPointer(unsigned reg);
  inline bool checkIfRegInstrumentable(unsigned reg);
};

inline bool PartsUtils::registerFitsPointer(unsigned reg)
{
  const auto RC = TRI->getMinimalPhysRegClass(reg);
  if (64 <= TRI->getRegSizeInBits(*RC)) {
    return true;
  }
  return false;
}

inline bool PartsUtils::checkIfRegInstrumentable(unsigned reg)
{
  if (reg == AArch64::FP || reg == AArch64::LR) {
    return false;
  }
  return registerFitsPointer(reg);
}



} // PARTS

} // llvm

#endif //LLVM_PARTSUTILS_H
