#include "globals.h"
#include "MicroInst.h"

#include <sstream>

void setupMicroInstFunctions();

RegisterFile::RegisterFile(string name, uint32 bits, uint32 count) : number(count) {
  stringstream * ss;

  reg = new Counter*[number];
  for(uint32 i = 0; i < number; ++i) {
    ss = new stringstream();
    (*ss) << i;
    reg[i] = new Counter((name+ss->str()).c_str(),bits);
    delete ss;
  }
}

RegisterFile::~RegisterFile() {
  for(uint32 i = 0; i < number; ++i)
    delete reg[i];
  delete reg;
}

Counter & RegisterFile::operator [] (uint32 i) {
  return *(reg[i]);
}

const uint32 BIT_SIZE(32);

Counter pc("PC",BIT_SIZE);
Counter ir("IR",BIT_SIZE);
Counter imm("IMM",BIT_SIZE);
StorageObject zero("ZERO",BIT_SIZE,0);
RegisterFile r("R",BIT_SIZE,16);
RegisterFile amr("AMR",BIT_SIZE,4);

BusALU alu("ALU",BIT_SIZE);
Bus abus("ABUS",BIT_SIZE);
Bus dbus("DBUS",BIT_SIZE);
Memory mem("MEM",BIT_SIZE/2,BIT_SIZE);

MicroInst microInst[NUMBER_MICRO_FUNCTIONS];

void makeConnections() {
  //pc
  pc.connectsTo(abus.IN());
  pc.connectsTo(abus.OUT());

  //ir
  ir.connectsTo(mem.READ());

  //imm
  imm.connectsTo(mem.READ());
  imm.connectsTo(alu.OP1());
  imm.connectsTo(alu.OP2());

  //r's
  for(uint32 i = 0; i < 16; ++i) {
    r[i].connectsTo(abus.IN());
    r[i].connectsTo(abus.OUT());
    r[i].connectsTo(dbus.IN());
    r[i].connectsTo(dbus.OUT());
    r[i].connectsTo(alu.OP1());
  }

  //amr's
  for(uint32 i = 0; i < 4; ++i) {
    amr[i].connectsTo(abus.IN());
    amr[i].connectsTo(abus.OUT());
    amr[i].connectsTo(dbus.IN());
    amr[i].connectsTo(dbus.OUT());
    amr[i].connectsTo(alu.OP1());
    amr[i].connectsTo(alu.OP2());
    amr[i].connectsTo(alu.OUT());
    amr[i].connectsTo(mem.READ());
    amr[i].connectsTo(mem.WRITE());
  }

  //zero
  zero.connectsTo(alu.OP1());

  //setup micro instruction functions
  setupMicroInstFunctions();
}

void setupMicroInstFunctions() {
  microInst[0] = AMn_X_RReg_S_AMn;
  microInst[1] = AMn_X_RReg;  
  microInst[2] = AM0_X_AMn_OP_AM0;
  microInst[3] = MAR_X_RReg;
  microInst[4] = AMn_X_AMt_S_AMn;
  microInst[5] = AMn_X_IMM_OP_AMn;
  microInst[6] = AMn_X_AMn_D_IMM;
  microInst[7] = AMn_X_MEMread;
  microInst[8] = MAR_X_AMn;
  microInst[9] = PC_X_AMn;
  microInst[10] = MEMwrite_X_AMn;
  microInst[11] = R15_X_R15_OP_1;
  microInst[12] = MAR_X_MEMread;
  microInst[13] = PC_X_PC_S_1;
  microInst[14] = MAR_X_PC;
  microInst[15] = AM0_X_N_AM0;
  microInst[16] = AM0_X_C_AM0;
  microInst[17] = AM0_X_AM1;
  microInst[18] = IR_X_MEMread;
}
