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

// -*-C++-*-
#ifndef LINT
static char const _Q93bCall_cc_rcsid_[] =
"$Id: Q93bCall.cc,v 1.26 1999/04/08 14:17:35 battou Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/basics/diag.h>
#include <FW/kernel/SimEvent.h>
#include <iostream.h>

#include <sean/daemon/ccd_config.h>
#include <codec/q93b_msg/q93b_msg.h>
#include <sean/templates/constants.h>
#include <sean/cc/sean_Visitor.h>
#include "Q93bTimers.h"
#include "Q93bState.h"
#include "Q93bCall.h"
#include "Q93bParty.h"
#include "Q93bLeaf.h"
#include "PartyState.h"

#define Q93B_DEBUGGING "fsm.uni.Q93B_debugging"

// -----------------------------------------
Call::Call(int un) : State(), _user(un), _call_type(p2p), 
  _call_state(ie_call_state::u0_null), _active_timer(0L), 
  _vpi(0), _vci(NO_VCI), _t301(0), _t303(0), _t308(0), 
  _t309(0), _t310(0), _t313(0), _t316(0), _t317(0),
  _t322(0), _me(0)
{
  _leaf_cnt = 0;
  zero_ids();
  for (int i = 0; i < num_ie; i++)
    _ie_array[i] = 0L;
  _t301 = new Q93b_t301(this);
  _t303 = new Q93b_t303(this);
  _t308 = new Q93b_t308(this);
  _t309 = new Q93b_t309(this);
  _t310 = new Q93b_t310(this);
  _t313 = new Q93b_t313(this);
  _t316 = new Q93b_t316(this);
  _t317 = new Q93b_t317(this);
  _t322 = new Q93b_t322(this);
  _cs = NullState::Instance();
  if (_user == USER_SIDE) {
    _un = ie_cause::user;
    DIAG("fsm.uni", DIAG_DEBUG, cout <<
	 OwnerName() << " Creating a user-side UNI Call" <<
	 " at time " << theKernel().CurrentElapsedTime() << endl);
  } else {
    _un = ie_cause::local_private_network;
    DIAG("fsm.uni", DIAG_DEBUG, cout <<
	 OwnerName() << " Creating a net-side UNI Call" <<
	 " at time " << theKernel().CurrentElapsedTime() << endl);
  }
}

Call::~Call() { }

ie_cause::Location Call::StackType(void) { return _un; }

void Call::ChangeState(Q93bState * s) { 
  _cs = s; 
  _call_state = _cs->StateID();
}

// there is probably more to do here
void Call::suicide(void)
{
  register int i;
  for (i = 0; i < num_ie; i++) {
    if (_ie_array[i]) {
      delete _ie_array[i];
      _ie_array[i] = 0L;
    }
  }
  if (_t301) 
    { _t301->StopTimer();delete _t301; _t301=0L; }
  if (_t303)
    {_t303->StopTimer();delete _t303; _t303=0L;}
  if (_t308)
    {_t308->StopTimer();delete _t308; _t308=0L;}
  if (_t309)
    {_t309->StopTimer();delete _t309; _t309=0L;}
  if (_t310)
    {_t310->StopTimer();delete _t310; _t310=0L;}
  if (_t313)
    {_t313->StopTimer();delete _t313; _t313=0L;}
  if (_t316)
    {_t316->StopTimer();delete _t316; _t316=0L;}
  if (_t317)
    {_t317->StopTimer();delete _t317; _t317=0L;}
  if (_t322)
    {_t322->StopTimer();delete _t322; _t322=0L;}

  Free();  // mark for deletion
}

void Call::KillParty(Party *p)
{
  int epr = p->get_epr();
  dic_item it = _party_map.lookup(epr);
  if (it) {
    delete _party_map.inf(it);
    _party_map.del_item(it);
  }
}

// returns true if there exists a party other than epr in state ps
// to check for all parties pass -1 as epr
bool Call::AnyParty(ie_end_pt_state::party_state ps, int epr)
{
  if(IsP2MP())
    {
      dic_item it;
      forall_items(it,_party_map)
	{
	  Party *party = _party_map.inf(it);
	  assert(party != 0);
	  if(party->get_epr() == epr)
	    continue;
	  if(party->get_eps() == ps)
	    return true;
	}
    }
  return false;
}

// call is busy if epr and another party are in state
//  active || party_alerting_delivered || add_party_initiated
bool Call::IsCallBusy(int epr)
{
  Party *p = 0;
  dic_item it;
  if (it = _party_map.lookup(epr))
    p = _party_map.inf(it);
  assert(p != 0);
  ie_end_pt_state::party_state ps = p->get_eps();
  if((ps == ie_end_pt_state::p_active) ||
     (ps == ie_end_pt_state::party_alert_delivered) ||
     (ps == ie_end_pt_state::add_party_initiated))
    if(AnyParty(ie_end_pt_state::p_active,epr) ||
       AnyParty(ie_end_pt_state::party_alert_delivered,epr) ||
       AnyParty(ie_end_pt_state::add_party_initiated,epr)
       )
      return true;
  return false;
}

// call is idle if all the other parties are in state
// null || drop_party_initiated || drop_party_received
// that is if we find 1 party in
//    add_party_initiated ||
//    party_alert_delivered ||
//    add_party_received ||
//    party_alert_received ||
//    p_active 
// the call is not idle and return false

bool Call::IsCallIdle(int epr)
{
  if(AnyParty(ie_end_pt_state::add_party_initiated,epr) ||
     AnyParty(ie_end_pt_state::party_alert_delivered,epr) ||
     AnyParty(ie_end_pt_state::add_party_received,epr) ||
     AnyParty(ie_end_pt_state::party_alert_received,epr) ||
     AnyParty(ie_end_pt_state::p_active,epr)
     )
    return false;
  return true;
}

