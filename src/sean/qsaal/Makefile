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

# -*-makefile-*-

#*** developer/module we are exposed to during development
OTHER_DEVS =



#*** other libraries we need to link with
OTHER_MODULES = codec FW

#*** library sources and associated headers
LIBSRCS = sscop_aal.cc sscf.cc sscop.cc def_sscop.cc idle.cc \
	inconn.cc inrec.cc inres.cc outconn.cc \
	outdisc.cc outrec.cc outres.cc recresp.cc \
	ready.cc sscop_timers.cc sscop_visitors.cc

HEADERS  = ${LIBSRCS:%.cc=%.h}

OTHERFILES = Makefile ${EXECSRCS} .needs_depend_local

include ../Make.local

ifeq (${GRANDPARENT},src)
   AAL_LIBDIR=${LIBDIR}
else
   AAL_LIBDIR=..
endif

FORE_AAL_LIB=${AAL_LIBDIR}/libfore_aal_${OS}${LIBTYPE}.${LIBEXT}
FAKE_AAL_LIB=${AAL_LIBDIR}/libfake_aal_${OS}${LIBTYPE}.${LIBEXT}

# ---------- FLAGS -----------

ifeq (${FORE},yes)
	buildable_libs = ${FAKE_AAL_LIB} ${FORE_AAL_LIB}
else
	buildable_libs = ${FAKE_AAL_LIB}
endif

EXECSRCS = 

# ---------- TARGETS -----------


# ---------- FLAGS -----------

ifeq (${FORE},yes)
_fore_sscop_${OS}${LIBTYPE}:	${EXECSRCS} ${MODULELIB} 
	$(CCC) -D__${OS}__ $(CCOPTS) ${EXECOPTS}  test-fore.cc \
		${INCLUDES} ${EXECSRCS} $(LINKS) -I$(FORE_INCLUDES) \
		${FORELINKS}
		-o _fore_sscop_${OS}${LIBTYPE}
else
_fore_sscop_${OS}:	
	@echo You cannot make FORE code on `hostname` because \
	@echo the FORE libraries are not available.
endif


library:	${MODULELIB} ${buildable_libs}
	@true

#*** executables, and the library ${MODULELIB}
all:	library  ${EXECS}
	@echo qsaal_${OS} is ready;

execs:	${EXECS}
	@true
other:	${EXECS}
	@true

fore:	_fore_sscop_${OS}
	@true
fake:	_fake_sscop_${OS}
	@true

_fake_sscop_${OS}:	${EXECSRCS} ${MODULELIB} 
	$(C++) -D__${OS}__ $(CCOPTS) ${EXECOPTS} test-fake.cc \
	    ${INCLUDES} ${EXECSRCS} $(LINKS) -L.. -lfake_aal_${OS}${DEBUGLIBTYPE} \
	    -o _fake_sscop_${OS} 


# ---------- FORE VERSION -----------

qsaal_expander_fore_${OS}.${OTYPE}: qsaal_expander.cc
	$(C++) -D__FORE_AAL__ -D__${OS}__ $(CCOPTS) -I$(FORE_INCLUDES) \
	${INCLUDES} \
		qsaal_expander.cc -c -o qsaal_expander_fore_${OS}.${OTYPE}

fore_aal_${OS}.${OTYPE}:	
	$(C++) -D__FORE_AAL__ -D__${OS}__ $(CCOPTS) -I$(FORE_INCLUDES) \
	${INCLUDES} \
		-c fore_aal.cc -o fore_aal_${OS}.${OTYPE}

${FORE_AAL_LIB}: qsaal_expander_fore_${OS}.${OTYPE} fore_aal_${OS}.${OTYPE}
	${LD} ${LDOPTS} \
		qsaal_expander_fore_${OS}.${OTYPE} fore_aal_${OS}.${OTYPE} \
		-o ${FORE_AAL_LIB}

make-fore-aal: ${FORE_AAL_LIB}
	@echo done making fore_aal library ${FORE_AAL_LIB}

test-fore_${OS}.${OTYPE}:
	$(C++) -D__FORE_AAL__ -D__${OS}__ $(CCOPTS) -I$(FORE_INCLUDES) \
	${INCLUDES} \
	-c test-fore.cc -o test-fore_${OS}.${OTYPE}

