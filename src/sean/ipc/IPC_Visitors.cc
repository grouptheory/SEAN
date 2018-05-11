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
static char const _IPC_Visitors_cc_rcsid_[] =
"$Id: IPC_Visitors.cc,v 1.4 1998/08/21 16:12:41 mountcas Exp $";
#endif

#include <common/cprototypes.h>

#include "IPC_Visitors.h"
#include "tcp_ipc.h"
#include <FW/basics/diag.h>
#include <FW/kernel/SimEvent.h>
#include <codec/q93b_msg/Buffer.h>

IPC_Visitor::IPC_Visitor(const abstract_local_id * id) 
  : Visitor(_my_type), _id(id) { }

IPC_Visitor::IPC_Visitor(const IPC_Visitor & rhs)
  : Visitor(rhs), _id(rhs._id) { }
  
IPC_Visitor::~IPC_Visitor() { }
  
const abstract_local_id* IPC_Visitor::share_local_id(void) {
  return _id;
}

const VisitorType IPC_Visitor::GetType(void) const
{   return VisitorType(GetClassType());  }

const vistype & IPC_Visitor::GetClassType(void) const {
  return _my_type;
}

void IPC_Visitor::Print(ostream& os) const {
  if (_id) _id->Print(os);
  else os << "(null)";
}

//-----------------------------------------------------------------------
IPC_SignallingVisitor::IPC_SignallingVisitor(const ipc_rel_peer_t op, 
					     const abstract_local_id* id) 
  : IPC_Visitor (id),
    _op(release_by_peer_op),_con(0),_dest(0),_ack(0),_id_pp(0) {
}

IPC_SignallingVisitor::IPC_SignallingVisitor(const ipc_rel_local_t op, 
					     const abstract_local_id* id) 
  : IPC_Visitor (id),
    _op(release_by_local_op),_con(0),_dest(0),_ack(0),_id_pp(0) {
}

IPC_SignallingVisitor::IPC_SignallingVisitor(const ipc_est_peer_t op, 
					     const abstract_local_id* id,
					     abstract_connection* con) 
  : IPC_Visitor (id),
    _op(establish_by_peer_op),_con(con),_dest(0),_ack(0),_id_pp(0) {
}

IPC_SignallingVisitor::IPC_SignallingVisitor(const ipc_est_local_t op, 
					     abstract_local_id*& id,
					     abstract_endpoint* dest,
					     SimEvent* ack) 
  : IPC_Visitor (0),
    _op(establish_by_local_op),_con(0),
    _dest(dest),_ack(ack),_id_pp(&id) {
}

IPC_SignallingVisitor::IPC_SignallingVisitor(const IPC_SignallingVisitor & rhs)
  : IPC_Visitor(rhs), _op(rhs._op), _con(rhs._con), _dest(rhs._dest), 
    _id_pp(rhs._id_pp), _ack(rhs._ack) { }

IPC_SignallingVisitor::~IPC_SignallingVisitor() {
}

IPC_SignallingVisitor::ipc_op 
IPC_SignallingVisitor::get_IPC_opcode(void) const {
  return _op;
}

abstract_connection* IPC_SignallingVisitor::take_connection(void) {
  abstract_connection* ret = _con;
  _con = 0;
  return ret;
}


abstract_endpoint* IPC_SignallingVisitor::take_destination_ep(void) {
  abstract_endpoint* ret = _dest;
  _dest = 0;
  return ret;
}

abstract_local_id** IPC_SignallingVisitor::take_id_pp(void) {
  abstract_local_id** ret = _id_pp;
  _id_pp=0; 
  return ret;
}

SimEvent* IPC_SignallingVisitor::take_simevent(void) {
  SimEvent * ret = _ack;
  _ack = 0;
  return ret;
}

void IPC_SignallingVisitor::Print(ostream& os) const {
  switch (_op) {
  case establish_by_peer_op:
    os << "op=establish_by_peer, ";
    IPC_Visitor::Print(os);
    os << endl;
    break;
  case establish_by_local_op:
    os << "op=establish_by_local, ";
    if (_dest) {
      _dest->Print(os);
      os << ", ";
    }
    else os << "dest=(null), ";
    IPC_Visitor::Print(os);
    os << endl;
    break;
  case release_by_peer_op:
    os << "op=release_by_peer, ";
    IPC_Visitor::Print(os);
    os << endl;
    break;
  case release_by_local_op:
    os << "op=release_by_local, ";
    IPC_Visitor::Print(os);
    os << endl;
    break;
  case unknown:
  default:
    os << "op=unknown";
    break;
  }
}


const vistype & IPC_SignallingVisitor::GetClassType(void) const {
  return _my_type;
}

const VisitorType IPC_SignallingVisitor::GetType(void) const 
{   return VisitorType(GetClassType());  }

Visitor * IPC_SignallingVisitor::dup(void) const
{
  return new IPC_SignallingVisitor(*this);
}

//-----------------------------------------------------------------------


IPC_DataVisitor::IPC_DataVisitor(const abstract_local_id * id,
				 Buffer* buffer)
  : IPC_Visitor(id) {

    _packet = buffer;
    _length = buffer->length();
    _dir = IPC_DataVisitor::outgoing;
    _id = id;
}

IPC_DataVisitor::IPC_DataVisitor(const abstract_local_id * id,
				 Buffer* buffer,
				 const IPC_DataVisitor::incoming_flag i)
  : IPC_Visitor(id) {

    _packet = buffer;
    _length = buffer->length();
    _dir = IPC_DataVisitor::incoming;
    _id = id;
}

IPC_DataVisitor::IPC_DataVisitor(const IPC_DataVisitor & rhs)
  : IPC_Visitor(rhs), _length(rhs._length), _dir(rhs._dir), 
    _packet(rhs._packet) { } 

IPC_DataVisitor::~IPC_DataVisitor() {
}

Buffer* IPC_DataVisitor::take_packet_and_zero_length(void) {
  Buffer* answer = _packet;
  _packet = 0;
  _length = 0;
  return answer;
}

int IPC_DataVisitor::get_length(void) const {
  return _length;
}

IPC_DataVisitor::dir IPC_DataVisitor::direction(void) const {
  return _dir;
}

void IPC_DataVisitor::Print(ostream& os) const {
  switch (_dir) {
  case outgoing:
    os << "type=outgoing, length="<<_length<<endl;
    break;
  case incoming:
    os << "type=incoming, length="<<_length<<endl;
    break;
  case unknown:
  default:
    os << "type=unknown"<<endl;
    break;
  }
}

const vistype & IPC_DataVisitor::GetClassType(void) const {
  return _my_type;
}

const VisitorType IPC_DataVisitor::GetType(void) const 
{   return VisitorType(GetClassType());  }

Visitor * IPC_DataVisitor::dup(void) const
{
  return new IPC_DataVisitor(*this);
}
