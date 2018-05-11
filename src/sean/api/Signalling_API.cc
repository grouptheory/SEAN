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

#ifndef LINT
static char const _Signalling_API_cc_rcsid_[] =
"$Id: Signalling_API.cc,v 1.2 1998/08/06 04:03:26 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "Signalling_API.h"
#include "ATM_Interface.h"
#include "Controllers.h"
#include "API_fsms.h"
#include <sean/cc/sean_Visitor.h>
#include <codec/q93b_msg/Buffer.h>
#include <codec/q93b_msg/generic_q93b_msg.h>


ATM_Call::ATM_Call(ATM_Interface& interface, call_direction d) 
  : ATM_Attributes(interface, ATM_Attributes::Call_FSM),
    _control(0), _dir(d) { }

ATM_Call::ATM_Call(ATM_Interface& interface, 
		   generic_q93b_msg* incoming_setup, 
		   int cid, call_direction d)
  : ATM_Attributes( interface, incoming_setup, cid ), 
    _control(0), _dir(d) { 
}

ATM_Call::~ATM_Call() { 
  set_cause( ie_cause::normal_call_clearing );
  TearDown();
  int cid = Get_Call_ID();
  kill_api_call_datastructure(cid);
}

ATM_Call::call_status ATM_Call::Status(void) const { 
  return (ATM_Call::call_status)( get_fsm_state() );
}

ATM_Attributes::result ATM_Call::TearDown(void) { 
  Buffer* buf = new Buffer(4096);
  generic_q93b_msg* msg;
  sean_Visitor* sv;
  bool approved = false;
  if ((Status()==CALL_P2P_READY)||(Status()==CALL_P2MP_READY)) {
    msg= make_release(buf);
    if (!msg) return ATM_Attributes::failure;
    buf->set_length( msg->encoded_bufferlen() );
    sv = new sean_Visitor(sean_Visitor::release_req, msg, buf);
    int cid = Get_Call_ID();
    sv->set_cid(cid);
    approved = Push_FSM(API_FSM::call_release_initiated,sv);
  }
  else {
    msg= make_release_comp(buf);
    if (!msg) return ATM_Attributes::failure;
    buf->set_length( msg->encoded_bufferlen() );
    sv = new sean_Visitor(sean_Visitor::release_comp_req, msg, buf);
    int cid = Get_Call_ID();
    sv->set_cid(cid);
    approved = Push_FSM(API_FSM::call_released,sv);
  }
  if (approved) return ATM_Attributes::success;
  return ATM_Attributes::failure;
}

int ATM_Call::SendData(u_char* buf, u_long length) { }
int ATM_Call::RecvData(u_char* buf, u_long maxlength) { }
int ATM_Call::RecvDataNB(u_char* buf, u_long maxlength) { }
int ATM_Call::Next_PDU_Length(void) { }

ostream& operator << (ostream& os, const ATM_Call& s) { s.Print(); }
void ATM_Call::Print(void) const {
  cout << "Call" << endl;
}

int ATM_Call::Get_Call_ID(void) const {
  return ATM_Attributes::get_id();
}

void ATM_Call::Associate_Call_Controller(Call_Controller& cc) {
  if (_control) 
    _control->Abandon_Managed_Call(this);
  _control = &cc;
  _control->Adopt_Managed_Call(this);
}


void ATM_Call::Orphaned_by_Call_Controller(void) {
  _control=0;
}

//-----------------------------------------------

Incoming_ATM_Call::~Incoming_ATM_Call() {
}

ATM_Attributes::result Incoming_ATM_Call::Accept(void) {
  Buffer* buf = new Buffer(4096);
  generic_q93b_msg* msg = make_connect(buf);
  if (!msg) return ATM_Attributes::failure;
  buf->set_length( msg->encoded_bufferlen() );
  sean_Visitor* sv = new sean_Visitor(sean_Visitor::connect_req,msg,buf);

  int cid = Get_Call_ID();
  sv->set_cid(cid);

  sv->set_crv( get_crv() );

  bool approved=false;
  if (is_P2P())
    approved = 
      Push_FSM(API_FSM::call_p2p_active, sv);
  else if (is_P2MP())
    approved = 
      Push_FSM(API_FSM::call_p2mp_active, sv);
    
  if (approved) return ATM_Attributes::success;
  return ATM_Attributes::failure;
}

Incoming_ATM_Call::Incoming_ATM_Call(ATM_Interface& interface, 
	     generic_q93b_msg* incoming_setup, int cid) 
  : ATM_Call(interface, incoming_setup, cid, ATM_Call::incoming) {
    set_crv( incoming_setup->get_crv() );
}


