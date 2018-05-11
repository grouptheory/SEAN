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
"$Id: Call_API.cc,v 1.15 1999/03/10 18:45:14 mountcas Exp $";
#endif
#include <common/cprototypes.h>

#include "Signalling_API.h"
#include "ATM_Interface.h"
#include "Controllers.h"
#include "API_fsms.h"
#include "Call_API.h"
#include "API_constants.h"

#include <sean/cc/sean_Visitor.h>
#include <codec/q93b_msg/Buffer.h>
#include <codec/q93b_msg/generic_q93b_msg.h>
#include <sean/xti/sean_io.h>

#include <sean/templates/constants.h>

ATM_Interface* ATM_Call::_current_interface = 0;
ATM_Call* ATM_Call::_current_call = 0;

extern long DRIVER_QOS;


//-----------------------------------------------
void ATM_Call::Suicide(void) {

  disable_controller_callbacks();
  API_SimEntity* apise = theAPISimEntity();
  apise->preempt_purge(this);

  delete _datapath;
  _datapath = 0;

  Set_cause( ie_cause::normal_call_clearing );
  TearDown();

  The_Recycle_Bin().Digest_Call( this );
}

//-----------------------------------------------
bool ATM_Call::leaf_is_dying(ATM_Leaf* leaf) {
  abort();
}

//-----------------------------------------------
void ATM_Call::induced_teardown(void) {
  abort();
}

//-----------------------------------------------
ATM_Call::ATM_Call(ATM_Interface& interface, call_type t) 
  : ATM_Attributes(interface, ATM_Attributes::Call_FSM),
    _t(t), _data_sending(false), _datapath(0)
{ }


//-----------------------------------------------
ATM_Call::ATM_Call(ATM_Interface& interface, 
		   generic_q93b_msg* incoming_setup, 
		   int cid, call_type t)
  : ATM_Attributes( interface, incoming_setup, cid ),
    _t(t), _data_sending(false), _datapath(0)
{ }

//-----------------------------------------------
ATM_Call::~ATM_Call() { 
  // tell leaves they no longer have an owner
  bool kill_call_fsm = 
    this->cleanup_leaves_at_time_of_destructor();  

  if ( kill_call_fsm ) {       // only one leaf object around
    int cid = Get_Call_ID();
    kill_api_call_datastructure(cid);
  }
}

//-----------------------------------------------
bool ATM_Call::Ready_To_Die(void) {
  return false;
}

//-----------------------------------------------
ATM_Call::call_type ATM_Call::Type(void) {
  return _t;
}

//-----------------------------------------------
void ATM_Call::state_has_changed(API_FSM::api_fsm_state from, 
		  API_FSM::api_fsm_state to,
		  ATM_Call* newc,
		  ATM_Leaf* newl) {
  diag("api.call",DIAG_ERROR, "ATM_Call base class virtual state_has_changed() called.\n");
}

//-----------------------------------------------
ATM_Call::call_status ATM_Call::Status(void) const { 
  return (ATM_Call::call_status)( get_fsm_state() );
}

//-----------------------------------------------
ATM_Attributes::result ATM_Call::TearDown(void) { 

  if (Get_cause()==ie_cause::unspecified_or_unknown)
    Set_cause( ie_cause::normal_unspecified );
  
  Buffer* buf = new Buffer(4096);
  generic_q93b_msg* msg = 0;
  sean_Visitor* sv = 0;
  int cid;
  int last_epr = -1;

  bool approved = false;
  switch (Status()) {
  case ATM_Call::CALL_P2MP_READY:
    last_epr = cleanup_leaves_at_time_of_teardown();
    if (last_epr != -1) {
      Set_ep_ref(last_epr);
    }

  case ATM_Call::CALL_P2P_READY:
  case ATM_Call::CALL_LEAF_JOIN_PRESENT:

    diag("api.call.releasebug",DIAG_DEBUG,"call cref = %d\n", Get_crv());
    msg= make_release(buf);
    if (!msg) {
      delete buf;
      return ATM_Attributes::failure;
    }
    diag("api.call.releasebug",DIAG_DEBUG,"release message cref = %d\n", msg->get_crv());

    buf->set_length( msg->encoded_bufferlen() );
    sv = new sean_Visitor(sean_Visitor::release_req, msg, buf);
    cid = Get_Call_ID();
    sv->set_cid(cid);
    sv->set_crv( Get_crv() );

    diag("api.call.releasebug",DIAG_DEBUG,"sean_Vis cref = %d\n", sv->get_crv());

    approved = push_FSM(sv);
    break;
  case ATM_Call::INCOMING_CALL_WAITING:
  case ATM_Call::OUTGOING_CALL_INITIATED:
      msg= make_release_comp(buf);
      if (!msg) {
	delete buf;
	return ATM_Attributes::failure;
      }
      buf->set_length( msg->encoded_bufferlen() );
      sv = new sean_Visitor(sean_Visitor::release_comp_req, msg, buf);
      cid = Get_Call_ID();
      sv->set_cid(cid);
      sv->set_crv( Get_crv() );

      approved = push_FSM(sv);
      break;
  default:
    break;
  }

  if (approved) return ATM_Attributes::success;
  return ATM_Attributes::failure;
}

