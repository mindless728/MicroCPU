#ifndef GLOBALS_H
#define GLOBALS_H

#include "includes.h"

#define ERR_HALT            0
#define ERR_INVALID_AM      1
#define ERR_INVALID_OPCODE  2

// convenient typedefs
typedef unsigned char byte;
typedef unsigned int uint32;

// a function called to run micro instructions
typedef string(*MicroInst)(byte);

// A class that wraps a register file
class RegisterFile {
private:
  uint32 number;
  Counter ** reg;
public:
  RegisterFile(string name, uint32 bits, uint32 count);
  ~RegisterFile();
  Counter & operator [] (uint32 i);
  void clear();
};


// Global definitions of CPU features/components
extern const uint32 BIT_SIZE;
const uint32 NUMBER_MICRO_FUNCTIONS = 22;

// register objects
extern Counter pc; 
extern Counter ir; 
extern Counter imm;
extern StorageObject zero;
extern StorageObject eight;
extern StorageObject mask;
extern RegisterFile r;
extern RegisterFile amr;

// bus objects
extern BusALU alu;
extern Bus abus;
extern Bus dbus;
extern Memory mem;

// micro registers
extern Counter mpc;
extern Counter mir;
extern Counter maux;

// micro bus objects
extern BusALU malu;
extern Bus mabus;
extern Memory mmem;

// the list of micro instructions
extern MicroInst microInst[NUMBER_MICRO_FUNCTIONS];

// creates connections in the CPU
void makeConnections();

#endif
