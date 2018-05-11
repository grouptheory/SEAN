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
OTHER_MODULES = sean codec DS FW

#*** library sources and associated headers
LIBSRCS = 
HEADERS  = ${LIBSRCS:%.cc=%.h}

#*** other targets, e.g. linktst.${THIS_DIR}_${OS}
EXECS = active_exit_${OS}${LIBTYPE} \
	active_hangup_${OS}${LIBTYPE} \
	call_blaster_${OS}${LIBTYPE} \
	call_periodic_${OS}${LIBTYPE} \
	DuplicateServicesTest_${OS}${LIBTYPE} \
	echo_client_${OS}${LIBTYPE} \
	echo_screen_${OS}${LIBTYPE} \
	HangupActiveTest_${OS}${LIBTYPE} \
	HangupPeriodicTest_${OS}${LIBTYPE} \
	NeverAcceptTest_${OS}${LIBTYPE} \
	NServiceTest_${OS}${LIBTYPE} \
	OneServiceTest_${OS}${LIBTYPE} \
	periodic_hangup_${OS}${LIBTYPE}


#*** files to be compiled for non-library targets
EXECSRCS =  ${ACTIVE_EXIT_SRCS} \
	${ACTIVE_HANGUP_SRCS} \
	${CALL_BLASTER_SRCS} \
	${CALL_PERIOD_SRCS} \
	${DUPLICATE_SRCS} \
	${ECHO_CLIENT_SRCS} \
	${ECHO_SCREEN_SRCS} \
	${HANGACTIVE_SRCS} \
	${HANGPERIODIC_SRCS} \
	${NEVER_SRCS} \
	${NSERVICE_SRCS} \
	${ONESERVICE_SRCS} \
	${PERIODIC_HANG_SRCS}


OTHERFILES = Makefile ${EXECSRCS}

LINKS += -lfake_aal_${OS}${LIBTYPE} -lfake_io_${OS}${LIBTYPE}

include ../Make.local
OP=./_o.${OS}
COMMON_SRCS = BaseController.cc PrintFunctions.cc
CLIENT_SRCS = BaseClient.cc ${COMMON_SRCS}
CLIENT_SERVER_SRCS = AcceptingOneService.cc BaseClient.cc ${COMMON_SRCS}


ACTIVE_EXIT_SRCS = active_exit.cc ${CLIENT_SERVER_SRCS}
ACTIVE_EXIT_OBJS = ${addprefix ${OP}/,${ACTIVE_EXIT_SRCS:%.cc=%.${OTYPE}}}

ACTIVE_HANGUP_SRCS = active_hangup.cc ${CLIENT_SERVER_SRCS}
ACTIVE_HANGUP_OBJS = ${addprefix ${OP}/,${ACTIVE_HANGUP_SRCS:%.cc=%.${OTYPE}}}

CALL_BLASTER_SRCS = call_blaster.cc ${CLIENT_SRCS}
CALL_BLASTER_OBJS = ${addprefix ${OP}/,${CALL_BLASTER_SRCS:%.cc=%.${OTYPE}}}

CALL_PERIODIC_SRCS = call_periodic.cc ${CLIENT_SRCS}
CALL_PERIODIC_OBJS = ${addprefix ${OP}/,${CALL_PERIODIC_SRCS:%.cc=%.${OTYPE}}}

DUPLICATE_SRCS = DuplicateServicesTest.cc DuplicateServices.cc ${COMMON_SRCS}
DUPLICATE_OBJS = ${addprefix ${OP}/,${DUPLICATE_SRCS:%.cc=%.${OTYPE}}}

ECHO_CLIENT_SRCS = echo_client.cc EchoService.cc ${CLIENT_SERVER_SRCS}
ECHO_CLIENT_OBJS = ${addprefix ${OP}/,${ECHO_CLIENT_SRCS:%.cc=%.${OTYPE}}}

ECHO_SCREEN_SRCS = echo_client.cc EchoService.cc ${CLIENT_SERVER_SRCS}
ECHO_SCREEN_OBJS = ${addprefix ${OP}/,${ECHO_SCREEN_SRCS:%.cc=%.${OTYPE}}}

