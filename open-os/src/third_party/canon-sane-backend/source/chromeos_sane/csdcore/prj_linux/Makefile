#-----------------------
# Target:
#-----------------------
TARGETROOT=../lib
TARGET=CsdCore.so
VERSION=1.0.0
#-----------------------
# Default Parameters
#-----------------------
CC=$(CXX)
OPT=$(CPPFLAGS) -shared -Wall -O2 -fPIC -pthread -std=c++11 -Wl,-soname,$(TARGET).$(VERSION)
OPT2=-ldl

INC=-I../include \
    -I../../vscsdsdk/include \
    -I$(SYSROOT)/usr/include
LIBPATH=-L$(LIBDIR)
        
RPATH=-Wl,-rpath=\$$ORIGIN
LIB=
ARFLAG=rus
#DEF=-DDEBUG
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
SRC=csdsdkmain.cpp \
scansequence_csd.cpp \
ipsequence_csd.cpp \
settings_csd.cpp \
vs_csd.cpp \
tags_csd.cpp
    
HED=$(shell ls *.h)
OBJ=$(SRC:.cpp=.o)

#-----------------------
# rules
#-----------------------

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OPT) -o $(TARGET) $(OBJ) ../../../csdcore/vscsdsdk/prj_csdsdk/prj_linux/libcsdsdk.a ../../../csdcore/vscsdsdk/prj_ipsdk/prj_linux/libipsdk.a $(OPT2) $(LIBPATH) $(LIB) $(LIBA) $(RPATH)


.cpp.o:
	$(CC) $(OPT) $(DEF) -c $< $(INC) 

dep:
	$(CC) -MM -MG $(SRC) $(INC) >makefile.depend

clean:
	rm -f *.o *.obj *.so
	rm -f *~ *.~*

#-----------------------
-include makefile.depend
