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
// File: NService.cc
// Version: $Id: NService.cc,v 1.7 1998/07/27 17:47:34 talmage Exp $
//
// Purpose: Tests the registration and deregistration of a single ATM
// service.  There are five parts to the test.  First, the service
// must be created.  Second, the service must be registered.  Third,
// the service must become active after it is registered.  Fourth, the
// service must become inactive after it is deregistered.  Fifth, and
// finally, when it is time to stop the test, the program must
// deregister each service and receive the acknowledgement for each
// one within another "lifetime" of seconds.
//
// Usage: NService ccd-port [lifetime [visualizer-pipe]] 
//
// where lifetime is the number of seconds to run the test.  The
// default value, -1, means that if some parts of the test are not
// executed, then the program will run forever.


#include <FW/basics/diag.h>

#include "BaseController.h"
#include "NService.h"
#include "PrintFunctions.h"

NService::NService(const char * diag_key, ATM_Interface &interface,
		       double lifetime, int blocksize) :
  BaseController(diag_key, interface),
  _lifetime(lifetime), _blocksize(blocksize), _cleaning(false),
  _inactives(0), _actives(0)
{
}


NService::~NService(void)
{
  _active_services.clear();
}


void NService::PeriodicCallback(int code)
{
  diag(_diag_key, DIAG_INFO, 
       "PeriodicCallback(code=%d)\n", code);

  if (!_cleaning) {
    diag(_diag_key, DIAG_INFO, "Stopping after %f seconds.\n", _lifetime);

    Cleanup();
  } else {
    diag(_diag_key, DIAG_ERROR, "Cleaning isn't finished after %f seconds.\n", 
	 _lifetime);
    exit(1);
  }
}


void NService::Boot(ATM_Interface &interface)
{
  ATM_Attributes::result err;
  ATM_Service *service = 0;

  if (_lifetime >= 0.0) RegisterPeriodicCallback(1, _lifetime);

  for (int i=0; i < _blocksize; i++) {
    diag(_diag_key, DIAG_INFO, 
	 "Creating new service with user bhli=%d\n", i);

    service = NewService(0, i);

    if (service != 0) {
      diag(_diag_key, DIAG_INFO, 
	   "Created a new service #%d.  CREATION TEST PASSED\n", i);

      RegisterService(service);
      _inactives ++;
    } else {
      diag(_diag_key, DIAG_INFO, 
	   "Could not create a new service #%d.  Aborting.  \
CREATION TEST FAILED\n", i);
      exit(1);
    }
  }

  _next_bhli = _blocksize;
}


void NService::Service_Active(ATM_Service& service)
{
  ATM_Attributes::result err;
  ATM_Service *active = &service;

  _inactives --;
  _actives ++;

  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Service Active #";
       PrintBHLIUser(service);
       cout << endl; );

  ReportServiceCount();
  
  diag(_diag_key, DIAG_INFO, "ACTIVE TEST PASSED\n");

  if (!_cleaning) {
    _active_services.push(active);
    int chunks = _actives - _blocksize;

    while (chunks >= 0) {
      chunks --;
      active = _active_services.Pop();

      DeregisterService(active);
      //      _inactives ++;
    }
  } else {
    // Deregister the service that just became active.
    DeregisterService(active);
    //    _inactives ++;
    if ((_inactives == _blocksize) && (_actives == 0)) {
      diag(_diag_key, DIAG_INFO, "Exiting.  TESTS PASSED.\n");
      exit(0);
    }
  }
}


void NService::Service_Inactive(ATM_Service& service)
{
  ATM_Attributes::result err;
  ATM_Service *inactive = 0;

  _actives --;
  _inactives ++;

  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Service Inactive #";
       PrintBHLIUser(service);
       cout << endl; );

  service.Suicide();

  if (!_cleaning) {
    int chunksize = _blocksize - _actives;

    diag(_diag_key, DIAG_INFO, 
	 "Service_Inactive(): blocksize %d actives %d inactives %d\n",
	 _blocksize, _actives, _inactives);

    while (chunksize > 0) {
      chunksize --;
      diag(_diag_key, DIAG_INFO, 
	   "Creating new service with user bhli=%d\n", _next_bhli);

      inactive = 
	NewService(0, _next_bhli);

      if (inactive != 0) {
	diag(_diag_key, DIAG_INFO, 
	     "Created a new service #%d.  CREATION TEST PASSED\n", _next_bhli);

	RegisterService(inactive);
      } else {
	diag(_diag_key, DIAG_INFO, 
	     "Could not create a new service #%d.  Aborting.  \
CREATION TEST FAILED\n", _next_bhli);
	exit(1);
      }
    
      _next_bhli ++;
    }

    ReportServiceCount();

  } else {
    // Unlike in the body of code above, nobody else is counting
    // inactive services.  We do it ourselves.
    //    _inactives ++;
    ReportServiceCount();
    if ((_inactives == _blocksize) && (_actives == 0)) {
      diag(_diag_key, DIAG_INFO, "Exiting.  TESTS PASSED.\n");
      exit(0);
    }
  }
}


void NService::Cleanup(void)
{
  _cleaning = true;

  diag(_diag_key, DIAG_INFO, "Deregistering all of the remaining services.\n");
  diag(_diag_key, DIAG_INFO, "Stopping when there are 0 active \
and %d inactive.\n", _blocksize);

  while (_active_services.size() > 0) {
    ATM_Service *active = _active_services.pop();

    DeregisterService(active);
  }

  
}


void NService::ReportServiceCount(void)
{
  diag(_diag_key, DIAG_INFO, "There are %d active services\n",
       _actives);
  diag(_diag_key, DIAG_INFO, "There are %d inactive services\n",
       _inactives);
}