// Q.2971 9.5.3.2.1, 9.5.3.2.2, 9.5.3.2.3 is not implemented
// instead  validate EPR results in YES (is party exists) or CLR (party does not exists)
// to be FIXED

int Call::ValidateEPR(ie_cause::CauseValues & cv, Visitor *v)
{
  const VisitorType * vt2 = QueryRegistry(SEAN_VISITOR_NAME);
  VisitorType vt1 = v->GetType();
  if (vt2 && vt1.Is_A(vt2))
    {
      sean_Visitor *qv = (sean_Visitor *)v;
      generic_q93b_msg * msg = qv->share_message();
      assert(msg != 0);
      int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
      Party *p = GetParty(epr);
      if(p == 0)
	{
	  cv = ie_cause::invalid_endpoint_reference;
	  return CLR;
	}
      else
	return YES;
    }
  cv = ie_cause::normal_unspecified;
  return CLR;
}


int Call::VerifyMessage(ie_cause::CauseValues & cv, Visitor *v)
{
  const VisitorType * vt2 = QueryRegistry(SEAN_VISITOR_NAME);
  VisitorType vt1 = v->GetType();
  if (vt2 && vt1.Is_A(vt2))
    {
      sean_Visitor *qv = (sean_Visitor *)v;
      generic_q93b_msg * msg = qv->share_message();
      // TO FIX:
      // get header_parser::_errors & body_parser::_errors and synthesize the
      // errors in trems of OK,I,CLR,RAI,RAP
      // header_parser_errmsg * e1 = msg->header_parser::_errors;
      // body_parser_errmsg *e2 = msg->body_parser::_errors;
      return OK;
    }
  // need to make a cause value based on the result of the synthesis
  cv = ie_cause::normal_unspecified;
  return CLR;
}

bool Call::IsTimer322Active() { return _t322->IsActive(); }

State * Call::Handle(Visitor * v)
{
  const VisitorType * vt2 = QueryRegistry(SEAN_VISITOR_NAME);
  VisitorType vt1 = v->GetType();
  char name[80];
  int ret = 0;
  if (_cs)
    _cs->StateName(name);
  if (vt2 && vt1.Is_A(vt2)) {
    sean_Visitor *qv = (sean_Visitor *)v;
    DIAG("fsm.uni", DIAG_DEBUG, cout <<
	 OwnerName() << " received a " << qv->GetType() << 
	 " v = " << qv <<
	 " in state = " << name <<
	 " at time " << theKernel().CurrentTime() << endl);
    if ((ret = CallProtocol(qv)) == -2)
      return 0;
    // else?
  } else
    PassThru(v);
  return this;
}

void  Call::Interrupt(SimEvent *event)
{
  DIAG(SIM, DIAG_DEBUG, 
       cout << OwnerName() << " interrupted by " << (int)event << endl;);
}

Party * Call::GetParty(int epr)
{
  dic_item it;
  if (it = _party_map.lookup(epr))
    return _party_map.inf(it);
  return 0;
}

int Call::NumParties()
{
  return _party_map.size();
}

int Call::NumLeaves()
{
  return _leaf_map.size();
}

Leaf * Call::GetLeaf(int seqnum)
{
  dic_item it;
  if (it = _leaf_map.lookup(seqnum))
    return _leaf_map.inf(it);
  return 0;
}

void Call::Save_CID(int cid)                           { _cid = cid;     }
void Call::Save_APPID(const abstract_local_id * appid) { _appid = appid; }

void Call::Stamp_CID(sean_Visitor * qv)   const { qv->set_cid(_cid);      }
void Call::Stamp_APPID(sean_Visitor * qv) const { qv->set_shared_app_id(_appid); }

void Call::zero_ids(void) 
{
  _cid   = NO_CID;
  _appid = 0;
}

ie_call_state::call_state_values Call::GetCallState() {return _call_state;}

void Call::SetCallState(ie_call_state::call_state_values call_state) {_call_state = call_state;}

int Call::IsCallNegotiating()
{
  if(_ie_array[InfoElem::UNI40_min_traff_desc_ix] || _ie_array[InfoElem::UNI40_alt_traff_desc_ix])
    return 1;
  return 0;
}

int Call::IsP2P()
{
  if (_call_type == p2p)
    return 1;
  return 0;
}

int Call::IsP2MP()
{
  if (_call_type != p2p)
    return 1;
  return 0;
}

int Call::IsRootLIJ()
{
  if (_call_type == p2mp_lij_root)
    return 1;
  return 0;
}

int Call::IsNetLIJ()
{
  if (_call_type == p2mp_lij_net)
    return 1;
  return 0;
}

Q93bState * Call::GetCS(void) { return _cs; }

