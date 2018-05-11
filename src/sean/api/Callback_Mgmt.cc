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
static char const _Callback_mgmt_cc_rcsid_[] =
"$Id: Callback_Mgmt.cc,v 1.5 1998/08/10 11:54:47 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "Controllers.h"
#include "Callback_Mgmt.h"
#include "Signalling_API.h"
#include <FW/kernel/SimEvent.h>
#include <sean/xti/sean_io.h>

//-----------------------------------------------------------
// for the interface-related callbacks

Delayed_User_Callback::Delayed_User_Callback(user_callback_code code, Controller* cont, ATM_Interface* interface) 
  : TimerHandler(0.0), _code(code), _c(0), _s(0), _l(0), _cont(cont), _interface(interface) {
    set_name();
}

//------------------------------------------------
// for call-related callbacks

Delayed_User_Callback::Delayed_User_Callback(user_callback_code code, Controller* cont, ATM_Call* c) 
  : TimerHandler(0.0), _code(code), _c(c), _s(0), _l(0), _cont(cont), _interface(0) {
    set_name();
}

//------------------------------------------------
// for leaf-related callbacks

Delayed_User_Callback::Delayed_User_Callback(user_callback_code code, Controller* cont, ATM_Leaf* l, ATM_Call* c) 
  : TimerHandler(0.0), _code(code), _c(c), _s(0), _l(l), _cont(cont), _interface(0)  {
    set_name();
}

//------------------------------------------------
// for service-related callbacks

Delayed_User_Callback::Delayed_User_Callback(user_callback_code code, Controller* cont, ATM_Service* s)
  : TimerHandler(0.0), _code(code), _s(s), _c(0), _l(0), _cont(cont), _interface(0)  {
    set_name();
}

//------------------------------------------------
void Delayed_User_Callback::Set_Associated_IO(sean_io* sio) {
  _sio = sio;
}


//------------------------------------------------
Delayed_User_Callback::~Delayed_User_Callback() {
}

//------------------------------------------------
void Delayed_User_Callback::set_name(void) {
  switch(_code) {
  case call_active_cc_c:
    sprintf(_name,"call_active");
    break;
  case call_inactive_cc_c:
    sprintf(_name,"call_inactive");
    break;
  case call_incoming_lij_cc_c:
    sprintf(_name,"call_incoming_lij");
    break;
  case serv_active_sc_s:
    sprintf(_name,"serv_active");
    break;
  case serv_inactive_sc_s:
    sprintf(_name,"serv_inactive");
    break;
  case serv_incoming_call_sc_s:
    sprintf(_name,"serv_incoming_call");
    break;
  case leaf_active_cc_l_c:
    sprintf(_name,"leaf_active");
    break;
  case leaf_inactive_cc_l_c:
    sprintf(_name,"leaf_inactive");
    break;
  case boot_cc_i: 
    sprintf(_name,"boot_interface");
    break;
  case data_cc_c: 
    sprintf(_name,"data_arrived");
    break;
  default:
    sprintf(_name,"unknown");
    break;
  }
}

//------------------------------------------------
const char* Delayed_User_Callback::get_name(void) {
  return _name;
}

