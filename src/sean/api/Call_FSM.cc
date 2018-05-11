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
static char const _Call_FSM_cc_rcsid_[] =
"$Id: Call_FSM.cc,v 1.9 1998/08/20 21:04:27 talmage Exp $";
#endif
#include <common/cprototypes.h>

#include "ATM_Attributes.h"
#include "Call_FSM.h"
#include "Signalling_API.h"
#include "ATM_Interface.h"
#include "API_constants.h"

#include <sean/cc/sean_Visitor.h>
#include <FW/basics/VisitorType.h>
#include <FW/basics/diag.h>
#include <FW/kernel/SimEvent.h>
#include <codec/q93b_msg/generic_q93b_msg.h>
#include <sean/xti/sean_io.h>

#include <sean/templates/constants.h>

//------------------------------------------------

State* FSM_ATM_Call::Handle(Visitor* v) {   
  VisitorType vt = v->GetType();
  bool approved=false;

  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;

    sean_Visitor::sean_Visitor_Type t = sv->get_op();
    if ((!_attr) && (t!=sean_Visitor::api_call_construction)) {
      diag("api.fsm.call",DIAG_INFO,"Call_FSM being asked to Handle() visitors after user destroys associated ATM_Call\n");
      abort();
    }

    InfoElem ** ies = 0;
    generic_q93b_msg* m = sv->share_message();
    if (m) {
      API_FSM::update_attributes(m);
      ies = m->get_ie_array();
    }

    char name[255];
    
    switch(t) {

      //----------------------------------------------------//
      //                     INDICATIONS                    //
      //----------------------------------------------------//

      /**********************************************/

    case sean_Visitor::api_call_destruction:

      _master_interface->_new_call_id = _id;
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received api_call_destruction sean_Visitor, %d\n",_id);
      assert(_parties_fsms == 1);

      switch(_state) {
      case API_FSM::call_preparing:
      case API_FSM::call_release_initiated:
      case API_FSM::call_released:
	diag("api.fsm.call",DIAG_INFO,"Pushing Call_FSM to state call_released\n");
	change_state_no_callback( API_FSM::call_released );
	mark_FSM_for_deletion();
	Free();
	v->Suicide();
	_attr = 0;
	return 0;
	break;
      default:
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM received api_call_destruction in wrong state\n");
	DIAG("api.fsm.call",DIAG_WARNING,Print_API_FSM_State(_state,cout); cout << endl;);
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      /**********************************************/

    case sean_Visitor::api_leaf_construction: 
      
      _parties_fsms++;
      diag("api.fsm.call",DIAG_INFO,"Call_FSM passes through api_leaf_construction sean_Visitor\n");
      approved = true;
      break;     


    case sean_Visitor::api_leaf_destruction:
      
      _parties_fsms--;

      diag("api.fsm.leaf",DIAG_INFO,"Call_FSM sees api_leaf_destruction sean_Visitor\n");
      if (_parties_fsms==0) {
	diag("api.fsm.leaf",DIAG_INFO,"Call_FSM self destructs since this was the last party\n");
	mark_FSM_for_deletion();
	Free();
	_attr = 0;
	return 0;
      }
      break; 


      /**********************************************/

    case sean_Visitor::api_call_construction:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received api_call_construction sean_Visitor\n");
      if (_virgin) {
	_master_interface->_new_call_fsm = (FSM_ATM_Call*)this;
	_master_interface->_new_call_id = _id;
	sv->set_cid(_id);
	_virgin = false;
      }
      else {
	_master_interface->_new_call_fsm = 0;
	_master_interface->_new_call_id  = NO_CID;	
	diag("api.fsm.call",DIAG_ERROR,"duplicate api_call_construction attempted\n");
      }
      break;

      /**********************************************/

    case sean_Visitor::api_setup_ind_response_to_lsr:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received api_setup_ind_response_to_lsr sean_Visitor\n");
      switch(_state) {
      case API_FSM::LIJ_requested__waiting_for_remote:

	set_crv_in_attributes( m->get_crv() );

	diag("api.fsm.call",DIAG_INFO,"Pushing Call_FSM to state call_incoming_setup__waiting_for_local\n");
	change_state_with_callback( API_FSM::call_incoming_setup__waiting_for_local,0,0 );
	approved=true;
	break; 
      default:
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM received setup_ind in wrong state\n");
	DIAG("api.fsm.call",DIAG_WARNING,Print_API_FSM_State(_state,cout); cout << endl;);
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      /**********************************************/

    case sean_Visitor::leaf_setup_failure_ind:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received leaf_setup_failure_ind sean_Visitor\n");
      switch(_state) {
      case API_FSM::LIJ_requested__waiting_for_remote:
	diag("api.fsm.call",DIAG_INFO,"Pushing Call_FSM to state call_released\n");
	change_state_with_callback( API_FSM::call_released,0,0 );
	approved=true;
	break; 
      default:
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM received leaf_setup_failure_ind in wrong state\n");
	DIAG("api.fsm.call",DIAG_WARNING,Print_API_FSM_State(_state,cout); cout << endl;);
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      /**********************************************/

    case sean_Visitor::leaf_setup_ind:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received leaf_setup_ind sean_Visitor\n");
      switch(_state) {
      case API_FSM::call_p2mp_active: {
	ATM_Leaf* Incoming_Join_Request = new ATM_Leaf( *(Outgoing_ATM_Call*)_attr,
							m,
							((Outgoing_ATM_Call*)_attr)->Get_Next_EPR() );

	diag("api.fsm.service",DIAG_INFO,"Pushing Leaf_FSM to state leaf_join_request__waiting_for_local\n");
	change_state_with_callback( call_p2mp_active__LIJ_present,0,Incoming_Join_Request );
	approved = true;
      }	
      break;

      case API_FSM::call_underspecified__waiting_for_incoming_LIJ:
	{
	  // leaf_setup_ind 'completed' the original 'delayed' setup, which has now been sent out...
	  diag("api.fsm.call",DIAG_INFO,"Incoming leaf_setup_ind completed the delayed setup...\n");
	  diag("api.fsm.call",DIAG_INFO,"Pushing Call_FSM to state call_outgoing_setup__waiting_for_remote\n");
	  change_state_with_callback( API_FSM::call_outgoing_setup__waiting_for_remote,0,0 );
	  approved=true;
	}
	break; 

      default:
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM received leaf_setup_ind in wrong state\n");
	DIAG("api.fsm.call",DIAG_WARNING,Print_API_FSM_State(_state,cout); cout << endl;);
	notify_attributes_of_fsm_failure( -1 );
	break;
	break;
      }
      break;

      /**********************************************/

    case sean_Visitor::call_proc_ind:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received call_proc_ind sean_Visitor\n");
      switch(_state) {
      case API_FSM::call_outgoing_setup__waiting_for_remote: {
	  long vp,vc;
	  ATM_Attributes::result res  = _attr->Get_vpi(vp);
	  ATM_Attributes::result res2 = _attr->Get_vci(vc);
	  diag("api.fsm.call",DIAG_INFO,"Call_FSM received call_proc_ind.\n");
	  if ((res2==ATM_Attributes::success) && (res==ATM_Attributes::success)) {
	    diag("api.fsm.call",DIAG_INFO,"noting VPVC=%d,%d and CRV=%lx\n",vp,vc,sv->get_crv());
	  }
	  else {
	    diag("api.fsm.call",DIAG_WARNING,"error VPVC=??,?? CRV=%lx\n",sv->get_crv());
	  }
	  
	  API_FSM::set_crv_in_attributes( sv->get_crv() );
	  approved=true;
        }
        break;
      default:
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM received call_proc_ind in wrong state\n");
	DIAG("api.fsm.call",DIAG_WARNING,Print_API_FSM_State(_state,cout); cout << endl;);
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      /**********************************************/

    case sean_Visitor::connect_ind:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received connect_ind sean_Visitor\n");
      switch(_state) {
      case API_FSM::call_outgoing_setup__waiting_for_remote:
	if (_attr->Is_P2P())  {
	diag("api.fsm.call",DIAG_INFO,"Pushing Call_FSM to state call_p2p_active\n");
	change_state_with_callback( API_FSM::call_p2p_active,0,0 );
	approved=true;
	}
	if (_attr->Is_P2MP()) {
	  diag("api.fsm.call",DIAG_INFO,"Pushing Call_FSM to state call_p2mp_active\n");
	  change_state_with_callback( API_FSM::call_p2mp_active,0,0 );
	  approved=true;
	}
	break;
      default:
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM received connect_ind in wrong state\n");
	DIAG("api.fsm.call",DIAG_WARNING,Print_API_FSM_State(_state,cout); cout << endl;);
	notify_attributes_of_fsm_failure( -1 );
	break;
	break;
      }
      break;

      /**********************************************/

      case sean_Visitor::release_ind:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received release_ind sean_Visitor\n");
      switch(_state) {
      case API_FSM::call_p2p_active:
      case API_FSM::call_p2mp_active:
      case API_FSM::call_p2mp_active__LIJ_present:
	diag("api.fsm.call",DIAG_INFO,"Pushing Call_FSM to state call_released\n");
	change_state_with_callback( API_FSM::call_released,0,0 );
	approved=true;
	break;
      default:
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM received release_ind in wrong state\n");
	DIAG("api.fsm.call",DIAG_WARNING,Print_API_FSM_State(_state,cout); cout << endl;);
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      /**********************************************/

      case sean_Visitor::release_comp_ind:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received release_comp_ind sean_Visitor\n");
      switch(_state) {
      case API_FSM::call_outgoing_setup__waiting_for_remote:
      case API_FSM::call_release_initiated:
      case API_FSM::call_p2p_active:
      case API_FSM::call_p2mp_active:
      case API_FSM::call_p2mp_active__LIJ_present:
	diag("api.fsm.call",DIAG_INFO,"Pushing Call_FSM to state call_released\n");
	change_state_with_callback( API_FSM::call_released,0,0 );
	break;
      default:
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM received release_comp_ind in wrong state\n");
	DIAG("api.fsm.call",DIAG_WARNING,Print_API_FSM_State(_state,cout); cout << endl;);
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      case sean_Visitor::connect_ack_ind:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received connect_ack_ind sean_Visitor\n");
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for connect_ack_ind\n");
	approved=true;
	break;
      case sean_Visitor::add_party_ind:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received add_party_ind sean_Visitor\n");
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for add_party_ind\n");
	approved=true;
	break;
      case sean_Visitor::add_party_comp_ind:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received add_party_comp_ind sean_Visitor\n");
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for add_party_comp_ind\n");
	approved=true;
	break;
      case sean_Visitor::add_party_ack_ind:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received add_party_ack_ind sean_Visitor\n");
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for add_party_ack_ind\n");
	approved=true;
	break;
      case sean_Visitor::add_party_rej_ind:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received add_party_rej_ind sean_Visitor\n");
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for add_party_rej_ind\n");
	approved=true;
	break;
      case sean_Visitor::drop_party_ind:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received drop_party_ind sean_Visitor\n");
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for drop_party_ind\n");
	approved=true;
	break;
      case sean_Visitor::drop_party_comp_ind:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received drop_party_comp_ind sean_Visitor\n");
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for drop_party_comp_ind\n");
	approved=true;
	break;
      case sean_Visitor::drop_party_ack_ind:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received drop_party_ack_ind sean_Visitor\n");
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for drop_party_ack_ind\n");
	approved=true;
	break;

      case sean_Visitor::status_enq_ind:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received status_enq_ind sean_Visitor\n");
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not defined actions for status_enq_ind\n");
	break;
      case sean_Visitor::status_ind:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received status_ind sean_Visitor\n");
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for status_ind\n");
	break;

      case sean_Visitor::restart_ind:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received restart_ind sean_Visitor\n");
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for restart_ind\n");
	break;
      case sean_Visitor::restart_comp_ind:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received restart_comp_ind sean_Visitor\n");
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for restart_comp_ind\n");
	break;
      case sean_Visitor::restart_ack_ind:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received restart_ack_ind sean_Visitor\n");
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for restart_ack_ind\n");
	break;
	
	
      //----------------------------------------------------//
      //                     REQUESTS                       //
      //----------------------------------------------------//
	
	/**********************************************/
    case sean_Visitor::leaf_setup_req:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received leaf_setup_req sean_Visitor\n");
      switch (_state){
      case API_FSM::LIJ_preparing:
	diag("api.fsm.call",DIAG_INFO,"Pushing Call_FSM to state LIJ_requested__waiting_for_remote\n");
	change_state_with_callback( API_FSM::LIJ_requested__waiting_for_remote,0,0 );
	approved=true;
	break; 

       case API_FSM::call_preparing:
	diag("api.fsm.call",DIAG_INFO,"Pushing Call_FSM to state LIJ_requested__waiting_for_remote\n");
	change_state_with_callback( API_FSM::LIJ_requested__waiting_for_remote,0,0 );
	approved=true;
	break; 

      default: 
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM received leaf_setup_req in wrong state\n");
	DIAG("api.fsm.call",DIAG_WARNING,Print_API_FSM_State(_state,cout); cout << endl;);
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;


      /**********************************************/
    case sean_Visitor::leaf_setup_failure_req:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received leaf_setup_failure_req sean_Visitor\n");
      switch (_state){
      case API_FSM::call_p2mp_active:
      case API_FSM::call_p2mp_active__LIJ_present:
	diag("api.fsm.call",DIAG_INFO,"Leaving call state unchanged\n");
	approved = true;
	break;
      default: 
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM received leaf_setup_failure_req in wrong state\n");
	break;
      };
      break;

      /**********************************************/
    case sean_Visitor::setup_req:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received setup_req sean_Visitor\n");
	switch (_state){

	case API_FSM::call_underspecified__waiting_for_incoming_LIJ:
	case API_FSM::call_preparing:
	  {
	    Addr* dummy_addr = newAddr( DELAYED_SETUP_ADDRESS_TOKEN );
	    Addr* dest = ((ie_called_party_num*)ies[InfoElem::ie_called_party_num_ix]) -> get_addr();
	    if ( compare(dest,dummy_addr) == 0 ) {

	      // This is the underspecified Establish(), so we will wait for the first
	      // leaf_setup_ind and use that to 'complete' the 'delayed' setup...

	      diag("api.fsm.call",DIAG_INFO,"Underspecified Establish() observed...\n");
	      diag("api.fsm.call",DIAG_INFO,"Pushing Call_FSM state to call_underspecified__waiting_for_incoming_LIJ");
	      change_state_with_callback( API_FSM::call_underspecified__waiting_for_incoming_LIJ,0,0 );
	    }
	    else {
	      diag("api.fsm.call",DIAG_INFO,"Pushing Call_FSM to state call_outgoing_setup__waiting_for_remote\n");
	      change_state_with_callback( API_FSM::call_outgoing_setup__waiting_for_remote,0,0 );
	    }
	    approved=true;
	  }
	break; 

      default: 
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM received setup_req in wrong state\n");
	DIAG("api.fsm.call",DIAG_WARNING,Print_API_FSM_State(_state,cout); cout << endl;);
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      /**********************************************/
      case sean_Visitor::connect_req:
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received connect_req sean_Visitor\n");
      switch (_state){
      case API_FSM::call_incoming_setup__waiting_for_local: 
	if (_attr->Is_P2P())  {
	  diag("api.fsm.call",DIAG_INFO,"Pushing Call_FSM to state call_p2p_active\n");
	  change_state_with_callback( API_FSM::call_p2p_active,0,0 );
	  approved=true;
	}
	if (_attr->Is_P2MP()) {
	  diag("api.fsm.call",DIAG_INFO,"Pushing Call_FSM to state call_p2mp_active\n");
	  change_state_with_callback( API_FSM::call_p2mp_active,0,0 );
	  approved=true;
	}
	break; 
      default: 
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM received connect_req in wrong state\n");
	DIAG("api.fsm.call",DIAG_WARNING,Print_API_FSM_State(_state,cout); cout << endl;);
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      /**********************************************/
      case sean_Visitor::release_req: 
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received release_req sean_Visitor\n");
      switch (_state){
      case API_FSM::call_p2p_active: 
      case API_FSM::call_p2mp_active: 
      case API_FSM::call_p2mp_active__LIJ_present:
	diag("api.fsm.call",DIAG_INFO,"Pushing Call_FSM to state call_release_initiated\n");
	change_state_with_callback( API_FSM::call_release_initiated,0,0 );
	approved=true;
	break;
      default: 
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM received release_req in wrong state\n");
	DIAG("api.fsm.call",DIAG_WARNING,Print_API_FSM_State(_state,cout); cout << endl;);
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

      /**********************************************/
      case sean_Visitor::release_comp_req: 
	diag("api.fsm.call",DIAG_INFO,"Call_FSM received release_comp_req sean_Visitor\n");
      switch (_state){
      case API_FSM::call_outgoing_setup__waiting_for_remote:
      case API_FSM::call_incoming_setup__waiting_for_local:
      case API_FSM::call_released:
	diag("api.fsm.call",DIAG_INFO,"Pushing Call_FSM to state call_released\n");
	change_state_with_callback( API_FSM::call_released,0,0 );
	approved=true;
	break;
      default: 
	diag("api.fsm.call",DIAG_WARNING,"Call_FSM received release_comp_req in wrong state\n");
	DIAG("api.fsm.call",DIAG_WARNING,Print_API_FSM_State(_state,cout); cout << endl;);
	notify_attributes_of_fsm_failure( -1 );
	break;
      }
      break;

    case sean_Visitor::call_proc_req:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received call_proc_req sean_Visitor\n");
      diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not defined actions for call_proc_req\n");
      approved = true;
      break;
    case sean_Visitor::connect_ack_req:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received connect_ack_req sean_Visitor\n");
      diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for connect_ack_req\n");
      approved = true;
      break;
    case sean_Visitor::add_party_req:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received add_party_req sean_Visitor\n");
      diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for add_party_req\n");
      approved = true;
      break;
    case sean_Visitor::add_party_comp_req:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received add_party_comp_req sean_Visitor\n");
      diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for add_party_comp_req\n");
      approved = true;
      break;
    case sean_Visitor::add_party_ack_req:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received add_party_ack_req sean_Visitor\n");
      diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for add_party_ack_req\n");
      approved = true;
      break;
    case sean_Visitor::add_party_rej_req:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received add_party_rej_req sean_Visitor\n");
      diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for add_party_rej_req\n");
      approved = true;
      break;
    case sean_Visitor::drop_party_req:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received drop_party_req sean_Visitor\n");
      diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for drop_party_req\n");
      approved = true;
      break;
    case sean_Visitor::drop_party_comp_req:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received drop_party_comp_req sean_Visitor\n");
      diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for drop_party_comp_req\n");
      approved = true;
      break;
    case sean_Visitor::drop_party_ack_req:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received drop_party_ack_req sean_Visitor\n");
      diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for drop_party_ack_req\n");
      approved = true;
      break;


    case sean_Visitor::status_enq_req:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received status_enq_req sean_Visitor\n");
      diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not defined actions for status_enq_req\n");
      break;
    case sean_Visitor::status_req:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received status_req sean_Visitor\n");
      diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for status_req\n");
      break;
    case sean_Visitor::restart_req:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received restart_req sean_Visitor\n");
      diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for restart_req\n");
      break;
    case sean_Visitor::restart_resp:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received restart_resp sean_Visitor\n");
      diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for restart_comp_req\n");
      break;
    case sean_Visitor::restart_ack_req:
      diag("api.fsm.call",DIAG_INFO,"Call_FSM received restart_ack_req sean_Visitor\n");
      diag("api.fsm.call",DIAG_WARNING,"Call_FSM has not define actions for restart_ack_req\n");
      break;

    case sean_Visitor::raw_simulation_pdu: {
      diag("api.fsm.call",DIAG_INFO,"Call_FSM sees ATM_PDU\n");
      sean_io* associated_datapath = get_datapath();
      if (associated_datapath) {
	Buffer* buf = sv->take_buffer();
	assert(buf);
	associated_datapath->enqueue_incoming_data_buffer( buf );
	diag("api.fsm.call",DIAG_INFO,"Appended the ATM_PDU into the call's associated sean_io object.\n");
      }
      //
      // since approved == false, the if(approved) following this
      // switch() will kill sv for us.
      //
      //      sv->Suicide();

    } break;

    default:
      diag("api.fsm.call",DIAG_ERROR,"Call_FSM has not defined actions for unexpected sean_Visitor\n");
      break;
    }

    if (approved) {
      PassThru(sv); sv=0;
    }
    else {
      sv->Suicide(); sv=0;
    }
  }
  else v->Suicide();

  return this;
}

void FSM_ATM_Call::Interrupt(class SimEvent *e) { 
}

FSM_ATM_Call::FSM_ATM_Call(int id, ATM_Interface* master) : API_FSM(id) { 
  _state = API_FSM::call_preparing;
  _master_interface = master;
  _virgin = true;
  _parties_fsms = 0;
}

//------------------------------------------------

FSM_ATM_Call::~FSM_ATM_Call() { 
  diag("api.fsm.call",DIAG_INFO,"Beginning dtor associated with the call FSM.\n");  
  _master_interface->_new_call_fsm = 0;
  _master_interface->_new_call_id  = _id;
  diag("api.fsm.call",DIAG_INFO,"Done with dtor associated with the call FSM.\n");  
}

//------------------------------------------------

void FSM_ATM_Call::set_queue_to_empty(void) { 
  if (_state==API_FSM::call_p2mp_active__LIJ_present)
    _state = API_FSM::call_p2mp_active;
}

//------------------------------------------------

void FSM_ATM_Call::Print(void) const {
  cout << "Call FSM\n";
}

//------------------------------------------------







