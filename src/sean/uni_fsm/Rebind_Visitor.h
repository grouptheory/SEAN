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

//* -*- C++ -*-
#ifndef LINT
static char const _Rebind_Visitor_h_rcsid_[] =
"$Id: Rebind_Visitor.h,v 1.1 1998/08/21 15:38:16 mountcas Exp $";
#endif

#ifndef _REBIND_VISITOR_H_
#define _REBIND_VISITOR_H_


#include <FW/basics/Visitor.h>
#include <FW/kernel/Kernel.h>
#include <FW/actors/State.h>

#define REBIND_VISITOR_NAME           "Rebind_Visitor"

class sean_Visitor;
class lij_key;

class Rebind_Visitor : public Visitor {
public:
  Rebind_Visitor(sean_Visitor* sv);
  virtual ~Rebind_Visitor();

  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;

  void at(Mux * m, Accessor * a);
protected:

  Rebind_Visitor(const Rebind_Visitor & rhs);
  virtual Visitor * dup(void) const;

private:
  static vistype _my_type;

  lij_key* _key;
  int      _crv;
};

#endif
