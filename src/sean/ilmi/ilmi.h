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

//-*-C++-*-

#ifndef ILMID_H
#define ILMID_H
#ifndef LINT
static char const _ilmi_h_rcsid_[] =
"$Id: ilmi.h,v 1.26 1999/04/08 20:07:57 marsh Exp marsh $";
#endif
#include <FW/kernel/Kernel.h>
#include <FW/actors/State.h>

#include <snmp_pp.h>
#include <snmpmsg.h>

const int S1 = 1;
const int S2 = 2;
const int S3 = 3;
const int S4 = 4;
const int S5 = 5;
const int S6 = 6;
const int S7 = 7;
const int S8 = 8;
const int S9 = 9;

class ilmiTimer;

class ilmi : public State {
public:
  ilmi(int & status, unsigned char *ESIaddr, IpAddress ip, OctetStr community, int k = 4, int s = 1, int t = 5);
  ~ilmi();
  virtual State * Handle(Visitor *v);
  virtual void    Interrupt(SimEvent *e);
  void GetStateName(char *name);
  int GetState();
  void ExpTimer();
  void SendColdStart();
  void SendResponse(Pdu & pdu);
  void SetAddr();
  void poll_addr();
  void Print(Pdu & pdu);
  int decode(unsigned char *data, int len);
  unsigned long GetSysTime();
  int GetMySystemIdentifier();
  int snmpGet(Vb & vb);
  int snmpGetOid(char *name);
  int GetSysObjID(char *buffer);
  void ResetSysUpTime();
  void ResetAP();
  void GetAP();
  void GetConf();
  void PrintNSAP();
  void GetAddrStatus();
  void SetPrefix(Pdu & pdu);
private:
  void ChangeState(int s);
  unsigned char _esi[7];
  IpAddress _ip;
  int _state;
  int _req_id;
  int _Retries;
  Oid _AddrStatusOid;
  Oid _PrefixStatusOid;
  ilmiTimer *_T;
  ilmiTimer *_S;
  int _K;
  char *_prefix;
  bool _GotPrefix;
  Snmp _snmp;
  CTarget _ctarget;
  OctetStr _community;
  snmp_version _version;
  // AP
  Vb _sysUpTime;
  Vb _atmfMySystemIdentifier;
  Vb _atmfPortIndex;
  int _APReqid;
  int _ConfReqid;
  int _AddrReqid;
  int _SetAddrReqid;
  int _VerAddrReqid;

  const VisitorType * _start_visitor_type;
  const VisitorType * _link_visitor_type;
  const VisitorType * _ilmi_visitor_type;
};

#endif
