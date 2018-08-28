
let isPseudo = 1 in {
  def PARTS_PACIA : Pseudo<(outs GPR64:$dst),
                           (ins GPR64:$ptr, GPR64:$mod),
                           [(set i64:$dst, (int_pa_pacia GPR64:$ptr, imm:$mod))]>,
                    Sched<[WriteBrReg]>;
}