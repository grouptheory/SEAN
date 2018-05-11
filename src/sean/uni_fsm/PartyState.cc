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
static char const _PartyState_cc_rcsid_[] =
"$Id: PartyState.cc,v 1.23 1999/03/31 20:50:26 battou Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/basics/diag.h>
#include <codec/q93b_msg/q93b_msg.h>

#include "Q93bParty.h"
#include "PartyState.h"
#include "Q93bCall.h"
#include <sean/cc/sean_Visitor.h>

#define Q93B_DEBUGGING "fsm.uni.Q93B_debugging"


PartyState *P0_Null::_instance = 0;
PartyState *P1_AddPartyInit::_instance = 0;
PartyState *P2_AddPartyRcv::_instance = 0;
PartyState *P3_PartyAlertDel::_instance = 0;
PartyState *P4_PartyAlertRcv::_instance = 0;
PartyState *P5_DropPartyInit::_instance = 0;
PartyState *P6_DropPartyRcv::_instance = 0;
PartyState *P7_PartyActive::_instance = 0;

PartyState::PartyState()
{
}

PartyState::~PartyState()
{
}

// DEFAULTS

// Signals related to primitives
int PartyState::SetupReq(sean_Visitor *qv, Party *p){ return -1; }

int PartyState::AlertReq(sean_Visitor *qv, Party *p)
{
  char name[80];
  StateName(name);
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  " Received an AlertReq in wrong state " << name << endl);
  return -1;
}

int PartyState::ConnectReq(sean_Visitor *qv, Party *p)
{
  char name[80];
  StateName(name);
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  " Received an ConnectReq (SetupResp) in wrong state " << name << endl);
  return -1;
}



int PartyState::ConnectInd(sean_Visitor *qv, Party *p)
{
  char name[80];
  StateName(name);
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  " Received an ConnectInd in wrong state " << name << endl);
  return -1;
}

int PartyState::ConnectAckInd(sean_Visitor *qv, Party *p)
{
  char name[80];
  StateName(name);
  DIAG(Q93B_DEBUGGING, DIAG_DEBUG, cout <<  " Received an ConnectAckInd in wrong state " << name << endl);
  return -1;
}


int PartyState::AddPartyReq(sean_Visitor *qv, Party *p){ return -1; }


int PartyState::SetupCompReq(sean_Visitor *qv, Party *p){ return -1; }

int PartyState::AddPartyAckReq(sean_Visitor *qv, Party *p){ return -1; }

int PartyState::AddPartyRejReq(sean_Visitor *qv, Party *p){ return -1; }

int PartyState::PartyAlertReq(sean_Visitor *qv, Party *p){ return -1; }

int PartyState::AddPartyAckInd(sean_Visitor *qv, Party *p){ return -1; }

// this is the default action -- P1,P3,P4,and P7 redefine as on page 103 Q.2971
int PartyState::DropPartyReq(sean_Visitor *qv, Party *p)
{
  return -1;
}

int PartyState::DropPartyAckReq(sean_Visitor *qv, Party *p)
{
  return -1;
}

int PartyState::DropPartyInd(sean_Visitor *qv, Party *p)
{
  assert(p != 0);
  Call *c = p->GetOwner();
  assert(c != 0);
  ie_cause::CauseValues cv;
  switch (c->VerifyMessage(cv,qv))
    {
    case CLR:
      break;

    default:
      p->StopT397();
      p->StopT399();
      ChangeState(p,P6_DropPartyRcv::Instance());
      break;
    }
  return 0;
}

int PartyState::DropPartyAckInd(sean_Visitor *qv, Party *p)
{
  Call *c = p->GetOwner();
  assert(c != 0);
  ie_cause::CauseValues cv;
  switch (c->VerifyMessage(cv,qv))
    {
    case CLR:
      return 0; // label D
      break;
    case CLP:
    case RAP: 
    case OK:
    case RAI:
    case I:
      p->StopT397();
      p->StopT398();
      p->StopT399();
      break;
    }
  return -2;
}

// page 164 Q2971
int PartyState::PartyStatusEnqReq(sean_Visitor *qv, Party *p)
{
  // The visitor carrying the req will flow down in CC
  return 0;
}

// page 107/164 Q2971
int PartyState::StatusEnqInd(sean_Visitor *qv, Party *p)
{
  assert(p != 0);
  Call *c = p->GetOwner();
  assert(c != 0);
  c->SendPartyStatusToPeer(ie_cause::response_to_status_enquiry,p->get_eps(), p->get_epr());
  return 0;
}

