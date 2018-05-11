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
static char const _Q93bState_cc_rcsid_[] =
"$Id: Q93bState.cc,v 1.30 1999/03/31 20:49:57 battou Exp $";
#endif
#include <common/cprototypes.h>


#include <FW/basics/diag.h>
#include "Q93bState.h"
#include "Q93bTimers.h"
#include "Q93bParty.h"
#include "Q93bLeaf.h"
#include "Q93bCall.h"
#include "PartyState.h"
#include "SSCFVisitor.h"
#include "Rebind_Visitor.h"
#include <sean/cc/sean_Visitor.h>
#include <codec/q93b_msg/q93b_msg.h>

#include <sean/templates/constants.h>

#define Q93B_DEBUGGING "fsm.uni.Q93B_debugging"

// DEFAULT: this is what you get if you do not redefine these methods
Q93bState::Q93bState() { }
Q93bState::~Q93bState() { }

int Q93bState::LeafSetupReq(sean_Visitor * qv, Call * c)
{
  assert(qv != 0 && c != 0);
  if (c->StackType() == ie_cause::user) {
    Leaf *leaf = c->InitLeaf(qv);
    if (leaf) {
      c->SetT331(leaf);
      c->Save_CID(qv->get_cid());
      c->Save_APPID(qv->get_shared_app_id());
      c->Send2Peer(qv);
      return 0;
    }
  }
  qv->Suicide();
  return -1;
}

int Q93bState::LeafSetupInd(sean_Visitor *qv, Call *c)
{
  c->Send2CCD(qv);
  return 0;
}


int Q93bState::LeafSetupFailureReq(sean_Visitor *qv, Call *c)
{
  c->Send2Peer(qv);
  return 0;
}

int Q93bState::LeafSetupFailureInd(sean_Visitor *qv, Call *c)
{
  c->Send2CCD(qv);
  return 0;
}

// expected only in NullState
int Q93bState::SetupReq(sean_Visitor *qv, Call *c)
{
  char name[80];
  StateName(name);
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "SetupReq in wrong state: " << name << endl);
  qv->Suicide();
  return(-1);
}

int Q93bState::SetupInd(sean_Visitor *qv, Call *c)
{
  char name[80];
  StateName(name);
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "SetupInd in wrong state: " << name << endl);
  qv->Suicide();
  return(-1);
}

void Q93bState::ExpT303(Call *c)
{
  char name[80];
  StateName(name);
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "T303 expired in wrong state: " << name << endl);
}

void Q93bState::ExpT301(Call *c)
{
  char name[80];
  StateName(name);
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "T301 expired in wrong state: " << name << endl);
}

void Q93bState::ExpT308(Call *c)
{
  char name[80];
  StateName(name);
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "T308 expired in wrong state: " << name << endl);
}

void Q93bState::ExpT310(Call *c)
{
  char name[80];
  StateName(name);
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "T310 expired in wrong state: " << name << endl);
}

void Q93bState::ExpT313(Call *c)
{
  char name[80];
  StateName(name);
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "T313 expired in wrong state: " << name << endl);
}

void Q93bState::ExpT316(Call *c)
{
  char name[80];
  StateName(name);
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "T316 expired in wrong state: " << name << endl);
}

void Q93bState::ExpT317(Call *c)
{
  char name[80];
  StateName(name);
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "T317 expired in wrong state: " << name << endl);
}


int Q93bState::ReleaseCompReq(sean_Visitor *qv, Call *c)
{
  assert(qv != 0 && c != 0);
  generic_q93b_msg *msg = qv->share_message();
  assert(msg != 0);
  InfoElem **ies = msg->get_ie_array();
  ie_cause::CauseValues cv = ie_cause::normal_unspecified;
  if(ies[InfoElem::ie_cause_ix])
    cv = ((ie_cause *)ies[InfoElem::ie_cause_ix])->get_cause_value();
  c->ResetAllTimers();
  if(c->IsP2MP())
    {
      c->DropAllParties(cv);
      c->RCParties();
    }
  c->Send2Peer(qv);
  c->suicide();
  return -2;
}

int Q93bState::ReleaseReq(sean_Visitor *qv, Call *c)
{
  assert(qv != 0 && c != 0);
  generic_q93b_msg *msg = qv->share_message();
  assert(msg != 0);
  InfoElem **ies = msg->get_ie_array();
  ie_cause::CauseValues cv = ((ie_cause *)ies[InfoElem::ie_cause_ix])->get_cause_value();
  c->ResetAllTimers();
  if(c->IsP2MP())
    {
      c->DropAllParties(cv);
      // see 9.3.6 dropping all parties in Q.2971 page 22
      c->ReleaseAllParties(cv);
    }
  c->SetT308();
  if(c->StackType() == ie_cause::user)
    {
      ChangeState(c, ReleaseReqState::Instance());
    }
  else
    {
      ChangeState(c, ReleaseIndState::Instance());
    }
  c->Send2Peer(qv);
  return 0;
}

