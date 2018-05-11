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
static char const _Controllers_cc_rcsid_[] =
"$Id: Controllers.cc,v 1.6 1998/08/10 11:54:47 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "Controllers.h"
#include "Signalling_API.h"
#include "ATM_Interface.h"
#include <FW/kernel/SimEvent.h>

API_SimEntity* Controller::_api_simentity=0;
int Controller::_num_controllers=0;

//-----------------------------------------------------------
Controller::Controller(void) {
  _my_boot_code = 0;
  _num_controllers++;
}

//-----------------------------------------------------------
Controller::~Controller(void) { 

  if ( !has_booted() )
    _api_simentity->preempt_purge(this);

  while( !_calls.empty() ) {
    list_item li = _calls.first();
    ATM_Call* c = (ATM_Call*)( _calls.inf( li ) );
    abandon_managed_call( c );
    c->orphaned_by_controller(this);
  }

  while( !_services.empty() ) {
    list_item li = _services.first();
    ATM_Service* s = (ATM_Service*)( _services.inf( li ) );
    abandon_managed_service( s );
    s->orphaned_by_controller(this);
  }

  while ( ! _interfaces.empty() ) {
    list_item li = _interfaces.first();
    ATM_Interface* c = _interfaces.inf( li );
    abandon_managed_interface( c );
    c->orphaned_by_controller(this);
  }

  _num_controllers--;
}

//-----------------------------------------------------------
void Controller::mark_as_booted(void) {
  _my_boot_code = -1;
}

//-----------------------------------------------------------
bool Controller::has_booted(void) {
  return (_my_boot_code == -1);
}

//-----------------------------------------------------------
bool Controller::RegisterPeriodicCallback(int code, double period) {
  if (_periodic_callbacks.lookup(code)) return false;
  API_Controller_CallbackTimer* cbt= new 
    API_Controller_CallbackTimer(code,period,this);
  _periodic_callbacks.insert(code,cbt);
  _api_simentity->register_API_timer(cbt);
  return true;
}

//-----------------------------------------------------------
bool Controller::CancelPeriodicCallback(int code) {
  dic_item di;
  if (!(di=_periodic_callbacks.lookup(code))) return false;
  API_Controller_CallbackTimer* cbt= _periodic_callbacks.inf(di);
  _periodic_callbacks.del_item(di);
  _api_simentity->cancel_API_timer(cbt);
  delete cbt;
  return true;
}

//-----------------------------------------------------------
void Controller::__PeriodicCallback(int code) {
  API_SimEntity* apise = theAPISimEntity();
  bool need_to_wait = apise->is_callback_active();
  if (need_to_wait) {
    diag("api.controller",DIAG_WARNING, 
	 "!!! Users PeriodicCallback <%d> skipped, because another user callback was already running.\n",code);
  }
  else {
    diag("api.controller",DIAG_INFO, 
	 "!!! Executing Users PeriodicCallback <%d>.\n",code);
    apise->user_callback_starting();
    this->PeriodicCallback(code);
    apise->user_callback_ending();
    diag("api.controller",DIAG_INFO, 
	 "!!! Completed Users PeriodicCallback <%d>.\n",code);
  }
}

//-----------------------------------------------------------
void Controller::__Boot(ATM_Interface& interface) {
  API_SimEntity* apise = theAPISimEntity();
  bool need_to_wait = apise->is_callback_active();
  if (need_to_wait) {
    diag("api.controller",DIAG_INFO, 
	 "!!! Queued callback <Boot> because another user callback is already running.\n");
    apise->enqueue(boot_cc_i, this, &interface);
  }
  else {
    diag("api.controller",DIAG_INFO, "!!! Executing Boot Callback.% lx\n",this);
    apise->user_callback_starting();
    this->Boot(interface);
    apise->user_callback_ending();
    diag("api.controller",DIAG_INFO, "!!! Completed Boot Callback. %lx\n",this);
  }
}

//-----------------------------------------------------------
void Controller::__Call_Active(ATM_Call& call) {
  API_SimEntity* apise = theAPISimEntity();
  bool need_to_wait = apise->is_callback_active();
  if (need_to_wait) {
    diag("api.controller",DIAG_INFO, 
	 "!!! Queued callback <Call_Active> because another user callback is already running.\n");
    apise->enqueue(call_active_cc_c, this, &call);
  }
  else {
    diag("api.controller",DIAG_INFO, 
	 "!!! Executing Call_Active callback immediately.\n");
    apise->user_callback_starting();
    this->Call_Active(call);  
    apise->user_callback_ending();
    diag("api.controller",DIAG_INFO, "!!! Completed Call_Active callback\n");
  }
}

