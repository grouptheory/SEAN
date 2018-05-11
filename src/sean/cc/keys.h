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

#ifndef __CCD_KEYS_H__
#define __CCD_KEYS_H__
#ifndef LINT
static char const _ccd_keys_h_rcsid_[] =
"$Id: keys.h,v 1.3 1998/08/18 15:06:00 bilal Exp $";
#endif

class abstract_local_id;
#include <codec/uni_ie/ie_base.h>

//---------------------------------------------------

class appid_and_int {
  friend int compare(appid_and_int * const & a, appid_and_int * const & b);
public:

  appid_and_int(const abstract_local_id* id, int x);
  appid_and_int(const appid_and_int & rhs);
  ~appid_and_int();

  int equals(const appid_and_int & rhs);

  int get_int(void) const;
  const abstract_local_id * get_id(void) const;

private:

  const abstract_local_id * _id;
  int                       _x;
};


//---------------------------------------------------
class setup_attributes {
public:
  setup_attributes(InfoElem** iearray, int crv);
  ~setup_attributes();

  friend int compare(setup_attributes *const & a, setup_attributes *const & b);

  int score( setup_attributes* incoming_call_attributes );
  bool is_distinguishable_from( setup_attributes* incoming_call_attributes );

  int crv(void) { return _crv; }

  int call_id(void);
  int seqnum(void);

private:  
  InfoElem* _ie[num_ie];
  int _crv;
};

#endif


