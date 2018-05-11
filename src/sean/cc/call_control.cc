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
static char const _call_control_cc_rcsid_[] =
"$Id: call_control.cc,v 1.27 1999/04/02 20:37:56 marsh Exp $";
#endif
#include <common/cprototypes.h>

#include "call_control.h"
#include "VPVCSimEvent.h"
#include "sean_Visitor.h"
#include "info.h"

#include <FW/basics/diag.h>
#include <FW/basics/Log.h>
#include <FW/kernel/SimEvent.h>

#include <codec/uni_ie/ie_base.h>
#include <codec/uni_ie/cause.h>
#include <codec/uni_ie/conn_id.h>
#include <codec/uni_ie/UNI40_leaf.h>
#include <codec/uni_ie/broadband_send_comp_ind.h>
#include <codec/uni_ie/end_pt_ref.h>
#include <codec/uni_ie/called_party_num.h>
#include <codec/uni_ie/calling_party_num.h>
#include <codec/uni_ie/addr.h>

#include <codec/q93b_msg/Buffer.h>
#include <codec/q93b_msg/generic_q93b_msg.h>
#include <codec/q93b_msg/release.h>
#include <codec/q93b_msg/release_comp.h>
#include <codec/q93b_msg/leaf_setup.h>
#include <codec/q93b_msg/setup.h>
#include <codec/q93b_msg/add_party_ack.h>
#include <codec/q93b_msg/add_party_rej.h>
#include <codec/q93b_msg/drop_party.h>
#include <codec/q93b_msg/drop_party_ack.h>

#include <sean/ipc/IPC_Visitors.h>
#include <sean/ipc/tcp_ipc.h>
#include <sean/daemon/ccd_config.h>
#include <sean/templates/constants.h>


#define _RELATE_DEBUG_ 1

//-----------------------------------------------------------------------
const VisitorType * call_control::_sean_vistype     = 0;
const VisitorType * call_control::_ipc_data_vistype = 0;
const VisitorType * call_control::_ipc_sig_vistype  = 0;

//-----------------------------------------------------------------------
call_control::call_control(int s, int p) :
  State( true ), // queueing
  _location(s), 
  _port(p), 
  _next_available_cref(2), 
  _no_bind(false),
  _next_sequence_number(1)
{
  if (!_sean_vistype)
    _sean_vistype = QueryRegistry(SEAN_VISITOR_NAME);
  if (!_ipc_data_vistype)
    _ipc_data_vistype = QueryRegistry(IPC_DATA_VISITOR_NAME);
  if (!_ipc_sig_vistype)
    _ipc_sig_vistype = QueryRegistry(IPC_SIGNALLING_VISITOR_NAME);
}

//-----------------------------------------------------------------------
// This function gets called only at USER side stacks
// and serves to transform IPC_DataVisitors into sean_Visitors
//-----------------------------------------------------------------------
State * call_control::ipc_data_visitor_from_A(IPC_DataVisitor * dv) 
{
  int len = dv->get_length();
  Buffer * buf = dv->take_packet_and_zero_length();

  sean_Visitor * sv = new sean_Visitor(buf);  // build the sean visitor

  buf->ShrinkHeader( 4*sizeof(int) );          // strip the buffer

  // transfer the application identifier
  const abstract_local_id * id = dv->share_local_id();
  sv->set_shared_app_id( id );
  
  if (sv->Valid()) {
    dv->Suicide();
    sean_visitor_from_A(sv);  // dispatch
  } 
  else {
    diag("cc_layer.call_control", DIAG_WARNING, "%s: received invalid IPC Data.\n", 
	 OwnerName());

    dv->Suicide();
    sv->Suicide();
  }
  return this;
}

//-----------------------------------------------------------------------
// This function gets called only at USER side stacks
// and serves to react to connectivity changes, in particular
// when instances of the SEAN API libraries connect or disconnect
// from their local signalling daemon
//-----------------------------------------------------------------------
State * call_control::ipc_signalling_visitor_from_A(IPC_SignallingVisitor * iv) 
{
  assert(_location==USER_SIDE);

  IPC_SignallingVisitor::ipc_op op = iv->get_IPC_opcode();
  switch (op) {
    /*** Another API library connects to the signalling daemon ***/
    case IPC_SignallingVisitor::establish_by_peer_op: {

      const abstract_local_id * id = iv->share_local_id();
      DIAG("cc_layer.call_control",DIAG_INFO, \
	   printf("%s: New client API connecting on <", OwnerName()); \
	   id->Print(cout); \
	   printf(">\n"););

      // initialize the callids dictionary
      if (_callids.lookup(id) != 0) {
	cout << "Current size of callids dictionary: " << _callids.size() << endl;
      }

      assert(_callids.lookup(id) == 0);
      _callids.insert(id, new list<call_info *>);
      _app_alive.insert(id,1);
      diag("cc_layer.call_control",DIAG_INFO,"%s: Added callinfo structures for new client\n", OwnerName());
      
      assert(_servids.lookup(id) == 0);
      _servids.insert(id, new list<serv_info *>);
      diag("cc_layer.call_control",DIAG_INFO,"%s: Added a servinfo structures for new client\n", OwnerName());
    } break;
  
    /*** An API library disconnects from the signalling daemon ***/
    case IPC_SignallingVisitor::release_by_peer_op: {

      const abstract_local_id* id = iv->share_local_id();
      dic_item dit = _app_alive.lookup(id);
      assert( dit );
      _app_alive.change_inf(dit, 0);
      diag("cc_layer.call_control",DIAG_INFO,"%s: Client API departed from <%x>."
	   "Commencing cleanup procedures.\n", OwnerName(), id);

      bool did_something=false;

      diag("cc_layer.call_control",DIAG_INFO,"%s: Now releasing all calls of departed client.\n", 
	   OwnerName());
      dic_item di;
      if (di = _callids.lookup(id)) {
	list<call_info *> * lst = _callids.inf(di);
	list_item li;
	if (!lst->empty()) {
	  // This was changed because it seems as though unrelate is being called
	  //  during this loop, which causes the deletion of a list_item, which
	  //  will break forall_items.  - mountcas 8-24-98
	  list_item nli;
	  for ( li = lst->first(); li ; li = nli) {
	    nli = lst->next(li);

	    // forall_items(li, *lst) {
	    call_info & ci = *(lst->inf(li));
	    did_something= true;
	    sean_Visitor* killv;
	    if (ci.is_active()) {
	      killv = new sean_Visitor(sean_Visitor::release_req);

	      // FIX THIS: If the call is P2MP we must drop all the 
	      // parties before we send this release, or the assertion
	      // of unique path will fail in the router... BK

	    } else {
	      killv = new sean_Visitor(sean_Visitor::release_comp_req);
	    }
	    killv->set_cid(ci._cid);
	    killv->set_crv(ci._crv);
	    killv->set_shared_app_id(id);
	    bool through_this_cc = true;     // self-stimulating
	    if (ci.is_active()) {
	      diag("cc_layer.call_control",DIAG_INFO,"%s: Sending <release_req> for cid=%d,cref=%lx.\n", 
		   OwnerName(), ci._cid, ci._crv);
	      Failure_SendRelease(killv,ie_cause::normal_call_clearing, through_this_cc);
	    } 
	    else 
	      {
		diag("cc_layer.call_control",DIAG_INFO,"%s: Sending <release_comp_req> for cid=%d,cref=%lx.\n", 
		     OwnerName(), ci._cid, ci._crv);
		Failure_SendReleaseComplete(killv,ie_cause::normal_call_clearing, through_this_cc);
	      }
	  }
	}
      }
      diag("cc_layer.call_control",DIAG_INFO,"%s: Done releasing all calls of departed client.\n", 
	   OwnerName());

      diag("cc_layer.call_control",DIAG_INFO,"%s: Now releasing all services of departed client.\n", 
	   OwnerName());
      if (di = _servids.lookup(id)) {
	list<serv_info *> *lst = _servids.inf(di);
	list_item li;
	if (!lst->empty()) {
	  forall_items(li, *lst) {
	    serv_info & si = *(lst->inf(li));
	    sean_Visitor* killv = new sean_Visitor(sean_Visitor::service_dereg_req);
	    killv->set_sid(si._sid);
	    killv->set_shared_app_id(id);
	    diag("cc_layer.call_control",DIAG_INFO,"%s: Sending <service_dereg_req> for sid=%d.\n", OwnerName(), si._sid);	    
	    did_something= true;
	    sean_visitor_from_A(killv);     // self-stimulating
	  }
	}
      }
      diag("cc_layer.call_control",DIAG_INFO,"%s: Done releasing all services belonging to the departed client.\n", 
	   OwnerName());
      
      // if we did nothing, then we must check explicitly here
      if (!did_something)
	attempt_final_app_death_cleanup(id);
	
    } break;

    default:
      break;
  }
  iv->Suicide();
  return this;
}

