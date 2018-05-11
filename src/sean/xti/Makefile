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

FORE_LIB  = /opt/FOREatm/lib
FORE_INC  = /opt/FOREatm/include

#*** developer/module we are exposed to during development
OTHER_DEVS = 

#*** other libraries we need to link with
OTHER_MODULES = codec FW

#*** library sources and associated headers
LIBSRCS = 
HEADERS  = ${LIBSRCS:%.cc=%.h}

#*** other targets, e.g. linktst.${THIS_DIR}_${OS}
EXECS = 
#linktst.${THIS_DIR}_${OS}${LIBTYPE}

#*** files to be compiled for non-library targets
EXECSRCS = linktst.cc 

OTHERFILES = Makefile ${EXECSRCS} .needs_depend_local

include ../Make.local

FORE:=${shell ${COMMON}/fore_exists.sh}

ifeq (${FORE},yes)
	buildable_libraries = ${FAKELIB} ${XTIOLIB}
else
	buildable_libraries = ${FAKELIB}
endif

ifeq (${GRANDPARENT},src)
   IO_LIBDIR=${LIBDIR}
else
   IO_LIBDIR=..
endif

FAKELIB=${IO_LIBDIR}/libfake_io_${OS}${LIBTYPE}.${LIBEXT}
XTIOLIB=${IO_LIBDIR}/libfore_io_${OS}${LIBTYPE}.${LIBEXT}
#*** executables, and the library ${MODULELIB}
all:	library ${EXECS}
	@echo ${MODULE}/${THIS_DIR} is ready.

library:  ${buildable_libraries}
	@true

#*** PLEASE test your module linkage before you push it into src
linktst.${THIS_DIR}_${OS}${LIBTYPE}:	${EXECSRCS} 
	$(C++) -D__${OS}__ $(CCOPTS) ${EXECOPTS} ${INCLUDES} \
	  ${EXECSRCS} ${LINKS} \
	  -l_fake_io_${OS}${DEBUGLIBTYPE} -lfake_aal_${OS}${DEBUGLIBTYPE} \
	  -o linktst.${THIS_DIR}_${OS}${LIBTYPE}
	@echo ${MODULE}/${THIS_DIR}/linktst.${THIS_DIR}_${OS}${LIBTYPE} is ready.
linktst: linktst.${THIS_DIR}_${OS}${LIBTYPE}
	@true

${FAKELIB}: sean_io_fake_${OS}.${OTYPE} sim_interface_${OS}.${OTYPE}
	${LD} ${LDOPTS} sean_io_fake_${OS}.${OTYPE} \
	sim_interface_${OS}.${OTYPE} -o ${FAKELIB}

${XTIOLIB}:	sean_io_fore_${OS}.${OTYPE}  xtio_${OS}.${OTYPE} 
	${LD} ${LDOPTS} sean_io_fore_${OS}.${OTYPE} xtio_${OS}.${OTYPE} \
		-o ${XTIOLIB}

sim_interface_${OS}.${OTYPE}: sim_interface.cc
	$(CCC) -D__${OS}__ $(CCOPTS) ${INCLUDES} \
	-c sim_interface.cc -o sim_interface_${OS}.${OTYPE}

sean_io_fore_${OS}.${OTYPE}:	sean_io.cc sean_io.h
	$(CCC) -D__FORE_IO__ -D__${OS}__ $(CCOPTS) -I$(FORE_INC) ${INCLUDES} \
	-c sean_io.cc -o sean_io_fore_${OS}.${OTYPE}

xtio_${OS}.${OTYPE} :
	$(CCC) -D__FORE_IO__ -D__${OS}__ $(CCOPTS) -I$(FORE_INC) ${INCLUDES} \
	-c xtio.cc -o xtio_${OS}.${OTYPE}

sean_io_fake_${OS}.${OTYPE}:	sean_io.cc sean_io.h
	$(CCC) -D__FAKE_IO__ -D__${OS}__ $(CCOPTS) ${INCLUDES} \
	-c sean_io.cc -o sean_io_fake_${OS}.${OTYPE}

testio: testio.cc
	$(C++) -D__${OS}__ $(CCOPTS) ${EXECOPTS} ${INCLUDES} \
	  testio.cc ${LINKS} -lfore_io_${OS} -lfore_aal_${OS}\
	  -o testio
	@echo ${MODULE}/${THIS_DIR}/testio is ready.

#
# sean_io.cc produces sean_io_{fore,fake}.[oO]
# xtio.cc    produces xtio_${OS}.[oO]
#
# make depend_local (at end of make depend if .needs_depend_local exists)
# produces rules for sean_io.o and xtio.o
#
# this rule fixes all that
depend_local:
	@touch .depend_${OS}; rm -f .fake_depend .fore_depend
	@$(CCC) -M -D__FAKE_IO__ -D__${OS}__ $(CCOPTS) ${INCLUDES}          \
	    sean_io.cc    > .fake_depend;                                   \
	  sed 's/^\([A-Za-z_]*\)\.o *:/\1_fake_${OS}\.o \1_fake${OS}\.O :/' \
	  < .fake_depend >>.depend_${OS}
	@if [ "${FORE}" = "yes" ]; then                               \
	  $(CCC) -M -D__FORE_IO__ -D__${OS}__ $(CCOPTS) -I$(FORE_INC) \
	    ${INCLUDES}                                               \
	    sean_io.cc xtio.cc > .fore_depend;                        \
	  sed 's/^\([A-Za-z_]*\)\.o *:/\1_${OS}\.o \1_${OS}\.O :/'    \
	   < .fore_depend >>.depend_${OS};                            \
	 fi
	@rm -f .fake_depend .fore_depend 

clean::
	rm -f *_${OS}.[oO] ${buildable_libraries} sim_interface.${OTYPE}

distclean::
	rm -f *.[oO] ../libfake* ../libfore* ${buildable_libraries}
