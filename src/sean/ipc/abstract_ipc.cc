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
static char const _abstract_ipc_cc_rcsid_[] =
"$Id: abstract_ipc.cc,v 1.3 1998/08/06 04:04:11 bilal Exp $";
#endif
#include <common/cprototypes.h>
// -*- C++ -*-

#include "abstract_ipc.h"
#include <FW/basics/diag.h>
#include <codec/q93b_msg/Buffer.h>

//----------------------------------------------------------
abstract_ipc_mechanism::abstract_ipc_mechanism(type t) : _t(t) {}
abstract_ipc_mechanism::~abstract_ipc_mechanism() {}

abstract_ipc_mechanism::type abstract_ipc_mechanism::
get_ipc_scheme_type(void) const { return _t; }


IPC_SignallingVisitor* abstract_ipc_mechanism::
make_IPC_SV(const IPC_SignallingVisitor::ipc_rel_peer_t op, abstract_local_id* id) {
  return new IPC_SignallingVisitor(op,id);
}

IPC_SignallingVisitor* abstract_ipc_mechanism::
make_IPC_SV(const IPC_SignallingVisitor::ipc_est_peer_t op, 
	    abstract_local_id* id,
	    abstract_connection* con) {
  return new IPC_SignallingVisitor(op,id,con);
}

IPC_SignallingVisitor::ipc_op abstract_ipc_mechanism::
IPC_SV_get_IPC_opcode(IPC_SignallingVisitor* sv) const {
  return sv->get_IPC_opcode();
}

abstract_connection* abstract_ipc_mechanism::
IPC_SV_take_connection(IPC_SignallingVisitor* sv) {
  return sv->take_connection();
}

abstract_local_id** abstract_ipc_mechanism::
IPC_SV_take_id_pp(IPC_SignallingVisitor* sv) {
  return sv->take_id_pp();
}

abstract_endpoint* abstract_ipc_mechanism::
IPC_SV_take_destination_ep(IPC_SignallingVisitor* sv) {
  return sv->take_destination_ep();
}

SimEvent* abstract_ipc_mechanism::
IPC_SV_take_simevent(IPC_SignallingVisitor* sv) {
  return sv->take_simevent();
}

//----------------------------------------------------------

abstract_listener::abstract_listener(abstract_endpoint* ep) :
  abstract_ipc_mechanism(ep->get_ipc_scheme_type()) {}
abstract_listener::~abstract_listener() {}

//----------------------------------------------------------

abstract_endpoint::abstract_endpoint(abstract_ipc_mechanism::type t) :
  abstract_ipc_mechanism(t) {}
abstract_endpoint::~abstract_endpoint() {}

//----------------------------------------------------------

abstract_local_id::abstract_local_id(abstract_ipc_mechanism::type t) :
  abstract_ipc_mechanism(t) {}  
abstract_local_id::~abstract_local_id() {}

//----------------------------------------------------------

abstract_connection::abstract_connection(abstract_ipc_mechanism::type t,
					 abstract_local_id* id) :
  abstract_ipc_mechanism(t), _id(id) {}
abstract_connection::~abstract_connection() {}

void abstract_connection::set_state(const state s) { _s = s; }
abstract_connection::state abstract_connection::get_state(void) const 
{ return _s; }

const abstract_local_id * abstract_connection::get_local_id(void) const {
  return _id;
}

IPC_DataVisitor* abstract_connection::IPC_DV_make_incoming(abstract_local_id* id,
							   Buffer* buf) {
  IPC_DataVisitor::incoming_flag xxx =
    IPC_DataVisitor::internal_use__incoming_flag;
  
  IPC_DataVisitor* dv = new IPC_DataVisitor(id,buf,xxx);
  return dv;
}

void abstract_connection::Suicide(void) {
  delete this;
}

void print_result(abstract_connection::result res) {
  switch (res) {
  case abstract_connection:: failure:
    diag("ipc.connection",DIAG_DEBUG,"failed.\n");
    break;
  case abstract_connection::success:
    diag("ipc.connection",DIAG_DEBUG,"succeeded.\n");
    break;
  case abstract_connection::overflow:
    diag("ipc.connection",DIAG_DEBUG,"overflow.\n");
    break;
  case abstract_connection::underflow:
    diag("ipc.connection",DIAG_DEBUG,"underflow.\n");
    break;
  case abstract_connection::death:
    diag("ipc.connection",DIAG_DEBUG,"death!\n");
    break;
  case abstract_connection::unknown_result:
    diag("ipc.connection",DIAG_WARNING,"unknown for send/recv operation\n");
  default:
    break;
  }
}

//----------------------------------------------------------

abstract_ipc_layer::abstract_ipc_layer(abstract_endpoint* lis_ep) : 
  abstract_ipc_mechanism(lis_ep->get_ipc_scheme_type()) {}
abstract_ipc_layer::~abstract_ipc_layer(void) {}


bool abstract_ipc_layer::bind_connection(const abstract_local_id* id, abstract_connection* con){
  if (_connection_table.lookup(id)) return false;
  _connection_table.insert(id,con);
  return true;
}


bool abstract_ipc_layer::unbind_connection(const abstract_local_id* id){
  dic_item di;
  if (!(di=_connection_table.lookup(id))) return false;
  _connection_table.del_item(di);
  return true;
}


abstract_connection* abstract_ipc_layer::lookup_connection(const abstract_local_id* id){
  dic_item di=_connection_table.lookup(id);
  if (di) return _connection_table.inf(di);
  else return 0;
}


int compare(abstract_local_id const *const & id1, abstract_local_id const *const & id2) {
  return id1->compare(id2);
}

void abstract_ipc_layer::process_all_connections(void) {
  dic_item di;
  forall_items(di,_connection_table) {
    abstract_connection* con = _connection_table.inf(di);
    con->process();
  }
}


