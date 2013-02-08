#-----------------------------------------------------------------------;
# COMMON MAKEFILE Variables	and Commands							;
#									;
# Generic Template for Legacy Applications ;
# Developed from PICTOR Makefile, Version 1.51, Copyright (c) 1992-94 SoftCircuits	;
# Redistributed by permission.						;
# Copyright (c) 2012 William D. Jones
#-----------------------------------------------------------------------;

COMPILER=6	# 1 = Microsoft C 7.0, 2 = Borland C 3.0, 3 = OpenWatcom 1.9
MODEL=1		# 1 = small/tiny, 2 = medium, 3 = compact, 4 = large/huge

#-----------------------------------------------------------------------;

HELPDIR=.\HELP
UTILDIR=.\UTILS
EDDIR=.\ED
EXAMPLEDIR=.\EXAMPLES
SRCDIR=.\SRC
BINDIR=.\BIN

NMAKEFLAGS= -NOLOGO
#ROOTDIR=

!IF $(COMPILER)==1
SMALL=S
MEDIUM=M
COMPACT=C
LARGE=L
!ELSE
!IF $(COMPILER)==2
SMALL=s
MEDIUM=m
COMPACT=c
LARGE=l
!ELSE
!IF $(COMPILER)==3
SMALL=S
MEDIUM=M
COMPACT=C
LARGE=L
!ELSE
!IF $(COMPILER)==4
#Do Nothing
!ELSE
!IF $(COMPILER)==5
!ELSE
!IF $(COMPILER)==6
!ELSE
!ERROR Unknown compiler specified
!ENDIF
!ENDIF
!ENDIF
!ENDIF

!IF $(MODEL)==1
MODELCHR=$(SMALL)
MODELSTR=small
!ELSE
!IF $(MODEL)==2
MODELCHR=$(MEDIUM)
MODELSTR=medium
!ELSE
!IF $(MODEL)==3
MODELCHR=$(COMPACT)
MODELSTR=compact
!ELSE
!IF $(MODEL)==4
MODELCHR=$(LARGE)
MODELSTR=large
!ELSE
!ERROR Unknown memory model
!ENDIF
!ENDIF
!ENDIF
!ENDIF

#All includes should be relative to the root directory.
!IF $(COMPILER)==1	# Microsoft C
LIBNAME=PICTOR$(MODELCHR)M
CC=cl
CFLAGS=/W3 /A$(MODELCHR) /Ox /c
AS=ml
AFLAGS=/Dmemmodel=$(MODELSTR) /W3 /c
LIB=lib
LIBFLAGS=
LINKER=
LFLAGS=
!ENDIF

!IF $(COMPILER)==2	# Borland C
LIBNAME=PICTOR$(MODELCHR)B
CC=bcc
CFLAGS=/m$(MODELCHR) /O1 /c
AS=tasm
AFLAGS=/Dmemmodel=$(MODELSTR) /W2 /m2 /mx
LIB=tlib
LIBFLAGS=
LINKER=
LFLAGS=
!ENDIF


!IF $(COMPILER)==3	# Watcom C
INCDIR=..\PICTOR
LIBDIR=..\PICTOR
LIBNAME=PICTOR$(MODELCHR)W
CC=wcc
CFLAGS=-m$(MODELCHR) -i=..\$(INCDIR) -d__DOS__ -ecw -q -d2
AS=wasm
AFLAGS=-dmemmodel=$(MODELSTR) -zq -d1
LIB=wlib
LIBFLAGS=-q
LINKER=wlink
LFLAGS=debug all option quiet system dos libpath ..\$(LIBDIR) library{$(LIBNAME).LIB}
!ENDIF

!IF $(COMPILER)==4	# Watcom C (Win32)
#LIBNAME=PICTOR$(MODELCHR)W
#INCDIR=..\PICTOR
#LIBDIR=..\PICTOR
CC=wcc386
CFLAGS=-d__WIN32__ -q -d2
AS=wasm
AFLAGS=-dmemmodel=$(MODELSTR) -zq -d1
LIB=wlib
LIBFLAGS=-q
LINKER=wlink
LFLAGS=debug all option quiet system nt libpath ..\$(LIBDIR)
!ENDIF

!IF $(COMPILER)==5 # 6502 C Compiler
CC=cc65
CFLAGS= -t vic20
AS=ca65
AFLAGS=-dmemmodel=$(MODELSTR) -zq -d1
LIB=wlib
LIBFLAGS=-q
LINKER=wlink
LFLAGS=debug all option quiet system nt libpath ..\$(LIBDIR)
!ENDIF

!IF $(COMPILER)==6	# MinGW C (Win32)
#LIBNAME=PICTOR$(MODELCHR)W
#INCDIR=..\PICTOR
#LIBDIR=..\PICTOR
CC=gcc
CFLAGS=-c -D__WIN32__ -std=c99 -pedantic-errors
AS=gas
AFLAGS=
LIB=ar
LIBFLAGS=
LINKER=gcc
LFLAGS=
!ENDIF


!IFNDEF TOPLEVEL
all: $(EXES)

redo: clean all install

#Assume BIN directory exists.
install: .symbolic
	copy /Y *.EXE ..\$(BINDIR)

dbg-clean: .symbolic
	del *.obj *.lib *.exe

clean: .symbolic
	del *.obj *.lib *.err *.exe
!ENDIF