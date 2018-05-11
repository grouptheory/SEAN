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

#include <sys/types.h>
#ifndef LINT
static char const _sscf_cc_rcsid_[] =
"$Id: sscf.cc,v 1.8 1999/03/26 00:43:08 battou Exp battou $";
#endif
#include <common/cprototypes.h>

#include <FW/basics/diag.h>
#include <codec/uni_ie/ie.h>
#include <codec/q93b_msg/q93b_msg.h>
#include <sean/uni_fsm/SSCFVisitor.h>
#include <sean/cc/sean_Visitor.h>

#include "sscf.h"
#include "sscop.h"
#include "sscop_visitors.h"

const VisitorType * SSCFconn::_uni_visitor_type = 0;
const VisitorType * SSCFconn::_sscf_visitor_type = 0;
const VisitorType * SSCFconn::_sscop_visitor_type = 0;


void crv_epr_toggle(u_char *pdu, int len)
{
  // print message before toggling the bits
  generic_q93b_msg *msg = Parse(pdu,len);

  if((pdu[0] != 0x9) && (pdu[1] != 0x3))
    return; // see figure 5.17 of uni-3.1 page 183
  pdu[2] ^= 0x80; // toggle crv flag
  // search for EPR and toggle
  int msg_len = pdu[7] << 8 | pdu[8];
  u_char *bptr = &pdu[9];
  int epr_id = 0x54;
  while(msg_len > 0)
    {
      if(bptr[0] != epr_id)
	{
	  int ie_len = bptr[2] << 8 | bptr[3];
	  bptr += (ie_len + 4);
	  msg_len -= (ie_len + 4);
	  continue;
	}
      // we got it
      bptr[5] ^= 0x80;
      return;
    }
}

SSCFconn::SSCFconn() : State( false ) // not queueing
{
  if (!_uni_visitor_type)
    _uni_visitor_type = (VisitorType *)QueryRegistry(SEAN_VISITOR_NAME);
  if (!_sscf_visitor_type)
    _sscf_visitor_type = (VisitorType *)QueryRegistry(SSCF_VISITOR_NAME);
  if (!_sscop_visitor_type)
    _sscop_visitor_type = (VisitorType *)QueryRegistry(SSCOP_VISITOR_NAME);
}

SSCFconn::~SSCFconn(){}

