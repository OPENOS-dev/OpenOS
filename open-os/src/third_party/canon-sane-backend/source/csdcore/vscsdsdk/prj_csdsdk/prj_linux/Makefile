#-----------------------
# Target:
#-----------------------
TARGETROOT=../../lib
TARGET=libcsdsdk.a
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
command.cpp \
global_apis.cpp \
tags.cpp \
sdk_tag.cpp \
sdk_tag_basic.cpp \
sdk_tag_pagesize.cpp \
sdk_tag_scan.cpp \
sdk_tag_scanner.cpp \
scan_sequence_thread.cpp \
ip_sequence_thread.cpp \
comp_sequence_thread.cpp \
end_sequence_thread.cpp \
compsequence.cpp \
scan_csd.cpp \
sdk_message.cpp \
sdk_pagesize_table.cpp \
sdk_image.cpp \
sdk_information.cpp \
csd.cpp \
endsequence_csd.cpp \
vs_error2csd_error.cpp

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