// page 107/164 Q2971
// PS: P0 and P5 have to redefine
int PartyState::StatusInd(sean_Visitor *qv, Party *p)
{
  assert(p != 0 && qv != 0);
  assert(p != 0);
  Call *c = p->GetOwner();
  assert(c != 0);
  generic_q93b_msg *msg = qv->share_message();
  assert(msg != 0);
  InfoElem **ies = msg->get_ie_array();
  ie_cause::CauseValues cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
  if(p->get_eps() == ie_end_pt_state::p_null)
    {
      p->StopT397();
      p->StopT398();
      p->StopT399();
      p->GetOwner()->SendDPAKToCCD(p->get_epr(),cv);
      return -2;
    }
  // get end point state of the remote node from msg and check compatibility
  ie_end_pt_state::party_state epr_state = ((ie_end_pt_state *)msg->ie(InfoElem::ie_end_pt_state_ix))->get_epr_state();
  if(p->get_eps() == epr_state)
    {
      if(cv == ie_cause::mandatory_information_element_is_missing ||
	 cv == ie_cause::message_type_nonexistent_or_not_implemented ||
	 cv == ie_cause::information_element_nonexistent_or_not_implemented ||
	 cv == ie_cause::invalid_information_element_contents )
	{
	  // do label E
	  c->SendDPIndToCCD(p->get_epr(),cv);
	  p->StopT399();
	  p->SetT398();
	  c->SendDPToPeer(p->get_epr(),cv);
	}
      return 0;
    }
  // imcompatible state do E with cause 101 (ie_cause::message_not_compatible_with_call_state)
  c->SendDPIndToCCD(p->get_epr(),ie_cause::message_not_compatible_with_call_state);
  p->StopT399();
  p->SetT398();
  c->SendDPToPeer(p->get_epr(),ie_cause::message_not_compatible_with_call_state);
  return 0;
}

int PartyState::StatusEnqReq(sean_Visitor *qv, Party *p)
{
  return 0;
}

int PartyState::AlertInd(sean_Visitor *qv, Party *p)
{
  return -1;
}

// default behavior on page 106/163 Q.2971
// PS: P0 and P5 have to redefine
int PartyState::ReleaseInd(sean_Visitor *qv, Party *p)
{
  p->StopT397();
  p->StopT399();
  return -2;
}

// default behavior on page 106/163 Q.2971
// PS: P0 and P5 have to redefine
int PartyState::ReleaseReq(sean_Visitor *qv, Party *p)
{
  p->StopT397();
  p->StopT399();
  return -2;
}

// default behavior on page 106/163 Q.2971
// PS: P0 has to redefine
int PartyState::ReleaseCompInd(sean_Visitor *qv, Party *p)
{
  p->StopT397();
  p->StopT398();
  p->StopT399();
  return -2;
}

// default behavior on page 106/163 Q.2971
// PS: P0 has to redefine
int PartyState::ReleaseRespReq(sean_Visitor *qv, Party *p)
{
  p->StopT397();
  p->StopT398();
  p->StopT399();
  return -2;
}

// default behavior on page 106/163 Q.2971
// PS: P0 has to redefine
int PartyState::ReleaseCompReq(sean_Visitor *qv, Party *p)
{
  p->StopT397();
  p->StopT398();
  p->StopT399();
  return -2;
}

// default action page 106/163 for P1, P3, P4,P5,P6,P7
// P0 and P2 have to redefine
int PartyState::AddPartyInd(sean_Visitor *qv, Party *p)
{
  assert(p != 0);
  Call *c = p->GetOwner();
  assert(c != 0);
  c->SendPartyStatusToPeer(ie_cause::message_not_compatible_with_call_state,p->get_eps(), p->get_epr());
  return 0;
}

int PartyState::SetupInd(sean_Visitor *qv, Party *p){ return -1; }
int PartyState::AddPartyRejInd(sean_Visitor *qv, Party *p){ return -1; }
int PartyState::PartyAlertInd(sean_Visitor *qv, Party *p){ return -1; }

