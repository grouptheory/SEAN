# +++++++++++++++
#    S E A N        ---  Signalling Entity for ATM Networks  ---
# +++++++++++++++
# Version: 1.0.1
# 
# 			  Copyright (c) 1998
# 		 Naval Research Laboratory (NRL/CCS)
# 			       and the
# 	  Defense Advanced Research Projects Agency (DARPA)
# 
# 			 All Rights Reserved.
# 
# Permission to use, copy, and modify this software and its
# documentation is hereby granted, provided that both the copyright notice and
# this permission notice appear in all copies of the software, derivative
# works or modified versions, and any portions thereof, and that both notices
# appear in supporting documentation.
# 
# NRL AND DARPA ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
# DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM
# THE USE OF THIS SOFTWARE.
# 
# NRL and DARPA request users of this software to return modifications,
# improvements or extensions that they make to:
# 
#                 sean-dev@cmf.nrl.navy.mil
#                         -or-
#                Naval Research Laboratory, Code 5590
#                Center for Computation Science
#                Washington, D.C.  20375
# 
# and grant NRL and DARPA the rights to redistribute these changes in
# future upgrades.

FORE_DIR = /opt/FOREatm
FORE_INC = -I$(FORE_DIR)/include 
#FORE_LIB = -R$(FORE_DIR)/lib -L$(FORE_DIR)/lib -lans -lnsl
FW_INC = -I$(TOP)/src

SNMP_DIR = .
SNMP_DEF = -D_XPG4_EXTENDED -Dsun -D__unix
SNMP_INC = -I$(SNMP_DIR)/include -I$(SNMP_DIR)/lib
SNMP_LIB = -R$(SNMP_DIR)/lib -L$(SNMP_DIR)/lib -lsnmp_${OS}${LIBTYPE} \
	   -lsocket -lintl -lposix4


#*** developer/module we are exposed to during development
OTHER_DEVS = 

#*** other libraries we need to link with
OTHER_MODULES = 

#*** library sources and associated headers
LIBSRCS = 

HEADERS  = ${LIBSRCS:%.cc=%.h}

#*** other targets, e.g. linktst.${THIS_DIR}_${OS}
EXECS = ilmid_${OS}${LIBTYPE}

OP=./_o.${OS}
#*** files to be compiled for non-library targets
EXECSRCS = ilmiTimers.cc ilmiVisitors.cc ilmiTerm.cc ilmi.cc ilmid.cc xti_nsap.cc

OTHERFILES = Makefile ${EXECSRCS} lib include

# to make make depend work reliably
INCLUDES=$(SNMP_INC) $(FORE_INC)

include ../Make.local

# this must be after include ../Make.local to override global setting
EXECOBJS =  ${addprefix ${OP}/,${EXECSRCS:%.cc=%.${OTYPE}}}

#*** executables, and the library ${MODULELIB}

all:	_o.${OS} ${EXECS}
	@echo ${MODULE}/${THIS_DIR} is ready.

_o.${OS}:
	@if [ ! -d _o.${OS} ]; then mkdir _o.${OS}; fi


_o.${OS}/%.${OTYPE}: %.cc
	@if [ ! -d _o.${OS} ]; then mkdir _o.${OS}; fi
	${CCC}  $(SNMP_DEF) $(SNMP_INC) $(FW_INC) $(FORE_INC) \
	-c $< -o _o.${OS}/$*.${OTYPE}

ilmid_${OS}${LIBTYPE}: 	$(EXECOBJS)  ./lib/libsnmp_${OS}${LIBTYPE}.so	
	$(CCC) $(SNMP_DEF) $(SNMP_INC) $(FW_INC) \
	$(FORE_INC) -L$(FORE_LIB) -lans -lnsl $(SNMP_LIB) \
	-L$(TOP)/lib/$(OS) -lFW_${OS}${LIBTYPE} -lDS_${OS}${LIBTYPE} \
	-lctorsFW_${OS}${LIBTYPE} -lctorsDS_${OS}${LIBTYPE} $(RLINKS) \
	 $(EXECOBJS) -o ilmid_${OS}${LIBTYPE} 

# Note: lib/Makefile (HP/snmp) is hardwired to work with Solaris
./lib/libsnmp_${OS}${LIBTYPE}.so:		
	cd lib; $(MAKE) OS=${OS} OTYPE=${OTYPE} LIBTYPE=${LIBTYPE} CXX="${CCC}"

library:
	@true

clean::
	rm -rf _o.${OS} *_${OS} ./lib/libsnmp_${OS}*.so
	cd lib; ${MAKE} clean OS=${OS}

# will leave other OS _o files -- needs to be fixed.
distclean::
	rm -rf _o.${OS}
	cd lib; make distclean OS=${OS}


parse: parse.cc  ./lib/libsnmp_${OS}.so
	$(CCC) $(SNMP_DEF) $(SNMP_INC) $(SNMP_LIB) \
	  parse.cc  $(RLINKS) -o parse
