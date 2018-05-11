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
"$Id: p2mp_daemon.cc,v 1.1 1998/09/28 12:32:11 bilal Exp $";
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
#include "ccd.h"
#include "Router.h"
#include "PortMux.h"
#include "Toggler.h"

class UserNetwork : public Expander {
public:

  UserNetwork(int listening_ipc_port, double t, int portnumber)
  {
    signalling_stack * CCD_exp = new signalling_stack(listening_ipc_port, 
					      USER_SIDE,
					      signalling_stack::sans_aal,
					      true,    // has_ipc_above
					      NO_PORT);    

    Cluster * CCD_cluster = new Cluster(CCD_exp);
    _user_ccd = new Conduit("USER", CCD_cluster);

    Toggler * togg = new Toggler(t);
    Protocol * pro = new Protocol(togg);
    _link = new Conduit("LINK", pro);

    CCD_exp = new signalling_stack(listening_ipc_port,
			       NETWORK_SIDE,
			       signalling_stack::sans_aal,
			       false,
			       portnumber);

    CCD_cluster = new Cluster(CCD_exp);
    _network_ccd = new Conduit("NET", CCD_cluster);

    Join(B_half(_user_ccd), A_half(_link));
    Join(B_half(_link),     B_half(_network_ccd));
  }

  Conduit * GetAHalf(void) const
  {
    // This will cause the program to abort ... signalling_stack's A_half is fookered
    // return A_half(_user_ccd);
    return 0;
  }

  Conduit * GetBHalf(void )const
  {
    return A_half(_network_ccd);
  }

  void OwnerNameChanged(const char * const name)
  {
    char buf[256];
    sprintf(buf, "%s %s", name, _user_ccd->GetName());
    _user_ccd->SetName(buf);
    sprintf(buf, "%s %s", name, _link->GetName());
    _link->SetName(buf);
    sprintf(buf, "%s %s", name, _network_ccd->GetName());
    _network_ccd->SetName(buf);
  }

protected:

  virtual ~UserNetwork( ) { }

  Conduit * _user_ccd;
  Conduit * _network_ccd;
  Conduit * _link;
};

//-------------------------------------------------------------
int main(int argc, char ** argv)
{
  VisPipe("debugging.output");
  DiagLevel("api",     DIAG_DEBUG);
  DiagLevel("ccd",     DIAG_DEBUG);
  DiagLevel("daemon",  DIAG_DEBUG);
  DiagLevel("cc",      DIAG_DEBUG);
  DiagLevel("ipc",     DIAG_DEBUG);
  DiagLevel("fsm.uni", DIAG_DEBUG);

  if ((argc != 2) && (argc != 3)) {
    cout << "CCD usage: " << argv[0] << " listening-port [network-transit-time]" << endl << endl;
    exit(1);
  }

  int lport = atoi(argv[1]);
  double t;
  if (argc == 2) {
    t = 0.01;
    cout << "Taking network transit time as " << t << "." << endl;
  } else 
    t = atof(argv[2]);

  UserNetwork * un1 = new UserNetwork(lport, t, 0);
  Cluster     * clu = new Cluster(un1);
  Conduit     * cc1 = new Conduit("1", clu);

  UserNetwork * un2 = new UserNetwork(lport + 1, t, 1);
                clu = new Cluster(un2);
  Conduit     * cc2 = new Conduit("2", clu);

  UserNetwork * un3 = new UserNetwork(lport + 2, t, 2);
                clu = new Cluster(un3);
  Conduit     * cc3 = new Conduit("3", clu);

  // -----------------------------------------------
  Router  * router = new Router( );

  // Register attached hosts in Router
  NSAP_DCC_ICD_addr * addr0 = new NSAP_DCC_ICD_addr("0x47.0005.80.ffde00.0000.0000.0104.0020480620eb.01");
  NSAP_DCC_ICD_addr * addr1 = new NSAP_DCC_ICD_addr("0x47.0005.80.ffde00.0000.0000.0104.0020480620ec.01");
  NSAP_DCC_ICD_addr * addr2 = new NSAP_DCC_ICD_addr("0x47.0005.80.ffde00.0000.0000.0104.0020480620ed.01");
  router->RegisterAddr(addr0, 0);
  router->RegisterAddr(addr1, 1);
  router->RegisterAddr(addr2, 2);

  Adapter * ada = new Adapter(router);
  Conduit * rte = new Conduit("Router", ada);

  PortAccessor * acc = new PortAccessor( );
  Mux          * mux = new Mux(acc);
  Conduit      * pmx = new Conduit("PortMux", mux);

  // Mux <--> Router
  Join(A_half(rte), A_half(pmx));
  ConnectorVisitor v(cc1, 0);
  // cc1 <--> Mux
  Join(B_half(cc1), B_half(pmx), &v, &v);
  v.SetPort(1);
  // cc2 <--> Mux
  Join(B_half(cc2), B_half(pmx), &v, &v);
  v.SetPort(2);
  // cc3 <--> Mux
  Join(B_half(cc3), B_half(pmx), &v, &v);

  theKernel().SetSpeed(Kernel::REAL_TIME);
  theKernel().Run();

  exit(0);
}