// default is on page 106/163
// PS: only P0 and P2 have to redefine it
int PartyState::UnexpectedMSG(sean_Visitor *qv, Party *p)
{
  assert(p != 0);
  Call *c = p->GetOwner();
  assert(c != 0);
  ie_cause::CauseValues cv;
  switch (c->VerifyMessage(cv,qv))
    {
    case RAP: 
    case OK:
      c->SendPartyStatusToPeer(cv, p->get_eps(), p->get_epr());
      break;

    case I:
      break;
    case CLR:
      // label D
      break;

    case CLP:
      // do label E
      c->SendDPIndToCCD(p->get_epr(),cv);
      p->StopT399();
      p->SetT398();
      c->SendDPToPeer(p->get_epr(),cv);
      break;
    }
  return 0;
}

// timers
int PartyState::party_t397_timeout(Party *p)
{
  cout << "T397 can't expire in this state" << endl;
  return -1;
}

int PartyState::party_t398_timeout(Party *p)
{
  cout << "T398 can't expire in this state" << endl;
  return -1;
}

int PartyState::party_t399_timeout(Party *p)
{
  cout << "T399 can't expire in this state" << endl;
  return -1;
}

void PartyState::ChangeState(Party *p, PartyState *s)
{
  p->ChangeState(s);
}

// SPECIALIZED (state-dependent)


P0_Null::P0_Null()
{
}

P0_Null::~P0_Null()
{
}

PartyState *P0_Null::Instance()
{
  if (!_instance)
    _instance = new P0_Null;
  return(_instance);
}

void P0_Null::StateName(char *name) { strcpy(name,"P0_Null");}

ie_end_pt_state::party_state P0_Null::StateID() {return ie_end_pt_state::p_null;}


// can't locate in Q.2971 what the redefine should be !!!
int P0_Null::UnexpectedMSG(sean_Visitor *qv, Party *p)
{
  return 0;
}

int P0_Null::AddPartyReq(sean_Visitor *qv, Party *p)
{
  p->SetT399();
  ChangeState(p, P1_AddPartyInit::Instance());
  return 0;
}

int P0_Null::SetupReq(sean_Visitor *qv, Party *p)
{
  ChangeState(p, P1_AddPartyInit::Instance());
  return 0;
}


int P0_Null::SetupInd(sean_Visitor *qv, Party *p)
{
  ChangeState(p, P2_AddPartyRcv::Instance());
  return 0;
}

int P0_Null::AddPartyInd(sean_Visitor *qv, Party *p)
{
  Call *c = p->GetOwner();
  assert(c != 0);
  ie_cause::CauseValues cv;
  switch (c->VerifyMessage(cv,qv))
    {
    case RAP: 
      c->SendPartyStatusToPeer(cv, ie_end_pt_state::add_party_received, p->get_epr());
    case OK:
      ChangeState(p, P2_AddPartyRcv::Instance());
      break;
    case CLR:
      return 0; // label D
      break;
    case CLP:
      c->SendAPRejToPeer(p->get_epr(),cv);
      return -2;
      break;

    case RAI:
      c->SendPartyStatusToPeer(cv, ie_end_pt_state::p_null, p->get_epr());
      // fall thru
    case I:
      break;
    }
  return 0;
}

// can't locate in specs -- must be unexpected
int P0_Null::PartyStatusEnqReq(sean_Visitor *qv, Party *p)
{
  return -1;
}

// page 165 Q.2971

int P0_Null::StatusEnqInd(sean_Visitor *qv, Party *p)
{
  p->GetOwner()->SendPartyStatusToPeer(ie_cause::response_to_status_enquiry,ie_end_pt_state::p_null,p->get_epr());
  qv->Suicide();
  return -2;
}

int P0_Null::StatusInd(sean_Visitor *qv, Party *p)
{
  generic_q93b_msg *msg = qv->share_message();
  InfoElem **ies = msg->get_ie_array();
  ie_end_pt_state *es = (ie_end_pt_state *)ies[InfoElem::ie_end_pt_state_ix];
  assert(es != 0);
  if(es->get_epr_state() != ie_end_pt_state::p_null)
    p->GetOwner()->SendDPAKToPeer(p->get_epr(),ie_cause::message_not_compatible_with_call_state);
  qv->Suicide();
  return -2;
}

// redefined to avoid doing what's on page 161 Q.2971

int P0_Null::DropPartyInd(sean_Visitor *qv, Party *p)
{
  return -1;
}

// redefined to avoid doing what's on page 160 Q.2971

int P0_Null::DropPartyAckInd(sean_Visitor *qv, Party *p)
{
  return -1;
}