int Q93bState::AlertReq(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::CallProcReq(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::ConnectAckReq(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::StatusReq(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::StatusEnqReq(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::RestartReq(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::RestartResp(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::RestartAckReq(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::AddPartyReq(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::AddPartyRejReq(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::AddPartyAckReq(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::DropPartyAckReq(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::AlertInd(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::CallProcInd(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::ConnectAckInd(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::RestartInd(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::RestartCompInd(sean_Visitor *qv, Call *c) {qv->Suicide(); return -1;}
int Q93bState::RestartAckInd(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::AddPartyInd(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::AddPartyRejInd(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::AddPartyAckInd(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}
int Q93bState::DropPartyCompInd(sean_Visitor *qv, Call *c) {qv->Suicide(); return -1;}
int Q93bState::PartyAlertInd(sean_Visitor *qv, Call *c) {qv->Suicide(); return -1;}
int Q93bState::AddPartyCompInd(sean_Visitor *qv, Call *c) {qv->Suicide(); return -1;}
int Q93bState::DropPartyCompReq(sean_Visitor *qv, Call *c) {qv->Suicide(); return -1;}
int Q93bState::PartyAlertReq(sean_Visitor *qv, Call *c) {qv->Suicide(); return -1;}
int Q93bState::AddPartyCompReq(sean_Visitor *qv, Call *c) {qv->Suicide(); return -1;}
int Q93bState::StatusRespInd(sean_Visitor *qv, Call *c) {qv->Suicide(); return -1;}
int Q93bState::SetupCompInd(sean_Visitor *qv, Call *c) {qv->Suicide(); return -1;}
int Q93bState::ConnectInd(sean_Visitor *qv, Call *c) {qv->Suicide(); return -1;}
int Q93bState::StatusRespReq(sean_Visitor *qv, Call *c) {qv->Suicide(); return -1;}
int Q93bState::SetupCompReq(sean_Visitor *qv, Call *c) {qv->Suicide(); return -1;}
int Q93bState::ConnectReq(sean_Visitor *qv, Call *c) {qv->Suicide(); return -1;}

int Q93bState::DropPartyReq(sean_Visitor *qv, Call *c) { qv->Suicide(); return -1;}


int Q93bState::DropPartyInd(sean_Visitor *qv, Call *c)
{
  if (c->IsP2P()) {
    UnexpectedMSG(qv,c);
    return(-1);
  }
  // we have a P2MP call we need to verify the ER
  generic_q93b_msg *msg = qv->share_message();
  assert(msg != 0);
  int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
  ie_cause::CauseValues cv;
  Party *p = 0;
  int ret = 0;
  switch(c->ValidateEPR(cv,qv))
    {
    case YES:
      p = c->GetParty(epr);
      assert(p != 0);
      ret = p->GetPS()->DropPartyInd(qv,p);
      if(ret == -2)
	c->KillParty(p);
      c->Send2CCD(qv);
      break;

    case RAI:
      c->SendCallStatusToPeer(c->GetCS()->StateID(),cv);
      // fallthru
    case I:
    case NOPC:
      qv->Suicide();
      break;
    case CLR:
      c->DoLabelD(cv);
      break;
    default:
      diag("fsm.uni", DIAG_ERROR, "invalid return code from ValidateEPR\n");
      qv->Suicide();
      break;
    }
  return 0;
}


int Q93bState::DropPartyAckInd(sean_Visitor *qv, Call *c)
{
  if (c->IsP2P())
    UnexpectedMSG(qv,c);
  qv->Suicide();
  return 0;
}

// must be redefined only in N0 and N12 (Q.2971 p.83)
// we will move to Null once we get the ReleaseRespReq (ReleaseCompReq)
// from CCD
int Q93bState::ReleaseInd(sean_Visitor *qv, Call *c)
{
  assert(qv != 0 && c != 0);
  generic_q93b_msg *msg = qv->share_message();
  assert(msg != 0);
  InfoElem **ies = msg->get_ie_array();
  ie_cause::CauseValues cv = ((ie_cause *)ies[InfoElem::ie_cause_ix])->get_cause_value();
  c->ResetAllTimers();
  if(c->IsP2MP())
    {
      dic_item it;
      forall_items(it,c->_party_map)
	{
	  Party *party = c->_party_map.inf(it);
	  assert(party != 0);
	  ie_end_pt_state::party_state ps = party->get_eps();
	  if((ps == ie_end_pt_state::p_active) ||
	     (ps == ie_end_pt_state::add_party_received) ||
	     (ps == ie_end_pt_state::add_party_initiated) // no call reoffering
	     )
	    {
	      party->GetPS()->DropPartyReq(0,party);
	      // clear these parties toward the remote by generating DropPartyInd to CCD
	      // c->SendDPIndToCCD(party->get_epr(),cv); _CRAP_
	    }
	}
    }
  if(c->StackType() == ie_cause::user)
    ChangeState(c, ReleaseIndState::Instance());
  else
    ChangeState(c, ReleaseReqState::Instance());
  c->Send2CCD(qv);
  return 0;
}

// must be redefined only in N0, N11, and N12 (Q.2931 p.150)
// coded from Q.2971 page 90
int Q93bState::StatusInd(sean_Visitor *qv, Call *c)
{
  assert(qv != 0);
  generic_q93b_msg *msg = qv->share_message();
  assert(msg != 0);
  ie_cause::CauseValues cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
  ie_call_state::call_state_values cs = ((ie_call_state *)msg->ie(InfoElem::ie_call_state_ix))->get_state_value();
  ie_end_pt_state *es_ie = (ie_end_pt_state *)msg->ie(InfoElem::ie_end_pt_state_ix);
  switch(c->VerifyMessage(cv,qv)) {
    case RAP: 
    case RAI:
      c->SendCallStatusToPeer(c->GetCS()->StateID(),cv);
    case OK:
    case I:
      if (StateID() == ie_call_state::u0_null)
	{
	  c->RCParties();
	  c->SendReleaseCompToPeer(cv);
	  c->SendReleaseCompIndToCCD(cv);
	  return -2;
	}
      if(cv == ie_cause::response_to_status_enquiry)
	c->SetT322();
      if(cs == c->GetCallState())
	{ // C12
	  if(es_ie)
	    {
	      int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
	      c->SendPartyStatusToPeer(cv,es_ie->get_epr_state(),epr);
	    }
	}
      else
	{
	  // no recovery option
	  ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->set_cause_value(ie_cause::message_not_compatible_with_call_state);
	  c->Send2CCD(qv);
	  c->DoLabelD(cv);
	}
      break;

    case CLR:
      c->DoLabelD(cv);
      break;
  }
  return 0;
}

// must be redefined only in N0 (Q.2931 p.150)
int Q93bState::StatusEnqInd(sean_Visitor *qv, Call *c)
{
  c->SendCallStatusToPeer(c->GetCallState(),ie_cause::response_to_status_enquiry);
  qv->Suicide();
  return 0;
}


// must be redefined only in N0 (Q.2931 p.144)
int Q93bState::NotifyInd(sean_Visitor *qv, Call *c)
{
  assert(qv != 0);
  generic_q93b_msg *msg = qv->share_message();
  assert(msg != 0);
  ie_cause::CauseValues cv;
  ie_call_state::call_state_values cs = ((ie_call_state *)msg->ie(InfoElem::ie_call_state_ix))->get_state_value();
  ie_end_pt_state *es_ie = (ie_end_pt_state *)msg->ie(InfoElem::ie_end_pt_state_ix);
  switch(c->VerifyMessage(cv,qv))
    {
    case RAP: 
      c->SendCallStatusToPeer(c->GetCS()->StateID(),cv);
    case OK:
      c->Send2CCD(qv);
      break;
    case CLR:
      c->DoLabelD(cv);
      break;

    case RAI:
      c->SendCallStatusToPeer(c->GetCS()->StateID(),cv);
    case I:
      break;
    }
  return 0;
}

// must be redefined only in N0 (Q.2931 p.144)
int Q93bState::NotifyReq(sean_Visitor *qv, Call *c)
{
  c->Send2Peer(qv);
  return 0;
}

// must be redefined only in N0 and N12 (Q.2931 p.146)
int Q93bState::ReleaseCompInd(sean_Visitor *qv, Call *c)
{
  assert(qv != 0 && c != 0);
  c->ResetAllTimers();
  if(c->IsP2MP())
    {
      dic_item it;
      forall_items(it,c->_party_map)
	{
	  Party *party = c->_party_map.inf(it);
	  assert(party != 0);
	  ie_end_pt_state::party_state ps = party->get_eps();
	  if((ps == ie_end_pt_state::p_active) ||
	     (ps == ie_end_pt_state::add_party_received) ||
	     (ps == ie_end_pt_state::add_party_initiated) // no call reoffering
	     )
	    {
	      party->GetPS()->DropPartyReq(0,party);
	      // clear these parties toward the remote by generating DropPartyInd to CCD
	      // c->SendDPIndToCCD(party->get_epr(),ie_cause::normal_unspecified); _CRAP_
	    }
	}
    }
  ChangeState(c, NullState::Instance());
  c->Send2CCD(qv);
  c->suicide();
  return -2;
}


// must be redefined only in N0 (Q.2931 p.151)
int Q93bState::q93b_t322_timeout(Call *c) {c->q93b_t322_timeout(); return 1;}

int Q93bState::q93b_t301_timeout(Call *c) { return -1; }

int Q93bState::q93b_t308_timeout(Call *c) { return -1; }

int Q93bState::q93b_t310_timeout(Call *c) { return -1; }

int Q93bState::q93b_t313_timeout(Call *c) { return -1; }

int Q93bState::q93b_t316_timeout(Call *c) { return -1; }

int Q93bState::q93b_t317_timeout(Call *c) { return -1; }

int Q93bState::UnexpectedMSG(sean_Visitor *qv, Call *c)
{
  c->DoUM(qv);
  return 0;
}

void Q93bState::ChangeState(Call *c, Q93bState *s) { c->ChangeState(s); }

// ----------------------------------------------------------------------
Q93bState *NullState::_instance = 0;

NullState::NullState() { }

NullState::~NullState() { }

Q93bState * NullState::Instance()
{
  if (!_instance)
    _instance = new NullState;
  return(_instance);
}

void NullState::StateName(char *name)
{
  if(name)
    strcpy(name,"Null");
}

ie_call_state::call_state_values NullState::StateID() { return ie_call_state::u0_null;}

int NullState::SetupReq(sean_Visitor *qv, Call *c)
{
  assert(qv != 0);
  Party *p = c->Init(qv);
  if (p)
    c->PartyProtocol(qv);
  c->SetT303();
  if(c->StackType() == ie_cause::user)
    ChangeState(c, CallInitiatedState::Instance());
  else
    ChangeState(c, CallPresentState::Instance());

  c->Save_CID(qv->get_cid());
  c->Save_APPID(qv->get_shared_app_id());
  c->Send2Peer(qv);
  return(0);
}

int NullState::SetupInd(sean_Visitor *qv, Call *c)
{
  assert(qv != 0);
  ie_cause::CauseValues cv;
  Party *p = 0;
  Leaf *l = 0;
  int ret;
  switch (c->VerifyMessage(cv,qv)) {
    case RAP: 
      // send a status with cs = u6_call_present and fall thru
      c->SendCallStatusToPeer(ie_call_state::u6_call_present,ie_cause::normal_unspecified);
    case OK:
      p = c->Init(qv);
      if (p)
	c->PartyProtocol(qv); // qv->PartyProtocol(c);
      if (c->StackType() == ie_cause::user) {
	ChangeState(c, CallPresentState::Instance());
	// is this setup in response to a LeafSetupReq ?
	generic_q93b_msg *msg = qv->share_message();
	assert(msg != 0);
	InfoElem **ies = msg->get_ie_array();
	if (ies[InfoElem::UNI40_leaf_sequence_num_ix]) {
	  int seqnum = ((UNI40_leaf_sequence_num *)ies[InfoElem::UNI40_leaf_sequence_num_ix])->getSeqNum();
	  if (l = c->GetLeaf(seqnum)) {
	    l->StopT331();
	    l->ChangeStateToNull();
	    // now we can remove the leaf state
	    c->KillLeaf(seqnum);
	    Rebind_Visitor* rv1 = new Rebind_Visitor(qv);
	    c->Send2Peer(rv1);
	    
	    Rebind_Visitor* rv2 = new Rebind_Visitor(qv);
	    c->Send2CCD(rv2);
	  }
	} else {
	  c->Save_CID(qv->get_cid());
	  c->Save_APPID(qv->get_shared_app_id());
	}
      } else
	ChangeState(c, CallInitiatedState::Instance());
      c->Send2CCD(qv);
      break;
  case CLR:
    break;
  case RAI:
    // send a status with cs = NullState and fall thru
      c->SendCallStatusToPeer(ie_call_state::u0_null,ie_cause::normal_unspecified);
    case I:
      return 0;
      break;
  }
  return(0);
}

int NullState::LeafSetupFailureInd(sean_Visitor *qv, Call *c)
{
  assert(c != 0);
  if (c->StackType() == ie_cause::user)
    {
      Leaf *l = 0;
      generic_q93b_msg *msg = qv->share_message();
      assert(msg != 0);
      InfoElem **ies = msg->get_ie_array();
      int seqnum = ((UNI40_leaf_sequence_num *)ies[InfoElem::UNI40_leaf_sequence_num_ix])->getSeqNum();
      if (l = c->GetLeaf(seqnum))
	{
	  l->StopT331();
	  c->Send2CCD(qv);
	  return -2;
	}
      DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a LeafSetupFailureind with wrong seqnum in NullState\n");
    }
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a LeafSetupFailureInd in wrong state: NullState\n");
  qv->Suicide();
  return -2;
}

// don't do page 85 & 140 Q.2971
inline int NullState::ReleaseReq(sean_Visitor *qv, Call *c){qv->Suicide(); return -1; }
// don't do page 144 Q.2931
inline int NullState::NotifyReq(sean_Visitor *qv, Call *c){ qv->Suicide();return -1; }
inline int NullState::NotifyInd(sean_Visitor *qv, Call *c){ qv->Suicide();return -1; }
// don't do page 138 Q.2971
inline int NullState::ReleaseInd(sean_Visitor *qv, Call *c){ qv->Suicide();return -1; }
inline int NullState::ReleaseCompInd(sean_Visitor *qv, Call *c){ qv->Suicide();return -1; }
// don't do page 139 Q.2971
inline int NullState::DropPartyInd(sean_Visitor *qv, Call *c){ qv->Suicide();return -1; }
inline int NullState::DropPartyReq(sean_Visitor *qv, Call *c){ qv->Suicide();return -1; }
inline int NullState::DropPartyAckReq(sean_Visitor *qv, Call *c){ qv->Suicide();return -1; }
// don't do page 144 Q.2971
inline int NullState::StatusEnqReq(sean_Visitor *qv, Call *c){ qv->Suicide();return -1; }
inline int NullState::StatusEnqInd(sean_Visitor *qv, Call *c){ qv->Suicide();return -1; }
// don't do page 145 Q.2971
inline int NullState::StatusInd(sean_Visitor *qv, Call *c){ qv->Suicide();return -1; }
// don't do page 147 Q.2971
inline void ExpT322(Call *){ }
// don't do page 148 Q.2971
inline int NullState::UnexpectedMSG(sean_Visitor *qv, Call *c){ qv->Suicide();return -1; }

// -------------------------------------------------------------------------
Q93bState *CallInitiatedState::_instance = 0;

CallInitiatedState::CallInitiatedState(){}
CallInitiatedState::~CallInitiatedState(){}

Q93bState* CallInitiatedState::Instance()
{
  if (!_instance)
    _instance = new CallInitiatedState;
  return(_instance);
}

void CallInitiatedState::StateName(char *name)
{
  if(name)
    strcpy(name,"CallInitiated");
}

ie_call_state::call_state_values CallInitiatedState::StateID() { return ie_call_state::u1_call_initiated;}

// page 63 Q.2971 -- same as ReleaseResp
int CallInitiatedState::ReleaseCompReq(sean_Visitor *qv, Call *c)
{
  if(c->StackType() == ie_cause::local_private_network)
    {
      c->Send2Peer(qv);
      c->suicide();
      return -2;
    }
  // we are user side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a ReleaseResp in wrong state: CallInitiatedState\n");
  qv->Suicide();
  return -1;
}

// page 63 Q.2971
int CallInitiatedState::CallProcReq(sean_Visitor *qv, Call *c)
{
  if(c->StackType() == ie_cause::local_private_network)
    {
      c->Save_APPID(qv->get_shared_app_id());
      c->Send2Peer(qv);
      ChangeState(c, OutCallProcState::Instance());
      return 0;
    }
  // we are a user side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a CallProcReq in wrong state: CallInitiatedState\n");
  qv->Suicide();
  return -1;
}

// page 119 Q.2971
int CallInitiatedState::CallProcInd(sean_Visitor *qv, Call *c)
{
  ie_cause::CauseValues cv;
  if (c->StackType() == ie_cause::user) {
    switch (c->VerifyMessage(cv,qv)) {
    case RAP: 
      // send a status with cs = u3_out_going_call_proceeding and a cause from msg
      c->SendCallStatusToPeer(ie_call_state::u3_out_going_call_proceeding,ie_cause::normal_unspecified);
      // fall thru
    case OK:
      c->StopT303();
      c->SetT310();
      ChangeState(c, OutCallProcState::Instance());
      c->Send2CCD(qv);
      break;

    case CLR:
      c->DoLabelD(ie_cause::normal_unspecified);
      break;

    case RAI:
      // send a status with cs = u1_call_initiated and a cause from msg
      c->SendCallStatusToPeer(ie_call_state::u1_call_initiated,ie_cause::normal_unspecified);
      // fall thru
    case I:
      break;
    }
  } else {
    DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a CallProcInd in wrong state: CallInitiatedState\n");
    qv->Suicide();
    return -1;
  }
  return 0;
}


// page 119 Q.2971
void CallInitiatedState::ExpT303(Call *c)
{
  if (c->StackType() == ie_cause::user) {
    c->SendSetupToPeer();
    c->SetT303();
  } else {
    // As a network side T303 should not expire in this state
    DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "T303 expired in wrong state: CallInitiatedState\n");
  }
}

// extensions

int CallInitiatedState::ConnectReq(sean_Visitor *qv, Call *c)
{
  assert(c != 0 && qv != 0);
  if(c->StackType() == ie_cause::local_private_network)
    {
      if(c->IsP2MP())
        {
          // API is trustworthy so no need to validate msg data
          generic_q93b_msg *msg = qv->share_message();
          assert(msg != 0);
          int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
          Party *p = c->GetParty(epr);
          assert(p != 0);
          int ret = p->GetPS()->ConnectReq(qv,p);
	  if(ret == -2)
	    c->KillParty(p);
        }
      c->Send2Peer(qv);
      ChangeState(c, ActiveState::Instance());
      return(0);
    }
  // user side -- error
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "SetupResp/ConnectReq in wrong state: CallInitiatedState\n");
  qv->Suicide();
  return -1;
}

// -------------------------------------------------------------------
Q93bState *OutCallProcState::_instance = 0;

OutCallProcState::OutCallProcState(){}
OutCallProcState::~OutCallProcState(){}

Q93bState* OutCallProcState::Instance()
{
  if (!_instance)
    _instance = new OutCallProcState;
  return(_instance);
}

void OutCallProcState::StateName(char *name)
{
  if(name)
    strcpy(name,"OutgoingCallProceeding");
}

ie_call_state::call_state_values OutCallProcState::StateID() { return ie_call_state::u3_out_going_call_proceeding;}

// page 64 Q.2971 Network side
int OutCallProcState::AlertReq(sean_Visitor *qv, Call *c)
{
  assert(qv != 0 && c != 0);
  if(c->StackType() == ie_cause::local_private_network)
    {
      if(c->IsP2MP())
	{
	  // API is trustworthy so no need to validate msg data
	  generic_q93b_msg *msg = qv->share_message();
	  assert(msg != 0);
	  int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
	  Party *p = c->GetParty(epr);
	  assert(p != 0);
	  int ret = p->GetPS()->AlertReq(qv,p);
	  if(ret == -2)
	    c->KillParty(p);
	}
      ChangeState(c, CallDeliveredState::Instance());
      c->Send2Peer(qv);
      return(0);
    }
  // user side -- error
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "AlertReq message received in wrong state: OutCallProcState\n");
  qv->Suicide();
  return -1;
}

// page 64 Q.2971 Network side
// SetupResp or ConnectReq
int OutCallProcState::ConnectReq(sean_Visitor *qv, Call *c)
{
  assert(c != 0 && qv != 0);
  if(c->StackType() == ie_cause::local_private_network)
    {
      if(c->IsP2MP())
	{
	  // API is trustworthy so no need to validate msg data
	  generic_q93b_msg *msg = qv->share_message();
	  assert(msg != 0);
	  int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
	  Party *p = c->GetParty(epr);
	  assert(p != 0);
	  int ret = p->GetPS()->ConnectReq(qv,p);
	  if(ret == -2)
	    c->KillParty(p);
	}
      ChangeState(c, ActiveState::Instance());
      c->Send2Peer(qv);
      return(0);
    }
  // user side -- error
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "SetupResp/ConnectReq message received in wrong state: OutCallProcState\n");
  qv->Suicide();
  return -1;
}

// page 120 Q.2971 -- user side only

int OutCallProcState::AlertInd(sean_Visitor *qv, Call *c)
{
  assert(c != 0 && qv != 0);
  generic_q93b_msg *msg = qv->share_message();
  assert(msg != 0);
  ie_cause::CauseValues cv;
  if (c->StackType() == ie_cause::user) {
    switch (c->VerifyMessage(cv,qv)) {
	case RAP: 
	  // send a status with cs = u4_call_delivered
	  cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
	  c->SendCallStatusToPeer(ie_call_state::u4_call_delivered,cv);
	  // fall thru
	case OK:
	  c->StopT303();
	  if (c->IsP2MP()) {
	    int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
	    Party * p = c->GetParty(epr);
	    assert(p != 0);
	    int ret = p->GetPS()->AlertInd(qv,p);
	    if(ret == -2)
	      c->KillParty(p);
	  }
	  ChangeState(c, CallDeliveredState::Instance());
	  c->Send2CCD(qv);
	  break;

	case RAI:
	  // send a status with cs = u3_out_going_call_proceeding
	  cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
	  c->SendCallStatusToPeer(ie_call_state::u3_out_going_call_proceeding,cv);
	  // fall thru
	case I:
	  break;

	case CLR:
	  cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
	  c->DoLabelD(cv);
	  break;
    }
    return 0;
  }
  // user side -- error
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "AlertInd message received in wrong state: OutCallProcState\n");
  qv->Suicide();
  return -1;
}

int OutCallProcState::ConnectInd(sean_Visitor *qv, Call *c)
{
  assert(c != 0 && qv != 0);
  generic_q93b_msg *msg = qv->share_message();
  assert(msg != 0);
  sean_Visitor *v = 0;
  InfoElem *ie_array[num_ie];
  ie_cause::CauseValues cv;
  if (c->StackType() == ie_cause::user) {
    switch(c->VerifyMessage(cv,qv)) {
    case RAP: 
      // send a status with cs = u4_call_delivered
      cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
      c->SendCallStatusToPeer(ie_call_state::u4_call_delivered,cv);
      // fall thru
    case OK:
      c->StopT310();
      if (c->IsP2MP()) {
	int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
	Party *p = c->GetParty(epr);
	if(p == 0)
	  {
	    cout << "no party under epr = " << hex << epr << dec <<  endl; // Abdella
	    exit(1);
	  }
	int ret = p->GetPS()->ConnectInd(qv,p);
	if(ret == -2)
	  c->KillParty(p);
      }
      ChangeState(c,ActiveState::Instance());
      // send connect_ack for symmetrical call control procedures
      c->SendConnectAckToPeer();
      c->Send2CCD(qv);
      break;

    case RAI:
      // send a status with cs = u3_out_going_call_proceeding
      cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
      c->SendCallStatusToPeer(ie_call_state::u3_out_going_call_proceeding,cv);
      // fall thru
    case I:
      break;

    case CLR:
      cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
      c->DoLabelD(cv);
      break;
    }
    return 0;
  }
  // user side -- error
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "ConnectInd message received in wrong state: OutCallProcState\n");
  qv->Suicide();
  return -1;
}

void OutCallProcState::ExpT310(Call *c)
{
  assert(c != 0);
  if (c->StackType() == ie_cause::user) {
    ie_cause::CauseValues cv = ie_cause::no_user_responding;
    c->DoLabelD(cv);
  } else {
    // user side -- error
    DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "T310 expired in wrong state: OutCallProcState\n");
  }
}

// --------------------------------------------------------------------------------
Q93bState *CallDeliveredState::_instance = 0;

CallDeliveredState::CallDeliveredState(){}
CallDeliveredState::~CallDeliveredState(){}

Q93bState* CallDeliveredState::Instance()
{
  if (!_instance)
    _instance = new CallDeliveredState;
  return(_instance);
}

void CallDeliveredState::StateName(char *name)
{
  if(name)
    strcpy(name,"CallDelivered");
}

ie_call_state::call_state_values CallDeliveredState::StateID() { return ie_call_state::u4_call_delivered;}

// page 65 for network side
int CallDeliveredState::ConnectReq(sean_Visitor *qv, Call *c)
{
  assert(qv != 0 && c != 0);
  if(c->StackType() == ie_cause::local_private_network)
    {
      if(c->IsP2MP())
        {
          // API is trustworthy so no need to validate msg data
          generic_q93b_msg *msg = qv->share_message();
          assert(msg != 0);
          int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
          Party *p = c->GetParty(epr);
          assert(p != 0);
          int ret = p->GetPS()->ConnectReq(qv,p);
	  if(ret == -2)
	    c->KillParty(p);
        }
      ChangeState(c, ActiveState::Instance());
      c->Send2Peer(qv);
      return(0);
    }
  // user side -- error
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "SetupResp/ConnectReq message received in wrong state: CallDeliveredState\n");
  qv->Suicide();
  return -1;
}

int CallDeliveredState::PartyAlertReq(sean_Visitor *qv, Call *c)
{
  if(c->StackType() == ie_cause::local_private_network)
    {
      if(c->IsP2MP())
        {
          // API is trustworthy so no need to validate msg data
          generic_q93b_msg *msg = qv->share_message();
          assert(msg != 0);
          int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
          Party *p = c->GetParty(epr);
          assert(p != 0);
          int ret = p->GetPS()->PartyAlertReq(qv,p);
	  if(ret == -2)
	    c->KillParty(p);
        }
      c->Send2Peer(qv);
      return(0);
    }
  // user side -- error
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "PartyAlertReq message received in wrong state: CallDeliveredState\n");
  qv->Suicide();
  return -1;
}

int CallDeliveredState::AddPartyRejReq(sean_Visitor *qv, Call *c)
{
  if(c->StackType() == ie_cause::local_private_network)
    {
      if(c->IsP2MP())
        {
          // API is trustworthy so no need to validate msg data
          generic_q93b_msg *msg = qv->share_message();
          assert(msg != 0);
          int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
          Party *p = c->GetParty(epr);
          assert(p != 0);
          p->GetPS()->AddPartyRejReq(qv,p);
        }
      c->Send2Peer(qv);
      return(0);
    }
  // user side -- error
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "AddPartyRejReq message received in wrong state: CallDeliveredState\n");
  qv->Suicide();
  return -1;
}

int CallDeliveredState::AddPartyInd(sean_Visitor *qv, Call *c)
{
  int ret;
  
  if(c->StackType() == ie_cause::local_private_network)
    {
      if(c->IsP2MP())
        {
          generic_q93b_msg *msg = qv->share_message();
          assert(msg != 0);
          int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
	  InfoElem **ies = msg->get_ie_array();
	  // uni-40 8.2.1.1 page 63
	  if(ies[InfoElem::UNI40_min_traff_desc_ix] || ies[InfoElem::UNI40_alt_traff_desc_ix])
	    {
	      if(c->IsCallNegotiating())
		{
		  int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
		  c->SendAPRejToPeer(epr, ie_cause::protocol_error_unspecified);
		  return 0;
		}
	    }
          Party *p = c->GetParty(epr);
	  if(!p)
	    p = c->Init(qv);
	  assert(p != 0);
	  int ret = p->GetPS()->AddPartyInd(qv,p);
	  if(ret == -2)
	    c->KillParty(p);
	  c->Send2CCD(qv);
        }
      else
	c->DoUM(qv);
      return(0);
    }


  // user side -- error
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "AddPartyInd message received in wrong state: CallDeliveredState\n");
  qv->Suicide();
  return -1;
}

// page 122-124 for user side

int CallDeliveredState::PartyAlertInd(sean_Visitor *qv, Call *c)
{
  if(c->IsP2MP())
    c->DoUM(qv);
  else
    {
      // we have a P2MP call
      generic_q93b_msg *msg = qv->share_message();
      assert(msg != 0);
      int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
      ie_cause::CauseValues cv;
      Party *p = 0;
      int ret = 0;
      switch(c->ValidateEPR(cv,qv))
	{
	case YES:
	  p = c->GetParty(epr);
	  assert(p != 0);
	  ret = p->GetPS()->PartyAlertInd(qv,p);
	  if(ret == -2)
	    c->KillParty(p);
	  c->Send2CCD(qv);
	  break;

	case RAI:
	  c->SendCallStatusToPeer(ie_call_state::u4_call_delivered,cv);
	  // fallthru
	case I:
	  qv->Suicide();
	  break;

	case NOPC:
	  c->SendDPAKToPeer(epr,ie_cause::invalid_endpoint_reference);
	  qv->Suicide();
	  break;
	case CLR:
	  c->DoLabelD(cv);
	  break;
	default:
	  diag("fsm.uni", DIAG_ERROR, "invalid return code from ValidateEPR\n");
	  qv->Suicide();
	  break;
	}
    }
  return 0;
}

// API should enforce 8.2.1.1 page 63 of UNI-40
// no AddParty for negotiating SETUPs 
int CallDeliveredState::AddPartyReq(sean_Visitor *qv, Call *c)
{
  assert(c != 0);
  if(c->StackType() == ie_cause::user)
    {
      Party *p = c->Init(qv);
      assert(p != 0);
      int ret = p->GetPS()->AddPartyReq(qv,p);
      if(ret == -2)
	c->KillParty(p);
      c->Send2Peer(qv);
      return 0;
    }
  return -1;
}

int CallDeliveredState::AddPartyRejInd(sean_Visitor *qv, Call *c)
{
  assert(c != 0);
  ie_cause::CauseValues cv;
  Party *p;
  int ret = 0;
  if (c->StackType() == ie_cause::user) {
    if (c->IsP2P())
      c->DoUM(qv);
    else {
      // we have a P2MP call
      generic_q93b_msg *msg = qv->share_message();
      assert(msg != 0);
      int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
      switch(c->ValidateEPR(cv,qv))
	{
	case CLR:
	  c->DoLabelD(cv);
	  qv->Suicide();
	  break;
	  
	case YES:
	  p = c->GetParty(epr);
	  assert(p != 0);
	  ret = p->GetPS()->PartyAlertInd(qv,p);
	  if(ret == -2)
	    c->KillParty(p);
	  c->Send2Peer(qv);
	  break;
	  
	case RAI:
	  c->SendCallStatusToPeer(ie_call_state::u4_call_delivered,cv);
	  // fallthru
	case I:
	case NOPC:
	  qv->Suicide();
	  break;
	default:
	  diag("fsm.uni", DIAG_ERROR, "invalid return code from ValidateEPR\n");
	  qv->Suicide();
	  break;
	}
    }
    return 0;
  }
  return -1;
}

int CallDeliveredState::ConnectInd(sean_Visitor *qv, Call *c)
{
  generic_q93b_msg *msg = qv->share_message();
  assert(msg != 0);
  sean_Visitor *v = 0;
  InfoElem *ie_array[num_ie];
  ie_cause::CauseValues cv;
  if (c->StackType() == ie_cause::user) {
    switch (c->VerifyMessage(cv,qv)) {
    case RAP: 
      c->SendCallStatusToPeer(ie_call_state::u10_active,cv);
      // fall thru
    case OK:
      if (c->IsP2MP()) {
	int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
	Party *p = c->GetParty(epr);
	assert(p != 0);
	int ret = p->GetPS()->ConnectInd(qv,p);
	if(ret == -2)
	  c->KillParty(p);
      }
      ChangeState(c,ActiveState::Instance());
      // send connect_ack for symmetrical call control procedures
      c->SendConnectAckToPeer();
      c->Send2CCD(qv);
      break;

    case RAI:
      // send a status with cs = u4
      c->SendCallStatusToPeer(ie_call_state::u4_call_delivered,cv);
      // fall thru
    case I:
      qv->Suicide();
      break;

    case CLR:
      c->DoLabelD(cv);
      qv->Suicide();
      break;
    }
    return 0;
  }
  // network side -- error
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, 
       cout <<  "ConnectInd message received in wrong state: CallDeliveredState\n");
  qv->Suicide();
  return -1;
}

// ------------------------------------------------------------------------------
Q93bState *CallPresentState::_instance = 0;

CallPresentState::CallPresentState(void){}
CallPresentState::~CallPresentState(void){}

Q93bState* CallPresentState::Instance()
{
  if (!_instance)
    _instance = new CallPresentState;
  return(_instance);
}

void CallPresentState::StateName(char *name)
{
  if(name)
    strcpy(name,"CallPresent");
}

ie_call_state::call_state_values CallPresentState::StateID() { return ie_call_state::u6_call_present;}

// pages 66-68 for network side
int CallPresentState::AlertInd(sean_Visitor *qv, Call *c)
{
  ie_cause::CauseValues cv;
  if(c->StackType() == ie_cause::local_private_network)
    {
      switch (c->VerifyMessage(cv,qv))
	{
	case RAP: 
	  c->SendCallStatusToPeer(ie_call_state::u7_call_received,cv);
	  // fall thru
	case OK:
	  c->SetT303();
	  if(c->IsP2P())
	    c->SetT301();
	  else
	    {
	      generic_q93b_msg *msg = qv->share_message();
	      assert(msg != 0);
	      int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
	      Party *p = c->GetParty(epr);
	      assert(p != 0);
	      int ret = p->GetPS()->AlertInd(qv,p);
	      if(ret == -2)
		c->KillParty(p);
	    }
	  ChangeState(c, CallReceivedState::Instance());
	  c->Send2CCD(qv);
	  break;

	case CLR:
	  c->DoLabelD(cv);
	  break;

	case RAI:
	  c->SendCallStatusToPeer(ie_call_state::u6_call_present,cv);
	  break;
	case I:
	  // no-op
	  break;
	}
      return 0;
    }
  // user side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received an AlertInd in wrong state: CallPresentState\n");
  qv->Suicide();
  return -1;
}

// pages 66-68 for network side
int CallPresentState::ConnectInd(sean_Visitor *qv, Call *c)
{
  ie_cause::CauseValues cv;
  if(c->StackType() == ie_cause::local_private_network)
    {
      switch (c->VerifyMessage(cv,qv))
	{
	case RAP: 
	  c->SendCallStatusToPeer(ie_call_state::u8_connect_request,cv);
	  ChangeState(c, ConnReqState::Instance());
	  // fall thru
	case OK:
	  c->SetT303();
	  c->Send2CCD(qv);
	  break;

	case CLR:
	  c->DoLabelD(cv);
	  break;

	case RAI:
	  c->SendCallStatusToPeer(ie_call_state::u6_call_present,cv);
	  break;
	case I:
	  // no-op
	  break;
	}
      return 0;
    }
  // user side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received an AlertInd in wrong state: CallPresentState\n");
  qv->Suicide();
  return -1;
}

// pages 66-68 for network side
int CallPresentState::CallProcInd(sean_Visitor *qv, Call *c)
{
  assert(qv != 0 && c != 0);
  if(c->StackType() == ie_cause::local_private_network)
    {
      c->StopT303();
      c->SetT310();
      ChangeState(c, InCallProcState::Instance());
      // forward this CallProcInd so that DF can do the SOLID binding
      c->Send2CCD(qv);
      return(0);
    }
  // user side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a CallProcInd in wrong state: CallPresentState\n");
  qv->Suicide();
  return -1;
}

// pages 66-68 for network side
void CallPresentState::ExpT303(Call *c)
{
  if(c->StackType() == ie_cause::local_private_network)
    c->SendSetupToPeer();
  else
    DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "T303 expired in wrong state: CallPresent\n");
}

// pages 125-126 for user side
int CallPresentState::ReleaseCompReq(sean_Visitor *qv, Call *c)
{
  if(c->StackType() == ie_cause::user)
    {
      if(c->IsP2MP())
	c->RCParties();
      c->Send2Peer(qv);
      c->suicide();
      return -2;
    }
  // network side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a RelaseCompReq in wrong state: CallPresentState\n");
  qv->Suicide();
  return -1;
}

// pages 125-126 for user side
int CallPresentState::CallProcReq(sean_Visitor *qv, Call *c)
{

  if(c->StackType() == ie_cause::user)
    {
      ChangeState(c, InCallProcState::Instance());
      c->Save_APPID(qv->get_shared_app_id());
      c->Send2Peer(qv);
      return 0;
    }
  // network side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a RelaseCompReq in wrong state: CallPresentState\n");
  qv->Suicide();
  return -1;
}

int CallPresentState::AlertReq(sean_Visitor *qv, Call *c)
{
  if(c->StackType() == ie_cause::user)
    {
      if(c->IsP2MP())
	c->AlertParties();
      ChangeState(c, CallReceivedState::Instance());
      c->Send2Peer(qv);
      return 0;
    }
  // network side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a RelaseCompReq in wrong state: CallPresentState\n");
  qv->Suicide();
  return -1;
}

int CallPresentState::ConnectReq(sean_Visitor *qv, Call *c)
{
  if(c->StackType() == ie_cause::user)
    {
      c->SetT313();
      ChangeState(c, ConnReqState::Instance());
      c->Send2Peer(qv);
      return 0;
    }
  // network side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a RelaseCompReq in wrong state: CallPresentState\n");
  qv->Suicide();
  return -1;
}

// --------------------------------------------------------------------
Q93bState *CallReceivedState::_instance = 0;

/* CallReceived:
 * this state exists when a preceding side has recieved an ALERTING
 * message from the succeeding side of the PNNI interface.
 *
 */

CallReceivedState::CallReceivedState(){}
CallReceivedState::~CallReceivedState(){}


Q93bState* CallReceivedState::Instance()
{
  if (!_instance)
    _instance = new CallReceivedState;
  return(_instance);
}

void CallReceivedState::StateName(char *name)
{
  if(name)
    strcpy(name,"CallReceived");
}

ie_call_state::call_state_values CallReceivedState::StateID() { return ie_call_state::u7_call_received;}

// pages 69-71 for network side
int CallReceivedState::PartyAlertInd(sean_Visitor *qv, Call *c)
{
  assert(c != 0 && qv != 0);
  int ret = 0;
  if(c->StackType() == ie_cause::local_private_network)
    {
      if(c->IsP2P())
	c->DoUM(qv);
      else
	{
	  generic_q93b_msg *msg = qv->share_message();
	  assert(msg != 0);
	  int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
	  ie_cause::CauseValues cv;
	  Party *p = 0;
	  switch(c->ValidateEPR(cv,qv))
	    {
	    case YES:
	      p = c->GetParty(epr);
	      assert(p != 0);
	      ret = p->GetPS()->PartyAlertInd(qv,p);
	      if(ret == -2)
		c->KillParty(p);
	      c->Send2CCD(qv);
	      break;

	    case RAI:
	      c->SendCallStatusToPeer(ie_call_state::u7_call_received,cv);
	      // fallthru
	    case I:
	      qv->Suicide();
	      break;

	    case NOPC:
	      c->SendDPAKToPeer(epr,ie_cause::invalid_endpoint_reference);
	      qv->Suicide();
	      break;
	      
	    case CLR:
	      c->DoLabelD(cv);
	      qv->Suicide();
	      break;
	      
	    default:
	      diag("fsm.uni", DIAG_ERROR, "invalid return code from ValidateEPR\n");
	      qv->Suicide();
	      break;
	    }
	}
      return 0;
    }
  // user side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a PartyAlertInd in wrong state: CallReceivedState\n");
  qv->Suicide();
  return -1;
}
int CallReceivedState::AddPartyReq(sean_Visitor *qv, Call *c)
{
  assert(c != 0);
  if(c->StackType() == ie_cause::local_private_network)
    {
      Party *p = c->Init(qv);
      assert(p != 0);
      int ret = p->GetPS()->AddPartyReq(qv,p);
      if(ret == -2)
	c->KillParty(p);
      c->Send2Peer(qv);
      return 0;
    }
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a AddPartyReq in wrong state: CallReceivedState\n");
  qv->Suicide();
  return -1;
}
int CallReceivedState::AddPartyRejInd(sean_Visitor *qv, Call *c)
{
  assert(c != 0);
  int ret = 0;
  if(c->StackType() == ie_cause::local_private_network)
    {
      if(c->IsP2P())
	c->DoUM(qv);
      else
	{
	  generic_q93b_msg *msg = qv->share_message();
	  assert(msg != 0);
	  int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
	  Party *p = 0;
	  ie_cause::CauseValues cv;
	  switch(c->ValidateEPR(cv,qv))
	    {
	    case CLR:
	      c->DoLabelD(cv);
	      qv->Suicide();
	      break;

	    case YES:
	      p = c->GetParty(epr);
	      assert(p != 0);
	      ret = p->GetPS()->PartyAlertInd(qv,p);
	      if(ret == -2)
		c->KillParty(p);
	      c->Send2Peer(qv);
	      break;

	    case RAI:
	      c->SendCallStatusToPeer(ie_call_state::u7_call_received,cv);
	      // fallthru
	    case I:
	    case NOPC:
	      qv->Suicide();
	      break;
	    default:
	      diag("fsm.uni", DIAG_ERROR, "invalid return code from ValidateEPR\n");
	      qv->Suicide();
	      break;
	    }
	}
      return 0;
    }
  // user side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a AddPartyRejInd in wrong state: CallReceivedState\n");
  qv->Suicide();
  return -1;
}
void CallReceivedState::ExpT301(Call *c)
{
  assert(c != 0);
  // cause 19 in Q.850 is no user answering (cause 18 is the same)
  if(c->StackType() == ie_cause::local_private_network)
    c->DoLabelD(ie_cause::no_user_responding);
  else
    DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "T301 expired in wrong state: CallReceivedState\n");
}

int CallReceivedState::ConnectInd(sean_Visitor *qv, Call *c)
{
  assert(c != 0 && qv != 0);
  if(c->StackType() == ie_cause::local_private_network)
    {
      ie_cause::CauseValues cv;
      switch (c->VerifyMessage(cv,qv))
	{
	case RAP: 
	  c->SendCallStatusToPeer(ie_call_state::u8_connect_request,cv);
	  // fall thru
	case OK:
	  ChangeState(c, ConnReqState::Instance());
	  if(c->IsP2P())
	    c->SetT301();
	  c->Send2CCD(qv);
	  break;

	case CLR:
	  c->DoLabelD(cv);
	  qv->Suicide();
	  break;

	case RAI:
	  c->SendCallStatusToPeer(ie_call_state::u7_call_received,cv);
	  break;
	case I:
	  // no-op
	  break;
	}
      return 0;
    }
  // user side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a ConnectInd in wrong state: CallReceivedState\n");
  qv->Suicide();
  return -1;
}

// page 127 for user side
int CallReceivedState::ConnectReq(sean_Visitor *qv, Call *c)   // same as setupResp
{
  assert(c != 0 && qv != 0);
  if(c->StackType() == ie_cause::user)
    {
      c->SetT313();
      ChangeState(c,ConnReqState::Instance());
      c->Send2Peer(qv);
      return 0;
    }
  // network side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a ConnectReq in wrong state: CallReceivedState\n");
  qv->Suicide();
  return -1;
}

int CallReceivedState::PartyAlertReq(sean_Visitor *qv, Call *c)
{
  assert(c != 0 && qv != 0);
  if(c->StackType() == ie_cause::user)
    {
      generic_q93b_msg *msg = qv->share_message();
      assert(msg != 0);
      int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
      Party *p = c->GetParty(epr);
      assert(p != 0);
      int ret = p->GetPS()->PartyAlertReq(qv,p);
      if(ret == -2)
	c->KillParty(p);
      c->Send2Peer(qv);
      return 0;
    }
  // network side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a PartyAlertReq in wrong state: CallReceivedState\n");
  qv->Suicide();
  return -1;
}

int CallReceivedState::AddPartyRejReq(sean_Visitor *qv, Call *c)
{
  assert(c != 0 && qv != 0);
  if(c->StackType() == ie_cause::user)
    {
      generic_q93b_msg *msg = qv->share_message();
      assert(msg != 0);
      int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
      Party *p = c->GetParty(epr);
      assert(p != 0);
      int ret = p->GetPS()->AddPartyRejReq(qv,p);
      if(ret == -2)
	c->KillParty(p);
      c->Send2Peer(qv);
      return 0;
    }
  // network side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a AddPartyRejReq in wrong state: CallReceivedState\n");
  qv->Suicide();
  return -1;
}


// need to check when does this situation arises

int CallReceivedState::AddPartyInd(sean_Visitor *qv, Call *c)
{
  assert(c != 0 && qv != 0);
  if(c->StackType() == ie_cause::user)
    {
      if(c->IsP2P())
	c->DoUM(qv);
      else
	{
	  Party *p = c->Init(qv);
	  assert(p != 0);
	  int ret = p->GetPS()->AddPartyInd(qv,p);
	  if(ret == -2)
	    c->KillParty(p);
	  c->Send2CCD(qv);
	}
      return 0;
    }
  // network side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a AddPartyInd in wrong state: CallReceivedState\n");
  qv->Suicide();
  return -1;
}

// -------------------------------------------------------------------------------------
Q93bState *ConnReqState::_instance = 0;

ConnReqState::ConnReqState(void){}
ConnReqState::~ConnReqState(void){}

Q93bState* ConnReqState::Instance()
{
  if (!_instance)
    _instance = new ConnReqState;
  return(_instance);
}

void ConnReqState::StateName(char *name)
{
  if(name)
    strcpy(name,"ConnReq");
}

ie_call_state::call_state_values ConnReqState::StateID() { return ie_call_state::u8_connect_request;}


// pages 72-73 for network side

// same as SetupCompResp
int ConnReqState::ConnectAckReq(sean_Visitor *qv, Call *c) 
{
  assert(qv != 0 && c != 0);
  if(c->StackType() == ie_cause::local_private_network)
    {
      ChangeState(c,ActiveState::Instance());
      c->Send2Peer(qv);
      return 0;
    }
  // user side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a SetupCompReq in wrong state: ConnReqState\n");
  qv->Suicide();
  return -1;
}

int ConnReqState::AddPartyRejInd(sean_Visitor *qv, Call *c)
{
 assert(c != 0 && qv != 0);
 int ret = 0;
 if(c->StackType() == ie_cause::local_private_network)
   {
     if(c->IsP2P())
       c->DoUM(qv);
     else
       {
	 generic_q93b_msg *msg = qv->share_message();
	 assert(msg != 0);
	 int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
	 ie_cause::CauseValues cv;
	 Party *p = 0;
	 switch(c->ValidateEPR(cv,qv))
	   {
	   case YES:
	     p = c->GetParty(epr);
	     assert(p != 0);
	     ret = p->GetPS()->AddPartyRejInd(qv,p);
	     if(ret == -2)
	       c->KillParty(p);
	     c->Send2CCD(qv);
	     break;

	   case RAI:
	     c->SendCallStatusToPeer(ie_call_state::u8_connect_request,cv);
	     // fallthru
	   case I:
	   case NOPC:
	     qv->Suicide();
	     break;
	      
	   case CLR:
	     c->DoLabelD(cv);
	     qv->Suicide();
	     break;
	      
	   default:
	     diag("fsm.uni", DIAG_ERROR, "invalid return code from ValidateEPR\n");
	     qv->Suicide();
	     break;
	   }
       }
     return 0;
   }
  // user side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received an AddPartyRejInd in wrong state: ConnReqState\n");
  qv->Suicide();
  return -1;
}

int ConnReqState::AddPartyAckInd(sean_Visitor *qv, Call *c)
{
  assert(c != 0 && qv != 0);
  int ret = 0;
  if(c->StackType() == ie_cause::local_private_network)
    {
      if(c->IsP2P())
	c->DoUM(qv);
      else
	{
	  generic_q93b_msg *msg = qv->share_message();
	  assert(msg != 0);
	  int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
	  ie_cause::CauseValues cv;
	  Party *p = 0;
	  switch(c->ValidateEPR(cv,qv))
	    {
	    case YES:
	      p = c->GetParty(epr);
	      assert(p != 0);
	      ret = p->GetPS()->AddPartyAckInd(qv,p);
	      if(ret == -2)
		c->KillParty(p);
	      c->Send2CCD(qv);
	      break;

	    case RAI:
	      c->SendCallStatusToPeer(ie_call_state::u8_connect_request,cv);
	      // fallthru
	    case I:
	      qv->Suicide();
	      break;

	    case NOPC:
	      c->SendDPAKToPeer(epr,ie_cause::invalid_endpoint_reference);
	      qv->Suicide();
	      break;
	      
	    case CLR:
	      c->DoLabelD(cv);
	      qv->Suicide();
	      break;
	      
	    default:
	      diag("fsm.uni", DIAG_ERROR, "invalid return code from ValidateEPR\n");
	      qv->Suicide();
	      break;
	    }
	}
      return 0;
    }
  // user side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received an AddPartyAckInd in wrong state: ConnReqState\n");
  qv->Suicide();
  return -1;
}

int ConnReqState::PartyAlertInd(sean_Visitor *qv, Call *c)
{
  assert(c != 0 && qv != 0);
  int ret = 0;
  if(c->StackType() == ie_cause::local_private_network)
    {
      if(c->IsP2P())
	c->DoUM(qv);
      else
	{
	  generic_q93b_msg *msg = qv->share_message();
	  assert(msg != 0);
	  int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
	  ie_cause::CauseValues cv;
	  Party *p = 0;
	  switch(c->ValidateEPR(cv,qv))
	    {
	    case YES:
	      p = c->GetParty(epr);
	      assert(p != 0);
	      ret = p->GetPS()->PartyAlertInd(qv,p);
	      if(ret == -2)
		c->KillParty(p);
	      c->Send2CCD(qv);
	      break;

	    case RAI:
	      c->SendCallStatusToPeer(ie_call_state::u8_connect_request,cv);
	      // fallthru
	    case I:
	      qv->Suicide();
	      break;

	    case NOPC:
	      c->SendDPAKToPeer(epr,ie_cause::invalid_endpoint_reference);
	      qv->Suicide();
	      break;
	      
	    case CLR:
	      c->DoLabelD(cv);
	      qv->Suicide();
	      break;
	      
	    default:
	      diag("fsm.uni", DIAG_ERROR, "invalid return code from ValidateEPR\n");
	      qv->Suicide();
	      break;
	    }
	}
      return 0;
    }
  // user side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received an PartyAlertInd in wrong state: ConnReqState\n");
  qv->Suicide();
  return -1;
}

// pages 128-129 for user side

void ConnReqState::ExpT313(Call *c)
{
  assert(c != 0);
  if(c->StackType() == ie_cause::user)
    c->DoLabelD(ie_cause::recovery_on_timer_expiry);
  else     // network side
    DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "T313 in wrong state: ConnReqState\n");
}

int ConnReqState::ConnectAckInd(sean_Visitor *qv, Call *c)
{
  assert(c != 0 && qv != 0);
  ie_cause::CauseValues cv;
  if (c->StackType() == ie_cause::user) {
    switch(c->VerifyMessage(cv,qv)) {
	case RAP: 
	  // send a status with cs = u10_active and fall thru
	  c->SendCallStatusToPeer(ie_call_state::u10_active,ie_cause::normal_unspecified);
	case OK:
	  c->SetT313();
	  ChangeState(c,ActiveState::Instance());
	  if (c->IsP2MP())
	    c->ConnAckParty(); // this call must be a leaf thus 1 party
	  c->Send2CCD(qv);
	  break;
	case CLR:
	  c->DoLabelD(ie_cause::normal_unspecified);
	  break;
	case RAI:
	  // send a status with cs = u8_connect_request and fall thru
	  c->SendCallStatusToPeer(ie_call_state::u8_connect_request,ie_cause::normal_unspecified);
	case I:
	  break;
    }
    return 0;
  }
  // network side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a ConnectAckInd in wrong state: ConnReqState\n");
  qv->Suicide();
  return -1;
}

int ConnReqState::PartyAlertReq(sean_Visitor *qv, Call *c)
{
  assert(c != 0 && qv != 0);
  if(c->StackType() == ie_cause::user)
    {
      generic_q93b_msg *msg = qv->share_message();
      assert(msg != 0);
      int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
      Party *p = c->GetParty(epr);
      assert(p != 0);
      int ret = p->GetPS()->PartyAlertReq(qv,p);
      if(ret == -2)
	c->KillParty(p);
      c->Send2Peer(qv);
      return 0;
    }
  // network side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a ConnectReq in wrong state: ConnReqState\n");
  qv->Suicide();
  return -1;
}

int ConnReqState::AddPartyRejReq(sean_Visitor *qv, Call *c)
{
  assert(c != 0 && qv != 0);
  if(c->StackType() == ie_cause::user)
    {
      generic_q93b_msg *msg = qv->share_message();
      assert(msg != 0);
      int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
      Party *p = c->GetParty(epr);
      assert(p != 0);
      int ret = p->GetPS()->AddPartyRejReq(qv,p);
      if(ret == -2)
	c->KillParty(p);
      c->Send2Peer(qv);
      return 0;
    }
  // network side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a ConnectReq in wrong state: ConnReqState\n");
  qv->Suicide();
  return -1;
}

int ConnReqState::AddPartyInd(sean_Visitor *qv, Call *c)
{
  assert(c != 0 && qv != 0);
  if(c->StackType() == ie_cause::user)
    {
      if(c->IsP2P())
        c->DoUM(qv);
      else
        {
	  Party *p = c->Init(qv);
          assert(p != 0);
          int ret = p->GetPS()->AddPartyInd(qv,p);
	  if(ret == -2)
	    c->KillParty(p);
          c->Send2CCD(qv);
        }
      return 0;
    }
  // network side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a ConnectReq in wrong state: ConnReqState\n");
  qv->Suicide();
  return -1;
}

// ----------------------------------------------------------------------------------------------
Q93bState *InCallProcState::_instance = 0;

/* InCallProc:
 * when a prededing side has received acknowledgement
 * that the succeeding side has received the call establishment request.
 *
 */
InCallProcState::InCallProcState(void){}
InCallProcState::~InCallProcState(void){}

Q93bState* InCallProcState::Instance()
{
  if (!_instance)
    _instance = new InCallProcState;
  return(_instance);
}

void InCallProcState::StateName(char *name)
{
  if(name)
    strcpy(name,"IncomingCallProceeding");
}

ie_call_state::call_state_values InCallProcState::StateID() { return ie_call_state::u9_incoming_call_proceeding;}

// page 74 for network side
int InCallProcState::AlertInd(sean_Visitor *qv, Call *c)
{
  assert(c != 0 && qv != 0);
  int ret = 0;
  if(c->StackType() == ie_cause::local_private_network)
    {
      generic_q93b_msg *msg = qv->share_message();
      ie_cause::CauseValues cv = ie_cause::normal_unspecified;
      assert(msg != 0);
      if(msg->ie(InfoElem::ie_cause_ix))
	cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
      switch(c->VerifyMessage(cv,qv)) {
        case RAP: 
          c->SendCallStatusToPeer(ie_call_state::u7_call_received,cv);
          // fall thru
        case OK:
          c->StopT310();
	  // do C3
	  ChangeState(c,CallReceivedState::Instance());
	  if(c->IsP2P())
	    c->SetT301();
	  else
	    {
	      int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
	      Party *p = c->GetParty(epr);
	      assert(p != 0);
	      ret = p->GetPS()->PartyAlertInd(qv,p);
	      if(ret == -2)
		c->KillParty(p);
	    }
          c->Send2CCD(qv);
          break;

        case RAI:
          // send a status with cs = u9_incoming_call_proceeding
          c->SendCallStatusToPeer(ie_call_state::u9_incoming_call_proceeding,cv);
          // fall thru
        case I:
	  qv->Suicide();
          break;

        case CLR:
          c->DoLabelD(cv);
	  qv->Suicide();
          break;
      }
      return 0;
    }
  // user side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received an AlertInd in wrong state: InCallProcState\n");
  qv->Suicide();
  return -1;
}

int InCallProcState::ConnectInd(sean_Visitor *qv, Call *c)
{
  assert(qv != 0 && c != 0);
  if(c->StackType() == ie_cause::local_private_network)
    {
      generic_q93b_msg *msg = qv->share_message();
      ie_cause::CauseValues cv = ie_cause::normal_unspecified;
      assert(msg != 0);
      if(msg->ie(InfoElem::ie_cause_ix))
	cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
      switch(c->VerifyMessage(cv,qv)) {
        case RAP: 
          c->SendCallStatusToPeer(ie_call_state::u8_connect_request,cv);
          // fall thru
        case OK:
          c->StopT310();
	  ChangeState(c,ConnReqState::Instance());
          c->Send2CCD(qv);
          break;

        case RAI:
          // send a status with cs = u9_incoming_call_proceeding
          c->SendCallStatusToPeer(ie_call_state::u9_incoming_call_proceeding,cv);
          // fall thru
        case I:
	  qv->Suicide();
          break;

        case CLR:
          c->DoLabelD(cv);
	  qv->Suicide();
          break;
      }
      return 0;
    }
  // user side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a ConnectInd in wrong state: InCallProcState\n");
  qv->Suicide();
  return -1;
}

void InCallProcState::ExpT310(Call *c)
{
  assert(c != 0);
  // cause 18 and 19 are the same
  if(c->StackType() == ie_cause::local_private_network)
    c->DoLabelD(ie_cause::no_user_responding);
  else
    DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "T310 expired in wrong state: InCallProcState\n");
}

// page 130 user side
int InCallProcState::AlertReq(sean_Visitor *qv, Call *c)
{
  assert(c != 0);
  if(c->StackType() == ie_cause::user)
    {
      if(c->IsP2MP())
	{
	  generic_q93b_msg *msg = qv->share_message();
	  assert(msg != 0);
	  int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
	  Party *p = c->GetParty(epr);
	  assert(p != 0);
	  int ret = p->GetPS()->AlertReq(qv,p);
	  if(ret == -2)
	    c->KillParty(p);
	}
      c->Send2Peer(qv);
      ChangeState(c, CallReceivedState::Instance());
      return(0);
    }
  // error -- network side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "AlertReq message received in wrong state: InCallProcState\n");
  qv->Suicide();
  return -1;
}

int InCallProcState::ConnectReq(sean_Visitor *qv, Call *c)
{
  assert(c != 0);
  if(c->StackType() == ie_cause::user)
    {
      c->SetT313();
      ChangeState(c, ConnReqState::Instance());
      c->Send2Peer(qv);
      return 0;
    }
  // error -- network side
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "ConnectReq message received in wrong state: InCallProcState\n");
  qv->Suicide();
  return -1;
}

// ----------------------------------------------------------------------------------
Q93bState * ActiveState::_instance = 0;

ActiveState::ActiveState(){}
ActiveState::~ActiveState(){}

Q93bState* ActiveState::Instance()
{
  if (!_instance)
    _instance = new ActiveState;
  return(_instance);
}

void ActiveState::StateName(char *name)
{
  if(name)
    strcpy(name,"CallActive");
}

ie_call_state::call_state_values ActiveState::StateID() { return ie_call_state::u10_active;}

int ActiveState::ConnectAckInd(sean_Visitor *qv, Call *c)
{
  if (c->StackType() == ie_cause::user)
    {
      DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "ConnectAck message received in wrong state: ActiveSate\n");
      qv->Suicide();
      return -1;
    }
  ie_cause::CauseValues cv;
  switch (c->VerifyMessage(cv,qv)) {
    case RAP: 
    case RAI:
      c->SendCallStatusToPeer(ie_call_state::u10_active,cv);
      c->Send2CCD(qv);
      break;
    case CLR:
      c->DoLabelD(ie_cause::normal_unspecified);
      qv->Suicide();
    break;
    case OK:
    case I:
      c->Send2CCD(qv);
      break;
  }
  return 0;
}

// page 77 & 132 Q.2971
int ActiveState::LinkReleaseInd(sscfVisitor *lsv, Call *c)
{
  if(c->IsP2MP())
    c->RCNonActiveParties();
  // turn it into a request and send it down to reestablish the link
  lsv->SetVT(sscfVisitor::EstReq);
  c->Send2Peer(lsv);
  return 0;
}

int ActiveState::LinkEstablishError(sscfVisitor *lsv, Call *c)
{
  if(c->IsP2MP())
    c->RCParties();
  // send an ReleaseConf with cause 27
  lsv->Suicide();
  c->SendReleaseCompIndToCCD(ie_cause::destination_out_of_order);
  c->suicide();
  return -2;
}

int ActiveState::LinkEstablishInd(sscfVisitor *lsv, Call *c)
{
  if(c->IsP2MP())
    c->SendPartyStatusEnqReq(true); 
  else
    if(!c->IsTimer322Active())
      c->SendStatusEnqToPeer();
  lsv->Suicide();
  return 0;
}

int ActiveState::LinkEstablishConf(sscfVisitor *lsv, Call *c)
{
  if(c->IsP2MP())
    c->SendPartyStatusEnqReq(false);
  else
    if(!c->IsTimer322Active())
      c->SendStatusEnqToPeer();
  lsv->Suicide();
  return 0;
}

// page 78 & 134 Q.2971
// I'am root
int ActiveState::AddPartyReq( sean_Visitor *qv,  Call *c)
{
  assert(qv != 0 && c != 0);
  int epr = qv->get_pid();
  Party *party = c->GetParty(epr);
  if (party) {
    DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "party already exists epr = " << epr << endl;);
    qv->Suicide();
    return -1;
  }
  if (party = c->InitParty(qv)) {
    int ret = party->GetPS()->AddPartyReq(qv,party);
    if(ret == -2)
      c->KillParty(party);
    c->Send2Peer(qv);
  }
  return 0;
}

