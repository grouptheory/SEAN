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
#ifndef LINT
static char const _AcceptingOneService_cc_rcsid_[] =
"$Id: AcceptingOneService.cc,v 1.7 1998/08/06 04:06:15 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/basics/Conduit.h>
#include <FW/basics/diag.h>

#include "BaseController.h"
#include "AcceptingOneService.h"
#include "PrintFunctions.h"

AcceptingOneService::AcceptingOneService(const char * diag_key, 
					 ATM_Interface & interface,
					 double lifetime) :
  BaseController(diag_key, interface), _service(0), _accepted_call(false)
{
  RegisterPeriodicCallback( 666, lifetime );
}

AcceptingOneService::~AcceptingOneService(void)
{
  _service->Suicide();
}

void AcceptingOneService::PeriodicCallback(int code)
{
  if ( code == 666 ) {
    if (_accepted_call)
      diag(_diag_key, DIAG_INFO, "Program is exiting normally, the test was a success.\n");
    else
      diag(_diag_key, DIAG_INFO, "Program is exiting normally, the test was a failure.\n");

    exit( 1 );
  } else
    diag(_diag_key, DIAG_INFO, "%s received an unsupported callback with code %d.\n", _diag_key, code);
}

void AcceptingOneService::Boot(ATM_Interface &interface)
{
  ATM_Attributes::result err;

  diag(_diag_key, DIAG_INFO, 
       "Creating new service with user bhli = %s\n", _diag_key);

  _service = NewService(0, _diag_key);

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

void AcceptingOneService::Call_Active(ATM_Call& call) {
  DIAG(_diag_key, DIAG_INFO, 
       cout << endl;
       cout << "Call Active to BHLI #";
       PrintBHLIUser(call);
       cout << endl; );
}



void AcceptingOneService::Call_Inactive(ATM_Call& call) {
  DIAG(_diag_key, DIAG_INFO, 
       cout << endl;
       cout << "Call Inactive to BHLI #";
       PrintBHLIUser(call);
       cout << endl; );

  call.Suicide();
}


void AcceptingOneService::Service_Active(ATM_Service & service)
{
  ATM_Attributes::result err;

  DIAG(_diag_key, DIAG_INFO, 
       cout << endl;
       cout << _diag_key << " (Info): ";
       cout << "Service Active #";
       PrintBHLIUser(service);
       cout << endl; );

  if (_service == &service) {
    diag(_diag_key, DIAG_INFO, "Service has been registered and become active.\n");
  } else {
    diag(_diag_key, DIAG_INFO, 
	 "It's NOT the same one I registered in Boot(). ACTIVE TEST FAILED\n");
    exit(1);
  } 
}

void AcceptingOneService::Service_Inactive(ATM_Service& service)
{
  ATM_Attributes::result err;

  DIAG(_diag_key, DIAG_INFO, 
       cout << endl;
       cout << _diag_key << " (Info): ";
       cout << "Service Inactive #";
       PrintBHLIUser(service);
       cout << endl; );

  if (_service == &service) {
    diag(_diag_key, DIAG_INFO, "Service has been registered and become inactive.\n");
  }
}

void AcceptingOneService::Incoming_Call(ATM_Service & service)
{
  ATM_Attributes::result err;

  DIAG(_diag_key, DIAG_INFO, 
       cout << endl << "AcceptingOneService (Info): "
            << "Incoming call on Service #";
       PrintBHLIUser(service);
       cout << endl);

  diag(_diag_key, DIAG_INFO, "About to Get_Incoming_Call()\n");

  Incoming_ATM_Call * c = service.Get_Incoming_Call();

  if (c) {
    diag(_diag_key, DIAG_INFO, 
         "Associating the call with this controller.\n");

    if (c->Associate_Controller(*this)) {
      diag(_diag_key, DIAG_INFO, 
           "Associate_Controller() Succeeded!\n");

      DIAG(_diag_key, DIAG_INFO, 
           cout << "AcceptingOneService (Info): "
                <<  "beginning establish on incoming call..."
                << c << endl);

      diag(_diag_key, DIAG_INFO, "About to accept the call\n");

      err = c->Accept();

      DIAG(_diag_key, DIAG_INFO, 
           cout << "AcceptingOneService (Info): ";
           PrintResult(err);
           cout << endl);

      diag(_diag_key, DIAG_INFO, 
           "establish of incoming call complete\n");
    } else {
      diag(_diag_key, DIAG_INFO, "Associate_Controller() failed\n");
    }
  }  else {
    diag(_diag_key, DIAG_INFO, "call ptr is 0\n");
  }

  _accepted_call = true;
}
