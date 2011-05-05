#ifndef MICROINST_H
#define MICROINST_H

#include "globals.h"

string AMn_X_RReg_S_AMn(byte inst);   // AMn <- R[Reg] + AMn
string AMn_X_RReg(byte inst);         // AMn <- R[Reg]
string AM0_X_AM0_OP_AMn(byte inst);   // AM0 <- AMn OP AM0
string MAR_X_RReg(byte inst);         // AMR <- R[Reg]
string AMn_X_AMt_S_AMn(byte inst);    // AMn <- AMt + AMn
string AMn_X_IMM_OP_AMn(byte inst);   // AMn <- imm OP AMn
string AMn_X_AMn_D_IMM(byte inst);    // AMn <- AMn - imm
string AMn_X_MEMread(byte inst);      // AMn <- MEMread
string MAR_X_AMn(byte inst);          // MAR <- AMn
string PC_X_AMn(byte inst);           // PC <- AMn
string MEMwrite_X_AMn(byte inst);     // MEMwrite <- AMn
string R15_X_R15_OP_1(byte inst);     // R15 <- R15 OP 1
string MAR_X_MEMread(byte inst);      // MAR <- MEMread
string PC_X_PC_S_1(byte inst);        // PC <- PC + 1
string MAR_X_PC(byte inst);           // MAR <- PC
string AM0_X_N_AM0(byte inst);        // AM0 <- -AM0
string AM0_X_C_AM0(byte inst);        // AM0 <- ~AM0
string AM0_X_AM1(byte inst);          // AM0 <- AM1
string IR_X_MEMread(byte inst);       // IR <- MEMread
string IMM_X_MEMread(byte inst);      // imm <- MEMread
string RReg_X_AM0(byte inst);         // R[Reg] <- AM0
string halt(byte inst);               // halt

byte getMicroFunction(byte inst);

#endif