//------------------------------------------------
void Delayed_User_Callback::Callback(void) {

  API_SimEntity* apise = theAPISimEntity();
  bool err = apise->is_callback_active();
  if (err) {
      diag("api.callback",DIAG_FATAL, "!!! one Delayed_User_Callback occurring while a user callback is executing\n");
  }

  bool reregister = false;

  apise->user_callback_starting();
  switch(_code) {
  case call_active_cc_c:
    diag("api.callback",DIAG_INFO, "!!! Starting Delayed_User_Callback <Call_Active>...\n");
    _cont->Call_Active(*_c);
    diag("api.callback",DIAG_INFO, "!!! Completed Delayed_User_Callback <Call_Active>.\n");
    break;
  case call_inactive_cc_c:
    diag("api.callback",DIAG_INFO, "!!! Starting Delayed_User_Callback <Call_Inactive>...\n");
    _cont->Call_Inactive(*_c);
    diag("api.callback",DIAG_INFO, "!!! Completed Delayed_User_Callback <Call_Inactive>.\n");
    break;
  case call_incoming_lij_cc_c:
    diag("api.callback",DIAG_INFO, "!!! Starting Delayed_User_Callback <Call_Incoming_LIJ>...\n");
    _cont->Incoming_LIJ(*(Outgoing_ATM_Call*)_c);
    diag("api.callback",DIAG_INFO, "!!! Completed Delayed_User_Callback <Call_Incoming_LIJ>.\n");
    break;
    
  case serv_active_sc_s:
    diag("api.callback",DIAG_INFO, "!!! Starting Delayed_User_Callback <Service_Active>...\n");
    _cont->Service_Active(*_s);
    diag("api.callback",DIAG_INFO, "!!! Completed Delayed_User_Callback <Service_Active>.\n");
    break;
  case serv_inactive_sc_s:
    diag("api.callback",DIAG_INFO, "!!! Starting Delayed_User_Callback <Service_Inactive>...\n");
    _cont->Service_Inactive(*_s);
    diag("api.callback",DIAG_INFO, "!!! Completed Delayed_User_Callback <Service_Inactive>.\n");
    break;
  case serv_incoming_call_sc_s:
    diag("api.callback",DIAG_INFO, "!!! Starting Delayed_User_Callback <Service_Incoming_Call>...\n");
    _cont->Incoming_Call(*_s);
    diag("api.callback",DIAG_INFO, "!!! Completed Delayed_User_Callback <Service_Incoming_Call>.\n");
    break;
    
  case leaf_active_cc_l_c:
    diag("api.callback",DIAG_INFO, "!!! Starting Delayed_User_Callback <Leaf_Active>...\n");
    _cont->Leaf_Active(*_l, *_c);
    diag("api.callback",DIAG_INFO, "!!! Completed Delayed_User_Callback <Leaf_Active>.\n");
    break;
  case leaf_inactive_cc_l_c:
    diag("api.callback",DIAG_INFO, "!!! Starting Delayed_User_Callback <Leaf_Inactive>...\n");
    _cont->Leaf_Inactive(*_l, *_c);
    diag("api.callback",DIAG_INFO, "!!! Completed Delayed_User_Callback <Leaf_Inactive>.\n");
    break;
    
  case boot_cc_i: 
    diag("api.callback",DIAG_INFO, "!!! Starting Delayed_User_Callback <Boot>...\n");
    _cont->Boot( *_interface );
    diag("api.callback",DIAG_INFO, "!!! Completed Delayed_User_Callback <Boot>.\n");
  break;

  case data_cc_c:
    diag("api.callback",DIAG_INFO, "!!! Starting Delayed_User_Callback <Data_Arrived>...\n");
    assert(_sio);
    _cont->Incoming_Data( *_c );
    if (_sio->peek_incoming_data_buffer() > 0) 
      reregister = true;

    diag("api.callback",DIAG_INFO, "!!! Completed Delayed_User_Callback <Data_Arrived>.\n");
    break;

  default:
    diag("api.callback",DIAG_ERROR, "!!! Observed delayed_User_Callback of unknown type.\n");
    break;
  }
  
  if (!reregister) {
    apise->dequeue(this);
  }
  else {
    diag("api.callback",DIAG_INFO, "!!! reregistering Delayed_User_Callback due to partial handling of <Data_Arrived>.\n");
    Register(this);
  }

  apise->user_callback_ending();
}

//-----------------------------------------------------------
//-----------------------------------------------------------
API_Controller_CallbackTimer::
API_Controller_CallbackTimer(int api_timer_id, 
			     double period, 
			     Controller* master) 
: TimerHandler(master->_api_simentity,period),
  _api_timer_id(api_timer_id), _master(master), 
  _controller_cb_period(period) { }

//------------------------------------------------
API_Controller_CallbackTimer::~API_Controller_CallbackTimer() { }

