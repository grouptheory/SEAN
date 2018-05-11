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
static char const _Signalling_API_cc_rcsid_[] =
"$Id: Service_API.cc,v 1.8 1998/08/18 15:05:32 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "Signalling_API.h"
#include "ATM_Interface.h"
#include "Controllers.h"
#include "API_fsms.h"
#include <sean/cc/sean_Visitor.h>
#include <codec/q93b_msg/Buffer.h>
#include <codec/q93b_msg/generic_q93b_msg.h>
#include "Service_API.h"

//-----------------------------------------------
void ATM_Service::Suicide(void) {

  disable_controller_callbacks();
  API_SimEntity* apise = theAPISimEntity();
  apise->preempt_purge(this);

  Deregister();

  The_Recycle_Bin().Digest_Service( this );
}

//-----------------------------------------------

ATM_Service::ATM_Service(ATM_Interface& interface)
  : ATM_Attributes(interface, ATM_Attributes::Service_FSM)
{ }

//-----------------------------------------------

ATM_Service::~ATM_Service() { 
  int sid = Get_Service_ID();
  kill_api_service_datastructure(sid);
}

//-----------------------------------------------
bool ATM_Service::Ready_To_Die(void) {
  return false;
}

//-----------------------------------------------

ATM_Service::service_status ATM_Service::Status(void) const { 
  return (ATM_Service::service_status)( get_fsm_state() );
}

//-----------------------------------------------

ATM_Attributes::result ATM_Service::Register(void) { 
  ATM_Attributes::result ret = ATM_Attributes::failure;
  switch (Status()) {
  case ATM_Service::USER_PREPARING_SERVICE:
    {
      Buffer* buf = new Buffer(4096);
      generic_q93b_msg* msg = make_freeform(buf);
      if (!msg) return ATM_Attributes::failure;
      buf->set_length( msg->encoded_bufferlen() );
      sean_Visitor* sv = new sean_Visitor(sean_Visitor::service_register_req,
					  msg,
					  buf);
      int sid = Get_Service_ID();
      sv->set_sid(sid);
      bool approved = push_FSM(sv);
      if (approved) ret = ATM_Attributes::success;
      // DEFINITELY NOT.  -- BK   delete msg; // sjm leak_scope
      // DEFINITELY NOT.  -- BK   delete buf; // sjm leak_scope
    }
    break;
  default:
    break;
  };
  return ret;
}

//-----------------------------------------------

ATM_Attributes::result ATM_Service::Deregister(void) { 
  ATM_Attributes::result ret = ATM_Attributes::failure; 
  switch (Status()) {
  case ATM_Service::SERVICE_REGISTER_INITIATED:
  case ATM_Service::SERVICE_ACTIVE:
  case ATM_Service::INCOMING_CALL_PRESENT:
    {
      Buffer* buf = new Buffer(4096);
      generic_q93b_msg* msg = make_freeform(buf);
      sean_Visitor* sv = new sean_Visitor(sean_Visitor::service_dereg_req,
					  msg,
					  buf);
      if (!msg) return ATM_Attributes::failure;
      buf->set_length( msg->encoded_bufferlen() );
      int sid = Get_Service_ID();
      sv->set_sid(sid);
      bool approved = push_FSM(sv);      
      if (approved) ret = ATM_Attributes::success;
    }
    break;
  default:
    break;
  };
  return ret;
}

//-----------------------------------------------

Incoming_ATM_Call* ATM_Service::Get_Incoming_Call(void){
  Incoming_ATM_Call* inc = 0;
  switch (Status()) {
  case ATM_Service::INCOMING_CALL_PRESENT:
    if (!_incoming_call_queue.empty())
      inc = _incoming_call_queue.pop();
    break;
  default:
    break;
  }
  return inc;
}

//-----------------------------------------------

int ATM_Service::Get_Service_ID(void) const {
  return ATM_Attributes::get_id();
}

//-----------------------------------------------
ostream& operator << (ostream& os, const ATM_Service& s) 
{ 
  s.Print();
  return os;
}

//-----------------------------------------------
void ATM_Service::Print(void) const 
{
  cout << "Service" << endl;
}

//-----------------------------------------------

void ATM_Service::state_has_changed(API_FSM::api_fsm_state from, 
			       API_FSM::api_fsm_state to,
			       ATM_Call* newc,
			       ATM_Leaf* newl) {
  ATM_Service::service_status from_ = (ATM_Service::service_status) from;
  ATM_Service::service_status to_   = (ATM_Service::service_status) to;

  switch(to_) {
  case ATM_Service::SERVICE_ACTIVE:
    switch(from_) {
    case ATM_Service::SERVICE_REGISTER_INITIATED:
      if (_control)
	_control->__Service_Active(*this);
      else
	diag("api.service",DIAG_DEBUG,
	     "ATM_Service moves to SERVICE_ACTIVE\n");
      break;
    default:
      break;
    };
    break;

  case ATM_Service::SERVICE_INACTIVE:
    if (_control)
      _control->__Service_Inactive(*this);
    else
      diag("api.service",DIAG_DEBUG,
	   "ATM_Service moves to SERVICE_INACTIVE\n");
    break;

  case ATM_Service::INCOMING_CALL_PRESENT:
    assert (newc);
    _incoming_call_queue.append((Incoming_ATM_Call*)newc);
    if (_control)
      _control->__Incoming_Call(*this);
    else
      diag("api.service",DIAG_DEBUG,
	   "ATM_Service moves to INCOMING_CALL_PRESENT\n");
    break;

  default:
    break;
  }
}
