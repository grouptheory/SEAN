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
#ifndef __SET_H__
#define __SET_H__

#ifndef LINT
static char const _set_h_rcsid_[] =
"$Id: set.h,v 1.22 1999/01/26 18:06:56 mountcas Exp $";
#endif

#include <DS/containers/list.h>

typedef list_item seq_item;

template < class T > 
class set : protected list < T > {
public:

  set(void);
  virtual ~set();

  void insert(T x);
  void del(T x);
  bool member(T x) const;
  T    choose(void) const;

  list_item first(void) const;
  list_item next(list_item li) const;

  int size(void) const;
  T   inf(list_item li) const;

  void clear(void);
  bool empty(void) const;

  set & operator &= (const set & rhs);
  set & operator += (const set & rhs);
  set & operator -= (const set & rhs);

  T & operator [] (int index) const;
};

template < class T > set< T > operator & (set< T > const & lhs, set< T > const & rhs);
template < class T > set< T > operator + (set< T > const & lhs, set< T > const & rhs);
template < class T > set< T > operator - (set< T > const & lhs, set< T > const & rhs);
#endif
