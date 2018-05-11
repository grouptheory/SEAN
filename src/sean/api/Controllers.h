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

#ifndef __CONTROLLERS_H__
#define __CONTROLLERS_H__
#ifndef LINT
static char const _Controllers_h_rcsid_[] =
"$Id: Controllers.h,v 1.7 1998/08/18 15:05:32 bilal Exp $";
#endif

#include <DS/containers/dictionary.h>
#include <FW/kernel/Handlers.h>
#include <FW/kernel/SimEntity.h>
class ATM_Call;
class ATM_Service;
class ATM_Leaf;
class Controller;
class Leaf_Initiated_ATM_Call;

#include "Signalling_API.h"
#include "Callback_Mgmt.h"

//-----------------------------------------------------------
class Controller {
private:
  virtual void PeriodicCallback(int code)                    { }
  virtual void Boot(ATM_Interface& i)                        { }

  virtual void Incoming_LIJ(Outgoing_ATM_Call& call)         { }
  virtual void Call_Active(ATM_Call& call)                   { }
  virtual void Call_Inactive(ATM_Call& call)                 { }

  virtual void Leaf_Active(ATM_Leaf& leaf, ATM_Call& call)   { }
  virtual void Leaf_Inactive(ATM_Leaf& leaf, ATM_Call& call) { }

  virtual void Service_Active(ATM_Service& service)          { }
  virtual void Service_Inactive(ATM_Service& service)        { }
  virtual void Incoming_Call(ATM_Service& service)           { }

  virtual void Incoming_Call_From_Root(Leaf_Initiated_ATM_Call& lic)   { }

  virtual void Incoming_Data(ATM_Call& call)                 { }

protected:
  bool RegisterPeriodicCallback(int code, double period);
  bool CancelPeriodicCallback(int code);

  Controller(void);
  virtual ~Controller(void);

private:

  void __PeriodicCallback(int code);
  void __Boot(ATM_Interface& interface);

  void __Call_Active(ATM_Call& call);
  void __Incoming_LIJ(Outgoing_ATM_Call& call);
  void __Call_Inactive(ATM_Call& call);

  void __Leaf_Active(ATM_Leaf& leaf, ATM_Call& call);
  void __Leaf_Inactive(ATM_Leaf& leaf, ATM_Call& call);

  void __Service_Active(ATM_Service& service);
  void __Service_Inactive(ATM_Service& service);
  void __Incoming_Call(ATM_Service& service);
  void __Incoming_Call_From_Root(Leaf_Initiated_ATM_Call& call);

  void mark_as_booted(void);
  bool has_booted(void);

  bool abandon_managed_call(ATM_Attributes* c);
  bool adopt_managed_call(ATM_Attributes* c);
  bool abandon_managed_service(ATM_Attributes* c);
  bool adopt_managed_service(ATM_Attributes* c);
  bool abandon_managed_interface(ATM_Interface* c);
  bool adopt_managed_interface(ATM_Interface* c);

  //---internal--

  int _my_boot_code;

  static API_SimEntity* _api_simentity;
  static int            _num_controllers;
  dictionary<int,API_Controller_CallbackTimer*> _periodic_callbacks;

  list<ATM_Attributes*> _calls;
  list<ATM_Attributes*> _services;
  list<ATM_Interface*>  _interfaces;

  // friends ----

  friend API_SimEntity* theAPISimEntity(void);
  friend void API_SimEntity::Interrupt(SimEvent* e);
  friend class Delayed_User_Callback;
  friend class API_Controller_CallbackTimer;

  friend void Incoming_ATM_Call::state_has_changed(API_FSM::api_fsm_state from, 
						   API_FSM::api_fsm_state to,
						   ATM_Call* newc,
						   ATM_Leaf* newl);
  friend void Outgoing_ATM_Call::state_has_changed(API_FSM::api_fsm_state from, 
						   API_FSM::api_fsm_state to,
						   ATM_Call* newc,
						   ATM_Leaf* newl);

  friend void ATM_Leaf::state_has_changed(API_FSM::api_fsm_state from, 
					  API_FSM::api_fsm_state to,
					  ATM_Call* newc,
					  ATM_Leaf* newl);
  friend void ATM_Service::state_has_changed(API_FSM::api_fsm_state from, 
					     API_FSM::api_fsm_state to,
					     ATM_Call* newc,
					     ATM_Leaf* newl);

  friend void Outgoing_ATM_Call::induced_teardown(void);

  friend class Leaf_Initiated_ATM_Call;
  friend class ATM_Call;
  friend class ATM_Service;
  friend class ATM_Attributes;
  friend class ATM_Interface;
};


int compare(ATM_Call *const & c1, ATM_Call *const & c2);
int compare(ATM_Service *const & s1, ATM_Service *const & s2);

#endif
