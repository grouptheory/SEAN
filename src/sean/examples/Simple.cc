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

/* -*- C++ -*-
 * File: Simple.cc
 * Version: $Id: Simple.cc,v 1.7 1998/08/05 17:28:33 talmage Exp $
 *
 * Purpose: Simple implements a client and a server.  The client calls
 * the server and exchanges some data with it.  The server hangs up.
 * Each can hang up due to inactivity.
 *
 * To run Simple, start a signalling daemon
 */

#include <FW/basics/Conduit.h>
#include <sean/daemon/HostFile.h>
#include "SimpleService.h"
#include "SimpleCaller.h"

void Usage(char *name)
{
  cout << "Usage: " << name 
       << " s my-name [host-file [visualizer-pipe]]" << endl;
  cout << "       " << name 
       << " c my-name server-name [host-file [visualizer-pipe]]" << endl;
}


//
// Create a SimpleCaller client.
//
int StartClient(int argc, char **argv)
{
  int answer = 0;

  if (argc >= 4) {
    char * host_file_name = "./host.file";

    if ((argc >= 5) && (strlen(argv[4]) > 0))
      host_file_name = argv[4];

    //
    // Establish the name-to-ATM-address mapping facility as defined
    // by the host file.
    //
    HostFile name_service(host_file_name);

    //
    // Look up the port number of the signalling daemon.
    //
    int port = name_service.get_port_byname(argv[2]);

    if (port > 0) {
      //
      // Create an ATM interface on /dev/una0.
      // Make it listen on port 'port'.
      //
      ATM_Interface &interface = Open_ATM_Interface("/dev/una0", port);
      SimpleCaller *caller = 0;
      char *called_party = 0;

      //
      // Use the mapping facility to learn the ATM address
      // of the server.
      //
      called_party = name_service.get_nsap_byname(argv[3]);

      if (called_party != 0) {
	cout << "called party name = " << argv[3] 
	     << " ATM Address  = " << called_party << endl;

	caller = new SimpleCaller(interface, called_party);

	//
	// SimpleCaller is a Controller.
	//
	// A Controller will learn about calls on an ATM_Interface
	// only if it is associated with the ATM_Interface.
	//
	interface.Associate_Controller(*caller);
    
	//
	// Decide where to send the visualizer output.
	//
	if (argc >= 6) {
	  cout << "vispipe = " << argv[5] << endl;
	  VisPipe(argv[5]);
	} else {
	  cout << "vispipe = SimpleCaller.output" << endl;
	  VisPipe("SimpleCaller.output");
	}

	//
	// Go, SEAN, go!
	//
	SEAN_Run();

	delete caller;

      } else {
	cout << argv[0] << ": Can't find host or can't find port file" << endl;
	answer = 1;
      }
    } else {
      cout << argv[0] << ": Can't find host or can't find port file" << endl;
      answer = 1;
    }
  } else {
    Usage(argv[0]);
    answer = 1;
  } 
  return answer;
}


//
// Create a server that answers calls from SimpleCallers.
//
int StartServer(int argc, char **argv)
{
  int answer = 0;
  char * host_file_name = "./host.file";
  
  if ((argc >= 4) && (strlen(argv[3]) > 0))
    host_file_name = argv[3];

  //
  // Establish the name-to-ATM-address mapping facility as defined
  // by the host file.
  //
  HostFile name_service(host_file_name);

  //
  // Look up the port number of the signalling daemon.
  //
  int port = name_service.get_port_byname(argv[2]);

  if (port > 0) {
    //
    // Create an ATM interface on /dev/una0.
    // Make it listen on port 'port'.
    //
    ATM_Interface &interface = Open_ATM_Interface("/dev/una0", port);
    SimpleService *service = 0;

    service = new SimpleService(interface);

    //
    // SimpleCaller is a Controller.
    //
    // A Controller will learn about calls on an ATM_Interface
    // only if it is associated with the ATM_Interface.
    //
    interface.Associate_Controller(*service);

    //
    // Decide where to send the visualizer output.
    //
    if (argc >= 5) {
      cout << "vispipe = " << argv[5] << endl;
      VisPipe(argv[4]);
    } else {
      cout << "vispipe = SimpleService.output" << endl;
      VisPipe("SimpleService.output");
    }

    //
    // Go, SEAN, go!
    //
    SEAN_Run();

    delete service;

  } else {
    cout << argv[0] << ": Can't find host or can't find port file" << endl;
    answer = 1;
  }

  return answer;
}


int main(int argc, char **argv)
{
  int answer = 0;

  DiagLevel("api", DIAG_ENV);
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


