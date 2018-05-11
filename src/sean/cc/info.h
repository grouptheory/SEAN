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
#ifndef __INFO_H__
#define __INFO_H__

#ifndef LINT
static char const _info_h_rcsid_[] =
"$Id: info.h,v 1.3 1998/07/24 11:16:57 bilal Exp $";
#endif

#include <DS/containers/dictionary.h>
#include <sean/templates/constants.h>

#define LEAF_INITIATED 1
#define LEAF_ACTIVE    2

//-------------------------------------------------------
class call_info {
  friend int compare(const call_info& c1, const call_info& c2);
  friend bool operator ==(class call_info, class call_info);
public:

  call_info(int cid = NO_CID, int crv = NO_CREF);

  bool is_active(void) const;
  void make_active(void);
  void make_inactive(void);
  
  bool was_release_ind_seen(void) const;
  void set_release_ind_seen(void);

  int  _cid;
  int  _crv;
  bool _active;
  bool _release_ind_seen;

  int  _vp;
  int  _vc;
  bool _p2mp;
  
  dictionary<int,int> _leafstate;
};

class serv_info {
  friend int compare(const serv_info& c1, const serv_info& c2);
  friend bool operator ==(class serv_info, class serv_info);
public:

  serv_info(int sid = NO_SID);

  int _sid;
};

#endif