# ---------- FAKE VERSION -----------

qsaal_expander_fake_${OS}.${OTYPE}: qsaal_expander.cc
	$(C++) -D__FAKE_AAL__ -D__${OS}__ $(CCOPTS)  \
	${INCLUDES} \
	qsaal_expander.cc -c -o qsaal_expander_fake_${OS}.${OTYPE}

fake_aal_${OS}.${OTYPE}: fake_aal.cc
	$(C++) -D__FAKE_AAL__ -D__${OS}__ $(CCOPTS)  \
	${INCLUDES} \
	-c fake_aal.cc -o fake_aal_${OS}.${OTYPE}

${FAKE_AAL_LIB}: qsaal_expander_fake_${OS}.${OTYPE} fake_aal_${OS}.${OTYPE}
	${LD} ${LDOPTS} \
	qsaal_expander_fake_${OS}.${OTYPE} fake_aal_${OS}.${OTYPE} \
	    -o ${FAKE_AAL_LIB} 


make-fake-aal: ${FAKE_AAL_LIB} 
	@echo done making fake_aal library ${FAKE_AAL_LIB} 

test-fake_${OS}.${OTYPE}:	
	$(C++) -D__FAKE_AAL__ -D__${OS}__ $(CCOPTS) -I$(FORE_INCLUDES) \
	  ${INCLUDES} \
	  -c test-fake.cc -o test-fake_${OS}.${OTYPE}


#
# fake_aal.cc       produces fake_aal_${OS}.[oO]
# fore_aal.cc       produces fore_aal_${OS}.[oO]
# qsaal_expander.cc produces qsall_expander_{fake,fore}_${OS}.[oO]
# make depend_local (at end of make depend if .needs_depend_local exists)
# produces rules for sean_io.o and xtio.o
#
# this rule fixes all that
define fake_depend_local_rule
	touch .depend_${OS};                                                 \
	$(CCC) -M -D__FAKE_AAL__ -D__${OS}__ $(CCOPTS)                       \
	  ${INCLUDES}                                                        \
	  qsaal_expander.cc                                                | \
	sed 's/^\([A-Za-z_]*\)\.o *:/\1_fake_${OS}\.o \1_fake_${OS}\.O :/'   \
	   >>.depend_${OS} ;                                                 \
	$(CCC) -M -D__FAKE_AAL__ -D__${OS}__ $(CCOPTS)                       \
	  ${INCLUDES}                                                        \
	   fake_aal.cc                                                     | \
	sed 's/^\([A-Za-z_]*\)\.o *:/\1_${OS}\.o \1_${OS}\.O :/'             \
	   >>.depend_${OS} ;
endef

ifeq (${FORE},yes)
  define fore_depend_local_rule
	$(CCC) -M -D__FORE_AAL__ -D__${OS}__ $(CCOPTS) -I$(FORE_INCLUDES)   \
	  ${INCLUDES}   qsaal_expander.cc                                 | \
	sed 's/^\([A-Za-z_]*\)\.o *:/\1_fore_${OS}\.o \1_fore_${OS}\.O :/'  \
	>> .depend_${OS} ;                                                  \
	$(CCC) -M -D__FORE_AAL__ -D__${OS}__ $(CCOPTS) -I$(FORE_INCLUDES)   \
	  ${INCLUDES}                                                       \
	  fore_aal.cc                                                     | \
	sed 's/^\([A-Za-z_]*\)\.o *:/\1_${OS}\.o \1_${OS}\.O :/'            \
	 >>.depend_${OS} ;
  endef
else
  define fore_depend_local_rule
	echo cannot make Fore dependencies on this host - files are missing
  endef
endif



depend_local:
	@${fake_depend_local_rule}
	@${fore_depend_local_rule}
	@echo ${THIS_DIR} local dependencies updated

clean::
	rm -f *_${OS}.[oO]  ${FORE_AAL_LIB} ${FAKE_AAL_LIB}

distclean::
	rm -f *_${OS}.[oO]  ${FORE_AAL_LIB} ${FAKE_AAL_LIB}
