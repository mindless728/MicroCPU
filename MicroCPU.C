#include <sstream>
#include "includes.h"
#include "MicroInst.h"

list<string> fetch_strings;
list<string> decode_strings;
list<string> execute_strings;
list<string> writeback_strings;
 
//#define DEBUG

void gotoFetch();
void mFetch();
string mExecute(byte ai = 0);
void decode();
list<string> writeback();
void AM( list<string>& trace );
byte AMmodify(byte inst, byte ai);
void trace( const int& inst, const list<string>& fetch, const list<string>& decode, const list<string>& execute, const list<string>& writeback );
string get_inst_mnemonic( byte inst );
string resolve_address_modes( uint32 inst );

char t;

int main(int argc, char ** argv) {
    byte flags;
    cout << hex;
    //CPUObject::debug |= CPUObject::trace;
    if( argc < 2 ) {
        cout << "Usage: " << argv[0] << " [OBJ]\n";
    }

    try {
        makeConnections();
        mmem.load("mMemory.obj.o");
        mem.load(argv[1]);

        pc.latchFrom( mem.READ() );
        
       gotoFetch();
        while(1) {
            mFetch();
            execute_strings.push_back( mExecute() );
            flags = mir.uvalue() >> 24;
            if((flags & 0x7) == 1) {
                fetch_strings = execute_strings;
                execute_strings.clear();
                AM(decode_strings);
                decode();
            } else if((flags & 0x2) == 2) {
                if(!(flags & 0x80))
                    writeback_strings = writeback();
                    
#ifdef DEBUG
                for( int i = 0; i < 16; i += 4 ) {
                    cout << endl << r[i] << " " << r[i+1] << " " << r[i+2] << " " << r[i+3];
                }
                cout << endl;
#endif
                trace( ir.value(), fetch_strings, decode_strings, execute_strings, writeback_strings );
                execute_strings.clear();
                decode_strings.clear();
                writeback_strings.clear();
                gotoFetch();
            }
        }

    } catch(ArchLibError) {
        cout << "Simulation aborted - ArchLib runtime error" << endl;
    } catch(int err_code) {
        switch( err_code ) {
            case ERR_HALT:
                trace( ir.value(), fetch_strings, decode_strings, execute_strings, writeback_strings );
                cout << "CPU halted successfully!" << endl;
                break;
        }
    }
}

void gotoFetch() {
    mpc.clear();
    Clock::tick();

    mabus.IN().pullFrom(mpc);
    mmem.MAR().latchFrom(mabus.OUT());
    Clock::tick();

    mmem.read();
    mpc.latchFrom(mmem.READ());
    Clock::tick();
}

void mFetch() {
    mabus.IN().pullFrom(mpc);
    mmem.MAR().latchFrom(mabus.OUT());
    Clock::tick();

    mmem.read();
    mir.latchFrom(mmem.READ());
    mpc.incr();
    Clock::tick();
}

/**
 * Executes a micro instruction
 *
 * @param   ai  The instruction to execute.
 */
