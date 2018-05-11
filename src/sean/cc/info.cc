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
static char const _info_cc_rcsid_[] =
"$Id: info.cc,v 1.4 1998/08/06 04:04:06 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "info.h"

//-------------------------------------------------------
//-------------------------------------------------------
call_info::call_info(int cid, int crv) 
  : _cid(cid), _crv(crv), _active(false), 
    _release_ind_seen(false), _vc(NO_VCI), _vp(NO_VPI),
    _p2mp(false)
{ }

bool call_info::is_active(void) const { return _active; }
void call_info::make_active(void) { _active=true; }
void call_info::make_inactive(void) { _active=false; }

bool call_info::was_release_ind_seen(void) const {
  return _release_ind_seen;
}

void call_info::set_release_ind_seen(void) {
  _release_ind_seen=true;
}

int compare(const call_info & c1, const call_info & c2) 
{
  if (c1._cid < c2._cid) 
    return -1;
  if (c1._cid > c2._cid) 
    return  1;
  return 0;
}

bool operator ==(class call_info a, class call_info b) 
{
  return !compare(a,b);
}

serv_info::serv_info(int sid) 
{
  _sid = sid;
}

int compare(const serv_info& c1, const serv_info& c2) 
{
  if (c1._sid < c2._sid) 
    return -1;
  if (c1._sid > c2._sid) 
    return  1;
  return 0;
}

bool operator ==(class serv_info a, class serv_info b) 
{
  return !compare(a,b);
}

