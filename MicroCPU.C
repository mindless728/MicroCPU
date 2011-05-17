#include <sstream>
#include "includes.h"
#include "MicroInst.h"

//#define DEBUG

void gotoFetch();
void mFetch();
string mExecute(byte ai = 0);
void decode();
list<string> writeback();
void AM( list<string>& trace );
byte AMmodify(byte inst, byte ai);
void trace( const int& inst, const list<string>& fetch, const list<string>& decode, const list<string>& execute, const list<string>& writeback, const string& sideeffects );

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

        list<string> fetch_strings;
        list<string> decode_strings;
        list<string> execute_strings;
        list<string> writeback_strings;
        gotoFetch();
        while(1) {
            mFetch();
            execute_strings.push_back( mExecute() );
            flags = mir.uvalue() >> 24;
            if((flags & 0x7) == 1) {
                AM(decode_strings);
                decode();
            } else if((flags & 0x82) == 2) {
                writeback_strings = writeback();
#ifdef DEBUG
                for( int i = 0; i < 16; i += 4 ) {
                    cout << endl << r[i] << " " << r[i+1] << " " << r[i+2] << " " << r[i+3];
                }
                cout << endl;
#endif
                trace( ir.value(), fetch_strings, decode_strings, execute_strings, writeback_strings, string("sideeffects"));
                execute_strings.clear();
                decode_strings.clear();
                gotoFetch();
            }
        }

    } catch(ArchLibError) {
        cout << "Simulation aborted - ArchLib runtime error" << endl;
    } catch(int) {
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
    uint32 left = amr[0].uvalue(),
           right = (ir.uvalue() & 0x10000000?amr[1].uvalue():0);
    bool value = false;

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

    //run microcode based on the truth value of comparison
    if(inst[2-value]) {
      func = getMicroFunction(inst[2-value]);
      str += microInst[func](inst[2-value]);
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
            ss << "  (R[" << (amdst & 0xF) << "] <- " << amr[0].uvalue() << ")";
            trace.push_back( ss.str() );
            //dbus.IN().pullFrom( amr[0] );
            //r[amdst & 0xF].latchFrom(dbus.OUT());         
        } else {    // otherwise we writeback to memory
            trace.push_back( MEMwrite_X_AMn(0) ); 
            stringstream ss;
            ss << "  (MEM[" << mem.MAR().uvalue() << "] <- " << amr[0].uvalue() << ")";
            trace.push_back( ss.str() );
            //mem.WRITE().pullFrom( amr[0] );
            //mem.write();
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
        if(!(maux.uvalue() & mask.uvalue())) {
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
            if((flags & 0x7) == 1) {
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
            }
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
void trace( const int& inst, const list<string>& fetch, const list<string>& decode, const list<string>& execute, const list<string>& writeback, const string& sideeffects ) {
    string inst_string = "TODO: INSTRUCTION TRANSLATION";
    cout << inst << ": " << inst_string << endl;

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

    // print micro instructions for writeback
    cout << "  Writeback:" << endl;
    i = writeback.begin();
    while( i != writeback.end() ) {
        cout << "    " << *i << endl;
        ++i;
    }

    // print the sideeffects of the instruction (memory/register changes)
    cout << "  " << sideeffects << endl;
}
