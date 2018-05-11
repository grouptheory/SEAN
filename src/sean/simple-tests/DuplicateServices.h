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
// File: DuplicateServices.h
//
#ifndef __DuplicateServices_H__
#define __DuplicateServices_H__

#ifndef LINT
static char const _DuplicateServices_h_rcsid_[] =
"$Id: DuplicateServices.h,v 1.2 1998/07/27 17:48:04 talmage Exp $";
#endif

#include <sean/api/ATM_Interface.h>
#include <sean/api/Call_API.h>
#include <sean/api/Service_API.h>

#include <sean/simple-tests/BaseController.h>

class DuplicateServices : public BaseController {
public:

  DuplicateServices(const char * diag_key, ATM_Interface &interface, 
		    double lifetime);
  virtual ~DuplicateServices(void);

private:

  virtual void PeriodicCallback(int code);
  virtual void Boot(ATM_Interface &interface);

  virtual void Service_Active(ATM_Service& service);
  virtual void Service_Inactive(ATM_Service& service);

  ATM_Service *_service1;	/* This one should go active after
				 * registration.
				 */

  ATM_Service *_service2;	/* This one should go inactive after
				 * registration.
				 */

  double _lifetime;		// Max number of seconds to run the test.

  bool _service1_active;	/* Set this true when _service1 goes
				 * active.  We expect this to happen
				 * after _service1 is registered.
				 */

  bool _service1_inactive;	/* Set this true when _service1 goes
				 * inactive.  We expect this to
				 * happend after _service1 is
				 * deregistered.  
				 */

  bool _service2_active;	/* Set this true when _service2 goes
				 * active.  We don't expect this to
				 * happen ever.  
				 */

  bool _service2_inactive;	/* Set this true when _service2 goes
				 * inactive.  We expect this to happen
				 * after _service2 is registered. 
				 */
};

#endif
