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
static char const _sscop_timers_cc_rcsid_[] =
"$Id: sscop_timers.cc,v 1.2 1998/08/06 04:04:18 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/kernel/Kernel.h>
#include <FW/kernel/SimEvent.h>
#include <FW/kernel/Handlers.h>

#include "sscop.h"
#include "sscopState.h"
#include "sscop_timers.h"

SSCOPTimer::SSCOPTimer(SSCOPconn *h, double ht)  : TimerHandler(h,ht), _active(false),
  _owner(h), _t(ht) { }

SSCOPTimer::~SSCOPTimer() { }

inline bool SSCOPTimer::Active(void) { return _active; }


TimerCC::TimerCC(SSCOPconn * h, double ht)  : SSCOPTimer(h,ht) { }

TimerCC::~TimerCC() { }

void TimerCC::Callback(void)
{
  _owner->GetCS()->ExpTimerCC(_owner);
  _owner->SetTimerCC();
}


TimerPOLL::TimerPOLL(SSCOPconn *h, double ht)  : SSCOPTimer(h,ht) { }

TimerPOLL::~TimerPOLL() { }

void TimerPOLL::Callback(void)
{
  _owner->GetCS()->ExpTimerPOLL(_owner);
  Register(this);
}


TimerNORESP::TimerNORESP(SSCOPconn *h, double ht)  : SSCOPTimer(h,ht) { }

TimerNORESP::~TimerNORESP() { }

void TimerNORESP::Callback(void)
{
  _owner->GetCS()->ExpTimerNORESP(_owner);
  _owner->SetTimerNORESP();
}

TimerKEEPALIVE::TimerKEEPALIVE(SSCOPconn *h, double ht)  : SSCOPTimer(h,ht) { }

TimerKEEPALIVE::~TimerKEEPALIVE() { }

void TimerKEEPALIVE::Callback(void)
{
  _owner->GetCS()->ExpTimerKEEPALIVE(_owner);
  _owner->SetTimerKEEPALIVE();
}

TimerIDLE::TimerIDLE(SSCOPconn *h, double ht) : SSCOPTimer(h,ht) { }

TimerIDLE::~TimerIDLE() { }

void TimerIDLE::Callback(void)
{
  _owner->GetCS()->ExpTimerIDLE(_owner);
  _owner->SetTimerIDLE();
}







