// -*- C++ -*-
// +++++++++++++++
//    S E A N        ---  Signalling Entity for ATM Networks  ---
// +++++++++++++++
// Version: 1.0.1
// 
// 			  Copyright (c) 1998
// 		 Naval Research Laboratory (NRL/CCS)
// 			       and the
// 	  Defense Advanced Research Projects Agency (DARPA)
// 
// 			 All Rights Reserved.
// 
// Permission to use, copy, and modify this software and its
// documentation is hereby granted, provided that both the copyright notice and
// this permission notice appear in all copies of the software, derivative
// works or modified versions, and any portions thereof, and that both notices
// appear in supporting documentation.
// 
// NRL AND DARPA ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
// DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM
// THE USE OF THIS SOFTWARE.
// 
// NRL and DARPA request users of this software to return modifications,
// improvements or extensions that they make to:
// 
//                 sean-dev@cmf.nrl.navy.mil
//                         -or-
//                Naval Research Laboratory, Code 5590
//                Center for Computation Science
//                Washington, D.C.  20375
// 
// and grant NRL and DARPA the rights to redistribute these changes in
// future upgrades.
//

// -*- C++ -*-
#ifndef LINT
static char const _ilmi_cc_rcsid_[] =
"$Id: ilmi.cc,v 1.26 1999/04/08 20:10:44 marsh Exp marsh $";
#endif
#include <common/cprototypes.h>
#include <iostream.h>
#include <FW/basics/diag.h>
#include <FW/basics/Visitor.h>
#include <FW/kernel/Kernel.h>
#include <FW/actors/State.h>

#include "ilmi.h"
#include "ilmiVisitors.h"
#include "ilmiTimers.h"
#include "snmpmsg.h"

extern "C" {
#include <stdio.h>                  // standard io
#include <stdlib.h>                 // need for malloc
#include <unistd.h>                 // unix
#include <sys/socket.h>         // bsd socket stuff
#include <netinet/in.h>         // network types
#include <arpa/inet.h>          // arpa types
#include <sys/types.h>          // system types
#include <sys/time.h>           // time stuff
#include <errno.h>          // ux errs
#include <strings.h>
#include <string.h>
#include <sys/fcntl.h>
#include <fore_xti/xti_user_types.h>
#include <fore_xti/xti.h>
#include <fore_xti/xti_atm.h>
#include <fore_xti/ans.h>
}

#define MAX_SNMP_PACKET 2048  // maximum snmp packet len

#define PDU_TRAP         0xA4  

#define SVERS_LEN       1               // SNMP version is 1 byte long
#define SVERSION        0               // current SNMP version 
#define SCOMM_STR       "ILMI"          // ILMI SNMP community string

#define ASN1_SEQ                 0x30    // sequence object 
#define ASN1_INT                 0x02    // integer         
#define ASN1_OCTSTR              0x04    // octet string    
#define ASN1_NULL                0x05    // null            
#define ASN1_OBJID               0x06    // object identifier
#define ASN1_IPADDR              0x40    // ip address       
#define ASN1_COUNTER             0x41    // counter          
#define ASN1_GAUGE               0x42    // gauge            
#define ASN1_TIMETICKS           0x43    // time ticks       

#define CHAR_BITS        8               // number of bits per char      
#define CHAR_HIBIT       0x80            // octet with the high bit set 

static  int request_id = 0;

extern char *esi;

struct mibdata {
  char *oid;
  char *name;
  int value;
};

#define MAX_SIZE 13
#define TOTAL_SIZE 95