//-----------------------------------------------------------
void Controller::__Incoming_LIJ(Outgoing_ATM_Call& call) {
  API_SimEntity* apise = theAPISimEntity();
  bool need_to_wait = apise->is_callback_active();
  if (need_to_wait) {
    diag("api.controller",DIAG_INFO, 
	 "!!! Queued callback <Incoming_LIJ> because another user callback is already running.\n");
    apise->enqueue(call_incoming_lij_cc_c, this, &call);
  }
  else {
    diag("api.controller",DIAG_INFO, 
	 "!!! Executing Incoming_LIJ callback immediately.\n");
    apise->user_callback_starting();
    this->Incoming_LIJ(call);
    apise->user_callback_ending();
    diag("api.controller",DIAG_INFO, "!!! Completed Incoming_LIJ callback\n");
  }
}

//-----------------------------------------------------------
void Controller::__Call_Inactive(ATM_Call& call) {
  API_SimEntity* apise = theAPISimEntity();
  bool need_to_wait = apise->is_callback_active();
  if (need_to_wait) {
    diag("api.controller",DIAG_INFO, 
	 "!!! Queued callback <Call_Inactive> because another user callback is already running.\n");
    apise->enqueue(call_inactive_cc_c, this, &call);
  }
  else {
    diag("api.controller",DIAG_INFO, 
	 "!!! Executing Call_Inactive callback immediately.\n");
    apise->user_callback_starting();
    this->Call_Inactive(call);
    apise->user_callback_ending();
    diag("api.controller",DIAG_INFO, "!!! Completed Call_Inactive callback\n");
  }
}

//-----------------------------------------------------------
void Controller::__Leaf_Active(ATM_Leaf& leaf, ATM_Call& call) {
  API_SimEntity* apise = theAPISimEntity();
  bool need_to_wait = apise->is_callback_active();
  if (need_to_wait) {
    diag("api.controller",DIAG_INFO, 
	 "!!! Queued callback <Leaf_Active> because another user callback is already running.\n");
    apise->enqueue(leaf_active_cc_l_c, this, &leaf, &call);
  }
  else {
    diag("api.controller",DIAG_INFO, 
	 "!!! Executing Leaf_Active callback immediately.\n");
    apise->user_callback_starting();
    this->Leaf_Active(leaf, call);
    apise->user_callback_ending();
    diag("api.controller",DIAG_INFO, "!!! Completed Leaf_Active callback\n");
  }
}

//-----------------------------------------------------------
void Controller::__Leaf_Inactive(ATM_Leaf& leaf, ATM_Call& call) {
  API_SimEntity* apise = theAPISimEntity();
  bool need_to_wait = apise->is_callback_active();
  if (need_to_wait) {
    diag("api.controller",DIAG_INFO, 
	 "!!! Queued callback <Leaf_Inactive> because another user callback is already running.\n");
    apise->enqueue(leaf_inactive_cc_l_c, this, &leaf, &call);
  }
  else {
    diag("api.controller",DIAG_INFO, 
	 "!!! Executing Leaf_Inactive callback immediately.\n");
    apise->user_callback_starting();
    this->Leaf_Inactive(leaf, call);
    apise->user_callback_ending();
    diag("api.controller",DIAG_INFO, "!!! Completed Leaf_Inactive callback\n");
  }
}

//-----------------------------------------------------------
void Controller::__Service_Active(ATM_Service& service) {
  API_SimEntity* apise = theAPISimEntity();
  bool need_to_wait = apise->is_callback_active();
  if (need_to_wait) {
    diag("api.controller",DIAG_INFO, 
	 "!!! Queued callback <Service_Active> because another user callback is already running.\n");
    apise->enqueue(serv_active_sc_s, this, &service);
  }
  else {
    diag("api.controller",DIAG_INFO, 
	 "!!! Executing Service_Active callback immediately.\n");
    apise->user_callback_starting();
    this->Service_Active(service);
    apise->user_callback_ending();
    diag("api.controller",DIAG_INFO, "!!! Completed Service_Active callback\n");
  }
}

