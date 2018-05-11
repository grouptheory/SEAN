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
// File: NService.h
//
#ifndef __NSERVICE_H__
#define __NSERVICE_H__

#ifndef LINT
static char const _NService_h_rcsid_[] =
"$Id: NService.h,v 1.4 1998/07/27 17:47:30 talmage Exp $";
#endif

#include <sean/api/ATM_Interface.h>
#include <sean/api/Call_API.h>
#include <sean/api/Service_API.h>

#include <sean/simple-tests/BaseController.h>

class NService : public BaseController {
public:

  NService(const char * diag_key, ATM_Interface &interface, double lifetime,
	   int blocksize);
  virtual ~NService(void);

private:

  virtual void PeriodicCallback(int code);
  virtual void Boot(ATM_Interface &interface);

  virtual void Service_Active(ATM_Service& service);
  virtual void Service_Inactive(ATM_Service& service);

  virtual void Cleanup(void);
  virtual void ReportServiceCount(void);

  /* List of services that are currently active.  There can be no more
   * than _blocksize services in the list.  If the list has fewer than
   * _blocksize entries, then we add more in the Service_Inactive()
   * callback.*/
  list<ATM_Service *>_active_services;

  double _lifetime;	// Max number of seconds to run the test.

  int _blocksize;	/* Create and register this many services at a
			 * time */

  int _next_bhli;	// The high water mark of user bhlis.

  int _inactives;	/* Number of services that are waiting
			 * to become active.
			 */

  int _actives;		/* Number of services that are active.
			 */

  bool _cleaning;	/* true when we're cleaning up after ourselves.
			 * In that case, we deregister all active 
			 * services.  When the number if active
			 * services is zero, we exit(0).
			 */
 };

#endif