static mibdata mib[] = {
  {"1.3.6.1.2.1.1.3.0",            "sysUpTime: TimeTicks", -1}, 
  {"1.3.6.1.2.1.1.2.0",            "sysObjectID: OBJECT IDENTIFIER", -1}, 
  {"1.3.6.1.4.1.353.2.1.2.0",      "atmfMyIpNmAddress: IpAddress", -1},
  {"1.3.6.1.4.1.353.2.1.4.0",      "atmfMySystemIdentifier: OCTET STRING (SIZE(6))", -1},

  {"1.3.6.1.4.1.353.2.1.1.1.1.0",  "atmfPortIndex: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.1.1.1.8.0",  "atmfPortMyIfIdentifier: INTEGER (0..2147483647)", 0},

  {"1.3.6.1.4.1.353.2.2.1.1.3.0",  "atmfAtmLayerMaxVCCs: INTEGER (0..16777215)", 4096},
  {"1.3.6.1.4.1.353.2.2.1.1.6.0",  "atmfAtmLayerMaxVpiBits: INTEGER (0..8)", 8},
  {"1.3.6.1.4.1.353.2.2.1.1.7.0",  "atmfAtmLayerMaxVciBits: INTEGER (0..16)", 12},
  {"1.3.6.1.4.1.353.2.2.1.1.8.0",  "atmfAtmLayerUniType: Enum", 2},
  {"1.3.6.1.4.1.353.2.2.1.1.9.0",  "atmfAtmLayerUniVersion: Enum", 4},
  {"1.3.6.1.4.1.353.2.2.1.1.10.0",  "atmfAtmLayerDeviceType: Enum", 1},
  {"1.3.6.1.4.1.353.2.2.1.1.11.0",  "atmfAtmLayerIlmiVersion: Enum", 2}, // ilmi 4.0

  {"1.3.6.1.4.1.353.2.2.1.1.12.0",  "atmfAtmLayerNniSigVersion: Enum", 1}, // unsupported
  {"1.3.6.1.4.1.353.2.2.1.1.13.0",  "atmfAtmLayerMaxSvpcVpi: Enum", 0}, // max perm VPI 
  {"1.3.6.1.4.1.353.2.2.1.1.14.0",  "atmfAtmLayerMaxSvccVpi: Enum", 0}, // max switched VPI 
  {"1.3.6.1.4.1.353.2.2.1.1.15.0",  "atmfAtmLayerMinSvpcVci: Enum", 32}, // min Vcc 
  {"1.3.6.1.4.1.353.2.7.1.1.3.0",  "atmfNetPrefixStatus: Enum", 2},

  // end of known mib data -- change MAX_SIZE if new entry is made above

  {"1.3.6.1.4.1.353.2.1.1.1.1.0",  "atmfPortIndex: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.1.1.1.2.0",  "atmfPortAddress: AtmAddress", 0},
  {"1.3.6.1.4.1.353.2.1.1.1.3.0",  "atmfPortTrasAddnsmissionType: OBJECT IDENTIFIER", 0},
  {"1.3.6.1.4.1.353.2.1.1.1.4.0",  "atmfPortMediaType: OBJECT IDENTIFIER", 0},
  {"1.3.6.1.4.1.353.2.1.1.1.5.0",  "atmfPortOperStatus: Enum", 0},
  {"1.3.6.1.4.1.353.2.1.1.1.8.0",  "atmfPortMyIfIdentifier: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.1.2.0",  "atmfMyIpNmAddress: IpAddress", 0},
  {"1.3.6.1.4.1.353.2.1.4.0",  "atmfMySystemIdentifier: OCTET STRING (SIZE(6))", 0},
  {"1.3.6.1.4.1.353.2.2.0",  "atmfAtmLayerGroup: reg point", 0},
  {"1.3.6.1.4.1.353.2.2.1.0",  "atmfAtmLayerTable: SEQUENCE OF AtmfAtmLayerEntry", 0},
  {"1.3.6.1.4.1.353.2.2.1.1.0",  "atmfAtmLayerEntry: SEQUENCE AtmfAtmLayerEntry", 0},
  {"1.3.6.1.4.1.353.2.2.1.1.1.0",  "atmfAtmLayerIndex: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.2.1.1.2.0",  "atmfAtmLayerMaxVPCs: INTEGER (0..255)", 0},
  {"1.3.6.1.4.1.353.2.2.1.1.3.0",  "atmfAtmLayerMaxVCCs: INTEGER (0..16777215)", 0},
  {"1.3.6.1.4.1.353.2.2.1.1.4.0",  "atmfAtmLayerConfiguredVPCs: INTEGER (0..255)", 0},
  {"1.3.6.1.4.1.353.2.2.1.1.5.0",  "atmfAtmLayerConfiguredVCCs: INTEGER (0..16777215)", 0},
  {"1.3.6.1.4.1.353.2.2.1.1.6.0",  "atmfAtmLayerMaxVpiBits: INTEGER (0..8)", 0},
  {"1.3.6.1.4.1.353.2.2.1.1.7.0",  "atmfAtmLayerMaxVciBits: INTEGER (0..16)", 0},
  {"1.3.6.1.4.1.353.2.2.1.1.8.0",  "atmfAtmLayerUniType: Enum", 0},
  {"1.3.6.1.4.1.353.2.2.1.1.9.0",  "atmfAtmLayerUniVersion: Enum", 0},
  {"1.3.6.1.4.1.353.2.2.1.1.10.0",  "atmfAtmLayerDeviceType: Enum", 0},
  {"1.3.6.1.4.1.353.2.2.1.1.11.0",  "atmfAtmLayerIlmiVersion: Enum", 0},
  {"1.3.6.1.4.1.353.2.2.1.1.12.0",  "atmfAtmLayerNniSigVersion: Enum", 0},
  {"1.3.6.1.4.1.353.2.3.0",  "atmfAtmStatsGroup: reg point", 0},
  {"1.3.6.1.4.1.353.2.3.1.0",  "atmfAtmStatsTable: SEQUENCE OF AtmfAtmStatsEntry", 0},
  {"1.3.6.1.4.1.353.2.3.1.1.0",  "atmfAtmStatsEntry: SEQUENCE AtmfAtmStatsEntry", 0},
  {"1.3.6.1.4.1.353.2.3.1.1.1.0",  "atmfAtmStatsIndex: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.3.1.1.2.0",  "atmfAtmStatsReceivedCells: Counter", 0},
  {"1.3.6.1.4.1.353.2.3.1.1.3.0",  "atmfAtmStatsDroppedReceivedCells: Counter", 0},
  {"1.3.6.1.4.1.353.2.3.1.1.4.0",  "atmfAtmStatsTransmittedCells: Counter", 0},
  {"1.3.6.1.4.1.353.2.4.0",  "atmfVpcGroup: reg point", 0},
  {"1.3.6.1.4.1.353.2.4.1.0",  "atmfVpcTable: SEQUENCE OF AtmfVpcEntry", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.0",  "atmfVpcEntry: SEQUENCE AtmfVpcEntry", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.1.0",  "atmfVpcPortIndex: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.2.0",  "atmfVpcVpi: INTEGER (0..255)", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.3.0",  "atmfVpcOperStatus: Enum", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.4.0",  "atmfVpcTransmitTrafficDescriptorType: OBJECT IDENTIFIER", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.5.0",  "atmfVpcTransmitTrafficDescriptorParam1: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.6.0",  "atmfVpcTransmitTrafficDescriptorParam2: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.7.0",  "atmfVpcTransmitTrafficDescriptorParam3: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.8.0",  "atmfVpcTransmitTrafficDescriptorParam4: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.9.0",  "atmfVpcTransmitTrafficDescriptorParam5: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.10.0",  "atmfVpcReceiveTrafficDescriptorType: OBJECT IDENTIFIER", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.11.0",  "atmfVpcReceiveTrafficDescriptorParam1: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.12.0",  "atmfVpcReceiveTrafficDescriptorParam2: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.13.0",  "atmfVpcReceiveTrafficDescriptorParam3: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.14.0",  "atmfVpcReceiveTrafficDescriptorParam4: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.15.0",  "atmfVpcReceiveTrafficDescriptorParam5: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.16.0",  "atmfVpcQoSCategory: Enum", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.17.0",  "atmfVpcTransmitQoSClass: INTEGER (0..255)", 0},
  {"1.3.6.1.4.1.353.2.4.1.1.18.0",  "atmfVpcReceiveQoSClass: INTEGER (0..255)", 0},
  {"1.3.6.1.4.1.353.2.5.0",  "atmfVccGroup: reg point", 0},
  {"1.3.6.1.4.1.353.2.5.1.0",  "atmfVccTable: SEQUENCE OF AtmfVccEntry", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.0",  "atmfVccEntry: SEQUENCE AtmfVccEntry", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.1.0",  "atmfVccPortIndex: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.2.0",  "atmfVccVpi: INTEGER (0..255)", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.3.0",  "atmfVccVci: INTEGER (0..65535)", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.4.0",  "atmfVccOperStatus: Enum", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.5.0",  "atmfVccTransmitTrafficDescriptorType: OBJECT IDENTIFIER", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.6.0",  "atmfVccTransmitTrafficDescriptorParam1: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.7.0",  "atmfVccTransmitTrafficDescriptorParam2: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.8.0",  "atmfVccTransmitTrafficDescriptorParam3: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.9.0",  "atmfVccTransmitTrafficDescriptorParam4: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.10.0",  "atmfVccTransmitTrafficDescriptorParam5: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.11.0",  "atmfVccReceiveTrafficDescriptorType: OBJECT IDENTIFIER", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.12.0",  "atmfVccReceiveTrafficDescriptorParam1: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.13.0",  "atmfVccReceiveTrafficDescriptorParam2: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.14.0",  "atmfVccReceiveTrafficDescriptorParam3: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.15.0",  "atmfVccReceiveTrafficDescriptorParam4: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.16.0",  "atmfVccReceiveTrafficDescriptorParam5: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.17.0",  "atmfVccQoSCategory: Enum", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.18.0",  "atmfVccTransmitQoSClass: INTEGER (0..255)", 0},
  {"1.3.6.1.4.1.353.2.5.1.1.19.0",  "atmfVccReceiveQoSClass: INTEGER (0..255)", 0},

  {"1.3.6.1.4.1.353.2.6.0",  "atmfAddressGroup: reg point", 0},
  {"1.3.6.1.4.1.353.2.6.1.0",  "atmfAddressTable: SEQUENCE OF AtmfAddressEntry", 0},
  {"1.3.6.1.4.1.353.2.6.1.1.0",  "atmfAddressEntry: SEQUENCE AtmfAddressEntry", 0},
  {"1.3.6.1.4.1.353.2.6.1.1.1.0",  "atmfAddressPort: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.6.1.1.2.0",  "atmfAddressAtmAddress: AtmAddress2", 0},
  {"1.3.6.1.4.1.353.2.6.1.1.3.0",  "atmfAddressStatus: Enum", 0},

  {"1.3.6.1.4.1.353.2.7.0",  "atmfNetPrefixGroup: reg point", 0},
  {"1.3.6.1.4.1.353.2.7.1.0",  "atmfNetPrefixTable: SEQUENCE OF AtmfNetPrefixEntry", 0},
  {"1.3.6.1.4.1.353.2.7.1.1.0",  "atmfNetPrefixEntry: SEQUENCE AtmfNetPrefixEntry", 0},
  {"1.3.6.1.4.1.353.2.7.1.1.1.0",  "atmfNetPrefixPort: INTEGER (0..2147483647)", 0},
  {"1.3.6.1.4.1.353.2.7.1.1.2.0",  "atmfNetPrefixPrefix: NetPrefix", 0},
  {"1.3.6.1.4.1.353.2.7.1.1.3.0",  "atmfNetPrefixStatus: Enum", 0}
};



ilmi::ilmi(int & status, unsigned char *ESIaddr, IpAddress ip, OctetStr community, int k, int s, int t) : State(true),
  _snmp(status),_ctarget(ip),_community(community),
  _AddrStatusOid("1.3.6.1.4.1.353.2.6.1.1.3.0"),
  _PrefixStatusOid("1.3.6.1.4.1.353.2.7.1.1.3.0")
{
  _ip = ip;
  for(int i = 0; i < 7; i++)
    _esi[i] = ESIaddr[i];
  _GotPrefix = false;
  _prefix = NULL;


  //  double time = theKernel().CurrentTime();
  //  _req_id = ((int)time) & 0xFFFFFF;

  _req_id = 0;

  _APReqid = -1;
  _ConfReqid = -1;
  _AddrReqid = -1;
  _SetAddrReqid = -1;
  _VerAddrReqid = -1;

  _Retries = 0;
  _state = S1;
  // T = 5 secs S = 1 sec and K = 4
  _K = k;
  _S = new ilmiTimer(this,s);
  _T = new ilmiTimer(this,t);
  _version = version1;
  _ctarget.set_version(_version);    
  _ctarget.set_readcommunity(_community);
  Oid o1("1.3.6.1.2.1.1.3.0"); // sysUpTime
  Oid o2("1.3.6.1.4.1.353.2.1.4.0"); // atmfMySystemIdentifier - mac addr
  Oid o3("1.3.6.1.4.1.353.2.1.1.1.8.0"); //  atmfPortMyIfIdentifier: INTEGER (0..2147483647)
  _sysUpTime.set_oid(o1);
  _atmfMySystemIdentifier.set_oid(o2);
  _atmfPortIndex.set_oid(o3);

  _start_visitor_type = QueryRegistry(START_VISITOR_NAME);
  _link_visitor_type = QueryRegistry(LINKUP_VISITOR_NAME);
  _ilmi_visitor_type = QueryRegistry(ILMI_VISITOR_NAME);

}

ilmi::~ilmi()
{
}


void  ilmi::ResetSysUpTime()
{
  _sysUpTime.set_value(GetSysTime());
}

void  ilmi::ResetAP()
{
  _sysUpTime.set_value(GetSysTime());
  GetMySystemIdentifier();
  _atmfPortIndex.set_value(0);
}

void ilmi::Print(Pdu & pdu)
{
  char buffer[100];
  switch(pdu.get_type())
    {
    case sNMP_PDU_GET:
      strcpy(buffer,"sNMP_PDU_GET");
      break;
    case sNMP_PDU_GETNEXT:
      strcpy(buffer,"sNMP_PDU_GETNEXT");
      break;
    case sNMP_PDU_RESPONSE:
      strcpy(buffer,"sNMP_PDU_RESPONSE");
      break;
    case sNMP_PDU_SET:
      strcpy(buffer,"sNMP_PDU_SET");
      break;
    case sNMP_PDU_V1TRAP:
      strcpy(buffer,"sNMP_PDU_V1TRAP");
      break;
    case sNMP_PDU_GETBULK:
      strcpy(buffer,"sNMP_PDU_GETBULK");
      break;
    case sNMP_PDU_INFORM:
      strcpy(buffer,"sNMP_PDU_INFORM");
      break;
    case sNMP_PDU_TRAP:
      strcpy(buffer,"sNMP_PDU_TRAP");
      break;
    default:
      strcpy(buffer,"UNKNOWN");
      break;
    }
  DIAG("ilmi", DIAG_DEBUG, cout << OwnerName() <<  "Print: this is a " << buffer << endl;);
  DIAG("ilmi", DIAG_DEBUG, cout << "request_id = " << pdu.get_request_id() << endl;);
  int k;
  int vb_cnt = pdu.get_vb_count();
  DIAG("ilmi", DIAG_DEBUG, cout << "vb_cnt = " << vb_cnt << endl;);
  for(k = 0; k < vb_cnt; k++)
    {
      Vb ivb;
      pdu.get_vb(ivb,k);
      DIAG("ilmi", DIAG_DEBUG, cout << "Oid = " << ivb.get_printable_oid() << endl;);
      DIAG("ilmi", DIAG_DEBUG, cout << "Value = " << ivb.get_printable_value() << endl;);
    }
}

int ilmi::GetState() {return _state;}

void ilmi::GetStateName(char *name)
{
  switch(_state)
    {
    case S1:
      strcpy(name, "Stopped");
      break;
    case S2:
      strcpy(name, "Link Failing");
      break;
    case S3:
      strcpy(name, "Establishing");
    break;
    case S4:
      strcpy(name, "Configuring");
    break;
    case S5:
      strcpy(name, "Retrieving NetPrefix");
      break;
    case S6:
      strcpy(name, "Registering NetPrefix");
      break;
    case S7:
      strcpy(name, "Retrieving Address");
      break;
    case S8:
      strcpy(name, "Registering Address");
      break;
    case S9:
      strcpy(name, "Verifying");
      break;
    }
}

void ilmi::ChangeState(int s) {_state = s;}


void ilmi::ExpTimer()
{
  DIAG("ilmi", DIAG_DEBUG, cout << "ilmiTimer has expired in state = " << _state << endl;);
  switch(_state)
    {
    case S1:
      break;
    case S2:
      break;

    case S3:
      if(_Retries <= _K)
	{
	  _Retries++;
	  GetAP();
	}
    break;

    case S4:
      if(_Retries <= _K)
	{
	  _Retries++;
	  GetConf();
	  _T->SetTimer();
	}
      else
	{
	  SendColdStart();
	  GetAP();
	  _T->SetTimer();
	  ChangeState(S3);
	}
    break;

    case S5:
      break;
    case S6:
      break;
    case S7:
      if(_Retries <= _K)
	{
	  _Retries++;
	  GetAddrStatus();
	  _T->SetTimer();
	}
      else
	{
	  SendColdStart();
	  GetAP();
	  _T->SetTimer();
	  ChangeState(S3);
	}
      break;

    case S8:
      if(_Retries <= _K)
	{
	  _Retries++;
	  SetAddr();
	  _T->SetTimer();
	}
      else
	{
	  SendColdStart();
	  GetAP();
	  _T->SetTimer();
	  ChangeState(S3);
	}
      break;

    case S9:
      if(_Retries <= _K)
	{
	  _Retries++;
	  GetAP();
	  _T->SetTimer();
	}
      else
	{
	  SendColdStart();
	  GetAP();
	  _T->SetTimer();
	  ChangeState(S3);
	}
      break;
    }
}


void ilmi::GetAP()
{
  Oid o1("1.3.6.1.2.1.1.3.0"); // sysUpTime
  Oid o2("1.3.6.1.4.1.353.2.1.4.0"); // atmfMySystemIdentifier - mac addr
  Oid o3("1.3.6.1.4.1.353.2.1.1.1.8.0"); //  atmfPortIndex: INTEGER (0..2147483647)
  Vb vb1;
  vb1.set_oid(o1);
  Vb vb2;
  vb2.set_oid(o2);
  Vb vb3;
  vb3.set_oid(o3);
  Pdu pdu;
  pdu.set_type(sNMP_PDU_GET);
  pdu += vb1;
  pdu += vb2;
  pdu += vb3;
  _req_id++;
  char name[40];
  GetStateName(name);
  //  cout << "####GetAP: " << _req_id << " in State " << name << endl;
  _APReqid = _req_id;
  set_request_id(&pdu, _req_id);
  ilmiVisitor *iv = new GetVisitor(pdu,_req_id);
  PassVisitorToB(iv);
}

void ilmi::GetConf()
{
  Pdu pdu;
  pdu.set_type(sNMP_PDU_GET);
  // build the OIDs
  Oid MaxVPC("1.3.6.1.4.1.353.2.2.1.1.2.0"); //  atmfAtmLayerMaxVPCs: INTEGER (0..255) 
  Oid MaxVCC("1.3.6.1.4.1.353.2.2.1.1.3.0"); //  atmfAtmLayerMaxVCCs: INTEGER (0..16777215)
  Oid ConfVPC("1.3.6.1.4.1.353.2.2.1.1.4.0"); //  atmfAtmLayerConfiguredVPCs: INTEGER (0..255)
  Oid ConfVCC("1.3.6.1.4.1.353.2.2.1.1.5.0"); //  atmfAtmLayerConfiguredVCCs: INTEGER (0..16777215)
  Oid MaxVpiBits("1.3.6.1.4.1.353.2.2.1.1.6.0"); //  atmfAtmLayerMaxVpiBits: INTEGER (0..8)
  Oid MaxVciBits("1.3.6.1.4.1.353.2.2.1.1.7.0"); //  atmfAtmLayerMaxVciBits: INTEGER (0..16)
  Oid UniType("1.3.6.1.4.1.353.2.2.1.1.8.0"); //  atmfAtmLayerUniType: Enum
  Oid UniVer("1.3.6.1.4.1.353.2.2.1.1.9.0"); //  atmfAtmLayerUniVersion: Enum
  Oid DeviceType("1.3.6.1.4.1.353.2.2.1.1.10.0"); //  atmfAtmLayerDeviceType: Enum
  Oid ilmiVer("1.3.6.1.4.1.353.2.2.1.1.11.0"); //  atmfAtmLayerIlmiVersion: Enum
  //  Oid NniSigVer("1.3.6.1.4.1.353.2.2.1.1.12.0"); //  atmfAtmLayerNniSigVersion: Enum
  // Build the VBs
  Vb vb[11];
  vb[0].set_oid(MaxVPC);
  vb[1].set_oid(MaxVCC);
  vb[2].set_oid(ConfVPC);
  vb[3].set_oid(ConfVCC);
  vb[4].set_oid(MaxVpiBits);
  vb[5].set_oid(MaxVciBits);
  vb[6].set_oid(UniType);
  vb[7].set_oid(UniVer);
  vb[8].set_oid(DeviceType);
  vb[9].set_oid(ilmiVer);
  //  vb[10].set_oid(NniSigVer);
  // load the VBs into the PDU
  for(int i = 0; i < 10; i++)
  pdu += vb[i];
  _req_id++;
  char name[40];
  GetStateName(name);
  //  cout << "####GetConf: " << _req_id << " in State " << name << endl;
  _ConfReqid = _req_id;
  set_request_id(&pdu, _req_id);
  ilmiVisitor *iv = new GetVisitor(pdu,_req_id);
  PassVisitorToB(iv);
}


State * ilmi::Handle(Visitor * v)
{
  char name[100];
  GetStateName(name);
  DIAG("ilmi", DIAG_DEBUG, cout << OwnerName() << " : in state " << name << endl;);
  VisitorType vt1 = v->GetType();
  //  const VisitorType * vt2 = QueryRegistry(START_VISITOR_NAME);
  //  const VisitorType * vt3 = QueryRegistry(LINKUP_VISITOR_NAME);
  //  const VisitorType * vt4 = QueryRegistry(ILMI_VISITOR_NAME);

  // StartVisitor is only expected in state S1
  if (_start_visitor_type && vt1.Is_A(_start_visitor_type))
    {
      assert(_state == S1);
      ResetSysUpTime();
      ResetAP();
      ChangeState(S2);
      v->Suicide();
      return this;
    }
  // LinkUpVisitor is only expected in state S2
  if (_link_visitor_type && vt1.Is_A(_link_visitor_type))
    {
      assert(_state == S2);
      SendColdStart();
      GetAP();
      _T->SetTimer();
      ChangeState(S3);
      v->Suicide();
      return this;
    }

  // process ilmiVisitors here
  ilmiVisitor *iv = (ilmiVisitor *)v;
  Pdu pdu;
  int value;
  Vb ivb;
  Oid o;
  pdu = iv->GetPDU();
  //  cout << "PDU with req_id = " << pdu.get_request_id() << endl;
  int  err = pdu.get_error_status();
  if(pdu.get_type() != sNMP_PDU_RESPONSE)
    {
      if(err != 0)
	{
	  cout << "received a BAD pdu with error = " << err << endl;
	  v->Suicide();
	  return this;
	}
      switch(pdu.get_type())
	{
	case sNMP_PDU_V1TRAP:
	  cout << "got a V1TRAP\n";
	  GetAP();
	  _T->SetTimer();
	  ChangeState(S3);
	  break;

	case sNMP_PDU_GET:
	case sNMP_PDU_GETNEXT:
	  SendResponse(pdu);
	  break;

	case sNMP_PDU_SET:
	  // it's got a be for the prefix
	  DIAG("ilmi", DIAG_DEBUG, cout << OwnerName() << " switch doing a SET " << endl;);
	  pdu.get_vb(ivb,0);
	  ivb.get_oid(o);
	  if(_PrefixStatusOid < o)
	    {
	      ivb.get_value(value);
	      if(value == 1)
		{
		  DIAG("ilmi", DIAG_DEBUG, cout << OwnerName() << "switch registered prefix " << endl;);
		  SetPrefix(pdu);
		}
	      else
		{
		  DIAG("ilmi", DIAG_DEBUG, cout << OwnerName() << "switch de-registered prefix " << endl;);
		  _GotPrefix = false;
		  // have to redo my address registration
		  ChangeState(S7);
		}
	      // make a response
	      pdu.set_type(sNMP_PDU_RESPONSE);
	      ivb.set_value(1);
	      pdu.set_vb(ivb,0);
	      ilmiVisitor *rv = new SetVisitor(pdu,pdu.get_request_id());
	      PassVisitorToB(rv);
	    }
	  break;

	default:
	  cout << "unknown req_id = " << pdu.get_request_id() << endl;
	  break;
	}
      v->Suicide();
      return this;
    }
  // if we got here we have a RESPONSE  
  int req_id = pdu.get_request_id();
  if((req_id != _APReqid) && (req_id != _ConfReqid) && (req_id != _AddrReqid) &&
     (req_id !=_SetAddrReqid) && (req_id != _VerAddrReqid))
    {
      cout << " Unexpected RESPONSE req_id = " << req_id << endl;
      cout << "_APReqid = " << _APReqid <<  " _ConfReqid = " << _ConfReqid << " _AddrReqid = " << _AddrReqid << " _SetAddrReqid = " << _SetAddrReqid << endl;
      iv->Suicide();
      return this;
    }
    
  // must be a RESPONSE 
  switch(_state)
    {
    case S1: // Stopped State
      break;


    case S2: // Link Failing
      break;

    case S3: // E7 or E8 depending on the response to GetAP()
      // answer to the AP request
      _T->StopTimer();
      _Retries = 0;
      _T->SetTimer();
      GetConf();
      ChangeState(S4);
      break;

    case S4:
      if(_prefix)
	{
	  // answer to the GetConf request should be E11
	  _T->StopTimer();
	  // start signalling
	  _Retries = 0;
	  _T->SetTimer();
	  GetAddrStatus(); // A14
	  ChangeState(S7);
	}
      break;

    case S5:
      break;

    case S6:
      break;

    case S7: // what we are looking for is E13
      pdu.get_vb(ivb,0);
      ivb.get_oid(o);
      ivb.get_value(value);
      // check for E13
      if((err == SNMP_ERROR_NO_SUCH_NAME) || ((err == 0) && (o != _AddrStatusOid)))
	{
	  _T->StopTimer();
	  SetAddr(); // A15
	  _Retries = 0;
	  _T->SetTimer();
	  ChangeState(S8);
	  iv->Suicide();
	  return this;
	}
      // need to add the other cases
      cout << "unexpected case !!!" << endl;
      break;

    case S8: // what we are looking for is E14
      pdu.get_vb(ivb,0);
      ivb.get_oid(o);
      ivb.get_value(value);
      // check for E14
      if(err == 0 && (req_id == _SetAddrReqid) || (req_id == _VerAddrReqid))
	{
	  _T->StopTimer();
	  _Retries = 0;
	  _T->SetTimer();
	  GetAP();
	  ChangeState(S9);
	  iv->Suicide();
	  return this;
	}
      // need to add the other cases
      cout << "unexpected case !!!" << endl;
      break;

    case S9:
      _T->StopTimer();
      _Retries = 0;
      _T->SetTimer();
      break;
    }
  iv->Suicide();
  return this;
}

void  ilmi::Interrupt(SimEvent *event)
{
  DIAG("ilmi", DIAG_DEBUG, cout << OwnerName() << " interrupt ! " << endl;);
}



int ilmi::snmpGetOid(char *name)
{
  Oid o(name);
  Vb vb;
  vb.set_oid(o);
  Pdu pdu;
  pdu += vb;
  int status;
  if((status = _snmp.get(pdu,_ctarget)) != SNMP_CLASS_SUCCESS)
    {
       DIAG("ilmi", DIAG_DEBUG,cout << _snmp.error_msg(status) << endl);
      return -1;
    }
  Vb ivb;
  pdu.get_vb(ivb,0);
  cout << "oid = " << ivb.get_printable_oid() << " oid_value = " << ivb.get_printable_value() << endl;
  return 0;
}

int ilmi::snmpGet(Vb &ivb)
{
  int status;
  Pdu pdu;
  Vb vb(ivb);
  pdu += vb;
  if((status = _snmp.get(pdu,_ctarget)) != SNMP_CLASS_SUCCESS)
    {
      cout << _snmp.error_msg(status) << endl;
      return 0;
    }
  pdu.get_vb(ivb,0);
  cout << "oid = " << ivb.get_printable_oid() << " oid_value = " << ivb.get_printable_value() << endl;
  return 0;
}

unsigned long ilmi::GetSysTime()
{
  int status;
  Pdu pdu;
  Vb vb;
  Oid o("1.3.6.1.2.1.1.3.0");
  vb.set_oid(o);
  pdu += vb;
  if((status = _snmp.get(pdu,_ctarget)) != SNMP_CLASS_SUCCESS)
    {
      cout << _snmp.error_msg(status) << endl;
      return 0;
    }
  Vb ovb;
  pdu.get_vb(ovb,0);
  unsigned long value;
  ovb.get_value(value);
  return value;
}

int ilmi::GetMySystemIdentifier()
{
  int status;
  Pdu pdu;
  Vb vb;
  Oid o("1.3.6.1.4.1.353.2.1.4.0"); // atmfMySystemIdentifier - mac addr
  vb.set_oid(o);
  pdu += vb;
  if((status = _snmp.get(pdu,_ctarget)) != SNMP_CLASS_SUCCESS)
    {
      cout << _snmp.error_msg(status) << endl;
      return 0;
    }
  Vb ovb;
  pdu.get_vb(ovb,0);
  MacAddress ma;
  _atmfMySystemIdentifier.set_value(ma);
  return 0;
}

int ilmi::GetSysObjID(char *buffer)
{
  char name[256];
  int status;
  Pdu pdu;
  Vb vb;
  Oid o("1.3.6.1.2.1.1.2.0");
  vb.set_oid(o);
  pdu += vb;
  if((status = _snmp.get(pdu,_ctarget)) != SNMP_CLASS_SUCCESS)
    {
      cout << _snmp.error_msg(status) << endl;
      return -1;
    }
  Vb ovb;
  pdu.get_vb(ovb,0);
  strcpy(buffer,ovb.get_printable_value());
  return 0;
}

void ilmi::SendColdStart()
{
  Pdu pdu;
  pdu.set_notify_id("1.3.6.1.6.3.1.1.5.1");
  ilmiVisitor *iv = new TrapVisitor(pdu,0);
  PassVisitorToB(iv);
}

void ilmi::PrintNSAP()
{
  int i;
  static bool reported_NSAP = false;
  if (_prefix == 0 || reported_NSAP ) return;

  reported_NSAP = true;  // nsap is reregistered every second or so.
  cout << "NSAP: ";
  char buffer[43];  // 13*2 + 1 + 6*2 + 1 + 2 + \0
  int j =0;
  int val; 
  for ( i = 0; i<13; i++) {
    val = 0;
    while (_prefix[j] && _prefix[j] != '.') {
      val = val *10 + _prefix[j] - '0';
      j++;
    }
    sprintf(buffer+i*2,"%02x",(u_char)val);
    j++; // step over the trailing .
  }
  buffer[26] = '.';
  for ( i = 0; i<6; i++) 
    sprintf(buffer+27+i*2,"%02x",_esi[i]);
  buffer[39] = '.';
  buffer[40] = '0';
  buffer[41] = '0';
  buffer[42] = '\0';
  cout << buffer << endl; 
}

void ilmi::GetAddrStatus()
{
  Pdu pdu;
  Vb vb;
  Oid oid("1.3.6.1.4.1.353.2.6.1.0"); // ab 03/19/99
  vb.set_oid(oid);
  pdu += vb;
  pdu.set_type(sNMP_PDU_GETNEXT);
  _req_id++;
  _AddrReqid = _req_id;
  char name[40];
  GetStateName(name);
  //  cout << "####GetAddrStatus: " << _req_id << " in State " << name << endl;
  set_request_id(&pdu, _req_id);
  ilmiVisitor *iv = new GetVisitor(pdu,_req_id);
  PassVisitorToB(iv);
}

void ilmi::SetPrefix(Pdu & pdu)
{
  Vb vb;
  pdu.get_vb(vb,0);
  Oid o;
  vb.get_oid(o);
  char buffer[200];
  strcpy(buffer,o.get_printable(15,13));
  if (_prefix) delete _prefix;
  _prefix = new char [strlen(buffer)+1];
  strcpy(_prefix,buffer);
  _GotPrefix = true;
}

// prepare the adddress set request : we need the prefix to finalize it
// it should look like this:
// 1.3.6.1.4.1.353.2.6.1.1.3.0.20.71.0.5.128.255.222.0.0.0.0.0.11.1.0.32.72.6.145.185.0.0
// set the addr Variable binding valid(1) : register
// ps: invalid (2) means deregister

void ilmi::SetAddr()
{
  int len;
  Pdu pdu; 
  Vb vb;
  PrintNSAP();
  // 1.3.6.1.4.1.353.2.6.1.1.3.0.20.71.0.5.128.255.222.0.0.0.0.0.11.1.0.32.72.6.145.185.0.0
  Oid a("1.3.6.1.4.1.353.2.6.1.1.3.0");
  a += 20;
  a += _prefix;
  for(int k = 0; k<6; k++)
    a += (int )_esi[k];
  a += (unsigned long )0;
  vb.set_oid(a);
  vb.set_value(1);
  pdu += vb;
  pdu.set_type(sNMP_PDU_SET);
  _req_id++;
  char name[40];
  GetStateName(name);
  //  cout << "####SetAddr: " << _req_id << " in State " << name << endl;
  _SetAddrReqid = _req_id;
  set_request_id(&pdu, _req_id);
  ilmiVisitor *iv = new SetVisitor(pdu,_req_id);
  PassVisitorToB(iv);
}


void ilmi::SendResponse(Pdu & pdu)
{
  Vb vb;
  int ivb;
  int vb_cnt = pdu.get_vb_count();
  bool reply = false;

  for (ivb=0; ivb<vb_cnt; ivb++) {
    pdu.get_vb(vb,ivb);
    Oid oid;
    vb.get_oid(oid);
    int i;
    pdu.set_type(sNMP_PDU_RESPONSE);
    bool found = false;
    for(i = 0; i < MAX_SIZE; i++)
      {
	Oid toid(mib[i].oid);
	if(toid == oid)
	  {
	    found = true;
	    break;
	  }
      }
    if(found == true)
      {
	reply = true;  // if we find even one we will reply.
	switch(i)
	  {
	  case 0: // sysup time
	    {
	      TimeTicks tt(GetSysTime());
	      vb.set_value(tt);
	      pdu.set_vb(vb,ivb);
	    }
	  break;

	  case 1:
	    {
	      char buffer[256];
	      GetSysObjID(buffer);
	      vb.set_value(buffer);
	      pdu.set_vb(vb,ivb);
	    }
	  break;

	  case 2:
	    vb.set_value(_ip);
	    pdu.set_vb(vb,ivb);
	    break;

	  case 3: // atmfMySystemIdentifier: OCTET STRING (SIZE(6))
	    {
	      OctetStr mysysid(_esi,6);
	      vb.set_value(mysysid);
	      pdu.set_vb(vb,ivb);
	    }
	  break;
	  /*
	  case 4: // netprefix status
	    if(_prefix == 0)
	      vb.set_value(2);
	    else
	      vb.set_value(1);
	    pdu.set_vb(vb,ivb);
	  break;
	  */

	  default:
	    vb.set_value(mib[i].value);
	    pdu.set_vb(vb,ivb);
	    break;
	  }
      }
    else
      { 
	set_error_status(&pdu,SNMP_ERROR_NO_SUCH_NAME);
	pdu.set_vb(vb,ivb);
	DIAG("ilmi", DIAG_DEBUG, cout << "Add to the MIB : " << oid.get_printable() << endl;);
      }
  }
  reply = true; // 
  if (reply == true) {
    ilmiVisitor *iv = new SetVisitor(pdu,pdu.get_request_id());
    PassVisitorToB(iv);
  }
}