//-----------------------------------------------------------------------
// This function reacts to all the sean Visitors 
// that arrive at the call control by way of the UNI.
//-----------------------------------------------------------------------
State * call_control::sean_visitor_from_B(sean_Visitor * sv) 
{
  const abstract_local_id          * id     = sv->get_shared_app_id();
  sean_Visitor::sean_Visitor_Type    t      = sv->get_op();
  InfoElem                        ** ies    = 0;
  ie_conn_id                       * cid_ie = 0;
  int epr                                   = NO_EPR;

  generic_q93b_msg                 * msg    = sv->share_message();
  if (msg) {
    ies = msg->get_ie_array();
    if(ies[InfoElem::ie_end_pt_ref_ix])  // P2MP indeed.
      {
	epr = ((ie_end_pt_ref *)ies[InfoElem::ie_end_pt_ref_ix])->get_epr_value();
	sv->set_pid(epr);
	sv->set_epr(epr);
      }
    else if (is_related(sv)) {
      call_info & ci = get_related_call_info(sv);
      if (ci._leafstate.size()==1) {
	epr = ci._leafstate.key( ci._leafstate.first() );
	sv->set_pid(epr);
	sv->set_epr(epr);
      }
    }
  }

  sv->set_iport(_port);
  u_long crv = sv->get_crv();
  int flg   = sv->get_flg();

  char name[80];
  sv->Name(name);
  diag("cc_layer.call_control",DIAG_INFO,"%s: Now processing <%s> crv=%lx from B in the Call Control.\n", OwnerName(),name,crv);

  if (is_related(sv)) {
    call_info & cix = get_related_call_info(sv);

    DIAG("cc_layer.call_control.epr",DIAG_DEBUG, { 
      cout << "<" << OwnerName() << "> EPR-list of " << crv << ": ";
      dic_item li; forall_items(li, cix._leafstate) {
	cout << " " << cix._leafstate.key(li) << " . "; }
      cout << endl; } );

  } else {
    DIAG("cc_layer.call_control.epr", DIAG_INFO, cout << "<" << OwnerName() <<
	 "> from B, EPR-list of " << crv << ": NON-EXISTENT" << endl);
  }

  switch (t) {

    //----------------------------------------------------------------------------
    //--------- S E R V I C E  --  R E L A T E D   I N D I C A T I O N S ---------
    //----------------------------------------------------------------------------


  case sean_Visitor::service_register_accept:             // SERVICE_REGISTER_ACCEPT
    {                                                    //=========================
      assert(_location == USER_SIDE);
      
      diag("cc_layer.call_control",DIAG_INFO,"%s: received <service_register_accept>.", OwnerName());
      // now it should be added to _servids
      dic_item di = _servids.lookup(id);
      assert(di != 0);
      list<serv_info *> * lst = _servids.inf(di);
      serv_info * si = new serv_info( sv->get_sid() );
      list_item li = lst->search(si);
      assert(!li);
      lst->append(si);
      diag("cc_layer.call_control",DIAG_INFO,"%s: inserted sid=%d into _servids dictionary\n", OwnerName(), si->_sid);
      
      ForwardMessageToAPI(sv);
    } 
  break;
  
  case sean_Visitor::service_register_deny:                  // SERVICE_REGISTER_DENY
    {                                                       //=======================
      assert(_location == USER_SIDE);
      diag("cc_layer.call_control",DIAG_INFO,"%s: received <service_register_deny>.\n", OwnerName());
      ForwardMessageToAPI(sv);
    } 
  break;
  
  case sean_Visitor::service_dereg_deny:                    // SERVICE_DEREGISTER_DENY
    {                                                      //=========================
      assert(_location == USER_SIDE);
      diag("cc_layer.call_control",DIAG_INFO,"%s: received <service_dereg_deny>...\n", OwnerName());
      ForwardMessageToAPI(sv);
    } 
  break;
  
  case sean_Visitor::service_dereg_accept:                // SERVICE_DEREGISTER_ACCEPT              
    {                                                    //===========================
      assert(_location == USER_SIDE);
      
      diag("cc_layer.call_control",DIAG_INFO,"%s: received <service_dereg_accept>.\n", OwnerName());
      dic_item di = _servids.lookup(id);
      assert(di != 0);
      list<serv_info *> * lst = _servids.inf(di);
      serv_info si( sv->get_sid() );
      list_item li = lst->search(&si);
      assert(li);
      delete lst->inf(li);
      lst->del_item(li);
      diag("cc_layer.call_control",DIAG_INFO,"%s: removed sid=%d from _servids dictionary\n", OwnerName(), si._sid);
      
      bool all_over = attempt_final_app_death_cleanup(sv->get_shared_app_id());
      if (!all_over)
	ForwardMessageToAPI(sv);
      else 
	sv->Suicide();
    }
  break;
  
  
  //----------------------------------------------------------------------------
  //------------ C A L L  --  R E L A T E D   I N D I C A T I O N S ------------
  //----------------------------------------------------------------------------


  case sean_Visitor::leaf_setup_failure_ind:
    {
      ForwardMessageToAPI(sv);
      break;
    }

  case sean_Visitor::api_setup_ind_response_to_lsr:
  case sean_Visitor::setup_ind:                                     // SETUP_IND
    {                                                              //===========
      // No repeated setups
      if ( is_related(sv) ) {  
	diag("cc_layer.call_control",DIAG_WARNING,"%s: repeat <setup_ind> for unknown call.\n", 
	     OwnerName());
	break;
      }

      diag("cc_layer.call_control",DIAG_INFO,"%s: received <setup_ind>\n", OwnerName());

      if ((_location == USER_SIDE) &&       // on the user side, but
	  ( ! sv->get_shared_app_id())) {   // no app is interested
	
	diag("cc_layer.call_control", DIAG_WARNING, 
	     "%s: <setup_ind> is pathological: did not match any service profiles.\n", 
	     OwnerName());
	
	diag("cc_layer.call_control", DIAG_WARNING,
	     "%s: Sending a <release_comp_req> to UNI, to flog pathological <setup_ind>.\n", 
	     OwnerName());

	Failure_SendReleaseComplete(sv, ie_cause::no_user_responding);
	sv->Suicide();
      }
      else     // Network side, OR User side with interested service.
	{
	  diag("cc_layer.call_control",DIAG_INFO,"%s: first-time <setup_ind>, causes CC to call relate()...\n", 
	       OwnerName());

	  relate(sv);
	  Maintain_CRV_To_EPR_Table( sv, PHASE_EPR_ADDITION );
	  
	  if (_location == USER_SIDE) {                      // USER SIDE AND APP IS INTERESTED
	    call_info & ci = get_related_call_info(sv);
	    ci._vp = sv->get_vpi();
	    ci._vc = sv->get_vci();

	    diag("cc_layer.call_control",DIAG_INFO,
		 "%s: User stack updated callinfo VPVC when sending down a <call_proc_req> vp=%d, vc=%d...\n", 
		 OwnerName(), ci._vp, ci._vc);

	    SendMessageToUNI(sv, sean_Visitor::call_proc_req);
	  } else if (_location == NETWORK_SIDE) {              // NETWORK SIDE 
	    // 3 possibilities: see page 84 Q.2931
	    //    a) exclusive VPCI, any vci
	    //    b) exclusive VPCI, exclusive vci
	    //    c) no cid_ie
	    
	    // we must wait for the SimEvent from the Router,
	    // telling us what VPVC to use, before we can send
	    // a call proceeding
	    
	    sv->set_iport(_port);  // so the router knows what port the sean_Visitor was sent from
	    
	    if (cid_ie = (ie_conn_id *)(msg->ie(InfoElem::ie_conn_identifier_ix))) {
	      // the setup came with a vpi and vci, then we have the case of
	      // executing procedures for non-associated signalling       
	      // ** FIX ** the code for vpvc computation is not complete here.
	      // also we must rebind, as is done in the interrupt() method
	      // for Router-allocated VPVCs.
	      int _vpi = NO_VPI;
	      int _vci = NO_VCI;
	      
	      switch (cid_ie->GetPrefExclusive()) {
	      case 0: // exclusive VPCI, exclusive vci
		_vpi = cid_ie->get_vpi();
		_vci = cid_ie->get_vci();
		// validate both the vpi and the vci
		break;
		
	      case 1: // exclusive VPCI, any vci
		_vpi = cid_ie->get_vpi();
		// we must validate the vci
		break;
	      case 4: // exclusive VPCI, no vci (switched VPCs)
		break;
	      default:
		break;
	      }
	      sv->set_vpi( _vpi );
	      sv->set_vci( _vci );
	    } else {
	      // the setup came WITHOUT vpi and vci.
	      // Router will allocate the real VPVC and call us back by SimEvent.
		
	      sv->set_vpi( NO_VPI );    
	      sv->set_vci( NO_VCI );
		
	      VPVCSimEvent* vse = new VPVCSimEvent(this, this);
	      vse->SetCREF( sv->get_crv() );
	      vse->SetVPI( sv->get_vpi() );
	      vse->SetVCI( sv->get_vci() );
	      sv->SetReturnEvent( vse );         // Router will send back this simevent
		
	      diag("cc_layer.call_control", DIAG_INFO,
		   "%s: <setup_ind> triggers initiation SimEvent mechanism for VPI VCI allocation...\n", 
		   OwnerName());
	    }
	    
	    // save the setup for future replays, fattening of addparty messages at branch points, etc 
	    // ** FIX ** we should only really do this if the call is a P2MP
	    
	    int crefvalue = sv->get_crv();
	    sean_Visitor * preserve = sv->Clone(sean_Visitor::setup_req);
	    _cref2setup.insert(crefvalue,preserve);
	    
	    diag("cc_layer.call_control", DIAG_INFO,
		 "%s: waiting for notification of VPVC before addressing <setup_ind>...\n", 
		 OwnerName());
	    
	  } // end of NETWORK_SIDE
	  
	  // The sv is ready to send.
	  ForwardMessageToAPI(sv);
	}
    }
  break;
  
  case sean_Visitor::call_proc_ind:                                   // CALL_PROC_IND
    {                                                                //===============
      if ( ! is_related(sv) ) {
	diag("cc_layer.call_control",DIAG_WARNING,"%s: <call_proc_ind> for unknown call.\n", 
	     OwnerName());
	break;
      }
      
      diag("cc_layer.call_control",DIAG_INFO,"%s: received <call_proc_ind>\n", 
	   OwnerName());
      
      call_info & ci = get_related_call_info(sv);   // update VP,VC in relation datastructure
      ci._vp = sv->get_vpi();
      ci._vc = sv->get_vci();
      
      diag("cc_layer.call_control",DIAG_INFO,"%s: UPDATING vpi,vci from <call_proc_ind> to %d,%d...\n", 
	   OwnerName(),ci._vp,ci._vc);

      if (_location == USER_SIDE) {
	ForwardMessageToAPI(sv);
      } else {
	sv->Suicide();  // In the NET side, the buck stops here for call proceedings
      }
    }
  break;
    
  case sean_Visitor::connect_ind:                                        // CONNECT_IND
    {                                                                   //=============
      if ( ! is_related(sv) ) {
	diag("cc_layer.call_control",DIAG_WARNING,"%s: <connect_ind> for unknown call.\n", OwnerName());
	diag("cc_layer.call_control",DIAG_INFO,"%s: Sending <release_comp_req> to UNI...", OwnerName()); 
	Failure_SendReleaseComplete(sv,ie_cause::no_user_responding);
	break;
      }
      
      diag("cc_layer.call_control",DIAG_INFO,"%s: received <connect_ind>\n", OwnerName());

      // changing the recorded state of this call to 'active'
      call_info & ci = get_related_call_info(sv);
      ci.make_active();
      diag("cc_layer.call_control", DIAG_INFO,
	   "%s: <connect_ind> indicates that call cid=%d,crv=%lx is 'active' ...\n", 
	   OwnerName(), sv->get_cid() , sv->get_crv());

      // NOTE: connect_ack is done automagically in the UNI fsm
      // so all we need is to forward to this message to the API.
      ForwardMessageToAPI(sv);
    }
  break;
  
  case sean_Visitor::release_ind:                                        // RELEASE_IND
    {                                                                   //=============
      if ( ! is_related(sv) ) {
	diag("cc_layer.call_control",DIAG_WARNING,"%s: <release_ind> for unknown call.\n", OwnerName());
	diag("cc_layer.call_control",DIAG_INFO,"%s: Sending <release_comp_req> to UNI...", OwnerName()); 
	Failure_SendReleaseComplete(sv,ie_cause::normal_call_clearing);
	break;
      }
      
      call_info & ci = get_related_call_info(sv);
      ci.set_release_ind_seen();

      diag("cc_layer.call_control",DIAG_INFO,"%s: received <release_ind>\n", OwnerName());

      // NOTE: must we wait for a release_comp before we unrelate().
      // For now, we only need to send a release complete request
      // (that is self-stimulatory if we are on the user side.

      bool through_this_cc;
      if (_location == USER_SIDE)
	through_this_cc = true;    // we will clear the relation when we handle this
      else
	through_this_cc = false;   // we wait for the release_comp_req for downstream

      SendMessageToUNI(sv, sean_Visitor::release_comp_req, through_this_cc);
      ForwardMessageToAPI(sv);
    } 
  break;
  
  case sean_Visitor::release_comp_ind:                                // RELEASE_COMP_IND
    {                                                                //==================
      if ( ! is_related(sv) ) {
	diag("cc_layer.call_control",DIAG_WARNING,"%s: <release_comp_ind> for unknown call... absorbed.\n", 
	     OwnerName());

	sv->Suicide(); sv=0;
	break;
      }

      diag("cc_layer.call_control",DIAG_INFO,"%s: received <release_comp_ind>,  triggers CC to call unrelate()...\n", 
	   OwnerName());

      Maintain_CRV_To_EPR_Table( sv, PHASE_EPR_DELETION );
      unrelate(sv);

      if (_location == NETWORK_SIDE)
	ForwardMessageToAPI(sv);
      else
	{   // USER side
	  bool all_over = attempt_final_app_death_cleanup(sv->get_shared_app_id());
	  if (!all_over)
	    ForwardMessageToAPI(sv);
	  else 
	    sv->Suicide(); sv=0;
	}
    }
  break;

  case sean_Visitor::connect_ack_ind:                                     // CONNECT_ACK_IND
    {                                                                    //=================
      if ( ! is_related(sv) ) {
	diag("cc_layer.call_control",DIAG_WARNING,"%s: <connect_ack_ind> for unknown call... absorbed.\n", 
	     OwnerName());

	sv->Suicide(); sv=0;
	break;
      }

      diag("cc_layer.call_control", DIAG_DEBUG, "%s: received <connect_ack_ind>. ok.\n", 
	   OwnerName());

      sv->Suicide(); sv=0;
    }
  break;

  
  //----------------------------------------------------------------------------
  //------------ L E A F  --  R E L A T E D   I N D I C A T I O N S ------------
  //----------------------------------------------------------------------------


  case sean_Visitor::add_party_ind:                                         // ADD_PARTY_IND
    {                                                                      //===============
      if ( ! is_related(sv) ) {
	diag("cc_layer.call_control",DIAG_WARNING,"%s: <add_party_ind> for unknown call... absorbed.\n", 
	     OwnerName());

	sv->Suicide(); sv=0;
	break;
      }

      diag("cc_layer.call_control", DIAG_DEBUG, "%s: received <add_party_ind>\n",OwnerName());

      Maintain_CRV_To_EPR_Table( sv, PHASE_EPR_ADDITION );

      if (_location == NETWORK_SIDE) {
	// We must put in enough IEs so that if the addparty needs to be made into
	// a setup in the outgoing port CC, the message will be complete.

	int cref = sv->get_crv();
	dic_item di = _cref2setup.lookup(cref);
	assert(di);                              // there better be a setup preserved
	  
	sean_Visitor * preserved = _cref2setup.inf(di);
	generic_q93b_msg * setupmsg = preserved->take_message();
	generic_q93b_msg * addpartymsg = sv->take_message();
	InfoElem ** iesetup, ** ieaddparty;
	  
	assert( setupmsg && addpartymsg );
	  
	iesetup = setupmsg->get_ie_array();
	ieaddparty = addpartymsg->get_ie_array();
	  
	for (int ix= 0; ix<num_ie; ix++) {
	  if ((! ieaddparty[ix]) && (iesetup[ix]))  {
	    ieaddparty[ix] = iesetup[ix]->copy();
	    diag("cc_layer.call_control", DIAG_DEBUG, 
		 "%s: add_party_ind copycats <ie %d> from preserved setup\n",OwnerName(),ix);
	  }
	}

	/*
	if (! ieaddparty[InfoElem::ie_traffic_desc_ix]) {
	  ieaddparty[InfoElem::ie_traffic_desc_ix] = 
	    iesetup[InfoElem::ie_traffic_desc_ix]->copy();
	  diag("cc_layer.call_control", DIAG_DEBUG, 
	       "%s: add_party_ind copycats <ie_traffic_desc> from preserved setup\n",
	       OwnerName());
	}
	  
	if (! ieaddparty[InfoElem::ie_qos_param_ix])  {
	  ieaddparty[InfoElem::ie_qos_param_ix] = 
	    iesetup[InfoElem::ie_qos_param_ix]->copy();
	  diag("cc_layer.call_control", DIAG_DEBUG, 
	       "%s: add_party_ind copycats <ie_qos_param> from preserved setup\n",
	       OwnerName());
	}
	  
	if (! ieaddparty[InfoElem::ie_broadband_bearer_cap_ix])  {
	  ieaddparty[InfoElem::ie_broadband_bearer_cap_ix] = 
	    iesetup[InfoElem::ie_broadband_bearer_cap_ix]->copy();
	  diag("cc_layer.call_control", DIAG_DEBUG, 
	       "%s: add_party_ind copycats <ie_bbc> from preserved setup\n",
	       OwnerName());
	}
	*/

	preserved->give_message(setupmsg);

	// addpartymsg->re_encode();  Only if neccessary, in exiting port stack

	sv->give_message(addpartymsg);
      } else {  // _location == USER_SIDE i.e. two leaves on the same host!
	// user side, need to send an AddpartyAckReq
	InfoElem * ie[num_ie];

	msg = new q93b_add_party_ack_message(ie, sv->get_crv() , flg );

	sean_Visitor * new_sv = new sean_Visitor(sean_Visitor::add_party_ack_req, msg);
	new_sv->set_vci(           sv->get_vci());
	new_sv->set_vpi(           sv->get_vpi());
	new_sv->set_crv(           sv->get_crv());
	new_sv->set_pid(           sv->get_pid());
	new_sv->set_shared_app_id( sv->get_shared_app_id());
	new_sv->set_cid(           sv->get_cid());
	new_sv->set_lid(           sv->get_lid());
	  
	new_sv->set_iport(_port);

	// pass the visitor to B
	ForwardMessageToUNI(new_sv);
      }
      ForwardMessageToAPI(sv);
    }
  break;

  case sean_Visitor::add_party_ack_ind:                                   // ADD_PARTY_ACK_IND
    {                                                                    //===================
      if ( ! is_related(sv) ) {
	diag("cc_layer.call_control",DIAG_WARNING,"%s: <add_party_ack_ind> for unknown call... absorbed.\n", 
	     OwnerName());

	sv->Suicide(); sv=0;
	break;
      }

      diag("cc_layer.call_control", DIAG_DEBUG, "%s: received <add_party_ack_ind>\n", OwnerName());

      if (_location == NETWORK_SIDE) {
	sv->set_vpi( NO_VPI );
	sv->set_vci( NO_VCI );
	sv->SetReturnEvent( new VPVCSimEvent(this, this) );
      }

      // in the network stack, we send this to the Router
      // who will then Interrupt us with the allocated VPVC.
      // In the user stack, <add_party_ack_ind> goes over to the API.

      ForwardMessageToAPI(sv);
    }
  break;

  case sean_Visitor::add_party_rej_ind:                                         // ADD_PARTY_REJ_IND
    {                                                                          //===================
      if ( ! is_related(sv) ) {
	diag("cc_layer.call_control",DIAG_WARNING,"%s: <add_party_rej_ind> for unknown call... absorbed.\n", 
	     OwnerName());

	sv->Suicide(); sv=0;
	break;
      }
      
      call_info & ci = get_related_call_info(sv);
      dic_item di = ci._leafstate.lookup(epr);
      if (di) {
	diag("cc_layer.call_control",DIAG_INFO,"%s: received <add_party_rej_ind>, triggers removal of leafstate entry from callinfo... \n", OwnerName());
	Maintain_CRV_To_EPR_Table( sv, PHASE_EPR_DELETION );
      }
      else {
	diag("cc_layer.call_control",DIAG_WARNING,"%s: <add_party_rej_ind> for unknown leaf (epr=%d)... absorbed.\n", OwnerName(), epr);

  DIAG("cc_layer.call_control.epr",DIAG_INFO, { printf("** My EPR LIST:  "); \
    dic_item li; forall_items(li,ci._leafstate) { \
       printf(" %d . ",ci._leafstate.key(li)); };  \
       printf("\n"); } );
	
	sv->Suicide(); sv=0;
	break;
      }

      ForwardMessageToAPI(sv);
    }  
  break;
      
  case sean_Visitor::drop_party_ind:                                             // DROP_PARTY_IND
    {                                                                           //================
      if ( ! is_related(sv) ) {
	diag("cc_layer.call_control",DIAG_WARNING,"%s: <drop_party_ind> for unknown call... absorbed.\n", 
	     OwnerName());

	sv->Suicide(); sv=0;
	break;
      }

      SendMessageToUNI(sv, sean_Visitor::drop_party_ack_req);

      diag("cc_layer.call_control", DIAG_DEBUG, "%s: received <drop_party_ind>\n", OwnerName());
      // NOTE: we are really forwarding to Router if this is the NETWORK_SIDE...
      ForwardMessageToAPI(sv);
    }
  break;
      
  case sean_Visitor::drop_party_ack_ind:                                      // DROP_PARTY_ACK_IND
    {                                                                        //====================
      if ( ! is_related(sv) ) {
	diag("cc_layer.call_control",DIAG_WARNING,"%s: <drop_party_ack_ind> for unknown call... absorbed.\n", 
	     OwnerName());

	sv->Suicide(); sv=0;
	break;
      }
      
      call_info & ci = get_related_call_info(sv);
      dic_item di = ci._leafstate.lookup(epr);
      if (di) {
      diag("cc_layer.call_control",DIAG_INFO,
	   "%s: received <drop_party_ack_ind>, triggers removal of leafstate entry from callinfo... forwarding up...\n", OwnerName());
	Maintain_CRV_To_EPR_Table( sv, PHASE_EPR_DELETION);
      }
      else {
	diag("cc_layer.call_control",DIAG_WARNING,"%s: <drop_party_ack_ind> for unknown leaf (epr)... absorbed.\n", 
	     OwnerName());

	sv->Suicide(); sv=0;
	break;
      }

      ForwardMessageToAPI(sv);
    }  
  break;
    
  case sean_Visitor::leaf_setup_ind:                                                // LEAF_SETUP_IND
    {                                                                              //================
      if ((_location == USER_SIDE) &&       // on the user side, but
	  ( ! sv->get_shared_app_id())) {   // no app is interested

	diag("cc_layer.call_control", DIAG_WARNING, 
	     "%s: <leaf_setup_ind> is pathological: did not match any service profiles.\n", 
	     OwnerName());
	
	diag("cc_layer.call_control", DIAG_WARNING,
	     "%s: Sending a <leaf_setup_failure_req> to UNI, to flog pathological <leaf_setup_ind>.\n", 
	     OwnerName());

	Failure_SendLeafSetupFailure(sv, ie_cause::no_user_responding);
	sv->Suicide();
      }
      else {
	if (_location == NETWORK_SIDE) {
	  sv->set_iport(_port);
	  sv->set_vpi( NO_VPI );
	  sv->set_vci( NO_VCI );
	}

	UNI40_lij_call_id * lij_cid = (UNI40_lij_call_id *)ies[InfoElem::UNI40_leaf_call_id_ix];
	UNI40_leaf_sequence_num * leaf_seq = 
	  (UNI40_leaf_sequence_num *)ies[InfoElem::UNI40_leaf_sequence_num_ix];
	if (!lij_cid || !leaf_seq) {
	  diag("cc_layer.call_control", DIAG_DEBUG, "%s: leaf_setup_ind with missing/bad call id.\n");
	  sv->Suicide();
	  break;
	}

	// really forwarding to Router if we are a NETWORK_SIDE...
	ForwardMessageToAPI(sv);
      }
    }
  break;
  
  //----------------------------------------------------------------------------
  //-------------- U N S U P P O R T E D   I N D I C A T I O N S ---------------
  //----------------------------------------------------------------------------


  case sean_Visitor::setup_comp_ind:
    diag("cc_layer.call_control", DIAG_DEBUG, "%s: Unable to handle status_comp_ind (call_control.cc:%d).\n",OwnerName(), __LINE__);
    break;
  case sean_Visitor::status_enq_ind:
    diag("cc_layer.call_control", DIAG_DEBUG, "%s: Unable to handle status_enq_ind (call_control.cc:%d).\n",OwnerName(), __LINE__);
    break;
  case sean_Visitor::status_ind:
    diag("cc_layer.call_control", DIAG_DEBUG, "%s: status_ind (call_control.cc:%d).\n",OwnerName(), __LINE__);
    sv->Suicide();
    break;
  case sean_Visitor::add_party_comp_ind:
    diag("cc_layer.call_control", DIAG_DEBUG, "%s: Unable to handle add_party_comp_ind (call_control.cc:%d).\n",OwnerName(), __LINE__);
    break;
  case sean_Visitor::drop_party_comp_ind:
    diag("cc_layer.call_control", DIAG_DEBUG, "%s: Unable to handle <drop_party_comp_ind> (call_control.cc:%d).\n", OwnerName(), __LINE__);
    break;
  case sean_Visitor::restart_ind:
    diag("cc_layer.call_control", DIAG_DEBUG, "%s: Unable to handle <restart_ind> (call_control.cc:%d).\n", OwnerName(), __LINE__);
    break;
  case sean_Visitor::restart_comp_ind:
    diag("cc_layer.call_control", DIAG_DEBUG, "%s: Unable to handle <restart_comp_ind> (call_control.cc:%d).\n", OwnerName(), __LINE__);
    break;
  case sean_Visitor::restart_ack_ind:
    diag("cc_layer.call_control", DIAG_DEBUG, "%s: Unable to handle <restart_ack_ind> (call_control.cc:%d).\n", OwnerName(), __LINE__);
    break;
  default:
    break;
  }
  return this;
}