string mExecute(byte ai) {
    string str = "";
    byte inst[3] = {0};
    byte flags = mir.uvalue() >> 24;
    byte func = 0;

    for(uint32 i = 0; i < 3; ++i)
        inst[i] = (mir.uvalue() >> (8 * (2 - i))) & 0xFF;

  if(flags & 0x80) { //processing a jump
    //get the left and right test values
    int left = amr[0].value(),
           right = (ir.uvalue() & 0x10000000?0:amr[1].value());
    bool value = false;
    byte run = 0;

    //determine if the testing value is tue or not
    switch(inst[0]) {
    case JMP_OP_L:
      if(left < right)
        value = true;
      break;
    case JMP_OP_LE:
      if(left <= right)
        value = true;
      break;
    case JMP_OP_G:
      if(left > right)
        value = true;
      break;
    case JMP_OP_GE:
      if(left >= right)
        value = true;
      break;
    case JMP_OP_E:
      if(left == right)
        value = true;
      break;
    case JMP_OP_NE:
      if(left != right)
        value = true;
      break;
    }

    //grab the correct micro instruction to run
    if(ir.uvalue() & 0x10000000) {
        if(value)
            run = inst[1];
        else
            run = inst[2];
    } else if(value) {
        run = inst[1];
    } else
        str = "Branch Not Taken";

    //run microcode based on the truth value of comparison
    if(run) {
      func = getMicroFunction(run);
      str += microInst[func](run);
    }
  } else if(flags & 0x04) { //processing a adress mode isntruction
    for(uint32 i = 0; i < 3; ++i) {
      if(inst[i] == 0)
        continue;
      if(i)
        str += " : ";
      func = getMicroFunction(inst[i]);
      str += microInst[func](AMmodify(inst[i],ai));
    }
  } else { //processing a regular instruction
    for(uint32 i = 0; i < 3; ++i) {
      if(inst[i] == 0)
        continue;
      if(i)
        str += " : ";
      func = getMicroFunction(inst[i]);
      str += microInst[func](inst[i]);
    }
  }

#ifdef DEBUG
    cout << endl << (uint32)flags;
    for(uint32 i = 0; i < 3; ++i)
        cout << ' ' << (uint32)inst[i];
    cout << endl;
#endif

    Clock::tick();

    return str;
}

void decode() {
    malu.OP1().pullFrom(maux);
    malu.perform(BusALU::op_rop1);
    mmem.MAR().latchFrom(malu.OUT());
    Clock::tick();

    mmem.read();
    mpc.latchFrom(mmem.READ());
    Clock::tick();
}

list<string> writeback() {
    uint32 inst = ir.uvalue() >> 24;
    uint32 amdst = (ir.uvalue() & 0x00FF0000) >> 16;
    uint32 ri = amdst & 0xF;
    list<string> trace;
    if( ( inst & 0xE0 ) == 0 || inst == 0x40 || inst == 0x42 ) {
        // math instruction, mov instruction, pop instr: 
        // writeback occurs as exepected
        //  - Data to write is in amr[0]
        //  - MAR is already loaded with destination address in memory writebacks
        //  - register writebacks are

        // if we are using a register address mode, we writeback to a register
        if( (amdst & 0xF0) == 0x80 ) {
            trace.push_back( RReg_X_AM0( amdst & 0xF ) );
            stringstream ss;
            ss << " (R" << ri << " <- " << amr[0] << ")";
            trace.push_back( ss.str() );
        } else {    // otherwise we writeback to memory
            trace.push_back( MEMwrite_X_AMn(0) ); 
            stringstream ss;
            ss << "  (MEM[" << mem.MAR().uvalue() << "] <- " << amr[0] << ")";
            trace.push_back( ss.str() );
        }
    } 
    Clock::tick();
    return trace;
}

void AM( list<string>& trace ) {
    //setup maux to have ir infor for temp uses
    malu.OP1().pullFrom(ir);
    malu.perform(BusALU::op_rop1);
    maux.latchFrom(malu.OUT());
    Clock::tick();

    for(uint32 i = 2; i != -1; --i) {
        if(!(maux.uvalue() & 0x80/*mask.uvalue()*/)) {
            //shift the maux 8 bits
            malu.OP1().pullFrom(maux);
            malu.OP2().pullFrom(eight);
            malu.perform(BusALU::op_rshift);
            maux.latchFrom(malu.OUT());
            Clock::tick();
            continue;
        }

        //grab the last most address mode
        malu.OP1().pullFrom(maux);
        malu.OP2().pullFrom(mask);
        malu.perform(BusALU::op_and);
        mmem.MAR().latchFrom(malu.OUT());
        Clock::tick();

        //setup the mpc to execute from a new location
        mmem.read();
        mpc.latchFrom(mmem.READ());
        Clock::tick();

        //execute the address mode
        do {
            mFetch();
            trace.push_back( mExecute(i) );
            byte flags = mir.uvalue() >> 24;
            /*if((flags & 0x7) == 1) {
                //AM();
                decode();
            } else if((flags & 0x82) == 2) {
                writeback();
#ifdef DEBUG
                for( int j = 0; j < 16; j += 4 ) {
                    cout << endl << r[j] << " " << r[j+1] << " " << r[j+2] << " " << r[j+3];
                }
                cout << endl;
#endif
                gotoFetch();
            }*/
        } while(!(mir.uvalue() & 0x08000000));

        //shift the maux 8 bits
        malu.OP1().pullFrom(maux);
        malu.OP2().pullFrom(eight);
        malu.perform(BusALU::op_rshift);
        maux.latchFrom(malu.OUT());
        Clock::tick();
    }
}

