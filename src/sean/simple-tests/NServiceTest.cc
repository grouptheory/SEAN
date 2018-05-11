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
// File: NService.cc
// Version: $Id: NServiceTest.cc,v 1.3 1998/07/30 19:33:46 talmage Exp $
//
// Purpose: main() for the instantiation of NService.  See
// NService.cc

#include <FW/basics/Conduit.h>
#include <sean/daemon/HostFile.h>
#include "NService.h"

int main(int argc, char **argv)
{
  int answer = 0;
  double lifetime = -1.0;
  char * host_file_name = "../daemon/host.file";
  int blocksize = 10;

  DiagLevel("NService", DIAG_INFO);

  if (argc >= 3) {

    blocksize = atoi(argv[2]);

    if ((argc >= 4) && (strlen(argv[3]) > 0 ))
      host_file_name = argv[3];

    if (argc >= 5)
      lifetime = atof(argv[4]);

  //
  // Visualize into the file named on the command line or to
  // "NService.output".  This lets us run live with GT if we
  // want to.
  //
    if (argc > 6)
      VisPipe(argv[5]);
    else 
      VisPipe("NService.output");

    NService *smanager;
    HostFile name_service(host_file_name);
    int port = name_service.get_port_byname(argv[1]);

    if (port > 0) {
      ATM_Interface &host_nic = Open_ATM_Interface("/dev/una0", port);
      smanager = new NService("NService", host_nic, lifetime, blocksize);
      host_nic.Associate_Controller(*smanager);

      SEAN_Run();

    } else {
      cout << argv[0] << ": Can't find host " << argv[1] 
	   << " or can't find port file" << endl;
      answer = 1;
    }
  } else {
    cout << "Usage: " << argv[0] 
	 << " my-name [num-services [host-file [lifetime [visualizer-pipe]]]]" 
	 << endl;
    cout << "       Default host-file = " << host_file_name << endl;
    cout << "       Default num-services = 10" << endl;
    cout << "       Default lifetime = -1 (forever)" << endl;

    answer = 1;
  }

  exit(answer);
}
