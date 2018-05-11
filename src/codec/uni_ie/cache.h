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
#ifndef __CACHE_H__
#define __CACHE_H__

#ifndef LINT
static char const _cache_h_rcsid_[] =
"$Id: cache.h,v 1.4 1999/03/22 17:07:08 mountcas Exp $";
#endif

#include <iostream.h>
#include <common/cprototypes.h>

class Cache {
public:
  /// Constructor
  Cache();
  /// Destructor
  virtual ~Cache();
  /// Methods
  u_char * LookupCache(void);
  void     MakeStale(void);
  void     FillCache(u_char *buf, int len);
  int      CacheLength(void) const;
  /// returns 1 for TRUE 0 for FALSE
  virtual  u_char IsVariableLen(void) const = 0;

  virtual char PrintSpecific(ostream & os) const;
  friend ostream & operator << (ostream & os, Cache & x);

private:

  u_char _status;
  u_char * _cache;
  int    _length;
};

#endif  // __CACHE_H__
