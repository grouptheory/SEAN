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
#ifndef __call_BLASTER_H__
#define __call_BLASTER_H__

#ifndef LINT
static char const _call_blaster_h_rcsid_[] =
"$Id: call_blaster.h,v 1.4 1998/07/29 22:03:44 talmage Exp $";
#endif

#include <sean/simple-tests/BaseClient.h>

// Run this example w/o a server
//   It never gives up, establishes calls when they become inactive
class call_blaster : public BaseClient {
public:

  call_blaster(ATM_Interface & interface, double stop_time,
	       char *called_party);
  virtual ~call_blaster();

private:

  virtual void PeriodicCallback(int code);
  virtual void Call_Active(ATM_Call   & call);
  virtual void Call_Inactive(ATM_Call & call);
  //  virtual void Incoming_LIJ(ATM_Call  & call);

  int _num_active;
  int _num_inactive;
  int _total_calls;
};

#endif
