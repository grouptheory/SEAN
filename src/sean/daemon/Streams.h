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
#ifndef __STREAMS_H__
#define __STREAMS_H__

#ifndef LINT
static char const _Streams_h_rcsid_[] =
"$Id: Streams.h,v 1.2 1998/07/24 11:17:57 bilal Exp $";
#endif

#include <FW/actors/State.h>
#include <FW/actors/Accessor.h>
class VisitorType;
class abstract_local_id;
class call_control;

//-------------------------------------
class StreamFilter : public State {
public:

  StreamFilter(void);

  State * Handle(Visitor * v);
  void    Interrupt(SimEvent * se);
  
  void Inform_Of_Call_Control(call_control* cc);

protected:

  virtual ~StreamFilter();

  call_control* _cc;

  static const VisitorType * _ipc_dat_vistype;
  static const VisitorType * _ipc_sig_vistype;
  static const VisitorType * _data_vistype;
};


//-------------------------------------
class StackSelector : public Accessor {
public:

  StackSelector(void);

  // Find destination for this visitor.
  Conduit * GetNextConduit(Visitor * v);

protected:

  virtual ~StackSelector();

  bool Broadcast(Visitor * v);

  // Allows the factory to update the accessor's map.
  bool Add(Conduit * c, Visitor * v);
  bool Del(Conduit * c);
  bool Del(Visitor * v);

  // Call control or UNI
  Conduit * _c;

  static const VisitorType * _sean_vistype;
  static const VisitorType * _data_vistype;
  static const VisitorType * _ipc_sig_vistype;
  static const VisitorType * _ipc_dat_vistype;
};

#endif
