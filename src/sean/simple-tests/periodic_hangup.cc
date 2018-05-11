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
static char const _periodic_hangup_cc_rcsid_[] =
"$Id: periodic_hangup.cc,v 1.14 1998/08/06 04:06:21 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/basics/Conduit.h>
#include <codec/uni_ie/cause.h>
#include "PrintFunctions.h"
#include "AcceptingOneService.h"
#include "periodic_hangup.h"

periodic_hangup::periodic_hangup(ATM_Interface & interface, double stop_time,
				 char *called_party) 
  : BaseClient("periodic_hangup", interface, called_party), 
    _activated(false), _call(0)
{ 
  if (stop_time == -1.0)
    stop_time = 10.0;

  RegisterPeriodicCallback( 666, stop_time );
  // hangup callback
  RegisterPeriodicCallback( 100, 1.0 );
}

periodic_hangup::~periodic_hangup() { }

void periodic_hangup::Call_Active(ATM_Call & call)
{
  diag(_diag_key, DIAG_INFO, 
       "The call has become active ... it will be torndown in next PeriodicCallback\n");
  _call = &call;
  _activated = true;
}

void periodic_hangup::Call_Inactive(ATM_Call & call)
{
  if (_activated)
    diag(_diag_key, DIAG_INFO,
	 "The call has become inactive, the test was a success.  Exiting ...\n");
  else
    diag(_diag_key, DIAG_INFO,
	 "The call has become inactive, the test was a failure.  Exiting ...\n");

  exit( 1 );
}

void periodic_hangup::PeriodicCallback(int code)
{
  if (code == 666) {
    diag(_diag_key, DIAG_INFO, "%s has exited normally, the test has failed.\n", _diag_key);
    exit(0);
  } else if (code == 100) {
    if (!_call || !_activated) return;

    ATM_Attributes::result err = _call->Set_cause(ie_cause::normal_call_clearing);
    err = _call->TearDown();
    
    DIAG(_diag_key, DIAG_INFO, 
	 cout << "call_periodic (Info): ";
	 cout << "Result of call->Teardown: ";
	 PrintResult(err);
	 cout << endl);
    
    ATM_Call::call_status c_status = _call->Status();
    
    DIAG(_diag_key, DIAG_INFO, 
	 cout << "call_periodic (Info): ";
	 cout << "call status:";
	 PrintStatus(c_status);
	 cout << "." << endl);
    // Set the call to 0 so we don't continually try to tear it down...
    _call = 0;
  } else
    diag(_diag_key, DIAG_INFO, "%s received an unsupported code %d.\n", _diag_key, code);
}

void Usage(char *name)
{
  cerr << "usage: " << name
       << " s port [stoptime] [visualizer-pipe]" << endl;
  cerr << "usage: " << name
       << " c port called-party-nsap [stoptime] [visualizer-pipe]" << endl;
}


void main(int argc, char ** argv)
{
  int answer = 0;
  double stop_time = -1.0;
  AcceptingOneService * service = 0;
  periodic_hangup * caller = 0;

  DiagLevel("api", DIAG_DEBUG);
  DiagLevel("periodic_hangup", DIAG_DEBUG);

  if (argc >= 3) {
    int  port = atoi(argv[2]);

    ATM_Interface &host_nic = Open_ATM_Interface("/dev/una0", port);

    switch (argv[1][0]) {
    case 'c':
      //
      // Run as a client
      // 
      if (argc >= 4) {

	if (argc >= 5)
	  stop_time = atof(argv[4]);

	caller = new periodic_hangup(host_nic, stop_time, argv[3]);
	host_nic.Associate_Controller(*caller);

	if (argc >= 6) {
	  VisPipe(argv[5]);
	} else {
	  VisPipe("periodic_hangup_client.output");
	}
      } else {
	Usage(argv[0]);
	answer = 1;
      }
      break;

    case 's':
      //
      // Run as a server
      //
      if (argc >= 4)
	stop_time = atof(argv[3]);

      service = 
	new AcceptingOneService("periodic_hangup", host_nic, stop_time);
      host_nic.Associate_Controller(*service);

      if (argc >= 5) {
	VisPipe(argv[4]);
      } else {
	VisPipe("periodic_hangup_server.output");
      }
      break;

    default:
      Usage(argv[0]);
      answer = 1;
      break;
    }
    
    //
    // Still good?  Then start SEAN.
    //
    if (answer == 0) {
      SEAN_Run();
    }
  } else {
    Usage(argv[0]);
    answer = 1;
  }

  return answer;
}