int ActiveState::AddPartyAckReq( sean_Visitor *qv,  Call *c)
{
  assert(qv != 0 && c != 0);
  Party *party = c->GetParty(qv->get_pid());
  if(party == 0)
    {
      DIAG(Q93B_DEBUGGING, DIAG_DEBUG, \
	   cout <<  "Bad party in AddPartyAckReq, re: " << qv->get_pid() << \
	            ", party_map size:" << c->NumParties() << endl; );
      qv->Suicide();
      return -1;
    }
  int ret = party->GetPS()->AddPartyAckReq(qv,party);
  if(ret == -2)
    c->KillParty(party);
  c->Send2Peer(qv);
  return 0;
}

int ActiveState::AddPartyRejReq( sean_Visitor *qv,  Call *c)
{
  assert(qv != 0 && c != 0);
  Party *party = c->GetParty(qv->get_pid());
  if(party == 0)
    {
      DIAG(Q93B_DEBUGGING, DIAG_DEBUG, \
	   cout <<  "Bad party in AddPartyRejReq, re: " << qv->get_pid() << \
	            ", party_map size:" << c->NumParties() << endl; );
      qv->Suicide();
      return -1;
    }
  int ret = party->GetPS()->AddPartyRejReq(qv,party);
  if(ret == -2)
    c->KillParty(party);
  c->Send2Peer(qv);
  return 0;
}

