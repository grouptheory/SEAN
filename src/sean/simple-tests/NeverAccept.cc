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
// File: NeverAccept.cc
// Version: $Id: NeverAccept.cc,v 1.6 1998/07/27 17:47:43 talmage Exp $
//
// Purpose: Implements the controller for a service that never accepts
// incoming calls.  The controller tears down the first incoming call
// it sees.  When the call goes inactive, the controller deregisters
// the service.  When the service goes inactive, the controller exits.
// If the lifetime expires before all of these things happen, the
// controller exits with a failing grade.
//
// Usage: NeverAccept ccd-port lifetime [visualizer-pipe]
//
// where lifetime is the number of seconds to run the test.

#include <FW/basics/diag.h>

#include "NeverAccept.h"
#include "PrintFunctions.h"

NeverAccept::NeverAccept(const char * diag_key, ATM_Interface &interface,
			 double lifetime):
  BaseController(diag_key, interface),
  _lifetime(lifetime), _cleaning(false),
  _passed_creation_test(false),
  _passed_active_test(false), _passed_inactive_test(false),
  _passed_incoming_call_test(false), _passed_call_inactive_test(false)
{
}


NeverAccept::~NeverAccept(void)
{
}


void NeverAccept::PeriodicCallback(int code)
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


void NeverAccept::Boot(ATM_Interface &interface)
{
  ATM_Attributes::result err;

  if (_lifetime >= 0.0) RegisterPeriodicCallback(1, _lifetime);

  diag(_diag_key, DIAG_INFO, 
       "Creating new service with user bhli=NEVER\n");

  _service = NewService(0, "NEVER");

  if (_service != 0) {
    DIAG(_diag_key, DIAG_INFO, 
	 cout << _diag_key << " (Info): ";
	 cout << "Created a new service #";
	 PrintBHLIUser(*_service);
	 cout << " CREATION TEST PASSED";
	 cout << endl; );

    RegisterService(_service);
    _passed_creation_test = true;
  } else {
    diag(_diag_key, DIAG_INFO, 
	 "Could not create a new service #NEVER.  Aborting.  \
CREATION TEST FAILED\n");
    exit(1);
  }
}


void NeverAccept::Service_Active(ATM_Service& service)
{
  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Service Active #";
       PrintBHLIUser(service);
       cout << endl; );

  diag(_diag_key, DIAG_INFO, "ACTIVE TEST PASSED\n");
  _passed_active_test = true;
}


void NeverAccept::Service_Inactive(ATM_Service& service)
{
  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Service Inactive #";
       PrintBHLIUser(service);
       cout << endl; );

  service.Suicide();

  diag(_diag_key, DIAG_INFO, "INACTIVE TEST PASSED\n");

  _passed_inactive_test = true;

  if (_passed_creation_test &&
      _passed_active_test && _passed_inactive_test &&
      _passed_incoming_call_test && _passed_call_inactive_test) {
    diag(_diag_key, DIAG_INFO, "TEST PASSED\n");
    exit(0);
  } else {
    DIAG(_diag_key, DIAG_INFO, 
	 cout << _diag_key << " (Info): ";
	 cout << "TEST FAILED because of ";
	 if (!_passed_creation_test) cout << "CREATION, ";
	 if (!_passed_active_test) cout << "ACTIVE, ";
	 if (!_passed_inactive_test) cout << "INACTIVE, ";
	 if (!_passed_incoming_call_test) cout << "INCOMING CALL, ";
	 if (!_passed_call_inactive_test) cout << "CALL INACTIVE, ";
	 cout << endl; );
    exit(1);
  }
}


void NeverAccept::Incoming_Call(ATM_Service& service) {
  ATM_Attributes::result err;

  DIAG(_diag_key, DIAG_INFO, 
       cout << endl;
       cout << _diag_key << " (Info): ";
       cout << "Incoming Call to BHLI #";
       PrintBHLIUser(service);
       cout << endl; );

  diag(_diag_key, DIAG_INFO,
       "Getting the incoming call\n");

  diag(_diag_key, DIAG_INFO, "Getting incoming call\n");

  Incoming_ATM_Call *call = service.Get_Incoming_Call();

  if (call != 0) {

    call->Associate_Controller(*this);

    diag(_diag_key, DIAG_INFO, 
	 "Tearing down the call\n");

    err = call->Set_cause(ie_cause::normal_call_clearing);

    if (err != ATM_Attributes::success) {
      diag(_diag_key, DIAG_INFO, "SET CAUSE TEST FAILED\n");
      exit(1);
    }

    err = call->TearDown();

    DIAG(_diag_key, DIAG_INFO, 
	 cout << _diag_key << " (Info): ";
	 PrintResult(err);
	 cout << endl; );

    if (err == ATM_Attributes::success) {
      diag(_diag_key, DIAG_INFO, " GET INCOMING CALL TEST PASSED\n");
      _passed_incoming_call_test = true;
    } else {
      diag(_diag_key, DIAG_INFO, " GET INCOMING CALL TEST FAILED\n");
      exit(1);
    }
  } else {
    diag(_diag_key, DIAG_INFO, " GET INCOMING CALL TEST FAILED\n");
    exit(1);
  }

}



void NeverAccept::Call_Inactive(ATM_Call& call)
{
  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Call Inactive #";
       PrintBHLIUser(call);
       cout << endl; );

  call.Suicide();

  diag(_diag_key, DIAG_INFO, "CALL INACTIVE TEST PASSED\n");

  _passed_call_inactive_test = true;
  Cleanup();
}


void NeverAccept::Cleanup(void)
{
  _cleaning = true;
  _service->Deregister();
}




