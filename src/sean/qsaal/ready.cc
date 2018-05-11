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

//sscop_ready
#ifndef LINT
static char const _ready_cc_rcsid_[] =
"$Id: ready.cc,v 1.3 1998/08/13 04:31:32 battou Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/basics/diag.h>
#include <FW/kernel/Kernel.h>
#include "sscop.h"
#include "sscopState.h"
#include "sscop_visitors.h"
#include "sscop_timers.h"

sscopState *sscop_ready::_instance = 0;

sscop_ready::sscop_ready(){}
sscop_ready::~sscop_ready(){}

sscopState* sscop_ready::Instance()
{
  if (!_instance)
    _instance = new sscop_ready;
  return(_instance);
}

inline void sscop_ready::Name(char *name) {strcpy(name,"ready");}

inline SSCOPconn::sscop_states  sscop_ready::StateID()
{
  return SSCOPconn::sscop_ready;
}


int sscop_ready::AA_ResyncReq(SSCOPconn *c, SSCOPVisitor *v)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ": AA_ResyncReq() " << endl);
  c->ResetDataTransferTimers();
  c->_vt_cc = 1;
  c->IncVTSQ();
  c->InitVRMR();
  c->Send2Peer(v);
  c->ReleaseBuffers();
  c->SetTimerCC();
  ChangeState(c,sscop_outres::Instance(),"sscop_outres");
  return 1;
}

int sscop_ready::AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v)
{
  c->ResetDataTransferTimers();
  c->_vt_cc = 1;
  c->Send2Peer(v);
  c->PrepareRetrieval();
  c->SetTimerCC();
  ChangeState(c,sscop_outdisc::Instance(),"sscop_outdisc");
  return 1;
}

inline int sscop_ready::RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}

int sscop_ready::RecvERAK(SSCOPconn *c, SSCOPVisitor *v)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ": Received an ERAK in ready state we ignore it" << endl);
  v->Suicide();
  return 1;
}

int sscop_ready::RecvER(SSCOPconn *c, SSCOPVisitor *v)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ": Received an ER in ready state -- send an ERAK " << endl);
  if (c->DetectRetransmission(v->GetSQ()))
    {
      v->Suicide();
      c->SetTimerNORESP();
      SSCOPVisitor *sv = new RecovAckVisitor(SSCOPVisitor::RecovAckReq,0,0);
      c->Send2Peer(sv);
      return 1;
    }
  c->ResetDataTransferTimers();
  c->_vt_ms = v->GetMR();
  c->PrepareRecovery();
  c->DeliverData();
  ChangeState(c,sscop_inrec::Instance(),"sscop_inrec");
  c->Send2SSCF(v); // AA-RecoverInd
  return 1;
}

int sscop_ready::RecvBGN(SSCOPconn *c, SSCOPVisitor *v)
{
  if (c->DetectRetransmission(v->GetSQ()))
    {
      v->Suicide();
      c->SetTimerNORESP();
      SSCOPVisitor *sv = new BeginAckVisitor(SSCOPVisitor::BeginAckReq,0,0);
      c->Send2Peer(sv);
      return 1;
    }
  c->ResetDataTransferTimers();
  c->_vt_ms = v->GetMR();
  c->PrepareRetrieval();
  ChangeState(c,sscop_inconn::Instance(),"sscop_inconn");
  SSCOPVisitor *rv = new EndAckVisitor(SSCOPVisitor::EndAckInd,0,0);
  c->Send2SSCF(rv); // AA-ReleaseInd
  c->Send2SSCF(v); // AA-EstablishInd
  return 1;
}

int sscop_ready::RecvENDAK(SSCOPconn *c, SSCOPVisitor *v)
{
  c->ResetDataTransferTimers();
  char name[40];
  Name(name);
  c->MAAError(name,'F',v);
  c->PrepareRetrieval();
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v); // AA-ReleaseInd
  return 1;
}

int sscop_ready::RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v)
{
  c->ResetDataTransferTimers();
  char name[40];
  Name(name);
  c->MAAError(name,'D',v);
  c->PrepareRetrieval();
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  return 1;
}

