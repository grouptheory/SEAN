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
static char const _inconn_cc_rcsid_[] =
"$Id: inconn.cc,v 1.2 1998/08/06 04:04:15 bilal Exp battou $";
#endif
#include <common/cprototypes.h>

#include "sscop.h"
#include "sscopState.h"
#include "sscop_visitors.h"
#include "sscop_timers.h"

sscopState *sscop_inconn::_instance = 0;

sscop_inconn::sscop_inconn() {}
sscop_inconn::~sscop_inconn() {}

sscopState* sscop_inconn::Instance()
{
  if (!_instance)
    _instance = new sscop_inconn;
  return(_instance);
}

inline void sscop_inconn::Name(char *name) {strcpy(name,"inconn");}

inline SSCOPconn::sscop_states  sscop_inconn::StateID()
{
  return SSCOPconn::sscop_inconn;
}

int sscop_inconn::AA_EstablishResp(SSCOPconn *c, SSCOPVisitor *v)
{
  c->ClearTransmitter();
  c->_cb = v->GetBR();
  c->InitVRMR();
  c->Send2Peer(v);
  c->InitStateVariables();
  c->SetDataTransferTimers();
  ChangeState(c,sscop_ready::Instance(),"sscop_ready");
  cout << "SSCOP is in READY now ...." << endl;
  return(1);
}

int sscop_inconn::AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v)
{
  SSCOPVisitor *sv = new BeginRejVisitor(SSCOPVisitor::BeginRejReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  return(1);
}

int sscop_inconn::RecvBGN(SSCOPconn *c, SSCOPVisitor *v)
{
  if (c->DetectRetransmission(v->GetSQ()))
    return 1;
  c->_vt_ms = v->GetMR();
  // send a AA-ReleaseInd and AA-EstablishInd to SSCF, perhaps combined
  SSCOPVisitor *rv = new BeginRejVisitor(SSCOPVisitor::BeginRejInd,0,0);
  c->Send2SSCF(rv); // AA-ReleaseInd
  c->Send2SSCF(v); // AA-EstablishInd
  return 1;
}

int sscop_inconn::RecvER(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'L',v);
  return 1;
}

int sscop_inconn::RecvENDAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->MAAError(name,'F',0);
  c->Send2SSCF(v);
  return 1;
}

int sscop_inconn::RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v)
{

  char name[40];
  Name(name);
  c->MAAError(name,'F',v);
  return 1;
}

int sscop_inconn::RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v)
{

  char name[40];
  Name(name);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->MAAError(name,'D',0);
  c->Send2SSCF(v); // AA-ReleaseInd
  return 1;
}


int sscop_inconn::RecvSD(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'A',0);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  return 1;
}


int sscop_inconn::RecvSTAT(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'H',0);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  return 1;
}

int sscop_inconn::RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'I',0);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  return 1;
}

int sscop_inconn::RecvPOLL(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'G',0);
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  return 1;
}

int sscop_inconn::RecvERAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'M',v);
  return 1;
}

int sscop_inconn::RecvRSAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'K',v);
  return 1;
}


int sscop_inconn::RecvRS(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'J',v);
  return 1;
}

int sscop_inconn::RecvEND(SSCOPconn *c, SSCOPVisitor *v)
{
  SSCOPVisitor *sv = new EndAckVisitor(SSCOPVisitor::EndAckReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v); // AA-ReleaseInd
  return 0;
}