Party * Call::Init(sean_Visitor * qv)
{
  assert(qv != 0);
  Party * party = 0;
  generic_q93b_msg * msg = qv->share_message();
  assert(msg != 0);
  InfoElem * an_ie;
  for (int i = 0; i < num_ie; i++) {
    if (an_ie = msg->ie((InfoElem::ie_ix )i)) {
      _ie_array[i] = an_ie->copy();
    }
  }

  ie_bbc *bbc_ie = (ie_bbc *)_ie_array[InfoElem::ie_broadband_bearer_cap_ix];
  if (bbc_ie->get_conn_type() == ie_bbc::p2mp) {
      // use 6.2.2.1 page 50 of UNI-4.0 to check if it is an LIJ Call
      // Net LIJ requires CallingPartyNumber & LIJ params & LIJ callid
      if (_ie_array[InfoElem::UNI40_leaf_params_ix]) {
	// Net LIJ
	if (!_ie_array[InfoElem::UNI40_leaf_call_id_ix] || 
	    !_ie_array[InfoElem::ie_calling_party_num_ix]) {
	  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Mandatory IE is missing for an LIJ net call\n"); 
	  return NULL;
	}
	_call_type = p2mp_lij_net;
      } else {
	// root LIJ
	_call_type = p2mp_lij_root;
      }
      if (IsP2MP())
	party = InitParty(qv);
      if (_ie_array[InfoElem::UNI40_leaf_sequence_num_ix]) {
	int seqnum = ((UNI40_leaf_sequence_num *)_ie_array[InfoElem::UNI40_leaf_sequence_num_ix])->getSeqNum();
	if (party)
	  party->set_seqnum(seqnum);
      }
  }
  _crv = qv->get_crv();
  return (party);
}

Leaf * Call::InitLeaf(sean_Visitor * qv)
{
  assert(qv != 0);
  Leaf * leaf = 0;
  generic_q93b_msg * msg = qv->share_message();
  assert(msg != 0);
  InfoElem* an_ie;
  for (int i = 0; i < num_ie; i++)
    if (an_ie=msg->ie((InfoElem::ie_ix )i))
      _ie_array[i] = an_ie->copy();
  if (!_ie_array[InfoElem::ie_calling_party_num_ix] ||
     !_ie_array[InfoElem::ie_called_party_num_ix] ||
     !_ie_array[InfoElem::UNI40_leaf_call_id_ix]  ||
     !_ie_array[InfoElem::UNI40_leaf_sequence_num_ix]
     ) {
    DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Mandatory IE is missing for an LIJ setupReq\n"); 
    return NULL;
  }
  int seqnum = ((UNI40_leaf_sequence_num *)_ie_array[InfoElem::UNI40_leaf_sequence_num_ix])->getSeqNum();
  leaf = new Leaf(this,seqnum);
  assert(leaf != 0);
  _leaf_map.insert(seqnum,leaf);
  ie_called_party_subaddr *subaddr = NULL;
  if (subaddr = (ie_called_party_subaddr *)_ie_array[InfoElem::ie_called_party_subaddr_ix])
    leaf->set_called_subaddr(subaddr);
  ie_called_party_num *number = (ie_called_party_num *)_ie_array[InfoElem::ie_called_party_num_ix];
  leaf->set_called_number(number);
  _crv = qv->get_crv(); // dummy cref
  _call_type = p2mp_lij_net;
  _leaf_cnt++;
  return(leaf);
}

Party * Call::InitParty(sean_Visitor * qv)
{
  Party * party = 0;
  generic_q93b_msg *msg = qv->share_message();
  assert(msg != 0);
  InfoElem **ies = msg->get_ie_array();
  if (ies[InfoElem::ie_called_party_num_ix] == 0) {
    DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Mandatory IE Called Party Number missing in AddPartyReq\n");
    qv->Suicide();
    return 0;
  }
  ie_called_party_num *number = (ie_called_party_num *)ies[InfoElem::ie_called_party_num_ix]->copy();
  if (ies[InfoElem::ie_end_pt_ref_ix] == 0) {
    DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Mandatory IE epr missing in AddPartyReq\n");
    qv->Suicide();
    return 0;
  }
  ie_end_pt_ref  *epr = (ie_end_pt_ref  *)ies[InfoElem::ie_end_pt_ref_ix]->copy();
  party =  new Party(this,number,epr);
  assert(party != 0);
  int eprv = epr->get_epr_value();
  qv->set_pid(eprv);
  _party_map.insert(eprv,party);
  ie_called_party_subaddr *subaddr = NULL;
  if (subaddr = (ie_called_party_subaddr *)ies[InfoElem::ie_called_party_subaddr_ix])
    party->set_called_subaddr(subaddr);
  return party;
}

void  Call::SendCallStatusToPeer(ie_call_state::call_state_values cs,ie_cause::CauseValues cv)
{
  InfoElem *ie_array[num_ie];
  for(int i=0;i < num_ie;i++)
    ie_array[i] = NULL;
  ie_array[InfoElem::ie_cause_ix] = new ie_cause(cv,_un);
  ie_array[InfoElem::ie_call_state_ix] = new ie_call_state(cs);
  generic_q93b_msg *msg = new q93b_status_message(ie_array, _crv, 0);
  sean_Visitor *qv = new sean_Visitor(sean_Visitor::status_req, msg);
  qv->set_crv(_crv);
  PassVisitorToB(qv);
}

void  Call::SendPartyStatusToPeer(ie_cause::CauseValues cv, ie_end_pt_state::party_state ps, int epr)
{
  InfoElem *ie_array[num_ie];
  for (int i=0;i < num_ie;i++)
    ie_array[i] = NULL;
  ie_array[InfoElem::ie_cause_ix] = new ie_cause(cv,_un);
  ie_array[InfoElem::ie_call_state_ix] = new ie_call_state(GetCallState());
  ie_array[InfoElem::ie_end_pt_ref_ix] = new ie_end_pt_ref(epr);
  ie_array[InfoElem::ie_end_pt_state_ix] = new ie_end_pt_state(ps);
  generic_q93b_msg *msg = new q93b_status_message(ie_array, _crv, 0);
  sean_Visitor *qv = new sean_Visitor(sean_Visitor::status_req, msg);
  qv->set_crv(_crv);
  PassVisitorToB(qv);
}


