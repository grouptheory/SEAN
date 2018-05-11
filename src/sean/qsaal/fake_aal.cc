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

// -*- C++ -*-
#ifndef LINT
static char const _sscop_aal_cc_rcsid_[] =
"$Id: fake_aal.cc,v 1.2 1998/08/06 04:04:14 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "sscop_visitors.h"
#include "sscop.h"
#include "fake_aal.h"
#include <FW/basics/Visitor.h>
#include <FW/actors/State.h>


extern "C" {
#include <fcntl.h>
#include <netinet/in.h>
};

fake_SSCOPaal::fake_SSCOPaal(void) { }

// closes the device
fake_SSCOPaal::~fake_SSCOPaal()
{
}

SSCOPVisitor *fake_SSCOPaal::DecodePDU(u_char *pdu, int len)
{
  u_int *tptr = (u_int *)(pdu + len - 4);
  SSCOPconn::PDUtypes pdu_type = (SSCOPconn::PDUtypes )((ntohl(*tptr) >> 24) & 15);
  int pad = (ntohl(*tptr) >> 30);
  int size = len -4 - pad;
  SSCOPVisitor *sv = 0;
  switch (pdu_type)
    {
    case SSCOPconn::SSCOP_BGN  :
      sv = new BeginVisitor(SSCOPVisitor::BeginInd,0,0);
      break;
    case SSCOPconn::SSCOP_BGAK :  
      sv = new BeginAckVisitor(SSCOPVisitor::BeginAckInd,0,0);
      break;
    case SSCOPconn::SSCOP_BGREJ:  
      sv = new BeginRejVisitor(SSCOPVisitor::BeginRejInd,0,0);
      break;
    case SSCOPconn::SSCOP_END  : 
      sv = new EndVisitor(SSCOPVisitor::EndInd,0,0);
      break;
    case SSCOPconn::SSCOP_ENDAK: 
      sv = new EndAckVisitor(SSCOPVisitor::EndAckInd,0,0);
      break;
    case SSCOPconn::SSCOP_RS   : 
      sv = new ResyncVisitor(SSCOPVisitor::ResyncInd,0,0);
      break;
    case SSCOPconn::SSCOP_RSAK :  
      sv = new ResyncAckVisitor(SSCOPVisitor::ResyncAckInd,0,0);
      break;
    case SSCOPconn::SSCOP_ER   :  
      sv = new RecovVisitor(SSCOPVisitor::RecovInd,0,0);
      break;
    case SSCOPconn::SSCOP_ERAK : 
      sv = new RecovAckVisitor(SSCOPVisitor::RecovAckInd,0,0);
      break;
    case SSCOPconn::SSCOP_SD   :  
      sv = new SDVisitor(SSCOPVisitor::SDInd,0,0);
      break;
    case SSCOPconn::SSCOP_POLL : 
      sv = new PollVisitor(SSCOPVisitor::PollInd,0,0);
      break;
    case SSCOPconn::SSCOP_STAT : 
      sv = new StatVisitor(SSCOPVisitor::StatInd,0,0);
      break;
    case SSCOPconn::SSCOP_USTAT: 
      sv = new UStatVisitor(SSCOPVisitor::UstatInd,0,0);
      break;
    case SSCOPconn::SSCOP_UD   : 
      sv = new UDVisitor(SSCOPVisitor::UDInd,0,0);
      break;
    case SSCOPconn::SSCOP_MD   : 
      sv = new MDVisitor(SSCOPVisitor::MDInd,0,0);
      break;
    default:
      sv = new MAAErrorVisitor(SSCOPVisitor::MAAErrorInd,0,0);
      break;
    }
  sv->UnPackPDU(pdu,len);
  return sv;
}

int fake_SSCOPaal::AALread(u_char *pdu, int len)
{
  PassVisitorToA(DecodePDU(pdu,len));
  return len;
}

int fake_SSCOPaal::AALwrite(u_char *pdu, int len)
{
  PassVisitorToB(DecodePDU(pdu,len));
  return len;
}


void fake_SSCOPaal::Interrupt(class SimEvent *e) {}


State* fake_SSCOPaal::Handle(Visitor *v)
{
  const VisitorType * vt2 = QueryRegistry(SSCOP_VISITOR_NAME);
  VisitorType vt1 = v->GetType();
  // is it an SSCOPVisitor?
  if (vt2 && vt1.Is_A(vt2)) {  
    SSCOPVisitor *sv = (SSCOPVisitor *)v;
    
    switch( VisitorFrom(v) ) {
    case Visitor::A_SIDE:
      PassThru(v);
      break;
       
    case Visitor::B_SIDE:
      AALread(sv->TakePDU(), sv->TakeLen());
      v->Suicide();
      break;
    }
  }
  return this;
}
