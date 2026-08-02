#-----------------------
# Target:
#-----------------------
TARGET_DEFAULT=vs.so
VERSION=1.0.0
TARGETROOT=../lib
ifdef product
  TARGET=$(product)vs.so
else
  TARGET=$(TARGET_DEFAULT)
endif

ifdef version
  VERSION=$(version)
endif


#-----------------------
# Default Parameters
#-----------------------

CC=$(CXX)
OPT=$(CPPFLAGS) -shared -Wall -O2 -fPIC -pthread -std=c++11 -Wl,-soname,$(TARGET).$(VERSION)

INC=-I../include \
    -I../../vscsdsdk/include \
    -I../../adjustsdk/include \
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

VPATH=../src \
      ../../vs_common/src
	  
SRC=vssdkmain.cpp \
scansequence.cpp \
endsequence_drc240.cpp \
commandhook.cpp \
settings_drc240.cpp \
adjust.cpp \
shading.cpp \
sort.cpp \
toimage_drc240.cpp \
trim.cpp
    
HED=$(shell ls *.h)
OBJ=$(SRC:.cpp=.o)

#-----------------------
# rules
#-----------------------

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OPT) -o $(TARGET) $(OBJ) ../../../csdcore/vscsdsdk/prj_vssdk/prj_linux/libvssdk.a ../../../csdcore/vscsdsdk/prj_ipsdk/prj_linux/libipsdk.a ../../adjustsdk/LLiPmDRHachi/LLiPm.linuxproj/Obj/LLiPmDRC240.a ../../../csdcore/vscsdsdk/prj_csdsdk/prj_linux/libcsdsdk.a $(LIBPATH) $(LIB) $(LIBA) $(RPATH)

.cpp.o:
	$(CC) $(OPT) $(DEF) -c $< $(INC) 

dep:
	$(CC) -MM -MG $(SRC) $(INC) >makefile.depend

clean:
	rm -f *.o *.obj *.so
	rm -f *~ *.~*

#-----------------------
-include makefile.depend
