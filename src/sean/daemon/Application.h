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
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#ifndef LINT
static char const _Application_h_rcsid_[] =
"$Id: Application.h,v 1.3 1998/07/21 15:30:16 bilal Exp $";
#endif

#include <sean/cc/sean_Visitor.h>
#include <FW/basics/diag.h>

#include <sean/ipc/IPC_Visitors.h>
#include <codec/q93b_msg/Buffer.h>
#include <codec/uni_ie/ie_base.h>
#include <codec/uni_ie/cause.h>
#include <codec/uni_ie/conn_id.h>
#include <codec/uni_ie/UNI40_leaf.h>
#include <codec/uni_ie/broadband_send_comp_ind.h>
#include <codec/uni_ie/end_pt_ref.h>
#include <codec/q93b_msg/generic_q93b_msg.h>
#include <codec/q93b_msg/release.h>
#include <codec/q93b_msg/release_comp.h>
#include <codec/q93b_msg/setup.h>
#include <codec/q93b_msg/add_party_ack.h>
#include <FW/kernel/SimEvent.h>
#include <FW/kernel/Handlers.h>

#include <sean/daemon/ccd_config.h>

#include <FW/actors/State.h>
#include <sean/cc/sean_Visitor.h>
#include <codec/uni_ie/cause.h>
#include <DS/containers/dictionary.h>

class IPC_DataVisitor;
class IPC_SignallingVisitor;
class sean_Visitor;
class Application;
class tcp_local_id;
class Addr;


class VisitorLaunch;

class Application : public State {
public:

  Application(char * file, int identity);

  State * Handle(Visitor * v);
  void    Interrupt(class SimEvent * e);

  void CUSTOM_HANDLE(Visitor * v);

protected:

  VisitorLaunch* register_service(double timefromnow);
  VisitorLaunch* setup_call(int hostnumber, double timefromnow);
  VisitorLaunch* add_party(int hostnumber, double timefromnow);
  VisitorLaunch* teardown(double timefromnow);

  virtual ~Application();
  void SendMessageToUNI(sean_Visitor * sv, sean_Visitor::sean_Visitor_Type t);
  void Failure_SendRelease(sean_Visitor * sv, ie_cause::CauseValues c);
  void Failure_SendReleaseComplete(sean_Visitor * sv, ie_cause::CauseValues c);

  // Visitors from below
  State * sean_visitor_from_B(sean_Visitor * sv);
  State * visitor_arrived_from_B_side(Visitor * v);

private:
  friend class VisitorLaunch;
  void Send_To_Signalling_Daemon(Visitor* v) {
    PassVisitorToB(v);
  }

  NSAP_DCC_ICD_addr  * _addr;

  InfoElem * _ieo[num_ie];
  InfoElem * _iei[num_ie];
  tcp_local_id * _appid;
  int _cid; // one call for now
  int _crv;
  int _vpi;
  int _vci;

  int _identity;

  VisitorLaunch * _boot;
  VisitorLaunch * _setup_go;
  VisitorLaunch * _ap_go;
  VisitorLaunch * _dp_go;
  VisitorLaunch * _rgo;

  static VisitorType * _sean_type;
};




class VisitorLaunch : public TimerHandler  {
public:
  
  VisitorLaunch(Application * t, Visitor * v, double time) : 
    TimerHandler(t, time), _state(t), _v(v) { }
  virtual ~VisitorLaunch() { }

  void Callback(void) { 
    _state->Send_To_Signalling_Daemon(_v); 
  }

private:

  Application * _state;
  Visitor     * _v;
};



#endif // __APPLICATION_H__
