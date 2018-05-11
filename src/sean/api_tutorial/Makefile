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
LIBSRCS = 

HEADERS = ${LIBSRCS:%.cc=%.h}

#*** other targets, e.g. linktst.${THIS_DIR}_${OS}
EXECS = ${PREFIX}client1_${OS}${LIBTYPE} \
	${PREFIX}client2_${OS}${LIBTYPE} \
	${PREFIX}client3_${OS}${LIBTYPE} \
	${PREFIX}client4_${OS}${LIBTYPE} \
	${PREFIX}client5_${OS}${LIBTYPE} \
	${PREFIX}client6_${OS}${LIBTYPE} \
	${PREFIX}client7_${OS}${LIBTYPE} \
	${PREFIX}client8_${OS}${LIBTYPE} \
	${PREFIX}client9_${OS}${LIBTYPE} \
	${PREFIX}client10_${OS}${LIBTYPE} \
	${PREFIX}server1_${OS}${LIBTYPE} \
	${PREFIX}server2_${OS}${LIBTYPE} \
	${PREFIX}server3_${OS}${LIBTYPE} \
	${PREFIX}server4_${OS}${LIBTYPE} \
	${PREFIX}server5_${OS}${LIBTYPE} 

IOLIB=${FAKELINKS}
PREFIX=

#*** files to be compiled for non-library targets
EXECSRCS = client1.cc client2.cc client3.cc client4.cc client5.cc \
	client6.cc client7.cc client8.cc client9.cc client10.cc \
	server1.cc server2.cc server3.cc server4.cc server4.cc

OTHERFILES = Makefile hosts INDEX ${EXECSRCS}

include ../Make.local

#*** executables, and the library ${MODULELIB}
all:	
	@echo make sim or make atm

sim:  ${EXECS}
	@echo ready

#will not work if called directly use the atm target
FoRe_LiVe:  ${EXECS}
	@echo ready

atm:
	${MAKE} PREFIX=atm_ IOLIB="${FORELINKS}" FoRe_LiVe --no-print-directory

library:
	@true

${PREFIX}client1_${OS}${LIBTYPE}:    client1.${OTYPE}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} client1.cc ${LINKS} ${IOLIB} \
	 -o ${PREFIX}client1_${OS}${LIBTYPE}
	@echo ${PREFIX}client1_${OS}${LIBTYPE} is ready.

1: ${PREFIX}client1_${OS}${LIBTYPE} ${PREFIX}server1_${OS}${LIBTYPE}
	@true

${PREFIX}client2_${OS}${LIBTYPE}:    client2.${OTYPE}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} client2.cc ${LINKS} ${IOLIB} \
	 -o ${PREFIX}client2_${OS}${LIBTYPE}
	@echo ${PREFIX}client2_${OS}${LIBTYPE} is ready.

2: ${PREFIX}client2_${OS}${LIBTYPE} ${PREFIX}server2_${OS}${LIBTYPE}
	@true

${PREFIX}client3_${OS}${LIBTYPE}:    client3.${OTYPE}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} client3.cc ${LINKS} ${IOLIB} \
	 -o ${PREFIX}client3_${OS}${LIBTYPE}
	@echo ${PREFIX}client3_${OS}${LIBTYPE} is ready.

3: ${PREFIX}client3_${OS}${LIBTYPE} ${PREFIX}server3_${OS}${LIBTYPE}
	@true

${PREFIX}client4_${OS}${LIBTYPE}:    client4.${OTYPE}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} client4.cc ${LINKS} ${IOLIB} \
	 -o ${PREFIX}client4_${OS}${LIBTYPE}
	@echo client4_${OS}${LIBTYPE} is ready.

4: ${PREFIX}client4_${OS}${LIBTYPE} ${PREFIX}server4_${OS}${LIBTYPE}
	@true

${PREFIX}client5_${OS}${LIBTYPE}:    client5.${OTYPE}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} client5.cc ${LINKS} ${IOLIB} \
	 -o ${PREFIX}client5_${OS}${LIBTYPE}
	@echo ${PREFIX}client5_${OS}${LIBTYPE} is ready.

