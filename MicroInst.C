/**
 * File: MicroInst.h
 *
 * Authors: Benjamin David Mayes <bdm8233@rit.edu>
 *          Colin Alexander Barr <colin.a.barr@gmail.com>
 *
 * Description: Declarations for micro-instruction functions.
 */

#include "MicroInst.h"
#include "globals.h"

#include <sstream>

/**
 * Performs the following micro-op:
 *
 * AMn <- R[reg] + AMn
 *
 * In this specific instruction, reg corresponds to the low-order 4 bits of the
 * micro-instruction and bits 6..4 correspond to the index of the AM register.
 *
 * @param inst The micro-instruction.
 * @return A string used for debug output.
 */
string AMn_X_RReg_S_AMn(byte inst) {   // AMn <- R[Reg] + AMn
    // obtain indices out of the micro-instruction.
    uint32 ri = inst & 0xF;
    uint32 ai = (inst & 0x30) >> 4;

    // generate the trace output
    stringstream ss;
    //ss << "AM[" << ai << "] <- R[" << ri << "] + AM[" << ai << "]" << " (" << r[ri].value() << " + " << amr[ai].value() << ")";
    ss << "AM" << ai << " <- " << r[ri] << " + " << amr[ai];

    // perform the micro-operation.
    alu.OP1().pullFrom(r[ri]);
    alu.OP2().pullFrom(amr[ai]);
    alu.perform(BusALU::op_add);
    amr[ai].latchFrom(alu.OUT());

    return ss.str();
}

/**
 * Performs the following micro-op:
 *
 * AMn <- R[Reg]
 *
 * In this specific instruction, reg corresponds to the low-order 4 bits of the
 * micro-instruction and bits 6..4 correspond to the index of the AM register.
 *
 * @param inst The micro-instruction.
 * @return A string used for trace output.
 */

string AMn_X_RReg(byte inst) {         // AMn <- R[Reg]
    uint32 ri = inst & 0xF;
    uint32 ai = (inst & 0x30) >> 4;
    stringstream ss;
    //ss << "AM[" << ai << "] <- R[" << ri << "]" << " (" << r[ri].value() << ")";
    ss << "AM" << ai << " <- " << r[ri];

    dbus.IN().pullFrom(r[ri]);
    amr[ai].latchFrom(dbus.OUT());

    return ss.str();
}

/**
 * Peforms the following micro-op:
 * AM0 <- AMn OP AM0
 *
 * The OP in this case is obtained by examining the low order 3 bits of the
 * micro-instruction.
 *
 * Bit 4 decides whether or not we are using AM0 or AM1 for the second operand. 
 *
 * @param The micro-instruction.
 * @return A string used for trace outpu.
 */
string AM0_X_AM0_OP_AMn(byte inst) {   // AM0 <- AM0 OP AMn
    // extract the operands
    uint32 ai = (inst & 0x8) >> 3;
    uint32 op = inst & 0x7;


    stringstream ss;
    //ss << "AM[0] <- AM[0] (" << amr[0].value() << ")";
    ss << "AM0" << " <- " << amr[0];

    // load the ALU operands
    alu.OP1().pullFrom(amr[0]);
    alu.OP2().pullFrom(amr[ai]);

    // perform the ALU operation
    switch(op) {
        case ALU_OP_ADD:
            alu.perform(BusALU::op_add);
            ss << " + ";
            break;
        case ALU_OP_OR:
            alu.perform(BusALU::op_or);
            ss << " | ";
            break;
        case ALU_OP_AND:
            alu.perform(BusALU::op_and);
            ss << " & ";
            break;
        case ALU_OP_XOR:
            alu.perform(BusALU::op_xor);
            ss << " ^ ";
            break;
        case ALU_OP_SLL:
            alu.perform(BusALU::op_lshift);
            ss << " << ";
            break;
        case ALU_OP_SRL:
            alu.perform(BusALU::op_rshift);
            ss << " >> ";
            break;
        case ALU_OP_SRA:
            alu.perform(BusALU::op_rashift);
            ss << " >>a ";
            break;
    }
    //ss << "AM[" << ai <<"] (" << amr[ai].value() << ")";
    ss << amr[ai];

    amr[0].latchFrom(alu.OUT());

    return ss.str();
}

