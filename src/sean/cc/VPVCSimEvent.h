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
#ifndef __VPVCSIMEVENT_H__
#define __VPVCSIMEVENT_H__

#ifndef LINT
static char const _VPVCSimEvent_h_rcsid_[] =
"$Id: VPVCSimEvent.h,v 1.1 1998/07/20 11:10:53 bilal Exp $";
#endif

#include <FW/kernel/SimEvent.h>

#define VPVC_CODE 0xFEF1F0FA

class VPVCSimEvent : public SimEvent {
public:

  VPVCSimEvent(SimEntity * from, SimEntity * to, 
	       int vp = 0, int vc = 0, int cref = 0);
  virtual ~VPVCSimEvent();

  int GetVPI(void) const;
  int GetVCI(void) const;
  int GetCREF(void) const;
  int GetPort(void) const;

  void SetVPI(const int v);
  void SetVCI(const int v);
  void SetCREF(const int v);
  void SetPort(const int p);

private:

  int _vp;
  int _vc;
  int _cref;
  int _port;
};

#endif
