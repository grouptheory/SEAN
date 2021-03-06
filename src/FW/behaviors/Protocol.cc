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

#if !defined(LINT)
static char const _Protocol_cc_rcsid_[] =
"$Id: Protocol.cc,v 1.1 1999/01/13 19:12:50 mountcas Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/behaviors/Protocol.h>

#include <FW/basics/Visitor.h>
#include <FW/actors/State.h>
#include <FW/kernel/Kernel.h>
#include <FW/kernel/KDB.h>

//------------- PROTOCOL ---------------------------
Protocol::Protocol(State * s) : Behavior(s, Behavior::PROTOCOL) { }

Protocol::~Protocol() { }

void Protocol::Accept(Visitor * v, int is_breakpoint) 
{
  if (is_breakpoint) {
    ALERT_DEBUGGER;
  }

  v->_at(this, (State *)_actor);
}

void Protocol::UpdateState(State * ns)
{ 
  if (ns) {
    _actor = ns; 
    ns->SetBehavior(this);
  }
}

