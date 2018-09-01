//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

let isPseudo = 1 in {
  def PARTS_PACIA : Pseudo<(outs GPR64:$dst),
                           (ins GPR64:$ptr, GPR64:$mod),
                           [(set i64:$dst, (int_pa_pacia GPR64:$ptr, imm:$mod))]>,
                    Sched<[WriteBrReg]>;
}