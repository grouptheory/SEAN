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
#ifndef __active_HANGUP_H__
#define __active_HANGUP_H__

#ifndef LINT
static char const _active_hangup_h_rcsid_[] =
"$Id: active_hangup.h,v 1.2 1998/07/29 22:03:28 talmage Exp $";
#endif

#include "BaseClient.h"

class active_hangup : public BaseClient {
public:

  active_hangup(ATM_Interface & interface, double stop_time,
		char *called_party);
  virtual ~active_hangup();

private:

  virtual void PeriodicCallback(int code);
  virtual void Call_Active(ATM_Call   & call);
  virtual void Call_Inactive(ATM_Call & call);

  bool _activated;
};

#endif
