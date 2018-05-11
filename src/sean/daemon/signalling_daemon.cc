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
"$Id: signalling_daemon.cc,v 1.9 1999/04/01 18:01:03 battou Exp $";
#endif
#include <common/cprototypes.h>
#include <sean/daemon/ccd_config.h>

#include <iostream.h>
#include <FW/kernel/Kernel.h>
#include <FW/basics/Conduit.h>
#include <FW/behaviors/Adapter.h>
#include <FW/behaviors/Protocol.h>
#include <FW/basics/diag.h>

#include "Host.h"
#include <sean/api/ATM_Interface.h>

//----------------------------------------------------------------------

int main(int argc, char** argv){

  VisPipe("sd.output");

  //  DiagLevel("api",DIAG_ENV);
  //  DiagLevel("ccd",DIAG_ENV);
  //  DiagLevel("ipc",DIAG_ENV);
  //  DiagLevel("fsm",DIAG_ENV);
  //  DiagLevel("fsm.sscop",DIAG_ERROR);

  loadDiagPrefs();

  if (argc!=2) {
    cout << "usage: " << argv[0] << " listening-port" << endl;
    exit(1);
  }

  int lport = atoi(argv[1]);
  
  Host* my_host = new Host( lport );
  Cluster* host_cluster = new Cluster(my_host);
  Conduit* sd = new Conduit("SEAN_daemon",host_cluster);

  SEAN_Run();
  exit(0);
}
