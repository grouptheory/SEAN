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

#*** developer/module we are exposed to during development
OTHER_DEVS = 

#*** other libraries we need to link with
OTHER_MODULES = codec FW DS

#*** library sources and associated headers
LIBSRCS = abstract_ipc.cc tcp_ipc.cc IPC_Visitors.cc async.cc

HEADERS  = ${LIBSRCS:%.cc=%.h}

#*** other targets, e.g. linktst.${THIS_DIR}_${OS}
EXECS = gecko_${OS}${LIBTYPE}

#*** files to be compiled for non-library targets
EXECSRCS = linktst.cc 

OTHERFILES = Makefile ${EXECSRCS}

include ../Make.local

#*** executables, and the library ${MODULELIB}
all:	${MODULELIB}
	@echo ${MODULE}/${THIS_DIR} is ready.

library: ${MODULELIB}
	@true

#*** PLEASE test your module linkage before you push it into src
linktst_${OS}${LIBTYPE}:	${EXECSRCS:.cc=.o} ${MODULELIB} linktst.h
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} ${INCLUDES} \
		${EXECSRCS} ${LINKS}  ${FAKELINKS} \
		 -o linktst_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/linktst_${OS}${LIBTYPE} is ready.

test: 	linktst_${OS}${LIBTYPE}
	@true

gecko_${OS}${LIBTYPE}:	gecko.cc abstract_ipc.cc tcp_ipc.cc IPC_Visitors.cc 
	${CCC} -D__${OS}__ $(CCOPTS) ${EXECOPTS} ${INCLUDES} \
		gecko.cc abstract_ipc.cc tcp_ipc.cc IPC_Visitors.cc \
		${LINKS} ${FAKELINKS} ${OS_LDLIBS} -o gecko_${OS}${LIBTYPE}
	@echo "gecko: any of numerous small harmless chiefly tropical and nocturnal insectivorous lizards (family Gekkonidae)"

gecko:	gecko_${OS}${LIBTYPE}