int ActiveState::PartyAlertReq( sean_Visitor *qv,  Call *c)
{
  assert(qv != 0 && c != 0);
  Party *party = c->GetParty(qv->get_pid());
  if(party == 0)
    {
      DIAG(Q93B_DEBUGGING, DIAG_DEBUG, \
	   cout <<  "Bad party in PartyAlertReq, re: " << qv->get_pid() << \
	            ", party_map size:" << c->NumParties() << endl; );
      qv->Suicide();
      return -1;
    }
  int ret = party->GetPS()->PartyAlertReq(qv,party);
  if(ret == -2)
    c->KillParty(party);
  c->Send2Peer(qv);
  return 0;
}

int ActiveState::DropPartyReq( sean_Visitor *qv,  Call *c)
{
  assert(qv != 0 && c != 0);
  Party *party = c->GetParty(qv->get_pid());
  if(party == 0)
    {
      DIAG(Q93B_DEBUGGING, DIAG_DEBUG, \
	   cout <<  "Bad party in DropPartyReq, re: " << qv->get_pid() << \
	            ", party_map size:" << c->NumParties() << endl; );
      qv->Suicide();
      return -1;
    }
  int ret = party->GetPS()->DropPartyReq(qv,party);
  if(ret == -2)
    {
      c->KillParty(party);
    }
  c->Send2Peer(qv);
  return 0;
}