void  Call::SendConnectAckToPeer()
{
  InfoElem *ie_array[num_ie];
  for(int i=0;i < num_ie;i++)
    ie_array[i] = NULL;
  generic_q93b_msg *msg = new q93b_connect_ack_message(ie_array, _crv, 0);
  sean_Visitor *qv = new sean_Visitor(sean_Visitor::connect_ack_req, msg);
  qv->set_crv(_crv);
  PassVisitorToB(qv);
}


void  Call::SendDPAKToPeer(int epr, ie_cause::CauseValues cv)
{
  InfoElem *ie_array[num_ie];
  for(int i=0;i < num_ie;i++)
    ie_array[i] = NULL;
  ie_array[InfoElem::ie_cause_ix] = new ie_cause(cv,_un);
  Party *party = GetParty(epr);
  assert(party != 0);
  party->GetPS()->DropPartyReq(0,party);
  ie_array[InfoElem::ie_end_pt_ref_ix] = new ie_end_pt_ref(epr);
  generic_q93b_msg *msg = new q93b_drop_party_ack_message(ie_array, _crv, 0);
  sean_Visitor *qv = new sean_Visitor(sean_Visitor::drop_party_ack_req, msg);
  qv->set_pid(epr);
  qv->set_crv(_crv);
  PassVisitorToB(qv);
}

void  Call::SendDPAKToCCD(int epr, ie_cause::CauseValues cv)
{
  InfoElem *ie_array[num_ie];
  for(int i=0;i < num_ie;i++)
    ie_array[i] = NULL;
  ie_array[InfoElem::ie_cause_ix] = new ie_cause(cv,_un);
  ie_array[InfoElem::ie_end_pt_ref_ix] = new ie_end_pt_ref(epr);
  generic_q93b_msg *msg = new q93b_drop_party_ack_message(ie_array, _crv, 0);
  sean_Visitor *qv = new sean_Visitor(sean_Visitor::drop_party_ack_ind, msg);
  qv->set_pid(epr);
  qv->set_crv(_crv);
  PassVisitorToA(qv);
}

void  Call::SendDPToPeer(int epr, ie_cause::CauseValues cv)
{
  InfoElem *ie_array[num_ie];
  for(int i=0;i < num_ie;i++)
    ie_array[i] = NULL;
  ie_array[InfoElem::ie_cause_ix] = new ie_cause(cv,_un);
  ie_array[InfoElem::ie_end_pt_ref_ix] = new ie_end_pt_ref(epr);
  generic_q93b_msg *msg = new q93b_drop_party_message(ie_array, _crv, 0);
  sean_Visitor *qv = new sean_Visitor(sean_Visitor::drop_party_req, msg);
  qv->set_pid(epr);
  qv->set_crv(_crv);
  PassVisitorToB(qv);
}

void  Call::SendAPRejToPeer(int epr, ie_cause::CauseValues cv)
{
  InfoElem *ie_array[num_ie];
  for(int i=0;i < num_ie;i++)
    ie_array[i] = NULL;
  ie_array[InfoElem::ie_cause_ix] = new ie_cause(cv,_un);
  ie_array[InfoElem::ie_end_pt_ref_ix] = new ie_end_pt_ref(epr);
  generic_q93b_msg *msg = new q93b_add_party_rej_message(ie_array, _crv, 0);
  sean_Visitor *qv = new sean_Visitor(sean_Visitor::add_party_rej_req, msg);
  qv->set_pid(epr);
  qv->set_crv(_crv);
  PassVisitorToB(qv);
}

void  Call::SendDPIndToCCD(int epr, ie_cause::CauseValues cv)
{
  InfoElem *ie_array[num_ie];
  for(int i=0;i < num_ie;i++)
    ie_array[i] = NULL;
  ie_array[InfoElem::ie_cause_ix] = new ie_cause(cv,_un);
  ie_array[InfoElem::ie_end_pt_ref_ix] = new ie_end_pt_ref(epr);
  generic_q93b_msg *msg = new q93b_drop_party_message(ie_array, _crv, 0);
  sean_Visitor *qv = new sean_Visitor(sean_Visitor::drop_party_ind, msg);
  qv->set_pid(epr);
  Send2CCD(qv);
}


// top of call is A while bottom of call is B
void  Call::Send2Peer(Visitor *v)
{
  PassVisitorToB(v);
}

void  Call::Send2CCD(Visitor * v)
{
  const VisitorType * vt2 = QueryRegistry(SEAN_VISITOR_NAME);
  VisitorType vt1 = v->GetType();
  if (vt2 && vt1.Is_A(vt2))
    {
      sean_Visitor* qv=(sean_Visitor*)v;
      this->Stamp_APPID(qv);
      this->Stamp_CID(qv);
      if(_un == ie_cause::local_private_network)
	{
	  qv->set_vpi(_vpi);
	  qv->set_vci(_vci);
	  qv->set_crv(_crv);
	  qv->set_iport(_port);
	}
      char name[80];
      qv->Name(name);
      DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout << OwnerName() << " : Sending to CCD a " << name << " vci = " << _vci << endl);
    }
  PassVisitorToA(v);
}

void Call::SendSetupToPeer()
{
  generic_q93b_msg *msg = new q93b_setup_message(_ie_array, _crv, 0);
  sean_Visitor *qv = new sean_Visitor(sean_Visitor::setup_req, msg);
  qv->set_crv(_crv);
  Send2Peer(qv);
  SetT303();
}