HANGACTIVE_SRCS = HangupActiveTest.cc HangupActive.cc NeverAccept.cc \
		${COMMON_SRCS}
HANGACTIVE_OBJS = ${addprefix ${OP}/,${HANGACTIVE_SRCS:%.cc=%.${OTYPE}}}

HANGPERIODIC_SRCS = HangupPeriodicTest.cc HangupPeriodic.cc HangupActive.cc \
		NeverAccept.cc ${COMMON_SRCS}
HANGPERIODIC_OBJS = ${addprefix ${OP}/,${HANGPERIODIC_SRCS:%.cc=%.${OTYPE}}}

NEVER_SRCS = NeverAcceptTest.cc NeverAccept.cc ${COMMON_SRCS}
NEVER_OBJS = ${addprefix ${OP}/,${NEVER_SRCS:%.cc=%.${OTYPE}}}

NSERVICE_SRCS = NServiceTest.cc NService.cc ${COMMON_SRCS}
NSERVICE_OBJS = ${addprefix ${OP}/,${NSERVICE_SRCS:%.cc=%.${OTYPE}}}

ONESERVICE_SRCS = OneServiceTest.cc OneService.cc ${COMMON_SRCS}
ONESERVICE_OBJS = ${addprefix ${OP}/,${ONESERVICE_SRCS:%.cc=%.${OTYPE}}}

PERIODIC_HANG_SRCS = periodic_hangup.cc ${CLIENT_SERVER_SRCS}
PERIODIC_HANG_OBJS = ${addprefix ${OP}/,${PERIODIC_HANG_SRCS:%.cc=%.${OTYPE}}}

#*** executables, and the library ${MODULELIB}
all:	_o.${OS} $(EXECS)
	@echo ${MODULE}/${THIS_DIR} is ready.

library: _o.${OS} ${MODULELIB}
	@true

#*** PLEASE test your module linkage before you push it into src
linktst.${THIS_DIR}_${OS}${LIBTYPE}:	${EXECSRCS:%.cc=%.${OTYPE}} 
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} ${INCLUDES} \
	${EXECSRCS:%.cc=%.${OTYPE}} ${LINKS} \
	-o linktst.${THIS_DIR}_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/linktst.${THIS_DIR}_${OS}${LIBTYPE} is ready.

_o.${OS}:
	if [ ! -d _o.${OS} ]; then mkdir _o.${OS}; fi


_o.${OS}/%.${OTYPE}: %.cc
	@if [ ! -d _o.${OS} ]; then mkdir _o.${OS}; fi
	${CCC} -D__${OS}__ ${CCOPTS} ${EXECOPTS} ${INCLUDES} \
	-c $< -o _o.${OS}/$*.${OTYPE}


active_exit_${OS}${LIBTYPE}: ${ACTIVE_EXIT_OBJS}
	${CCC} -D__${OS}__ ${CCOPTS} ${EXECOPTS} ${INCLUDES} \
	${ACTIVE_EXIT_OBJS} ${LINKS} \
	-o active_exit_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/active_exit_${OS}${LIBTYPE} is ready.

active_exit:	active_exit_${OS}${LIBTYPE}
	@true

active_hangup_${OS}${LIBTYPE}:	${ACTIVE_HANGUP_OBJS}
	${CCC} -D__${OS}__ ${CCOPTS} ${EXECOPTS} ${INCLUDES} \
	${ACTIVE_HANGUP_OBJS} ${LINKS} \
	-o active_hangup_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/active_hangup_${OS}${LIBTYPE} is ready.

active_hangup:	active_hangup_${OS}${LIBTYPE}
	@true

call_blaster_${OS}${LIBTYPE}:	${CALL_BLASTER_OBJS}
	${CCC} -D__${OS}__ ${CCOPTS} ${EXECOPTS} ${INCLUDES} \
	${CALL_BLASTER_OBJS} ${LINKS} \
	-o call_blaster_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/call_blaster_${OS}${LIBTYPE} is ready.

