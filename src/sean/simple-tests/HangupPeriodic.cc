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
// File: HangupPeriodic.cc
// Version: $Id: HangupPeriodic.cc,v 1.5 1998/07/27 17:47:51 talmage Exp $
//
// Purpose: Implements the controller for a service that regularly
// hangs up on the calls that it accepts.  Each call must go thought
// the usual life cycle of incoming call, call active, call inactive.
//
// Usage: HangupPeriodic ccd-port period [lifetime [visualizer-pipe]]
//
// where period is how often to hangup a call.
//       lifetime is the number of seconds to run the test.

#include <FW/basics/diag.h>

#include "HangupPeriodic.h"
#include "PrintFunctions.h"

HangupPeriodic::HangupPeriodic(const char * diag_key, ATM_Interface &interface,
			       double period, double lifetime):
  HangupActive(diag_key, interface, lifetime),
  _period(period), _incoming_call(0)
{
}


HangupPeriodic::~HangupPeriodic(void)
{
}


void HangupPeriodic::PeriodicCallback(int code)
{
  ATM_Attributes::result err;
  
  diag(_diag_key, DIAG_INFO, 
      "BaseController::PeriodicCallback(code=%d)\n", code);

  switch (code) {

  case 1:
    if (_incoming_call != 0) {
      diag(_diag_key, DIAG_INFO, "Hanging up on the call after %f seconds\n",
	   _period);

      err = _incoming_call->Set_cause(ie_cause::normal_call_clearing);

      if (err != ATM_Attributes::success) {
	diag(_diag_key, DIAG_INFO, "SET CAUSE TEST FAILED\n");
	exit(1);
      }

      err = _incoming_call->TearDown();

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

      _incoming_call = 0;
    }
    break;

  case 2:
    diag(_diag_key, DIAG_INFO, "Stopping up after %f seconds.\n", 
	 _lifetime);
    Cleanup();
    break;

  default:
    break;
  }
}


void HangupPeriodic::Boot(ATM_Interface &interface)
{
  ATM_Attributes::result err;

  // Hangup any call every _period seconds
  RegisterPeriodicCallback(1, _period);

  // Exit after _lifetime seconds
  RegisterPeriodicCallback(2, _lifetime);

  diag(_diag_key, DIAG_INFO, 
       "Creating new service with user bhli=HANGPER\n");

  _service = NewService(0, "HANGPER");

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
	 "Could not create a new service #HANGPER.  Aborting.  \
CREATION TEST FAILED\n");
    exit(1);
  }
}


void HangupPeriodic::Incoming_Call(ATM_Service& service) {
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

  _incoming_call = service.Get_Incoming_Call();

  if (_incoming_call != 0) {

    _incoming_call->Associate_Controller(*this);

    diag(_diag_key, DIAG_INFO, 
	 "Accepting the call\n");

    err = _incoming_call->Accept();

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



void HangupPeriodic::Call_Active(ATM_Call& call)
{
  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Call Active #";
       PrintBHLIUser(call);
       cout << endl; );

  diag(_diag_key, DIAG_INFO, "CALL ACTIVE TEST PASSED\n");

  _passed_call_active_test = true;
}


void HangupPeriodic::Call_Inactive(ATM_Call& call)
{
  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Call Inactive #";
       PrintBHLIUser(call);
       cout << endl; );

  //  call.Suicide();

  diag(_diag_key, DIAG_INFO, "CALL INACTIVE TEST PASSED\n");

  _passed_call_inactive_test = true;
}


