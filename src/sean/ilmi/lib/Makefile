CXX = g++ -g
#CXX = g++ -g -DDEBUG
RM = rm -f
OTYPE=o

FORE_DIR = /opt/FOREatm
SNMP_DIR = ..

SNMP_DEF = -D_XPG4_EXTENDED -Dsun -D__unix

FORE_INC = -I$(FORE_DIR)/include
SNMP_INC = -I$(SNMP_DIR)/include -I$(SNMP_DIR)/lib

FORE_LIB = -R$(FORE_DIR)/lib -L$(FORE_DIR)/lib -lans -lnsl
SNMP_LIB = -R$(SNMP_DIR)/lib -L$(SNMP_DIR)/lib -lsnmp -lsocket -lintl -lposix4

OPT = -O

SRC = address.cpp counter.cpp ctr64.cpp gauge.cpp integer.cpp octet.cpp \
	oid.cpp pdu.cpp target.cpp timetick.cpp vb.cpp asn1.cpp snmpmsg.cpp \
	uxsnmp.cpp eventlist.cpp msgqueue.cpp userdefined.cpp usertimeout.cpp \
	msec.cpp notifyqueue.cpp
OBJ = ${addprefix _o.${OS}/,${SRC:%.cpp=%.${OTYPE}}}

_o.${OS}/%.${OTYPE}:  %.cpp
	$(CXX) $(SNMP_DEF) $(SNMP_INC) $(FORE_INC) $(OPT) \
	-c $< -o _o.${OS}/$*.${OTYPE}

ALL:  _o.${OS} libsnmp_${OS}.so CLEAN

_o.${OS}:
	@if [ "x${OS}" != x ]; then \
	   if [ ! -d _o.${OS} ]; then mkdir _o.${OS}; fi; \
	 else \
	   echo "OS not set"; exit 99; \
	 fi


libsnmp_${OS}.so:  $(OBJ)
	$(CXX) $(OPT) $(OBJ) $(FORE_LIB) -G -o libsnmp_${OS}.so

test_app:  test_app.o
	$(CXX) $(OPT) test_app.o $(SNMP_LIB) -o test_app

test_app_atm:  test_app_atm.o
	$(CXX) $(OPT) test_app_atm.o $(SNMP_LIB) -o test_app_atm

CLEAN:
	$(RM) *~ *.h~ *.cpp~ core

clean:  CLEAN
	$(RM) *.o *_${OS}.so test_app test_app_atm _o.${OS}/*.o

distclean:
	$(RM) *.o *.so test_app test_app_atm
	rm -rf _o.*