//-----------------------------------------------------------------------
// This function deals with sean visitors from the A side.
// These can be a result of: 
// * self-stimulus flag in the 'SendMessageToUNI()' method.
// * redirected messages sent by the Router into NETWORK_SIDE call_controls
// * sean_Visitors made via translation of IPC_DataVisitor,
//     and sent in a self-stimulating way.
//-----------------------------------------------------------------------
State * call_control::sean_visitor_from_A(sean_Visitor * sv) 
{
  generic_q93b_msg * m = 0L;
  InfoElem  * ie[num_ie];
  for(int i = 0; i < num_ie; i++) ie[i] = 0;

  int epr = sv->get_pid();;
  generic_q93b_msg * msg = sv->share_message();
  InfoElem ** ies = 0;
  if (msg) {
    ies = msg->get_ie_array();

    if(ies[InfoElem::ie_end_pt_ref_ix])  // P2MP indeed.
      {
	epr = ((ie_end_pt_ref *)ies[InfoElem::ie_end_pt_ref_ix])->get_epr_value();
      }
  }

  int crv   = sv->get_crv();
  int flg   = sv->get_flg();
  int iport = sv->get_iport();

  sv->set_pid(epr);
  sv->set_epr(epr);

  sv->set_oport ( _port );

  char name[80];
  sv->Name(name);
  diag("cc_layer.call_control",DIAG_INFO,"%s: Now processing <%s> crv=%lx from A in the Call Control.\n", OwnerName(),name,crv);



  if (is_related(sv)) {
    call_info & cix = get_related_call_info(sv);

    DIAG("cc_layer.call_control",DIAG_INFO, { printf("<%s> EPR-list of %lx:  ",OwnerName(), crv); \
      dic_item li; forall_items(li,cix._leafstate) { \
         printf(" %d . ",cix._leafstate.key(li)); };  \
         printf("\n"); } );
  }
  else DIAG("cc_layer.call_control",DIAG_INFO, { printf("<%s> from A, EPR-list of %lx : NON-EXISTENT\n ",OwnerName(), crv);} );


  switch (sv->get_op()) 
    {
    case sean_Visitor::leaf_setup_failure_req:
      {
	ForwardMessageToUNI(sv);
	break;
      }

    case sean_Visitor::release_req:
      {
	if ( ! is_related(sv) ) {
	  diag("cc_layer.call_control",DIAG_WARNING,"%s: <release_req>, for unknown call... absorbed.\n", OwnerName());
	  sv->Suicide(); sv=0;
	  break;
	}
	
	if (_location == USER_SIDE) {
	  ForwardMessageToUNI(sv);
	  return this;
	}
	
	if (epr != NO_EPR)   // P2MP release_req
	  {
	    diag("cc_layer.call_control", DIAG_DEBUG, "%s: got a release_req, with epr = %d\n" , OwnerName(), epr);
	    call_info & cix = get_related_call_info(sv);
	    if (cix._leafstate.size() > 1)
	      {
		diag("cc_layer.call_control",DIAG_INFO,"%s: CC detects that <release_req> is at branch node.\n", OwnerName());
		diag("cc_layer.call_control",DIAG_INFO,"%s: Transmogrifying this <release_req> into a <drop_party_req>.\n", OwnerName());
		
		ie_cause::CauseValues cv = ie_cause::normal_unspecified;
		if(ies[InfoElem::ie_cause_ix])
		  cv = ((ie_cause *)ies[InfoElem::ie_cause_ix])->get_cause_value();
		ie[InfoElem::ie_cause_ix] = new ie_cause(cv,ie_cause::local_private_network);
		ie[InfoElem::ie_end_pt_ref_ix] = new ie_end_pt_ref(epr);
		
		m = new q93b_drop_party_message(ie,crv,flg);
		sv->Suicide();
		sv = 0;
		
		sean_Visitor* new_sv = new sean_Visitor(sean_Visitor::drop_party_req, m);
		new_sv->set_crv   ( crv   );
		new_sv->set_pid   ( epr   );
		new_sv->set_iport ( iport );
		new_sv->set_oport ( _port );
		ForwardMessageToUNI(new_sv);
	      }
	    else ForwardMessageToUNI(sv); // release_req continues...
	  }
	else ForwardMessageToUNI(sv); // release_req continues...
      }
    break;
    
    case sean_Visitor::release_comp_req:
      {
	if ( ! is_related(sv) ) {
	  diag("cc_layer.call_control",DIAG_WARNING,"%s: <release_comp_req>, for unknown call... absorbed.\n", OwnerName());
	  sv->Suicide(); sv=0;
	  break;
	}

	if (_location == USER_SIDE) {
	  
	  diag("cc_layer.call_control",DIAG_INFO,"%s: <release_comp_req> on user side triggers unrelate()...\n", OwnerName());
	  Maintain_CRV_To_EPR_Table( sv, PHASE_EPR_DELETION );
	  unrelate(sv);
  
	  bool all_over = attempt_final_app_death_cleanup(sv->get_shared_app_id());
	  ForwardMessageToUNI(sv);
	} 
	else   	  // NETWORK SIDE
	  {
	    call_info & ci = get_related_call_info(sv);
	    if ( ci.was_release_ind_seen() )
	      {
		diag("cc_layer.call_control",DIAG_INFO,
		     "%s: acknowledgement of release_req/ind -- <release_comp_req> was absorbed. ok.\n", 
		     OwnerName());
		Maintain_CRV_To_EPR_Table( sv, PHASE_EPR_DELETION );
		unrelate(sv);

		sv->Suicide();
		sv = 0;
		break;
	      }

	    if (epr == NO_EPR) {   // P2P
	      diag("cc_layer.call_control",DIAG_INFO,
		   "%s: <release_comp_req> on P2P network side triggers unrelate()...\n", 
		   OwnerName());
	      Maintain_CRV_To_EPR_Table( sv, PHASE_EPR_DELETION );
	      unrelate(sv);

	      ForwardMessageToUNI(sv);
	    }
	    else {                                               // P2MP
	      call_info & cix = get_related_call_info(sv);
	      if (cix._leafstate.size() > 1) {                   // BRANCH POINT
	      
		diag("cc_layer.call_control",DIAG_INFO,
		     "%s: <release_comp_req> at P2MP network side branch point... epr=%d\n", 
		     OwnerName(), epr);

		call_info & cix = get_related_call_info(sv);
		dic_item ls = cix._leafstate.lookup(epr);
		assert (ls);
		if (cix._leafstate.inf(ls) == LEAF_ACTIVE) {
		  
		  diag("cc_layer.call_control", DIAG_INFO,
		       "%s: Turning <release_comp_req> into <drop_party_ack_req>...\n", OwnerName());
		  
		  // make DROP_PARTY_ACK message
		  ie_cause::CauseValues cv = ie_cause::normal_unspecified;
		  if(ies[InfoElem::ie_cause_ix])
		    cv = ((ie_cause *)ies[InfoElem::ie_cause_ix])->get_cause_value();
		  ie[InfoElem::ie_cause_ix] = new ie_cause(cv,ie_cause::local_private_network);
		  ie[InfoElem::ie_end_pt_ref_ix] = new ie_end_pt_ref(epr);
		  m = new q93b_drop_party_ack_message(ie,crv,flg);
		  Maintain_CRV_To_EPR_Table( sv, PHASE_EPR_DELETION );
		  sv->Suicide();
		  sv = 0;
		  
		  sean_Visitor* new_sv = new sean_Visitor(sean_Visitor::drop_party_ack_req, m);
		  new_sv->set_pid  ( epr   );
		  new_sv->set_oport( _port );
		  new_sv->set_iport( iport );
		  ForwardMessageToUNI(new_sv);
		}
		else {
		  // turn the RELEASE_COMP.req into a ADD_PARTY_REJECT
		  diag("cc_layer.call_control", DIAG_INFO,
		       "%s: Turning <release_comp_req> into <add_party_rej_req>...\n", OwnerName());
		  
		  // make ADD_PARTY_REJECT message
		  ie_cause::CauseValues cv = ie_cause::normal_unspecified;
		  if(ies[InfoElem::ie_cause_ix])
		    cv = ((ie_cause *)ies[InfoElem::ie_cause_ix])->get_cause_value();
		  ie[InfoElem::ie_cause_ix] = new ie_cause(cv,ie_cause::local_private_network);
		  ie[InfoElem::ie_end_pt_ref_ix] = new ie_end_pt_ref(epr);
		  m = new q93b_add_party_rej_message(ie,crv,flg);
		  Maintain_CRV_To_EPR_Table( sv, PHASE_EPR_DELETION );
		  sv->Suicide();
		  sv = 0;
		  
		  sean_Visitor* new_sv = new sean_Visitor(sean_Visitor::add_party_rej_req, m);
		  new_sv->set_crv(   crv   );
		  new_sv->set_pid(   epr   );
		  new_sv->set_oport( _port );
		  new_sv->set_iport( iport );
		  ForwardMessageToUNI(new_sv);
		}
	      }
	      else    // NOT BRANCH POINT // ci._leafstate.size() <= 1
		{
		  diag("cc_layer.call_control",DIAG_INFO,"%s: <release_comp_req> at P2MP network side NON-branch point...\n", OwnerName());
		  Maintain_CRV_To_EPR_Table( sv, PHASE_EPR_DELETION );
		  if (ci._leafstate.size() == 0)
		    unrelate(sv);
		  ForwardMessageToUNI(sv);
		}
	    }
	  }
      }
    break;

    case sean_Visitor::drop_party_req:
      {
	if ( ! is_related(sv) ) {
	  diag("cc_layer.call_control",DIAG_WARNING,"%s: <drop_party_req>, for unknown call... absorbed.\n", OwnerName());
	  sv->Suicide(); sv=0;
	  break;
	}

	// USER_SIDE, or not a branch point in multicast tree
	if (_location == USER_SIDE) 
	  {
	    ForwardMessageToUNI(sv);
	  }
	else 
	  {
	    call_info & cix = get_related_call_info(sv);
	    if (cix._leafstate.size() > 1) // MULTIPLE PARTIES DOWNSTREAM
	      {
		ForwardMessageToUNI(sv);
	      }
	    else 
	      {                                                                  // SINGLE PARTY DOWNSTREAM
		diag("cc_layer.call_control",DIAG_INFO,"%s: Turning <drop_party_req> into <release_req>...\n", OwnerName());
	
		ie_cause::CauseValues cv = ie_cause::normal_unspecified;
		if (ies[InfoElem::ie_cause_ix])
		  cv = ((ie_cause *)ies[InfoElem::ie_cause_ix])->get_cause_value();
	
		// need to convert _location due to a lack of consistency 
		ie_cause::Location loc = ie_cause::user;
		loc = ie_cause::local_private_network;
		ie[InfoElem::ie_cause_ix] = new ie_cause(cv,loc);
		ie[InfoElem::ie_end_pt_ref_ix] = new ie_end_pt_ref(epr);
		m = new q93b_release_message(ie, crv, flg);
		sv->Suicide();
		sv = 0;

		sean_Visitor* new_sv = new sean_Visitor(sean_Visitor::release_req, m);
		new_sv->set_crv   ( crv   );
		new_sv->set_pid   ( epr   );
		new_sv->set_oport ( _port );
		new_sv->set_iport ( iport );
		ForwardMessageToUNI(new_sv);
	      }
	  }
      }
      break;

  case sean_Visitor::add_party_req: 
    {      
      if (_location == USER_SIDE) {	
	Maintain_CRV_To_EPR_Table( sv, PHASE_EPR_ADDITION );
	ForwardMessageToUNI(sv);
	break;
      } else { // NETWORK SIDE
	
	bool many_downstream = false;

	if (is_related(sv)) {
	  call_info & cix = get_related_call_info(sv);
	  if (cix._leafstate.size() >= 1)  { // PARTIES DOWNSTEAM
	    many_downstream = true;
	  }
	}

	if (many_downstream) {
	  ForwardMessageToUNI(sv);
	  break;
	} else {     // NO PARTY DOWNSTREAM
	  
	  diag("cc_layer.call_control",DIAG_INFO,"%s: Turning <add_party_req> into <setup_req>...\n", OwnerName());
	  
	  for (int i = 0; i < num_ie; i++) {
	    ie[i] = 0;
	    if (ies[i])
	      ie[i] = ies[i]->copy();
	  }
	  // soemone has to keep track of Q0S (port,cref) ---> QoS
	      
	  // Whew, the incoming ports CC shoved these IEs into the AddParty 
	  // indication, so that we'd have enough info to make a valid setup!  
	  
	  // ie[InfoElem::ie_traffic_desc_ix] = ?
	  // ie[InfoElem::ie_qos_param_ix] = ?
	  // ie[InfoElem::ie_broadband_bearer_cap_ix] = ?
	
	  ie[InfoElem::ie_conn_identifier_ix] = new ie_conn_id(sv->get_vpi(), sv->get_vci(), 1, 0);
	  SimEvent* transfer_event = sv->GetReturnEvent();
	  sv->Suicide();
	  sv = 0;

	  m = new q93b_setup_message(ie, crv, flg);
	  sean_Visitor* new_sv = new sean_Visitor(sean_Visitor::setup_req, m);
	  new_sv->set_vpi( ((ie_conn_id *)ie[InfoElem::ie_conn_identifier_ix])->get_vpi() );
	  new_sv->set_vci( ((ie_conn_id *)ie[InfoElem::ie_conn_identifier_ix])->get_vci() );
	  new_sv->set_oport( _port );
	  new_sv->set_iport( iport );
	  new_sv->SetReturnEvent(transfer_event);
	  
	  sean_visitor_from_A(new_sv);
	}
      }
    }
  break;

    case sean_Visitor::leaf_setup_req: {
      if (_location == USER_SIDE) {	
	UNI40_leaf_sequence_num * leaf_seq = new UNI40_leaf_sequence_num( _next_sequence_number );

	if (ies[InfoElem::UNI40_leaf_sequence_num_ix])
	  delete ies[InfoElem::UNI40_leaf_sequence_num_ix];
	ies[InfoElem::UNI40_leaf_sequence_num_ix] = leaf_seq;
	msg->re_encode();
      
	_next_sequence_number++;
      }
      ForwardMessageToUNI(sv);
      break;
    }
    break;


    case sean_Visitor::setup_req: 
      {
	if ( is_related(sv) ) {
	  diag("cc_layer.call_control",DIAG_WARNING,"%s: repeat <setup_req>... absorbed.\n", OwnerName());
	  sv->Suicide(); sv=0;
	  break;
	}

	// We always allocate the CREF for setup requests
	diag("cc_layer.call_control", DIAG_DEBUG, "%s: allocating cref = %lx for <setup_req> ...\n", 
	     OwnerName(), _next_available_cref);
	
	sv->set_crv( _next_available_cref );
	_next_available_cref ++;
	
	sv->set_flg(0);     // we initiated the setup

	// call back the Router with the newly alloc'd crv if this is net side
	if (_location == NETWORK_SIDE) 
	  {
	    SimEvent * se = sv->GetReturnEvent();
	    if (!se)
	      diag("cc_layer.call_control", DIAG_WARNING, "%s %s arrived from Router w/o VPVCSimEvent!\n", 
		   sv->GetType().Name(), sv->op_name());
	    else 
	      {
		assert(se->GetCode() == VPVC_CODE);
		VPVCSimEvent * vse = (VPVCSimEvent *)se;
		vse->SetCREF( sv->get_crv() );
		ReturnToSenderSynchronous(vse);
	      }
	  }
	else { // USER SIDE
	  if( ies[InfoElem::UNI40_leaf_call_id_ix] ) {
	    sv->set_sid( -1 * sv->get_crv() );           // the root sensing service has sid = (-1 * call crv)
	  }
	}

	diag("cc_layer.call_control",DIAG_INFO,"%s: <setup_req> triggers relate()...\n", OwnerName());
	relate(sv);
	Maintain_CRV_To_EPR_Table( sv, PHASE_EPR_ADDITION );
	ForwardMessageToUNI(sv);
      }
    break;
    
    case  sean_Visitor::connect_req:
      {

	if ( ! is_related(sv) ) {
	  diag("cc_layer.call_control",DIAG_WARNING,"%s: <connect_req>, for unknown call... absorbed.\n", OwnerName());
	  sv->Suicide(); sv=0;
	  break;
	}

	call_info & ci = get_related_call_info(sv);
	ci.make_active();
	
	diag("cc_layer.call_control",DIAG_INFO,"%s: <connect_req> indicates that call cid=%d,crv=%lx is 'active' ...\n", 
	     OwnerName(), sv->get_cid() , sv->get_crv());
	
	if (_location == USER_SIDE) 
	{
	  ForwardMessageToUNI(sv);
	}
	else           // NETWORK SIDE
	  {
	    if (epr == NO_EPR) {          // P2P 
	      diag("cc_layer.call_control", DIAG_INFO,
		   "%s: (P2P) <connect_req> cref= %lx being forwarded down to UNI...\n", 
		   OwnerName(),sv->get_crv());
	      ForwardMessageToUNI(sv);
	    }
	    else                         // P2MP
	      {
		call_info & cix = get_related_call_info(sv);
		if (cix._leafstate.size() > 1)   {   // MULTIPLE PARTIES DOWNSTREAM
		  diag("cc_layer.call_control",DIAG_INFO,"%s: Turning <connect_req> into <add_party_ack_req>...\n", OwnerName());
		  
		  for (int i = 0; i < num_ie; i++) {
		    ie[i] = 0;
		    if (ies[i])
		      ie[i] = ies[i]->copy();
		  }
		  int epr = sv->get_pid();
		  ie[InfoElem::ie_end_pt_ref_ix] = new ie_end_pt_ref(epr);
		  m = new q93b_add_party_ack_message(ie,crv,flg);
		  sv->Suicide();
		  sv = 0;
		  
		  sean_Visitor* new_sv = new sean_Visitor(sean_Visitor::add_party_ack_req, m);
		  new_sv->set_oport ( _port );
		  new_sv->set_iport ( iport );
		  new_sv->set_pid   ( epr   );
		  new_sv->set_crv   ( crv   );
		  
		  assert( is_related(new_sv) );

		  call_info & ci = get_related_call_info(new_sv);
		  dic_item ls = ci._leafstate.lookup(epr);
		  if (ls)
		    {
		      ci._leafstate.change_inf(ls, LEAF_ACTIVE);
		      diag("cc_layer.call_control",DIAG_INFO,"%s: Noting that leaf %d is now ACTIVE...\n", OwnerName(), epr);
		      ForwardMessageToUNI(new_sv);
		    }
		  else 
		    {
		      diag("cc_layer.call_control",DIAG_WARNING,"%s: <add_party_ack_req> for unknown leaf (epr)... absorbed.\n", 
			   OwnerName());
			
		      new_sv->Suicide(); sv=0;
		      break;
		    }
		}
		else    // SINGLE PARTY DOWNSTREAM
		  {
		    int epr = sv->get_pid();
		  
		    assert( is_related(sv) );
		    call_info & ci = get_related_call_info(sv);
		    dic_item ls = ci._leafstate.lookup(epr);
		    if (ls)
		      {
			ci._leafstate.change_inf(ls, LEAF_ACTIVE);
			diag("cc_layer.call_control",DIAG_INFO,"%s: Noting that leaf %d is now ACTIVE...\n", OwnerName(), epr);
			ForwardMessageToUNI(sv);
		      }
		    else 
		      {
			diag("cc_layer.call_control",DIAG_WARNING,"%s: <connect_req> for unknown leaf (epr)... absorbed.\n", 
			     OwnerName());
			
			sv->Suicide(); sv=0;
			break;
		      }
		  }
	      }
	  }
      }
    break;

  default:                         // Pass down the riffraff !!!
    ForwardMessageToUNI(sv);
    break;
  }
  return this;
}