int ActiveState::DropPartyAckReq( sean_Visitor *qv,  Call *c)
{
  assert(qv != 0 && c != 0);
  Party *party = c->GetParty(qv->get_pid());
  if(party == 0)
    {
      DIAG(Q93B_DEBUGGING, DIAG_DEBUG,                                      \
	   cout << "Bad party in DropPartyAckReq, re: " << qv->get_pid() << \
	   ", party_map size:" << c->NumParties() << " -- ";                \
	   c->PrintPartyMap();                                              \
	   if (c->StackType() == ie_cause::user) {                          \
             cout << "USER";                                                \
	   }                                                                \
	   else {                                                           \
             cout << "NET";                                                 \
	   }                                                                \
	   cout << endl;                                                    \
      );

      qv->Suicide();
      return -1;
    }
  int ret = party->GetPS()->DropPartyAckReq(qv,party);
  if(ret == -2)
    {
      c->KillParty(party);
    }
  c->Send2Peer(qv);
  return 0;
}

int ActiveState::AddPartyAckInd(sean_Visitor *qv,  Call *c)
{
  assert(qv != 0 && c != 0);
  if(c->IsP2P())
    c->DoUM(qv);
  else
    {
      generic_q93b_msg *msg = qv->share_message();
      assert(msg != 0);
      int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
      Party *party = c->GetParty(epr);
      if(party == 0)
	{
	  c->SendDPAKToPeer(epr,ie_cause::invalid_endpoint_reference);
	  qv->Suicide();
	  return 0;
	}
      int ret = party->GetPS()->AddPartyAckInd(qv,party);
      if(ret == -2)
	c->KillParty(party);
      c->Send2CCD(qv);
    }
  return 0;
}