//------------------------------------------------
void API_Controller_CallbackTimer::Callback(void) {
  Register(this);
  _master->__PeriodicCallback(_api_timer_id);
}

//-----------------------------------------------------------
//-----------------------------------------------------------

API_SimEntity* theAPISimEntity(void) {
  if (Controller::_api_simentity==0) 
    Controller::_api_simentity=new API_SimEntity();
  return Controller::_api_simentity;
}

//------------------------------------------------
API_SimEntity::API_SimEntity(void) 
  : SimEntity(),  _in_callback(false) { }

//------------------------------------------------
API_SimEntity::~API_SimEntity() { }

//------------------------------------------------
void API_SimEntity::Interrupt(SimEvent* e) { 
}

//------------------------------------------------
bool API_SimEntity::register_API_timer(API_Controller_CallbackTimer* cbt) {
  return Register(cbt);
}

//------------------------------------------------
bool API_SimEntity::cancel_API_timer(API_Controller_CallbackTimer* cbt) {
  return Cancel(cbt);
}

//------------------------------------------------
void API_SimEntity::user_callback_starting(void) {
  _in_callback = true;
}

//------------------------------------------------
void API_SimEntity::user_callback_ending(void) {
  _in_callback = false;
}

//------------------------------------------------
bool API_SimEntity::is_callback_active(void) {
  return _in_callback;
}

//------------------------------------------------
Delayed_User_Callback * API_SimEntity::enqueue(user_callback_code code, Controller* cont, ATM_Interface* interface) {
  Delayed_User_Callback * duc = new Delayed_User_Callback(code,cont,interface);
  _controller_delayed_actions.insert(cont,duc);
  Register(duc);
  return duc;
}

//------------------------------------------------
Delayed_User_Callback * API_SimEntity::enqueue(user_callback_code code, Controller* cont, ATM_Call* c) {
  Delayed_User_Callback * duc = new Delayed_User_Callback(code,cont,c);
  _call_delayed_actions.insert(c,duc);
  _controller_delayed_actions.insert(cont,duc);
  Register(duc);
  return duc;
}

//------------------------------------------------
Delayed_User_Callback * API_SimEntity::enqueue(user_callback_code code, Controller* cont, ATM_Leaf* l, ATM_Call* c) {
  Delayed_User_Callback * duc = new Delayed_User_Callback(code,cont,l,c);
  _call_delayed_actions.insert(c,duc);
  _leaf_delayed_actions.insert(l,duc);
  _controller_delayed_actions.insert(cont,duc);
  Register(duc);
  return duc;
}

//------------------------------------------------
Delayed_User_Callback * API_SimEntity::enqueue(user_callback_code code, Controller* cont, ATM_Service* s) {
  Delayed_User_Callback * duc = new Delayed_User_Callback(code,cont,s);
  _service_delayed_actions.insert(s,duc);
  _controller_delayed_actions.insert(cont,duc);
  Register(duc);
  return duc;
}