//-----------------------------------------------------------------------
// dispatch visitors that supposedly enter from A
//-----------------------------------------------------------------------
State * call_control::visitor_arrived_from_A_side(Visitor * v) 
{
  State * override = this;
  VisitorType vt = v->GetType();
  
  if (vt.Is_A( _ipc_data_vistype )) {
    IPC_DataVisitor * dv = (IPC_DataVisitor*)v;
    override = ipc_data_visitor_from_A( dv );
  } else if (vt.Is_A( _ipc_sig_vistype )) {
    IPC_SignallingVisitor * iv = (IPC_SignallingVisitor*)v;
      override = ipc_signalling_visitor_from_A( iv );
  } else if (vt.Is_A( _sean_vistype )) {
    sean_Visitor * sv = (sean_Visitor*)v;
    override = sean_visitor_from_A( sv );
  } else
    v->Suicide();
  
  return override;
}

 
//-----------------------------------------------------------------------
// dispatch visitors that supposedly enter from B
//-----------------------------------------------------------------------
State * call_control::visitor_arrived_from_B_side(Visitor * v) 
{
  State* override = this;
  VisitorType vt = v->GetType();
  
  if (vt.Is_A( _sean_vistype )) {
    sean_Visitor * sv = (sean_Visitor*)v;
    
    char type[255];
    sv->Name(type);
    diag("cc_layer.call_control",DIAG_INFO,"%s:CallControl - from B side we get a <%s>\n",OwnerName(), type);
    
    override = sean_visitor_from_B( sv );
  } else
    v->Suicide();  
  return override;
}


