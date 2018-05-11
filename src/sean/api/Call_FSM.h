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

#ifndef __CALL_FSM_H__
#define __CALL_FSM_H__
#ifndef LINT
static char const _Call_FSM_h_rcsid_[] =
"$Id: Call_FSM.h,v 1.1 1998/07/20 11:10:34 bilal Exp $";
#endif

#include <FW/actors/State.h>
class VisitorType;
class sean_Visitor;
class ATM_Attributes;
class ATM_Leaf;
class ATM_Call;
class ATM_Interface;

#include "API_fsms.h"
//------------------------------------------------

class FSM_ATM_Call : public API_FSM {
public:
  State* Handle(Visitor* v);
  void Interrupt(class SimEvent *e);

  void Print(void) const;

private:
friend class ATM_Call;
friend class API_Single_Call;

  FSM_ATM_Call(int id, ATM_Interface* master);

  ATM_Interface* _master_interface;
  bool           _virgin;
  int            _parties_fsms;

protected:
  virtual ~FSM_ATM_Call();
  void set_queue_to_empty(void);
};

#endif



