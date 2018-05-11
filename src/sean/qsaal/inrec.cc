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

//sscop_inrec
#ifndef LINT
static char const _inrec_cc_rcsid_[] =
"$Id: inrec.cc,v 1.2 1998/08/06 04:04:15 bilal Exp $";
#endif
#include <common/cprototypes.h>
#include <FW/basics/diag.h>
#include <FW/kernel/Kernel.h>
#include "sscop.h"
#include "sscopState.h"
#include "sscop_visitors.h"
#include "sscop_timers.h"

sscopState *sscop_inrec::_instance = 0;

sscop_inrec::sscop_inrec(){}
sscop_inrec::~sscop_inrec(){}

sscopState* sscop_inrec::Instance()
{
  if (!_instance)
    _instance = new sscop_inrec;
  return(_instance);
}

inline void sscop_inrec::Name(char *name) {strcpy(name,"idle");}

inline SSCOPconn::sscop_states  sscop_inrec::StateID()
{
  return SSCOPconn::sscop_inrec;
}


int sscop_inrec::AA_RecoverResp(SSCOPconn *c, SSCOPVisitor *sv)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ":AA_RecoverResp() " << endl);
  if (!c->_cb)
    c->_TXBuffer.clear();
  c->InitVRMR();
  c->Send2Peer(sv);
  c->InitStateVariables();
  c->SetDataTransferTimers();
  ChangeState(c,sscop_ready::Instance(),"sscop_ready");
  return 1;
}

int sscop_inrec::AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ": AA_ReleaseReq() " << endl);
  c->_vt_cc = 1;
  c->Send2Peer(v);
  c->SetTimerCC();
  ChangeState(c,sscop_outdisc::Instance(),"sscop_outdisc");
  return 1;
}

int sscop_inrec::RecvEND(SSCOPconn *c, SSCOPVisitor *v)
{
  SSCOPVisitor *sv = new EndAckVisitor(SSCOPVisitor::EndAckReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v); // AA-ReleaseInd
  return 1;
}


int sscop_inrec::AA_ResyncReq(SSCOPconn *c, SSCOPVisitor *v)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ": AA_ResyncReq() " << endl);
  c->ClearTransmitter();
  c->_vt_cc = 1;
  c->IncVTSQ();
  c->InitVRMR();
  c->Send2Peer(v);
  c->SetTimerCC();
  ChangeState(c,sscop_outres::Instance(),"sscop_outres");
  return 1;
}

int sscop_inrec::RecvENDAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'F',0);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v);
  return 1;
}

int sscop_inrec::RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v)
{

  char name[40];
  Name(name);
  c->MAAError(name,'D',0);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v);
  return 1;
}




int sscop_inrec::RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'I',0);
  v->Suicide();
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  SSCOPVisitor *rv = new EndVisitor(SSCOPVisitor::EndInd,0,0);
  c->Send2SSCF(rv);
  return 1;
}

int sscop_inrec::RecvSTAT(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'H',0);
  v->Suicide();
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  SSCOPVisitor *rv = new EndVisitor(SSCOPVisitor::EndInd,0,0);
  c->Send2SSCF(rv);
  return 1;
}

int sscop_inrec::RecvPOLL(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'G',0);
  v->Suicide();
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  SSCOPVisitor *rv = new EndVisitor(SSCOPVisitor::EndInd,0,0);
  c->Send2SSCF(rv);
  return 1;
}

int sscop_inrec::RecvSD(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'A',0);
  v->Suicide();
  SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  SSCOPVisitor *rv = new EndVisitor(SSCOPVisitor::EndInd,0,0);
  c->Send2SSCF(rv);
  return 1;
}

int sscop_inrec::RecvRSAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'K',0);
  return 1;
}

int sscop_inrec::RecvRS(SSCOPconn *c, SSCOPVisitor *v)
{
  if (c->DetectRetransmission(v->GetSQ()))
    {
      char name[40];
      Name(name);
      c->MAAError(name,'J',0);
      return 1;
    }
  c->_vt_ms = v->GetMR();
  ChangeState(c,sscop_inrec::Instance(),"sscop_inres");
  c->Send2SSCF(v); // AA-ResyncInd
  return 1;
}

int sscop_inrec::RecvER(SSCOPconn *c, SSCOPVisitor *v)
{
  if (!c->DetectRetransmission(v->GetSQ()))
    {
      char name[40];
      Name(name);
      c->MAAError(name,'L',0);
    }
  return 1;
}


int sscop_inrec::RecvBGN(SSCOPconn *c, SSCOPVisitor *v)
{
  if (c->DetectRetransmission(v->GetSQ()))
    {
      char name[40];
      Name(name);
      c->MAAError(name,'B',0);
      ChangeState(c,sscop_inrec::Instance(),"sscop_inrec");
      return 1;
    }
  c->_vt_ms = v->GetMR();
  ChangeState(c,sscop_inconn::Instance(),"sscop_inconn");
  SSCOPVisitor *rv = new EndAckVisitor(SSCOPVisitor::EndAckInd,0,0);
  c->Send2SSCF(rv); // AA-ReleaseInd
  c->Send2SSCF(v); // AA-EstablishInd
  return 1;
}

int sscop_inrec::RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'C',0);
  return 1;
}

int sscop_inrec::RecvERAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'M',0);
  return 1;
}
