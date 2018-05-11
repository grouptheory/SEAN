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
#ifndef _IPC_VISITORS_H_
#define _IPC_VISITORS_H_

#ifndef LINT
static char const _IPC_Visitors_h_rcsid_[] =
"$Id: IPC_Visitors.h,v 1.3 1998/08/21 14:48:14 mountcas Exp $";
#endif

#include <FW/basics/Visitor.h>

class abstract_ipc_mechanism;
class abstract_endpoint;
class abstract_local_id;
class abstract_connection;
class abstract_listener;
class abstract_ipc_layer;
class SimEvent;
class Buffer;

#define IPC_VISITOR_NAME "IPC_Visitor"
//----------------------------------------------------------
class IPC_Visitor : public Visitor {
public:
  IPC_Visitor(const abstract_local_id* id);
  virtual ~IPC_Visitor();
  
  const abstract_local_id* share_local_id(void);

  const VisitorType GetType(void) const;

protected:

  IPC_Visitor(const IPC_Visitor & rhs);
  void Print(ostream& os) const;

  const abstract_local_id*   _id;

private:

  virtual const vistype &GetClassType(void) const;
  static vistype _my_type;
};


#define IPC_SIGNALLING_VISITOR_NAME "IPC_SignallingVisitor"
//----------------------------------------------------------
class IPC_SignallingVisitor : public IPC_Visitor {
public:

  enum ipc_est_peer_t  { establish_by_peer  = 1 };
  enum ipc_est_local_t { establish_by_local = 2 };
  enum ipc_rel_peer_t  { release_by_peer    = 4 };
  enum ipc_rel_local_t { release_by_local   = 8 };

  enum ipc_op {
    unknown               = 0,
    establish_by_peer_op  = 1,
    establish_by_local_op = 2,
    release_by_peer_op    = 4,
    release_by_local_op   = 8
  };
      
  IPC_SignallingVisitor(const ipc_rel_local_t op, const abstract_local_id* id);
  IPC_SignallingVisitor(const ipc_est_local_t op, 
			abstract_local_id*& id,
			abstract_endpoint* dest,
			SimEvent* ack);
  
  ipc_op get_IPC_opcode(void) const;
  void   Print(ostream& os) const;

  const VisitorType     GetType(void) const;

protected:

  virtual ~IPC_SignallingVisitor();
  IPC_SignallingVisitor(const IPC_SignallingVisitor & rhs);
  virtual Visitor * dup(void) const;

private:

  friend class abstract_ipc_mechanism;
  friend class Application;

  IPC_SignallingVisitor(const ipc_rel_peer_t op, const abstract_local_id* id);
  IPC_SignallingVisitor(const ipc_est_peer_t op, 
			const abstract_local_id* id,
			abstract_connection* con);
  abstract_connection*  take_connection(void);
  abstract_local_id**   take_id_pp(void);
  abstract_endpoint*    take_destination_ep(void);
  SimEvent*             take_simevent(void);

  ipc_op                _op;

  abstract_connection*  _con;
  abstract_endpoint*    _dest;
  abstract_local_id**   _id_pp;
  SimEvent*             _ack;

  virtual const vistype&  GetClassType(void) const;
  static vistype          _my_type;
};


#define IPC_DATA_VISITOR_NAME "IPCDataVisitor"
//----------------------------------------------------------
class IPC_DataVisitor;
class IPC_DataVisitor : public IPC_Visitor {
public:
  enum dir { unknown = 0,
	     outgoing = 1,
	     incoming = 2 };

  IPC_DataVisitor(const abstract_local_id* id,
		  Buffer* buffer);

  Buffer*              take_packet_and_zero_length(void);
  int                  get_length(void) const;
  IPC_DataVisitor::dir direction(void) const;

  void Print(ostream& os) const;

  const VisitorType    GetType(void) const;

protected:

  friend abstract_connection;

  enum incoming_flag {internal_use__incoming_flag = 1};

  IPC_DataVisitor(const abstract_local_id* id,
		  Buffer* buffer,
		  const incoming_flag i);

  virtual ~IPC_DataVisitor();
  IPC_DataVisitor(const IPC_DataVisitor & rhs);
  virtual Visitor * dup(void) const;

private:

  virtual const vistype& GetClassType(void) const;

  Buffer*              _packet;
  int                  _length;
  dir                  _dir;

  static vistype         _my_type;
};

#endif
