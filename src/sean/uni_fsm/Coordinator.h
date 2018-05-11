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
#ifndef __COORD_H__
#define __COORD_H__
#ifndef LINT
static char const _SSCF_h_rcsid_[] =
"$Id: Coordinator.h,v 1.1 1998/07/20 11:12:34 bilal Exp $";
#endif

#include<DS/containers/list.h>
#include <FW/kernel/Kernel.h>
#include <FW/actors/State.h>
#include <codec/uni_ie/ie.h>
#include <sean/daemon/SigStack.h>

class CoordState;
class CallTimer;

class Coordinator : public State {
public:

 enum CoordStates {
    cu0_conn_released = 0,
    cu1_await_establish = 1,
    cu2_await_release = 2,
    cu3_conn_established = 3
  };
  Coordinator(int s);
  virtual ~Coordinator();
  State * Handle(Visitor* v);
  void Interrupt(SimEvent *event);
  inline void SetT309();
  inline void StopT309();
  void ExpT309();

private:

  int _user;
  CoordStates _cs;
  CallTimer  *_t309;
  static const VisitorType* _uni_visitor_type;
  static const VisitorType* _link_visitor_type;
  list<Visitor *> _ov; // outgoing visitors waiting for Link Up
};

#endif






