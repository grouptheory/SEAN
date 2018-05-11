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
// File: DuplicateServicesTest.cc
// Version: $Id: DuplicateServicesTest.cc,v 1.2 1998/07/30 20:35:11 talmage Exp $
//
// Purpose: main() for the instantiation of NService.  See
// NService.cc

#include <FW/basics/Conduit.h>
#include <sean/daemon/HostFile.h>
#include "DuplicateServices.h"

int main(int argc, char **argv)
{
  int answer = 0;
  double lifetime = 10.0;
  char * host_file_name = "../daemon/host.file";

  DiagLevel("DuplicateServices", DIAG_INFO);

  if (argc >= 2) {

    if ((argc >= 3) && (strlen(argv[2]) > 0))
      host_file_name = argv[2];

    if (argc >= 4)
      lifetime = atof(argv[3]);

  //
  // Visualize into the file named on the command line or to
  // "DuplicateServices.output".  This lets us run live with GT if we
  // want to.
  //
    if (argc > 5)
      VisPipe(argv[4]);
    else 
      VisPipe("DuplicateServices.output");

    DuplicateServices *smanager;
    HostFile name_service(host_file_name);
    int port = name_service.get_port_byname(argv[1]);

    if (port > 0) {
      ATM_Interface &host_nic = Open_ATM_Interface("/dev/una0", port);
      smanager = 
	new DuplicateServices("DuplicateServices", host_nic, lifetime);

      host_nic.Associate_Controller(*smanager);

      SEAN_Run();
    } else {
      cout << argv[0] << ": Can't find host " << argv[1] 
	   << " or can't find port file" << endl;
      answer = 1;
    }

  } else {
    cout << "Usage: " << argv[0] 
	 << " my-name [host-file [lifetime [visualizer-pipe]]]" 
	 << endl;
    cout << "       default host-file is " << host_file_name << endl;
    cout << "       default lifetime is " << lifetime << " seconds" << endl;
    answer = 1;
  }

  exit(answer);
}