int ActiveState::PartyAlertInd(sean_Visitor *qv,  Call *c)
{
  // do C8 FIX the decode
  assert(qv != 0 && c != 0);
  if(c->IsP2P())
    c->DoUM(qv);
  else
    {
      generic_q93b_msg *msg = qv->share_message();
      assert(msg != 0);
      int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
      Party *party = c->GetParty(epr);
      if(party == 0)
	{
	  c->SendDPAKToPeer(epr,ie_cause::invalid_endpoint_reference);
	  qv->Suicide();
	  return 0;
	}
      int ret = party->GetPS()->PartyAlertInd(qv,party);
      if(ret == -2)
	c->KillParty(party);
      c->Send2CCD(qv);
    }
  return 0;
}

int ActiveState::AddPartyInd(sean_Visitor *qv,  Call *c)
{
  assert(qv != 0 && c != 0);
  if(c->IsP2P())
    c->DoUM(qv);
  else
    {
      generic_q93b_msg *msg = qv->share_message();
      assert(msg != 0);
      int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
      Party *party = c->GetParty(epr);
      if(party == 0)
	{
	  if(!(party = c->InitParty(qv)))
	    return -1;
	  int ret = party->GetPS()->AddPartyInd(qv,party);
	  if(ret == -2)
	    c->KillParty(party);
	  c->Send2CCD(qv);
	  return 0;
	}
      DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "epr already in use\n");
      qv->Suicide();
    }
  return 0;
}