byte AMmodify(byte inst, byte ai) {
    byte ri = maux.uvalue() & 0xF;
    if((inst & 0xC0) > 0x40)
        return ((inst & 0xC0) | (ai << 4) | ri);
    else if((inst & 0xF0) == 0x60)
        return ((inst & 0xF0) | ri);
    else if((inst & 0xFC) == 0x3C)
        return ((inst & 0xFC) | ai);
    else if((inst & 0xFC) == 0x30)
        return ((inst & 0xFC) | ai);
    else if((inst & 0xFC) == 0x2C)
        return ((inst & 0xFC) | ai);
    return inst;
}

/**
 * Function for obtaining readable trace output from the CPU's execution.
 *
 * @param inst      The instruction executed by the CPU
 * @param fetch     Micro-ops from the instruction fetch stage
 * @param decode    Micro-ops from the decode/AM resolution stage
 * @param execute   Micro-ops from the instruction execution stage
 * @param writeback Micro-ops from the writeback stage.
 * @param sideeffects   A more readable version of what occured during the writeback stage.
 */
void trace( const int& inst, const list<string>& fetch, const list<string>& decode, const list<string>& execute, const list<string>& writeback ) {
    string inst_string = get_inst_mnemonic( (byte)(inst >> 24) );
    string am_string = resolve_address_modes( inst );

    cout << endl << inst << ": " << inst_string << " " << am_string;
    if( !writeback.empty() ) {
        cout << writeback.back();
    }
    cout << endl;

    // print micro instructions for fetch:
    cout << "  Fetch:" << endl;
    list<string>::const_iterator i = fetch.begin();
    while( i != fetch.end() ) {
        cout << "    " << *i << endl;
        ++i;
    }

    // print micro instructions for decode + AMs:
    cout << "  Decode:" << endl;
    i = decode.begin();
    while( i != decode.end() ) {
        cout << "    " << *i << endl;
        ++i;
    }

    // print micro instructions for execution:
    cout << "  Execute:" << endl;
    i = execute.begin();
    while( i != execute.end() ) {
        cout << "    " << *i << endl;
        ++i;
    }

    if( !writeback.empty() ) {
        // print micro instructions for writeback
        cout << "  Writeback:" << endl;
        cout << "    " << writeback.front() << endl;
    }
}

/**
 * Obtains a meaningful mnemonic for the give opcode
 *
 * @param opc The opcode.
 */
