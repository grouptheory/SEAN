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
static char const _dec_cc_rcsid_[] =
"$Id: def_sscop.cc,v 1.3 1998/08/06 04:04:14 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <iostream.h>

#include "sscop.h"
#include "sscopState.h"
#include "sscop_visitors.h"
#include "sscop_timers.h"
#include <FW/basics/diag.h>

inline void sscopState::SetVTCC(SSCOPconn *c,int x) { c->_vt_cc = x;}
inline int sscopState::GetVTCC(SSCOPconn *c) { return c->_vt_cc;}

inline void sscopState::ChangeState(SSCOPconn *c, sscopState *s, char *name)
{
  c->ChangeState(s, name);
}

inline void sscopState::GetStateName(SSCOPconn *c, char *name)
{
  c->GetStateName(name);
}

int sscopState::AA_EstablishReq(SSCOPconn *c, SSCOPVisitor *v)
{ 
  char name[80];
  c->GetStateName(name);
  diag("fsm.sscop",DIAG_INFO,"AA_EstablishReq: state is %s\n", name);
  return -1;
 } 

int sscopState::AA_EstablishResp(SSCOPconn *c, SSCOPVisitor *v)
{
  char name[80];
  c->GetStateName(name);
  diag("fsm.sscop",DIAG_INFO,"AA_EstablishResp: state is %s\n", name);
  return -1;
}

/* section d. on page 10 Q.2110
 * RN+1 = the seqnum that is N(S) of the first SDU to be retrieved.
 * RN = UNKNOWN means retrieve the not yet transmitted SDUs only
 * RN = TOTAL means both the not yet transmitted (in _TXQueue)
 * and the not yet ACKED (_TXBuffer).
 */
int sscopState::AA_RetrieveReq(SSCOPconn *c, SSCOPVisitor *v)
{
  return -1; 
}

int sscopState::AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v){ return -1; }

int sscopState::AA_ResyncReq(SSCOPconn *c, SSCOPVisitor *v){ return -1; }

int sscopState::AA_ResyncResp(SSCOPconn *c, SSCOPVisitor *v){ return -1; }


int sscopState::AA_DATAReq(SSCOPconn *c, SSCOPVisitor *v){ return -1; }

int sscopState::AA_UNITDATAReq(SSCOPconn *c, SSCOPVisitor *v)
{
  v->SetPDUType(SSCOPconn::SSCOP_UD);
  c->Send2Peer(v);
  return 1;
}


int sscopState::MAA_UNITDATAReq(SSCOPconn *c, SSCOPVisitor *v)
{
  v->SetPDUType(SSCOPconn::SSCOP_MD);
  c->Send2Peer(v);
  return 1;
}

int sscopState::AA_RecoverResp(SSCOPconn *c, SSCOPVisitor *v) {return -1;}

int sscopState::RecvBGN(SSCOPconn *c, SSCOPVisitor *v){ return 0; }
int sscopState::RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v){ return 0; }
int sscopState::RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v){ return 0; }
int sscopState::RecvEND(SSCOPconn *c, SSCOPVisitor *v){ return 0; }
int sscopState::RecvENDAK(SSCOPconn *c, SSCOPVisitor *v){ return 0; }
int sscopState::RecvRS(SSCOPconn *c, SSCOPVisitor *v){ return 0; }
int sscopState::RecvRSAK(SSCOPconn *c, SSCOPVisitor *v){ return 0; }
int sscopState::RecvER(SSCOPconn *c, SSCOPVisitor *v){ return 0; }
int sscopState::RecvERAK(SSCOPconn *c, SSCOPVisitor *v){ return 0; }
int sscopState::RecvSD(SSCOPconn *c, SSCOPVisitor *v){ return 0; }
int sscopState::RecvPOLL(SSCOPconn *c, SSCOPVisitor *v){ return 0; }
int sscopState::RecvSTAT(SSCOPconn *c, SSCOPVisitor *v){ return 0; }
int sscopState::RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v){ return 0; }

int sscopState::RecvMAAError(SSCOPconn *c, SSCOPVisitor *v){ return 0; }

int sscopState::RecvUD(SSCOPconn *c, SSCOPVisitor *v)
{
  c->Send2SSCF(v);
  return 1;
}

int sscopState::RecvMD(SSCOPconn *c, SSCOPVisitor *v)
{
  c->Send2SSCF(v);
  return 1;
}


void sscopState::ExpTimerCC(SSCOPconn *c)
{

}

void sscopState::ExpTimerPOLL(SSCOPconn *c)
{
  //  _timer_poll = NULL;
}

void sscopState::ExpTimerNORESP(SSCOPconn *c)
{
  //  _timer_noresp = NULL;
  if (c->_cs->StateID() != SSCOPconn::sscop_ready)
    return;
  c->ResetDataTransferTimers();
  c->MAAError(NULL,'P',0);
  SSCOPVisitor *v = new EndVisitor(SSCOPVisitor::EndReq,0,0);
  v->SetPDUType(SSCOPconn::SSCOP_END);
  c->Send2Peer(v);
  c->PrepareRetrieval();
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->release_ind(0); // to FIX
}

void sscopState::ExpTimerKEEPALIVE(SSCOPconn *c)
{
  //  _timer_keepalive = NULL;
}

void sscopState::ExpTimerIDLE(SSCOPconn *c)
{
  //  _timer_idle = NULL;
  c->SetTimerNORESP();
}

