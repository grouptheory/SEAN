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
static char const _Service_FSM_cc_rcsid_[] =
"$Id: Service_FSM.cc,v 1.2 1998/08/06 04:03:26 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "ATM_Attributes.h"
#include "Service_FSM.h"
#include "Signalling_API.h"
#include "ATM_Interface.h"

#include <sean/cc/sean_Visitor.h>
#include <FW/basics/VisitorType.h>
#include <FW/kernel/SimEvent.h>

#include <sean/templates/constants.h>

//------------------------------------------------

State* FSM_ATM_Service::Handle(Visitor* v) {   
  VisitorType vt = v->GetType();
  bool approved = false;
  
  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;

    sean_Visitor::sean_Visitor_Type t = sv->get_op();
    if ((!_attr) && (t!=sean_Visitor::api_service_construction)) {
      diag("api.fsm.service",DIAG_INFO,"Service_FSM being asked to Handle() visitors after user destroys associated ATM_Service\n");
      abort();
    }

    generic_q93b_msg* m = sv->share_message();
    if (m) {
      API_FSM::update_attributes(m);
    }

    switch(t) {

      //----------------------------------------------------//
      //                     INDICATIONS                    //
      //----------------------------------------------------//


      /**********************************************/

    case sean_Visitor::service_register_accept:     
      diag("api.fsm.service",DIAG_INFO,"Service_FSM received service_register_accept sean_Visitor\n");
      switch(_state) {
      case API_FSM::service_register_requested:
	diag("api.fsm.service",DIAG_INFO,"Pushing Service_FSM to state service_active__no_call_present\n");
	change_state_with_callback( API_FSM::service_active__no_call_present,0,0 );
	approved = true;
	break;
      default:
	diag("api.fsm.service",DIAG_WARNING,"Service_FSM received service_register_accept in wrong state\n");
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      /**********************************************/

    case sean_Visitor::service_register_deny:    
      diag("api.fsm.service",DIAG_INFO,"Service_FSM received service_register_deny sean_Visitor\n");
      switch(_state) {
      case API_FSM::service_register_requested:
	diag("api.fsm.service",DIAG_INFO,"Pushing Service_FSM to state service_released\n");
	change_state_with_callback( API_FSM::service_released,0,0 );
	approved = true;
	break;
      default:
	diag("api.fsm.service",DIAG_WARNING,"Service_FSM received service_register_deny in wrong state\n");
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      /**********************************************/

    case sean_Visitor::service_dereg_accept:  
      diag("api.fsm.service",DIAG_INFO,"Service_FSM received service_dereg_accept sean_Visitor\n");
      switch(_state) {
      case API_FSM::service_deregister_requested:
	diag("api.fsm.service",DIAG_INFO,"Pushing Service_FSM to state service_released\n");
	change_state_with_callback( API_FSM::service_released,0,0 );
	approved = true;
	if (is_FSM_marked_for_deletion()) {
	  Free();
	  return 0;
	}
	break;
      default:
	diag("api.fsm.service",DIAG_WARNING,"Service_FSM received service_dereg_accept in wrong state\n");
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      /**********************************************/

    case sean_Visitor::service_dereg_deny:
      diag("api.fsm.service",DIAG_INFO,"Service_FSM received service_dereg_deny sean_Visitor\n");
      switch(_state) {
      case API_FSM::service_deregister_requested:
	diag("api.fsm.service",DIAG_WARNING,"Deregistration of Service_FSM was rejected\n");
	approved = true;
	break;
      default:
	diag("api.fsm.service",DIAG_WARNING,"Service_FSM received service_dereg_deny in wrong state\n");
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      /**********************************************/

    case sean_Visitor::setup_ind:
      diag("api.fsm.service",DIAG_INFO,"Service_FSM received setup_ind sean_Visitor\n");
      switch (_state){
      case API_FSM::service_active__no_call_present:
      case API_FSM::service_active__call_present: {
	Incoming_ATM_Call* call = new Incoming_ATM_Call(_attr->Get_interface(),
							sv->take_message(),
							sv->get_cid());
	diag("api.fsm.service",DIAG_INFO,"Pushing Service_FSM to state service_active__call_present\n");
	change_state_with_callback( service_active__call_present,call,0 );
	approved = true;
      }	break;
      default:
	diag("api.fsm.service",DIAG_WARNING,"Service_FSM received setup_ind in wrong state\n");
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      //----------------------------------------------------//
      //                     REQUESTS                       //
      //----------------------------------------------------//

      /**********************************************/

    case sean_Visitor::api_service_destruction:

      _master_interface->_new_service_id = _id;
      diag("api.fsm.service",DIAG_INFO,"Service_FSM received api_service_destruction sean_Visitor, %d\n",_id);
      switch (_state){
      case API_FSM::service_active__no_call_present:
      case API_FSM::service_active__call_present: 
      case API_FSM::service_register_requested: 
	{
	  ATM_Service* serv = ((ATM_Service*)(_attr));
	  serv->Deregister();
	}
	// fall through

      case API_FSM::service_deregister_requested:
	mark_FSM_for_deletion();
	diag("api.fsm.service",DIAG_INFO,"Pushing Service_FSM to state service_released\n");
	change_state_with_callback( API_FSM::service_released,0,0 );
	approved = true;
	break;

      case API_FSM::service_preparing:
      case API_FSM::service_released:
	mark_FSM_for_deletion();
	Free();
	approved = true;	
	break;
      default:
	diag("api.fsm.service",DIAG_WARNING,"Service_FSM received api_service_destruction in wrong state\n");
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      /**********************************************/

    case sean_Visitor::api_service_construction:
      diag("api.fsm.service",DIAG_INFO,"Service_FSM received api_service_construction sean_Visitor\n");
      if (_virgin) {
	_master_interface->_new_service_fsm = (FSM_ATM_Service*)this;
	_master_interface->_new_service_id = _id;
	sv->set_sid(_id);
	_virgin = false;
      }
      else {
	_master_interface->_new_service_fsm = 0;
	_master_interface->_new_service_id =  NO_SID;
      }
      break;
      
      /**********************************************/

    case sean_Visitor::service_register_req:
      diag("api.fsm.service",DIAG_INFO,"Service_FSM received service_register_req sean_Visitor\n");
      switch (_state){
      case API_FSM::service_preparing:
	diag("api.fsm.service",DIAG_INFO,"Pushing Service_FSM to state service_register_requested\n");
	change_state_with_callback( API_FSM::service_register_requested,0,0 );
	approved = true;
	break; 
      default: 
	diag("api.fsm.service",DIAG_WARNING,"Service_FSM received service_register_req in wrong state\n");
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break; 

      /**********************************************/

    case sean_Visitor::service_dereg_req:
      diag("api.fsm.service",DIAG_INFO,"Service_FSM received service_dereg_req sean_Visitor\n");
      switch (_state){
      case API_FSM::service_active__no_call_present:
      case API_FSM::service_active__call_present: 
	diag("api.fsm.service",DIAG_INFO,"Pushing Service_FSM to state service_deregister_requested\n");
	change_state_with_callback( API_FSM::service_deregister_requested,0,0 );
	approved = true;
	break;
      default:
	diag("api.fsm.service",DIAG_WARNING,"Service_FSM received service_dereg_req in wrong state\n");
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

    default:
      diag("api.fsm.service",DIAG_ERROR,"Service_FSM has not defined actions for unexpected sean_Visitor\n");
      break;
    }
  }

  if (approved) {
    PassThru(v);
  }
  else v->Suicide();

  return this;
}

void FSM_ATM_Service::Interrupt(class SimEvent *e) { 
}

bool FSM_ATM_Service::Approve_API_Request(API_FSM::api_fsm_state transition_to) {
  bool result=false;
  switch (transition_to){

  case API_FSM::service_register_requested:
    switch (_state){
    case API_FSM::service_preparing: result=true;
      break; 
    default: 
      diag("api.fsm.service",DIAG_ERROR,"Denied API request to move to service state service_register_requested\n");
      break;
    }
  break;     

  case API_FSM::service_active__no_call_present:
    switch (_state){
    case API_FSM::service_register_requested: result=true;
      break; 
    default: 
      diag("api.fsm.service",DIAG_ERROR,"Denied API request to move to service state service_active__no_call_present\n");
      break;
    }
  break; 

  case API_FSM::service_deregister_requested: 
    switch (_state){
    case API_FSM::service_active__no_call_present:
    case API_FSM::service_active__call_present: result=true;
      break;
    default: 
      diag("api.fsm.service",DIAG_ERROR,"Denied API request to move to service state service_deregister_requested\n");
      break;
    }
    break;

  default: 
    diag("api.fsm.service",DIAG_ERROR,"Denied API request to move to unexpected service state\n");
    break;
  }
  return result;
}

//------------------------------------------------

FSM_ATM_Service::FSM_ATM_Service(int id, ATM_Interface* master) : API_FSM(id) { 
  _state = API_FSM::service_preparing;
  _master_interface = master;
  _virgin = true;
}

//------------------------------------------------

FSM_ATM_Service::~FSM_ATM_Service() { 
  _master_interface->_new_service_fsm = 0;
  _master_interface->_new_service_id = _id;
}

//------------------------------------------------

void FSM_ATM_Service::set_queue_to_empty(void) { 
  if (_state==API_FSM::service_active__call_present)
    _state = service_active__no_call_present;
}

//------------------------------------------------

void FSM_ATM_Service::Print(void) const {
  cout << "Service FSM\n";
}











