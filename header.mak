BASE = /home/course/vcsg720
ARCHVER = arch2-5a
CXX = /opt/SUNWspro/bin/CC
CCFLAGS = +w -g -xsb -I$(BASE)/include/$(ARCHVER)
CXXFLAGS = $(CCFLAGS)
LIBFLAGS = -g -L$(BASE)/lib/$(SYSTEM_TYPE) -l$(ARCHVER)
CCLIBFLAGS = $(LIBFLAGS)
