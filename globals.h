#ifndef GLOBALS_H
#define GLOBALS_H

#include "includes.h"

typedef unsigned char byte;
typedef unsigned int uint32;

typedef string(*MicroInst)(byte);

class RegisterFile {
private:
  uint32 number;
  Counter ** reg;
public:
  RegisterFile(string name, uint32 bits, uint32 count);
  ~RegisterFile();
  Counter & operator [] (uint32 i);
};

extern const uint32 BIT_SIZE;
const uint32 NUMBER_MICRO_FUNCTIONS = 21;

extern Counter pc;
extern Counter ir;
extern Counter imm;
extern StorageObject zero;
extern StorageObject twentyfour;
extern RegisterFile r;
extern RegisterFile amr;

extern BusALU alu;
extern Bus abus;
extern Bus dbus;
extern Memory mem;

extern Counter mpc;
extern Counter mir;

extern BusALU malu;
extern Bus mabus;
extern Memory mmem;

extern MicroInst microInst[NUMBER_MICRO_FUNCTIONS];

void makeConnections();

#endif
