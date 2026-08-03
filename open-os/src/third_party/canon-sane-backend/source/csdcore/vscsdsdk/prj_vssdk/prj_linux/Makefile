#-----------------------
# Target:
#-----------------------
TARGETROOT=../../lib
TARGET=libvssdk.a
#-----------------------
# Default Parameters
#-----------------------
CC=$(CXX)
OPT=$(CPPFLAGS) -Wall -O2 -fPIC -std=c++11
INC=-I../../include
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

VPATH=../../src

SRC=ceilogwrite.cpp \
global_apis.cpp \
command.cpp \
commandhook.cpp \
end_sequence_thread.cpp \
scan_vs.cpp \
scan_sequence_thread.cpp \
sense2vs_error.cpp \
sdk_message.cpp \
sdk_image.cpp \
sdk_information.cpp \
vs.cpp

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
