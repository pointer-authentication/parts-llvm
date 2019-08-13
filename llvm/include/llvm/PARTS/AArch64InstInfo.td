//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
//         Carlos Chinea <carlos.chinea.perez@huawei.com>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
// Copyright (C) 2019 Huawei Technologies Oy (Finland) Co. Ltd
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

let isPseudo = 1, hasNoSchedulingInfo = 1 in {
  def PARTS_PACIA : Pseudo<(outs GPR64:$dst),
                           (ins GPR64:$ptr, GPR64sp:$mod),
                           [(set i64:$dst, (int_pa_pacia GPR64:$ptr, GPR64sp:$mod))],
                           "$dst = $ptr">;

  def PARTS_PACDA : Pseudo<(outs GPR64:$dst),
                           (ins GPR64:$ptr, GPR64sp:$mod),
                           [(set i64:$dst, (int_pa_pacda GPR64:$ptr, GPR64sp:$mod))],
                           "$dst = $ptr">;

  def PARTS_XPACD : Pseudo<(outs GPR64:$dst),
                           (ins GPR64:$ptr),
                           [(set i64:$dst, (int_pa_xpacd GPR64:$ptr))],
                           "$dst = $ptr">;

  def PARTS_AUTIA : Pseudo<(outs GPR64:$dst),
                           (ins GPR64:$ptr, GPR64sp:$mod),
                           [(set i64:$dst, (int_pa_autia GPR64:$ptr, GPR64sp:$mod))],
                           "$dst = $ptr">;

  def PARTS_AUTDA : Pseudo<(outs GPR64:$dst),
                           (ins GPR64:$ptr, GPR64sp:$mod),
                           [(set i64:$dst, (int_pa_autda GPR64:$ptr, GPR64sp:$mod))],
                           "$dst = $ptr">;

  def PARTS_AUTCALL : Pseudo<(outs GPR64:$dst),
                           (ins GPR64:$ptr, GPR64sp:$mod),
                           [(set i64:$dst, (int_pa_autcall GPR64:$ptr, GPR64sp:$mod))],
                           "$dst = $ptr">;

  def PARTS_DATA_PTR: Pseudo<(outs),
                           (ins GPR64:$ptr),
                           [(int_parts_data_pointer_argument GPR64:$ptr)]>;

  let mayStore = 1 in {
    def PARTS_SPILL: Pseudo <(outs), (ins GPR64z:$ptr, GPR64sp:$Rn, i32imm:$offset), []>;
    def PARTS_USPILL: Pseudo <(outs), (ins GPR64:$ptr, GPR64sp:$Rn, i32imm:$offset), []>;
  }
  let mayLoad = 1 in {
    def PARTS_RELOAD: Pseudo <(outs GPR64:$ptr), (ins GPR64sp:$Rn, i32imm:$offset), []>;
    def PARTS_URELOAD: Pseudo <(outs GPR64:$ptr), (ins GPR64sp:$Rn, i32imm:$offset), []>;
  }
}

let isCall = 1, isTerminator = 1, isReturn = 1, isBarrier = 1, Uses = [SP] in {
  def TCRETURNriAA : Pseudo<(outs), (ins tcGPR64:$dst, i32imm:$FPDiff, GPR64sp:$mod), []>,
                   Sched<[WriteBrReg]>;
  def TCRETURNriAB : Pseudo<(outs), (ins tcGPR64:$dst, i32imm:$FPDiff, GPR64sp:$mod), []>,
                   Sched<[WriteBrReg]>;
}
