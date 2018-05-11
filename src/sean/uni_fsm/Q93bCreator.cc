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
static char const _Q93bCreator_cc_rcsid_[] =
"$Id: Q93bCreator.cc,v 1.2 1998/08/06 04:04:23 bilal Exp $";
#endif
#include <common/cprototypes.h>
// -*- C++ -*-

extern "C" {
#include <stdio.h>
};

#include <FW/behaviors/Protocol.h>
#include <FW/basics/Conduit.h>
#include <FW/basics/diag.h>
#include <FW/basics/VisitorType.h>
#include "VCAllocator.h"
#include "Q93bCreator.h"
#include "Q93bCall.h"
#include "Q93bVisitors.h"

const VisitorType * Q93bCreator::_uni_visitor_type = 0;

Q93bCreator::Q93bCreator(int un):_un(un)
{
  if (!_uni_visitor_type)
    _uni_visitor_type = (VisitorType *)QueryRegistry(Q93B_VISITOR_NAME);
}

Q93bCreator::~Q93bCreator() { }

Conduit * Q93bCreator::Create(Visitor * v)
{
  Conduit *c = 0;
  Protocol *p = 0;
  
  VisitorType vt = v->GetType();
  diag("fsm.nni", DIAG_DEBUG, "*** Factory (%x) ***\n", this);

  if (vt.Is_A(_uni_visitor_type))
    {
      Call * s = new Call(_un);
      p = new Protocol(s);
      c = new Conduit("Call",p);
      s->SetIdentity(c);
      Register(c);
      return c;
    }
  return 0;
}


