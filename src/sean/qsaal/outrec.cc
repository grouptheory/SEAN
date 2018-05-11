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

//sscop_outrec
#ifndef LINT
static char const _outrec_cc_rcsid_[] =
"$Id: outrec.cc,v 1.2 1998/08/06 04:04:16 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/basics/diag.h>
#include <FW/kernel/Kernel.h>

#include "sscop.h"
#include "sscopState.h"
#include "sscop_visitors.h"
#include "sscop_timers.h"

sscopState *sscop_outrec::_instance = 0;

sscop_outrec::sscop_outrec(){}
sscop_outrec::~sscop_outrec(){}

sscopState* sscop_outrec::Instance()
{
  if (!_instance)
    _instance = new sscop_outrec;
  return(_instance);
}

inline void sscop_outrec::Name(char *name) {strcpy(name,"idle");}

inline SSCOPconn::sscop_states  sscop_outrec::StateID()
{
  return SSCOPconn::sscop_outrec;
}

int sscop_outrec::AA_DATAReq(SSCOPconn *c, SSCOPVisitor *v)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ": AA_DATAReq() " << endl);
   if(!c->_cb)
     {
       c->_TXQueue.append(v);
       // this is the equivalent to generating the
       // internal signal  SD PDU queued up
       c->SendQueuedSD(); 
     }
   return 1;
}

int sscop_outrec::RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'C',v);
  return 1;
}

int sscop_outrec::RecvERAK(SSCOPconn *c, SSCOPVisitor *v)
{
  c->StopTimerCC();
  c->_vt_ms = v->GetMR();
  // give the user whatever is in _RXBuffer
  c->DeliverData();
  ChangeState(c,sscop_recresp::Instance(),"sscop_recresp");
  c->Send2SSCF(v);
  return 1;
}

int sscop_outrec::RecvEND(SSCOPconn *c, SSCOPVisitor *v)
{
  c->StopTimerCC();
  SSCOPVisitor *sv = new EndAckVisitor(SSCOPVisitor::EndAckReq,0,0);
  c->Send2Peer(sv);
  c->_RXBuffer.clear();
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v);
  return 1;
}


int sscop_outrec::RecvENDAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'F',0);
  c->StopTimerCC();
  c->_RXBuffer.clear();
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v);
  return 1;
}

int sscop_outrec::RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'D',0);
  c->StopTimerCC();
  c->_RXBuffer.clear();
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v);
  return 1;
}

void sscop_outrec::ExpTimerCC(SSCOPconn *c)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ": ExpTimerCC() " << endl);
  if (c->_vt_cc >= c->_MaxCC)
    {
      char name[40];
      Name(name);
      c->MAAError(name,'O',0);
      SSCOPVisitor *sv = new EndVisitor(SSCOPVisitor::EndReq,0,0);
      c->Send2Peer(sv);
      c->_RXBuffer.clear();
      ChangeState(c,sscop_idle::Instance(),"sscop_idle");
      // send a ReleaseInd
      SSCOPVisitor *rv = new EndVisitor(SSCOPVisitor::EndInd,0,0);
      c->Send2SSCF(rv);
      return;
    }
  c->_vt_cc++;
  c->ReSendVisitorToPeer(c->_last_er);
  c->SetTimerCC();
  return;
}

inline int sscop_outrec::RecvSTAT(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}

inline int sscop_outrec::RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}

inline int sscop_outrec::RecvPOLL(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}

int sscop_outrec::AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ": AA_ReleaseReq() " << endl);
  c->StopTimerCC();
  c->_vt_cc = 1;
  c->Send2Peer(v);
  c->_RXBuffer.clear();
  c->SetTimerCC();
  ChangeState(c,sscop_outdisc::Instance(), "sscop_outdisc");
  return 1;
}

int sscop_outrec::AA_ResyncReq(SSCOPconn *c, SSCOPVisitor *v)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ": AA_ResyncReq() " << endl);
  c->StopTimerCC();
  c->_vt_cc = 1;
  c->IncVTSQ();
  c->InitVRMR();
  c->Send2Peer(v);
  c->ClearTransmitter();
  c->_RXBuffer.clear();
  c->SetTimerCC();
  ChangeState(c,sscop_outres::Instance(), "sscop_outres");
  return 1;
}

inline int sscop_outrec::RecvSD(SSCOPconn *c,SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}

int sscop_outrec::RecvBGN(SSCOPconn *c, SSCOPVisitor *v)
{
  if (c->DetectRetransmission(v->GetSQ()))
    {
      char name[40];
      Name(name);
      c->MAAError(name,'B',v);
      return 1;
    }
  c->StopTimerCC();
  c->_vt_ms = v->GetMR();
  c->_RXBuffer.clear();
  ChangeState(c,sscop_inconn::Instance(),"sscop_inconn");
  // send AA_ReleaseInd and AA-EstablishInd
  SSCOPVisitor *rv = new EndAckVisitor(SSCOPVisitor::EndAckInd,0,0);
  c->Send2SSCF(rv); // AA-ReleaseInd
  c->Send2SSCF(v); // AA-EstablishInd
  return 1;
}

int sscop_outrec::RecvER(SSCOPconn *c, SSCOPVisitor *v)
{
  if (c->DetectRetransmission(v->GetSQ()))
    {
      char name[40];
      Name(name);
      c->MAAError(name,'L',v);
      return 1;
    }
  c->StopTimerCC();
  c->_vt_ms = v->GetMR();
  c->InitVRMR();
  SSCOPVisitor *sv = new RecovAckVisitor(SSCOPVisitor::RecovAckReq,0,0);
  c->Send2Peer(sv);
  c->DeliverData();
  ChangeState(c,sscop_recresp::Instance(),"sscop_recresp");
  c->Send2SSCF(v);
  return 1;
}


int sscop_outrec::RecvRSAK(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[40];
  Name(name);
  c->MAAError(name,'K',v);
  return 1;
}



int sscop_outrec::RecvRS(SSCOPconn *c, SSCOPVisitor *v)
{
  if (c->DetectRetransmission(v->GetSQ()))
    {
      char name[40];
      Name(name);
      c->MAAError(name,'J',v);
      return 1;
    }
  c->StopTimerCC();
  c->_vt_ms = v->GetMR();
  c->_RXBuffer.clear();
  ChangeState(c,sscop_inres::Instance(),"sscop_inres");
  c->Send2SSCF(v);
  return 0;
}


inline int sscop_outrec::AA_RetrieveReq(SSCOPconn *c, SSCOPVisitor *v)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << c->OwnerName() << ": AA_RetrieveReq() " << endl);
  v->Suicide();
  return -1;
}



