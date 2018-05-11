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
static char const _Loss_cc_rcsid_[] =
"$Id: Loss.cc,v 1.2 1998/08/06 04:04:14 bilal Exp $";
#endif
#include <common/cprototypes.h>
#include <iostream.h>
#include <FW/kernel/SimEvent.h>
#include <FW/basics/diag.h>

#include "Loss.h"
#include "sscop_visitors.h"


Loss::Loss(int size) : State()
{
  _size = size;
  _loseit = 0L;
  if(size > 0)
    {
      _loseit = new u_int[size];
      _size = size;
    }
}


Loss::~Loss()
{
  if(_loseit)
    delete [] _loseit;
}


State * Loss::Handle(Visitor * v)
{
  static  int cnt2 = 0;
  const VisitorType * vt2 = QueryRegistry(SSCOP_VISITOR_NAME);
  VisitorType vt1 = v->GetType();
  // is it a SSCOPVisitor?
  if (vt2 && vt1.Is_A(vt2))
    {
      SSCOPVisitor *sv = (SSCOPVisitor *)v;
      if(sv->GetVT() == SSCOPVisitor::SDReq || sv->GetVT() == SSCOPVisitor::SDInd)
	{
	  int seq = sv->GetS();
	  if(seq == 2)
	    {
	      if(_loseit[2] > 0)
		{
		  sv->Suicide();
		  _loseit[2]--;
		  return this;
		}
	    }
	  if(seq == 3)
	    {
	      SSCOPVisitor *x = sv->Copy();
	      x->SetLast(0);
	      PassVisitorToB(x);
	    }
	}
    }
  PassThru(v);
  return this;
}

void  Loss::Interrupt(SimEvent *event)
{
  
}

void Loss::MarkSeqNum(int s, int cnt)
{
  if(s < 0 || s >= _size)
    return;
  _loseit[s] = cnt;
}