// NOTE: We must only call this in ACTIVE state
//-----------------------------------------------
void ATM_Call::make_datapath(int QoS, bool data_sending) {   
  long vp,vc;
  ATM_Attributes::result err;

  assert(!_datapath);
  _datapath = new sean_io();
  _datapath->open();
  _data_sending = data_sending;

  err = Get_vpi(vp);  assert(err!=ATM_Attributes::failure);
  err = Get_vci(vc);  assert(err!=ATM_Attributes::failure);

  _current_interface = & ( Get_interface() );   // the subsequent attach() will
  _current_call = this;                         // snoop and save these statics

  _datapath->attach((int)vp, (int)vc, (int)QoS);

  _current_interface = 0;
  _current_call = 0;
}

//-----------------------------------------------
sean_io* ATM_Call::get_datapath(void) {
  return _datapath;
}

//-----------------------------------------------
void ATM_Call::kill_datapath(void) { 
  if (_datapath) {
    _datapath->close();
  }
}

//-----------------------------------------------
int ATM_Call::SendData(u_char* buf, u_long length) { 
  int ret = -1;
  if ( _data_sending && _datapath ){
    switch (Status()) {
    case ATM_Call::CALL_P2MP_READY:
    case ATM_Call::CALL_P2P_READY:
    case ATM_Call::CALL_LEAF_JOIN_PRESENT:
      assert(_datapath);
      ret = _datapath->write((caddr_t)buf, (int)length);
      break;
    default:
      break;
    }
  }
  return ret;
}

//-----------------------------------------------
int ATM_Call::RecvData(u_char* buf, u_long maxlength) { 
  int ret = -1;
  int bytes = 0;

  if (_datapath) {
    switch (Status()) {
    case ATM_Call::CALL_P2MP_READY:
    case ATM_Call::CALL_P2P_READY:
    case ATM_Call::CALL_LEAF_JOIN_PRESENT:
      assert(_datapath);
      bytes = _datapath->read((caddr_t)buf, (int)maxlength);
      ret = bytes;
      break;
    default:
      assert(_datapath);
      bytes = _datapath->read((caddr_t)buf, (int)maxlength);
      if (bytes==0)
	ret = -1;
      break;
    }
  }

  return ret;
}

//-----------------------------------------------
int ATM_Call::Next_PDU_Length(void) { 
  int ret = -1;

  if (_datapath) {
    switch (Status()) {
    case ATM_Call::CALL_P2MP_READY:
    case ATM_Call::CALL_P2P_READY:
    case ATM_Call::CALL_LEAF_JOIN_PRESENT:
      assert(_datapath);
      ret = _datapath->peek_incoming_data_buffer();
      break;
    default:
      break;
    }
  }

  return ret;
}

//-----------------------------------------------
void ATM_Call::Drain(void) {

  if (_datapath) {
    switch (Status()) {
    case ATM_Call::CALL_P2MP_READY:
    case ATM_Call::CALL_P2P_READY:
    case ATM_Call::CALL_LEAF_JOIN_PRESENT:
      assert(_datapath);
      _datapath->drain();
      break;
    default:
      break;
    }
  }
}

//-----------------------------------------------
ostream& operator << (ostream& os, const ATM_Call& s) 
{ 
  s.Print(); 
  return os;
}

void ATM_Call::Print(void) const {
  cout << "Call" << endl;
}

//-----------------------------------------------
int ATM_Call::Get_Call_ID(void) const {
  return ATM_Attributes::get_id();
}


