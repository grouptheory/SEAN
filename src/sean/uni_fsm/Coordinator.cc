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
static char const _Coordinator_cc_rcsid_[] =
"$Id: Coordinator.cc,v 1.2 1998/08/06 04:04:21 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/basics/diag.h>
#include <FW/basics/VisitorType.h>
#include <sean/cc/sean_Visitor.h>
#include "Coordinator.h"
#include "Q93bTimers.h"
#include "SSCFVisitor.h"
#include <codec/q93b_msg/q93b_msg.h>

#define  CREF_FLAG  0x800000

const VisitorType * Coordinator::_uni_visitor_type = 0;
const VisitorType * Coordinator::_link_visitor_type = 0;

Coordinator::Coordinator(int user) : State( true ) , _user(user)
{
  if (!_uni_visitor_type)
    _uni_visitor_type = (VisitorType *)QueryRegistry(SEAN_VISITOR_NAME);
  if (!_link_visitor_type)
    _link_visitor_type = (VisitorType *)QueryRegistry(SSCF_VISITOR_NAME);
  _t309 = new Q93b_t309(this);
  _cs = cu0_conn_released;
}

Coordinator::~Coordinator() { }

void Coordinator::Interrupt(SimEvent *event)
{
  // does nothing
}

void Coordinator::SetT309() { Register(_t309);}

void Coordinator::StopT309() {Cancel(_t309); }

State * Coordinator::Handle(Visitor* v)
{
  diag("fsm.nni", DIAG_DEBUG, "*** Coordinator (%x) ***\n", this);
  VisitorType v2 = v->GetType();
  sscfVisitor *lv;
  switch (_cs) {
    case cu0_conn_released:
      if (v2.Is_A(_uni_visitor_type)) {
	// uni visitors
	_ov.append(v);
	Visitor *nv = new sscfVisitor(sscfVisitor::EstReq);
	nv->SetLast(0);
	_cs = cu1_await_establish;
	SetT309();
	PassVisitorToB(nv);
      } else {
	// must be a link visitor
	lv = (sscfVisitor *)v;
	switch (lv->GetVT()) {
	  case sscfVisitor::EstReq:
	    _cs = cu1_await_establish;
	    SetT309();
	    PassVisitorToB(v);
	    break;
	  case sscfVisitor::EstInd:
	    _cs = cu3_conn_established;
	    PassThru(v);
	    break;
	  default:// should not be getting anything else
	    break;
	}
      }
      break;

    case cu1_await_establish:
      // nothing else but sscfVisitor at this stage
      assert(true == v2.Is_A(_link_visitor_type));
      lv = (sscfVisitor *)v;
      switch (lv->GetVT()) {
	case sscfVisitor::EstConf:
	case sscfVisitor::EstInd:
	  StopT309();
	  _cs = cu3_conn_established;
	  PassThru(v);
	  break;

	case sscfVisitor::RelInd:
	  _cs = cu0_conn_released;
	  PassThru(v);
	  break;
	default: // should not be getting anything else
	  break;
	}
      break;

    case cu2_await_release:
      // nothing else but sscfVisitor at this stage
      assert(true == v2.Is_A(_link_visitor_type));
      lv = (sscfVisitor *)v;
      switch (lv->GetVT()) {
	case sscfVisitor::RelConf:
	  _cs = cu0_conn_released;
	  PassThru(v);
	  break;
	default:// should not be getting anything else
	  break;
      }
      break;


    case cu3_conn_established:
      if (v2.Is_A(_link_visitor_type)) {
	// must be link visitor
	assert(true == v2.Is_A(_link_visitor_type));
	lv = (sscfVisitor *)v;
	switch (lv->GetVT()) {
	  case sscfVisitor::RelReq:
	    _cs = cu2_await_release;
	    PassThru(v);
	    break;
	    
	  case sscfVisitor::RelInd:
	    _cs = cu0_conn_released;
	    PassThru(v);
	    break;
	  case sscfVisitor::EstInd:
	  case sscfVisitor::EstConf:
	    PassThru(v);
	    break;
	  case sscfVisitor::EstReq: // bouce back as an EstConf
	    lv->SetVT(sscfVisitor::EstConf);
	    PassVisitorToA(v); 
	    break;
	}
      } else {
	// upward flow: verify the Q93b message here before it gets to UNI 
	// MAYBE the flag ?
	// Q93bVisitor *qv = (Q93bVisitor *)v;
	// this is should be set earlier maybe SSCF
	// qv->SetDecodeStatus(Q93bVisitor::OK);
	PassThru(v);
      }
      break;
  }
  if (_cs == cu3_conn_established)
    while (!_ov.empty())
      PassThru(_ov.pop());

  return this;
}

void Coordinator::ExpT309()
{
  if (_cs == cu0_conn_released || _cs ==  cu1_await_establish) {
    Visitor *v = new sscfVisitor(sscfVisitor::EstErr);
    v->SetLast(0);
    PassVisitorToA(v);
  }
}
