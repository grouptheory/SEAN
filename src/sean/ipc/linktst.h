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


#ifndef __LINKTST_H__
#define __LINKTST_H__


#ifndef LINT
static char const _linktst_h_rcsid_[] =
"$Id: linktst.h,v 1.1 1998/07/20 11:11:40 bilal Exp $";
#endif

#include <FW/kernel/SimEvent.h>
#include <codec/q93b_msg/Buffer.h>

//----------------------------------------------------------

class ClientTimer: public TimerHandler {
public:
  ClientTimer(double meanwait, tcp_local_id* lid, State* owner) : 
    TimerHandler(owner, meanwait), _meanwait(meanwait), _lid(lid) {
  }

  virtual ~ClientTimer(){
  }
  
  void Callback(void) {

    int sz=strlen("01234567890123456789001234567890123456789001234567890");
    Buffer* buf = new Buffer(sz);
    strcpy((char*) ( buf->data() ),
	   "01234567890123456789001234567890123456789001234567890");
    IPC_DataVisitor* data = new IPC_DataVisitor(_lid,buf);
    PassVisitorToB(data);

    cout << "** periodic timer fires in the client" << endl;
    ExpiresIn(_meanwait);
    Handler::Register(this);
  }

private:
  double _meanwait;
  tcp_local_id* _lid;
};



class Client : public State {
public:
  Client(char* machine, int port) {
    _port = port;
    strcpy(_machine, machine);

    SimEvent* boot = new SimEvent(this,this,1);
    ReturnToSender(boot);
  }

  State* Handle(Visitor* v) {
    VisitorType vt = v->GetType();
    const VisitorType * const data_type = QueryRegistry(IPC_DATA_VISITOR_NAME);
    const VisitorType * const signalling_type = QueryRegistry(IPC_SIGNALLING_VISITOR_NAME);

    if (vt.Is_A(data_type)) {
      IPC_DataVisitor* iv = (IPC_DataVisitor*)v;
 
      cout << "**Client**  received IPC Data Visitor:" << endl;
      iv->Print(cout);
    }
    else if (vt.Is_A(signalling_type)) {
      IPC_SignallingVisitor* sv = (IPC_SignallingVisitor*)v;

      cout << "**Client**  received IPC Signalling Visitor:" << endl;
      sv->Print(cout);
    }

    return this;
  }

  void Interrupt(class SimEvent *e) {
    if (e->GetCode() == 1) {
      tcp_endpoint* dest = new tcp_endpoint(_port, _machine);
      SimEvent* ack = new SimEvent(this,this,2);
      
      IPC_SignallingVisitor* v = new IPC_SignallingVisitor
	(IPC_SignallingVisitor::establish_by_local, _local_id, dest, ack);
    
      PassVisitorToB(v);
    }
    else if (e->GetCode() == 2) {
      cout << "**Client**  reports being notified via SimEvent" << endl;
      tcp_local_id* tlid = (tcp_local_id*)_local_id;
      cout << "**Client**  that the connection has been established on ";
      tlid->Print(cout);
      cout << "." << endl;

      _timer = new ClientTimer(10.0, tlid, this);
      Register(_timer);
    }
  }

protected:
  virtual ~Client() {
    delete _timer;
  }

private:
  ClientTimer       * _timer;
  abstract_local_id * _local_id;

  int  _port;
  char _machine[255];
};

//----------------------------------------------------------

class Server : public State {
public:
  Server(void) {
  }

  State* Handle(Visitor* v) {
    VisitorType vt = v->GetType();
    const VisitorType * const data_type = QueryRegistry(IPC_DATA_VISITOR_NAME);
    const VisitorType * const signalling_type = QueryRegistry(IPC_SIGNALLING_VISITOR_NAME);

    if (vt.Is_A(data_type)) {
      IPC_DataVisitor* iv = (IPC_DataVisitor*)v;
 
      cout << "**Server**  received IPC Data Visitor:" << endl;
      iv->Print(cout);
    }
    else if (vt.Is_A(signalling_type)) {
      IPC_SignallingVisitor* sv = (IPC_SignallingVisitor*)v;

      cout << "**Server**  received IPC Signalling Visitor:" << endl;
      sv->Print(cout);
    }

    return this;
  }

  void Interrupt(class SimEvent *e) {
  }

protected:
  virtual ~Server() {
  }
};

#endif
