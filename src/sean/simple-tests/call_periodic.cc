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
static char const _call_periodic_cc_rcsid_[] =
"$Id: call_periodic.cc,v 1.17 1998/08/06 04:06:21 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/basics/Conduit.h>
#include <sean/daemon/HostFile.h>
#include "call_periodic.h"
#include "PrintFunctions.h"

call_periodic::call_periodic(ATM_Interface & interface, 
			     double period, double stop_time,
			     char *called_party, char *bhli) 
  : BaseClient("call_periodic", interface, called_party, bhli), 
    _call_is_active(0), _total_calls(1), _active_calls(0), _inactive_calls(0)
{ 
  if (stop_time == -1.0)
    stop_time = 10.0;
  if (period == -1.0)
    period = 1.0;

  RegisterPeriodicCallback( 666, stop_time );
  RegisterPeriodicCallback( 100, period );
}

call_periodic::~call_periodic() { }

void call_periodic::Call_Active(ATM_Call   & call)
{
  diag(_diag_key, DIAG_INFO, 
       "The call has become active ...\n");
  _call = (Outgoing_ATM_Call *)&call;
  _call_is_active = 1;
  _active_calls++;
}

void call_periodic::Call_Inactive(ATM_Call & call)
{
  diag(_diag_key, DIAG_INFO,
       "The call has become inactive ...\n");
  _inactive_calls++;
  // delete (Outgoing_ATM_Call *)(&call);
  _call_is_active = -1;
}

void call_periodic::PeriodicCallback(int code)
{
  if (code == 666) {
    diag(_diag_key, DIAG_INFO, "%s has exited normally.\n"
	 "Total Calls: %d (Active: %d/Inactive: %d)\n",
	 _diag_key, _total_calls, _active_calls, _inactive_calls);
    exit(0);
  } else if (code == 100) {
    ATM_Attributes::result err;
    // Call is Inactive
    if (_call_is_active == -1) {
      Outgoing_ATM_Call * newc = NewCall(_called_party_number, _bhli); 
      
      if (newc) {
	diag(_diag_key, DIAG_INFO, "Establishing call to service.\n");
	
	err = newc->Establish();
	_total_calls++;
	
	DIAG(_diag_key, DIAG_INFO, 
	     cout << "call_periodic (Info): ";
	     cout << "Result of call->Establish: ";
	     PrintResult(err);
	     cout << endl; );
	
	ATM_Call::call_status c_status = newc->Status();
	
	DIAG(_diag_key, DIAG_INFO, 
	     cout << "call_periodic (Info): ";
	     cout << "call status:";
	     PrintStatus(c_status);
	     cout << "." << endl; );
      } else {
	diag(_diag_key, DIAG_INFO, 
	     "call_periodic::Call_Inactive(): Couldn't create a call.  Aborting\n");
	exit(1);
      }
      _call_is_active = 0;
    } else if (_call_is_active == 1) {
      // Call is active
      diag(_diag_key, DIAG_INFO, "Tearing down call to service.\n");

      err = _call->Set_cause(ie_cause::normal_call_clearing);

      if (err != ATM_Attributes::success) {
	diag(_diag_key, DIAG_INFO, "SET CAUSE TEST FAILED\n");
	exit(1);
      }

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
      _call_is_active = 0;
    }
  } else
    diag(_diag_key, DIAG_INFO, "%s received an unsupported code %d.\n", _diag_key, code);
}

void main(int argc, char ** argv)
{
  if (argc < 4) {
    cerr << "usage: " << argv[0] 
	 << " my-name server-name [periodicity [host-file [stoptime \
[bhli [visualizer-pipe]]]]]" << endl;
    cerr << "       default periodicity is 1.0 seconds" << endl;
    cerr << "       default stoptime is 10.0 seconds" << endl;
    cerr << "       default bhli is \"call_periodic\"" << endl;
    cerr << "       default visualizer-pipe is active_exit.output" << endl;
    exit(1);
  }

  char *host_file_name = "host.file";
  char *bhli = 0;
  char *called_party = 0;
  double stop_time = -1.0, period = -1.0;

  DiagLevel("call_periodic", DIAG_DEBUG);
  DiagLevel("api", DIAG_DEBUG);

  if ((argc >= 5) && (strlen(argv[4]) > 0))
    host_file_name = argv[4];

  HostFile name_service(host_file_name);
  int port = name_service.get_port_byname(argv[1]);

  if (port > 0) {

    called_party = name_service.get_nsap_byname(argv[2]);
    if (called_party != 0) {

      if (argc >= 4)
	period = atof(argv[3]);

      if (argc >= 6)
	stop_time = atof(argv[5]);

      if (argc >=7)
	bhli = argv[6];

      // call_periodic
  
      if (argc >= 8)
	VisPipe(argv[7]);
      else
	VisPipe("call_periodic.output");

      ATM_Interface &host_nic = Open_ATM_Interface("c", port);
      call_periodic * caller = new call_periodic(host_nic, period, stop_time, 
						 called_party, bhli);

      host_nic.Associate_Controller(*caller);

      SEAN_Run();
    } else {
      cout << argv[0] << ": Can't find host " << argv[2] << endl;
    }
  } else {
    cout << argv[0] << ": Can't find host " << argv[1]
	 << " or can't find port file" << endl;
  }
}
