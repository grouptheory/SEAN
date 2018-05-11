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
#ifndef __CALL_PERIODIC_H__
#define __CALL_PERIODIC_H__

#ifndef LINT
static char const _call_periodic_h_rcsid_[] =
"$Id: call_periodic.h,v 1.5 1998/07/29 22:04:17 talmage Exp $";
#endif

#include <sean/simple-tests/BaseClient.h>

// It never gives up, 
// establishes calls when they become inactive, 
// and releases calls when they become active.
class call_periodic : public BaseClient {
public:

  call_periodic(ATM_Interface & interface, double period, double stop_time,
		char *called_party, char *bhli = 0);
  ~call_periodic();

private:

  virtual void PeriodicCallback(int code);
  virtual void Call_Active(ATM_Call   & call);
  virtual void Call_Inactive(ATM_Call & call);

  int _call_is_active;
  Outgoing_ATM_Call * _call;

  int _total_calls;
  int _active_calls;
  int _inactive_calls;
};

#endif // __CALL_PERIODIC_H__
