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
// File: DuplicateServices.cc
// Version: $Id: DuplicateServices.cc,v 1.3 1998/07/27 17:48:08 talmage Exp $
//
// Purpose: Tests the registration of two, identical services.  The
// first one must become active and the second one must become
// inactive.  When the first one becomes active, it Service_Active()
// deregisters it.  The first one must then become inactive.  After a
// lifetime of seconds, the program ends.  If all three conditions
// have been met then the teset succeeded.  Otherwise, the test
// failed.
//
// Usage: DuplicateServices ccd-port lifetime [visualizer-pipe]
//
// where lifetime is the number of seconds to run the test.
//

#include <FW/basics/diag.h>

#include "DuplicateServices.h"
#include "PrintFunctions.h"

DuplicateServices::DuplicateServices(const char * diag_key, 
				     ATM_Interface &interface,
				     double lifetime) :
  BaseController(diag_key, interface), 
  _lifetime(lifetime), 
  _service1(0), _service1_active(false), _service1_inactive(false),
  _service2(0), _service2_active(false), _service2_inactive(false)
{
}


DuplicateServices::~DuplicateServices(void)
{
  if (_service1 != 0)
    _service1->Suicide();

  if (_service2 != 0)
    _service2->Suicide();
}


void DuplicateServices::PeriodicCallback(int code)
{
  diag(_diag_key, DIAG_INFO, 
       "PeriodicCallback(code=%d)\n", code);

  diag(_diag_key, DIAG_INFO, "Stopping after %f seconds.\n", _lifetime);

  if (_service1_active && _service1_inactive && _service2_inactive &&
      !_service2_active) {
    diag(_diag_key, DIAG_INFO, "TEST PASSED\n");
    exit(0); 
  } else {
    diag(_diag_key, DIAG_INFO, "TEST FAILED\n");
    exit(1);
  }
}


void DuplicateServices::Boot(ATM_Interface &interface)
{
  ATM_Attributes::result err;

  if (_lifetime >= 0.0) RegisterPeriodicCallback(1, _lifetime);

  diag(_diag_key, DIAG_INFO, 
       "Creating 2 new services with user bhli=DUPE\n");

  _service1 = NewService(0, "DUPE");

  _service2 = NewService(0, "DUPE");

  if ((_service1 != 0) && (_service2 != 0)) {
    diag(_diag_key, DIAG_INFO, 
	 "Created 2 new services.  CREATION TEST PASSED\n");


    RegisterService(_service1);
    RegisterService(_service2);
  } else {
    diag(_diag_key, DIAG_INFO, 
	 "Could not create 2 new services.  Aborting.  \
CREATION TEST FAILED\n");
    exit(1);
  }
}


void DuplicateServices::Service_Active(ATM_Service& service)
{
  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Service Active #";
       PrintBHLIUser(service);
       cout << endl; );

  if (_service1 == &service) {

    diag(_diag_key, DIAG_INFO, "service1 is active\n");
    _service1_active = true;
    DeregisterService(&service);

  } else if (_service2 == &service) {

    diag(_diag_key, DIAG_INFO, "service2 is active\n");
    _service2_active = true;
    DeregisterService(&service);

  }
  else {

    diag(_diag_key, DIAG_INFO, "Unknown service is active.  TEST FAILED\n");
    exit(1);

  }
}


void DuplicateServices::Service_Inactive(ATM_Service& service)
{
  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Service Inactive #";
       PrintBHLIUser(service);
       cout << endl; );

  if (_service1 == &service) {

    diag(_diag_key, DIAG_INFO, "service1 is inactive\n");
    _service1_inactive = true;
    
  } else if (_service2 == &service) {

    diag(_diag_key, DIAG_INFO, "service2 is inactive\n");
    _service2_inactive = true;
  }
  else {

    diag(_diag_key, DIAG_INFO, "Unknown service is active.  TEST FAILED\n");
    exit(1);

  }
}
