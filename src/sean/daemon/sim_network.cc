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
"$Id: sim_network.cc,v 1.5 1998/09/30 04:18:47 bilal Exp $";
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

  int speedvalue;
  if (argc > 1)
    speedvalue = atoi(argv[1]);
  else {
    cout << "usage: " << argv[0] << " speed duration app1 app2 app3" << endl;
    cout << "Enter log_2 speed (-2,-1,0,1,2) or enter 666 for sim-time: ";
    cin >> speedvalue;
  }

  double simtime;
  if (argc > 2) {
    simtime = atof(argv[2]);
  }
  else {
    cout << "How long (seconds) to run the simulation: ";
    cin >> simtime;
  }

  char fname1[255],fname2[255], fname3[255];
  if (argc > 3) strcpy(fname1,argv[3]);
  else {
    cout << "Exec file for app1: "; cin >> fname1;
  }

  if (argc > 4) strcpy(fname1,argv[4]);
  else {
    cout << "Exec file for app2: "; cin >> fname2;
  }

  if (argc > 5) strcpy(fname1,argv[5]);
  else {
    cout << "Exec file for app3: "; cin >> fname3;
  }

  NSAP_DCC_ICD_addr * addr0 = 
    new NSAP_DCC_ICD_addr("0x47.0005.80.ffde00.0000.0000.0104.000000000000.00");
  NSAP_DCC_ICD_addr * addr1 = 
    new NSAP_DCC_ICD_addr("0x47.0005.80.ffde00.0000.0000.0104.000000000001.00");
  NSAP_DCC_ICD_addr * addr2 = 
    new NSAP_DCC_ICD_addr("0x47.0005.80.ffde00.0000.0000.0104.000000000002.00");

  Conduit* sw = make_Switch(3);
  Conduit* h0 = make_Host(addr0,fname1);
  Conduit* h1 = make_Host(addr1,fname2);
  Conduit* h2 = make_Host(addr2,fname3);
  
  LinkTogether(sw,h0,0);
  LinkTogether(sw,h1,1);
  LinkTogether(sw,h2,2);

  Kernel & controller = theKernel();
  controller.SetSpeed((Kernel::Speed)speedvalue);
  controller.StopIn(simtime);

  theKernel().Run();
  exit(0);
}
