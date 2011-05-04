#ifndef MICROINST_H
#define MICROINST_H

#include "globals.h"

void AMn_X_RReg_S_AMn(byte inst);   // AMn <- R[Reg] + AMn
void AMn_X_RReg(byte inst);         // AMn <- R[Reg]
void AM0_X_AMn_OP_AM0(byte inst);   // AM0 <- AMn OP AM0
void MAR_X_RReg(byte inst);         // AMR <- R[Reg]
void AMn_X_AMt_S_AMn(byte inst);    // AMn <- AMt + AMn
void AMn_X_IMM_OP_AMn(byte inst);   // AMn <- imm OP AMn
void AMn_X_AMn_D_IMM(byte inst);    // AMn <- AMn - imm
void AMn_X_MEMread(byte inst);      // AMn <- MEMread
void MAR_X_AMn(byte inst);          // MAR <- AMn
void PC_X_AMn(byte inst);           // PC <- AMn
void MEMwrite_X_AMn(byte inst);     // MEMwrite <- AMn
void R15_X_R15_OP_1(byte inst);     // R15 <- R15 OP 1
void MAR_X_MEMread(byte inst);      // MAR <- MEMread
void PC_X_PC_S_1(byte inst);        // PC <- PC + 1
void MAR_X_PC(byte inst);           // MAR <- PC
void AM0_X_N_AM0(byte inst);        // AM0 <- -AM0
void AM0_X_C_AM0(byte inst);        // AM0 <- ~AM0
void AM0_X_AM1(byte inst);          // AM0 <- AM1
void IR_X_MEMread(byte inst);       // IR <- MEMread

byte getMicroFunction(byte inst);

#endif