call_blaster:	call_blaster_${OS}${LIBTYPE}
	@true

call_periodic_${OS}${LIBTYPE}:	${CALL_PERIODIC_OBJS} 
	${CCC} -D__${OS}__ ${CCOPTS} ${EXECOPTS} ${INCLUDES} \
	${CALL_PERIODIC_OBJS} ${LINKS} \
	-o call_periodic_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/call_periodic_${OS}${LIBTYPE} is ready.

call_periodic:	call_periodic_${OS}${LIBTYPE}
	@true

DuplicateServicesTest_${OS}${LIBTYPE}:	${DUPLICATE_OBJS} 
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} ${INCLUDES} \
	${DUPLICATE_OBJS} ${LINKS} \
	-o DuplicateServicesTest_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/DuplicateServicesTest_${OS}${LIBTYPE} is ready.

echo_client_${OS}${LIBTYPE}:	${ECHO_CLIENT_OBJS} 
	${CCC} -D__${OS}__ ${CCOPTS} ${EXECOPTS} ${INCLUDES} \
	${ECHO_CLIENT_OBJS} ${LINKS} \
	-o echo_client_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/echo_client_${OS}${LIBTYPE} is ready.

echo_client:	echo_client_${OS}${LIBTYPE}
	@true

echo_screen_${OS}${LIBTYPE}:	${ECHO_SCREEN_OBJS} 
	${CCC} -D__${OS}__ -DECHO_S ${CCOPTS} ${EXECOPTS} ${INCLUDES} \
	${ECHO_SCREEN_OBJS} ${LINKS} \
	-o echo_screen_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/echo_screen_${OS}${LIBTYPE} is ready.

echo_screen:	echo_screen_${OS}${LIBTYPE}
	@true

HangupActiveTest_${OS}${LIBTYPE}: ${HANGACTIVE_OBJS} 
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} ${INCLUDES} \
	${HANGACTIVE_OBJS} ${LINKS} \
	-o HangupActiveTest_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/HangupActiveTest_${OS}${LIBTYPE} is ready.


HangupPeriodicTest_${OS}${LIBTYPE}: ${HANGPERIODIC_OBJS} 
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} ${INCLUDES} \
	${HANGPERIODIC_OBJS} ${LINKS} \
	-o HangupPeriodicTest_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/HangupPeriodicTest_${OS}${LIBTYPE} is ready.


NeverAcceptTest_${OS}${LIBTYPE}: ${NEVER_OBJS} 
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} ${INCLUDES} \
	${NEVER_OBJS} ${LINKS} \
	-o NeverAcceptTest_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/NeverAcceptTest_${OS}${LIBTYPE} is ready.


NServiceTest_${OS}${LIBTYPE}:	${NSERVICE_OBJS} 
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} ${INCLUDES} \
	${NSERVICE_OBJS} ${LINKS} \
	-o NServiceTest_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/NServiceTest_${OS}${LIBTYPE} is ready.

OneServiceTest_${OS}${LIBTYPE}:	${ONESERVICE_OBJS} 
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} ${INCLUDES} \
	${ONESERVICE_OBJS} ${LINKS} \
	-o OneServiceTest_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/OneServiceTest_${OS}${LIBTYPE} is ready.

periodic_hangup_${OS}${LIBTYPE}: ${PERIODIC_HANG_OBJS} 
	${CCC} -D__${OS}__ ${CCOPTS} ${EXECOPTS} ${INCLUDES} \
	${PERIODIC_HANG_OBJS} ${LINKS} \
	-o periodic_hangup_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/periodic_hangup_${OS}${LIBTYPE} is ready.

periodic_hangup:	periodic_hangup_${OS}${LIBTYPE}
	@true

clean::
	rm -f _o.${OS}/* *_${OS}

distclean:: 
	@for O in  _o.*; do \
	  if [ "x$$O" != "x_o.*" ]; then \
	    Os=`echo $$O | sed 's/_o\.//'`; \
	    rm -rf $$O *_$$Os;\
	  fi \
	done