//-----------------------------------------------------------------------
State * call_control::Handle(Visitor * v) 
{  
  State * override = 0;
  switch (VisitorFrom(v)) {
  case Visitor::A_SIDE: 
    override = visitor_arrived_from_A_side(v);
    break;
  case Visitor::B_SIDE: 
    override = visitor_arrived_from_B_side(v);
    break;
  default:
    break;
  }

  if (override) 
    return override;
  else
    
    return this;  // pass it on !
}


//-----------------------------------------------------------------------
void call_control::Maintain_CRV_To_EPR_Table( Visitor* v , int phase)
{
  sean_Visitor * sv = 0;

  int crv = NO_CREF;
  int epr = NO_EPR;
  sean_Visitor::sean_Visitor_Type t;

  VisitorType vt = v->GetType();
  if (vt.Is_A( _sean_vistype )) {
    sv = (sean_Visitor*)v;
    CheckP2MP(sv);                   // sets the visitor's P2MP flag
    t = sv->get_op();
    epr = sv->get_pid();
    crv = sv->get_crv();
  }
  else {
    abort();
  }

  call_info & ci = get_related_call_info(sv);
  dic_item ls = ci._leafstate.lookup(epr);
  if (epr==NO_EPR) {
    if (ci._leafstate.size() == 1) {
      epr = ci._leafstate.key( ci._leafstate.first() );
    }
  }

  dic_item li; 
  diag("cc_layer.call_control.epr",DIAG_INFO,"START ******************* EPR  %lx ????? %d\n",crv,epr);

  switch (t) {
  case sean_Visitor::setup_ind:
  case sean_Visitor::setup_req:
  case sean_Visitor::add_party_req:
  case sean_Visitor::add_party_ind:
    if (phase == PHASE_EPR_ADDITION) {

  DIAG("cc_layer.call_control.epr",DIAG_INFO, { printf("PRE ********************** EPR LIST OF %lx:  ",crv); \
    forall_items(li,ci._leafstate) { \
       printf(" %d . ",ci._leafstate.key(li)); };  \
       printf("\n"); } );

  if (ci._leafstate.lookup(epr)) {
    diag("cc_layer.call_control.epr",DIAG_ERROR,
	 "************************** UNABLE TO DO EPR --> ADDING to %lx:  %d\n",crv,epr);
      }
  else {
    ci._leafstate.insert(epr, LEAF_INITIATED);
    diag("cc_layer.call_control.epr",DIAG_INFO,"************************** EPR --> ADDING to %lx:  %d\n",crv,epr);
  }

  DIAG("cc_layer.call_control.epr",DIAG_INFO, { printf("POST ********************** EPR LIST OF %lx:  ",crv); \
    forall_items(li,ci._leafstate) { \
       printf(" %d . ",ci._leafstate.key(li)); };  \
       printf("\n"); } );
    }
    break;

  case sean_Visitor::add_party_rej_req:
  case sean_Visitor::add_party_rej_ind:
  case sean_Visitor::drop_party_ack_req:
  case sean_Visitor::drop_party_ack_ind:

    // case sean_Visitor::release_comp_ind: // release_complete does not carry EPR
    // case sean_Visitor::release_comp_req: // release_complete does not carry EPR!

    if (phase == PHASE_EPR_DELETION) {

DIAG("cc_layer.call_control.epr",DIAG_INFO, { printf("PRE ********************** EPR LIST OF %lx:  ",crv); \
    forall_items(li,ci._leafstate) { \
       printf(" %d . ",ci._leafstate.key(li)); };  \
       printf("\n"); } );

       li = ci._leafstate.lookup(epr);
       if (li) {
	 ci._leafstate.del_item(li);
	 diag("cc_layer.call_control.epr",DIAG_INFO,"************************** EPR --> DELETING to %lx:  %d\n",crv,epr);
       }
       else {
	 diag("cc_layer.call_control.epr",DIAG_ERROR,"************************** %d UNABLE TO DO EPR --> DELETING to %lx:  %d\n",t,crv,epr);
	 char debug[255];
	 strcpy(debug, "Here is the error of UNABLE TO DO EPR --> DELETING\n");
	 AppendCommentToLog(debug);
       }

    
  DIAG("cc_layer.call_control.epr",DIAG_INFO, { printf("POST ********************** EPR LIST OF %lx:  ",crv); \
    forall_items(li,ci._leafstate) { \
       printf(" %d . ",ci._leafstate.key(li)); };  \
       printf("\n"); } );
    }
    break;
    
  default:
    break;
  }

  diag("cc_layer.call_control.epr",DIAG_INFO,"DONE ******************* EPR  %lx ????? %d\n",crv,epr);

}

