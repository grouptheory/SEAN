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
#ifndef LINT
static char const _ATM_Interface_Components_cc_rcsid_[] =
"$Id: ATM_Interface_Components.cc,v 1.13 1999/03/26 21:16:50 battou Exp $";
#endif
#include <common/cprototypes.h>

#include "ATM_Interface_Components.h"
#include "API_fsms.h"
#include "ATM_Interface.h"

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
#include <FW/kernel/SimEvent.h>

#include <FW/basics/diag.h>

#include <sean/cc/keys.h>
#include <sean/cc/cc_layer.h>
#include <sean/cc/sean_Visitor.h>
#include <sean/ipc/IPC_Visitors.h>
#include <sean/ipc/tcp_ipc.h>

#include <sean/templates/constants.h>

#include <codec/q93b_msg/Buffer.h>
#include <codec/q93b_msg/generic_q93b_msg.h>

extern "C"{
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
};

const VisitorType* call_accessor::_sean_visitor_type=0;
const VisitorType* leaf_accessor::_sean_visitor_type=0;

const VisitorType* API_Translator::_sean_visitor_type=0;
const VisitorType* API_Translator::_ipc_data_type=0;
const VisitorType* API_Translator::_ipc_signalling_type=0;

const VisitorType* API_ServiceFactory::_sean_visitor_type=0;
const VisitorType* API_CallFactory::_sean_visitor_type=0;
const VisitorType* API_PartyFactory::_sean_visitor_type=0;
const VisitorType* API_Op_Accessor::_sean_visitor_type=0;
const VisitorType* API_Injector_Terminal::_sean_visitor_type=0;


API_Services_Block::API_Services_Block(ATM_Interface* i) 
{ 
  Accessor* a= new userside_serv_accessor();
  Mux* m= new Mux(a);
  _sid_outside_mux= new Conduit(API_SERVS_OUTER_MUX,m);

  Creator* c= new API_ServiceFactory(i);
  Factory* f= new Factory(c);
  _service_factory= new Conduit(API_SERVS_FACTORY,f);

  a= new userside_serv_accessor();
  m= new Mux(a);
  _sid_inside_mux= new Conduit(API_SERVS_INNER_MUX,m);

  Join(B_half(_sid_outside_mux),A_half(_service_factory));
  Join(B_half(_sid_inside_mux),B_half(_service_factory));

  DefinitionComplete();
}

API_Services_Block::~API_Services_Block() { }

Conduit *API_Services_Block::GetAHalf(void) const { 
  return A_half(_sid_outside_mux);
}

Conduit *API_Services_Block::GetBHalf(void )const { 
  return A_half(_sid_inside_mux);
}

//------------------------------------------------

API_ServiceFactory::API_ServiceFactory(ATM_Interface* i) 
: _master_interface(i) 
{ 
  if (_sean_visitor_type == 0)
    _sean_visitor_type = QueryRegistry(SEAN_VISITOR_NAME);
  _next_allocated_sid=2;
}

Conduit * API_ServiceFactory::Create(Visitor * v) 
{
  VisitorType vt = v->GetType();
  State * s;  Protocol * p;  Conduit * c = 0;
  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;
    sean_Visitor::sean_Visitor_Type t = sv->get_op();
    switch (t) {
      case sean_Visitor::api_service_construction:
	s = new FSM_ATM_Service(_next_allocated_sid, _master_interface);
	sv->set_sid(_next_allocated_sid);
	_next_allocated_sid+=2;
	p = new Protocol(s);
	Register(c = new Conduit("Service",p));
	break;
      default:
	break;
    }
  } else 
    v->Suicide();

  return c;
}

void API_ServiceFactory::Interrupt(SimEvent* e) { }

API_ServiceFactory::~API_ServiceFactory() { }

//------------------------------------------------

API_Calls_Block::API_Calls_Block(ATM_Interface* i) { 
  Accessor* a= new call_accessor();
  Mux* m= new Mux(a);
  _cid_outside_mux=new Conduit(API_CALLS_OUTER_MUX,m);

  Creator* c=new API_CallFactory(i);
  Factory* f=new Factory(c);
  _call_factory=new Conduit(API_CALLS_FACTORY,f);

  a= new call_accessor();
  m= new Mux(a);
  _cid_inside_mux=new Conduit(API_CALLS_INNER_MUX,m);

  Join(B_half(_cid_outside_mux),A_half(_call_factory));
  Join(B_half(_cid_inside_mux),B_half(_call_factory));

  DefinitionComplete();
}

