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

#ifndef LINT
static char const _linktst_cc_rcsid_[] =
"$Id: linktst.cc,v 1.2 1998/08/06 04:04:12 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <iostream.h>
#include "tcp_ipc.h"
#include "IPC_Visitors.h"
#include <FW/basics/Conduit.h>
#include <FW/behaviors/Adapter.h>
#include <FW/behaviors/Protocol.h>
#include <FW/basics/diag.h>

#include "linktst.h"


int main(int argc, char** argv){

  VisPipe("debugging.output");
  DiagLevel("ipc",DIAG_DEBUG);

  char type, machine[255];
  int port;
  if (argc<2) {
    cout << "Client usage: " << argv[0] << " c machine port" << endl;
    cout << "Server usage: " << argv[0] << " s port" << endl;
    exit(1);
  }

  tcp_ipc_layer *TCP = 0;
  Adapter* ad_tcp;
  Conduit* cc_tcp;

  State *state_cs;  // client server
  Protocol* ad_cs;
  Conduit* cc_cs;

  type=argv[1][0];  
  switch (type) {
  case 'c': {
    if (argc!=4) {
      cout << "Client usage: " << argv[0] << " c machine port" << endl;
      exit(1);
    }
    strcpy(machine,argv[2]);
    port=atoi(argv[3]);
    cout << "Client running, will attempt to connect to " << machine << 
      "." << port << endl;

    TCP = new tcp_ipc_layer;
    ad_tcp = new Adapter(TCP);
    cc_tcp = new Conduit("client-tcp-layer",ad_tcp);

    state_cs = new Client(machine,port);
    ad_cs = new Protocol(state_cs);
    cc_cs = new Conduit("client",ad_cs);

    Join(A_half(cc_tcp),B_half(cc_cs));
    } break;
  case 's': {
    if (argc!=3) {
      cout << "Server usage: " << argv[0] << " s port" << endl;
      exit(1);
    }
    strcpy(machine,"");
    port=atoi(argv[2]);
    cout << "Server running, will attempt to listen on " << port << endl;

    TCP = new tcp_ipc_layer(port);
    ad_tcp = new Adapter(TCP);
    cc_tcp = new Conduit("server-tcp-layer",ad_tcp);

    state_cs = new Server();
    ad_cs = new Protocol(state_cs);
    cc_cs = new Conduit("server",ad_cs);

    Join(A_half(cc_tcp),B_half(cc_cs));
    } break;
  default:
    cout << "Client usage: " << argv[0] << " c machine port" << endl;
    cout << "Server usage: " << argv[0] << " s port" << endl;
    exit(1);
    break;
  }

  theKernel().SetSpeed(Kernel::REAL_TIME);
  theKernel().Run();

  exit(0);
}




