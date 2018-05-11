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
static char const _ATM_Interface_cc_rcsid_[] =
"$Id: ATM_Interface.cc,v 1.7 1998/08/12 20:15:36 bilal Exp $";
#endif
#include <common/cprototypes.h>
// -*- C++ -*-

#include "ATM_Interface.h"
#include "ATM_Interface_Components.h"
#include "Signalling_API.h"
#include "Controllers.h"
#include "signal_handlers.h"


#include <FW/behaviors/Mux.h>
#include <FW/behaviors/Protocol.h>
#include <FW/behaviors/Factory.h>
#include <FW/behaviors/Cluster.h>
#include <FW/behaviors/Adapter.h>
#include <FW/actors/Accessor.h>
#include <FW/actors/State.h>
#include <FW/actors/Creator.h>
#include <FW/actors/Expander.h>
#include <FW/actors/Terminal.h>
#include <FW/basics/Visitor.h>
#include <FW/basics/Conduit.h>
#include <FW/kernel/Kernel.h>

#include <FW/basics/diag.h>

#include <sean/ipc/tcp_ipc.h>
#include <sean/cc/sean_Visitor.h>

#include <sean/templates/constants.h>

// -------------------------------------------------------------

ATM_Interface& Open_ATM_Interface(const char* devname, int ccdport) {
  ATM_Interface * atmi = new ATM_Interface(devname, ccdport);
  return * atmi;
}

void ATM_Interface::Suicide(void) {
  dic_item di;
  if ( !_services.empty() ) {
    diag("api.interface",DIAG_INFO,"ATM_Interface destructor releasing some leftover Services\n");
    forall_items(di,_services) {
      FSM_ATM_Service* fs = _services.inf(di);
      ATM_Service* serv = ((ATM_Service*)(fs->_attr));
      serv->Deregister();
    }
  }

  if ( !_calls.empty() ) {
    diag("api.interface",DIAG_INFO,"ATM_Interface destructor releasing some leftover Calls\n");
    forall_items(di,_calls) {
      FSM_ATM_Call* fc = _calls.inf(di);
      ((ATM_Call*)(fc->_attr))->Set_cause( ie_cause::normal_unspecified );
      ((ATM_Call*)(fc->_attr))->TearDown();
    }
  }


  list_item li;
  while (!_associated_controllers.empty()) {
    li = _associated_controllers.first();
    Controller* c = _associated_controllers.inf(li);
    _associated_controllers.del_item(li);
    c->abandon_managed_interface(this);
  }

  The_Recycle_Bin().Digest_Interface( this );
}

// -------------------------------------------------------------

ATM_Interface::ATM_Interface(const char* devname, int ccdport) 
  :_connection_id(0) { 

  _ipc_injector_terminal = new tcp_ipc_layer();
  Terminal* t = _ipc_injector_terminal;
  Adapter* ad = new Adapter(t);
  _ipc = new Conduit(API_IPC,ad);

  State* s = new API_Translator(ccdport, this);
  Protocol* p = new Protocol(s);
  _translator = new Conduit(API_TRANSLATOR,p);

  Accessor* ac = new API_Op_Accessor();  
  Mux* m = new Mux(ac);
  _outside_op_mux = new Conduit(API_OUTER_OP,m);

  Expander* e = new API_Calls_Block(this);
  Cluster* cl = new Cluster(e);
  _calls_block=new Conduit(API_CALLS_BLOCK,cl);

  e = new API_Services_Block(this);
  cl = new Cluster(e);
  _services_block=new Conduit(API_SERVS_BLOCK,cl);

  ac = new API_Op_Accessor();  
  m = new Mux(ac);
  _inside_op_mux = new Conduit(API_INNER_OP,m);

  t = new API_Injector_Terminal();
  _api_injector_terminal = t;
  ad = new Adapter(t);
  _api_injector = new Conduit(API_INJECTOR,ad);

  sean_Visitor* vc = new sean_Visitor(sean_Visitor::api_call_construction);
  sean_Visitor* vs = new sean_Visitor(sean_Visitor::api_service_construction);
  
  Join(A_half(_ipc),A_half(_translator));
  Join(B_half(_translator),A_half(_outside_op_mux));
  Join(B_half(_outside_op_mux),A_half(_calls_block),vc,0);
  Join(B_half(_inside_op_mux),B_half(_calls_block),vc,0);
  Join(B_half(_outside_op_mux),A_half(_services_block),vs,0);
  Join(B_half(_inside_op_mux),B_half(_services_block),vs,0);
  Join(A_half(_api_injector),A_half(_inside_op_mux));
  
  vc->Suicide();
  vs->Suicide();

  _next_call_id = 2;
  _next_service_id = 2;

  _new_call_id =    NO_CID;
  _new_service_id = NO_SID;
  _new_leaf_id =    NO_LID;
   
  _new_call_fsm = 0;
  _new_service_fsm = 0;
  _new_leaf_fsm = 0;

  _alive = false;
}

// -------------------------------------------------------------
bool ATM_Interface::Ready_To_Die(void) {
  return false;
}

// -------------------------------------------------------------
ATM_Interface::~ATM_Interface() { 
}

// -------------------------------------------------------------

FSM_ATM_Call* ATM_Interface::make_call(int& the_call_id) { 
  _new_call_fsm = 0; _new_call_id = NO_CID;
  int req_id;
  sean_Visitor* vop = new sean_Visitor(sean_Visitor::api_call_construction);
  if (!the_call_id)
    req_id = _next_call_id;
  else
    req_id = the_call_id;
  vop->set_cid(req_id);

  inject_visitor(vop);
  assert (_new_call_id == req_id);

  _calls.insert(_new_call_id,_new_call_fsm);
  _next_leaf_id.insert(_new_call_id,2);

  diag("api.interface",DIAG_INFO,"ATM_Interface made a call with ID %d\n",_new_call_id);

  if (!the_call_id)
    _next_call_id += 2;

  the_call_id = _new_call_id;
  return _new_call_fsm;
}