API_Calls_Block::~API_Calls_Block() { }

Conduit *API_Calls_Block::GetAHalf(void) const { 
  return A_half(_cid_outside_mux);
}

Conduit *API_Calls_Block::GetBHalf(void )const { 
  return A_half(_cid_inside_mux);
}

//------------------------------------------------

API_CallFactory::API_CallFactory(ATM_Interface* i) 
: _master_interface(i) { 
  if (_sean_visitor_type == 0)
    _sean_visitor_type = QueryRegistry(SEAN_VISITOR_NAME);
  _next_allocated_cid=2;
}

Conduit * API_CallFactory::Create(Visitor * v) 
{ 
  VisitorType vt = v->GetType();
  Expander* e; Cluster* cl; Conduit *co = 0;
  int cid;
  if (vt.Is_A(_sean_visitor_type)) {
    sean_Visitor * sv = (sean_Visitor*)v;
    sean_Visitor::sean_Visitor_Type t = sv->get_op();
    switch (t) {
      case sean_Visitor::api_call_construction:
	cid = sv->get_cid();
	if ( cid == NO_CID ) {
	  e = new API_Single_Call(_master_interface, _next_allocated_cid);
	  sv->set_cid(_next_allocated_cid);
	  _next_allocated_cid+=2;
	} else {
	  e = new API_Single_Call(_master_interface, cid);
	  sv->set_cid(cid);
	}
	cl = new Cluster(e);
	if (cid==0)
	  co = new Conduit("UNI-Outgoing-Call",cl);
	else
	  co = new Conduit("UNI-Incoming-Call",cl);
	break;
      default:
	break;
    }
  } else 
    v->Suicide();

  if (co)
    Register(co);
  return co;
}

void API_CallFactory::Interrupt(SimEvent* e) { }
API_CallFactory::~API_CallFactory() { }
  
//------------------------------------------------

State * API_Single_Call::Handle(Visitor * v) 
{ 
  // How are you planning on returning a state when you're an Expander ...
  // Why did you define Handle for that matter?
  return 0; 
}
void API_Single_Call::Interrupt(class SimEvent *e) { }

API_Single_Call::API_Single_Call(ATM_Interface* i, int cid) { 
  Accessor* a = new leaf_accessor();
  Mux* m = new Mux(a);
  _lid_outside_mux= new Conduit(API_PARTY_OUTER_MUX,m);

  a = new leaf_accessor();
  m = new Mux(a);
  _lid_inside_mux= new Conduit(API_PARTY_INNER_MUX,m);

  Creator* c= new API_PartyFactory(i);
  Factory* f = new Factory(c);
  _leaf_factory= new Conduit(API_PARTY_FACTORY,f);

  State* s= new FSM_ATM_Call(cid,i);
  Protocol* p= new Protocol(s);
  _call= new Conduit(API_CALL_STATE,p);

  Join(B_half(_lid_outside_mux),A_half(_leaf_factory));
  Join(B_half(_lid_inside_mux),B_half(_leaf_factory));
  Join(A_half(_lid_inside_mux),A_half(_call));

  DefinitionComplete();
}

API_Single_Call::~API_Single_Call() { }

Conduit *API_Single_Call::GetAHalf(void) const {
  return A_half(_lid_outside_mux);
}

Conduit *API_Single_Call::GetBHalf(void )const{
  return B_half(_call);
}

//------------------------------------------------

API_PartyFactory::API_PartyFactory(ATM_Interface* i) 
: _master_interface(i) { 
  if (_sean_visitor_type == 0)
    _sean_visitor_type = QueryRegistry(SEAN_VISITOR_NAME);
}

