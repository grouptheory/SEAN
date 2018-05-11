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
"$Id: Leaf_API.cc,v 1.10 1999/03/10 18:45:16 mountcas Exp $";
#endif
#include <common/cprototypes.h>

#include "Signalling_API.h"
#include "ATM_Interface.h"
#include "Controllers.h"
#include "API_fsms.h"
#include <sean/cc/sean_Visitor.h>
#include <codec/q93b_msg/Buffer.h>
#include <codec/q93b_msg/generic_q93b_msg.h>
#include "Leaf_API.h"

//-----------------------------------------------
void ATM_Leaf::Suicide(void) {

  disable_controller_callbacks();
  API_SimEntity* apise = theAPISimEntity();
  apise->preempt_purge(this);

  Drop();

  if (_owning_call) {
    bool success = _owning_call->abandon_leaf(this);
    assert(success);
    _owning_call = 0;
  }

  The_Recycle_Bin().Digest_Leaf( this );
}

//-----------------------------------------------
ATM_Leaf* Make_Leaf(ATM_Call& call) {
  ATM_Call::call_type t = call.Type();
  if (t != ATM_Call::outgoing) {
    // only outgoing calls can make leaves
    return 0;
  }
  else {
    return new ATM_Leaf( *((Outgoing_ATM_Call*)(&call)) );
  }
}

//-----------------------------------------------
ATM_Leaf::ATM_Leaf(Outgoing_ATM_Call& call) : 
  ATM_Attributes(call.Get_interface(), call, 
		 ATM_Attributes::Leaf_FSM),
  _owning_call(&call), _owning_call_id( call.Get_Call_ID() )
{ 
  _control = call._control;        // of course the call
                                   // may still be unassociated!

  bool success = call.adopt_leaf(this);
  assert(success);
}

//-----------------------------------------------
bool ATM_Leaf::Ready_To_Die(void) {
  return false;
}

//-----------------------------------------------

ATM_Leaf::~ATM_Leaf() { 
  int lid = Get_Leaf_ID();
  kill_api_leaf_datastructure( _owning_call_id ,lid );
}

//-----------------------------------------------

ATM_Leaf::leaf_status ATM_Leaf::Status(void) const { 
  return (ATM_Leaf::leaf_status)( get_fsm_state() );
}

//-----------------------------------------------

