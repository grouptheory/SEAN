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

//sscop_recresp
#ifndef LINT
static char const _recresp_cc_rcsid_[] =
"$Id: recresp.cc,v 1.2 1998/08/06 04:04:17 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "sscop.h"
#include "sscopState.h"
#include "sscop_visitors.h"
#include "sscop_timers.h"

sscopState *sscop_recresp::_instance = 0;

sscop_recresp::sscop_recresp(){}
sscop_recresp::~sscop_recresp(){}

sscopState* sscop_recresp::Instance()
{
  if (!_instance)
    _instance = new sscop_recresp;
  return(_instance);
}

inline void sscop_recresp::Name(char *name) {strcpy(name,"recresp");}

inline SSCOPconn::sscop_states  sscop_recresp::StateID()
{
  return SSCOPconn::sscop_recresp;
}

// AA-RetrieveReq uses the default one

int sscop_recresp::RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'C',v);
  return 1;
}

inline int sscop_recresp::RecvERAK(SSCOPconn *c, SSCOPVisitor *v)
{
  return 1;
}

int sscop_recresp::RecvEND(SSCOPconn *c, SSCOPVisitor *v)
{
  SSCOPVisitor *sv = new EndAckVisitor(SSCOPVisitor::EndAckReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v);
  return 1;
}

int sscop_recresp::RecvENDAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'F',v);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v);
  return 1;
}
int sscop_recresp::RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'D',v);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v);
  return 1;
}

int sscop_recresp::AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v)
{
  c->_vt_cc = 1;
  c->Send2Peer(v);
  c->SetTimerCC();
  ChangeState(c,sscop_outdisc::Instance(),"sscop_outdisc");
  return 1;
}

int sscop_recresp::RecvRSAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'K',v);
  return 1;
}

int sscop_recresp::RecvRS(SSCOPconn *c, SSCOPVisitor *v)
{
  if (c->DetectRetransmission(v->GetSQ()))
    {
      char name[40];
      Name(name);
      c->MAAError(name,'J',v);
      return 1;
    }
  c->_vt_ms = v->GetMR();
  ChangeState(c,sscop_inres::Instance(),"sscop_inres");
  c->Send2SSCF(v); // AA-ResyncInd
  return 1;
}

inline int sscop_recresp::RecvSD(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}
inline int sscop_recresp::RecvPOLL(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}


int sscop_recresp::AA_RecoverResp(SSCOPconn *c, SSCOPVisitor *v)
{
  if (!c->_cb)
    c->_TXBuffer.clear();
  c->InitStateVariables();
  c->SetDataTransferTimers();
  ChangeState(c,sscop_ready::Instance(),"sscop_ready");
  v->Suicide();
  c->SendQueuedSD();
  return 1;
}

int sscop_recresp::AA_ResyncReq(SSCOPconn *c, SSCOPVisitor *v)
{
  c->_vt_cc = 1;
  c->IncVTSQ();
  c->InitVRMR();
  c->Send2Peer(v);
  c->ClearTransmitter();
  c->SetTimerCC();
  ChangeState(c,sscop_outres::Instance(),"sscop_outres");
  return 1;
}

int sscop_recresp::RecvER(SSCOPconn *c, SSCOPVisitor *v)
{
  if (!c->DetectRetransmission(v->GetSQ()))
    {
      char name[40];
      Name(name);
      c->MAAError(name,'R',v);
    }
  else
    {
      v->Suicide();
      SSCOPVisitor *sv = new RecovAckVisitor(SSCOPVisitor::RecovAckReq,0,0);
      c->Send2Peer(sv);
    }
  return 1;
}


int sscop_recresp::RecvBGN(SSCOPconn *c, SSCOPVisitor *v)
{
  if (c->DetectRetransmission(v->GetSQ()))
    {
      char name[40];
      Name(name);
      c->MAAError(name,'B',v);
      return 1;
    }
  c->_vt_ms = v->GetMR();
  ChangeState(c,sscop_inconn::Instance(),"sscop_inconn");
  SSCOPVisitor *rv = new EndAckVisitor(SSCOPVisitor::EndAckInd,0,0);
  c->Send2SSCF(rv); // AA-ReleaseInd
  c->Send2SSCF(v); // AA-EstablishInd
  return 1;
}

int sscop_recresp::RecvSTAT(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'H',v);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  SSCOPVisitor *rv = new EndAckVisitor(SSCOPVisitor::EndAckInd,0,0);
  c->Send2SSCF(rv); // AA-ReleaseInd
  return 1;
}

int sscop_recresp::RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'I',v);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  SSCOPVisitor *rv = new EndAckVisitor(SSCOPVisitor::EndAckInd,0,0);
  c->Send2SSCF(rv); // AA-ReleaseInd
  return 1;
}







