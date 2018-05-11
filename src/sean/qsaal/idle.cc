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

//sscop_idle state

#ifndef LINT
static char const _idle_cc_rcsid_[] =
"$Id: idle.cc,v 1.2 1998/08/06 04:04:15 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "sscop.h"
#include "sscopState.h"
#include "sscop_visitors.h"
#include "sscop_timers.h"

sscopState *sscop_idle::_instance = 0;

sscop_idle::sscop_idle() {}
sscop_idle::~sscop_idle() {}

sscopState* sscop_idle::Instance()
{
  if (!_instance)
    _instance = new sscop_idle;
  return(_instance);
}

inline void sscop_idle::Name(char *name) {strcpy(name,"idle");}

inline SSCOPconn::sscop_states  sscop_idle::StateID()
{
  return SSCOPconn::sscop_idle;
}

// march 5,98
int sscop_idle::AA_EstablishReq(SSCOPconn *c, SSCOPVisitor *v)
{
  c->ClearTransmitter();
  c->_cb = v->GetBR();
  c->_vt_cc = 1;
  c->IncVTSQ();
  c->_vr_mr = c->_MR;
  v->SetPDUType(SSCOPconn::SSCOP_BGN);
  c->Send2Peer(v);
  c->SetTimerCC();
  ChangeState(c,sscop_outconn::Instance(),"sscop_outconn");
  return(1);
}


// from the Driver

int sscop_idle::RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'D',v);
  return 1;
}

int sscop_idle::RecvBGN(SSCOPconn *c, SSCOPVisitor *v)
{
  int sq = v->GetSQ();
  if (c->DetectRetransmission(sq))
    {
      v->Suicide();
      SSCOPVisitor *sv = new BeginRejVisitor(SSCOPVisitor::BeginRejReq,0,0);
      c->Send2Peer(sv);
      return 1;
    }
  c->_vt_ms = v->GetMR();
  ChangeState(c,sscop_inconn::Instance(),"sscop_inconn");
  c->Send2SSCF(v);
  return 1;
}

int sscop_idle::RecvENDAK(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}

int sscop_idle::RecvEND(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  SSCOPVisitor *sv = new EndAckVisitor(SSCOPVisitor::EndAckReq,0,0);
  c->Send2Peer(sv);
  return 1;
}


// got an ER PDU
int sscop_idle::RecvER(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  c->MAAError(name,'L',v);
  return 1;
}


// march 5,98
int sscop_idle::RecvPOLL(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  c->MAAError(name,'G',v);
  return 1;
}

int sscop_idle::RecvSD(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  c->MAAError(name,'A',v);
  return 1;
}

int sscop_idle::RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  c->MAAError(name,'C',v);
  return 1;
}

int sscop_idle::RecvERAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  c->MAAError(name,'M',v);
  return 1;
}

int sscop_idle::RecvSTAT(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  c->MAAError(name,'H',v);
  return 1;
}

int sscop_idle::RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  c->MAAError(name,'I',v);
  return 1;
}

int sscop_idle::RecvRS(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  c->MAAError(name,'J',v);
  return 1;
}

int sscop_idle::RecvRSAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  c->MAAError(name,'K',v);
  return 1;
}