//-----------------------------------------------------------------------
void call_control::Failure_SendRelease(sean_Visitor* sv, 
				       ie_cause::CauseValues c,
				       bool self_inject) 
{
  InfoElem ** ie_array;
  ie_array=new InfoElem* [num_ie];
  for (int i=0;i<num_ie;i++) ie_array[i]=0;
  ie_cause::Location loc;
  switch(_location) {
  case USER_SIDE:
    loc= ie_cause::user;
    break;
  case NETWORK_SIDE:
    loc=ie_cause::local_private_network;
    break;
  default:
    break;
  }
  int flg   = sv->get_flg();

  ie_array[InfoElem::ie_cause_ix] = new ie_cause(c,loc);
  generic_q93b_msg* msg = new q93b_release_message(ie_array, sv->get_crv(), flg);
  sean_Visitor* killv = new sean_Visitor(sean_Visitor::release_req, msg);
  killv->set_crv( sv->get_crv() );
  killv->set_cid( sv->get_cid() );
  killv->set_sid( -1 * sv->get_cid() );
  killv->set_shared_app_id( sv->get_shared_app_id() );
  
  if (self_inject)
    sean_visitor_from_A(killv);   // pass it through the cc !
  else
    PassVisitorToB(killv);        // pass it straight down
}


//-----------------------------------------------------------------------
void call_control::Failure_SendReleaseComplete(sean_Visitor* sv, 
					       ie_cause::CauseValues c,
					       bool self_inject) 
{
  InfoElem** ie_array;
  ie_array=new InfoElem* [num_ie];
  for (int i=0;i<num_ie;i++) ie_array[i]=0;
  ie_cause::Location loc;
  switch(_location) {
  case USER_SIDE:
    loc= ie_cause::user;
    break;
  case NETWORK_SIDE:
    loc=ie_cause::local_private_network;
    break;
  default:
    break;
  }
  int flg   = sv->get_flg();

  ie_array[InfoElem::ie_cause_ix] = new ie_cause(c,loc);
  generic_q93b_msg* msg = new q93b_release_comp_message(ie_array, sv->get_crv(), flg);
  sean_Visitor* killv = new sean_Visitor(sean_Visitor::release_comp_req, msg);
  killv->set_crv( sv->get_crv() );
  killv->set_cid( sv->get_cid() );
  killv->set_sid( -1 * sv->get_cid() );
  killv->set_shared_app_id( sv->get_shared_app_id() );
  
  if (self_inject)
    sean_visitor_from_A(killv);   // pass it through the cc !
  else
    PassVisitorToB(killv);        // pass it straight down
}

