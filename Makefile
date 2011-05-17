#
# Created by makemake (Sparc Jul 27 2005) on Thu May  5 11:14:33 2011
#

#
# Definitions
#

.SUFFIXES:
.SUFFIXES:	.a .o .c .C .cpp
.c.o:
		$(COMPILE.c) $<
.C.o:
		$(COMPILE.cc) $<
.cpp.o:
		$(COMPILE.cc) $<
.c.a:
		$(COMPILE.c) -o $% $<
		$(AR) $(ARFLAGS) $@ $%
		$(RM) $%
.C.a:
		$(COMPILE.cc) -o $% $<
		$(AR) $(ARFLAGS) $@ $%
		$(RM) $%
.cpp.a:
		$(COMPILE.cc) -o $% $<
		$(AR) $(ARFLAGS) $@ $%
		$(RM) $%

CC =		cc
CXX =		CC

RM = rm -f
AR = ar
LINK.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)
LINK.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)
COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) -c
COMPILE.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c

########## Flags from header.mak

BASE = /home/course/vcsg720
ARCHVER = arch2-5a
CXX = /opt/SUNWspro/bin/CC
CCFLAGS = +w -g -xsb -I$(BASE)/include/$(ARCHVER)
CXXFLAGS = $(CCFLAGS)
LIBFLAGS = -g -L$(BASE)/lib/solaris_SPARC -l$(ARCHVER)
CCLIBFLAGS = $(LIBFLAGS)

########## End of flags from header.mak


CPP_FILES =	 MicroCPU.C MicroInst.C globals.C
C_FILES =	
H_FILES =	 MicroInst.h globals.h includes.h
SOURCEFILES =	$(H_FILES) $(CPP_FILES) $(C_FILES)
.PRECIOUS:	$(SOURCEFILES)
OBJFILES =	 MicroInst.o globals.o

#
# Main targets
#

all:	 MicroCPU Memory.obj.o mMemory.obj.o

Memory.obj.o: Memory.obj
	cpp -P Memory.obj > Memory.obj.o

mMemory.obj.o: mMemory.obj
	cpp -P mMemory.obj > mMemory.obj.o

MicroCPU:	MicroCPU.o $(OBJFILES)
	$(CXX) $(CXXFLAGS) -o MicroCPU MicroCPU.o $(OBJFILES) $(CCLIBFLAGS)

#
# Dependencies
#

MicroCPU.o:	 MicroInst.h globals.h includes.h
MicroInst.o:	 MicroInst.h globals.h includes.h
globals.o:	 MicroInst.h globals.h includes.h

#
# Housekeeping
#

Archive:	archive.tgz

archive.tgz:	$(SOURCEFILES) Makefile
	tar cf - $(SOURCEFILES) Makefile | gzip > archive.tgz

clean:
	-/bin/rm -r $(OBJFILES) MicroCPU.o ptrepository SunWS_cache .sb ii_files core 2> /dev/null
	rm *~

realclean:        clean
	/bin/rm -rf  MicroCPU 
