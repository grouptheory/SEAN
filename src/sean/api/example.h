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

#ifndef LINT
static char const _linktst_h_rcsid_[] =
"$Id: example.h,v 1.7 1998/09/28 12:31:16 bilal Exp $";
#endif
// -*- C++ -*-
#include "Controllers.h"
#include <DS/containers/list.h>

class ATM_Interface;
class Incoming_ATM_Call;
class Outgoing_ATM_Call;
class ATM_Service;

class My_Call_Manager : public Controller {
public:
  My_Call_Manager(ATM_Interface& i, bool silent = false);
  virtual ~My_Call_Manager();

  void PeriodicCallback(int code);

  void Boot(ATM_Interface& interface);
  void Call_Active(ATM_Call& call);
  void Incoming_LIJ(ATM_Call& call);
  void Call_Inactive(ATM_Call& call);
  void Incoming_Data(ATM_Call& call);

  void Leaf_Active(class ATM_Leaf & leaf, class ATM_Call & c);
  void Leaf_Inactive(class ATM_Leaf & leaf, class ATM_Call & c);

  // no op
       void Service_Active(ATM_Service& service) { }
  void Service_Inactive(ATM_Service& service)    { }
  void Incoming_Call(ATM_Service& service)       { }

private:
  ATM_Interface* _interface;
  int            _counter;
  const int      _MYCODE = 666;

  list<Incoming_ATM_Call*> _incoming_calls;
  list<Outgoing_ATM_Call*> _outgoing_calls;

  bool _silent;
};

class My_Service_Manager : public Controller {
public:
  My_Service_Manager(ATM_Interface& i);
  virtual ~My_Service_Manager();

  void PeriodicCallback(int code);

  void Boot(ATM_Interface& interface);
  void Service_Active(ATM_Service& service);
  void Service_Inactive(ATM_Service& service);
  void Incoming_Call(ATM_Service& service);

  // no op
  void Call_Active(ATM_Call& call)                                { }
  void Incoming_LIJ(ATM_Call& call)                               { }
  void Call_Inactive(ATM_Call& call)                              { }
  void Incoming_Data(ATM_Call& call)                              { }
  void Leaf_Active(class ATM_Leaf & leaf, class ATM_Call & c)     { }
  void Leaf_Inactive(class ATM_Leaf & leaf, class ATM_Call & c)   { }

private:
  ATM_Interface* _interface;
  int            _counter;
  const int      _MYCODE = 777;

  ATM_Service* _service;
};


