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
#ifndef __ACCESSOR_H__
#define __ACCESSOR_H__
#if !defined(LINT)
static char const _Accessor_h_rcsid_[] =
"$Id: Accessor.h,v 1.1 1999/01/13 19:12:11 mountcas Exp $";
#endif
 
#include <FW/actors/Actor.h>
class Conduit;
class Mux;
class Visitor;

class Accessor : public Actor {
  friend class Mux;
public:

  Accessor();

  // Find destination for this visitor.
  virtual Conduit * GetNextConduit(Visitor * v) = 0;

protected:

  virtual ~Accessor();

  void SetVisitorLast(Visitor * v);

  virtual bool Broadcast(Visitor * v) = 0;

  // Allows the factory to update the accessor's map.
  virtual bool Add(Conduit * c, Visitor * v) = 0;
  virtual bool Del(Conduit * c) = 0;
  virtual bool Del(Visitor * v) = 0;
};

#endif // __ACCESSOR_H__



