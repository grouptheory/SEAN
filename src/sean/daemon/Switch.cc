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
static char const _Switch_cc_rcsid_[] =
"$Id: Switch.cc,v 1.8 1998/09/04 17:24:26 mountcas Exp $";
#endif
#include <common/cprototypes.h>

#include "Switch.h"
#include "SigStack.h"
#include "SwitchFabric.h"
#include "Fiber.h"

#include <FW/behaviors/Adapter.h>
#include <FW/behaviors/Mux.h>
#include <FW/behaviors/Factory.h>
#include <FW/behaviors/Protocol.h>
#include <FW/basics/Conduit.h>

//----------------------------------------------------
Switch::Switch(int num_ports)
  : _router(0), _rc(0), _hi_mux(0), _lo_mux(0), 
    _num_ports(num_ports)
{
  _router = new Router( );
  Adapter * ada = new Adapter(_router);
  _rc = new Conduit("Router", ada);
  
  PortAccessor * acc = new PortAccessor( );
  Mux * mux = new Mux(acc);
  _hi_mux = new Conduit("UpperPortMux", mux);

  Join(A_half(_rc),      A_half(_hi_mux));

  acc = new PortAccessor( );
  mux = new Mux(acc);
  _lo_mux = new Conduit("LowerPortMux", mux);
  
  acc = new PortAccessor( );
  mux = new Mux(acc);
  _outer_conn_mux = new Conduit("OuterPortMux", mux);

  assert( Join(A_half(_lo_mux), A_half(_outer_conn_mux)) );
  
  for (int i = 0; i < _num_ports; i++) {
    ConnectorVisitor * pv = new ConnectorVisitor( 0, i );

    SigStack * netcc = new SigStack(NETWORK_SIDE, 
				    SigStack::sans_aal, 
				    i);
    Cluster * clu = new Cluster(netcc);
    char buf[256];
    sprintf(buf, "NetCC-%d", i);
    Conduit * newc = new Conduit(buf, clu);

    assert( Join(B_half(_hi_mux), A_half(newc), pv, 0) );
    assert( Join(B_half(_lo_mux), B_half(newc), pv, 0) );

    pv->Suicide();
    _ports.append(newc);
  }

  DefinitionComplete();
}

//----------------------------------------------------
Switch::~Switch()
{
  delete _rc;
  delete _hi_mux;
  delete _lo_mux;
  list_item li;
  forall_items(li, _ports)
    delete _ports.inf(li);
  _ports.clear();
}

//----------------------------------------------------
Conduit * Switch::GetAHalf(void) const
{
  return A_half(_rc);
}

//----------------------------------------------------
Conduit * Switch::GetBHalf(void) const
{
  return B_half(_outer_conn_mux);
}

//----------------------------------------------------
void Switch::RegisterAddr(NSAP_DCC_ICD_addr * addr, int port)
{ 
  assert(port < _num_ports && port >= 0);
  _router->RegisterAddr(addr, port); 
}

//----------------------------------------------------
Conduit * LinkTogether(Conduit * switch1, 
		       Conduit * switch2, 
		       int port1, 
		       int port2)
{
  Conduit * rval = 0;

  if (switch1 && switch2) {
    State    * st = new Fiber(port1,port2);
    Protocol * pr = new Protocol(st);
    rval = new Conduit("Link", pr);

    ConnectorVisitor * cv1 = new ConnectorVisitor( 0, port1 ),
                     * cv2 = new ConnectorVisitor( 0, port2 );
    if (!Join(B_half(switch1), A_half(rval), cv1, cv2)) {
      delete rval; rval = 0;
    } else if (!Join(B_half(rval), B_half(switch2), cv1, cv2)) {
      delete rval; rval = 0;
    }
  }

  return rval;
}

//----------------------------------------------------
Conduit * make_Switch(int ports, Switch*& sw) {
  sw = new Switch(ports);

  Cluster* clu = new Cluster(sw);
  Conduit* c = new Conduit("Switch", clu);
  return c;
}

