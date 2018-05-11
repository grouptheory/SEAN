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
static char const _API_fsms_cc_rcsid_[] =
"$Id: API_fsms.cc,v 1.3 1998/08/06 04:03:21 bilal Exp $";
#endif
#include <common/cprototypes.h>
// -*- C++ -*-

#include "ATM_Attributes.h"
#include "API_fsms.h"
#include "Signalling_API.h"
#include "ATM_Interface.h"

#include <sean/cc/sean_Visitor.h>
#include <FW/basics/VisitorType.h>
#include <FW/kernel/SimEvent.h>


const VisitorType* API_FSM::_sean_visitor_type=0;

//---------------------------------------------------------
API_FSM::API_FSM(int id) 
  : _attr(0), _id(id), 
    _mark_for_deletion(false), 
    _state(API_FSM::no_fsm) {
  if (_sean_visitor_type == 0)
    _sean_visitor_type = QueryRegistry(SEAN_VISITOR_NAME);
}

//---------------------------------------------------------
void API_FSM::mark_FSM_for_deletion(void) {
  _mark_for_deletion = true;
}

//---------------------------------------------------------
bool API_FSM::is_FSM_marked_for_deletion(void) {
  return _mark_for_deletion;
}

//---------------------------------------------------------
void API_FSM::set_crv_in_attributes(unsigned long crv) {
  if (_attr) 
    _attr->set_crv(crv);
  else
    diag("api.fsm",DIAG_DEBUG,"API_FSM::set_crv_in_attributes() attempted to access deleted ATM_Attributes.\n");
}

//---------------------------------------------------------
void API_FSM::notify_attributes_of_fsm_failure(int code) {
  if (_attr) 
    _attr->fsm_failure(code);
  else
    diag("api.fsm",DIAG_DEBUG,"API_FSM::notify_attributes_of_fsm_failure() attempted to access deleted ATM_Attributes.\n");
}

//---------------------------------------------------------
void API_FSM::update_attributes(generic_q93b_msg* m) {
  if (_attr) 
    _attr->update_from_message(m);
  else
    diag("api.fsm",DIAG_ERROR,"API_FSM::update_attributes() attempted to access deleted ATM_Attributes.\n");
}

//---------------------------------------------------------
API_FSM::~API_FSM() { 
  if (! _mark_for_deletion ) {
    diag("api.fsm",DIAG_FATAL,"Pathological delete of API_FSM Object\n");
    abort();
  }
}

//---------------------------------------------------------
API_FSM::api_fsm_state API_FSM::get_state(void) const { return _state; }

//---------------------------------------------------------
void API_FSM::associate_attributes(ATM_Attributes* attr) {
  _attr = attr;
}

//---------------------------------------------------------
void API_FSM::set_queue_to_empty(void) {  
  diag("api.fsm",DIAG_FATAL,"Abstract API_FSM method set_queue_empty called\n");
}

//---------------------------------------------------------
void API_FSM::change_state_with_callback(API_FSM::api_fsm_state st, 
					 ATM_Call* call, 
					 ATM_Leaf* leaf) { 
  API_FSM::api_fsm_state old = _state;
  _state = st; 
  if (_attr) {
    _attr->set_modifiable_bits(st);
    _attr->state_has_changed(old, st, call, leaf);
  }
  else
    diag("api.fsm",DIAG_DEBUG,"API_FSM::change_state_with_callback() attempted to access deleted ATM_Attributes.\n");
}

//---------------------------------------------------------
void API_FSM::change_state_no_callback(API_FSM::api_fsm_state transition_to) {
  _state = transition_to;
  if (_attr) 
    _attr->set_modifiable_bits(transition_to);
  else
    diag("api.fsm",DIAG_DEBUG,"API_FSM::change_state_no_callback() attempted to access deleted ATM_Attributes.\n");
}

//---------------------------------------------------------
void Print_API_FSM_State(API_FSM::api_fsm_state s, ostream& os) {
  switch(s) {
  case API_FSM::no_fsm: 
    os << "no_fsm";
    break;
  case API_FSM::ccd_dead: 
    os << "ccd_dead";
    break;

  case API_FSM::call_preparing: 
    os << "call_preparing";
    break;
  case API_FSM::call_outgoing_setup__waiting_for_remote: 
    os << "call_outgoing_setup__waiting_for_remote";
    break;
  case API_FSM::call_incoming_setup__waiting_for_local: 
    os << "call_incoming_setup__waiting_for_local";
    break;
  case API_FSM::call_p2p_active: 
    os << "call_p2p_active";
    break;
  case API_FSM::call_p2mp_active: 
    os << "call_p2mp_active";
    break;
  case API_FSM::call_p2mp_active__LIJ_present: 
    os << "call_p2mp_active__LIJ_present";
    break;
  case API_FSM::call_release_initiated: 
    os << "call_release_initiated";
    break;
  case API_FSM::call_released: 
    os << "call_released";
    break;

  case API_FSM::service_preparing: 
    os << "service_preparing";
    break;
  case API_FSM::service_register_requested: 
    os << "service_register_requested";
    break;
  case API_FSM::service_deregister_requested: 
    os << "service_deregister_requested";
    break;
  case API_FSM::service_active__no_call_present: 
    os << "service_active__no_call_present";
    break;
  case API_FSM::service_active__call_present: 
    os << "service_active__call_present";
    break;
  case API_FSM::service_released: 
    os << "service_released";
    break;

  case API_FSM::LIJ_preparing:
    os << "LIJ_preparing";
    break;
  case API_FSM::LIJ_requested__waiting_for_remote:
    os << "LIJ_requested__waiting_for_remote";
    break;

  case API_FSM::leaf_preparing:
    os << "leaf_preparing";
    break;
  case API_FSM::leaf_join_request__waiting_for_local:
    os << "leaf_join_request__waiting_for_local";
    break;
  case API_FSM::leaf_addparty_requested__waiting_for_remote:
    os << "leaf_addparty_requested__waiting_for_remote";
    break;
  case API_FSM::leaf_active:
    os << "leaf_active";
    break;
  case API_FSM::leaf_released:
    os << "leaf_released";
    break;

  default: os << "UNKNOWN";
    break;
  }
}

//---------------------------------------------------------
sean_io* API_FSM::get_datapath(void) {
  if (_attr) 
    return _attr->get_datapath();
  return 0;
}