//-----------------------------------------------------------
void Controller::__Service_Inactive(ATM_Service& service) {
  API_SimEntity* apise = theAPISimEntity();
  bool need_to_wait = apise->is_callback_active();
  if (need_to_wait) {
    diag("api.controller",DIAG_INFO, 
	 "!!! Queued callback <Service_Inactive> because another user callback is already running.\n");
    apise->enqueue(serv_inactive_sc_s, this, &service);
  }
  else {
    diag("api.controller",DIAG_INFO, 
	 "!!! Executing Service_Inactive callback immediately.\n");
    apise->user_callback_starting();
    this->Service_Inactive(service);
    apise->user_callback_ending();
    diag("api.controller",DIAG_INFO, "!!! Completed Service_Inactive callback\n");
  }
}

//-----------------------------------------------------------
void Controller::__Incoming_Call(ATM_Service& service) {
  API_SimEntity* apise = theAPISimEntity();
  bool need_to_wait = apise->is_callback_active();
  if (need_to_wait) {
    diag("api.controller",DIAG_INFO, 
	 "!!! Queued callback <Service_Incoming_Call> because another user callback is already running.\n");
    apise->enqueue(serv_incoming_call_sc_s, this, &service);
  }
  else {
    diag("api.controller",DIAG_INFO, 
	 "!!! Executing Service_Incoming_Call callback immediately.\n");
    apise->user_callback_starting();
    this->Incoming_Call(service);
    apise->user_callback_ending();
    diag("api.controller",DIAG_INFO, "!!! Completed Service_Incoming_Call callback\n");
  }
}

//-----------------------------------------------------------
void Controller::__Incoming_Call_From_Root(Leaf_Initiated_ATM_Call& call) {
  API_SimEntity* apise = theAPISimEntity();
  bool need_to_wait = apise->is_callback_active();
  if (need_to_wait) {
    diag("api.controller",DIAG_INFO, 
	 "!!! FIX THIS Queued callback <LIJ_Incoming_Call> because another user callback is already running.\n");
    // apise->enqueue(serv_incoming_call_sc_s, this, &service);
  }
  else {
    diag("api.controller",DIAG_INFO, 
	 "!!! Executing LIJ_Incoming_Call callback immediately.\n");
    apise->user_callback_starting();
    this->Incoming_Call_From_Root(call);
    apise->user_callback_ending();
    diag("api.controller",DIAG_INFO, "!!! Completed LIJ_Incoming_Call callback\n");
  }
}

//-----------------------------------------------------------
int compare(ATM_Call *const & c1, ATM_Call *const & c2) {
  if (c1==c2) return 0;
  if (c1<c2) return -1;
  return 1;
}

//-----------------------------------------------------------
int compare(ATM_Service *const & s1, ATM_Service *const & s2) {
  if (s1==s2) return 0;
  if (s1<s2) return -1;
  return 1;
}


//-----------------------------------------------------------
bool Controller::abandon_managed_call(ATM_Attributes* c) {
  assert(c);
  list_item li = _calls.search(c);
  assert( li );
  _calls.del_item(li);
  return true;
}

//-----------------------------------------------------------
bool Controller::adopt_managed_call(ATM_Attributes* c) {
  assert(c);
  list_item li = _calls.search(c);
  assert( ! li );
  _calls.append(c);
  return true;
}


//-----------------------------------------------------------
bool Controller::abandon_managed_interface(ATM_Interface* c) {
  assert(c);
  list_item li = _interfaces.search(c);
  assert( li );
  _interfaces.del_item(li);
  return true;
}

//-----------------------------------------------------------
bool Controller::adopt_managed_interface(ATM_Interface* c) {
  assert(c);
  list_item li = _interfaces.search(c);
  assert( ! li );
  _interfaces.append(c);
  return true;
}

//-----------------------------------------------------------
bool Controller::abandon_managed_service(ATM_Attributes* c) {
  assert(c);
  list_item li = _services.search(c);
  assert( li );
  _services.del_item(li);
  return true;
}

//-----------------------------------------------------------
bool Controller::adopt_managed_service(ATM_Attributes* c) {
  assert(c);
  list_item li = _services.search(c);
  assert( ! li );
  _services.append(c);
  return true;
}

