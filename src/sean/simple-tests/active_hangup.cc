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
static char const _active_hangup_cc_rcsid_[] =
"$Id: active_hangup.cc,v 1.13 1998/08/06 04:06:20 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/basics/Conduit.h>
#include <codec/uni_ie/cause.h>
#include <sean/daemon/HostFile.h>
#include "active_hangup.h"
#include "PrintFunctions.h"
#include "AcceptingOneService.h"

active_hangup::active_hangup(ATM_Interface & interface, double stop_time,
			     char *called_party) 
  : BaseClient("active_hangup", interface, called_party), _activated(false)
{ 
  if (stop_time == -1.0)
    stop_time = 10.0;

  RegisterPeriodicCallback( 666, stop_time );
}

active_hangup::~active_hangup() { }

void active_hangup::Call_Active(ATM_Call & call)
{
  diag(_diag_key, DIAG_INFO, 
       "The call has become active, tearing down the call ...\n");
  
  ATM_Attributes::result err = call.Set_cause(ie_cause::normal_call_clearing);
  err = call.TearDown();
  
  DIAG(_diag_key, DIAG_INFO, 
       cout << "call_periodic (Info): ";
       cout << "Result of call->Teardown: ";
       PrintResult(err);
       cout << endl);
  
  ATM_Call::call_status c_status = call.Status();
  
  DIAG(_diag_key, DIAG_INFO, 
       cout << "call_periodic (Info): ";
       cout << "call status:";
       PrintStatus(c_status);
       cout << "." << endl);

  _activated = true;
}

void active_hangup::Call_Inactive(ATM_Call & call)
{
  if (_activated)
    diag(_diag_key, DIAG_INFO,
	 "The call has become inactive, the test was a success.  Exiting ...\n");
  else
    diag(_diag_key, DIAG_INFO,
	 "The call has become inactive without ever becoming active, the test was a failure.  Exiting ...\n");

  exit( 1 );
}

void active_hangup::PeriodicCallback(int code)
{
  if (code == 666) {
    diag(_diag_key, DIAG_INFO, "Time has run out, the test has failed.\n", _diag_key);
    exit(0);
  } else
    diag(_diag_key, DIAG_INFO, "%s received an unsupported code %d.\n", _diag_key, code);
}

void Usage(char *name)
{
  cerr << "usage: " << name
       << " s my-name [host-file [stoptime [visualizer-pipe]]]" << endl;
  cerr << "usage: " << name
       << " c my-name server-name [host-file [stoptime [visualizer-pipe]]]" 
       << endl;
}


int StartClient(int argc, char ** argv)
{
  int answer = 0;
  double stop_time = -1.0;

  if (argc >= 4) {

    char * host_file_name = "../daemon/host.file";

    if ((argc >= 5) && (strlen(argv[4]) > 0))
      host_file_name = argv[4];

    if (argc >= 6)
      stop_time = atof(argv[5]);

    HostFile name_service(host_file_name);
    int port = name_service.get_port_byname(argv[2]);

    if (port > 0) {
      ATM_Interface &interface = Open_ATM_Interface("/dev/una0", port);
      active_hangup * caller = 0;
      char *called_party = 0;

      called_party = name_service.get_nsap_byname(argv[3]);

      if (called_party != 0) {
	cout << "called party name = " << argv[3] 
	     << " ATM Address  = " << called_party << endl;

	caller = new active_hangup(interface, stop_time, called_party);

	// This might crash filed.  The NSAP that active_hangup() builds
	// from argv[3] will be zero.
	//	caller = new active_hangup(interface, stop_time, argv[3]);
	interface.Associate_Controller(*caller);

	if (argc >= 7) {
	  VisPipe(argv[6]);
	} else {
	  VisPipe("active_hangup_client.output");
	}

	SEAN_Run();

      } else {
	cout << argv[0] << ": Can't find host " << argv[3] << endl;
	answer = 1;
      }
    } else {
      cout << argv[0] << ": Can't find host " << argv[2] 
	   << " or can't find port file" << endl;
      answer = 1;
    }
  } else {
    Usage(argv[0]);
    answer = 1;
  } 
  return answer;
}


int StartServer(int argc, char ** argv)
{
  int answer = 0;
  double stop_time = 10.0;
  char * host_file_name = "../daemon/host.file";

  if ((argc >= 4) && (strlen(argv[3]) > 0))
    host_file_name = argv[3];

  if (argc >= 5)
    stop_time = atof(argv[4]);

  HostFile name_service(host_file_name);
  int port = name_service.get_port_byname(argv[2]);

  if (port > 0) {

    ATM_Interface &interface = Open_ATM_Interface("/dev/una0", port);
    AcceptingOneService * service = 
      new AcceptingOneService("active_hangup", interface, stop_time);

    interface.Associate_Controller(*service);

    if (argc >= 6) {
      VisPipe(argv[5]);
    } else {
      VisPipe("active_hangup_server.output");
    }

    SEAN_Run();

  } else {
    cout << argv[0] << ": Can't find host " << argv[2]
	 << " or can't find port file" << endl;
    answer = 1;
  }

  return answer;
}

int main(int argc, char **argv)
{
  int answer = 0;

  DiagLevel("api", DIAG_DEBUG);
  DiagLevel("active_hangup", DIAG_DEBUG);
  DiagLevel("ipc", DIAG_ENV);


  if (argc >= 3) {
    if (argv[1][0] == 'c') answer = StartClient(argc, argv);
    else if (argv[1][0] == 's') answer = StartServer(argc, argv);
    else { 
      Usage(argv[0]);
      answer = 1;
    }
  } else {
    Usage(argv[0]);
    answer = 1;
  }

  return answer;
}