//-----------------------------------------------

bool ATM_Call::cleanup_leaves_at_time_of_destructor(void) { return true; }

int ATM_Call::cleanup_leaves_at_time_of_teardown(void)   { 
  /* redefined in Outgoing_ATM_Call */ 
  return -1;
}


//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------

Incoming_ATM_Call::~Incoming_ATM_Call() {
}

//-----------------------------------------------
ATM_Attributes::result Incoming_ATM_Call::Accept(void) {

  bool approved=false;

  switch (Status()) {
  case ATM_Call::INCOMING_CALL_WAITING:
    {
      Buffer* buf = new Buffer(4096);
      generic_q93b_msg* msg = make_connect(buf);
      if (!msg) {
	delete buf;
	return ATM_Attributes::failure;
      }
      buf->set_length( msg->encoded_bufferlen() );
      sean_Visitor* sv = new sean_Visitor(sean_Visitor::connect_req,msg,buf);
      
      int cid = Get_Call_ID();
      sv->set_cid(cid);
      sv->set_crv( Get_crv() );
      
      if (Is_P2P())
	approved = push_FSM(sv);
      else if (Is_P2MP())
	approved = push_FSM(sv);
    }
    break;
  default:
    break;
  }

  if (approved) return ATM_Attributes::success;
  return ATM_Attributes::failure;
}

//-----------------------------------------------
Incoming_ATM_Call::Incoming_ATM_Call(ATM_Interface& interface, call_type t)
  : ATM_Call(interface, t) {
}

//-----------------------------------------------
Incoming_ATM_Call::Incoming_ATM_Call(ATM_Interface& interface, 
	     generic_q93b_msg* incoming_setup, int cid) 
  : ATM_Call(interface, incoming_setup, cid, ATM_Call::incoming) {
    set_crv( incoming_setup->get_crv() );
}


//-----------------------------------------------
void Incoming_ATM_Call::state_has_changed(API_FSM::api_fsm_state from, 
				     API_FSM::api_fsm_state to,
				     ATM_Call* newc,
				     ATM_Leaf* newl) {
  ATM_Call::call_status from_ = (ATM_Call::call_status)from;
  ATM_Call::call_status to_   = (ATM_Call::call_status)to;

  bool data_sending = true;

  switch (to_){

  case ATM_Call::CALL_P2MP_READY:

    data_sending = false;
    // fall through

  case ATM_Call::CALL_P2P_READY:

    make_datapath(DRIVER_QOS, data_sending);

    break;

  case ATM_Call::CALL_RELEASED:
    
    kill_datapath();

    switch(from_) {
    case ATM_Call::CALL_RELEASE_INITIATED:
    case ATM_Call::CALL_P2MP_READY:
    case ATM_Call::CALL_P2P_READY:
      if (_control)
	_control->__Call_Inactive(*this);
      else
	diag("api.call",DIAG_DEBUG,
	     "Incoming_ATM_Call moves to CALL_RELEASED\n");
      break;
    default:
      break;
    }
    break;

  default:    
    break;
  }
}


//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------


Leaf_Initiated_ATM_Call::Leaf_Initiated_ATM_Call(ATM_Interface& interface) : 
  Incoming_ATM_Call(interface, ATM_Call::leaf_initiated) {

    set_crv( NO_CREF );
    initialize_leaf_initiated_call();
}

//-----------------------------------------------
Leaf_Initiated_ATM_Call::~Leaf_Initiated_ATM_Call() { }

//-----------------------------------------------
ATM_Attributes::result Leaf_Initiated_ATM_Call::Request(void) {
  bool approved = false;
  switch (Status()) {
  case ATM_Call::LIJ_PREPARING:
    {
      Buffer* buf = new Buffer(4096);

      set_dummy_LIJ_sequence_number();

      generic_q93b_msg* msg = make_leaf_setup_request(buf);

      if (!msg) {
	delete buf;
	return ATM_Attributes::failure;
      }
      buf->set_length( msg->encoded_bufferlen() );
      sean_Visitor* sv = new sean_Visitor(sean_Visitor::leaf_setup_req,msg,buf);
      sv->set_cid( Get_Call_ID() );
      sv->set_crv( Get_crv() );
      approved = 
	push_FSM(sv);
    }
  break;
  default:
    break;
  }

  if (approved) return ATM_Attributes::success;
  return ATM_Attributes::failure;
}
  
