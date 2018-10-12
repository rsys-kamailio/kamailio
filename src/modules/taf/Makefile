#
# rr module makefile
#
# 
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=taf.so
LIBS=

DEFS+=-I/usr/include/libxml2 \
		-I$(LOCALBASE)/include
LIBS+=/usr/lib/x86_64-linux-gnu/libxml2.so.2 -lxml2
LIBS+=-lpthread

#DEFS+=-DENABLE_USER_CHECK

DEFS+=-DKAMAILIO_MOD_INTERFACE

include ../../Makefile.modules