// can't locate it in specs
int P0_Null::ReleaseCompInd(sean_Visitor *qv, Party *p)
{
  return -1; 
}

// can't locate it in specs
int P0_Null::ReleaseInd(sean_Visitor *qv, Party *p)
{
  return -1; 
}

int P0_Null::ReleaseRespReq(sean_Visitor *qv, Party *p)
{
  return -1; 
}


// P1_AddPartyInit

P1_AddPartyInit::P1_AddPartyInit()
{
}

P1_AddPartyInit::~P1_AddPartyInit()
{
}

PartyState *P1_AddPartyInit::Instance()
{
  if (!_instance)
    _instance = new P1_AddPartyInit;
  return(_instance);
}

void P1_AddPartyInit::StateName(char *name) { strcpy(name,"P1_AddPartyInitiated");}

ie_end_pt_state::party_state P1_AddPartyInit::StateID() {return ie_end_pt_state::add_party_initiated;}

int P1_AddPartyInit::ConnectInd(sean_Visitor *qv, Party *p)
{
  p->StopT399();
  ChangeState(p,  P7_PartyActive::Instance());
  return 0;
}

int P1_AddPartyInit::AddPartyAckInd(sean_Visitor *qv, Party *p)
{
  Call *c = p->GetOwner();
  assert(c != 0);
  ie_cause::CauseValues cv;
  switch (c->VerifyMessage(cv,qv))
    {
    case RAP: 
      c->SendPartyStatusToPeer(cv, ie_end_pt_state::p_active, p->get_epr());
    case OK:
      p->StopT399();
      ChangeState(p,  P7_PartyActive::Instance());
      // the AddPartyAckInd continues its flow up
      break;
    case CLR:
      return 0; // label D
      break;

    case CLP:
      // do label E
      c->SendDPIndToCCD(p->get_epr(),cv);
      p->StopT399();
      p->SetT398();
      c->SendDPToPeer(p->get_epr(),cv);
      break;

    case RAI:
      c->SendPartyStatusToPeer(cv, ie_end_pt_state::add_party_initiated, p->get_epr());
      // fall thru
    case I:
      break;
    }
  return 0;
}

int P1_AddPartyInit::AddPartyRejInd(sean_Visitor *qv, Party *p)
{
  p->StopT399();
  // the AddPartyRejInd flows up to CCD
  return -2;
}

int P1_AddPartyInit::party_t399_timeout(Party *p)
{
  // do label E page 160 Q.2971
  Call *c = p->GetOwner();
  assert(c != 0);
 if(c->StackType() == ie_cause::user)
   c->SendDPIndToCCD(p->get_epr(),ie_cause::recovery_on_timer_expiry); // cause 102
 else
   c->SendDPIndToCCD(p->get_epr(),ie_cause::no_user_responding); // cause 18
 p->StopT397();
 p->SetT398();
 ChangeState(p, P5_DropPartyInit::Instance());
 return 0;
}


int P1_AddPartyInit::SetupCompReq(sean_Visitor *qv, Party *p)
{
  Call *c = p->GetOwner();
  assert(c != 0);
  if(c->StackType() == ie_cause::local_private_network)
   {
     p->SetT399();
     ChangeState(p, P7_PartyActive::Instance());
     return 0;
   }
  return -1;
}

int P1_AddPartyInit::PartyAlertInd(sean_Visitor *qv, Party *p)
{
  Call *c = p->GetOwner();
  assert(c != 0);
  ie_cause::CauseValues cv;
  switch (c->VerifyMessage(cv,qv))
    {
    case RAP: 
      c->SendPartyStatusToPeer(cv, ie_end_pt_state::party_alert_received, p->get_epr());
    case OK:
      p->StopT399();
      p->SetT397(); // optional on the user side
      ChangeState(p,  P4_PartyAlertRcv::Instance());
      // the PartyAlertInd continues its flow up
      break;

    case CLR:
      return 0; // label D
      break;

    case CLP:
      // do label E
      c->SendDPIndToCCD(p->get_epr(),cv);
      p->StopT399();
      p->SetT398();
      c->SendDPToPeer(p->get_epr(),cv);
      break;

    case RAI:
      c->SendPartyStatusToPeer(cv, ie_end_pt_state::add_party_initiated, p->get_epr());
      // fall thru
    case I:
      break;
    }
  return 0;
}

