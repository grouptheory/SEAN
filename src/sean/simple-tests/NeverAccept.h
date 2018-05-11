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
// File: NeverAccept.h
//
#ifndef __NeverAccept_H__
#define __NeverAccept_H__

#ifndef LINT
static char const _NeverAccept_h_rcsid_[] =
"$Id: NeverAccept.h,v 1.3 1998/07/27 17:47:39 talmage Exp $";
#endif

#include <sean/api/ATM_Interface.h>
#include <sean/api/Call_API.h>
#include <sean/api/Service_API.h>

#include <sean/simple-tests/BaseController.h>

class NeverAccept : public BaseController {
public:

  NeverAccept(const char * diag_key, ATM_Interface &interface, 
	      double lifetime);
  virtual ~NeverAccept(void);

protected:

  virtual void PeriodicCallback(int code);
  virtual void Boot(ATM_Interface &interface);

  virtual void Call_Inactive(ATM_Call& call);

  virtual void Service_Active(ATM_Service& service);
  virtual void Service_Inactive(ATM_Service& service);
  virtual void Incoming_Call(ATM_Service& service);

  virtual void Cleanup(void);

  ATM_Service *_service;	// Register it.  Don't accept any calls.

  double _lifetime;	// Max number of seconds to run the test.

  bool _cleaning;	/* true when we're cleaning up after ourselves.
			 * In that case, we deregister all active 
			 * services.  When the number if active
			 * services is zero, we exit(0).
			 */
  /* Set these true when we pass the appropriate test */
  bool _passed_creation_test;	// Boot()
  bool _passed_active_test;	// Service_Active()
  bool _passed_inactive_test;	// Service_Inactive()
  bool _passed_incoming_call_test;	// Incoming_Call()
  bool _passed_call_inactive_test;	// Call_Inactive()
 };

#endif
