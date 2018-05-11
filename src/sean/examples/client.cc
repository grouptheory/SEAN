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
#include <sean/daemon/HostFile.h>

int main(int argc, char **argv)
{
  int answer = 0;

  //  DiagLevel("api", DIAG_ENV);
  //  DiagLevel("ipc", DIAG_ENV);

  if(argc < 4)
    {
      cout << "USAGE: " << argv[0] << " sd-port dst-host p2mp(0/1) [host-filename]" <<  endl;
      exit(1);
    }

  char * host_file_name = "/etc/xti_hosts";
  if(argc > 4)
    host_file_name = argv[4];
  HostFile name_service(host_file_name);

  char *called_party = name_service.get_nsap_byname(argv[2]);
  if(called_party == 0)
    {
      cout << "called party name " << argv[2] << " not in " << host_file_name << endl;
      exit(1);
    }
  
  int port = atoi(argv[1]);
  ATM_Interface &interface = Open_ATM_Interface("/dev/una0", port);
  bool p2mp = false;
  if(atoi(argv[3]))
    p2mp = true;
  BaseClient *client = new BaseClient(interface, called_party, p2mp);
  if(p2mp)
    {
      char leaf_hostname[80];
      leaf_hostname[0] ='\0';
      printf("Enter Leaf host name : ");
      fgets(leaf_hostname,80,stdin);
      // fgets return \n + user might enter space (mouse paste)
      // clean up these problems
      int len = strlen(leaf_hostname);
      while (len && leaf_hostname[len] <= ' ')
	leaf_hostname[len--] = '\0';
      
      if(leaf_hostname[0] != '\0' && leaf_hostname[0] != '\n')
	{
	  char *leaf_party = name_service.get_nsap_byname(leaf_hostname);
	  if (leaf_party)
	    {
	      Addr *leaf_addr =  newAddr(leaf_party);
	      assert(leaf_addr != 0);
	      client->AddParty(leaf_addr);
	    }
	  else
	    {
	      cout << "leaf node not found in " << host_file_name << endl;
	    }
	}
    }
  interface.Associate_Controller(*client);
  VisPipe("/dev/null");
  SEAN_Run();
  delete client;
  return answer;
}