${PREFIX}client6_${OS}${LIBTYPE}:    client6.${OTYPE}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} client6.cc ${LINKS} ${IOLIB} \
	 -o ${PREFIX}client6_${OS}${LIBTYPE}
	@echo ${PREFIX}client6_${OS}${LIBTYPE} is ready.

${PREFIX}client7_${OS}${LIBTYPE}:    client7.${OTYPE}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} client7.cc ${LINKS} ${IOLIB} \
	 -o ${PREFIX}client7_${OS}${LIBTYPE}
	@echo ${PREFIX}client7_${OS}${LIBTYPE} is ready.

${PREFIX}client8_${OS}${LIBTYPE}:    client8.${OTYPE}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} client8.cc ${LINKS} ${IOLIB} \
	 -o ${PREFIX}client8_${OS}${LIBTYPE}
	@echo ${PREFIX}client8_${OS}${LIBTYPE} is ready.

${PREFIX}client9_${OS}${LIBTYPE}:    client9.${OTYPE}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} client9.cc ${LINKS} ${IOLIB} \
	 -o ${PREFIX}client9_${OS}${LIBTYPE}
	@echo ${PREFIX}client9_${OS}${LIBTYPE} is ready.

${PREFIX}client10_${OS}${LIBTYPE}:    client10.${OTYPE}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} client10.cc ${LINKS} ${IOLIB} \
	 -o ${PREFIX}client10_${OS}${LIBTYPE}
	@echo ${PREFIX}client10_${OS}${LIBTYPE} is ready.

${PREFIX}server1_${OS}${LIBTYPE}:    server1.${OTYPE}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} server1.cc ${LINKS} ${IOLIB} \
	 -o ${PREFIX}server1_${OS}${LIBTYPE}
	@echo ${PREFIX}server1_${OS}${LIBTYPE} is ready.

${PREFIX}server2_${OS}${LIBTYPE}:    server2.${OTYPE}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} server2.cc ${LINKS} ${IOLIB} \
	 -o ${PREFIX}server2_${OS}${LIBTYPE}
	@echo ${PREFIX}server2_${OS}${LIBTYPE} is ready.

${PREFIX}server3_${OS}${LIBTYPE}:    server3.${OTYPE}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} server3.cc ${LINKS} ${IOLIB} \
	 -o ${PREFIX}server3_${OS}${LIBTYPE}
	@echo ${PREFIX}server3_${OS}${LIBTYPE} is ready.

${PREFIX}server4_${OS}${LIBTYPE}:    server4.${OTYPE}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} server4.cc ${LINKS} ${IOLIB} \
	 -o ${PREFIX}server4_${OS}${LIBTYPE}
	@echo ${PREFIX}server4_${OS}${LIBTYPE} is ready.

${PREFIX}server5_${OS}${LIBTYPE}:    server5.${OTYPE}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} server5.cc ${LINKS} ${IOLIB} \
	 -o ${PREFIX}server5_${OS}${LIBTYPE}
	@echo ${PREFIX}server5_${OS}${LIBTYPE} is ready.

#*** PLEASE test your module linkage before you push it into src
app_${OS}${LIBTYPE}:    ${EXECSRCS} ${MODULELIB}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} ${EXECSRCS} ${LINKS} ${IOLIB} \
	 -o app_${OS}${LIBTYPE}
	@echo app_${OS}${LIBTYPE} is ready.

app: app_${OS}${LIBTYPE}
	@true

FORE_app_${OS}${LIBTYPE}:    fore_app.${OTYPE}
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS} \
	 ${INCLUDES} fore_app.cc ${LINKS} -lfore_aal_${OS} \
	-lfore_io_${OS} -o FORE_app_${OS}${LIBTYPE}
	@echo FORE_app_${OS}${LIBTYPE} is ready.

FORE_app:	FORE_app_${OS}${LIBTYPE}
	@true
