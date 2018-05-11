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
static char const _ilmiTimers_cc_rcsid_[] =
"$Id: ilmiTimers.cc,v 1.4 1998/08/07 04:37:01 marsh Exp $";
#endif
#include <common/cprototypes.h>
#include <FW/kernel/Kernel.h>
#include <FW/kernel/SimEvent.h>
#include <FW/kernel/Handlers.h>

#include "ilmi.h"
#include "ilmiTimers.h"

ilmiTimer::ilmiTimer(ilmi *i, double it)  : TimerHandler(i,it), _active(false),
  _owner(i), _t(it) { }

ilmiTimer::~ilmiTimer() { }

inline bool ilmiTimer::Active(void) { return _active; }


void ilmiTimer::Callback(void)
{
  _active = false;
  _owner->ExpTimer();
}

void ilmiTimer::SetTimer()
{
  _active = true;
  Register(this);
}

void ilmiTimer::StopTimer()
{
  _active = false;
  Cancel(this);
}