void Incoming_ATM_Call::StateChanged(API_FSM::api_fsm_state from, 
				     API_FSM::api_fsm_state to,
				     ATM_Call* newc,
				     ATM_Leaf* newl) {
  if (!_control) return;

  ATM_Call::call_status from_ = (ATM_Call::call_status)from;
  ATM_Call::call_status to_   = (ATM_Call::call_status)to;
  switch (to_){
  case ATM_Call::CALL_P2P_READY:
  case ATM_Call::CALL_P2MP_READY:
    _control->__Call_Active(*this);
    break;
  case ATM_Call::CALL_RELEASED:
    _control->__Call_Inactive(*this);
    break;
  default:    break;
  }
}

//-----------------------------------------------

Outgoing_ATM_Call::Outgoing_ATM_Call(ATM_Interface& interface) 
  : ATM_Call(interface, ATM_Call::outgoing) {
    set_crv( NO_CREF );
}

Outgoing_ATM_Call::~Outgoing_ATM_Call() {
}

ATM_Attributes::result Outgoing_ATM_Call::Establish(void) { 
  Buffer* buf = new Buffer(4096);
  generic_q93b_msg* msg = make_setup(buf);
  if (!msg) return ATM_Attributes::failure;
  buf->set_length( msg->encoded_bufferlen() );
  sean_Visitor* sv = new sean_Visitor(sean_Visitor::setup_req,msg,buf);
  int cid = Get_Call_ID();
  sv->set_cid(cid);

  sv->set_crv( get_crv() );

  bool approved = 
    Push_FSM(API_FSM::call_outgoing_setup__waiting_for_remote,
			   sv);
  if (approved) return ATM_Attributes::success;
  return ATM_Attributes::failure;
}

ATM_Leaf* Outgoing_ATM_Call::Get_Leaf_Join_Request(void) {
  if (Status()!=ATM_Call::CALL_LIJ_PRESENT) return 0;
  ATM_Leaf* leaf = _incoming_LIJ_queue.pop();
  if (_incoming_LIJ_queue.empty()) {
    Notify_FSM_That_Queue_Is_Empty();
  }
  return leaf;
}


void Outgoing_ATM_Call::StateChanged(API_FSM::api_fsm_state from, 
			    API_FSM::api_fsm_state to,
			    ATM_Call* newc,
			    ATM_Leaf* newl) {
  if (!_control) return;

  ATM_Call::call_status from_ = (ATM_Call::call_status)from;
  ATM_Call::call_status to_   = (ATM_Call::call_status)to;
  switch (to_){
  case ATM_Call::CALL_P2P_READY:
  case ATM_Call::CALL_P2MP_READY:
    _control->__Call_Active(*this);
    break;
  case ATM_Call::CALL_RELEASED:
    _control->__Call_Inactive(*this);
    break;
  case ATM_Call::CALL_LIJ_PRESENT:
    if (newl) {
      _incoming_LIJ_queue.append(newl);
      _control->__Incoming_LIJ(*this);
    }
    break;
  default:
    break;
  }
}


bool Outgoing_ATM_Call::Adopt_Leaf(ATM_Leaf* lf);
bool Outgoing_ATM_Call::Abandon_Leaf(ATM_Leaf* lf);

//-----------------------------------------------

ATM_Service::ATM_Service(ATM_Interface& interface) 
  : ATM_Attributes(interface, ATM_Attributes::Service_FSM),
    _control(0) { }

ATM_Service::~ATM_Service() { 
  Deregister();
  int sid = Get_Service_ID();
  kill_api_service_datastructure(sid);
}

ATM_Service::service_status ATM_Service::Status(void) const { 
  return (ATM_Service::service_status)( get_fsm_state() );
}

ATM_Attributes::result ATM_Service::Register(void) { 
  Buffer* buf = new Buffer(4096);
  generic_q93b_msg* msg = make_freeform(buf);
  if (!msg) return ATM_Attributes::failure;
  buf->set_length( msg->encoded_bufferlen() );
  sean_Visitor* sv = new sean_Visitor(sean_Visitor::service_register_req,
				      msg,
				      buf);
  int sid = Get_Service_ID();
  sv->set_sid(sid);
  bool approved = Push_FSM(API_FSM::service_register_requested,sv);
  if (approved) return ATM_Attributes::success;
  return ATM_Attributes::failure;
}

ATM_Attributes::result ATM_Service::Deregister(void) { 
  Buffer* buf = new Buffer(4096);
  generic_q93b_msg* msg = make_freeform(buf);
  sean_Visitor* sv = new sean_Visitor(sean_Visitor::service_dereg_req,
				      msg,
				      buf);
  if (!msg) return ATM_Attributes::failure;
  buf->set_length( msg->encoded_bufferlen() );
  int sid = Get_Service_ID();
  sv->set_sid(sid);
  bool approved = Push_FSM(API_FSM::service_deregister_requested,sv);
  if (approved) return ATM_Attributes::success;
  return ATM_Attributes::failure;
}

