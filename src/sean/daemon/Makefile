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
OTHER_MODULES = codec FW

#*** library sources and associated headers
LIBSRCS = Streams.cc DataStack.cc SigStack.cc \
SwitchFabric.cc Switch.cc \
Host.cc Application.cc \
Fiber.cc ATM_PDU_Visitor.cc \
HostFile.cc

HEADERS  = ${LIBSRCS:%.cc=%.h}

#*** other targets, e.g. linktst.${THIS_DIR}_${OS}
EXECS = simswitch sd
# sd

#*** files to be compiled for non-library targets
EXECSRCS = signalling_daemon.cc simswitch.cc

OTHERFILES = Makefile host.file ${EXECSRCS}

include ../Make.local

#*** executables, and the library ${MODULELIB}
all:	${MODULELIB} ${EXECS}
	@echo ${MODULE}/${THIS_DIR} is ready.

library: ${MODULELIB}
	@true

ifeq (${FORE},yes)
#*** PLEASE test your module linkage before you push it into src
sd_${OS}${LIBTYPE}:	signalling_daemon.cc 
	$(C++) -D__${OS}__ $(CCOPTS) -L${ROOT}/lib/${OS} ${EXECOPTS} \
		${INCLUDES} signalling_daemon.cc ${LINKS} ${FORELINKS} \
		-o sd_${OS}${LIBTYPE}
	@echo The SEAN signalling daemon: sd_${OS}${LIBTYPE} is ready.
else
sd_${OS}${LIBTYPE}:	
	@echo ERROR: sd_${OS}${LIBTYPE} cannot be built because 
	@echo '       the FORE libraries could not be found.'
endif

sd:	sd_${OS}${LIBTYPE}
	@true

simswitch_${OS}${LIBTYPE}:	simswitch.cc 
	$(C++) -D__${OS}__ $(CCOPTS) -L${ROOT}/lib/${OS} ${EXECOPTS} \
		${INCLUDES} simswitch.cc ${LINKS} ${FAKELINKS} \
		-o simswitch_${OS}${LIBTYPE}
	@echo The SEAN simswitch: simswitch_${OS}${LIBTYPE} is ready.

simswitch:	simswitch_${OS}${LIBTYPE}
	@true

#---------------------------------------------------------------------------
#---------- internal testing.  unsupported targets.

netd_${OS}${LIBTYPE}:	network_daemon.cc 
	$(C++) -D__${OS}__ $(CCOPTS) -L${ROOT}/lib/${OS} ${EXECOPTS} \
		${INCLUDES} network_daemon.cc ${LINKS} ${FAKELINKS} \
		 -o netd_${OS}${LIBTYPE}
	@echo The SEAN network daemon: netd_${OS}${LIBTYPE} is ready.

netd:	netd_${OS}${LIBTYPE}
	@true

filed_${OS}${LIBTYPE}:	file_daemon.cc 
	$(C++) -D__${OS}__ $(CCOPTS) -L${ROOT}/lib/${OS} ${EXECOPTS} \
		${INCLUDES} file_daemon.cc ${LINKS} ${FAKELINKS} \
		 -o filed_${OS}${LIBTYPE}
	@echo The SEAN file net file daemon: filed_${OS}${LIBTYPE} is ready.

filed:	filed_${OS}${LIBTYPE}
	@true

sim_net_${OS}${LIBTYPE}:	sim_network.cc
	$(C++) -D__${OS}__ $(CCOPTS) -L${ROOT}/lib/${OS} ${EXECOPTS} \
		${INCLUDES} sim_network.cc ${LINKS} ${FAKELINKS} \
		 -o sim_net_${OS}${LIBTYPE}
	@echo The SEAN simulated network: sim_net_${OS}${LIBTYPE} is ready.

sim_net: sim_net_${OS}${LIBTYPE}
	@true

clean::
	@rm -rf *_${OS} core *.output _o.${OS}

distclean::
	@for O in  _o.*; do \
	  if [ "x$$O" != "x_o.*" ]; then \
	    Os=`echo $$O | sed 's/_o\.//'`; \
	    rm -rf $$O *_$$Os*;\
	  fi \
	done