inline int sscop_ready::RecvRSAK(SSCOPconn *c, SSCOPVisitor *v){return 1;}

int sscop_ready::RecvRS(SSCOPconn *c, SSCOPVisitor *v)
{
  if (c->DetectRetransmission(v->GetSQ()))
    {
      v->Suicide();
      c->SetTimerNORESP();
      SSCOPVisitor *sv = new ResyncAckVisitor(SSCOPVisitor::ResyncAckReq,0,0);
      c->Send2Peer(sv);
      return 1;
    }
  c->ResetDataTransferTimers();
  c->_vt_ms = v->GetMR();
  c->PrepareRetrieval();
  ChangeState(c,sscop_inres::Instance(),"sscop_inres");
  c->Send2SSCF(v); // AA-ResyncInd
  return 1;
}

int sscop_ready::RecvEND(SSCOPconn *c, SSCOPVisitor *v)
{
  c->ResetDataTransferTimers();
  SSCOPVisitor *sv = new EndAckVisitor(SSCOPVisitor::EndAckReq,0,0);
  c->Send2Peer(sv);
  c->PrepareRetrieval();
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v); // AA-ReleaseInd
  return 1;
}

void sscop_ready::ExpTimerPOLL(SSCOPconn *c)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ": ExpTimerPOLL() " << endl);
  c->IncVTPS();
  SSCOPVisitor *sv = new PollVisitor(SSCOPVisitor::PollReq,0,0);
  c->Send2Peer(sv);
  c->_vt_pd = 0;
  c->SetPollTimer();
}

void sscop_ready::ExpTimerKEEPALIVE(SSCOPconn *c)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ": ExpTimerKEEPALIVE() " << endl);
  c->IncVTPS();
  SSCOPVisitor *sv = new PollVisitor(SSCOPVisitor::PollReq,0,0);
  c->Send2Peer(sv);
  c->_vt_pd = 0;
  c->SetPollTimer();
}

void sscop_ready::ExpTimerIDLE(SSCOPconn *c)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ": ExpTimerIDLE() " << endl);
  c->SetTimerNORESP();
  c->IncVTPS();
  SSCOPVisitor *sv = new PollVisitor(SSCOPVisitor::PollReq,0,0);
  c->Send2Peer(sv);
  c->_vt_pd = 0;
  c->SetPollTimer();
}


void sscop_ready::ExpTimerNORESP(SSCOPconn *c)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ": ExpTimerNORESP() " << endl);
  c->ResetDataTransferTimers();
  char name[40];
  Name(name);
  c->MAAError(name,'P',0);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  c->PrepareRetrieval();
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  // inform SSCF with an AA-ReleaseInd
  SSCOPVisitor *rv = new EndAckVisitor(SSCOPVisitor::EndAckInd,0,0);
  c->Send2SSCF(rv); 
}

int sscop_ready::AA_DATAReq(SSCOPconn *c, SSCOPVisitor *v)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ": AA_DATAReq() " << endl);
  // at this point the seqnum is ZERO
  v->SetPDUType(SSCOPconn::SSCOP_SD);
  c->_TXQueue.append(v);
  c->SendQueuedSD();
  return 1;
}



int sscop_ready::RecvSD(SSCOPconn *c, SSCOPVisitor *v)
{
  c->DataSD(v);
  return 1;
}

int sscop_ready::RecvPOLL(SSCOPconn *c, SSCOPVisitor *v)
{
  c->DataPOLL(v);
  return 1;
}

int sscop_ready::RecvSTAT(SSCOPconn *c, SSCOPVisitor *v)
{
  c->DataSTAT(v);
  return 1;
}

int sscop_ready::RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v)
{
  c->DataUSTAT(v);
  return 1;
}


int sscop_ready::AA_RetrieveReq(SSCOPconn *c, SSCOPVisitor *v)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ": AA_RetrieveReq() " << endl);
  v->Suicide();
  return -1;
}