/**
 * Performs the following micro-op:
 *
 * MAR <- R[reg]
 *
 * The register index is taken from the to the low order four bits of the 
 * micro instruction.
 *
 * @param inst The micro-instruction.
 * @return A string used in trace output.
 */
string MAR_X_RReg(byte inst) {         // MAR <- R[Reg]
    // extract the operands
    uint32 ri = inst & 0xF;
    
    stringstream ss;
    ss << "MAR <- R[" << ri << "] (" << r[ri].value() << ")";

    // send the data from the register to the MAR over the abus.
    abus.IN().pullFrom(r[ri]);
    mem.MAR().latchFrom(abus.OUT());

    return ss.str();
}

/**
 * Performs the following micro-op:
 *
 * AMn <- AMt + AMn
 *
 * The AM temporary register index is taken from the low-order 2 bits of
 * the micro-instruction.
 *
 * @param inst The micro-instruction.
 * @return A string used in trace output.
 */
string AMn_X_AMt_S_AMn(byte inst) {    // AMn <- AMt + AMn
    uint32 ai = inst & 0x3;
    stringstream ss;
    //ss << "AM[" << ai << "] <- AM[3] + AM[" << ai << "]";
    ss << "AM" << ai << " <- " << amr[3] << " + " << amr[ai];

    alu.OP1().pullFrom(amr[ai]);
    alu.OP2().pullFrom(amr[3]);
    alu.perform(BusALU::op_add);
    amr[ai].latchFrom(alu.OUT());

    return ss.str();
}

/**
 * Performs the following micro-op:
 *
 * AMn <- imm OP AMn
 *
 * The operation is selected from the low order bit of the micro-instruction
 * The index of the AM temp register is taken from the next two bits.
 *
 * @param inst The micro-instruction.
 * @return A string used in trace output.
 */
string AMn_X_IMM_OP_AMn(byte inst) {   // AMn <- imm OP AMn
    uint32 ai = (inst & 0x2) >> 1;
    uint32 op = inst & 0x1;
    stringstream ss;

    alu.OP1().pullFrom(imm);
    alu.OP2().pullFrom(amr[ai]);

    //ss << "AM[" << ai << "] <- imm (" << imm.value() << ")";
    ss << "AM" << ai << " <- " << imm;

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
    //ss << "AM[" << ai << "] (" << amr[ai].value() << ")";
    ss << amr[ai];

    amr[ai].latchFrom(alu.OUT());

    return ss.str();
}

/**
 * Performs the following micro-op:
 * 
 * AMn <- AMn - imm
 *
 * The index of the AM temp register is taken from the low order 2 bits of the
 * micro-instruction.
 *
 * @param The micro-instruction.
 * @return A string used in trace output.
 */
string AMn_X_AMn_D_IMM(byte inst) {    // AMn <- AMn - imm
    uint32 ai = inst & 0x3;
    stringstream ss;
    //ss << "AM[" << ai << "] <- AM[" << ai << "] (" << amr[ai].value() <<") - imm" << " (" << imm.value() << ")";
    ss << "AM" << ai << " <- " << amr[ai] << " - " << imm;

    alu.OP1().pullFrom(amr[ai]);
    alu.OP2().pullFrom(imm);
    alu.perform(BusALU::op_sub);
    amr[ai].latchFrom(alu.OUT());

    return ss.str();
}

/**
 * Performs the following micro-op:
 *
 * AMn <- MEMread
 *
 * The index of the AM temp register is taken from the low order 2 bits of the
 * micro-instruction.
 *
 * @param The micro-instruction.
 * @return A string used in trace output.
 */
