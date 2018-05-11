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

# -*- Makefile -*-
#*** developer/module we are exposed to during development
OTHER_DEVS = 

#*** other libraries we need to link with
OTHER_MODULES = sean codec FW DS

#*** library sources and associated headers
LIBSRCS = 
HEADERS  = ${LIBSRCS:%.cc=%.h}

#*** other targets, e.g. linktst.${THIS_DIR}_${OS}
EXECS = ${EXT_TYPE}server_${OS}${LIBTYPE} ${EXT_TYPE}client_${OS}${LIBTYPE}

#*** files to be compiled for non-library targets
EXECSRCS = ${Server_SRCS} ${Client_SRCS}

OTHERFILES = Makefile ${EXECSRCS}

# makes either live or similation targets
# make sim for simulation targets sim_client, sim_sever
# make or make all for live targets
# live and simulation share the same objects.. diff only in libs linked to.
TYPE=live
IOTYPE=fore
EXT_TYPE=
ifeq (${TYPE},sim)
   IOTYPE=fake
   EXT_TYPE=sim_
endif

LINKS += -l${IOTYPE}_aal_${OS}${LIBTYPE} -l${IOTYPE}_io_${OS}${LIBTYPE}
#LINKS += -lfake_aal_${OS}${LIBTYPE} -lfake_io_${OS}${LIBTYPE}
#LINKS += -lfore_aal_${OS}${LIBTYPE} -lfore_io_${OS}${LIBTYPE}

include ../Make.local

OP=./_o.${OS}

Server_SRCS = BaseService.cc server.cc
Server_OBJS = ${addprefix ${OP}/,${Server_SRCS:%.cc=%.${OTYPE}}}
Client_SRCS = BaseClient.cc client.cc
Client_OBJS = ${addprefix ${OP}/,${Client_SRCS:%.cc=%.${OTYPE}}}

#*** executables, and the library ${MODULELIB}
all:	_o.${OS} $(EXECS)
	@echo ${MODULE}/${THIS_DIR} is ready.

sim:	force
	@${MAKE} all TYPE=sim --no-print-directory

library: ${MODULELIB}
	@true

real_clean:
	@rm -f ${OP}/* *.o *.O ${EXECS}

#*** PLEASE test your module linkage before you push it into src
linktst.${THIS_DIR}_${OS}${LIBTYPE}: ${EXECSRCS:%.cc=%.${OTYPE}} ${MODULELIB}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} ${INCLUDES} \
		${EXECSRCS:%.cc=%.${OTYPE}} ${LINKS} \
		-o linktst.${THIS_DIR}_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/linktst.${THIS_DIR}_${OS}${LIBTYPE} is ready.

_o.${OS}:
	@if [ ! -d _o.${OS} ]; then mkdir _o.${OS}; fi


_o.${OS}/%.${OTYPE}: %.cc
	@if [ ! -d _o.${OS} ]; then mkdir _o.${OS}; fi
	${CCC} -D__${OS}__ ${CCOPTS} ${EXECOPTS} ${INCLUDES} \
	-c $< -o _o.${OS}/$*.${OTYPE}

server: ${EXT_TYPE}server_${OS}${LIBTYPE}
	@true

client: ${EXT_TYPE}client_${OS}${LIBTYPE}
	@true

${EXT_TYPE}server_${OS}${LIBTYPE}: ${Server_OBJS}
	${CCC} -D__${OS}__ ${CCOPTS} ${EXECOPTS} ${INCLUDES} \
	${Server_OBJS} ${LINKS} \
	-o ${EXT_TYPE}server_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/${EXT_TYPE}server_${OS}${LIBTYPE} is ready.

${EXT_TYPE}client_${OS}${LIBTYPE}: ${Client_OBJS}
	${CCC} -D__${OS}__ ${CCOPTS} ${EXECOPTS} ${INCLUDES} \
	${Client_OBJS} ${LINKS} \
	-o ${EXT_TYPE}client_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/${EXT_TYPE}client_${OS}${LIBTYPE} is ready.

clean::
	@rm -f *_${OS} _o.${OS}/*.[oO]

distclean:: clean
	@for O in  _o.*; do \
	  if [ "x$$O" != "x_o.*" ]; then \
	    Os=`echo $$O | sed 's/_o\.//'`; \
	    rm -rf $$O *_$$Os*;\
	  fi \
	done

force:

