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

#ifndef ILMITERM_H
#define ILMITERM_H
#ifndef LINT
static char const _ilmiTerm_h_rcsid_[] =
"$Id: ilmiTerm.h,v 1.8 1999/04/08 19:05:04 marsh Exp marsh $";
#endif

#include <iostream.h>
#include <FW/basics/Conduit.h>
#include <FW/behaviors/Adapter.h>
#include <FW/behaviors/Protocol.h>
#include <FW/basics/Visitor.h>
#include <FW/kernel/Kernel.h>
#include <FW/basics/diag.h>
#include <FW/basics/FW.h>

#include <snmp_pp.h>

class ilmiTerm;

class DriverReader : public InputHandler {
public:
  DriverReader(ilmiTerm *owner, int fd);
  virtual ~DriverReader();
  void Callback(void);
private:
  ilmiTerm *_owner;
};

class StartTimer : public TimerHandler  {
public:
  StartTimer(Terminal * t, Visitor * v) : 
    TimerHandler(t,1.0), _t(t), _v(v) { }
  virtual ~StartTimer() { }
  void Callback(void) { _t->Inject(_v); }
private:
  Terminal * _t;
  Visitor  * _v;
};

class LinkUpTimer : public TimerHandler  {
public:
  LinkUpTimer(Terminal * t, Visitor * v) : 
    TimerHandler(t,1.0), _t(t), _v(v) { }
  virtual ~LinkUpTimer() { }
  void Callback(void) { _t->Inject(_v); }
private:
  Terminal * _t;
  Visitor  * _v;
};


class ilmiTerm : public Terminal {
public:
  ilmiTerm();
  ilmiTerm(int vpi, int vci);
  ~ilmiTerm();
  void Interrupt(SimEvent * e);
  void Absorb(Visitor * v);
  int receive_ilmi_response();
  int send_ilmi_request(unsigned char *, unsigned int);
  void DumpPkt(char *, char *, int);
  void Print(class Pdu &,int);
  void SendColdStart();
private:
  InputHandler *_reader;
  int _fd;
  OctetStr _community;
  snmp_version _version;
  StartTimer  * _go1;
  LinkUpTimer * _go2;
  const VisitorType * _trap_visitor_type;
};


#endif
