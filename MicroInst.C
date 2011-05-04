#include "MicroInst.h"
#include "globals.h"

void AMn_X_RReg_S_AMn(byte inst) {   // AMn <- R[Reg] + AMn
  byte ri = inst & 0xF;
  byte ai = (inst & 0x30) >> 4;

  alu.OP1().pullFrom(r[ri]);
  alu.OP2().pullFrom(amr[ai]);
  alu.perform(BusALU::op_add);
  amr[ai].latchFrom(alu.OUT());
}

void AMn_X_RReg(byte inst) {         // AMn <- R[Reg]
  byte ri = inst & 0xF;
  byte ai = (inst & 0x30) >> 4;

  dbus.IN().pullFrom(r[ri]);
  amr[ai].latchFrom(dbus.OUT());
}

void AM0_X_AMn_OP_AM0(byte inst) {   // AM0 <- AMn OP AM0
  byte ai = (inst & 0x8) >> 3;
  byte op = inst & 0x7;

  alu.OP1().pullFrom(amr[0]);
  alu.OP2().pullFrom(amr[ai]);

  switch(op) {
  case 0:
    alu.perform(BusALU::op_add);
    break;
  case 1:
    alu.perform(BusALU::op_or);
    break;
  case 2:
    alu.perform(BusALU::op_and);
    break;
  case 3:
    alu.perform(BusALU::op_xor);
    break;
  case 4:
    alu.perform(BusALU::op_lshift);
    break;
  case 5:
    alu.perform(BusALU::op_rshift);
    break;
  case 6:
    alu.perform(BusALU::op_rashift);
    break;
  }

  amr[0].latchFrom(alu.OUT());
}

void MAR_X_RReg(byte inst) {         // MAR <- R[Reg]
  byte ri = inst & 0xF;

  abus.IN().pullFrom(r[ri]);
  mem.MAR().latchFrom(abus.OUT());
}

void AMn_X_AMt_S_AMn(byte inst) {    // AMn <- AMt + AMn
  byte ai = inst & 0x3;

  alu.OP1().pullFrom(amr[ai]);
  alu.OP2().pullFrom(amr[3]);
  alu.perform(BusALU::op_add);
  amr[ai].latchFrom(alu.OUT());
}

void AMn_X_IMM_OP_AMn(byte inst) {   // AMn <- imm OP AMn
  byte ai = (inst & 0x2) >> 1;
  byte op = inst & 0x1;

  alu.OP1().pullFrom(imm);
  alu.OP2().pullFrom(amr[ai]);

  switch(op) {
  case 0:
    alu.perform(BusALU::op_add);
    break;
  case 1:
    alu.perform(BusALU::op_sub);
    break;
  }

  amr[ai].latchFrom(alu.OUT());
}

void AMn_X_AMn_D_IMM(byte inst) {    // AMn <- AMn - imm
  byte ai = inst & 0x3;

  alu.OP1().pullFrom(amr[ai]);
  alu.OP2().pullFrom(imm);
  alu.perform(BusALU::op_sub);
  amr[ai].latchFrom(alu.OUT());
}

void AMn_X_MEMread(byte inst) {      // AMn <- MEMread
  byte ai = inst & 0x3;

  mem.read();
  amr[ai].latchFrom(mem.READ());
}

void MAR_X_AMn(byte inst) {          // MAR <- AMn
  byte ai = inst & 0x3;

  abus.IN().pullFrom(amr[ai]);
  mem.MAR().latchFrom(abus.OUT());
}

void PC_X_AMn(byte inst) {           // PC <- AMn
  byte ai = inst & 0x3;
  
  abus.IN().pullFrom(amr[ai]);
  pc.latchFrom(abus.OUT());
}

void MEMwrite_X_AMn(byte inst) {     // MEMwrite <- AMn
  byte ai = inst & 0x3;

  mem.WRITE().pullFrom(amr[ai]);
  mem.write();
}

void R15_X_R15_OP_1(byte inst) {     // R15 <- R15 OP 1
  byte op = inst & 0x1;

  switch(op) {
  case 0:
    r[15].incr();
  case 1:
    r[15].decr();
  }
}

void MAR_X_MEMread(byte inst) {      // MAR <- MEMread
  mem.read();
  mem.MAR().latchFrom(mem.READ());
}

void PC_X_PC_S_1(byte inst) {        // PC <- PC + 1
  pc.incr();
}

void MAR_X_PC(byte inst) {           // MAR <- PC
  abus.IN().pullFrom(pc);
  mem.MAR().latchFrom(abus.OUT());
}

void AM0_X_N_AM0(byte inst) {        // AM0 <- -AM0
  alu.OP1().pullFrom(zero);
  alu.OP2().pullFrom(amr[0]);
  alu.perform(BusALU::op_sub);
  amr[0].latchFrom(alu.OUT());
}

void AM0_X_C_AM0(byte inst) {        // AM0 <- ~AM0
  alu.OP1().pullFrom(amr[0]);
  alu.perform(BusALU::op_not);
  amr[0].latchFrom(alu.OUT());
}

void AM0_X_AM1(byte inst) {          // AM0 <- AM1
  dbus.IN().pullFrom(amr[1]);
  amr[0].latchFrom(dbus.OUT());
}

void IR_X_MEMread(byte inst) {       // IR <- MEMread
  mem.read();
  ir.latchFrom(mem.READ());
}

byte getMicroFunction(byte inst) {
  if((inst & 0xC0) == 0xC0)
    return 0;
  else if((inst & 0xC0) == 0x80)
    return 1;
  else if((inst & 0xF0) == 0x70)
    return 2;
  else if((inst & 0xF0) == 0x60)
    return 3;
  else if((inst & 0xFC) == 0x3C)
    return 4;
  else if((inst & 0xFC) == 0x38)
    return 5;
  else if((inst & 0xFC) == 0x34)
    return 6;
  else if((inst & 0xFC) == 0x30)
    return 7;
  else if((inst & 0xFC) == 0x2C)
    return 8;
  else if((inst & 0xFC) == 0x28)
    return 9;
  else if((inst & 0xFC) == 0x24)
    return 10;
  else if((inst & 0xFE) == 0x22)
    return 11;
  else if(inst == 0x1F)
    return 12;
  else if(inst == 0x1E)
    return 13;
  else if(inst == 0x1D)
    return 14;
  else if(inst == 0x1C)
    return 15;
  else if(inst == 0x1B)
    return 16;
  else if(inst == 0x1A)
    return 17;
  else if(inst == 0x19)
    return 18;
  return -1;
}
