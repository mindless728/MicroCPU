#include "includes.h"
#include "MicroInst.h"

#define DEBUG

void gotoFetch();
void mFetch();
void mExecute(byte ai = 0);
void decode();
void writeback();
void AM();
byte AMmodify(byte inst, byte ai);

char t;

int main(int argc, char ** argv) {
  byte flags;
  cout << hex;
  //CPUObject::debug |= CPUObject::trace;

  try {
    makeConnections();
    mem.load("Memory.obj.o");
    mmem.load("mMemory.obj.o");
  
    gotoFetch();
    while(1) {
      mFetch();
      mExecute();
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

void mExecute(byte ai) {
  string str = "";
  byte inst[3] = {0};
  byte flags = mir.uvalue() >> 24;
  byte func = 0;

  for(uint32 i = 0; i < 3; ++i)
    inst[i] = (mir.uvalue() >> (8 * (2 - i))) & 0xFF;

  if(flags & 0x80) { //processing a jump
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
  
  cout << str << endl;
  Clock::tick();

  if((flags & 0x7) == 1) {
    AM();
    decode();
  } else if((flags & 0x82) == 2) {
    writeback();
    gotoFetch();
  }
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

void writeback() {
}

void AM() {
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
      mExecute(i);
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