// -------------------------------------------------------------

FSM_ATM_Service* ATM_Interface::make_service(int& the_service_id) { 
  _new_service_fsm = 0; _new_service_id = NO_SID;
  int req_id;
  sean_Visitor* vop = new sean_Visitor(sean_Visitor::api_service_construction);
  if (!the_service_id)
    req_id = _next_service_id;
  else
    req_id = the_service_id;
  vop->set_sid(req_id);

  inject_visitor(vop);

  assert (_new_service_id == req_id);

  _services.insert(_new_service_id,_new_service_fsm);
  diag("api.interface",DIAG_INFO,"ATM_Interface made a service with ID %d\n",_new_service_id);
  _next_service_id += 2;

  FSM_ATM_Service* answer = _new_service_fsm;
  the_service_id = _new_service_id;

  return _new_service_fsm;
}

// -------------------------------------------------------------

FSM_ATM_Leaf* ATM_Interface::make_leaf(int cid, int& the_leaf_id) { 
  _new_leaf_fsm = 0; _new_leaf_id = NO_LID;
  int req_id;
  sean_Visitor* vop = new sean_Visitor(sean_Visitor::api_leaf_construction);

  req_id = the_leaf_id;   // always, an explicit request

  vop->set_lid(req_id);
  vop->set_cid(cid);
  inject_visitor(vop);

  assert (_new_leaf_id == req_id);

  diag("api.interface",DIAG_INFO,"ATM_Interface made a leaf with ID %d\n",_new_leaf_id);
  if (!the_leaf_id) {
    dic_item di;
    di = _next_leaf_id.lookup(cid);
    assert(di);
    _next_leaf_id.change_inf(di, req_id+2);
  }

  the_leaf_id = _new_leaf_id;

  return _new_leaf_fsm;
}

// -------------------------------------------------------------

bool ATM_Interface::kill_call(int cid) {
  sean_Visitor* vop = new sean_Visitor(sean_Visitor::api_call_destruction);
  vop->set_cid(cid);
  inject_visitor(vop);

  assert (_new_call_id == cid);

  dic_item li =_calls.lookup(cid);
  assert(li);
  _calls.del_item(li);
  diag("api.interface",DIAG_INFO,"ATM_Interface killed call with ID %d\n",_new_call_id);

  return true;
}

// -------------------------------------------------------------

bool ATM_Interface::kill_service(int sid) { 
  sean_Visitor* vop = new sean_Visitor(sean_Visitor::api_service_destruction);
  vop->set_sid(sid);
  inject_visitor(vop);

  assert (_new_service_id == sid);

  dic_item li =_services.lookup(sid);
  assert(li);
  _services.del_item(li);
  diag("api.interface",DIAG_INFO,"ATM_Interface killed service with ID %d\n",_new_service_id);

  return true; 
}

// -------------------------------------------------------------

bool ATM_Interface::kill_leaf(int cid, int lid) {   
  sean_Visitor* vop = new sean_Visitor(sean_Visitor::api_leaf_destruction);
  vop->set_cid(cid);
  vop->set_lid(lid);
  inject_visitor(vop);

  assert (_new_leaf_id == lid);

  diag("api.interface",DIAG_INFO,"ATM_Interface killed leaf with ID %d, belonging to call with ID %d\n",lid,cid);
  return true; 
}

// -------------------------------------------------------------

void ATM_Interface::inject_visitor(Visitor* v) {
  if (v) _api_injector_terminal->Inject(v); 
}


// -------------------------------------------------------------

void SEAN_Run(void) {   
  theKernel().SetSpeed(Kernel::REAL_TIME);
  cout << "SEAN is Running...\n";  

  signal(SIGINT,  int_trapper);
  signal(SIGHUP,  hup_trapper);
  signal(SIGQUIT, quit_trapper);

  theKernel().Run();
}

// -------------------------------------------------------------

void SEAN_Halt(void) {   
  cout << "\nSEAN is Halting...\n";
  theKernel().Stop();
  exit(0);
}

// -------------------------------------------------------------

bool ATM_Interface::Associate_Controller(Controller& c) {
  bool ret = false;
  list_item li = _associated_controllers.search(&c);
  if ( ! li ) {
    _associated_controllers.append(&c);
    c.adopt_managed_interface(this);
    ret = true;
  }
  return ret;
}

// -------------------------------------------------------------

bool ATM_Interface::Divorce_Controller(Controller& c) {
  bool ret = false;
  list_item li = _associated_controllers.search(&c);
  if (li) {
    _associated_controllers.del_item(li);
    c.abandon_managed_interface(this);
    ret = true;
  }
  return ret;
}

// -------------------------------------------------------------

void ATM_Interface::orphaned_by_controller(Controller* abandoner) {
  list_item li = _associated_controllers.search( abandoner );
  assert (li);
  _associated_controllers.del_item(li);
}

// -------------------------------------------------------------

void ATM_Interface::notify_of_boot(abstract_local_id* connection_id) {
  list_item li;
  _connection_id = connection_id;
  forall_items ( li, _associated_controllers) {
    Controller* c = _associated_controllers.inf(li);
    c->__Boot(*this);
  }
}

// -------------------------------------------------------------

void ATM_Interface::Steal_Time(void) {
  _ipc_injector_terminal->process_all_connections();
}