int ActiveState::AddPartyRejInd(sean_Visitor *qv,  Call *c)
{
  // do C9 on page 134 Q.2971 
  assert(qv != 0 && c != 0);
  if(c->IsP2P())
    c->DoUM(qv);
  else
    {
      generic_q93b_msg *msg = qv->share_message();
      assert(msg != 0);
      int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
      Party *party = c->GetParty(epr);
      if(party)
	{
	  int ret = party->GetPS()->AddPartyRejInd(qv,party);
	  if(ret == -2)
	    c->KillParty(party);
	  c->Send2CCD(qv);
	  return 0;
	}
      DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "epr already in use\n");
      qv->Suicide();
    }
  return 0;
}

int ActiveState::DropPartyInd(sean_Visitor *qv,  Call *c)
{
  // do C9 on page 134 Q.2971
  assert(qv != 0 && c != 0);
  if(c->IsP2P())
    c->DoUM(qv);
  else
    {
      generic_q93b_msg *msg = qv->share_message();
      assert(msg != 0);
      int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
      Party *party = c->GetParty(epr);
      if(party)
	{
	  int ret = party->GetPS()->DropPartyInd(qv,party);
	  if((c->StackType() == ie_cause::local_private_network) &&
	     (party->get_eps() == ie_end_pt_state::add_party_initiated))
	    {
	      // special case 9.3.2 b) 
	      c->SendAPRejToPeer(epr, ie_cause::normal_unspecified);
	      if((c->AnyParty(ie_end_pt_state::p_active,epr) == false) &&
		 (c->AnyParty(ie_end_pt_state::party_alert_delivered,epr) == false) &&
		 (c->AnyParty(ie_end_pt_state::add_party_received,epr) == false))
		// send a RELEASE
		c->SendReleaseToPeer(ie_cause::normal_unspecified);
	    }
	  if(ret == -2)
	    c->KillParty(party);
	  c->Send2CCD(qv);
	  return 0;
	}
      DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "epr already in use\n");
      qv->Suicide();
    }
  return 0;
}

