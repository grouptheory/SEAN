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

//sscop_outdisc
#ifndef LINT
static char const _outdisc_cc_rcsid_[] =
"$Id: outdisc.cc,v 1.2 1998/08/06 04:04:16 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "sscop.h"
#include "sscopState.h"
#include "sscop_visitors.h"
#include "sscop_timers.h"

sscopState *sscop_outdisc::_instance = 0;

sscop_outdisc::sscop_outdisc(){}
sscop_outdisc::~sscop_outdisc(){}

sscopState* sscop_outdisc::Instance()
{
  if (!_instance)
    _instance = new sscop_outdisc;
  return(_instance);
}

inline void sscop_outdisc::Name(char *name) {strcpy(name,"idle");}

inline SSCOPconn::sscop_states  sscop_outdisc::StateID()
{
  return SSCOPconn::sscop_outdisc;
}

// march 2,98
int sscop_outdisc::AA_EstablishReq(SSCOPconn *c, SSCOPVisitor *v)
{
  c->StopTimerCC();
  c->ClearTransmitter();
  c->_cb = v->GetBR();
  c->_vt_cc = 1;
  c->IncVTSQ();
  c->InitVRMR();
  c->Send2Peer(v);
  c->SetTimerCC();
  ChangeState(c,sscop_outconn::Instance(),"sscop_outconn");
  return 1;
} 

inline int sscop_outdisc::RecvSD(SSCOPconn *c,SSCOPVisitor *v){ return 1; }
inline int sscop_outdisc::RecvBGNAK(SSCOPconn *c,SSCOPVisitor *v){return 1;}
inline int sscop_outdisc::RecvPOLL(SSCOPconn *c, SSCOPVisitor *v){ return 1; }
inline int sscop_outdisc::RecvSTAT(SSCOPconn *c, SSCOPVisitor *v){ return 1; }
inline int sscop_outdisc::RecvUSTAT(SSCOPconn *c,SSCOPVisitor *v){ return 1; }

int sscop_outdisc::RecvEND(SSCOPconn *c, SSCOPVisitor *v)
{
  c->StopTimerCC();
  SSCOPVisitor *sv = new EndAckVisitor(SSCOPVisitor::EndAckReq,0,0);
  c->Send2Peer(sv);
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  // AA-ReleaseConf to SSCF
  c->Send2SSCF(v);
  return 1;
}

int sscop_outdisc::RecvENDAK(SSCOPconn *c, SSCOPVisitor *v)
{
  c->StopTimerCC();
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v); // AA-ReleaseConf
  return 1;
}

int sscop_outdisc::RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v)
{
  c->StopTimerCC();
  ChangeState(c,sscop_idle::Instance(),"sscop_idle");
  c->Send2SSCF(v); // AA-ReleaseConf
  return 1;
}

void sscop_outdisc::ExpTimerCC(SSCOPconn *c)
{
  if (c->_vt_cc >= c->_MaxCC)
    {
      char name[40];
      Name(name);
      c->MAAError(name,'O',0);
      ChangeState(c,sscop_idle::Instance(),"sscop_idle");
      // AA-ReleaseConf
      SSCOPVisitor *rv = new EndAckVisitor(SSCOPVisitor::EndAckInd,0,0);
      c->Send2SSCF(rv);
      return;
    }
  c->_vt_cc++;
  c->ReSendVisitorToPeer(c->_last_end);
  c->SetTimerCC();
  return;
}

inline int sscop_outdisc::RecvERAK(SSCOPconn *c,SSCOPVisitor *v){return 1;}
inline int sscop_outdisc::RecvRS(SSCOPconn *c,SSCOPVisitor *v){return 1;}
inline int sscop_outdisc::RecvRSAK(SSCOPconn *c,SSCOPVisitor *v){return 1;}


int sscop_outdisc::RecvBGN(SSCOPconn *c, SSCOPVisitor *v)
{
  if (c->DetectRetransmission(v->GetSQ()))
    {
      SSCOPVisitor *sv = new BeginAckVisitor(SSCOPVisitor::BeginAckReq,0,0);
      c->Send2Peer(sv);
      c->ReSendVisitorToPeer(c->_last_end);
      v->Suicide();
      return 1;
    }
  c->StopTimerCC();
  c->_vt_ms = v->GetMR();
  ChangeState(c,sscop_inconn::Instance(),"sscop_inconn");
  // send a AA-ReleaseConf and AA-EstablishInd to SSCF, perhaps combined
  SSCOPVisitor *rv = new EndAckVisitor(SSCOPVisitor::EndAckInd,0,0);
  c->Send2SSCF(rv); // AA-ReleaseInd
  c->Send2SSCF(v); // AA-EstablishInd
  return 1;
}





int sscop_outdisc::RecvER(SSCOPconn *c, SSCOPVisitor *v)
{
  v->Suicide();
  return 1;
}



