#-----------------------
# Target:
#-----------------------
TARGETROOT=../../lib
TARGET=libipsdk.a
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
VPATH=../../src_ip \
../../src

SRC=ceilogwrite.cpp \
global_apis.cpp \
sdk_image.cpp \
sdk_autosize_deskew_simple.cpp \
sdk_bmp.cpp \
sdk_change_sync.cpp \
sdk_cut_offset_simple.cpp  \
sdk_cutout_simple.cpp \
sdk_gamma.cpp \
sdk_gray2binary.cpp \
sdk_image_accessor.cpp \
sdk_jpeg.cpp \
sdk_mirror_lineorder.cpp \
sdk_mirror.cpp \
sdk_resolution_convert.cpp \
sdk_to_pixel_order_simple.cpp

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