int ActiveState::DropPartyAckInd(sean_Visitor *qv,  Call *c)
{
  // do C9 on page 134 Q.2971
  assert(qv != 0 && c != 0);
  if(c->IsP2P())
    {
      c->DoUM(qv);
      return 0;
    }
  // this is a P2MP call
  generic_q93b_msg *msg = qv->share_message();
  assert(msg != 0);
  int epr = ((ie_end_pt_ref *)msg->ie(InfoElem::ie_end_pt_ref_ix))->get_epr_value();
  Party *party = c->GetParty(epr);
  if(party)
    {
      int ret = party->GetPS()->DropPartyAckInd(qv,party);
      if(ret == -2)
	c->KillParty(party);
      c->Send2CCD(qv);
      return 0;
    }
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "epr already in use\n");
  qv->Suicide();
  return 0;
}

// ------------------------------------------------------------------------------
Q93bState *ReleaseReqState::_instance = 0;

/* ReleaseReq: This state exists when a network node has sent a request to
 * the network node at the other side of the PNNI interface to release the
 * ATM connection and has not responded yet.
 */

ReleaseReqState::ReleaseReqState(void){}
ReleaseReqState::~ReleaseReqState(void){}

Q93bState* ReleaseReqState::Instance()
{
  if (!_instance)
    _instance = new ReleaseReqState;
  return(_instance);
}

void ReleaseReqState::StateName(char *name)
{
  if(name)
    strcpy(name,"ReleaseReq");
}

ie_call_state::call_state_values ReleaseReqState::StateID()
{
  return ie_call_state::u11_release_request;
}

// don't do page 84 of Q.2971
int ReleaseReqState::DropPartyReq(sean_Visitor *qv, Call *c)
{
  if(qv)
    qv->Suicide();
  return -1;
}

// don't do  page 84 of Q.2971
int ReleaseReqState::DropPartyAckReq(sean_Visitor *qv, Call *c)
{
  if(qv)
    qv->Suicide();
  return -1;
}

// don't do page 85 Q.2971
int ReleaseReqState::ReleaseReq(sean_Visitor *qv, Call *c)
{
  if(qv)
    qv->Suicide();
  return -1;
}

// page 86 Q.2971 same as ReLeaseResp
int ReleaseReqState::ReleaseCompReq(sean_Visitor *qv, Call *c)
{
  assert(qv != 0 && c != 0);
  c->Send2Peer(qv);
  ChangeState(c, NullState::Instance());
  c->suicide();
  return -2;
}


// page 86 Q.2971
int ReleaseReqState::DropPartyInd(sean_Visitor *qv, Call *c)
{
  if(qv)
    qv->Suicide();
  return -1;
}

// page 86 Q.2971
int ReleaseReqState::DropPartyAckInd(sean_Visitor *qv, Call *c)
{
  if(qv)
    qv->Suicide();
  return -1;
}


// page 86 Q.2971
int ReleaseReqState::StatusInd(sean_Visitor *qv, Call *c)
{
   generic_q93b_msg *msg = 0L;
  ie_cause::CauseValues cv;
  assert(qv != 0);
  switch(c->VerifyMessage(cv,qv)) {
    case RAP: 
    case RAI:
      // send a status with cs = u11_release_req and cause in the status msg
      msg = qv->share_message();
      assert(msg != 0);
      cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
      c->SendCallStatusToPeer(ie_call_state::u11_release_request,cv);
      // fall thru
    case OK:
    case I:
      if (StateID() == ie_call_state::u0_null) {
	c->suicide();
	return -2;
      }
      break;
    case CLR:
      msg = qv->share_message();
      assert(msg != 0);
      cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
      c->DoLabelD(cv);
      break;
  }
  return 0;

}

// page 141 Q.2971 -- user side only
int ReleaseReqState::ReleaseInd(sean_Visitor *qv, Call *c)
{
  if(c->StackType() != ie_cause::user)
    {
      DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a RelaseInd in wrong state: ReleaseReqState\n");
      qv->Suicide();
      return -1;
    }
  // user side
  c->StopT308();
  generic_q93b_msg *msg = qv->share_message();
  assert(msg != 0);
  ie_cause::CauseValues cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
  c->SendReleaseCompIndToCCD(cv);
  qv->Suicide();
  c->suicide();
  return -2;
}


// page 141 Q.2971 -- user side only
// do C10
void ReleaseReqState::ExpT308(Call *c)
{
  // we only get here on first expiry
  if(c->StackType() == ie_cause::user)
    {
      c->SetT308();
      c->SendReleaseToPeer(ie_cause::recovery_on_timer_expiry);
    }
  else
    DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "T308 expired in wrong state: ReleaseReqState\n");
}

// -----------------------------------------------------------------------
Q93bState *ReleaseIndState::_instance = 0;

/* ReleaseInd:
 * This state exists when a network node has received a request from 
 * the network node at the other side of the PNNI interface to release
 * the ATM connection and has not responded yet.
 */
ReleaseIndState::ReleaseIndState(void){}
ReleaseIndState::~ReleaseIndState(void){}

Q93bState* ReleaseIndState::Instance()
{
  if (!_instance)
    _instance = new ReleaseIndState;
  return(_instance);
}

void ReleaseIndState::StateName(char *name)
{
  if(name)
    strcpy(name,"ReleaseInd");
}

ie_call_state::call_state_values ReleaseIndState::StateID() { return ie_call_state::u12_release_indication;}

int ReleaseIndState::ReleaseInd(sean_Visitor *qv, Call *c)
{
  if(c->StackType() == ie_cause::user)
    {
      DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "Received a RelaseInd in wrong state: ReleaseIndState\n");
      qv->Suicide();
      return 0;
    }
  // Network side
  c->StopT308();
  generic_q93b_msg *msg = qv->share_message();
  assert(msg != 0);
  ie_cause::CauseValues cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
  c->SendReleaseCompIndToCCD(cv);
  qv->Suicide();
  c->suicide();
  return -2;
}

// see page 84 of Q.2971
int ReleaseIndState::DropPartyReq(sean_Visitor *qv, Call *c)
{
  if(qv)
    qv->Suicide();
  return -1;
}

int ReleaseIndState::DropPartyAckReq(sean_Visitor *qv, Call *c)
{
  if(qv)
    qv->Suicide();
  return -1;
}

int ReleaseIndState::ReleaseReq(sean_Visitor *qv, Call *c) { qv->Suicide();return -1; }
int ReleaseIndState::DropPartyInd(sean_Visitor *qv, Call *c) {qv->Suicide(); return 0; }


void ReleaseIndState::ExpT308(Call *c)
{
  // we only get here in first expiry
  // only network side
  if(c->StackType() == ie_cause::local_private_network)
    {
      c->SetT308();
      c->SendReleaseToPeer(ie_cause::recovery_on_timer_expiry);
    }
  else
    DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  "T308 expired in wrong state: ReleaseIndState\n");
}

// Also called ReleaseResp
int ReleaseIndState::ReleaseCompReq(sean_Visitor *qv, Call *c)
{
  assert(qv != 0 && c != 0);
  c->Send2Peer(qv);
  ChangeState(c, NullState::Instance());
  c->suicide();
  return -2;
}


int ReleaseIndState::StatusInd(sean_Visitor *qv, Call *c)
{
  generic_q93b_msg *msg = 0L;
  ie_cause::CauseValues cv;
  assert(qv != 0);
  switch (c->VerifyMessage(cv,qv)) {
    case RAP: 
    case RAI:
      // send a status with cs = u12_release_indication and cause in the status msg
      msg = qv->share_message();
      assert(msg != 0);
      cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
      c->SendCallStatusToPeer(ie_call_state::u12_release_indication,cv);
      // fall thru
    case OK:
    case I:
      if (StateID() == ie_call_state::u0_null) {
	c->suicide();
	return -2;
      }
      break;
    case CLR:
      msg = qv->share_message();
      assert(msg != 0);
      cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
      c->DoLabelD(cv);
      break;
  }
  return 0;
}