void Call::SendStatusEnqToPeer()
{
  InfoElem *ie_array[num_ie];
  for(int i=0;i < num_ie;i++)
    ie_array[i] = NULL;
  generic_q93b_msg *msg = new q93b_status_enq_message(ie_array, _crv, 0);
  sean_Visitor *qv = new sean_Visitor(sean_Visitor::status_enq_req, msg);
  qv->set_crv(_crv);
  Send2Peer(qv);
  SetT322();
}

void Call::SendLeafSetupToPeer(int leaf_seqnum)
{
  dic_item it = _leaf_map.lookup(leaf_seqnum);
  if (it) {
    Leaf *leaf = _leaf_map.inf(it);
    ((UNI40_leaf_sequence_num *)_ie_array[InfoElem::UNI40_leaf_sequence_num_ix])->setSeqNum(leaf_seqnum);
    generic_q93b_msg *msg = new UNI40_leaf_setup_request_message(_ie_array, _crv, 0);
    sean_Visitor *qv = new sean_Visitor(sean_Visitor::leaf_setup_req, msg);
    qv->set_crv(_crv);
    Send2Peer(qv);
    SetT331(leaf);
  }
}


void Call::KillLeaf(int seqnum)
{
  dic_item it = _leaf_map.lookup(seqnum);
  if (it) {
    delete _leaf_map.inf(it);
    _leaf_map.del_item(it);
  }
}

void Call::ReleaseAllParties(ie_cause::CauseValues cv)
{
  if (IsP2MP()) {
    dic_item it;
    forall_items(it,_party_map) {
      Party *party = _party_map.inf(it);
      assert(party != 0);
      party->GetPS()->ReleaseReq(0,party);
      delete party;
    }
    _party_map.clear();
  }
}

void Call::DropAllParties(ie_cause::CauseValues cv)
{
  if (IsP2MP())
    {
      dic_item it;
      int ct = _party_map.size();

      forall_items(it,_party_map)
	{
	  Party *party = _party_map.inf(it);
	  assert(party != 0);
	  ie_end_pt_state::party_state ps = party->get_eps();
          if((ps == ie_end_pt_state::p_active) ||
             (ps == ie_end_pt_state::add_party_received) ||
             (ps == ie_end_pt_state::add_party_initiated) // no call reoffering
             )
            {
              party->GetPS()->DropPartyReq(0,party);
	      // Send the DP to all but the last party!
	      if ((it != _party_map.first()) && (_user))    // _CRAP_
		SendDPToPeer(party->get_epr(),cv);         // _CRAP_
	    }
	}
    }
}

void Call::SendReleaseToPeer(ie_cause::CauseValues cv)
{
  InfoElem *ie_array[num_ie];
  for(int i=0;i < num_ie;i++)
    ie_array[i] = NULL;
  ie_array[InfoElem::ie_cause_ix] = new ie_cause(cv,_un);
  generic_q93b_msg *msg =  new q93b_release_message(ie_array, _crv, 0);
  sean_Visitor *qv = new sean_Visitor(sean_Visitor::release_req, msg);
  qv->set_crv(_crv);
  Send2Peer(qv);
}

void Call::SendReleaseCompToPeer(ie_cause::CauseValues cv)
{
  InfoElem *ie_array[num_ie];
  for(int i=0;i < num_ie;i++)
    ie_array[i] = NULL;
  ie_array[InfoElem::ie_cause_ix] = new ie_cause(cv,_un);
  generic_q93b_msg *msg =  new q93b_release_comp_message(ie_array, _crv, 0);
  sean_Visitor *qv = new sean_Visitor(sean_Visitor::release_comp_req, msg);
  qv->set_crv(_crv);
  Send2Peer(qv);
}

void Call::SendReleaseIndToCCD(ie_cause::CauseValues cv)
{
  InfoElem *ie_array[num_ie];
  for(int i=0;i < num_ie;i++)
    ie_array[i] = NULL;
  ie_array[InfoElem::ie_cause_ix] = new ie_cause(cv,_un);
  generic_q93b_msg *msg =  new q93b_release_message(ie_array, _crv, 0);
  sean_Visitor *qv = new sean_Visitor(sean_Visitor::release_ind, msg);
  qv->set_crv(_crv);
  Send2CCD(qv);
}

void Call::SendReleaseCompIndToCCD(ie_cause::CauseValues cv)
{
  InfoElem *ie_array[num_ie];
  for(int i=0;i < num_ie;i++)
    ie_array[i] = NULL;
  ie_array[InfoElem::ie_cause_ix] = new ie_cause(cv,_un);
  generic_q93b_msg *msg =  new q93b_release_comp_message(ie_array, _crv, 0);
  sean_Visitor *qv = new sean_Visitor(sean_Visitor::release_comp_ind, msg);
  qv->set_crv(_crv);
  Send2CCD(qv);
}

// not totally right ! see page 148 Q.2971
void Call::DoUM(sean_Visitor *qv)
{
  assert(qv != 0);
  generic_q93b_msg *msg = qv->share_message();
  assert(msg != 0);
  ie_cause::CauseValues cv;
  switch(VerifyMessage(cv,qv))
    {
    case RAI: 
      SendCallStatusToPeer(_call_state,ie_cause::message_not_compatible_with_call_state);
      break;
    case CLR:
      if((_call_state != ie_call_state::u11_release_request) &&
	 (_call_state != ie_call_state::u12_release_indication))
	DoLabelD(cv);
      break;
    case I:
      break;
    }
  qv->Suicide();
}

void Call::DoLabelD(ie_cause::CauseValues cv)
{
  SendReleaseIndToCCD(cv);
  ResetAllTimers();
  SendReleaseToPeer(cv);
  SetT308();
  if(_un == ie_cause::user)
    ChangeState(ReleaseReqState::Instance());
  else
    ChangeState(ReleaseIndState::Instance());
}