// pages 100 & 157 -- Q.2971
int P1_AddPartyInit::AlertInd(sean_Visitor *qv, Party *p)
{
  p->SetT397(); // this is optional on the user side
  ChangeState(p,P4_PartyAlertRcv::Instance());
  return 0;
}


// see page 103 Q.2971
int P1_AddPartyInit::DropPartyReq(sean_Visitor *qv, Party *p)
{
  p->StopT397();
  p->StopT399(); 
  p->SetT398();
  // the DropParty flows down in CC
  ChangeState(p, P5_DropPartyInit::Instance());
  return 0;
}


// P2_AddPartyRcv

P2_AddPartyRcv::P2_AddPartyRcv()
{
}

P2_AddPartyRcv::~P2_AddPartyRcv()
{
}

PartyState *P2_AddPartyRcv::Instance()
{
  if (!_instance)
    _instance = new P2_AddPartyRcv;
  return(_instance);
}

void P2_AddPartyRcv::StateName(char *name) { strcpy(name,"P2_AddPartyReceived");}

ie_end_pt_state::party_state P2_AddPartyRcv::StateID() { return ie_end_pt_state::add_party_received;}

int P2_AddPartyRcv::UnexpectedMSG(sean_Visitor *qv, Party *p)
{
  assert(p != 0);
  Call *c = p->GetOwner();
  assert(c != 0);
  ie_cause::CauseValues cv;
  switch (c->VerifyMessage(cv,qv))
    {
    case RAP: 
    case OK:
    case RAI:
      c->SendPartyStatusToPeer(cv, ie_end_pt_state::add_party_received, p->get_epr());
      break;

    case I:
      break;
    case CLR:
      // label D
      break;

    case CLP:
      // send an AP-Rej to peer
      c->SendAPRejToPeer(p->get_epr(),cv);
      return -2;
      break;
    }
  return 0;
}

// same as SetupResp -- page 101 Q.2971 (Network side only)
int P2_AddPartyRcv::ConnectReq(sean_Visitor *qv, Party *p)
{
  assert(p != 0);
  if(p->GetOwner()->StackType() == ie_cause::local_private_network)
    ChangeState(p,P7_PartyActive::Instance());
  return 0;
}

int P2_AddPartyRcv::AddPartyAckReq(sean_Visitor *qv, Party *p)
{
  assert(p != 0);
  ChangeState(p,P7_PartyActive::Instance());
  return 0;
}

int P2_AddPartyRcv::AddPartyRejReq(sean_Visitor *qv, Party *p)
{
  p->StopT397();
  p->StopT398();
  p->StopT399();
  return -2;
}


int P2_AddPartyRcv::AddPartyInd(sean_Visitor *qv, Party *p)
{
  assert(p != 0);
  Call *c = p->GetOwner();
  assert(c != 0);
  ie_cause::CauseValues cv;
  switch (c->VerifyMessage(cv,qv))
    {
    case RAP: 
    case RAI:
      c->SendPartyStatusToPeer(cv, ie_end_pt_state::party_alert_received, p->get_epr());
    case OK:
    case I:
      break;

    case CLR:
      return 0; // label D
      break;

    case CLP:
      // stop all party timers
      p->StopT397();
      p->StopT398();
      p->StopT399();
      // send an AP-Rej to peer
      c->SendAPRejToPeer(p->get_epr(),cv);
      return -2;
      break;
    }
  return 0;
}


int P2_AddPartyRcv::PartyAlertReq(sean_Visitor *qv, Party *p)
{
  ChangeState(p,P3_PartyAlertDel::Instance());
  return 0;
}

int P2_AddPartyRcv::AlertReq(sean_Visitor *qv, Party *p)
{
  ChangeState(p,P3_PartyAlertDel::Instance());
  return 0;
}

int P2_AddPartyRcv::ConnectAckInd(sean_Visitor *qv, Party *p)
{
  ChangeState(p,  P7_PartyActive::Instance());
  return 0;
}

// P3_PartyAlertDel

P3_PartyAlertDel::P3_PartyAlertDel()
{
}

P3_PartyAlertDel::~P3_PartyAlertDel()
{
}

PartyState *P3_PartyAlertDel::Instance()
{
  if (!_instance)
    _instance = new P3_PartyAlertDel;
  return(_instance);
}

void P3_PartyAlertDel::StateName(char *name) { strcpy(name,"P3_PartyAlertDelivered");}
ie_end_pt_state::party_state P3_PartyAlertDel::StateID() { return ie_end_pt_state::party_alert_delivered;}