Conduit * API_PartyFactory::Create(Visitor * v) 
{ 
  VisitorType vt = v->GetType();
  State* s;  Protocol* p;  Conduit* c = 0;
  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;
    sean_Visitor::sean_Visitor_Type t = sv->get_op();
    int pid = sv->get_lid();
    switch(t) {
    case sean_Visitor::api_leaf_construction:
	s = new FSM_ATM_Leaf( pid );
	_master_interface->_new_leaf_fsm = (FSM_ATM_Leaf*)s;
	_master_interface->_new_leaf_id = pid;
	p = new Protocol(s);
	Register(c = new Conduit("Leaf",p));
	break;

    case sean_Visitor::leaf_setup_ind:
      break;
	
    default:
      break;
    }
  } else 
    v->Suicide();
  return c;
}

void API_PartyFactory::Interrupt(SimEvent* e) { }

API_PartyFactory::~API_PartyFactory() { }
  
//------------------------------------------------
//------------------------------------------------
API_Translator::API_Translator(int port, ATM_Interface* owner, const char* machine) 
  : State( false ), // not queueing
    _port(port), _alive (false), _owner(owner)
{
  if (_sean_visitor_type == 0)
    _sean_visitor_type = QueryRegistry(SEAN_VISITOR_NAME);
  if (_ipc_data_type == 0)
    _ipc_data_type = QueryRegistry(IPC_DATA_VISITOR_NAME);
  if (_ipc_signalling_type == 0)
    _ipc_signalling_type = QueryRegistry(IPC_SIGNALLING_VISITOR_NAME);

  if (machine) strcpy(_machine,machine);
  else strcpy(_machine,"localhost");

  SimEvent* boot = new SimEvent(this,this,1);
  ReturnToSender(boot);
}

State* API_Translator::Handle(Visitor* v) {
  VisitorType vt = v->GetType();
  
  bool override = false;
  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;
    sean_Visitor::sean_Visitor_Type t = sv->get_op();
    switch(t) {
    case sean_Visitor::api_call_construction:
    case sean_Visitor::api_call_destruction:
    case sean_Visitor::api_service_construction:
    case sean_Visitor::api_service_destruction:
    case sean_Visitor::api_leaf_construction:
    case sean_Visitor::api_leaf_destruction:
      override=true;
    default:
      break;
    }

    if (override) {
      diag("api.translator",DIAG_INFO, "API_Translator kills API-local sean_Visitor.\n");
      v->Suicide();
    }
    else if (v && (VisitorFrom(v)==Visitor::B_SIDE)) {
      Buffer* buf = sv->take_buffer();

      sean_Visitor::sean_Visitor_Type op=sv->get_op();
      int cid=sv->get_cid();
      int sid=sv->get_sid();
      int lid=sv->get_lid();

      buf->GrowHeader(IPC_HEADER_OFFSET);
      unsigned char* buffer = buf->data();
      int index=0;
      bcopy((void *)(&op),
	    (void*)(buffer+index*sizeof(int)),
	    sizeof(int)); index++;
      bcopy((void *)(&cid),
	    (void*)(buffer+index*sizeof(int)),
	    sizeof(int)); index++;
      bcopy((void *)(&sid),
	    (void*)(buffer+index*sizeof(int)),
	    sizeof(int)); index++;
      bcopy((void *)(&lid),
	    (void*)(buffer+index*sizeof(int)),
	    sizeof(int)); index++;
      
      IPC_DataVisitor* dv = new IPC_DataVisitor(_local_id,buf);
      diag("api.translator",DIAG_INFO, "API_Translator kills sean_Visitor after successful transformation into IPC_datavisitor.\n");

      sv->Suicide();
      if (_alive) PassVisitorToA(dv);
      else _waiting_to_be_sent.append(dv);
    }
    else {
      diag("api.translator",DIAG_INFO, "API_Translator kills sean_Visitor from A side.\n");
      v->Suicide();
    }
  }

  if (vt.Is_A(_ipc_data_type)){
    diag("api.translator",DIAG_INFO, "API_Translator got some IPC data\n");
    IPC_DataVisitor* dv = (IPC_DataVisitor*)v;
    int len = dv->get_length();
    Buffer* buf = dv->take_packet_and_zero_length();
    
    sean_Visitor::sean_Visitor_Type op;
    int cid, sid, lid;
    bool ok=true;
    if (buf) {
      unsigned char* buffer = buf->data();
      int index=0;

      if ( len >= (4*sizeof(int)) ) {
	bcopy((void*)(buffer+index*sizeof(int)),
	      (void*)(&op),
	      sizeof(int)); 
	index++; 
	bcopy((void*)(buffer+index*sizeof(int)),
	      (void*)(&cid),
	      sizeof(int));
	index++;
	bcopy((void*)(buffer+index*sizeof(int)),
	      (void*)(&sid),
	      sizeof(int));
	index++;
	bcopy((void*)(buffer+index*sizeof(int)),
	      (void*)(&lid),
	      sizeof(int)); 
	index++;
	len -= 4*sizeof(int);
	buf->ShrinkHeader( 4*sizeof(int) );
      }
      else ok=false;
    }
    else ok=false;
    
    if (ok) {
      if (op!=sean_Visitor::raw_simulation_pdu) {
	generic_q93b_msg* m = 0;
	if (len>0)
	  m = Parse(buf->data(), len);
	if ((m) && (!m->Valid())) {
	  delete m;
	}
	else {
	  sean_Visitor* sv = new sean_Visitor(op,m);
	  sv->set_cid(cid);
	  sv->set_sid(sid);
	  sv->set_lid(lid);
	  if (m) {
	    sv->set_crv( m->get_crv() );
	    m->set_external_buffer_on();
	  }
	  PassVisitorToB(sv);
	}
      }
      else {
	sean_Visitor* sv = new sean_Visitor(sean_Visitor::raw_simulation_pdu, 0, buf);  // ship raw sim PDUs
	sv->set_cid(cid);
	sv->set_sid(-1);
	sv->set_lid(-1);
	PassVisitorToB(sv);
      }
    }
    else {
      diag("api.translator",DIAG_ERROR,"API Translator receives corrupt IPC data\n");
    }
    v->Suicide();
  }

  if (vt.Is_A(_ipc_signalling_type)){
    diag("api.translator",DIAG_INFO,"API_Translator got some IPC signalling\n");
    IPC_SignallingVisitor* sigv = (IPC_SignallingVisitor*)v;
    if (sigv->get_IPC_opcode() == IPC_SignallingVisitor::release_by_peer_op) {
      diag("api.translator",DIAG_FATAL,"API detects death of signalling agent\n");
      exit(0);
    }
  }

  return this;
}