string AMn_X_MEMread(byte inst) {      // AMn <- MEMread
    uint32 ai = inst & 0x3;
    stringstream ss;
    //ss << "AM[" << ai << "] <- MEMread";
    ss << "AM" << ai << " <- MEMread"; 

    mem.read();
    amr[ai].latchFrom(mem.READ());

    return ss.str();
}

/**
 * Performs the following micro-op:
 *
 * MAR <- AMn
 *
 * The index of the AM temp register is taken from the low order 2 bits of the
 * micro-instruction.
 *
 * @param The micro-instruction.
 * @return A string used in trace output.
 */
string MAR_X_AMn(byte inst) {          // MAR <- AMn
    uint32 ai = inst & 0x3;
    stringstream ss;
    //ss << "MAR <- AM[" << ai << "]" << " (" << amr[ai].value() << ")";
    ss << "MAR <- " << amr[ai];

    abus.IN().pullFrom(amr[ai]);
    mem.MAR().latchFrom(abus.OUT());

    return ss.str();
}

/**
 * Performs the following micro-op:
 *
 * PC <- AMn
 *
 * The index of the AM temp register is taken from the low order 2 bits of the
 * micro-instruction.
 *
 * @param The micro-instruction.
 * @return A string used in trace output.
 */
string PC_X_AMn(byte inst) {           // PC <- AMn
    uint32 ai = inst & 0x3;
    stringstream ss;
    //ss << "PC <- AM[" << ai << "]" << "  (" << amr[ai].value() << ")";
    ss << "PC <- " << amr[ai];

    abus.IN().pullFrom(amr[ai]);
    pc.latchFrom(abus.OUT());

    return ss.str();
}

/**
 * Performs the following micro-op:
 *
 * MEMwrite <- AMn
 *
 * The index of the AM temp register is taken from the low order 2 bits of the
 * micro-instruction.
 *
 * @param The micro-instruction.
 * @return A string used in trace output.
 */
string MEMwrite_X_AMn(byte inst) {     // MEMwrite <- AMn
    uint32 ai = inst & 0x3;
    stringstream ss;
    //ss << "MEMwrite <- AM[" << ai << "]";
    ss << "MEMwrite <- " << amr[ai];

    mem.WRITE().pullFrom(amr[ai]);
    mem.write();

    return ss.str();
}

/**
 * Performs the following micro-op:
 *
 * reg[15] <- reg[15] OP 1
 *
 * The op is obtained from the low order bit of the micro-instruction.
 *
 * @param The micro-instruction.
 * @return A string used in trace output.
 */
string R15_X_R15_OP_1(byte inst) {     // R15 <- R15 OP 1
    uint32 op = inst & 0x1;
    stringstream ss;
    //ss << "R[15] <- R[15]";
    ss << "R15 <- " << r[15];

    switch(op) {
        case 0:
            r[15].incr();
            ss << " + ";
            break;
        case 1:
            r[15].decr();
            ss << " - ";
    }
    ss << "1";

    return ss.str();
}

/**
 * Performs the following micro-op:
 *
 * MAR <- MEMread
 *
 * @param The micro-instruction.
 * @return A string used in trace output.
 */
string MAR_X_MEMread(byte inst) {      // MAR <- MEMread
    stringstream ss;
    ss << "MAR <- MEMread";

    mem.read();
    mem.MAR().latchFrom(mem.READ());

    return ss.str();
}

/**
 * Performs the following micro-op:
 *
 * PC <- PC + 1
 *
 * @param The micro-instruction.
 * @return A stirng used in trace output.
 */
string PC_X_PC_S_1(byte inst) {        // PC <- PC + 1
    stringstream ss;
    ss << "PC <- " << pc << " + 1";

    pc.incr();

    return ss.str();
}

