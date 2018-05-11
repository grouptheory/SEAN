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
static char const _outconn_cc_rcsid_[] =
"$Id: outconn.cc,v 1.2 1998/08/06 04:04:16 bilal Exp $";
#endif
#include <common/cprototypes.h>
#include <iostream.h>

#include <FW/basics/diag.h>
#include "sscop.h"
#include "sscopState.h"
#include "sscop_visitors.h"
#include "sscop_timers.h"

sscopState *sscop_outconn::_instance = 0;


sscop_outconn::sscop_outconn() {}
sscop_outconn::~sscop_outconn() {}

sscopState* sscop_outconn::Instance()
{
  if (!_instance)
    _instance = new sscop_outconn;
  return(_instance);
}

inline void sscop_outconn::Name(char *name){ strcpy(name,"outconn");}

inline SSCOPconn::sscop_states  sscop_outconn::StateID()
{
  return SSCOPconn::sscop_outconn;
}

int sscop_outconn::AA_RetrieveReq(SSCOPconn *c, SSCOPVisitor *v)
{
  cout << "sscop_outconn::AA_RetrieveReq -- illegal" << endl;
  return -1;
}

int sscop_outconn::RecvENDAK(SSCOPconn *c, SSCOPVisitor *v){ return 1; }

int sscop_outconn::RecvSD(SSCOPconn *c, SSCOPVisitor *v){ return 1; }

int sscop_outconn::RecvERAK(SSCOPconn *c, SSCOPVisitor *v){ return 1; }


int sscop_outconn::RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v)
{
  c->StopTimerCC();
  c->_vt_ms = v->GetMR();
  c->InitStateVariables();
  c->SetDataTransferTimers();
  ChangeState(c,sscop_ready::Instance(),"sscop_ready");
  cout << "SSCOP is READY now ..." << endl;
  c->Send2SSCF(v);
  return 1;
}

int sscop_outconn::RecvEND(SSCOPconn *c, SSCOPVisitor *v){ return 1;}

int sscop_outconn::RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v)
{
  c->StopTimerCC();
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v);
  return 1;
}

void sscop_outconn::ExpTimerCC(SSCOPconn *c)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << "TimerCC going off at time " << theKernel().CurrentTime() << endl);
  if (c->_vt_cc >= c->_MaxCC)
    {
      char name[40];
      Name(name);
      c->MAAError(name,'O',0);
      SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
      c->Send2Peer(sv);
      ChangeState(c, sscop_idle::Instance(),"sscop_idle");
      SSCOPVisitor *v = new BeginVisitor(SSCOPVisitor::BeginRejInd,0,0);
      c->Send2SSCF(v);
      return;
    }
  c->_vt_cc++;
  c->ReSendVisitorToPeer(c->_last_bgn);
  c->SetTimerCC();
}

int sscop_outconn::RecvSTAT(SSCOPconn *c, SSCOPVisitor *v){ return 1; }
int sscop_outconn::RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v){ return 1; }
int sscop_outconn::RecvPOLL(SSCOPconn *c, SSCOPVisitor *v){ return 1; }

int sscop_outconn::AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v)
{
  c->StopTimerCC();
  c->_vt_cc = 1;
  c->Send2Peer(v);
  c->SetTimerCC();
  ChangeState(c,sscop_outdisc::Instance(),"sscop_outdisc");
  v->Suicide();
  return(1);
}

int sscop_outconn::RecvER(SSCOPconn *c, SSCOPVisitor *v){ return 1; }

int sscop_outconn::RecvBGN(SSCOPconn *c, SSCOPVisitor *v)
{
  if (c->DetectRetransmission(v->GetSQ()))
    return 1;
  c->StopTimerCC();
  c->_vt_ms = v->GetMR();
  c->InitVRMR();
  SSCOPVisitor *sv = new BeginAckVisitor(SSCOPVisitor::BeginAckReq,0,0);
  c->Send2Peer(sv);
  c->InitStateVariables();
  c->SetDataTransferTimers();
  ChangeState(c,sscop_ready::Instance(),"sscop_ready");
  // send an Establish Confirm to SSCF
  c->Send2SSCF(v);
  return (1);
}

int sscop_outconn::RecvRSAK(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}

int sscop_outconn::RecvRS(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}