void API_Translator::Interrupt(class SimEvent *e) {
  if (e->GetCode() == 1) {
    tcp_endpoint* dest = new tcp_endpoint(_port, _machine);
    SimEvent* ack = new SimEvent(this,this,2);
      
    IPC_SignallingVisitor* v = new IPC_SignallingVisitor
      (IPC_SignallingVisitor::establish_by_local, _local_id, dest, ack);
    
    PassVisitorToA(v);
    // dwt - leak scope: dest ?
  }
  else if (e->GetCode() == 2) {
    tcp_local_id* tlid = (tcp_local_id*)_local_id;

    DIAG("api",DIAG_INFO,{ \
       printf("Connection to signalling agent established on"); \
       tlid->Print(cout); \
       cout << "." << endl; });
    _alive = true;
    _owner->notify_of_boot(_local_id);

    while (! _waiting_to_be_sent.empty()) {
      IPC_DataVisitor* dv = _waiting_to_be_sent.pop();
      PassVisitorToA(dv);
    }
  }
  delete e;
}

API_Translator::~API_Translator() {
}

//------------------------------------------------


API_Op_Accessor::API_Op_Accessor(void) { 
  if (_sean_visitor_type == 0)
    _sean_visitor_type = QueryRegistry(SEAN_VISITOR_NAME);
  _calls_side = _services_side = 0;
}

Conduit * API_Op_Accessor::GetNextConduit(Visitor * v) {
  VisitorType vt = v->GetType();
  Conduit* next=0;
  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;
    if (sv->is_call_related()) return _calls_side;
    else if (sv->is_service_related()) return _services_side;
    else return 0;
  }
  else return 0;
}

API_Op_Accessor::~API_Op_Accessor() { }

bool API_Op_Accessor::Broadcast(Visitor * v) { return false; }

bool API_Op_Accessor::Add(Conduit * c, Visitor * v) 
{ 
  VisitorType vt = v->GetType();
  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;
    if (sv->is_call_related()) { _calls_side=c; return true; }
    else if (sv->is_service_related()) { _services_side=c; return true; }
    else return false;
  }
  else return false;
}

