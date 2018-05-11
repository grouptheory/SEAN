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

//sscop_inres
#ifndef LINT
static char const _inres_cc_rcsid_[] =
"$Id: inres.cc,v 1.2 1998/08/06 04:04:16 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "sscop.h"
#include "sscopState.h"
#include "sscop_visitors.h"
#include "sscop_timers.h"

sscopState *sscop_inres::_instance = 0;

sscop_inres::sscop_inres() {}
sscop_inres::~sscop_inres() {}

sscopState* sscop_inres::Instance()
{
  if (!_instance)
    _instance = new sscop_inres;
  return(_instance);
}


inline void sscop_inres::Name(char *name) {strcpy(name,"inres");}

inline SSCOPconn::sscop_states  sscop_inres::StateID()
{
  return SSCOPconn::sscop_inres;
}

int sscop_inres::AA_ResyncResp(SSCOPconn *c, SSCOPVisitor *v)
{
  c->InitVRMR();
  c->Send2Peer(v);
  c->ClearTransmitter();
  c->InitStateVariables();
  c->SetDataTransferTimers();
  ChangeState(c,sscop_ready::Instance(),"sscop_ready");
  c->SendQueuedSD();
  return 0;
}


int sscop_inres::AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v)
{
  c->_vt_cc = 1;
  c->Send2Peer(v);
  c->SetTimerCC();
  ChangeState(c,sscop_outdisc::Instance(),"sscop_outdisc");
  return 1;
}

int sscop_inres::RecvENDAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'F',v);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v);
  return 1;
}

int sscop_inres::RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'D',v);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v);
  return 1;
}

int sscop_inres::RecvEND(SSCOPconn *c, SSCOPVisitor *v)
{
  SSCOPVisitor *sv = new EndAckVisitor(SSCOPVisitor::EndAckReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v);
  return 1;
}


int sscop_inres::RecvER(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'L',v);
  return 1;
}

int sscop_inres::RecvBGN(SSCOPconn *c, SSCOPVisitor *v)
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
   // send a AA-ReleaseConf and AA-EstablishInd to SSCF, perhaps combined
  SSCOPVisitor *rv = new EndAckVisitor(SSCOPVisitor::EndAckInd,0,0);
  c->Send2SSCF(rv); // AA-ReleaseInd
  c->Send2SSCF(v); // AA-EstablishInd
  return 1;
}


int sscop_inres::RecvSD(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->MAAError(name,'A',v);
  SSCOPVisitor *rv = new EndAckVisitor(SSCOPVisitor::EndAckInd,0,0);
  c->Send2SSCF(rv); // AA-ReleaseInd
  return 1;
}

int sscop_inres::RecvPOLL(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->MAAError(name,'G',v);
  SSCOPVisitor *rv = new EndAckVisitor(SSCOPVisitor::EndAckInd,0,0);
  c->Send2SSCF(rv); // AA-ReleaseInd
  return 1;
}

int sscop_inres::RecvSTAT(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->MAAError(name,'H',v);
  SSCOPVisitor *rv = new EndAckVisitor(SSCOPVisitor::EndAckInd,0,0);
  c->Send2SSCF(rv); // AA-ReleaseInd
  return 1;
}

int sscop_inres::RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->MAAError(name,'I',v);
  SSCOPVisitor *rv = new EndAckVisitor(SSCOPVisitor::EndAckInd,0,0);
  c->Send2SSCF(rv); // AA-ReleaseInd
  return 1;
}

int sscop_inres::RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'C',v);
  return 1;
}


int sscop_inres::RecvERAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'M',v);
  return 1;
}


int sscop_inres::RecvRS(SSCOPconn *c, SSCOPVisitor *v)
{
  if (!c->DetectRetransmission(v->GetSQ()))
    {
      char name[40];
      Name(name);
      c->MAAError(name,'J',v);
    }
  return 1;
}


int sscop_inres::RecvRSAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'K',v);
  return 1;
}


// AA-RetrieveReq use default in def.cc

