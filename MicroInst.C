#include "MicroInst.h"
#include "globals.h"

#include <sstream>

string AMn_X_RReg_S_AMn(byte inst) {   // AMn <- R[Reg] + AMn
  uint32 ri = inst & 0xF;
  uint32 ai = (inst & 0x30) >> 4;
  stringstream ss;
  ss << "AM[" << ai << "] <- R[" << ri << "] + AM[" << ai << "]";

  alu.OP1().pullFrom(r[ri]);
  alu.OP2().pullFrom(amr[ai]);
  alu.perform(BusALU::op_add);
  amr[ai].latchFrom(alu.OUT());

  return ss.str();
}

string AMn_X_RReg(byte inst) {         // AMn <- R[Reg]
  uint32 ri = inst & 0xF;
  uint32 ai = (inst & 0x30) >> 4;
  stringstream ss;
  ss << "AM[" << ai << "] <- R[" << ri << "]";

  dbus.IN().pullFrom(r[ri]);
  amr[ai].latchFrom(dbus.OUT());

  return ss.str();
}

string AM0_X_AM0_OP_AMn(byte inst) {   // AM0 <- AMn OP AM0
  uint32 ai = (inst & 0x8) >> 3;
  uint32 op = inst & 0x7;
  stringstream ss;
  ss << "AM[0] <- AM[0]";

  alu.OP1().pullFrom(amr[0]);
  alu.OP2().pullFrom(amr[ai]);

  switch(op) {
  case 0:
    alu.perform(BusALU::op_add);
    ss << " + ";
    break;
  case 1:
    alu.perform(BusALU::op_or);
    ss << " | ";
    break;
  case 2:
    alu.perform(BusALU::op_and);
    ss << " & ";
    break;
  case 3:
    alu.perform(BusALU::op_xor);
    ss << " ^ ";
    break;
  case 4:
    alu.perform(BusALU::op_lshift);
    ss << " << ";
    break;
  case 5:
    alu.perform(BusALU::op_rshift);
    ss << " >> ";
    break;
  case 6:
    alu.perform(BusALU::op_rashift);
    ss << " >>a ";
    break;
  }
  ss << "AM[" << ai <<"]";

  amr[0].latchFrom(alu.OUT());

  return ss.str();
}

string MAR_X_RReg(byte inst) {         // MAR <- R[Reg]
  uint32 ri = inst & 0xF;
  stringstream ss;
  ss << "MAR <- R[" << ri << "]";

  abus.IN().pullFrom(r[ri]);
  mem.MAR().latchFrom(abus.OUT());

  return ss.str();
}

string AMn_X_AMt_S_AMn(byte inst) {    // AMn <- AMt + AMn
  uint32 ai = inst & 0x3;
  stringstream ss;
  ss << "AM[" << ai << "] <- AM[3] + AM[" << ai << "]";

  alu.OP1().pullFrom(amr[ai]);
  alu.OP2().pullFrom(amr[3]);
  alu.perform(BusALU::op_add);
  amr[ai].latchFrom(alu.OUT());

  return ss.str();
}

string AMn_X_IMM_OP_AMn(byte inst) {   // AMn <- imm OP AMn
  uint32 ai = (inst & 0x2) >> 1;
  uint32 op = inst & 0x1;
  stringstream ss;

  alu.OP1().pullFrom(imm);
  alu.OP2().pullFrom(amr[ai]);

  ss << "AM[" << ai << "] <- imm";

  switch(op) {
  case 0:
    alu.perform(BusALU::op_add);
    ss << " + ";
    break;
  case 1:
    alu.perform(BusALU::op_sub);
    ss << " - ";
    break;
  }
  ss << "AM[" << ai << "]";

  amr[ai].latchFrom(alu.OUT());

  return ss.str();
}

string AMn_X_AMn_D_IMM(byte inst) {    // AMn <- AMn - imm
  uint32 ai = inst & 0x3;
  stringstream ss;
  ss << "AM[" << ai << "] <- AM[" << ai << "] - imm";

  alu.OP1().pullFrom(amr[ai]);
  alu.OP2().pullFrom(imm);
  alu.perform(BusALU::op_sub);
  amr[ai].latchFrom(alu.OUT());

  return ss.str();
}