string get_inst_mnemonic( byte opc ) {
    int category = (opc >> 5); // top 3 bits of the instruction
    int offset = ( opc & 0x1F ); // lower 5 bits of the instruction
    string ret;
    switch( category ) {
        case 0: // Math/ALU instruction
            switch( offset ) {
                case 1: 
                    ret = "ADD ";
                    break;
                case 2: 
                    ret = "SUB ";
                    break;
                case 3:
                    ret = "NEG ";
                    break;
                case 4:
                    ret = "OR  ";
                    break;
                case 5:
                    ret = "AND ";
                    break;
                case 6:
                    ret = "XOR ";
                    break;
                case 7:
                    ret = "CMP ";
                    break;
                case 8:
                    ret = "SLL ";
                    break;
                case 9:
                    ret = "SRL ";
                    break;
                case 10:
                    ret = "SRA ";
                    break;
                case 11:
                    ret = "INC ";
                    break;
                case 12:
                    ret = "DEC ";
                    break;
                default:
                    ret = "BAD MATH OPCODE ";
                    break;
            }
            break;
        case 1: // branch/jump instructions
            switch( offset ) {
                case 0:
                    ret = "JMP ";
                    break;
                case 1:
                    ret = "JL ";
                    break;
                case 2:
                    ret = "JLE ";
                    break;
                case 3:
                    ret = "JG ";
                    break;
                case 4:
                    ret = "JGE ";
                    break;
                case 5:
                    ret = "JEQ ";
                    break;
                case 6:
                    ret = "JNE ";
                    break;
                case 16:
                    ret = "JLC ";
                    break;
                case 17:
                    ret = "JLEC ";
                    break;
                case 18:
                    ret = "JGC ";
                    break;
                case 19:
                    ret = "JGEC ";
                    break;
                case 20:
                    ret = "JZ ";
                    break;
                case 21:
                    ret = "JNZ ";
                    break;

            }
            break;
        case 2: // data flow instructions
            switch( offset ) {
                case 0:
                    ret = "MOV ";
                    break;
                case 1: 
                    ret = "PUSH ";
                    break;
                case 2:
                    ret = "POP ";
                    break;
                default:
                    ret = "BAD DATAFLOW OPCODE ";
            }
            break;
        case 7: //misc/halt
            if( offset == 0x1f ) { // halt
                ret = "HALT";
            }
            break;
        default: //unused
            ret = "BAD CATEGORY ";
    }
    return ret;
}

/**
 * Resolves the AMs in the given instruction into a string that tells the
 * user about the instruction's AM operands.
 *
 * @param inst The instruction
 */
string resolve_address_modes( uint32 inst ) {
    // separate out meaningful fields from the instruction
    byte opc = (inst >> 24);
    int am[3] = { (( inst >> 16) & 0xFF), ((inst >> 8) & 0xFF), (inst & 0xFF) };

    int category = (opc >> 5);
    int offset = opc & 0x1F;
    int operands = 0;
    switch( category ) {
        case 0: // math instruction
            operands = 2;
            break;
        case 1: // jump instruction
            if( offset == 0 ) {
                operands = 1;
            } else {
                operands = 3;
            }
            break;
        case 2: // dataflow instructions
            if( offset == 0 ) {
                operands = 2;
            } else {
                operands = 2;
            }
            break;
    }
    stringstream ret;
    for( int i = 0; i < operands; ++i ) {
        // is this AM field populated? If not we definitely don't want to
        // print it
        if( am[i] ) {
            // we only want to print a comma if we are not the first field and 
            // our previous field is non-zero
            if( i != 0 && am[i-1] != 0 ) {
                ret << ", ";
            }
            // extract out the address mode and the operand of it
            int addr_mode = (am[i] >> 4); // the upper four bits
            int operand = (am[i] & 0xF); // the lower four bits
            switch( addr_mode ) {
                case 8: // register
                    ret << "R" << operand;
                    break;
                case 9: // register indirect
                    ret << "MEM[R" << operand << "]";
                    break;
                case 10: // memory indirect
                    ret << "MEM[MEM[R" << operand << "]]";
                    break;
                case 11: // indexed
                    ret << "MEM[R" << operand << "+IND]";
                    break;
                case 12: // indexed indirect
                    ret << "MEM[MEM[R" << operand << "+IND]]";
                    break;
                case 13: // indexed memory indirect
                    ret << "MEM[MEM[R" << operand << "]+IND]";
                    break;
                case 14: // double indexed
                    ret << "MEM[MEM[R" << operand << "+IND1]+IND2]";
                default: // an invalid AM
                    ret << "INVALID:(" << addr_mode << "," << operand << ")";
            }
        }
    }
    // we only want to print the immediate if we are using an ALU instruction
    // and there is some value in the immediate field.
    if( category == 0 && am[2] != 0 ) {
        ret << ", " << am[2];
    }
    return ret.str();
}
