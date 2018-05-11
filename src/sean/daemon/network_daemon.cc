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
"$Id: network_daemon.cc,v 1.9 1998/09/30 04:18:47 bilal Exp $";
#endif

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
#include "Switch.h"
#include "Fiber.h"
#include "Host.h"

//-------------------------------------------------------------
int main(int argc, char ** argv)
{
  VisPipe("sim_net.output");

  DiagLevel("api",     DIAG_ENV);
  DiagLevel("ccd",     DIAG_ENV);
  DiagLevel("daemon",  DIAG_ENV);
  DiagLevel("cc",      DIAG_ENV);
  DiagLevel("ipc",     DIAG_ENV);
  DiagLevel("fsm",     DIAG_ENV);

  if (argc!=2) {
    cout << "usage: " << argv[0] << " listening-port" << endl;
    exit(1);
  }

  int lport=atoi(argv[1]);

  NSAP_DCC_ICD_addr * addr0 = 
    new NSAP_DCC_ICD_addr("0x47.0005.80.ffde00.0000.0000.0104.000000000000.00");
  NSAP_DCC_ICD_addr * addr1 = 
    new NSAP_DCC_ICD_addr("0x47.0005.80.ffde00.0000.0000.0104.000000000001.00");
  NSAP_DCC_ICD_addr * addr2 = 
    new NSAP_DCC_ICD_addr("0x47.0005.80.ffde00.0000.0000.0104.000000000002.00");

  Conduit* sw = make_Switch(3);
  Conduit* h0 = make_Host(addr0,lport);
  Conduit* h1 = make_Host(addr1,lport+1);
  Conduit* h2 = make_Host(addr2,lport+2);
  
  LinkTogether(sw,h0,0);
  LinkTogether(sw,h1,1);
  LinkTogether(sw,h2,2);

  Kernel & controller = theKernel();
  controller.SetSpeed(Kernel::REAL_TIME);
  controller.StopIn(simtime);

  theKernel().Run();
  exit(0);
}
