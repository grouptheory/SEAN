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
// File: OneService.h
//
#ifndef __ONESERVICE_H__
#define __ONESERVICE_H__

#ifndef LINT
static char const _OneService_h_rcsid_[] =
"$Id: OneService.h,v 1.3 1998/07/27 17:49:06 talmage Exp $";
#endif

#include <sean/api/ATM_Interface.h>
#include <sean/api/Call_API.h>
#include <sean/api/Service_API.h>

#include <sean/simple-tests/BaseController.h>

class OneService : public BaseController {
public:

  OneService(const char * diag_key, ATM_Interface &interface, double lifetime);
  virtual ~OneService(void);

private:

  virtual void PeriodicCallback(int code);
  virtual void Boot(ATM_Interface &interface);

  virtual void Service_Active(ATM_Service& service);
  virtual void Service_Inactive(ATM_Service& service);

  ATM_Service *_service;	/* We create, register, and monitor a
				 * service.
				 */

  bool _active_test_passed;	/* True if the service was registered and
				 * it went active.
				 */

  bool _inactive_test_passed;	/* True if the _active_test_passed and the
				 * service went inactive.
				 */

  double _lifetime;		// Max number of seconds to run the test.
};

#endif
