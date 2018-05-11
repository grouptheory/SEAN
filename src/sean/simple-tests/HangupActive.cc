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
// File: HangupActive.cc
// Version: $Id: HangupActive.cc,v 1.3 1998/07/27 17:48:00 talmage Exp $
//
// Purpose: Implements the controller for a service that never accepts
// incoming calls.  The controller tears down the first incoming call
// it sees.  When the call goes inactive, the controller deregisters
// the service.  When the service goes inactive, the controller exits.
// If the lifetime expires before all of these things happen, the
// controller exits with a failing grade.
//
// Usage: HangupActive ccd-port lifetime [visualizer-pipe]
//
// where lifetime is the number of seconds to run the test.

#include <FW/basics/diag.h>

#include "HangupActive.h"
#include "PrintFunctions.h"

HangupActive::HangupActive(const char * diag_key, ATM_Interface &interface,
			 double lifetime):
  NeverAccept(diag_key, interface, lifetime),
  _passed_call_teardown_test(false),
  _passed_call_active_test(false)
{
}


HangupActive::~HangupActive(void)
{
}


void HangupActive::Boot(ATM_Interface &interface)
{
  ATM_Attributes::result err;

  if (_lifetime >= 0.0) RegisterPeriodicCallback(1, _lifetime);

  diag(_diag_key, DIAG_INFO, 
       "Creating new service with user bhli=HANGACT\n");

  _service = NewService(0, "HANGACT");

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
	 "Could not create a new service #HANGACT.  Aborting.  \
CREATION TEST FAILED\n");
    exit(1);
  }
}


void HangupActive::Service_Inactive(ATM_Service& service)
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
      _passed_active_test && 
      _passed_inactive_test &&
      _passed_incoming_call_test && 
      _passed_call_active_test &&
      _passed_call_teardown_test &&
      _passed_call_inactive_test) {
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
	 if (!_passed_call_active_test) cout << "CALL ACTIVE, ";
	 if (!_passed_call_teardown_test) cout << "CALL TEARDOWN, ";
	 if (!_passed_call_inactive_test) cout << "CALL INACTIVE, ";
	 cout << endl; );
    exit(1);
  }
}


void HangupActive::Incoming_Call(ATM_Service& service) {
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
	 "Accepting the call\n");

    err = call->Accept();

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



void HangupActive::Call_Active(ATM_Call& call)
{
  ATM_Attributes::result err;

  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Call Active #";
       PrintBHLIUser(call);
       cout << endl; );

  diag(_diag_key, DIAG_INFO, "CALL ACTIVE TEST PASSED\n");

  _passed_call_active_test = true;


  diag(_diag_key, DIAG_INFO, "Hanging up on the call\n");

  err = call.Set_cause(ie_cause::normal_call_clearing);

  if (err != ATM_Attributes::success) {
    diag(_diag_key, DIAG_INFO, "SET CAUSE TEST FAILED\n");
    exit(1);
  }

  err = call.TearDown();

  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Result of TearDown(): ";
       PrintResult(err);
       cout << endl; );

  if (err == ATM_Attributes::success) {
    diag(_diag_key, DIAG_INFO, "CALL TEARDOWN TEST PASSED\n");
    _passed_call_teardown_test = true;
  } else {
    diag(_diag_key, DIAG_INFO, "CALL TEARDOWN TEST FAILED\n");
    exit(1);
  }

}