State * SSCFconn::Handle(Visitor *v)
{
  SSCOPVisitor *s = 0;
  VisitorType v2 = v->GetType();
  if (v2.Is_A(_uni_visitor_type))
    {
      sean_Visitor *qv = (sean_Visitor *)v;
      char name[40];
      qv->Name(name);
      DIAG("fsm.sscop", DIAG_DEBUG, cout << "SSCFconn::Handle received : " << name << endl);
      generic_q93b_msg *msg = qv->share_message();
      s = new SDVisitor(SSCOPVisitor::SDReq,msg->encoded_buffer(),msg->encoded_bufferlen());
      DIAG("fsm.sscop", DIAG_DEBUG, cout << "OutGoing Encoded at SSCF::Handle: \n"; msg->PrintEncoded(cout); cout << endl);;
      DIAG("fsm.sscop", DIAG_DEBUG, cout << "OutGoing Message at SSCF::Handle: \n"; msg->PrintSymbolic(cout); cout << endl);;
      
      //  msg->UniDump("SSCF: vvv ");

      qv->Suicide();
      PassVisitorToB(s);
      return this;
    }
  if (v2.Is_A(_sscf_visitor_type))
    {
      sscfVisitor *cv = (sscfVisitor *)v;
      char name[40];
      cv->Name(name);
      DIAG("fsm.sscf", DIAG_DEBUG, cout << "SSCFconn::Handle received : " << name << endl);
      switch(cv->GetVT())
	{
	case sscfVisitor::EstReq:
	  s = new BeginVisitor(SSCOPVisitor::BeginReq,0,0);
	  PassVisitorToB(s);
	  break;
	case sscfVisitor::RelReq:
	  s = new EndVisitor(SSCOPVisitor::EndReq,0,0);
	  PassVisitorToB(s);
	  break;
	default:
	  break;
	}
      cv->Suicide();
      return this;
    }
  if (v2.Is_A(_sscop_visitor_type))
    {
      SSCOPVisitor *sv = (SSCOPVisitor *)v;
      char name[40];
      sv->Name(name);
      DIAG("fsm.sscf", DIAG_DEBUG, cout << "SSCFconn::Handle received : " << name << endl);
      if(sv->GetVT() == SSCOPVisitor::SDInd)
	{
	  int len = sv->TakeLen();
	  u_char *pdu = sv->TakePDU();

	  crv_epr_toggle(pdu,len);

	  DIAG("fsm.sscop", DIAG_INFO, {                 \
          cout << "len = " << dec << len << endl;        \
	  int val;                                       \
	  for(int i=0;i<len;i++)                         \
	    {                                            \
	      if ((i!=0) && (i%12 == 0)) cout << endl;   \
	      val = (int)(pdu[i]);                       \
	      if (val<16) cout << "0";                   \
	      cout << hex << val;                        \
	      if ((i%2) == 1) cout << " ";               \
	    }                                            \
	  cout << endl;                                  \
          });

	  generic_q93b_msg *msg = Parse(pdu,len);
	  if(msg && (1 == msg->Valid()))
	    {
	      DIAG("fsm.sscop", DIAG_INFO, {                                                          \
	      cout << "RAW, BEFORE FLIPPING CREF/EPR BITS\n";                                         \
	      cout << "Incoming Errors at SSCF::Handle: \n"; msg->PrintErrors(cout); cout << endl;    \
	      cout << "Incoming Message at SSCF::Handle: \n"; msg->PrintSymbolic(cout); cout << endl; \
	      });									

	      //   msg->UniDump("SSCF: ^^^ ");

	      sean_Visitor *qv = 0;
	      generic_q93b_msg::msg_type msg_type = msg->type();
	      // add a method to generic_q93b_msg to make the "right" visitor to replace this switch
	      // virtual sean_Visitor *MakeVisitor() = 0;
	      // and redefine it in each msg to make the corresponding visitor: setup <--> SetupVisitor etc.
	      switch(msg_type)
		{
		case generic_q93b_msg::connect_msg:
		  qv = new sean_Visitor(sean_Visitor::connect_ind, msg);
		  break;
		case generic_q93b_msg::setup_msg:
		  qv = new sean_Visitor(sean_Visitor::setup_ind, msg);
		  break;
		case generic_q93b_msg::release_msg:
		  qv = new sean_Visitor(sean_Visitor::release_ind, msg);
		  break;
		case generic_q93b_msg::release_comp_msg:
		  qv = new sean_Visitor(sean_Visitor::release_comp_ind,msg);
		  break;
		case generic_q93b_msg::add_party_ack_msg:
		  qv = new sean_Visitor(sean_Visitor::add_party_ack_ind,msg);
		  break;
		case generic_q93b_msg::add_party_rej_msg:
		  qv = new sean_Visitor(sean_Visitor::add_party_rej_ind,msg);
		  break;
		case generic_q93b_msg::drop_party_ack_msg:
		  qv = new sean_Visitor(sean_Visitor::drop_party_ack_ind,msg);
		  break;
		case generic_q93b_msg::call_proceeding_msg:
		  qv = new sean_Visitor(sean_Visitor::call_proc_ind,msg);
		  break;
		case generic_q93b_msg::connect_ack_msg:
		  qv = new sean_Visitor(sean_Visitor::connect_ack_ind,msg);
		  break;
		case generic_q93b_msg::status_msg:
		  qv = new sean_Visitor(sean_Visitor::status_ind,msg);
		  break;
		case generic_q93b_msg::status_enq_msg:
		  qv = new sean_Visitor(sean_Visitor::status_enq_ind,msg);
		  break;
		case generic_q93b_msg::add_party_msg:
		  qv = new sean_Visitor(sean_Visitor::add_party_ind,msg);
		  break;
		case generic_q93b_msg::drop_party_msg:
		  qv = new sean_Visitor(sean_Visitor::drop_party_ind,msg);
		  break;
		case generic_q93b_msg::restart_msg:
		  qv = new sean_Visitor(sean_Visitor::restart_ind,msg);
		  break;
		case generic_q93b_msg::restart_ack_msg:
		  qv = new sean_Visitor(sean_Visitor::restart_ack_ind,msg);
		  break;

		case generic_q93b_msg::leaf_setup_request_msg:
		  qv = new sean_Visitor(sean_Visitor::leaf_setup_ind,msg);
		  break;

		case generic_q93b_msg::leaf_setup_failure_msg:
		  qv = new sean_Visitor(sean_Visitor::leaf_setup_failure_ind,msg);
		  break;

		case generic_q93b_msg::notify_msg:
		  // ???
		  // qv = new sean_Visitor(sean_Visitor::notify_ind,msg);
		  break;

		case generic_q93b_msg::party_alerting_msg:
		  // ???
		  // qv = new sean_Visitor(sean_Visitor::party_alert_ind,msg);
		  break;

		default:
		  DIAG("fsm.sscop", DIAG_INFO, cout << "unknown message\n"; );
		  break;
		}
	      // set the vpi and vci
	      InfoElem ** ies = msg->get_ie_array();
	      if(ies[InfoElem::ie_conn_identifier_ix])
		{
		  ie_conn_id *cid = (ie_conn_id *)ies[InfoElem::ie_conn_identifier_ix];
		  qv->set_vpi(cid->get_vpi());
		  qv->set_vci(cid->get_vci());
		}
	      PassVisitorToA(qv);
	    }
	  sv->Suicide();
	  return this;
	}
      sscfVisitor *sscf_v = 0;
      switch(sv->GetVT())
	{
	case SSCOPVisitor::EndInd:
	  s = new EndAckVisitor(SSCOPVisitor::EndAckReq,0,0);
	  sscf_v = new sscfVisitor(sscfVisitor::RelInd);
	  break;
	case SSCOPVisitor::BeginInd :
	  sscf_v = new sscfVisitor(sscfVisitor::EstInd);
	  s = new BeginAckVisitor(SSCOPVisitor::BeginAckReq,0,0);
	  break;

	case SSCOPVisitor::BeginAckInd :
	  sscf_v = new sscfVisitor(sscfVisitor::EstConf);
	  break;

	case SSCOPVisitor::EndAckInd :
	  sscf_v = new sscfVisitor(sscfVisitor::RelConf);
	  break;
	case SSCOPVisitor::ResyncInd :
	  s = new ResyncAckVisitor(SSCOPVisitor::ResyncAckReq,0,0);
	  break;
	case SSCOPVisitor::ResyncAckInd :
	  break;
	case SSCOPVisitor::BeginRejInd :
	  break;
	case SSCOPVisitor::RecovInd :
	  s = new RecovAckVisitor(SSCOPVisitor::RecovAckReq,0,0);
	  break;
	case SSCOPVisitor::PollInd :
	  break;
	case SSCOPVisitor::StatInd :
	  break;
	case SSCOPVisitor::UstatInd :
	  break;
	case SSCOPVisitor::UDInd :
	  break;
	case SSCOPVisitor::MDInd :
	  break;
	case SSCOPVisitor::RecovAckInd :
	  s = new RecovAckVisitor(SSCOPVisitor::RecovAckReq,0,0);
	  break;
	case SSCOPVisitor::MAAErrorInd :
	  break;
	}
      if(s)
	PassVisitorToB(s);
      if(sscf_v)
	PassVisitorToA(sscf_v);
      sv->Suicide();
    }
  return this;
}

void SSCFconn::Interrupt(SimEvent *e)
{
}




