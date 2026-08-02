VERSION=1.0
CC:=$(CC)
CXX:=$(CXX)

PROGRAM=CeiUSBLinux.so
OBJS=CeiUSB_libusb.o


ifneq ($(findstring celp-project0,$(SYSROOT)),)
CELP=yes
CC=$(CC)
CXX=$(CXX)
else
CELP=no
endif

UNAME_S := $(shell uname -s)
UNAME_P := $(shell uname -p)


ifeq ($(CELP),yes)
OBJS2=-lusb-1.0
INCLUDE=-I$(SYSROOT)usr/include/libusb-1.0
CPPFLAGS+=-march=armv7-a -mfloat-abi=hard -mfpu=neon -Wall -O2 -Wno-write-strings -fPIC -shared --sysroot=$(SYSROOT) -Wl,-soname,$(PROGRAM).$(VERSION)
else
ifeq ($(UNAME_S),Darwin)
OBJS2=/usr/local/lib/libusb-1.0.a -lobjc -framework CoreFoundation -framework IOKit 
INCLUDE=-I/usr/local/include
CPPFLAGS+=-Wall -O2 -Wno-write-strings -fPIC -shared --sysroot=$(SYSROOT)
else
OBJS2=-lusb-1.0
INCLUDE=-I$(SYSROOT)usr/include/libusb-1.0
CPPFLAGS+=-Wall -O2 -Wno-write-strings -fPIC -shared --sysroot=$(SYSROOT) -Wl,-soname,$(PROGRAM).$(VERSION)
endif
endif


all:$(PROGRAM)

$(PROGRAM):$(OBJS)
	$(CXX) $(OBJS) $(OBJS2) $(LIBRARY) $(CPPFLAGS) -o $(PROGRAM)
#	cp $(PROGRAM) ../


.cpp.o:
	$(CXX) -c $(INCLUDE) $(CPPFLAGS) $< 

testtool:
	@make -f Makefile_test

update:
	svn export svn://10.5.2.236/repo/linux/CommonDriver/VS/CeiUSBLinux/CeiUSB.h
	svn export svn://10.5.2.236/repo/linux/CommonDriver/VS/CeiUSBLinux/windows_types.h
	svn export svn://10.5.2.236/repo/linux/CommonDriver/VS/CeiUSBLinux/stdafx.h
clean:
	rm -f *.o
	rm -f *.so
