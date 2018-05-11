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

#if !defined(LINT)
static char const _errmsg_cc_rcsid_[] =
"$Id: errmsg.cc,v 1.4 1999/03/22 17:09:37 mountcas Exp $";
#endif
#include <common/cprototypes.h>
#include <codec/uni_ie/errmsg.h>

ie_errmsg::ie_errmsg(ie_cause::CauseValues cv, diag_type dt, int id) : 
           _cause(cv), _errtype(dt),  _errid(id) { }

char ie_errmsg::PrintSpecific(ostream & os) const
{
  os << "cause::" << _cause << " ";
  os << "errtype::";
  switch (_errtype) {
  case none: os << "none "; break;
  case empty_ie: os << "empty ie "; break;
  case wrong_id: os << "wrong id "; break;
  default: os << "unknown "; break;
  }
  os << "errid::" << _errid << endl;
  return 0;
}

ostream & operator << (ostream & os, ie_errmsg & x)
{
  x.PrintSpecific(os);
  return (os);
}