ATM_Attributes::result ATM_Leaf::Add(void) { 

  if ((!_owning_call) ||
      (_owning_call->Status() != ATM_Call::CALL_P2MP_READY))
    return ATM_Attributes::failure;

  bool approved = false;
  
  switch( Status() ) {
  case ATM_Leaf::INCOMING_LIJ:
    {
      Buffer* buf = new Buffer(4096);

      swap_called_and_calling_party();

      generic_q93b_msg* msg = make_addparty(buf);
      if (!msg) {
	delete buf;
	return ATM_Attributes::failure;
      }
      buf->set_length( msg->encoded_bufferlen() );
      sean_Visitor* sv = new sean_Visitor(sean_Visitor::add_party_req,msg,buf);
      int lid = Get_Leaf_ID();
      sv->set_lid(lid);
      sv->set_cid( _owning_call->Get_Call_ID() );
      sv->set_crv( _owning_call->Get_crv() );
      approved = push_FSM(sv);
    }
    break;

  case ATM_Leaf::USER_PREPARING_LEAF:
    {
      Buffer* buf = new Buffer(4096);
      generic_q93b_msg* msg = make_addparty(buf);
      if (!msg) {
	delete buf;
	return ATM_Attributes::failure;
      }
      buf->set_length( msg->encoded_bufferlen() );
      sean_Visitor* sv = new sean_Visitor(sean_Visitor::add_party_req,msg,buf);
      int lid = Get_Leaf_ID();
      sv->set_lid(lid);
      sv->set_cid( _owning_call->Get_Call_ID() );
      sv->set_crv( _owning_call->Get_crv() );
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

ATM_Attributes::result ATM_Leaf::Drop(void) { 
  if ((!_owning_call) ||
      (_owning_call->Status() != ATM_Call::CALL_P2MP_READY))
    return ATM_Attributes::failure;

  bool approved = false;

  if (Get_cause()==ie_cause::unspecified_or_unknown)
    Set_cause( ie_cause::normal_unspecified );
  
  
  switch( Status() ) {
  case ATM_Leaf::INCOMING_LIJ:
    {
      swap_called_and_calling_party();

      Buffer* buf = new Buffer(4096);
      generic_q93b_msg* msg = make_leaf_setup_failure(buf);
      if (!msg) {
	delete buf;
	return ATM_Attributes::failure;
      }
      buf->set_length( msg->encoded_bufferlen() );
      sean_Visitor* sv = new sean_Visitor(sean_Visitor::leaf_setup_failure_req,msg,buf);
      int lid = Get_Leaf_ID();
      sv->set_lid(lid);
      sv->set_cid( _owning_call->Get_Call_ID() );
      sv->set_crv( _owning_call->Get_crv() );
      approved = push_FSM(sv);
    }
    break;
  case ATM_Leaf::LEAF_READY:
    {
      Buffer* buf = new Buffer(4096);
      generic_q93b_msg* msg = make_dropparty(buf);
      if (!msg) {
	delete buf;
	return ATM_Attributes::failure;
      }
      buf->set_length( msg->encoded_bufferlen() );
      sean_Visitor* sv = new sean_Visitor(sean_Visitor::drop_party_req,msg,buf);
      int lid = Get_Leaf_ID();
      sv->set_lid(lid);
      sv->set_cid( _owning_call->Get_Call_ID() );
      sv->set_crv( _owning_call->Get_crv() );
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

ostream & operator << (ostream & os, const ATM_Leaf & s) 
{ 
  // Why don't you pass the ostream into print?
  s.Print(); 
  return os;
}

void ATM_Leaf::Print(void) const 
{
  cout << "Leaf" << endl;
}

//-----------------------------------------------

int ATM_Leaf::Get_Leaf_ID(void) const {
  return ATM_Attributes::get_id();
}

//-----------------------------------------------

void ATM_Leaf::state_has_changed(API_FSM::api_fsm_state from, 
				 API_FSM::api_fsm_state to,
				 ATM_Call* newc,
				 ATM_Leaf* newl) {
  ATM_Leaf::leaf_status from_ = (ATM_Leaf::leaf_status) from;
  ATM_Leaf::leaf_status to_   = (ATM_Leaf::leaf_status) to;

  if (_owning_call)
    _control = _owning_call->get_call_controller();
  else
    _control = 0;

  bool still_truckin=false;

  switch(to_) {
  case ATM_Leaf::LEAF_READY:
    if (_control)
      _control->__Leaf_Active(*this, *_owning_call);
    else
      diag("api.leaf",DIAG_DEBUG,
	   "ATM_Leaf moves to LEAF_READY\n");
    break;

  case ATM_Leaf::LEAF_RELEASED:

    still_truckin = _owning_call->leaf_is_dying(this);

    if (still_truckin) {
      if (_control) 
	_control->__Leaf_Inactive(*this, *_owning_call);
      else
	diag("api.leaf",DIAG_DEBUG,
	     "ATM_Leaf moves to LEAF_RELEASED\n");
    }
    else {
      _owning_call->induced_teardown();
    }

    break;

  default:
    break;
  }
}

//-----------------------------------------------
// a leaf build as a result of leaf_setup_ind
//-----------------------------------------------
ATM_Leaf::ATM_Leaf(Outgoing_ATM_Call& call,
		   generic_q93b_msg* incoming_leaf_setup, 
		   int lid)  : 
  ATM_Attributes(call.Get_interface(), call, 
		 incoming_leaf_setup,
		 ATM_Attributes::Leaf_FSM,
		 lid),
  _owning_call(&call) 
{ 
  _control = call._control;        // of course the call
                                   // may still be unassociated!

  bool success = call.adopt_leaf(this);
  assert(success);
}

//-----------------------------------------------

void ATM_Leaf::orphaned_by_call(Outgoing_ATM_Call* abandoner) {
  assert(_owning_call == abandoner);
  _owning_call = 0;
}