// same as SetupResp -- page 102 Q.2971 (network side only)
int P3_PartyAlertDel::ConnectReq(sean_Visitor *qv, Party *p)
{
  assert(p != 0);
  if(p->GetOwner()->StackType() == ie_cause::local_private_network)
    ChangeState(p,P7_PartyActive::Instance());
  return 0;
}

int P3_PartyAlertDel::AddPartyAckReq(sean_Visitor *qv, Party *p)
{
  assert(p != 0);
  ChangeState(p,P7_PartyActive::Instance());
  return 0;
}


// see page 103 Q.2971
int P3_PartyAlertDel::DropPartyReq(sean_Visitor *qv, Party *p)
{
  p->StopT397();
  p->StopT399(); 
  p->SetT398();
  // the DropParty flows down in CC
  ChangeState(p, P5_DropPartyInit::Instance());
  return 0;
}

int P3_PartyAlertDel::ConnectAckInd(sean_Visitor *qv, Party *p)
{
  ChangeState(p,  P7_PartyActive::Instance());
  return 0;
}

// P4_PartyAlertRcv

P4_PartyAlertRcv::P4_PartyAlertRcv()
{
}

P4_PartyAlertRcv::~P4_PartyAlertRcv()
{
}

PartyState *P4_PartyAlertRcv::Instance()
{
  if (!_instance)
    _instance = new P4_PartyAlertRcv;
  return(_instance);
}

void P4_PartyAlertRcv::StateName(char *name) { strcpy(name,"P4_PartyAlertReceived");}
ie_end_pt_state::party_state P4_PartyAlertRcv::StateID() { return ie_end_pt_state::party_alert_received;}


int P4_PartyAlertRcv::ConnectInd(sean_Visitor *qv, Party *p)
{
  p->StopT397();
  ChangeState(p,  P7_PartyActive::Instance());
  return 0;
}

int P4_PartyAlertRcv::SetupCompReq(sean_Visitor *qv, Party *p)
{
  return 0;
}

int P4_PartyAlertRcv::party_t397_timeout(Party *p)
{
  // do label E page 160 Q.2971
  Call *c = p->GetOwner();
  assert(c != 0);
 if(c->StackType() == ie_cause::user)
   {
     p->StopT397();
     c->SendDPIndToCCD(p->get_epr(),ie_cause::recovery_on_timer_expiry); // cause 102
   }
 else
   {
     c->SendDPIndToCCD(p->get_epr(),ie_cause::no_user_responding); // cause 18
   }
 p->SetT398();
 ChangeState(p, P5_DropPartyInit::Instance());
 c->SendDPToPeer(p->get_epr(),ie_cause::recovery_on_timer_expiry); 
 return 0;
}

int P4_PartyAlertRcv::AddPartyAckInd(sean_Visitor *qv, Party *p)
{
  Call *c = p->GetOwner();
  assert(c != 0);
  ie_cause::CauseValues cv;
  switch (c->VerifyMessage(cv,qv))
    {
    case RAP: 
      c->SendPartyStatusToPeer(cv, ie_end_pt_state::p_active, p->get_epr());
    case OK:
      p->SetT397(); 
      ChangeState(p,  P7_PartyActive::Instance());
      break;

    case CLR:
      return 0; // label D
      break;

    case CLP:
      // do label E
      c->SendDPIndToCCD(p->get_epr(),cv);
      p->StopT399();
      p->SetT398();
      c->SendDPToPeer(p->get_epr(),cv);
      break;

    case RAI:
      c->SendPartyStatusToPeer(cv, ie_end_pt_state::party_alert_received, p->get_epr());
      // fall thru
    case I:
      break;
    }
  return 0;
}

// see page 103 & 160 Q.2971
int P4_PartyAlertRcv::DropPartyReq(sean_Visitor *qv, Party *p)
{
  p->StopT397(); 
  p->StopT399(); 
  p->SetT398();
  // the DropParty flows down in CC
  ChangeState(p, P5_DropPartyInit::Instance());
  return 0;
}


// P5_DropPartyInit

P5_DropPartyInit::P5_DropPartyInit()
{
}

P5_DropPartyInit::~P5_DropPartyInit()
{
}

 PartyState *P5_DropPartyInit::Instance()
{
  if (!_instance)
    _instance = new P5_DropPartyInit;
  return(_instance);
}

