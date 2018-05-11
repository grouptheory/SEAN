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
#ifndef __FIBER_H__
#define __FIBER_H__

#ifndef LINT
static char const _Fiber_h_rcsid_[] =
"$Id: Fiber.h,v 1.4 1998/07/28 13:20:17 bilal Exp $";
#endif

#include <FW/actors/State.h>
#include <sean/templates/constants.h>
#include <sean/cc/sean_Visitor.h>


class Fiber : public State {
public:

  Fiber(int portstampa, int portstampb, double t = 0.1);
  virtual ~Fiber();

  State * Handle(Visitor * v);
  void Interrupt(class SimEvent * e);
  
private:

  generic_q93b_msg* packetize(sean_Visitor* sv, 
			      sean_Visitor::sean_Visitor_Type& t);
  sean_Visitor* rebuild(sean_Visitor* sv, 
			generic_q93b_msg* m, 
			sean_Visitor::sean_Visitor_Type t);

  void rebuild(sean_Visitor* sv);
  
  int _portstampa, _portstampb;

  static const VisitorType * _uni_visitor_type;
  static const VisitorType * _data_vistype;
};

#endif
