# User variables
CC:=$(CC)
CXX:=$(CXX)
AR:=ar

TRIPLET:=$(shell $(CC) -dumpmachine)
ifneq ($(findstring darwin,$(TRIPLET)),)	#TODO This must be able to more simple
	OS:=mac
else
	OS:=linux
endif
ARCH:=$(shell $(CC) -dumpmachine | sed -e 's/^\([^-]*\).*/\1/')
USE_PAF:=no
USE_SBLANK:=no
LINK_LIBJPEG:=static


# System variables
_SCANNER:=
_NICKNAME:=
_CFLAGS:=
_CFLAGS2:=$(CPPFLAGS)

# Build variables
TARGET=$(OBJPATH)/LLiPm$(_SCANNER).a

INC_DIR:=../../3rdparty/$(OS)/$(ARCH)/include
LIB_DIR:=../../3rdparty/$(OS)/$(ARCH)/lib

INCPAF:=
INCPAF_DIR:=$(INC_DIR)/paf
LIBPAF:=
LIBPAF_DIR:=$(LIB_DIR)
ifneq ($(findstring yes,$(USE_PAF)),)
	INCPAF=-I"$(INCPAF_DIR)"
	ifneq ($(findstring mac,$(OS)),)
		LIBPAF=-F"$(LIBPAF_DIR)" -framework pafcv2
	else
		LIBPAF=-L"$(LIBPAF_DIR)" -lpafcv2
	endif
endif

LIBJPEG:=-ljpeg
LIBJPEG_DIR:=$(LIBDIR) # ebuild exported env var

CFLAGS:=
ifneq ($(findstring arm,$(ARCH)),)
	CFLAGS=$(_CFLAGS2) -fPIC -O2 -c -D$(_SCANNER)_Build $(_CFLAGS)
else
	CFLAGS=$(CFLAGS2) -fPIC -O2 -c -D$(_SCANNER)_Build $(_CFLAGS)
#	CFLAGS=$(CFLAGS2) -msse2 -fPIC -O2 -c -D$(_SCANNER)_Build $(_CFLAGS)
endif
ifneq ($(findstring yes,$(USE_PAF)),)
        CFLAGS+=-DUSE_PAF
endif
ifneq ($(findstring yes,$(USE_SBLANK)),)
        CFLAGS+=-DUSE_LIGHT_WEIGHT_BLANK_SKIP
endif


# Build consts
INCPATH=-I.. -I../../include -I../../Cei -I../../CeiIp -I../../Framework -I../../Test $(INCPAF) $(INCJPEG)

SRCPATH=..
SRCPATH_CEI=../../Cei
SRCPATH_CEIIP=../../CeiIp
SRCPATH_FRAMEWORK=../../Framework
SRCPATH_TEST=../../Test
OBJPATH=./Obj

OBJS=	\
	$(OBJPATH)/AdjustLight.o	\
	$(OBJPATH)/AdjustLightCurve.o	\
	$(OBJPATH)/CeiImg.o	\
	$(OBJPATH)/CeiImgEdit.o	\
	$(OBJPATH)/CeiImgList.o \
	$(OBJPATH)/ceiip.o \
	$(OBJPATH)/CeiLogger.o	\
	$(OBJPATH)/CeiRaster.o	\
	$(OBJPATH)/CollectArray.o	\
	$(OBJPATH)/CollectArrayForDuplex.o	\
	$(OBJPATH)/CollectArrayForSimplex.o	\
	$(OBJPATH)/DRHachiFilter.o	\
	$(OBJPATH)/DRHachiLogger.o	\
	$(OBJPATH)/IPBase.o	\
	$(OBJPATH)/IPController.o	\
	$(OBJPATH)/LLiPm.o	\
	$(OBJPATH)/MakePage.o	\
	$(OBJPATH)/NormalFilter.o	\
	$(OBJPATH)/Shading.o \
	$(OBJPATH)/ExtendBitData12To16.o

OBJADDON=

OBJPATCH=
OBJMOCR=
OBJOCR=
OBJRMBD=
OBJJPEG=
OBJSTEST=	\
	$(OBJPATH)/main.o	\
	$(OBJPATH)/TestLLiPm.o

ifneq ($(findstring -DJPEG_,$(_CFLAGS)),)
	OBJS+=$(OBJJPEG)
endif


help:
	@echo 'make <target> [options]'
	@echo '<target> is one of CR50, DRF120, DRC125, DRC225, DRC240, DRM1060, DRM140, DRM160, DRM260 DRP201, DRP208, DRP215, FSU101, FSU102 or FSU201.'
	@echo '[options] are as follows.'
	@echo 'Option features:'
	@echo '  USE_PAF(Text Orientation)      yes or no (default: no)'
	@echo '  USE_SBLANK(Simple blank skip)  yes or no (default: no)'
	@echo ''
	@echo 'Linking features:'
	@echo '  LINK_LIBJPEG                   static or dynamic (default: static)'
	@echo ''
	@echo 'Target environment:'
	@echo '  OS                             celp, linux or mac (default: Automatically detected by CC)'
	@echo '  ARCH                           arm, i386, i686, x86_64 (default: Automatically detected by CC)'
	@echo '  SYSROOT                        Path to sysroot (default: )'
	@echo '  CC                             Compiler for c (default: gcc)'
	@echo '  CXX                            Compiler for c++ (default: g++)'
	@echo ''
	@echo 'Build for arm-celp:'
	@echo ' make CC=arm-none-linux-gnueabi-gcc CXX=arm-none-linux-gnueabi-g++ SYSROOT="/path/to/sysroot" OS="Celp"'
