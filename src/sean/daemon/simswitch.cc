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
static char const _linktst_cc_rcsid_[] =
"$Id: simswitch.cc,v 1.4 1999/04/07 20:46:23 marsh Exp $";
#endif
#include <common/cprototypes.h>

#include <sean/daemon/ccd_config.h>

#include <iostream.h>
#include <FW/kernel/Kernel.h>
#include <FW/basics/Conduit.h>
#include <FW/behaviors/Adapter.h>
#include <FW/behaviors/Protocol.h>
#include <FW/basics/diag.h>
#include <FW/actors/Expander.h>
#include <codec/uni_ie/ie.h>

#include <sean/cc/sean_Visitor.h>
#include <sean/api/ATM_Interface.h>

#include "Switch.h"
#include "Fiber.h"
#include "HostFile.h"

//-------------------------------------------------------------
int main(int argc, char ** argv)
{
  char * user=getenv("USER");
  char logname[255]; sprintf(logname,"%s-simswitch.output",user);
  cout << "*** NOTICE *** " << argv[0] << " is logging to '" << logname << "'.\n";
  VisPipe( logname );

  //  DiagLevel("api",      DIAG_ENV);
  //  DiagLevel("cc_layer", DIAG_ENV);
  //  DiagLevel("daemon",   DIAG_ENV);
  //  DiagLevel("ipc",      DIAG_ENV);
  //  DiagLevel("fsm",      DIAG_ENV);
  loadDiagPrefs();
  
  if (argc!=3) {
    cout << "usage: " << argv[0] << " base-ipc-port host-file" << endl;
    exit(1);
  }

  int lport=atoi(argv[1]);
  
  HostFile* config = new HostFile(argv[2]);
  config->make_switch(lport);
  
  SEAN_Run();
  exit(0);
}



