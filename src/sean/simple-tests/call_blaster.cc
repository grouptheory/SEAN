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
static char const _call_blaster_cc_rcsid_[] =
"$Id: call_blaster.cc,v 1.13 1998/08/06 04:06:20 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/basics/Conduit.h>
#include <sean/daemon/HostFile.h>
#include "call_blaster.h"
#include "PrintFunctions.h"

call_blaster::call_blaster(ATM_Interface & interface, double stop_time,
			   char *called_party) 
  : BaseClient("call_blaster", interface, called_party), 
    _total_calls(1), _num_active(0), _num_inactive(0)
{ 
  if (stop_time == -1.0)
    stop_time = 25.0;

  RegisterPeriodicCallback( 666, stop_time );
}

call_blaster::~call_blaster() { }

void call_blaster::Call_Active(ATM_Call   & call)
{
  diag(_diag_key, DIAG_INFO, 
       "The call has become active, this test is supposed to be run without a server!\n");
  _num_active++;
  SEAN_Halt();
}

void call_blaster::Call_Inactive(ATM_Call & call)
{
  diag(_diag_key, DIAG_INFO,
       "The call has become inactive, attempting to establish a new call to the same service.\n");
  _num_inactive++;
  // delete (Outgoing_ATM_Call *)(&call);

  sleep( 1 );

  Outgoing_ATM_Call * newc =
    NewCall(_called_party_number);

  if (newc) {
    diag(_diag_key, DIAG_INFO, "Calling service.\n");

    ATM_Attributes::result err = newc->Establish();
    _total_calls++;

    DIAG(_diag_key, DIAG_INFO, 
	 cout << "call_blaster (Info): ";
	 cout << "Result of call->Establish: ";
	 PrintResult(err);
	 cout << endl; );

    ATM_Call::call_status c_status = newc->Status();

    DIAG(_diag_key, DIAG_INFO, 
	 cout << "call_blaster (Info): ";
	 cout << "call status:";
	 PrintStatus(c_status);
	 cout << "." << endl; );
  } else {
    diag(_diag_key, DIAG_INFO, 
	 "call_blaster::Call_Inactive(): Couldn't create a call.  Aborting\n");
    exit(1);
  }
}

void call_blaster::PeriodicCallback(int code)
{
  if (code == 666) {
    diag(_diag_key, DIAG_INFO, "%s has exited normally.\n"
	 "Total Calls: %d  (Active: %d/Inactive: %d)\n",
	 _diag_key, _total_calls, _num_active, _num_inactive);
    exit(0);
  } else
    diag(_diag_key, DIAG_INFO, "%s received an unsupported code %d.\n", _diag_key, code);
}

void main(int argc, char ** argv)
{
  if (argc < 3) {
    cerr << "usage: " << argv[0] 
	 << " my-name server-name [host-file [stoptime [visualizer-pipe]]]" 
	 << endl;
    exit(1);
  }

  char *host_file_name = "host.file";
  char *called_party = 0;
  double stop_time = -1.0;

  if (argc >= 5)
    stop_time = atof(argv[4]);

  // call_blaster
  DiagLevel("call_blaster", DIAG_DEBUG);
  
  if (argc >= 6)
    VisPipe(argv[5]);
  else
    VisPipe("call_blaster.output");

  if ((argc >= 4) && (strlen(argv[3]) > 0))
    host_file_name = argv[3];

  HostFile name_service(host_file_name);
  int port = name_service.get_port_byname(argv[1]);

  if (port > 0) {

    called_party = name_service.get_nsap_byname(argv[2]);

    if (called_party != 0) {
      ATM_Interface &host_nic = Open_ATM_Interface("c", port);
      call_blaster * caller = new call_blaster(host_nic, stop_time, 
					       called_party);
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
