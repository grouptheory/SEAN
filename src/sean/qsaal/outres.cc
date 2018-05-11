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

//sscop_outres
#ifndef LINT
static char const _outres_cc_rcsid_[] =
"$Id: outres.cc,v 1.2 1998/08/06 04:04:16 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "sscop.h"
#include "sscopState.h"
#include "sscop_visitors.h"
#include "sscop_timers.h"

sscopState *sscop_outres::_instance = 0;

sscop_outres::sscop_outres(){}
sscop_outres::~sscop_outres(){}

sscopState * sscop_outres::Instance()
{
  if (!_instance)
    _instance = new sscop_outres;
  return(_instance);
}

inline void sscop_outres::Name(char *name) {strcpy(name,"outres");}

inline SSCOPconn::sscop_states  sscop_outres::StateID()
{
  return SSCOPconn::sscop_outres;
}

inline int sscop_outres::RecvER(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}

int sscop_outres::RecvBGN(SSCOPconn *c, SSCOPVisitor *v)
{
  if (c->DetectRetransmission(v->GetSQ()))
    {
      v->Suicide();
      SSCOPVisitor *sv = new BeginAckVisitor(SSCOPVisitor::BeginAckReq,0,0);
      c->Send2Peer(sv);
      c->ReSendVisitorToPeer(c->_last_rs);
      return 1;
    }
  c->StopTimerCC();
  c->_vt_ms = v->GetMR();
  ChangeState(c,sscop_inconn::Instance(),"sscop_inconn");
  c->Send2SSCF(v); // AA-EstablishInd
  SSCOPVisitor *rv = new EndVisitor(SSCOPVisitor::EndInd,0,0);
  c->Send2SSCF(rv); // AA-ReleaseInd
  return 1;
}

inline int sscop_outres::RecvPOLL(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}

int sscop_outres::RecvENDAK(SSCOPconn *c, SSCOPVisitor *v)
{
  c->StopTimerCC();
  char name[40];
  Name(name);
  c->MAAError(name,'F',0);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v); // AA-ReleaseInd
  return 1;
}

int sscop_outres::RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'D',0);
  c->StopTimerCC();
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v); // AA-ReleaseInd
  return 1;
}

int sscop_outres::RecvEND(SSCOPconn *c, SSCOPVisitor *v)
{
  c->StopTimerCC();
  SSCOPVisitor *sv = new EndAckVisitor(SSCOPVisitor::EndAckReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v); // AA-ReleaseInd
  return 1;
}


void sscop_outres::ExpTimerCC(SSCOPconn *c)
{
  if(c->_vt_cc >= c->_MaxCC)
    {
      char name[40];
      Name(name);
      c->MAAError(name,'O',0);
      SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
      c->Send2Peer(sv);
      ChangeState(c,sscop_idle::Instance(),"sscop_idle");
      SSCOPVisitor *rv = new EndVisitor(SSCOPVisitor::EndInd,0,0);
      c->Send2SSCF(rv); // AA-ReleaseInd
      return;
    }
  c->_vt_cc++;
  c->ReSendVisitorToPeer(c->_last_rs);
  c->SetTimerCC();
  return;
}

inline int sscop_outres::RecvSTAT(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}
inline int sscop_outres::RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}

int sscop_outres::AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v)
{
  c->StopTimerCC();
  c->_vt_cc = 1;
  c->Send2Peer(v);
  c->SetTimerCC();
  ChangeState(c,sscop_outdisc::Instance(),"sscop_outdisc");
  return 1;
}

int sscop_outres::RecvRS(SSCOPconn *c, SSCOPVisitor *v)
{
  if (c->DetectRetransmission(v->GetSQ()))
    return 1;
  c->StopTimerCC();
  c->_vt_ms = v->GetMR();
  c->InitVRMR();
  SSCOPVisitor *sv = new ResyncAckVisitor(SSCOPVisitor::ResyncAckReq,0,0);
  c->Send2Peer(sv);
  c->InitStateVariables();
  c->SetDataTransferTimers();
  ChangeState(c,sscop_ready::Instance(),"sscop_ready");
  c->Send2SSCF(v); // AA-ResyncConf
  return 1;
}

inline int sscop_outres::RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}

inline int sscop_outres::RecvERAK(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}
inline int sscop_outres::RecvSD(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}

int sscop_outres::RecvRSAK(SSCOPconn *c, SSCOPVisitor *v)
{
  c->StopTimerCC();
  c->_vt_ms = v->GetMR();
  c->InitStateVariables();
  c->SetDataTransferTimers();
  ChangeState(c,sscop_ready::Instance(),"sscop_ready");
  c->Send2SSCF(v); // AA-ResyncConf
  return 1;
}

int sscop_outres::AA_RetrieveReq(SSCOPconn *c, SSCOPVisitor *v)
{
  cout << "sscop_outres::AA_RetrieveReq -- illegal" << endl;
  v->Suicide();
  return -1;
}