//-----------------------------------------------------------------------
void call_control::Failure_SendLeafSetupFailure(sean_Visitor * sv, 
						ie_cause::CauseValues c,
						bool self_inject) {
  InfoElem** ie_array;
  ie_array=new InfoElem* [num_ie];
  for (int i=0;i<num_ie;i++) ie_array[i]=0;

  generic_q93b_msg * msg = sv->share_message();
  InfoElem ** ies = 0;
  if (msg) {
    ies = msg->get_ie_array();

    for (int i=0;i<num_ie;i++) {
      if (ies[i]) {
	  ie_array[i]=ies[i]->copy();
      }
    }
  }

  // swap called and calling

  Addr* called = ((ie_called_party_num*)ie_array[InfoElem::ie_called_party_num_ix])-> get_addr()->copy();
  Addr* calling = ((ie_calling_party_num*)ie_array[InfoElem::ie_calling_party_num_ix])-> get_addr()->copy();

  delete ie_array[InfoElem::ie_called_party_num_ix];
  delete ie_array[InfoElem::ie_calling_party_num_ix];

  ie_array[InfoElem::ie_called_party_num_ix]  = new ie_called_party_num(calling);
  ie_array[InfoElem::ie_calling_party_num_ix] = new ie_calling_party_num(called,
									 ie_calling_party_num::presentation_allowed,
									 ie_calling_party_num::user_provided_not_screened);
  ie_cause::Location loc;
  switch(_location) {
  case USER_SIDE:
    loc= ie_cause::user;
    break;
  case NETWORK_SIDE:
    loc=ie_cause::local_private_network;
    break;
  default:
    break;
  }
  int flg   = sv->get_flg();

  ie_array[InfoElem::ie_cause_ix] = new ie_cause(c,loc);
  generic_q93b_msg* new_msg = new UNI40_leaf_setup_failure_message(ie_array, sv->get_crv(), flg);

  sean_Visitor* killv = new sean_Visitor(sean_Visitor::leaf_setup_failure_req, new_msg);
  killv->set_crv( sv->get_crv() );
  killv->set_cid( 0 );
  killv->set_shared_app_id( 0 );
  
  if (self_inject)
    sean_visitor_from_A(killv);   // pass it through the cc !
  else
    PassVisitorToB(killv);        // pass it straight down
}

//-----------------------------------------------------------------------
void call_control::ForwardMessageToAPI(sean_Visitor *& sv) 
{
  if (_location == USER_SIDE) {
    const abstract_local_id* id = sv->get_shared_app_id();
    if (( _app_alive.lookup(id) == 0) ||
	( _app_alive.access(id) == 0)) {
      diag("daemon.cc",DIAG_INFO,"%s application is gone already... \n", OwnerName());
      diag("daemon.cc",DIAG_INFO,"...killing upward bound sean_Visitor %lx\n",sv);
      sv->Suicide();
      return;
    }
    
    sean_Visitor::sean_Visitor_Type op=sv->get_op();
    
    Buffer * buf = sv->take_buffer();
    if (!buf) {
      switch (op) {
      case sean_Visitor::service_register_req:
      case sean_Visitor::service_register_accept:
      case sean_Visitor::service_register_deny:
      case sean_Visitor::service_dereg_accept:
      case sean_Visitor::service_dereg_deny:
	diag("cc_layer.call_control", DIAG_DEBUG, "%s %s has no Buffer to take, but that's perfectly normal...", sv->GetType().Name(), sv->op_name());
	buf = new Buffer( 1024 );
	buf->set_length( 0 );
	break;
      default:
	diag("cc_layer.call_control", DIAG_FATAL, "%s %s has no Buffer to take!", sv->GetType().Name(), sv->op_name());
	break;
      }
    }
    buf->GrowHeader( 4*sizeof(int) );
    unsigned char* data = buf->data();
    int cid=sv->get_cid();
    int sid=sv->get_sid();
    int lid=sv->get_lid();
    int index=0;
    
    bcopy(&op, (void*)( data + sizeof(int) * index ), sizeof(int)); index++;
    bcopy(&cid,(void*)( data + sizeof(int) * index ), sizeof(int)); index++;
    bcopy(&sid,(void*)( data + sizeof(int) * index ), sizeof(int)); index++;
    bcopy(&lid,(void*)( data + sizeof(int) * index ), sizeof(int)); index++;
    
    IPC_DataVisitor * dv = new IPC_DataVisitor(id, buf);
    PassVisitorToA(dv);
    
    sv->Suicide();
  } else      
    {
      if (_location == NETWORK_SIDE) {
	char name[80];
	sv->Name(name);
	diag("cc_layer.call_control",DIAG_INFO,"%s on port = %d : Forwarding to Router %s \n", OwnerName(),_port,name);
	diag("cc_layer.call_control",DIAG_INFO,"     crv = %lx epr = %d vci = %d\n",sv->get_crv(),sv->get_pid(), sv->get_vci());
	PassVisitorToA(sv);
      }
      else {
	char name[80];
	sv->Name(name);
	diag("cc_layer.call_control",DIAG_INFO,"*** %s now forwarding %s towards fake application.\n", OwnerName(),name);
	PassVisitorToA(sv);
      }
    }
  sv = 0;
}

//-----------------------------------------------------------------------
void call_control::ForwardMessageToUNI(sean_Visitor *& sv) 
{
  sv->set_oport(_port);
  PassVisitorToB(sv);
  sv = 0;
}

//-----------------------------------------------------------------------
void call_control::SendMessageToUNI(sean_Visitor * sv, 
				    sean_Visitor::sean_Visitor_Type t,
				    bool self_inject) 
{
  sean_Visitor * clone = sv->Clone(t);
  if (clone) {
    if (self_inject)
      sean_visitor_from_A(clone);   // pass it through the cc !
    else
      PassVisitorToB(clone);        // pass it straight down
    
  } else 
    diag("cc_layer.call_control",DIAG_ERROR,"Couldn't build clone sean_Visitor\n");
}

//-----------------------------------------------------------------------
void call_control::SendMessageToAPI(sean_Visitor * sv, 
				    sean_Visitor::sean_Visitor_Type t)
{
  sean_Visitor * clone = sv->Clone(t);
  if (clone) {
    if (_location == NETWORK_SIDE)
      PassVisitorToB(clone);
    else
      ForwardMessageToAPI(clone);      
  } else 
    diag("cc_layer.call_control",DIAG_ERROR,"Couldn't build clone sean_Visitor\n");
}

//-----------------------------------------------------------------------
void call_control::Interrupt(class SimEvent * e) 
{
  assert (_location == NETWORK_SIDE);

  if (e->GetCode() == VPVC_CODE) {
    diag("cc_layer.call_control", DIAG_DEBUG, "code = VPVC_CODE: Interrupted by VPVCSimEvent\n");
    
    VPVCSimEvent * vse = (VPVCSimEvent *)e;
    // send out the call proceeding message
    int vp = vse->GetVPI(), vc = vse->GetVCI(), cref = vse->GetCREF();
    
    // adjust the saved setup to reflect the newly chosen VPVC 
    // ** FIX ** we should only do this if it is a P2MP

    dic_item di = _cref2setup.lookup(cref);
    assert(di);
    sean_Visitor * preserved = _cref2setup.inf(di);
    generic_q93b_msg * msg = preserved->take_message();

    int flg   = preserved->get_flg();
    
    diag("cc_layer.call_control", DIAG_DEBUG, "VPVCSimEvent says use vp=%d vc=%d\n", vp,vc);
    
    InfoElem ** ies = msg->get_ie_array();
    if (ies[InfoElem::ie_conn_identifier_ix])
      delete ies[InfoElem::ie_conn_identifier_ix];
    ies[InfoElem::ie_conn_identifier_ix] = new ie_conn_id(vp, vc, 1, 0);
    generic_q93b_msg * new_msg = new q93b_setup_message(ies, cref,flg);
    preserved->give_message(new_msg);

    // Clean-up code added  8-21-98 - mountcas
    for (int i = 0; i < num_ie; i++)
      ies[i] = 0;
    delete msg;
    
    sean_Visitor * callproc = preserved->Clone(sean_Visitor::call_proc_req);
    callproc->set_vpi(vp);
    callproc->set_vci(vc);
    callproc->set_flg( flg );  // The preserved setup has the correct crv

    call_info & ci = get_related_call_info(preserved);   // update VP,VC in relation datastructure
    ci._vp = vp;
    ci._vc = vc;
    diag("cc_layer.call_control",DIAG_INFO,"%s: UPDATING VPVC in NETSTACK interrupt, sending back <call_proc_req> %d,%d...\n", OwnerName(),ci._vp,ci._vc);
	
    diag("cc_layer.call_control", DIAG_DEBUG, "%s: Sending CallProc to API...\n", OwnerName());
    ForwardMessageToUNI( callproc );
  } else
    diag("cc_layer.call_control", DIAG_DEBUG, "%s: Interrupted by unknown SimEvent with code %d", OwnerName(), e->GetCode());
}

//-----------------------------------------------------------------------
call_control::~call_control() { } 

//-----------------------------------------------------------------------
bool call_control::is_related(const sean_Visitor * sv)
{
  bool rval = false;

  if (_location == USER_SIDE) {
    // return TRUE if both  app_id and cid match
    const abstract_local_id * id = sv->get_shared_app_id();
    if(id == 0)
      return false;
    dic_item di = _callids.lookup(id);
    if (!di)
      rval = false;
    else {
      list<call_info *> * lst = _callids.inf(di);
      call_info ci ( sv->get_cid() , sv->get_crv() );
      list_item li = lst->search(&ci);
      if (li)
	rval = true;
      else {
	rval = false;
      }
    }

#ifdef _RELATE_DEBUG_    
    DIAG("cc_layer.call_control",DIAG_DEBUG,                                                      \
	 {                                                                         \
  	   if (!rval)                                                              \
             printf("USER call_control_%s IS_RELATED FAILS...\n", OwnerName());    \
           if (rval)                                                               \
             printf("USER call_control_%s IS_RELATED SUCCEEDS...\n", OwnerName()); \
 	   printf("call_control_%s: the _callids dictionary contains cids: ",      \
		  OwnerName());                                                    \
           dic_item dit;                                                           \
	   list_item lit;                                                          \
	   forall_items(dit , _callids) {                                          \
	     list<call_info *> * lst = _callids.inf(dit);                          \
	     forall_items(lit , *lst) {                                            \
	     call_info & ci = *(lst->inf(lit));                                    \
		 printf("%d  ",ci._cid);                                           \
	     }                                                                     \
	   }                                                                       \
	   printf(".\nUSER The sean_visitor call_id = %d\n", sv->get_cid());       \
	  }                                                                        \
         );
#endif

  } else if (_location == NETWORK_SIDE) {
    // no abstract_local_id, so lookup based on cref
    int crv = sv->get_crv();
    dic_item di;
    if (!(di = _crv2callinf.lookup(crv)))
      rval = false;
    else 
      rval = true;


#ifdef _RELATE_DEBUG_    
    DIAG("cc_layer.call_control",DIAG_DEBUG,                                                      \
	 {                                                                         \
  	   if (!rval)                                                              \
             printf("NET call_control_%s IS_RELATED FAILS...\n", OwnerName());     \
           if (rval)                                                               \
             printf("NET call_control_%s IS_RELATED SUCCEEDS...\n", OwnerName());  \
 	   printf("call_control_%s: the _crv2callinf dictionary contains crefs: ", \
		  OwnerName());                                                    \
           dic_item dit;                                                           \
	   forall_items(dit , _crv2callinf) {                                      \
 	     int cref = _crv2callinf.key(dit);                                     \
 	     printf("%lx  ",cref);                                                  \
	   }                                                                       \
	   printf(".\nNET The sean_visitor cref = %lx\n", sv->get_crv());           \
	  }                                                                        \
         );
#endif
  }
  return rval;
}