void P5_DropPartyInit::StateName(char *name) { strcpy(name,"P5_DropPartyInit");}
ie_end_pt_state::party_state P5_DropPartyInit::StateID() { return ie_end_pt_state::drop_party_initiated;}

// see page 105/162
int P5_DropPartyInit::DropPartyInd(sean_Visitor *qv, Party *p)
{
  assert(p != 0);
  Call *c = p->GetOwner();
  assert(c != 0);
  ie_cause::CauseValues cv;
  switch (c->VerifyMessage(cv,qv))
    {
    case CLR:
      return 0; // label D
      break;
    case CLP:
    case RAP: 
    case OK:
    case RAI:
    case I:
      p->StopT398();
      break;
    }
  return -2;
}

int P5_DropPartyInit::AddPartyRejInd(sean_Visitor *qv, Party *p)
{
  assert(p != 0);
  Call *c = p->GetOwner();
  assert(c != 0);
  ie_cause::CauseValues cv;
  switch (c->VerifyMessage(cv,qv))
    {
    case CLR:
      return 0; // label D
      break;
    case CLP:
    case RAP: 
    case OK:
    case RAI:
    case I:
      p->StopT398();
      break;
    }
  return -2;
}

int P5_DropPartyInit::party_t398_timeout(Party *p)
{
  p->GetOwner()->SendDPAKToPeer(p->get_epr(),ie_cause::recovery_on_timer_expiry);
  p->GetOwner()->SendDPAKToCCD(p->get_epr(),ie_cause::recovery_on_timer_expiry);
  return -2;
}

// page 162 Q.2971 redefined 
int P5_DropPartyInit::StatusInd(sean_Visitor *qv, Party *p)
{
  assert(p != 0 && qv != 0);
  assert(p != 0);
  Call *c = p->GetOwner();
  assert(c != 0);
  generic_q93b_msg *msg = qv->share_message();
  assert(msg != 0);
  InfoElem **ies = msg->get_ie_array();
  ie_cause::CauseValues cv = ((ie_cause *)msg->ie(InfoElem::ie_cause_ix))->get_cause_value();
  // get end point state of the remote node from msg and check compatibility
  ie_end_pt_state::party_state epr_state = ((ie_end_pt_state *)msg->ie(InfoElem::ie_end_pt_state_ix))->get_epr_state();
  if(epr_state == ie_end_pt_state::p_null)
    {
      p->StopT398();
      c->SendDPAKToCCD(p->get_epr(),cv);
      return -2;
    }
  return 0;
}

int P5_DropPartyInit::ReleaseReq(sean_Visitor * v, Party * p)
{
  return -2;
}

int P5_DropPartyInit::ReleaseInd(sean_Visitor *qv, Party *p)
{
  p->StopT398();
  return -2;
}

// P6_DropPartyRcv

P6_DropPartyRcv::P6_DropPartyRcv()
{
}

P6_DropPartyRcv::~P6_DropPartyRcv()
{
}

 PartyState *P6_DropPartyRcv::Instance()
{
  if (!_instance)
    _instance = new P6_DropPartyRcv;
  return(_instance);
}

void P6_DropPartyRcv::StateName(char *name) { strcpy(name,"P6_DropPartyRcv");}
ie_end_pt_state::party_state P6_DropPartyRcv::StateID() { return ie_end_pt_state::drop_party_received;}


// page 161 Q.2971
int P6_DropPartyRcv::DropPartyAckReq(sean_Visitor *qv, Party *p)
{
  p->StopT397();
  p->StopT398();
  p->StopT399();
  return -2;
}



// P7_PartyActive

P7_PartyActive::P7_PartyActive()
{
}

P7_PartyActive::~P7_PartyActive()
{
}

PartyState *P7_PartyActive::Instance()
{
  if (!_instance)
    _instance = new P7_PartyActive;
  return(_instance);
}

void P7_PartyActive::StateName(char *name) { strcpy(name,"P7_PartyActive");}

ie_end_pt_state::party_state P7_PartyActive::StateID() { return ie_end_pt_state::p_active;}

// page 103/160 Q.2971
int P7_PartyActive::DropPartyReq(sean_Visitor *qv, Party *p)
{
  p->StopT397();
  p->StopT399(); 
  p->SetT398();
  // the DropParty flows down in CC
  ChangeState(p, P5_DropPartyInit::Instance());
  return 0;
}






