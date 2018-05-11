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
// File: PersistantService.cc
// Version: $Id: PersistentService.cc,v 1.1 1998/07/24 21:52:28 talmage Exp $
//
// Purpose: Tests the registration and deregistration of a single ATM
// service.  There are four parts to the test.  First, the service
// must be created.  Second, the service must be registered.  Third,
// the service must become active after it is registered.  Fourth, the
// service must become inactive after it is deregistered.
//
// Usage: PersistantService ccd-port [lifetime [visualizer-pipe]] 
//
// where lifetime is the number of seconds to run the test.  The
// default value, -1, means that if some parts of the test are not
// executed, then the program will run forever.


#include <FW/basics/diag.h>

#include "BaseController.h"
#include "PersistantService.h"
#include "PrintFunctions.h"

PersistantService::PersistantService(const char * diag_key, 
				     ATM_Interface &interface,
				     double lifetime) :
  BaseController(diag_key, interface), 
  _lifetime(lifetime)
{
}


PersistantService::~PersistantService(void)
{
}


void PersistantService::PeriodicCallback(int code)
{
  diag(_diag_key, DIAG_INFO, 
       "PeriodicCallback(code=%d)\n", code);

  diag(_diag_key, DIAG_INFO, "Stopping after %f seconds.\n", _lifetime);
  exit(0);
}


void PersistantService::Boot(void)
{
  ATM_Attributes::result err;

  if (_lifetime >= 0.0) RegisterPeriodicCallback(1, _lifetime);

  diag(_diag_key, DIAG_INFO, 
       "Creating new service with user bhli=STUBBORN\n");

  _service = NewService(0, "STUBBORN");

  if (_service != 0) {
    diag(_diag_key, DIAG_INFO, 
	 "Created a new service.  CREATION TEST PASSED\n");

    DIAG(_diag_key, DIAG_INFO, 
	 cout << _diag_key << " (Info): ";
	 cout << "Registering the service";
	 cout << endl; );

    err = _service->Register();

    DIAG(_diag_key, DIAG_INFO, 
	 cout << _diag_key << " (Info): ";
	 cout << "Result of Register(): ";
	 PrintResult(err);
	 cout << endl; );

    if (err == ATM_Attributes::success) {
      diag(_diag_key, DIAG_INFO, 
	   "Register() succeeded.  REGISTER TEST PASSED\n");
    } else {
      diag(_diag_key, DIAG_INFO, 
	   "Register() failed.  Aborting.  REGISTER TEST FAILED\n");
      exit(1);
    }
  } else {
    diag(_diag_key, DIAG_INFO, 
	 "Could not create a new service.  Aborting.  CREATION TEST FAILED\n");
    exit(1);
  }
}


void PersistantService::Service_Active(ATM_Service& service)
{
  ATM_Attributes::result err;

  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Service Active #";
       PrintBHLIUser(service);
       cout << endl; );

  diag(_diag_key, DIAG_INFO, "Registering a duplicate service\n");

  ATM_Service *duplicate_service =
    NewService(0, "STUBBORN");

  err = duplicate_service->Deregister();
  
  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Result of Deregister(): ";
       PrintResult(err);
       cout << endl; );
}


void PersistantService::Service_Inactive(ATM_Service& service)
{
  ATM_Attributes::result err;

  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Service Inactive #";
       PrintBHLIUser(service);
       cout << endl; );

  if (_service == &service) {
    diag(_diag_key, DIAG_INFO, 
	 "It's the same persistant I registered in Boot().  INACTIVE TEST PASSED\n");
    _inactive_test_passed = true;
  } else {
    diag(_diag_key, DIAG_INFO, 
	 "It's NOT the same persistant I registered in Boot().  \
INACTIVE TEST FAILED\n");
  }

  if (_active_test_passed && _inactive_test_passed) {
    diag(_diag_key, DIAG_INFO, "TEST %s PASSED\n", _diag_key); 
    exit(0);
  }
  else {
    diag(_diag_key, DIAG_INFO, "TEST %s FAILED\n", _diag_key); 
    exit(1);
  }
}