//------------------------------------------------
void API_SimEntity::dequeue(Delayed_User_Callback* d) {
  dic_item di, magic;
  Delayed_User_Callback* cb;
  bool done = false;

  do {
    magic = di = 0;
    forall_items(di,_call_delayed_actions) {
      cb = _call_delayed_actions.inf(di);
      if (cb == d) magic = di;
    }
    if (magic) {
      _call_delayed_actions.del_item(magic);
      diag("api.callback",DIAG_INFO, "!!! Normal dequeueing of delayed call callback %lx <%s>\n",d,d->get_name());
      done = true;
    }
  }
  while (di);

  do {
    magic = di = 0;
    forall_items(di,_service_delayed_actions) {
      cb = _service_delayed_actions.inf(di);
      if (cb == d) magic = di;
    }
    if (magic) {
      _service_delayed_actions.del_item(magic);
      diag("api.callback",DIAG_INFO, "!!! Normal dequeueing of delayed service callback %lx <%s>\n",d,d->get_name());
      done = true;
    }
  }
  while (di);

  do {
    magic = di = 0;
    forall_items(di,_leaf_delayed_actions) {
      cb = _leaf_delayed_actions.inf(di);
      if (cb == d) magic = di;
    }
    if (magic) {
      _leaf_delayed_actions.del_item(magic);
      diag("api.callback",DIAG_INFO, "!!! Normal dequeueing of delayed leaf callback %lx <%s>\n",d,d->get_name());
      done = true;
    }
  }
  while (di);


  do {
    magic = di = 0;
    forall_items(di,_controller_delayed_actions) {
      cb = _controller_delayed_actions.inf(di);
      if (cb == d) magic = di;
    }
    if (magic) {
      _controller_delayed_actions.del_item(magic);
      diag("api.callback",DIAG_INFO, "!!! Normal dequeueing of delayed controller callback %lx <%s>\n",d,d->get_name());
      done = true;
    }
  }
  while (di);

  if (!done)
    diag("api.callback",DIAG_WARNING, "!!! Failed dequeueing of any delayed callback %lx <%s>\n",d,d->get_name());
}

//------------------------------------------------

void API_SimEntity::preempt_purge(ATM_Call*    c) {
  dic_item di;
  Delayed_User_Callback* cb;
  bool done = false;

  do{
    di = _call_delayed_actions.lookup(c);
    if (di) {
      cb = _call_delayed_actions.inf(di);
      diag("api.callback",DIAG_INFO, 
	   "!!! Successful pre-emptive purge of delayed callback <%s> for call %lx\n",cb->get_name(),c);
      _call_delayed_actions.del_item(di);
      done = true;
    }
  }
  while (di);

  if (!done) {
    diag("api.callback",DIAG_WARNING, 
	 "!!! Nothing to pre-emptive purge for call %lx\n",c);
  }
}

//------------------------------------------------

void API_SimEntity::preempt_purge(ATM_Service* s) {
  dic_item di;
  Delayed_User_Callback* cb;
  bool done = false;
  
  do{
    di = _service_delayed_actions.lookup(s);
    if (di) {
      cb = _service_delayed_actions.inf(di);
      diag("api.callback",DIAG_INFO, 
	   "!!! Successful pre-emptive purge of delayed callback <%s> for service %lx\n",cb->get_name(),s);
      _service_delayed_actions.del_item(di);
      done = true;
    }
  }
  while (di);

  if (!done) {
    diag("api.callback",DIAG_WARNING, 
	 "!!! Nothing to pre-emptive purge for service %lx\n",s);
  }
}

//------------------------------------------------

void API_SimEntity::preempt_purge(ATM_Leaf*    l) {
  dic_item di;
  Delayed_User_Callback* cb;
  bool done = false;
  
  do{
    di = _leaf_delayed_actions.lookup(l);
    if (di) {
      cb = _leaf_delayed_actions.inf(di);
      diag("api.callback",DIAG_INFO, 
	   "!!! Successful pre-emptive purge of delayed callback <%s> for leaf %lx\n",cb->get_name(),l);
      _leaf_delayed_actions.del_item(di);
      done = true;
    }
  }
  while (di);

  if (!done) {
    diag("api.callback",DIAG_WARNING, 
	 "!!! Nothing to pre-emptive purge for leaf %lx\n",l);
  }
}


//------------------------------------------------

void API_SimEntity::preempt_purge(Controller* k) {
  dic_item di;
  Delayed_User_Callback* cb;
  bool done = false;
  
  do{
    di = _controller_delayed_actions.lookup(k);
    if (di) {
      cb = _controller_delayed_actions.inf(di);
      diag("api.callback",DIAG_INFO, 
	   "!!! Successful pre-emptive purge of delayed callback <%s> for controller %lx\n",cb->get_name(),k);
      _controller_delayed_actions.del_item(di);
      done = true;
    }
  }
  while (di);

  if (!done) {
    diag("api.callback",DIAG_WARNING, 
	 "!!! Nothing to pre-emptive purge for controller %lx\n",k);
  }
}









