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
static char const _VPVCSimEvent_cc_rcsid_[] =
"$Id: VPVCSimEvent.cc,v 1.2 1998/08/06 04:04:04 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "VPVCSimEvent.h"

#include <sean/templates/constants.h>


VPVCSimEvent::VPVCSimEvent(SimEntity * from, SimEntity * to, 
			   int vp, int vc, int cref)
  : SimEvent(from, to, VPVC_CODE), _vp(vp), _vc(vc), _cref(cref), _port( NO_PORT ) { }

VPVCSimEvent::~VPVCSimEvent() { }

int VPVCSimEvent::GetVPI(void) const { return _vp; }
int VPVCSimEvent::GetVCI(void) const { return _vc; }
int VPVCSimEvent::GetCREF(void) const { return _cref; }
int VPVCSimEvent::GetPort(void) const { return _port; }

void VPVCSimEvent::SetVPI(const int v) { _vp = v; }
void VPVCSimEvent::SetVCI(const int v) { _vc = v; }
void VPVCSimEvent::SetCREF(const int v) { _cref = v; }
void VPVCSimEvent::SetPort(const int v) { _port = v; }
