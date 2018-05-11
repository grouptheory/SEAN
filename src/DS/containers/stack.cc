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
static char const _stack_cc_rcsid_[] =
"$Id: stack.cc,v 1.3 1998/08/06 04:02:25 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <DS/containers/stack.h>

template < class T >
stack < T >::stack(void) : _list() { }

template < class T >
stack < T >::~stack() { }

template < class T >
void stack < T >::push(T item)
{  _list.push(item);  }

template < class T >
T stack < T >::pop(void)
{  return _list.pop();  }

template < class T >
int stack < T >::size(void) const
{  return _list.size(); }

template < class T >
bool stack < T >::empty(void) const
{  return _list.empty(); }

template < class T >
void stack < T >::clear(void)
{  _list.clear(); }