/**
 * Perfors the following micro-op:
 *
 * MAR <- PC
 *
 * @param The micro-instruction.
 * @return A string used in trace output.
 */
string MAR_X_PC(byte inst) {           // MAR <- PC
    stringstream ss;
    ss << "MAR <- " << pc;

    abus.IN().pullFrom(pc);
    mem.MAR().latchFrom(abus.OUT());

    return ss.str();
}

/**
 * Performs the following micro-op:
 *
 * AM0 <- -AM0
 *
 * @param The micro-instruction.
 * @return A string used in trace output.
 */
string AM0_X_N_AM0(byte inst) {        // AM0 <- -AM0
    stringstream ss;
    ss << "AM0 <- -" << amr[0];

    alu.OP1().pullFrom(zero);
    alu.OP2().pullFrom(amr[0]);
    alu.perform(BusALU::op_sub);
    amr[0].latchFrom(alu.OUT());

    return ss.str();
}

/**
 * Performs the following operation:
 *
 * AM0 <- ~AM0
 * (where ~ corresponds to bitwise complement)
 *
 * @param The micro-instruction.
 * @return A string used in trace output.
 */
string AM0_X_C_AM0(byte inst) {        // AM0 <- ~AM0
    stringstream ss;
    ss << "AM0 <- ~" << amr[0]; 

    alu.OP1().pullFrom(amr[0]);
    alu.perform(BusALU::op_not);
    amr[0].latchFrom(alu.OUT());

    return ss.str();
}

/**
 * Performs the following operation:
 *
 * AM0 <- AM1
 *
 * @param The micro-instruction.
 * @return A string used in trace output.
 */
string AM0_X_AM1(byte inst) {          // AM0 <- AM1
    stringstream ss;
    ss << "AM0 <- " << amr[1];

    dbus.IN().pullFrom(amr[1]);
    amr[0].latchFrom(dbus.OUT());

    return ss.str();
}

/**
 * Performs the following operation:
 *
 * IR <- MEMread
 *
 * @param The micro-instruction.
 * @return A string used in trace output.
 */
string IR_X_MEMread(byte inst) {       // IR <- MEMread
    stringstream ss;
    ss << "IR <- MEMread";

    mem.read();
    ir.latchFrom(mem.READ());

    return ss.str();
}

/**
 * Performs the following micro-instruction:
 *
 * imm <- MEMread
 *
 * @param The micro-instruction
 * @return A string used for trace output.
 */
string IMM_X_MEMread(byte inst) {      // imm <- MEMread
    stringstream ss;
    ss << "imm <- MEMread";

    mem.read();
    imm.latchFrom(mem.READ());

    return ss.str();
}

/**
 * Performs the following micro-op:
 * 
 * R[Reg] <- AM0
 *
 * The register index corresponds to the low order 4 bits of the micro-instruction
 *
 * @param The micro instruction.
 * @return A string used for trace output.
 */
string RReg_X_AM0(byte inst) {         // R[Reg] <- AM0
    // extract the operands
    uint32 ri = inst & 0xF;

    stringstream ss;
    //ss << "R[" << ri << "] <- AM[0] (" << amr[0].value() << ")";
    ss << "R" << ri << " <- " << amr[0];

    // send the desired data from am[0] to reg[ri] over the data bus.
    dbus.IN().pullFrom(amr[0]);
    r[ri].latchFrom(dbus.OUT());

    return ss.str();
}

/**
 * Halts the CPU
 *
 * @param The micro-instruction
 * @return nothing - an exception is thrown.
 */
string halt(byte inst) {               // halt
    throw ERR_HALT;
    return "halt";
}

/**
 * Obtains the index into the function pointer table that contains function
 * pointers to functions to call for a given micro-op.
 * 
 * @param The micro-instruction.
 * @return An index into the function pointer table.
 */
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
    else if(inst == 0x01)
        return 21;
    return -1;
}