//-----------------------------------------------
void Leaf_Initiated_ATM_Call::state_has_changed(API_FSM::api_fsm_state from, 
					   API_FSM::api_fsm_state to,
					   ATM_Call* newc,
					   ATM_Leaf* newl) {
  diag("api.call",DIAG_INFO, "Leaf_Initiated_ATM_Call state_has_changed() was called.\n");
  ATM_Call::call_status from_ = (ATM_Call::call_status)from;
  ATM_Call::call_status to_   = (ATM_Call::call_status)to;
  bool data_sending = false;

  switch (to_){
  case ATM_Call::INCOMING_CALL_WAITING:
    if (_control)
      _control->__Incoming_Call_From_Root(*this);
    else
      diag("api.call",DIAG_DEBUG,
	   "Leaf_Initiated_ATM_Call moves to INCOMING_CALL_WAITING\n");
    break;

  case ATM_Call::CALL_P2MP_READY:    // LIJ_Calls cannot be p2p !

    make_datapath(DRIVER_QOS, data_sending);
    break;

  case ATM_Call::CALL_RELEASED:

    kill_datapath();

    switch(from_) {  
    case ATM_Call::CALL_P2MP_READY:
    case ATM_Call::CALL_RELEASE_INITIATED:
    case ATM_Call::LIJ_INITIATED:
      if (_control)
	_control->__Call_Inactive(*this);
      else
	diag("api.call",DIAG_DEBUG,
	     "Incoming_ATM_Call moves to CALL_RELEASED\n");
      break;
    };

  default:
    break;
  }
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------


Outgoing_ATM_Call::Outgoing_ATM_Call(ATM_Interface& interface) 
  : ATM_Call(interface, ATM_Call::outgoing),
    _next_leaf_id( 0 ) {
    set_crv( NO_CREF );
    _leaf0 = Make_Leaf(*this);  // we will adopt this leaf in the ctor of ATM_Leaf
    assert(_leaf0);
}


//-----------------------------------------------
Outgoing_ATM_Call::~Outgoing_ATM_Call() { }

//-----------------------------------------------
bool Outgoing_ATM_Call::leaf_is_dying(ATM_Leaf* dying_leaf) {
  list_item li;
  bool still_truckin=false;
  forall_items(li,_leaves) {
    ATM_Leaf* child = _leaves.inf(li);
    if ((child->Status() == ATM_Leaf::LEAF_READY) &&
	(child != dying_leaf)) {
      still_truckin=true;
    }
  }
  return still_truckin;
}

//-----------------------------------------------
void Outgoing_ATM_Call::induced_teardown(void) {
  list_item li;
  forall_items(li,_leaves) {
    ATM_Leaf* child = _leaves.inf(li);
    child->force_leaf_to_dead_state();
  }
  force_call_to_dead_state();

  if (_control) 
    _control->__Call_Inactive(*this);
  else
    diag("api.call",DIAG_DEBUG,
	 "Outgoing_ATM_Call moves to CALL_RELEASED\n");
}

//-----------------------------------------------
ATM_Attributes::result Outgoing_ATM_Call::Establish(void) { 
  bool approved = false;

  switch (Status()) {
  case ATM_Call::USER_PREPARING_CALL:
    {
      Addr* dst_addr;
      int call_id_ie_value;
      if ((Get_called_party_num(dst_addr) == ATM_Attributes::failure) &&           // we have no called party
	  (Get_LIJ_call_id(call_id_ie_value) == ATM_Attributes::success) &&        // we have a call id ie
	  (Is_P2MP())) {
	// we make dummy called party, so that we can listen for LIJs
	// and use the first leaf_setup_ind to establish the call 
	dst_addr = newAddr( DELAYED_SETUP_ADDRESS_TOKEN ); 
	Set_called_party_num(dst_addr);
      }

      int leaf_id = -1;
      if (Is_P2MP()) {
	leaf_id = _leaf0->Get_Leaf_ID();
	assert( leaf_id == 0);
	unprotect();
	ATM_Attributes::result res = Set_ep_ref( leaf_id );
	protect();
	assert( res == ATM_Attributes::success);
      }

      Buffer* buf = new Buffer(4096);
      generic_q93b_msg* msg = make_setup(buf);
      if (!msg) {
	delete buf;
	return ATM_Attributes::failure;
      }

      buf->set_length( msg->encoded_bufferlen() );
      sean_Visitor* sv = new sean_Visitor(sean_Visitor::setup_req,msg,buf);
      sv->set_cid( Get_Call_ID() );
      sv->set_crv( Get_crv() );
      sv->set_lid( leaf_id );
      
      approved = push_FSM(sv);
    }
    break;
  default:
    break;
  }

  if (approved) return ATM_Attributes::success;
  return ATM_Attributes::failure;
}


//-----------------------------------------------
ATM_Leaf* Outgoing_ATM_Call::Get_Initial_Leaf(void) {
  if (Status() == CALL_P2MP_READY)
    return _leaf0;
  else
    return 0;
}


//-----------------------------------------------
ATM_Leaf* Outgoing_ATM_Call::Get_Leaf_Join_Request(void) {
  ATM_Leaf* leaf = 0;
  if (Status()==ATM_Call::CALL_LEAF_JOIN_PRESENT) {
    leaf = _incoming_LIJ_queue.pop();
    if (_incoming_LIJ_queue.empty()) {
      notify_FSM_that_queue_is_empty();
    }
  }
  return leaf;
}


//-----------------------------------------------
void Outgoing_ATM_Call::state_has_changed(API_FSM::api_fsm_state from, 
			    API_FSM::api_fsm_state to,
			    ATM_Call* newc,
			    ATM_Leaf* newl) {
  ATM_Call::call_status from_ = (ATM_Call::call_status)from;
  ATM_Call::call_status to_   = (ATM_Call::call_status)to;

  bool data_sending = true;

  switch (to_){
  case ATM_Call::CALL_P2P_READY:
  case ATM_Call::CALL_P2MP_READY: 

    _leaf0->force_leaf_to_active_state();

    make_datapath(DRIVER_QOS, data_sending);

    if (_control) 
      _control->__Call_Active(*this);
    else
      diag("api.call",DIAG_DEBUG,
	   "Outgoing_ATM_Call moves to CALL_P2P_READY/CALL_P2MP_READY\n");
    break;

  case ATM_Call::CALL_RELEASED:

    kill_datapath();

    if (_control) 
      _control->__Call_Inactive(*this);
    else
      diag("api.call",DIAG_DEBUG,
	   "Outgoing_ATM_Call moves to CALL_RELEASED\n");
    break;

  case ATM_Call::CALL_LEAF_JOIN_PRESENT:
    assert (newl);
    _incoming_LIJ_queue.append(newl);
    if (_control) 
      _control->__Incoming_LIJ(*this);
    else
      diag("api.call",DIAG_DEBUG,
	   "Outgoing_ATM_Call moves to CALL_LEAF_JOIN_PRESENT\n");
    break;

  default:
    break;
  }
}

//-----------------------------------------------
bool Outgoing_ATM_Call::adopt_leaf(ATM_Leaf* lf) {
  list_item li = _leaves.search(lf);
  if (li) return false;
  else _leaves.append(lf);
  return true;
}

//-----------------------------------------------
bool Outgoing_ATM_Call::abandon_leaf(ATM_Leaf* lf) {
  if (_leaf0 == lf) _leaf0 = 0;
  list_item li = _leaves.search(lf);
  if (! li) return false;
  else _leaves.del_item(li);
  return true;
}

//-----------------------------------------------
int Outgoing_ATM_Call::cleanup_leaves_at_time_of_teardown(void) {
  list_item li;
  int last_epr = -1;
  int ct = _leaves.size();
  forall_items( li, _leaves ) {
    ATM_Leaf* leaf = _leaves.inf(li);
    if (ct>1) {
      leaf->Drop();
      ct--;
    }
    else {
      last_epr = leaf->Get_ep_ref();
      break;
    }
  }
  return last_epr;
}

//-----------------------------------------------
bool Outgoing_ATM_Call::cleanup_leaves_at_time_of_destructor(void) {
  list_item li;

  forall_items( li, _leaves ) {
    ATM_Leaf* leaf = _leaves.inf(li);
    leaf->orphaned_by_call(this);
  }

  return ( _leaves.size() == 1 );
}

//-----------------------------------------------
int Outgoing_ATM_Call::Get_Next_EPR(void) {
  _next_leaf_id+=1;
  return _next_leaf_id-1;
}

//-----------------------------------------------
Controller* Outgoing_ATM_Call::get_call_controller(void) {
  return _control;
}