DRP215:
	@make prebuild
	@make _SCANNER=$@ _CFLAGS= build
DRP208:
	@make prebuild
	@make _SCANNER=$@ _CFLAGS= build
DRC225:
	@make prebuild
	@make _SCANNER=$@ _CFLAGS= build
DRC240:
	@make prebuild
	@make _SCANNER=$@ _CFLAGS= build

prebuild:
	@echo 'Target environment:'
	@echo '  OS                             $(OS)'
	@echo '  ARCH                           $(ARCH)'
	@echo '  SYSROOT                        $(SYSROOT)'
	@echo ''
	@echo '  CC                             $(CC)'
	@echo '  CXX                            $(CXX)'
	@echo ''
	@echo 'Option features:'
ifneq ($(findstring yes,$(USE_PAF)),)
	@echo '  USE_PAF(Text Orientation)      yes'
else
	@echo '  USE_PAF(Text Orientation)      no'
endif
ifneq ($(findstring yes,$(USE_SBLANK)),)
	@echo '  USE_SBLANK(Simple blank skip)  yes'
else
	@echo '  USE_SBLANK(Simple blank skip)  no'
endif
	@echo ''
	@if [ ! -d $(OBJPATH) ]; then mkdir $(OBJPATH); fi

build: $(TARGET)
	@echo 'Linked libraries:'
ifdef INCJPEG
	@echo '  libjpeg                        static'
else ifdef LIBJPEG
	@echo '  libjpeg                        dynamic'
else
	@echo '  libjpeg                        no'
endif
	@echo ''

clean:
	@if test -d $(OBJPATH); then rm -rf $(OBJPATH)/*.[oa]; rmdir $(OBJPATH); else :; fi

$(TARGET): $(OBJS)
	@rm -f $(TARGET)
#ifdef INCJPEG
#	@$(AR) -xo $(LIBJPEG_DIR)/libjpeg.a
#	@$(AR) -qc $@ $? $(shell $(AR) -t $(LIBJPEG_DIR)/libjpeg.a | grep .o)
#	@rm $(shell $(AR) -t $(LIBJPEG_DIR)/libjpeg.a | grep .o)	#TODO File "__.SYMDEF SORTED" cannot be removed by current script
#else
	@$(AR) -rcs $@ $?
#endif

testLLiPm: $(OBJSTEST)
	@$(CXX) -g -o testLLiPm $(INCPATH) $(OBJSTEST) $(OBJPATH)/LLiPm*.a --sysroot=$(SYSROOT) $(LIBPAF) $(LIBJPEG)

#SRCPATH_CEI
$(OBJPATH)/CeiImg.o : $(SRCPATH_CEI)/CeiImg.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/CeiImgEdit.o : $(SRCPATH_CEI)/CeiImgEdit.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/CeiImgFile.o : $(SRCPATH_CEI)/CeiImgFile.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/CeiImgJpg.o : $(SRCPATH_CEI)/CeiImgJpg.c
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/CeiImgList.o : $(SRCPATH_CEI)/CeiImgList.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/CeiLogger.o : $(SRCPATH_CEI)/CeiLogger.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/CeiRaster.o : $(SRCPATH_CEI)/CeiRaster.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@


#SRCPATH_FRAMEWORK
$(OBJPATH)/IPBase.o : $(SRCPATH_FRAMEWORK)/IPBase.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/IPController.o : $(SRCPATH_FRAMEWORK)/IPController.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

#SRCPATH_CEIIP

$(OBJPATH)/ceiip.o : $(SRCPATH_CEIIP)/ceiip.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

#SRCPATH

$(OBJPATH)/AdjustLight.o : $(SRCPATH)/AdjustLight.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/AdjustLightCurve.o : $(SRCPATH)/AdjustLightCurve.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/CollectArray.o : $(SRCPATH)/CollectArray.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/CollectArrayForDuplex.o : $(SRCPATH)/CollectArrayForDuplex.cpp

	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/CollectArrayForSimplex.o : $(SRCPATH)/CollectArrayForSimplex.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/DRHachiFilter.o : $(SRCPATH)/DRHachiFilter.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/DRHachiLogger.o : $(SRCPATH)/DRHachiLogger.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/ExtendBitData12To16.o : $(SRCPATH)/ExtendBitData12To16.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/LLiPm.o : $(SRCPATH)/LLiPm.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/MakePage.o : $(SRCPATH)/MakePage.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/NormalFilter.o : $(SRCPATH)/NormalFilter.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/Shading.o : $(SRCPATH)/Shading.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@


#SRCPATH TEST
$(OBJPATH)/main.o : $(SRCPATH_TEST)/main.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@

$(OBJPATH)/TestLLiPm.o : $(SRCPATH_TEST)/TestLLiPm.cpp
	@$(CC) --sysroot=$(SYSROOT) $(INCPATH) $(CFLAGS) $< -o $@