void Call::RCNonActiveParties()
{
  if (IsP2MP()) {
    dic_item it;
    while (it = _party_map.first()) {
      Party * party = _party_map.inf(it);
      assert(party != 0);
      if (party->GetPS()->StateID() != ie_end_pt_state::p_active) {
	party->GetPS()->ReleaseCompReq(0,party);
	delete party;
      }
      dic_item prev_it = it;
      it = _party_map.next(prev_it);
      _party_map.del_item(prev_it);
    }
  }
}

void Call::RCParties()
{
  if (IsP2MP()) {
    dic_item it;
    forall_items(it,_party_map) {
      Party *party = _party_map.inf(it);
      assert(party != 0);
      party->GetPS()->ReleaseCompReq(0,party);
      delete party;
    }
    _party_map.clear();
  }
}

void Call::ConnAckParty()
{
  int cnt = 0;
  dic_item it;
  forall_items(it,_party_map) {
    Party *party = _party_map.inf(it);
    assert(party != 0);
    party->GetPS()->ConnectAckInd(0,party);
    cnt++;
  }
  if (cnt > 1)
    DIAG(Q93B_DEBUGGING, DIAG_DEBUG, 
	 cout <<  "Received a ConnectAckInd for a call with more than 1 party" << endl);
}

void Call::SendPartyStatusEnqReq(bool active_parties_only)
{
  generic_q93b_msg *msg = 0L;
  sean_Visitor *qv = 0L;
  InfoElem *ie_array[num_ie];
  for (int i=0;i < num_ie;i++)
    ie_array[i] = NULL;
  dic_item it;
  forall_items(it,_party_map) {
    Party *party = _party_map.inf(it);
    assert(party != 0);
    ie_array[InfoElem::ie_end_pt_ref_ix] = party->get_epr_ie();
    msg = new q93b_status_enq_message(ie_array, _crv, 0);
    qv = new sean_Visitor(sean_Visitor::status_enq_req, msg);
    party->GetPS()->StatusEnqReq(0,party);
  }
}


void Call::AlertParties()
{
  dic_item it;
  forall_items(it,_party_map) {
    Party * party = _party_map.inf(it);
    assert(party != 0);
    party->GetPS()->AlertReq(0,party);
  }
}

void Call::SetT301(void) { _active_timer = _t301; _t301->SetTimer(); }
void Call::SetT303(void) { _active_timer = _t303; _t303->SetTimer(); }
void Call::SetT308(void) { _active_timer = _t308; _t308->SetTimer(); }
void Call::SetT309(void) { _active_timer = _t309; _t309->SetTimer(); }
void Call::SetT310(void) { _active_timer = _t310; _t310->SetTimer(); }
void Call::SetT313(void) { _active_timer = _t313; _t313->SetTimer(); }
void Call::SetT316(void) { _active_timer = _t316; _t316->SetTimer(); }
void Call::SetT317(void) { _active_timer = _t317; _t317->SetTimer(); }
void Call::SetT322(void) { _active_timer = _t322; _t322->SetTimer(); }

void Call::ResetAllTimers()
{
  StopT301();
  StopT303();
  StopT308();
  StopT309();
  StopT310();
  StopT313();
  StopT316();
  StopT317();
  StopT322();
  if (IsP2MP()) {
    dic_item it;
    forall_items(it,_party_map) {
      Party *party = _party_map.inf(it);
      assert(party != 0);
      StopT397(party);
      StopT398(party);
      StopT399(party);
    }
  }
}

void Call::StopTimer(void)
{
  if (_active_timer) {
    Cancel(_active_timer);
    _active_timer = 0L;
  }
}

void Call::StopT301(void) {_active_timer = 0L; if (_t301) _t301->StopTimer(); }
void Call::StopT303(void) {_active_timer = 0L; if (_t303) _t303->StopTimer(); }
void Call::StopT308(void) {_active_timer = 0L; if (_t308) _t308->StopTimer(); }
void Call::StopT309(void) {_active_timer = 0L; if (_t309) _t309->StopTimer(); } 
void Call::StopT310(void) {_active_timer = 0L; if (_t310) _t310->StopTimer(); } 
void Call::StopT313(void) {_active_timer = 0L; if (_t313) _t313->StopTimer(); }
void Call::StopT316(void) {_active_timer = 0L; if (_t316) _t316->StopTimer(); }
void Call::StopT317(void) {_active_timer = 0L; if (_t317) _t317->StopTimer(); }
void Call::StopT322(void) {_active_timer = 0L; if (_t322) _t322->StopTimer(); }

int Call::q93b_t301_timeout()
{
  u_char timer_diag[3] = {'3','0','1'};
  _t301->_retries++;
  return (-1);
}

void Call::SetT397(Party *p){p->_active_timer = p->_t397; Register(p->_t397);}
void Call::SetT398(Party *p){p->_active_timer = p->_t398; Register(p->_t398);}
void Call::SetT399(Party *p){p->_active_timer = p->_t399; Register(p->_t399);}

void Call::SetT331(Leaf *l){Register(l->_t331);}

void Call::StopT331(Leaf *l)
{
  if (l != 0) {
    if(l->_t331)
      Cancel(l->_t331);
  }
}

void Call::StopTimer(Party *p)
{
  if (p != 0) {
    if (p->_active_timer) {
      Cancel(p->_active_timer);
      p->_active_timer = 0L;
    }
  }
}

