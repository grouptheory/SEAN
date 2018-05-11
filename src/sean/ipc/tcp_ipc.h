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
#ifndef __TCP_IPC_H__
#define __TCP_IPC_H__

#ifndef LINT
static char const _tcp_ipc_h_rcsid_[] =
"$Id: tcp_ipc.h,v 1.2 1998/07/31 02:59:36 bilal Exp $";
#endif

#include "abstract_ipc.h"

#include <FW/kernel/Handlers.h>
#include <FW/actors/Creator.h>
#include <FW/actors/State.h>

class Visitor;
class Conduit;

//----------------------------------------------------------
class tcp_listener : public abstract_listener, public InputHandler {
public:
  tcp_listener(Terminal * manager, int lis_portnumber);

  bool start_listening(int max_queue);
  abstract_connection* get_incoming_call(void);

  void Callback(void);

  virtual void Print(ostream& os) const;

protected:
  virtual ~tcp_listener();

private:
  static struct protoent * _ppe;   // protocol info
  Terminal* _manager;
  int _portnumber;
};

//----------------------------------------------------------
class tcp_endpoint : public abstract_endpoint {
public:
  tcp_endpoint(int portnumber, char* host=0);

  int compare(const abstract_endpoint* other) const;
  int portnumber(void) const;
  const char* host(void) const;

  virtual void Print(ostream& os) const;

protected:
  virtual ~tcp_endpoint();

private:
  int _id;          // port number
  char _host[255];  // hostname
};

//----------------------------------------------------------
class tcp_local_id : public abstract_local_id {
  friend class asynchronous_manager;
public:
  tcp_local_id(int socketnumber);

  int compare(const abstract_local_id* other) const;

  virtual void Print(ostream& os) const;

protected:
  virtual ~tcp_local_id();

private:

  int socketnumber(void) const;
  int _s;          // socket number
};

//----------------------------------------------------------
class tcp_connection : public abstract_connection, public InputHandler {
public:

  tcp_connection(Terminal * manager, int sock);
  
  abstract_connection::result send_to(Buffer* buf);
  abstract_connection::result recv_from(Buffer* buf, int& len);
  abstract_connection::result next_pdu_length(int& len);

  void Callback(void);

  virtual void Print(ostream& os) const;

  abstract_local_id* my_local_id(void);

  void process(void);

protected:

  virtual ~tcp_connection(void);

private:

  int socket(void);

  Terminal* _manager;
  int _sock;

  void die(void);

  bool _ignore, _first;
};


//----------------------------------------------------------
class tcp_ipc_layer : public abstract_ipc_layer, public Terminal {
public:

  tcp_ipc_layer(int lis_portnumber=0);

  void Absorb(Visitor* v);
  void Interrupt(class SimEvent *e);

  virtual void Print(ostream& os) const;

protected:

  virtual ~tcp_ipc_layer(void);

private:

  abstract_connection* initiate_connection(abstract_endpoint* target_ep);

  tcp_listener * _lis;

  static const VisitorType * _ipc_data_vistype;
  static const VisitorType * _ipc_sig_vistype;
};

#endif
