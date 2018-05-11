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
#ifndef __ATM_INTERFACE_H__
#define __ATM_INTERFACE_H__
#ifndef LINT
static char const _ATM_Interface_h_rcsid_[] =
"$Id: ATM_Interface.h,v 1.4 1998/07/31 02:59:24 bilal Exp $";
#endif

#include <sys/types.h>
#include <DS/containers/dictionary.h>
#include "Memory_Manager.h"

class Conduit;
class Terminal;
class ATM_Call;
class ATM_Service;
class ATM_Leaf;
class FSM_ATM_Call;
class FSM_ATM_Service;
class FSM_ATM_Leaf;
class Visitor;
class Controller;
class abstract_local_id;

//------------------------------------------------

#define API_IPC         "API_IPC"
#define API_TRANSLATOR  "API_Translator"
#define API_OUTER_OP    "Outer_OpMux"
#define API_CALLS_BLOCK "CallsBlock"
#define API_SERVS_BLOCK "ServicesBlock"
#define API_INNER_OP    "Inner_OpMux"
#define API_INJECTOR    "API_Injector"

class ATM_Interface;
class tcp_ipc_layer;


class ATM_Interface : public Recyclable {
public:

  friend ATM_Interface& Open_ATM_Interface(const char* devname, int ccdport);
  void Suicide(void);

  bool Associate_Controller(Controller& c);
  bool Divorce_Controller(Controller& c);
  
private:
  ATM_Interface(const char* devname, int ccdport);
  virtual ~ATM_Interface();

  bool Ready_To_Die(void);

  tcp_ipc_layer*      _ipc_injector_terminal;

  Conduit*       _ipc;
  Conduit*       _translator;
  Conduit*       _outside_op_mux;
  Conduit*       _calls_block;
  Conduit*       _services_block;
  Conduit*       _inside_op_mux;
  Conduit*       _api_injector;
  Terminal*      _api_injector_terminal;

  void inject_visitor(Visitor* v);

  FSM_ATM_Call* make_call(int& the_call_id);

  FSM_ATM_Service* make_service(int& the_call_id);
  FSM_ATM_Leaf* make_leaf(int cid, int& the_leaf_id);

  bool kill_call(int cid);
  bool kill_service(int sid);
  bool kill_leaf(int cid, int lid);

  FSM_ATM_Call* _new_call_fsm;
  int _new_call_id;
  int _next_call_id;
  
  FSM_ATM_Service* _new_service_fsm;
  int _new_service_id;
  int _next_service_id;

  FSM_ATM_Leaf* _new_leaf_fsm;
  int _new_leaf_id;
  dictionary <int,int> _next_leaf_id;

  dictionary<int, FSM_ATM_Call*>    _calls;
  dictionary<int, FSM_ATM_Service*> _services;
  void notify_of_boot(abstract_local_id* connection_id);
  bool _alive;
  abstract_local_id* _connection_id;

  list<Controller*> _associated_controllers;
  void orphaned_by_controller(Controller* abandoner);

  void Steal_Time(void);

  friend class ATM_Attributes;
  friend class ATM_Call;
  friend class FSM_ATM_Call;
  friend class FSM_ATM_Service;
  friend class FSM_ATM_Leaf;
  friend class Controller;
  friend class API_PartyFactory;
  friend class Memory_Manager;
  friend class API_Translator;

  friend int sim_interface_write(int fd, caddr_t buffer, int len);
};

void SEAN_Run(void);
void SEAN_Halt(void);


#endif