void Call::StopT397(Party *p)
{
  if (p != 0) {
    p->_active_timer = 0L;
    if (p->_t397)
      Cancel(p->_t397);
  }
}

void Call::StopT398(Party *p)
{
  if (p) {
    p->_active_timer = 0L;
    if (p->_t398)
      Cancel(p->_t398);
  }
}

void Call::StopT399(Party *p)
{
  if (p != 0) {
    p->_active_timer = 0L;
    if (p->_t399)
      Cancel(p->_t399);
  }
}

int Call::q93b_t303_timeout()
{
  u_char timer_diag[3] = {'3','0','3'};
  _t303->_retries++;
  if (_t303->_retries < _t303->_max_retries)
    _cs->ExpT303(this);
  else {
    _call_state = ie_call_state::u0_null;
    _t303->_retries = 0;
    SendReleaseCompToPeer(ie_cause::recovery_on_timer_expiry);
    SendReleaseCompIndToCCD(ie_cause::recovery_on_timer_expiry);
    suicide();
  }
  return 0;
}


int Call::q93b_t308_timeout()
{
  u_char timer_diag[3] = {'3','0','8'};
  _t308->_retries++;
  if (StackType() == ie_cause::user) {
    if (_cs->StateID() == ie_call_state::u11_release_request) {
      // generate an ExpT308 event if first expiry
      if (_t308->_retries < _t308->_max_retries)
	_cs->ExpT308(this);
      else {
	SendReleaseCompIndToCCD(ie_cause::temporary_failure);
	// spec actually says to place the call in maintenance
	suicide();
      }
      return 0;
    }
  } else // network side
    if (_cs->StateID() == ie_call_state::u12_release_indication) {
      // generate an ExpT308 event if first expiry
      if (_t308->_retries < _t308->_max_retries)
	_cs->ExpT308(this);
      else {
	SendReleaseCompIndToCCD(ie_cause::temporary_failure);
	// spec actually says to place the call in maintenance 
	suicide();
      }
      return 0;
    }
  return(-1);
}

int Call::q93b_t310_timeout()
{
  u_char timer_diag[3] = {'3','1','0'};
  if (_call_state == ie_call_state::u9_incoming_call_proceeding) {
    _call_state = ie_call_state::u0_null;
    SendReleaseCompToPeer(ie_cause::recovery_on_timer_expiry);
    SendReleaseIndToCCD(ie_cause::recovery_on_timer_expiry);
    return(0);
  }
  // incompatible state 
  return(-1);
}


int Call::q93b_t313_timeout()
{
  u_char timer_diag[3] = {'3','1','3'};

  if (_call_state == ie_call_state::u8_connect_request) {
    SendReleaseCompToPeer(ie_cause::recovery_on_timer_expiry);
    SendReleaseIndToCCD(ie_cause::recovery_on_timer_expiry);
    return(0);
  }
  // incompatible state
  DIAG("fsm.uni", DIAG_DEBUG, cout << "q93b:Call::q93b_t313_timeout(): incompatible state = " << _call_state << endl);
  return(-1);
}

// this timer is set when RESTART was sent
int Call::q93b_t316_timeout()
{
  u_char timer_diag[3] = {'3','1','6'};

  _t316->_retries++;
  if (_call_state == ie_call_state::rest1_restart_request) {
    if (_t316->_retries < _t316->_max_retries) {
      // Send2Peer(restart_req);
      return(0);
    }
    // no calls should be accepted or notify management here
    _call_state = ie_call_state::rest2_restart;
    // Send2CCD(restart_ind);
    return(0);
  }
  // incompatible state
  DIAG("fsm.uni", DIAG_DEBUG, cout << "q93b:Call::q93b_t316_timeout(): incompatible state = " << _call_state << endl);
  return(-1);
}


// T317 expired before we are done with internal clearing
int Call::q93b_t317_timeout()
{
  u_char timer_diag[3] = {'3','1','7'};
  _t317->_retries++;

  DIAG("fsm.uni", DIAG_DEBUG, cout << "T317 expired before clearing of call" << endl);
  return(-1);
}


int Call::q93b_t322_timeout()
{
  u_char timer_diag[3] = {'3','2','2'};
  _t322->_retries++;
  if (_t322->_retries < _t322->_max_retries) {
    InfoElem *ie_array[num_ie];
    for(int i=0;i < num_ie;i++)
      ie_array[i] = NULL;
    generic_q93b_msg *msg = new q93b_status_enq_message(ie_array, _crv, 0);
    sean_Visitor *qv = new sean_Visitor(sean_Visitor::status_enq_req, msg);
    Send2Peer(qv);
    return(0);
  }
  SendReleaseToPeer(ie_cause::temporary_failure);
  SendReleaseIndToCCD(ie_cause::temporary_failure);
  return(0);
}

void Call::SetIdentity(Conduit* c)
{
  _me = c;
}

Conduit* Call::get_me(void)
{
  return _me;
}

void Call::SetVCI(int vci){  _vci = vci;}
void Call::SetVPI(int vpi){  _vpi = vpi;}

int  Call::GetVCI(void) { return _vci; }
int  Call::GetVPI(void) { return _vpi; }