ostream& operator << (ostream& os, const ATM_Service& s) { s.Print(); }
void ATM_Service::Print(void) const {
  cout << "Service" << endl;
}

int ATM_Service::Get_Service_ID(void) const {
  return ATM_Attributes::get_id();
}

void ATM_Service::Associate_Service_Controller(Service_Controller& sc) {
  if (_control) 
    _control->Abandon_Managed_Service(this);
  _control = &sc;
  _control->Adopt_Managed_Service(this);
}


void ATM_Service::Orphaned_by_Service_Controller(void) {
  _control=0;
}

Incoming_ATM_Call* ATM_Service::Get_Incoming_Call(void) {
  if (Status()!=ATM_Service::INCOMING_CALL_PRESENT) return 0;
  Incoming_ATM_Call* call = _incoming_call_queue.pop();
  if (_incoming_call_queue.empty()) {
    Notify_FSM_That_Queue_Is_Empty();
  }
  return call;
}

void ATM_Service::StateChanged(API_FSM::api_fsm_state from, 
			       API_FSM::api_fsm_state to,
			       ATM_Call* newc,
			       ATM_Leaf* newl) {
  if (!_control) return;

  ATM_Service::service_status from_ = (ATM_Service::service_status) from;
  ATM_Service::service_status to_   = (ATM_Service::service_status) to;

  switch(to_) {
  case ATM_Service::SERVICE_ACTIVE:
    _control->__Service_Active(*this);
    break;
  case ATM_Service::SERVICE_INACTIVE:
    _control->__Service_Inactive(*this);
    break;
  case ATM_Service::INCOMING_CALL_PRESENT:
    if (newc) {
      _incoming_call_queue.append((Incoming_ATM_Call*)newc);
      _control->__Incoming_Call(*this);
    }
    break;
  default:
    break;
  }
}

//-----------------------------------------------

ATM_Leaf::ATM_Leaf(ATM_Call& call) 
  : ATM_Attributes(call.Get_Interface(), call, ATM_Attributes::Leaf_FSM),
    _owning_call(&call) { 
      _control = call._control;
}

ATM_Leaf::~ATM_Leaf() { 
  int lid = Get_Leaf_ID();
  kill_api_leaf_datastructure(0,lid);
}

ATM_Leaf::leaf_status ATM_Leaf::Status(void) const { 
  return (ATM_Leaf::leaf_status)( get_fsm_state() );
}

ATM_Attributes::result ATM_Leaf::Join(void) { 
  Buffer* buf = new Buffer(4096);
  generic_q93b_msg* msg = make_addparty(buf);
  if (!msg) return ATM_Attributes::failure;
  buf->set_length( msg->encoded_bufferlen() );
  sean_Visitor* sv = new sean_Visitor(sean_Visitor::add_party_req,msg,buf);
  int lid = Get_Leaf_ID();
  sv->set_lid(lid);
  bool approved = 
  Push_FSM(API_FSM::leaf_addparty_requested__waiting_for_remote,
			 sv);
  if (approved) return ATM_Attributes::success;
  return ATM_Attributes::failure;
}

ATM_Attributes::result ATM_Leaf::Drop(void) { 
  Buffer* buf = new Buffer(4096);
  generic_q93b_msg* msg = make_dropparty(buf);
  if (!msg) return ATM_Attributes::failure;
  buf->set_length( msg->encoded_bufferlen() );
  sean_Visitor* sv = new sean_Visitor(sean_Visitor::drop_party_req,msg,buf);
  int lid = Get_Leaf_ID();
  sv->set_lid(lid);
  bool approved = Push_FSM(API_FSM::leaf_released,sv);
  if (approved) return ATM_Attributes::success;
  return ATM_Attributes::failure;
}

ostream& operator << (ostream& os, const ATM_Leaf& s) { s.Print(); }
void ATM_Leaf::Print(void) const {
  cout << "Leaf" << endl;
}

int ATM_Leaf::Get_Leaf_ID(void) const {
  return ATM_Attributes::get_id();
}

void ATM_Leaf::StateChanged(API_FSM::api_fsm_state from, 
			    API_FSM::api_fsm_state to,
			    ATM_Call* newc,
			    ATM_Leaf* newl) {
  if (!_control) return;

  ATM_Leaf::leaf_status from_ = (ATM_Leaf::leaf_status) from;
  ATM_Leaf::leaf_status to_   = (ATM_Leaf::leaf_status) to;

  switch(to_) {
  case ATM_Leaf::LEAF_READY:
    _control->__Leaf_Active(*this, *_owning_call);
    break;
  case ATM_Leaf::LEAF_RELEASED:
    _control->__Leaf_Inactive(*this, *_owning_call);
    break;
  default:
    break;
  }
}