bool API_Op_Accessor::Del(Conduit * c) { return false; }
bool API_Op_Accessor::Del(Visitor * v) { return false; }

//------------------------------------------------
API_Injector_Terminal::API_Injector_Terminal(void) 
{
  if (_sean_visitor_type == 0)
    _sean_visitor_type = QueryRegistry(SEAN_VISITOR_NAME);
}

void API_Injector_Terminal::Absorb(Visitor* v) 
{
  VisitorType vt = v->GetType();
  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;
    sean_Visitor::sean_Visitor_Type t = sv->get_op();
    switch(t) {
    default:
      break;
    }
  }

  v->Suicide();
}

void API_Injector_Terminal::Interrupt(class SimEvent *e) {
  delete e;
}

API_Injector_Terminal::~API_Injector_Terminal(void) {
}


//-------------------------------------------------------
//-------------------------------------------------------

//-----------------
call_accessor::call_accessor(void){
  if (_sean_visitor_type == 0)
    _sean_visitor_type = QueryRegistry(SEAN_VISITOR_NAME);
}

//-----------------
Conduit * call_accessor::GetNextConduit(Visitor * v) {
  VisitorType vt = v->GetType();
  
  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;
    appid_and_int* id=sv->get_cid_key();
    dic_item d;
    Conduit * ret = 0;
    if (d = _access_map.lookup(id))
      ret = _access_map.inf(d);
    delete id;
    return ret;
  }
  return 0;
}

//-----------------
call_accessor::~call_accessor() {
}

//-----------------
bool call_accessor::Broadcast(Visitor * v) {
  // not supported
  return false;
}

//-----------------
bool call_accessor::Add(Conduit * c, Visitor * v){
  appid_and_int* id;
  
  VisitorType vt = v->GetType();
  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;
    sean_Visitor::sean_Visitor_Type t = sv->get_op();
    switch(t) {
    default: {
      id=sv->get_cid_key();
      bool ret;
      if (_access_map.lookup(id)) {
	ret=false;
	delete id;
      }
      else {
	_access_map.insert(id,c);
	ret=true;
      }
      return ret;
      } break;
    }
  }
  return false;
}

//-----------------
bool call_accessor::Del(Conduit * c){
  dic_item it;
  forall_items(it,_access_map) {
    if (_access_map.inf(it)==c) {
      _access_map.del_item(it);
      return true;
    }
  }
  return false;
}

//-----------------
bool call_accessor::Del(Visitor * v){
  // not supported;
  return false;
}


//-------------------------------------------------------
//-------------------------------------------------------

leaf_accessor::leaf_accessor(void){
  if (_sean_visitor_type == 0)
    _sean_visitor_type = QueryRegistry(SEAN_VISITOR_NAME);
}

//-----------------
Conduit * leaf_accessor::GetNextConduit(Visitor * v) { 
  VisitorType vt = v->GetType();
  Conduit * ret = 0;
  
  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;
    appid_and_int* pkey = sv->get_lid_key();
    dic_item d;
    if (d = _access_map.lookup(pkey))
      ret =_access_map.inf(d);
    delete pkey;
  }
  return ret;
}

//-----------------
leaf_accessor::~leaf_accessor() {
}

//-----------------
bool leaf_accessor::Broadcast(Visitor * v) {
  // not supported
  return false;
}

//-----------------
bool leaf_accessor::Add(Conduit * c, Visitor * v){
  appid_and_int* pkey;
  VisitorType vt = v->GetType();
  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;
    sean_Visitor::sean_Visitor_Type t = sv->get_op();
    switch(t) {
    default: {
      pkey = sv->get_lid_key();
      if (_access_map.lookup(pkey))
	return false;
  
      _access_map.insert(pkey,c);
      return true;
      } break;
    }
  }
  return false;
}

//-----------------
bool leaf_accessor::Del(Conduit * c){
  dic_item it;
  forall_items(it,_access_map) {
    if (_access_map.inf(it)==c) {
      _access_map.del_item(it);
      return true;
    }
  }
  return false;
}

//-----------------
bool leaf_accessor::Del(Visitor * v){
  // not supported
  return false;
}