int Call::CallProtocol(sean_Visitor * sv)
{
  int rval = -1;
  switch (sv->get_op()) {
    case sean_Visitor::setup_req:
      if(_user == NETWORK_SIDE)
	{
	  _vpi = sv->get_vpi();
	  _vci = sv->get_vci();
	}
      rval = _cs->SetupReq(sv, this);
      break;
    case sean_Visitor::setup_ind:
      rval = _cs->SetupInd(sv, this);
      break;
    case sean_Visitor::call_proc_req:
      if(_user == NETWORK_SIDE)
	{
	  _vpi = sv->get_vpi();
	  _vci = sv->get_vci();
	}
      rval = _cs->CallProcReq(sv, this);
      break;
    case sean_Visitor::call_proc_ind:
      if(_user == USER_SIDE)
	{
	  _vpi = sv->get_vpi();
	  _vci = sv->get_vci();
	}
      rval = _cs->CallProcInd(sv, this);
      break;
    case sean_Visitor::connect_req:
      rval = _cs->ConnectReq(sv, this);
      break;
    case sean_Visitor::connect_ind:
      rval = _cs->ConnectInd(sv, this);
      break;
    case sean_Visitor::release_req:
      rval = _cs->ReleaseReq(sv, this);
      break;
    case sean_Visitor::release_ind:
      rval = _cs->ReleaseInd(sv, this);
      break;
    case sean_Visitor::connect_ack_req:
      rval = _cs->ConnectAckReq(sv, this);
      break;
    case sean_Visitor::connect_ack_ind:
      rval = _cs->ConnectAckInd(sv, this);
      break;
    case sean_Visitor::setup_comp_req:
      rval = _cs->SetupCompReq(sv, this);
      break;
    case sean_Visitor::setup_comp_ind:
      rval = _cs->SetupCompInd(sv, this);
      break;
    case sean_Visitor::release_comp_req:
      rval = _cs->ReleaseCompReq(sv, this);
      break;
    case sean_Visitor::release_comp_ind:
      rval = _cs->ReleaseCompInd(sv, this);
      break;
    case sean_Visitor::status_enq_req:
      rval = _cs->StatusEnqReq(sv, this);
      break;
    case sean_Visitor::status_enq_ind:
      rval = _cs->StatusEnqInd(sv, this);
      break;
    case sean_Visitor::status_req:
      rval = _cs->StatusReq(sv, this);
      break;
    case sean_Visitor::status_ind:
      rval = _cs->StatusInd(sv, this);
      break;
    case sean_Visitor::add_party_req:
      rval = _cs->AddPartyReq(sv, this);
      break;
    case sean_Visitor::add_party_ind:
      rval = _cs->AddPartyInd(sv, this);
      break;
    case sean_Visitor::add_party_comp_req:
      rval = _cs->AddPartyCompReq(sv, this);
      break;
    case sean_Visitor::add_party_comp_ind:
      rval = _cs->AddPartyCompInd(sv, this);
      break;
    case sean_Visitor::add_party_ack_req:
      rval = _cs->AddPartyAckReq(sv, this);
      break;
    case sean_Visitor::add_party_ack_ind:
      rval = _cs->AddPartyAckInd(sv, this);
      break;
    case sean_Visitor::add_party_rej_req:
      rval = _cs->AddPartyRejReq(sv, this);
      break;
    case sean_Visitor::add_party_rej_ind:
      rval = _cs->AddPartyRejInd(sv, this);
      break;
    case sean_Visitor::drop_party_req:
      rval = _cs->DropPartyReq(sv, this);
      break;
    case sean_Visitor::drop_party_ind:
      rval = _cs->DropPartyInd(sv, this);
      break;
    case sean_Visitor::drop_party_comp_req:
      rval = _cs->DropPartyCompReq(sv, this);
      break;
    case sean_Visitor::drop_party_comp_ind:
      rval = _cs->DropPartyCompInd(sv, this);
      break;
    case sean_Visitor::drop_party_ack_req:
      rval = _cs->DropPartyAckReq(sv, this);
      break;
    case sean_Visitor::drop_party_ack_ind:
      rval = _cs->DropPartyAckInd(sv, this);
      break;
    case sean_Visitor::leaf_setup_req:
      rval = _cs->LeafSetupReq(sv, this);
      break;
    case sean_Visitor::leaf_setup_ind:
      rval = _cs->LeafSetupInd(sv, this);
      break;
    case sean_Visitor::leaf_setup_failure_req:
      rval = _cs->LeafSetupFailureReq(sv, this);
      break;
    case sean_Visitor::leaf_setup_failure_ind:
      rval = _cs->LeafSetupFailureInd(sv, this);
      break;
    case sean_Visitor::restart_req:
      rval = _cs->RestartReq(sv, this);
      break;
    case sean_Visitor::restart_ind:
      rval = _cs->RestartInd(sv, this);
      break;
    case sean_Visitor::restart_resp:
      rval = _cs->RestartResp(sv, this);
      break;
    case sean_Visitor::restart_comp_ind:
      rval = _cs->RestartCompInd(sv, this);
      break;
    case sean_Visitor::restart_ack_req:
      rval = _cs->RestartAckReq(sv, this);
      break;
    case sean_Visitor::restart_ack_ind:
      rval = _cs->RestartAckInd(sv, this);
      break;
    default:
      // crap
      break;
  }
  return rval;
}

void Call::PartyProtocol(sean_Visitor * sv)
{
  switch (sv->get_op()) {
    case sean_Visitor::setup_req:
      {
	int epr = sv->get_pid();
	Party * p = GetParty(epr);
	p->GetPS()->SetupReq(sv, p);
      }
      break;
    case sean_Visitor::setup_ind:
      {
	int epr = sv->get_pid();
	Party * p = GetParty(epr);
	p->GetPS()->SetupInd(sv, p);
      }
      break;
  }
}


void Call::PrintPartyMap(void) {
  dic_item di;
  forall_items (di, _party_map) {
    int epr = _party_map.key(di);
    cout << "." << epr;
  }
  cout << ".";
}
