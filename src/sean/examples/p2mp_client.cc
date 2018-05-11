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

//* -*- C++ -*-

#include <FW/basics/Conduit.h>
#include "BaseClient.h"

int main(int argc, char **argv)
{
  int answer = 0;

  DiagLevel("api", DIAG_ENV);
  DiagLevel("ipc", DIAG_ENV);

  char * host_file_name = "/etc/xti_hosts";
  
  if(argc < 4)
    {
      cout << "USAGE: " << argv[0] << " sd-port "  <<  " server-esi" << " p2mp(0/1) " <<  endl;
      exit(1);
    }
  int port = atoi(argv[1]);
  ATM_Interface &interface = Open_ATM_Interface("/dev/una0", port);
  bool p2mp = false;
  if(atoi(argv[3]))
    p2mp = true;
  BaseClient *client = new BaseClient(interface, argv[2], p2mp);
  if(p2mp)
    {
      char nsap[80];
      printf("Enter Leaf NASP : ");
      fgets(nsap,80,stdin);
      if(nsap[0] != '\0' && nsap[0] != '\n')
	{
	  Addr *leaf_addr =  newAddr(nsap);
	  assert(leaf_addr != 0);
	  client->AddParty(leaf_addr);
	}
    }
  interface.Associate_Controller(*client);
  VisPipe("/dev/null");
  SEAN_Run();
  delete client;
  return answer;
}


