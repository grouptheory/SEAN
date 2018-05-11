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
"$Id: fore_aal.cc,v 1.5 1998/08/14 00:50:26 marsh Exp $";
#endif
#include <common/cprototypes.h>
#include <iostream.h>
#include <FW/basics/diag.h>
#include "sscop_visitors.h"
#include "sscop.h"


extern "C" {
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/stream.h>
#include <tiuser.h>

  /* __FORE_AAL__ is set by the makefile */
  /* if fore_xti is present */

#ifdef __FORE_AAL__
#include <fore_xti/xti_user_types.h>
#include <fore_xti/xti.h>
#include <fore_xti/xti_atm.h>
#include <fore_xti/ans.h>
#include "fore_aal.h"
#endif
};

#define DEVICE          "/dev/xtipvc0"


DriverReader::DriverReader(fore_SSCOPaal *owner, int fd):InputHandler(owner,fd),_owner(owner){ }

inline void DriverReader::Callback(void) {_owner->Read();}

DriverReader::~DriverReader()
{
}

// open the device and set the _fd
fore_SSCOPaal::fore_SSCOPaal(void)
{
  int sig_vpi = 0;
  int sig_vci = 5;
  int sig_qos = 0;
  int qos = 0;
  char *options = NULL;
  long optlen = 0;

  struct t_info info;
  _fd = t_open(DEVICE, O_RDWR, &info);
  if(_fd < 0)
    {
      cout << "t_open failed\n";
      exit(1);
    }
  _max_TSDU = info.options;
  if(t_bind(_fd, (struct t_bind *) NULL, (struct t_bind *) NULL) < 0)
    {
      cout << "t_bind failed\n";
      exit(1);
    }
  // no QoS, don't send TRAFFIC option 
  struct t_call call_req;
  PVCAddress addr_req;
  addr_req.addressType=AF_ATM_PVC;
  addr_req.vpi = sig_vpi;
  addr_req.vci = sig_vci;
  memset(&call_req, 0, sizeof(call_req));
  call_req.addr.len = sizeof(addr_req);
  call_req.addr.buf = (char *) &addr_req;

  diag("sscop.fsm", DIAG_WARNING, "Connecting to (%d,%d)\n", sig_vpi, sig_vci);

  options = (char *) NULL;
  if(t_connect(_fd, &call_req, (struct t_call *) NULL) < 0)
    {
      cout << "t_connect failed " << endl;
      exit(1);
    }
  cout << "connected " << endl;
  _reader = new DriverReader(this,_fd);
  assert(_reader);
  Register(_reader);
}

// closes the device
fore_SSCOPaal::~fore_SSCOPaal()
{
}

SSCOPVisitor *fore_SSCOPaal::DecodePDU(u_char *pdu, int len)
{
  // check for minimum size and alignment
   if(len < 4 || (len & 3))
     {
       DIAG("fsm.sscop", DIAG_DEBUG, cout << " -- BAD SSCOP PDU" << endl);
       return (SSCOPVisitor *)0;
     }
  u_int *pt = (u_int *)(pdu + len - 4);
  SSCOPconn::PDUtypes pdu_type = (SSCOPconn::PDUtypes )((ntohl(*pt) >> 24) & 15);
  SSCOPVisitor *sv = 0;

  char name[40];
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
      sv->Name(name);
      diag("fsm.sscop", DIAG_INFO, "Received a %s\n", name);
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

void fore_SSCOPaal::Read(void)
{
  int bytes_read;
  char *pdu=0;
  u_long *cnt;
  int flags;

  // largest UNI message is about 650 bytes
  int max_uni_bytes = 2000;
  pdu = new char[max_uni_bytes];
  bytes_read = t_rcv(_fd, pdu, max_uni_bytes, &flags);
  if(bytes_read < 4 || (bytes_read & 3))
    {
      cerr << "Received a BAD PDU \n";
    }
  else
    {
      diag("fsm.sscop", DIAG_INFO, "AAL: read %d bytes\n", bytes_read);
      PassVisitorToA(DecodePDU((u_char *)pdu,bytes_read));
    }
  delete [] pdu;
}


void fore_SSCOPaal::Interrupt(class SimEvent *e) {}


State* fore_SSCOPaal::Handle(Visitor *v)
{
  const VisitorType * vt2 = QueryRegistry(SSCOP_VISITOR_NAME);
  VisitorType vt1 = v->GetType();
  // is it an SSCOPVisitor?
  if (vt2 && vt1.Is_A(vt2))
    {
      SSCOPVisitor *sv = (SSCOPVisitor *)v;
      u_char *pdu = sv->TakePDU();
      int len = sv->TakeLen();
      if (sv->GetVT() == SSCOPVisitor::SDReq)
	{
	  char name[40];
	  sv->Name(name);
	  DIAG("sscop.fsm",DIAG_WARNING, {          \
	    cout << " Sending " << name <<          \
	      " pdu len = " << len << endl;         \
	    for(int i=0; i < len; i++)              \
	      {                                     \
		char hex[17]="0123456789abcdef";    \
		char left=hex[(pdu[i] >> 4)];       \
		char right=hex[ (pdu[i] & 0xf) ];   \
		cout << left << right << " ";       \
	      }                                     \
	    cout <<  endl;                          \
	  });
	}

      int bytes_sent = t_snd(_fd,(char *)pdu,len,0);
      diag("fsm.sscop", DIAG_INFO, "AAL: sent %d bytes\n", bytes_sent);
      v->Suicide();
      delete [] pdu;
      pdu = 0;
    }
  delete vt2;
  return this;
}



