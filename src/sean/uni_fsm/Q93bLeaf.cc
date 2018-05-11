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
static char const _Q93bLeaf_cc_rcsid_[] =
"$Id: Q93bLeaf.cc,v 1.3 1998/08/12 20:16:03 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <iostream.h>
#include <sys/types.h>
#include <sys/time.h>

#include<DS/containers/h_array.h>

#include <FW/kernel/Kernel.h>
#include <FW/actors/State.h>
#include <FW/kernel/Handlers.h>
#include <FW/basics/diag.h>
#include <codec/q93b_msg/q93b_msg.h>

#include "Q93bCall.h"
#include "Q93bLeaf.h"
#include "Q93bTimers.h"

Leaf::Leaf(Call * ca,int seqnum):_ca(ca),_seqnum(seqnum)
{
  _t331 = new Q93b_t331(this);
  _t331_retries = 0;
  _t331_max_retries = 1;
  _cs = Leaf::LeafSetupInitiated;
  _dest = 0;
  _dest_subaddr = 0;
}

Leaf::~Leaf() { }

void Leaf::LeafSetupReq() { SetT331(); }

void Leaf::LeafSetupFailure(){ StopT331(); }

void Leaf::SetT331()  { _ca->SetT331(this); }

void Leaf::StopT331() { _ca->StopT331(this); }

void Leaf::ExpT331()
{
  _t331_retries++;
  if (_t331_retries < _t331_max_retries)
    _ca->SendLeafSetupToPeer(_seqnum);
  else
    _ca->KillLeaf(_seqnum);
}

void Leaf::ChangeStateToNull()
{
  _cs = Leaf::Null;
}

void Leaf::set_called_number(ie_called_party_num * cn_ie)
{
  // Don't clobber _dest if it's the same as cn_ie.
  if (_dest && _dest != cn_ie) delete _dest;
  _dest = cn_ie;
}
 
void Leaf::set_called_subaddr(ie_called_party_subaddr * cs_ie)
{
  // Don't clobber _dest if it's the same as cn_ie.
  if (_dest_subaddr && _dest_subaddr != cs_ie) delete _dest_subaddr;
  _dest_subaddr=cs_ie;
}

InfoElem* Leaf::get_called_number(void)
{
  return _dest;
}

InfoElem* Leaf::get_called_subaddr(void)
{
  return _dest_subaddr;
}






