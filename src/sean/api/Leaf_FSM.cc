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
static char const _Leaf_FSM_cc_rcsid_[] =
"$Id: Leaf_FSM.cc,v 1.4 1998/08/10 11:54:47 bilal Exp $";
#endif
#include <common/cprototypes.h>
#include "ATM_Attributes.h"
#include "Leaf_FSM.h"
#include "Signalling_API.h"
#include "ATM_Interface.h"

#include <sean/cc/sean_Visitor.h>
#include <FW/basics/VisitorType.h>
#include <FW/kernel/SimEvent.h>

#include <sean/templates/constants.h>

//------------------------------------------------

State* FSM_ATM_Leaf::Handle(Visitor* v) { 
  VisitorType vt = v->GetType();
  bool approved = false;

  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;
    sean_Visitor::sean_Visitor_Type t = sv->get_op();
    if ((!_attr) && (t!=sean_Visitor::api_leaf_construction)) {
      diag("api.fsm.leaf",DIAG_INFO,"Leaf_FSM being asked to Handle() visitors after user destroys associated ATM_Leaf\n");
      abort();
    }

    switch(t) {

      //----------------------------------------------------//
      //                     INDICATIONS                    //
      //----------------------------------------------------//

      /**********************************************/

      /**********************************************/

    case sean_Visitor::add_party_ack_ind:
      diag("api.fsm.leaf",DIAG_INFO,"Leaf_FSM received add_party_ack_ind sean_Visitor\n");
      switch (_state){
      case API_FSM::leaf_addparty_requested__waiting_for_remote: 
	diag("api.fsm.leaf",DIAG_INFO,"Pushing Leaf_FSM to state leaf_active\n");
	change_state_with_callback( API_FSM::leaf_active,0,0 );
	approved = true;
	break; 
      default: 
	diag("api.fsm.leaf",DIAG_WARNING,"Leaf_FSM received add_party_ack_ind in wrong state\n");
	DIAG("api.fsm.leaf",DIAG_WARNING, {Print_API_FSM_State(_state,cout); cout << endl;});

	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      /**********************************************/

    case sean_Visitor::add_party_rej_ind:
      diag("api.fsm.leaf",DIAG_INFO,"Leaf_FSM received add_party_rej_ind sean_Visitor\n");
      switch (_state){
      case API_FSM::leaf_addparty_requested__waiting_for_remote: 
	diag("api.fsm.leaf",DIAG_INFO,"Pushing Leaf_FSM to state leaf_released\n");
	change_state_with_callback( API_FSM::leaf_released,0,0 );
	approved = true;
	break; 
      default: 
	diag("api.fsm.leaf",DIAG_WARNING,"Leaf_FSM received add_party_rej_ind in wrong state\n");
	DIAG("api.fsm.leaf",DIAG_WARNING, {Print_API_FSM_State(_state,cout); cout << endl;});
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      /**********************************************/

    case sean_Visitor::drop_party_ind:
      diag("api.fsm.leaf",DIAG_INFO,"Leaf_FSM received drop_party_ind sean_Visitor\n");
      switch (_state){
      case API_FSM::leaf_addparty_requested__waiting_for_remote: 
      case API_FSM::leaf_active:  
	diag("api.fsm.leaf",DIAG_INFO,"Pushing Leaf_FSM to state leaf_released\n");
	change_state_with_callback( API_FSM::leaf_released,0,0 );
	approved = true;
	break; 
      default: 
	diag("api.fsm.leaf",DIAG_WARNING,"Leaf_FSM received drop_party_ind in wrong state\n");
	DIAG("api.fsm.leaf",DIAG_WARNING, {Print_API_FSM_State(_state,cout); cout << endl;});
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

     /**********************************************/

    case sean_Visitor::drop_party_ack_ind:
      diag("api.fsm.leaf",DIAG_INFO,"Leaf_FSM received drop_party_ack_ind sean_Visitor\n");
      switch (_state){
      case API_FSM::leaf_dropparty_requested__waiting_for_remote: 
	diag("api.fsm.leaf",DIAG_INFO,"Pushing Leaf_FSM to state leaf_released\n");
	change_state_with_callback( API_FSM::leaf_released,0,0 );
	approved = true;
	break; 
      default: 
	diag("api.fsm.leaf",DIAG_WARNING,"Leaf_FSM received drop_party_ack_ind in wrong state\n");
	DIAG("api.fsm.leaf",DIAG_WARNING, {Print_API_FSM_State(_state,cout); cout << endl;});
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

     /**********************************************/

    case sean_Visitor::leaf_setup_ind:
      diag("api.fsm.leaf",DIAG_INFO,"Leaf_FSM received leaf_setup_ind sean_Visitor\n");
      switch (_state){
      case API_FSM::leaf_preparing:
	diag("api.fsm.leaf",DIAG_INFO,"Pushing Leaf_FSM to state leaf_LIJ__waiting_for_local\n");
	change_state_with_callback( API_FSM::leaf_join_request__waiting_for_local,0,0 );
	approved = true;
	break; 
      default: 
	diag("api.fsm.leaf",DIAG_WARNING,"Leaf_FSM received leaf_setup_ind in wrong state\n");
	DIAG("api.fsm.leaf",DIAG_WARNING, {Print_API_FSM_State(_state,cout); cout << endl;});
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;


      //----------------------------------------------------//
      //                     REQUESTS                       //
      //----------------------------------------------------//

      /**********************************************/

    case sean_Visitor::api_leaf_construction:
      diag("api.fsm.leaf",DIAG_INFO,"Leaf_FSM received api_leaf_construction sean_Visitor\n");
      if (_virgin) {
	sv->set_lid(_id);
	_virgin = false;
      }
      else {
	((ATM_Leaf*)_attr)->_owning_call->Get_interface()._new_leaf_fsm = 0;
	((ATM_Leaf*)_attr)->_owning_call->Get_interface()._new_leaf_id =  NO_LID;
	diag("api.leaf.call",DIAG_ERROR,"duplicate api_leaf_construction attempted\n");
      }
      break; 


      /**********************************************/

    case sean_Visitor::api_leaf_destruction:
      diag("api.fsm.leaf",DIAG_INFO,"Leaf_FSM received api_leaf_destruction sean_Visitor\n");
      switch (_state){
      case API_FSM::leaf_released:
	mark_FSM_for_deletion();
	Free();
	_attr = 0;
	return 0;
	break;
	
      default: 
	diag("api.fsm.leaf",DIAG_WARNING,"Leaf_FSM received api_leaf_destruction in wrong state\n");
	DIAG("api.fsm.leaf",DIAG_WARNING, {Print_API_FSM_State(_state,cout); cout << endl;});
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break; 


      /**********************************************/

    case sean_Visitor::add_party_req:
      diag("api.fsm.leaf",DIAG_INFO,"Leaf_FSM received add_party_req sean_Visitor\n");
      switch (_state){
      case API_FSM::leaf_preparing:
	diag("api.fsm.leaf",DIAG_INFO,"Pushing Leaf_FSM to state leaf_addparty_requested__waiting_for_remote\n");
	change_state_with_callback( API_FSM::leaf_addparty_requested__waiting_for_remote,0,0 );
	approved = true;
	break;
      case API_FSM::leaf_join_request__waiting_for_local:
	diag("api.fsm.leaf",DIAG_INFO,"Pushing Leaf_FSM to state leaf_addparty_requested__waiting_for_remote\n");
	change_state_with_callback( API_FSM::leaf_addparty_requested__waiting_for_remote,0,0 );
	approved = true;
	break;
      default: 
	diag("api.fsm.leaf",DIAG_WARNING,"Leaf_FSM received add_party_req in wrong state\n");
	DIAG("api.fsm.leaf",DIAG_WARNING, {Print_API_FSM_State(_state,cout); cout << endl;});
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break; 

      /**********************************************/

    case sean_Visitor::drop_party_req:
      diag("api.fsm.leaf",DIAG_INFO,"Leaf_FSM received drop_party_req sean_Visitor\n");
      switch (_state){
      case API_FSM::leaf_addparty_requested__waiting_for_remote:
      case API_FSM::leaf_join_request__waiting_for_local:
      case API_FSM::leaf_active: 
	diag("api.fsm.leaf",DIAG_INFO,"Pushing Leaf_FSM to state leaf_dropparty_requested__waiting_for_remote\n");
	change_state_with_callback( API_FSM::leaf_dropparty_requested__waiting_for_remote,0,0 );
	approved = true;
	break;
      default: 
	diag("api.fsm.leaf",DIAG_WARNING,"Leaf_FSM received drop_party_req in wrong state\n");
	DIAG("api.fsm.leaf",DIAG_WARNING, {Print_API_FSM_State(_state,cout); cout << endl;});
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break; 

    default:
      diag("api.fsm.leaf",DIAG_ERROR,"Leaf_FSM ignores the unexpected sean_Visitor\n");
      approved=true; // pass through
      break;
    }
  }

  if (approved) PassThru(v);
  else v->Suicide();
  return this;
}


//------------------------------------------------

void FSM_ATM_Leaf::Interrupt(class SimEvent *e) { 
}

//------------------------------------------------

FSM_ATM_Leaf::FSM_ATM_Leaf(int id) : API_FSM(id) { 
  _state = API_FSM::leaf_preparing;
  _virgin = true;
}

//------------------------------------------------

FSM_ATM_Leaf::~FSM_ATM_Leaf() { 
}

//------------------------------------------------

void FSM_ATM_Leaf::set_queue_to_empty(void) { }

//------------------------------------------------

void FSM_ATM_Leaf::Print(void) const {
  cout << "Leaf FSM\n";
}

