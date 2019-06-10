//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
// Copyright (c) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

let isPseudo = 1 in {
  def PARTS_PACIA : Pseudo<(outs GPR64:$dst),
                           (ins GPR64:$ptr, GPR64:$mod),
                           [(set i64:$dst, (int_pa_pacia GPR64:$ptr, GPR64:$mod))]>,
                    Sched<[]>;
}

let isPseudo = 1 in {
  def PARTS_PACDA : Pseudo<(outs GPR64:$dst),
                           (ins GPR64:$ptr, GPR64:$mod),
                           [(set i64:$dst, (int_pa_pacda GPR64:$ptr, GPR64:$mod))]>,
                    Sched<[]>;
}

let isPseudo = 1 in {
  def PARTS_AUTIA : Pseudo<(outs GPR64:$dst),
                           (ins GPR64:$ptr, GPR64:$mod),
                           [(set i64:$dst, (int_pa_autia GPR64:$ptr, GPR64:$mod))]>,
                    Sched<[]>;
}

let isPseudo = 1 in {
  def PARTS_AUTDA : Pseudo<(outs GPR64:$dst),
                           (ins GPR64:$ptr, GPR64:$mod),
                           [(set i64:$dst, (int_pa_autda GPR64:$ptr, GPR64:$mod))]>,
                    Sched<[]>;
}
