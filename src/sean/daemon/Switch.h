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
#ifndef __SWITCH_H__
#define __SWITCH_H__

#ifndef LINT
static char const _Switch_h_rcsid_[] =
"$Id: Switch.h,v 1.3 1998/08/21 15:04:40 mountcas Exp $";
#endif

#include <FW/actors/Expander.h>
#include <DS/containers/list.h>

class NSAP_DCC_ICD_addr;
class Router;

class Switch : public Expander {
public:

  Switch(int num_ports);

  Conduit * GetAHalf(void) const;
  Conduit * GetBHalf(void) const;

  void RegisterAddr(NSAP_DCC_ICD_addr * addr, int port);

  virtual ~Switch();

private:

  Router        * _router;
  Conduit       * _rc;
  Conduit       * _hi_mux;
  Conduit       * _lo_mux;
  Conduit       * _outer_conn_mux;

  list<Conduit *> _ports;
  int             _num_ports;
};


// For linking switches and hosts ... 
Conduit * LinkTogether(Conduit * switch1, 
		       Conduit * switch2,
		       int port1 = 0, 
		       int port2 = 0);

Conduit * make_Switch(int ports, Switch*& sw);

#endif
