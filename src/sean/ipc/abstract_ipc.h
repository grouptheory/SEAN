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
#ifndef __ABSTRACT_IPC_H__
#define __ABSTRACT_IPC_H__

#ifndef LINT
static char const _abstract_ipc_h_rcsid_[] =
"$Id: abstract_ipc.h,v 1.2 1998/07/31 02:59:36 bilal Exp $";
#endif

#include <iostream.h>
#include <DS/containers/dictionary.h>

#include "IPC_Visitors.h"
class Buffer;

//----------------------------------------------------------
// This class holds the identity of the specific
// IPC mechanism being used in the implementation.
//----------------------------------------------------------
class abstract_ipc_mechanism {
public:
  enum type { 
    unknown = 0,
    tcp_sockets = 1
  };

  abstract_ipc_mechanism(type t);

  type get_ipc_scheme_type(void) const;

protected:
  virtual ~abstract_ipc_mechanism();

  IPC_SignallingVisitor*  make_IPC_SV(const IPC_SignallingVisitor::ipc_rel_peer_t op, 
				      abstract_local_id* id);
  IPC_SignallingVisitor*  make_IPC_SV(const IPC_SignallingVisitor::ipc_est_peer_t op, 
				      abstract_local_id* id,
				      abstract_connection* con);
  IPC_SignallingVisitor::ipc_op IPC_SV_get_IPC_opcode(IPC_SignallingVisitor* sv) const;
  abstract_connection*          IPC_SV_take_connection(IPC_SignallingVisitor* sv);
  abstract_local_id**           IPC_SV_take_id_pp(IPC_SignallingVisitor* sv);
  abstract_endpoint*            IPC_SV_take_destination_ep(IPC_SignallingVisitor* sv);
  SimEvent*                     IPC_SV_take_simevent(IPC_SignallingVisitor* sv);

private:
  type _t;
};

#define TYPECHECK(var,typ) (var->get_ipc_scheme_type()==abstract_ipc_mechanism::##typ)

//----------------------------------------------------------
// The abstract endpoint specifies the identity 
// of a listener and also the identity of the
// target of a connection request.
// this object has a globally consistent interpretation
//----------------------------------------------------------
class abstract_endpoint : public abstract_ipc_mechanism {
public:
  abstract_endpoint(abstract_ipc_mechanism::type t);

  virtual int compare(const abstract_endpoint * other) const=0;

  virtual void Print(ostream& os) const = 0;

protected:
  virtual ~abstract_endpoint();
};



//----------------------------------------------------------
// The abstract local identifier is used to name
// and subsequently refer to individual connections.
// this object has only a local interpretation.
//----------------------------------------------------------
class abstract_local_id : public abstract_ipc_mechanism {
public:
  abstract_local_id(abstract_ipc_mechanism::type t);

  virtual int compare(const abstract_local_id * other) const=0;

  virtual void Print(ostream& os) const = 0;

protected:
  virtual ~abstract_local_id();
};



//----------------------------------------------------------
// The abstract connection is an established duplex ipc channel
//----------------------------------------------------------
class abstract_connection : public abstract_ipc_mechanism {
protected:
  abstract_connection(abstract_ipc_mechanism::type t, abstract_local_id* id);
  virtual ~abstract_connection();

  IPC_DataVisitor* IPC_DV_make_incoming(abstract_local_id* id,
					Buffer* buf);

public:
  enum result {
    unknown_result,
    failure,
    success,
    overflow,
    underflow,
    death
  };

  enum state {
    unknown_state,
    alive,
    dead
  };

  enum { header_length = 4 };

  state                     get_state(void) const;
  const abstract_local_id * get_local_id(void) const;
  virtual abstract_local_id* my_local_id(void) = 0;

  friend void print_result(abstract_connection::result res);
  virtual void Print(ostream& os) const = 0;

  virtual result send_to(Buffer* buf)=0;
  virtual result recv_from(Buffer* buf, int& len)=0;
  virtual result next_pdu_length(int& len)=0;

  void Suicide(void);

  virtual void process(void) = 0;

protected:
  void set_state(const state s);
  state _s;

  unsigned char _header[ abstract_connection::header_length ];

  abstract_local_id* _id;
};


//----------------------------------------------------------
// The abstract listener class waits to be the 
// target of an ipc connection request
//----------------------------------------------------------
class abstract_listener : public abstract_ipc_mechanism {
public:
  abstract_listener(abstract_endpoint* ep);

  virtual bool start_listening(int max_queue)=0;

  virtual abstract_connection* get_incoming_call(void)=0;

  virtual void Print(ostream& os) const = 0;

protected:
  virtual ~abstract_listener();
};


//----------------------------------------------------------
// The abstract ipc layer manages the creation 
// and destruction of ipc connections.  If lis_ep is
// omitted from the ctors argument the layer will
// not be able to receive incoming connections.
//----------------------------------------------------------
class abstract_ipc_layer : public abstract_ipc_mechanism {
public:
  abstract_ipc_layer(abstract_endpoint* lis_ep=0);

  virtual abstract_connection* initiate_connection(abstract_endpoint* target_ep)=0;

  virtual void Print(ostream& os) const = 0;

  void process_all_connections(void);

protected:
  virtual ~abstract_ipc_layer(void);

  bool bind_connection(const abstract_local_id* id, abstract_connection* con);
  bool unbind_connection(const abstract_local_id* id);
  abstract_connection* lookup_connection(const abstract_local_id* id);

private:
  dictionary<const abstract_local_id*, abstract_connection*> _connection_table;
};

int compare(abstract_local_id const *const &, abstract_local_id const *const &);

#endif