//-----------------------------------------------------------------------
bool call_control::relate(const sean_Visitor * sv)
{
  bool rval = false;
  list<call_info *> * lst = 0;
  if (_location == USER_SIDE) {
      const abstract_local_id * id = sv->get_shared_app_id();
      dic_item di = _callids.lookup(id);
      if (!di) {
	lst = new list<call_info *>;
	_callids.insert(id, lst);
      } else
	lst = _callids.inf(di);

      call_info * ci = new call_info( sv->get_cid() , sv->get_crv() );
      list_item li = lst->search(ci);
      assert(!li);
      lst->insert(ci);

      diag("cc_layer.call_control",DIAG_INFO,"%s: got a setup_req registered sv:app_id = %lx  sv:cid = %d\n",
	   OwnerName(),sv->get_shared_app_id(),sv->get_cid());
      rval = true;


#ifdef _RELATE_DEBUG_    
      DIAG("cc_layer.call_control",DIAG_DEBUG,                                                    \
	 {                                                                         \
  	   if (!rval)                                                              \
             printf("USER call_control_%s RELATE FAILS...\n", OwnerName());        \
           if (rval)                                                               \
             printf("USER call_control_%s RELATE SUCCEEDS...\n", OwnerName());     \
	  printf("call_control_%s: the _callids dictionary contains cids: ",       \
		  OwnerName());                                                    \
           dic_item dit;                                                           \
	   list_item lit;                                                          \
	   forall_items(dit , _callids) {                                          \
	     list<call_info *> * lst = _callids.inf(dit);                          \
	     forall_items(lit , *lst) {                                            \
	     call_info &ci = *(lst->inf(lit));                                     \
		 printf("%d  ",ci._cid);                                           \
	     }                                                                     \
	   }                                                                       \
	   printf(".\nUSER The sean_visitor call_id = %d\n", sv->get_cid());       \
	  }                                                                        \
         );
#endif

    } else if (_location == NETWORK_SIDE) {
      // no abstract_local_id, so lookup based on cref
      int crv = sv->get_crv();
      dic_item di;
      assert (!(di = _crv2callinf.lookup(crv)));

      call_info * ci = new call_info( sv->get_cid(), sv->get_crv() );
      di = _crv2callinf.insert(crv, ci );
      rval = true;

#ifdef _RELATE_DEBUG_    
    DIAG("cc_layer.call_control",DIAG_DEBUG,                                                         \
	 {                                                                            \
  	   if (!rval)                                                                 \
             printf("NET call_control_%s RELATE FAILS...\n", OwnerName());            \
           if (rval)                                                                  \
             printf("call_control_%s RELATE SUCCEEDS...\n", OwnerName());             \
	  printf("NET call_control_%s: the _crv2callinf dictionary contains crefs: ", \
		  OwnerName());                                                       \
           dic_item dit;                                                              \
	   forall_items(dit , _crv2callinf) {                                         \
	     int cref = _crv2callinf.key(dit);                                        \
	     printf("%lx  ",cref);                                                     \
	   }                                                                          \
	   printf(".\nNET The sean_visitor cref = %lx\n", sv->get_crv());              \
	  }                                                                           \
         );
#endif

      }
  return rval;
} 


//-----------------------------------------------------------------------
bool call_control::unrelate(const sean_Visitor * sv)
{
  bool rval = false;

  if (_location == USER_SIDE) {
    const abstract_local_id * id = sv->get_shared_app_id();
    dic_item di = _callids.lookup(id);
    assert (di);

    list<call_info *> * lst = _callids.inf(di);
    call_info ci ( sv->get_cid() , sv->get_crv() );
    list_item li = lst->search(&ci);
    assert(li);
    delete lst->inf(li);
    lst->del_item(li);
    rval = true;

#ifdef _RELATE_DEBUG_    
    DIAG("cc_layer.call_control",DIAG_DEBUG,                                                      \
	 {                                                                         \
  	   if (!rval)                                                              \
             printf("USER call_control_%s UNRELATE FAILS...\n", OwnerName());      \
           if (rval)                                                               \
             printf("USER call_control_%s UNRELATE SUCCEEDS...\n", OwnerName());   \
	   printf("call_control_%s: the _callids dictionary contains cids: ",      \
		  OwnerName());                                                    \
           dic_item dit;                                                           \
	   list_item lit;                                                          \
	   forall_items(dit , _callids) {                                          \
	     list<call_info *> * lst = _callids.inf(di);                           \
	     forall_items(lit , *lst) {                                            \
	     call_info &ci = *(lst->inf(lit));                                     \
		 printf("%d  ",ci._cid);                                           \
	     }                                                                     \
	   }                                                                       \
	   printf(".\nUSER The sean_visitor call_id = %d\n", sv->get_cid());       \
	  }                                                                        \
         );
#endif

  } else if (_location == NETWORK_SIDE) {
    // no abstract_local_id, so lookup based on cref
    int crv = sv->get_crv();
    dic_item di;
    assert (di = _crv2callinf.lookup(crv));
    delete _crv2callinf.inf(di);
    _crv2callinf.del_item(di);
    rval = true;

#ifdef _RELATE_DEBUG_    
    DIAG("cc_layer.call_control",DIAG_DEBUG,                                       \
	 {                                                                         \
  	   if (!rval)                                                              \
             printf("NET call_control_%s UNRELATE FAILS...\n", OwnerName());       \
           if (rval)                                                               \
             printf("NET call_control_%s UNRELATE SUCCEEDS...\n", OwnerName());    \
	   printf("call_control_%s: the _crv2callinf dictionary contains crefs: ", \
		  OwnerName());                                                    \
           dic_item dit;                                                           \
	   forall_items(dit , _crv2callinf) {                                      \
	     int cref = _crv2callinf.key(dit);                                     \
	     printf("%lx  ",cref);                                                 \
	   }                                                                       \
	   printf(".\nNET The sean_visitor cref = %lx\n", sv->get_crv());          \
	  }                                                                        \
         );
#endif

  }
  return rval;
}


//-----------------------------------------------------------------------
// returns the ACTUAL call_info object maintained by the call_control
// precondition: is_related(sv) is true.
//-----------------------------------------------------------------------
call_info & call_control::get_related_call_info(const sean_Visitor * sv) const
{
  assert( _location == USER_SIDE ||
	  _location == NETWORK_SIDE );

  if (_location == USER_SIDE) {
    const abstract_local_id * id = sv->get_shared_app_id();
    dic_item di = _callids.lookup(id);
    assert (di);
    list<call_info *> * lst = _callids.inf(di);
    call_info ci(sv->get_cid(), sv->get_crv());
    list_item li = lst->search(&ci);
    return *(lst->inf(li));
  }

  // See assert above if you think this is wrong ...
  // no abstract_local_id, so lookup based on cref
  int crv = sv->get_crv();
  dic_item di = _crv2callinf.lookup(crv);
  assert (di);
  return *(_crv2callinf.inf(di));
}

serv_info & call_control::get_related_serv_info(const sean_Visitor * sv) const
{
  const abstract_local_id * id = sv->get_shared_app_id();
  dic_item di = _servids.lookup(id);
  assert(di != 0);
  list<serv_info *> * lst = _servids.inf(di);
  serv_info si( sv->get_sid() );
  list_item li = lst->search(&si);
  return *(lst->inf(li));
}

//-----------------------------------------------------------------------
bool call_control::attempt_final_app_death_cleanup(const abstract_local_id * id) {

  if (!id) return true;

  dic_item dit = _app_alive.lookup(id);
  assert( dit );

  dic_item cdi = _callids.lookup(id);
  assert( cdi );
  
  dic_item sdi = _servids.lookup(id);
  assert( sdi );

  bool ret;
  if ( (_app_alive.access(id) == 0) &&
       (_servids.inf(sdi)->size() == 0) &&
       (_callids.inf(cdi)->size() == 0)  ) {
    list<serv_info *> *lst = _servids.inf(sdi);
    lst->clear();
    delete lst;
    _servids.del_item(sdi);

    list<call_info *> *lst2 = _callids.inf(cdi);
    lst2->clear();
    delete lst2;
    _callids.del_item(cdi);

    _app_alive.del_item(dit);
    diag("cc_layer.call_control", DIAG_INFO,
	 "Cleanup procedures in response to app-death are complete.  We're in the money!\n");
    diag("cc_layer.call_control", DIAG_INFO,
	 "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
    ret = true;
  }
  else {
    diag("cc_layer.call_control",DIAG_INFO,"%s: app is in state alive=%d, #servids=%d, #callids=%d\n", 
	 OwnerName(), _app_alive.access(id), _servids.inf(sdi)->size(), _callids.inf(cdi)->size());
    ret = false;
  }
  
  return ret;
}


void call_control::CheckP2MP(sean_Visitor * sv)
{
  if (is_related(sv)) {
    call_info & ci = get_related_call_info(sv);
    if (ci._p2mp)
      sv->SetP2MP(true);
  }
  else {
    generic_q93b_msg * msg = sv->share_message();
  
    InfoElem ** ies;
    if (msg) 
      ies = msg->get_ie_array();

    if (((msg) && (ies[InfoElem::ie_end_pt_ref_ix]))
	||
	(sv->get_epr() != NO_EPR))
      {
	// P2MP indeed.      DON'T SET THE EPR/PID HERE!
	sv->SetP2MP(true);
      }
  }
}


bool call_control::streamfilter_data_query(int vpi, int vci, int& cid, abstract_local_id*& locid) 
{
  dic_item app;
  forall_items(app,_callids) {
    list<call_info *> * call_lst = _callids.inf(app);
    list_item call;
    forall_items(call, *call_lst) {
      call_info & cinfo = *(call_lst->inf(call));
      if ((cinfo._vp == vpi) && (cinfo._vc == vci)) {
	cid = cinfo._cid;
	locid = (abstract_local_id *)_callids.key(app);
	return true;
      }
    }
  }
  return false;
}


