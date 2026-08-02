#-----------------------
# Target:
#-----------------------

TARGETROOT=../lib
TARGET=libsanesdk.a

#-----------------------
# Default Parameters
#-----------------------

CC=$(CXX)
OPT=$(CPPFLAGS) -Wall -O2 -fPIC -std=c++11

INC=-I../include \
	-I$(SYSROOT)/usr/include
	
ARFLAG=rus
DEF=

#-----------------------
# Compile Option
#-----------------------

-include ./makefile.opt

#-----------------------
# Definition
#-----------------------

.SUFFIXES:.cpp .o

#-----------------------
# Source File
#-----------------------

VPATH=../src

SRC=csdcore.cpp \
get_devices.cpp \
log.cpp \
option_util.cpp \
sane_ctrl.cpp \
sane_global_apis.cpp \
simulation_sane_ctrl.cpp


HED=$(shell ls *.h)
OBJ=$(SRC:.cpp=.o)

#-----------------------
# rules
#-----------------------

all: $(TARGET)
atom: all

$(TARGET): $(OBJ)
	$(AR) $(ARFLAG) $(TARGET) $(OBJ)
	$(RANLIB) $(TARGET)
	cp $(TARGET) $(TARGETROOT)

.cpp.o:
	$(CC) $(OPT) $(DEF) -c $< $(INC) 

dep:
	$(CC) -MM -MG $(SRC) $(INC) >makefile.depend

clean:
	rm -f *.o *.obj *.a
	rm -f *~ *.~*

#-----------------------
-include makefile.depend