string AMn_X_MEMread(byte inst) {      // AMn <- MEMread
  uint32 ai = inst & 0x3;
  stringstream ss;
  ss << "AM[" << ai << "] <- MEMread";

  mem.read();
  amr[ai].latchFrom(mem.READ());

  return ss.str();
}

string MAR_X_AMn(byte inst) {          // MAR <- AMn
  uint32 ai = inst & 0x3;
  stringstream ss;
  ss << "MAR <- AM[" << ai << "]";

  abus.IN().pullFrom(amr[ai]);
  mem.MAR().latchFrom(abus.OUT());

  return ss.str();
}

string PC_X_AMn(byte inst) {           // PC <- AMn
  uint32 ai = inst & 0x3;
  stringstream ss;
  ss << "PC <- AM[" << ai << "]";
  
  abus.IN().pullFrom(amr[ai]);
  pc.latchFrom(abus.OUT());

  return ss.str();
}

string MEMwrite_X_AMn(byte inst) {     // MEMwrite <- AMn
  uint32 ai = inst & 0x3;
  stringstream ss;
  ss << "MEMwrite <- AM[" << ai << "]";

  mem.WRITE().pullFrom(amr[ai]);
  mem.write();

  return ss.str();
}

string R15_X_R15_OP_1(byte inst) {     // R15 <- R15 OP 1
  uint32 op = inst & 0x1;
  stringstream ss;
  ss << "R[15] <- R[15]";

  switch(op) {
  case 0:
    r[15].incr();
    ss << " + ";
  case 1:
    r[15].decr();
    ss << " - ";
  }
  ss << "1";

  return ss.str();
}

string MAR_X_MEMread(byte inst) {      // MAR <- MEMread
  stringstream ss;
  ss << "MAR <- MEMread";

  mem.read();
  mem.MAR().latchFrom(mem.READ());

  return ss.str();
}

string PC_X_PC_S_1(byte inst) {        // PC <- PC + 1
  stringstream ss;
  ss << "PC <- PC +1";

  pc.incr();

  return ss.str();
}

string MAR_X_PC(byte inst) {           // MAR <- PC
  stringstream ss;
  ss << "MAR <- PC";

  abus.IN().pullFrom(pc);
  mem.MAR().latchFrom(abus.OUT());

  return ss.str();
}

string AM0_X_N_AM0(byte inst) {        // AM0 <- -AM0
  stringstream ss;
  ss << "AM[0] <- -AM[0]";

  alu.OP1().pullFrom(zero);
  alu.OP2().pullFrom(amr[0]);
  alu.perform(BusALU::op_sub);
  amr[0].latchFrom(alu.OUT());

  return ss.str();
}

string AM0_X_C_AM0(byte inst) {        // AM0 <- ~AM0
  stringstream ss;
  ss << "AM[0] <- ~AM[0]";

  alu.OP1().pullFrom(amr[0]);
  alu.perform(BusALU::op_not);
  amr[0].latchFrom(alu.OUT());

  return ss.str();
}

string AM0_X_AM1(byte inst) {          // AM0 <- AM1
  stringstream ss;
  ss << "AM[0] <- AM[1]";

  dbus.IN().pullFrom(amr[1]);
  amr[0].latchFrom(dbus.OUT());

  return ss.str();
}

string IR_X_MEMread(byte inst) {       // IR <- MEMread
  stringstream ss;
  ss << "IR <- MEMread";

  mem.read();
  ir.latchFrom(mem.READ());

  return ss.str();
}

string IMM_X_MEMread(byte inst) {      // imm <- MEMread
  stringstream ss;
  ss << "imm <- MEMread";

  mem.read();
  imm.latchFrom(mem.READ());

  return ss.str();
}

string RReg_X_AM0(byte inst) {         // R[Reg] <- AM0
  uint32 ri = inst & 0xF;
  stringstream ss;
  ss << "R[" << ri << "] <- AM[0]";

  dbus.IN().pullFrom(amr[0]);
  r[ri].latchFrom(dbus.OUT());

  return ss.str();
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
  else if(inst == 0x18)
    return 19;
  else if((inst & 0xF0) == 0x50)
    return 20;
  return -1;
}
