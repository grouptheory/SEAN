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

#ifndef __CALLBACK_MGMT_H__
#define __CALLBACK_MGMT_H__
#ifndef LINT
static char const _Callback_mgmt_h_rcsid_[] =
"$Id: Callback_Mgmt.h,v 1.3 1998/08/07 20:14:41 marsh Exp $";
#endif

#include <DS/containers/dictionary.h>
#include <FW/kernel/Handlers.h>
#include <FW/kernel/SimEntity.h>
class ATM_Call;
class ATM_Service;
class ATM_Leaf;
class Controller;
class sean_io;

#include "Signalling_API.h"

//-----------------------------------------------------------
enum user_callback_code {
  call_active_cc_c,
  call_inactive_cc_c,
  call_incoming_lij_cc_c,
  serv_active_sc_s,
  serv_inactive_sc_s,
  serv_incoming_call_sc_s,
  leaf_active_cc_l_c,
  leaf_inactive_cc_l_c,
  boot_cc_i,
  data_cc_c
};

class Controller;
//-----------------------------------------------------------
// this class holds information about postponed user callbacks
// that will be delivered after the currently executing callback
// has relinquished control.
//-----------------------------------------------------------
class Delayed_User_Callback : public TimerHandler {
public:
  void Callback(void);

private:
  Delayed_User_Callback(user_callback_code code, Controller* cont, ATM_Interface* interface);
  Delayed_User_Callback(user_callback_code code, Controller* cont, ATM_Call* c);
  Delayed_User_Callback(user_callback_code code, Controller* cont, ATM_Leaf* l, ATM_Call* c);
  Delayed_User_Callback(user_callback_code code, Controller* cont, ATM_Service* s);
  virtual ~Delayed_User_Callback();

  const char* get_name(void);
  void Set_Associated_IO(sean_io* sio);

private:
  void set_name(void);

  user_callback_code   _code;
  char                 _name[255];

  ATM_Call*            _c;
  ATM_Leaf*            _l;
  ATM_Service*         _s;
  ATM_Interface*       _interface;
  Controller*          _cont;

  sean_io*             _sio;

  friend class API_SimEntity;
  friend class sean_io;
};

//-----------------------------------------------------------
// This class is used to manage periodic callbacks for 
// a programmers derived Controller classes.
//-----------------------------------------------------------
class API_Controller_CallbackTimer : public TimerHandler {
public:
  void Callback(void);

private:
  API_Controller_CallbackTimer(int api_timer_id,
			       double period, 
			       Controller* master);
  virtual ~API_Controller_CallbackTimer();

  int         _api_timer_id;
  double      _controller_cb_period;
  Controller* _master;
  bool        _in_callback;

  friend class API_SimEntity;
  friend class Controller;
};

//-----------------------------------------------------------
// This singleton SimEntity handles all timer and
// delayed callback registrations for the API internals.
//-----------------------------------------------------------
class API_SimEntity : public SimEntity {
  friend API_SimEntity* theAPISimEntity(void);
public:
  void Interrupt(SimEvent* e);

private:
  API_SimEntity(void);
  virtual ~API_SimEntity();

  bool register_API_timer(API_Controller_CallbackTimer* cbt);
  bool cancel_API_timer(API_Controller_CallbackTimer* cbt);

  void user_callback_starting(void);
  void user_callback_ending(void);
  bool is_callback_active(void);

  Delayed_User_Callback * enqueue(user_callback_code code, Controller* cont, ATM_Interface* interface);
  Delayed_User_Callback * enqueue(user_callback_code code, Controller* cont, ATM_Call* c);
  Delayed_User_Callback * enqueue(user_callback_code code, Controller* cont, ATM_Leaf* l, ATM_Call* c);
  Delayed_User_Callback * enqueue(user_callback_code code, Controller* cont, ATM_Service* s);

  void dequeue(Delayed_User_Callback* d);

  void preempt_purge(ATM_Call*    c);
  void preempt_purge(ATM_Service* s);
  void preempt_purge(ATM_Leaf*    l);
  void preempt_purge(Controller*  k);

  bool                           _in_callback;

  dictionary<ATM_Call*,    Delayed_User_Callback*>    _call_delayed_actions;
  dictionary<ATM_Service*, Delayed_User_Callback*>    _service_delayed_actions;
  dictionary<ATM_Leaf*,    Delayed_User_Callback*>    _leaf_delayed_actions;
  dictionary<Controller*,  Delayed_User_Callback*>    _controller_delayed_actions;
  
  friend class Controller;
  friend class Delayed_User_Callback;
  friend class sean_io;

  friend void ATM_Call::Suicide();
  friend void ATM_Service::Suicide();
  friend void ATM_Leaf::Suicide();
};

#endif
