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
LIBSRCS = API_fsms.cc Controllers.cc ATM_Attributes.cc \
	ATM_Interface.cc ATM_Interface_Components.cc \
	Leaf_FSM.cc Service_FSM.cc Call_FSM.cc \
	Leaf_API.cc Service_API.cc Call_API.cc \
	Callback_Mgmt.cc Memory_Manager.cc

HEADERS = ${LIBSRCS:%.cc=%.h}

#*** other targets, e.g. linktst.${THIS_DIR}_${OS}
EXECS = tester

#*** files to be compiled for non-library targets
EXECSRCS = tester.cc

OTHERFILES = Makefile ${EXECSRCS} lib--\>aal.so

include ../Make.local

#*** executables, and the library ${MODULELIB}
all:	${MODULELIB} ${EXECS}
	@echo ${MODULE}/${THIS_DIR} is ready.

library: ${MODULELIB}
	@true

#*** PLEASE test your module linkage before you push it into src
tester_${OS}${LIBTYPE}:    tester.cc tester.h
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} tester.cc -L../../.. ${LINKS} ${FAKELINKS} \
	 -o tester_${OS}${LIBTYPE}
	@echo tester_${OS}${LIBTYPE} is ready.

tester: tester_${OS}${LIBTYPE}
	@true

app_${OS}${LIBTYPE}:    ${EXECSRCS} ${MODULELIB}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} ${EXECSRCS} ${LINKS} ${FAKELINKS} \
	 -o app_${OS}${LIBTYPE}
	@echo app_${OS}${LIBTYPE} is ready.

app: app_${OS}${LIBTYPE}
	@true

FORE_app_${OS}${LIBTYPE}:    fore_app.cc
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} fore_app.cc ${LINKS} -lfore_aal_${OS} \
	-lfore_io_${OS} -o FORE_app_${OS}${LIBTYPE}
	@echo FORE_app_${OS}${LIBTYPE} is ready.

FORE_app:	FORE_app_${OS}${LIBTYPE}
	@true
