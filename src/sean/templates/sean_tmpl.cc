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
static char const _sean_tmpl_cc_rcsid_[] =
"$Id: sean_tmpl.cc,v 1.10 1998/10/21 21:29:52 marsh Exp $";
#endif
#include <common/cprototypes.h>

#include <sean/templates/sean_tmpl.h>
#include <DS/containers/list.cc>
#include <DS/containers/dictionary.cc>
#include <DS/containers/h_array.cc>

class sean_Visitor;
class appid_and_int;
class Conduit;
class ATM_Interface;

#include <sean/ipc/IPC_Visitors.h>
template class list<IPC_DataVisitor *> ;

#include <sean/api/Signalling_API.h>
template class list<ATM_Call *> ;
template class list<ATM_Service *> ;
template class list<ATM_Leaf *> ;
template class list<Incoming_ATM_Call *> ;
template class list<Outgoing_ATM_Call *> ;
class Controller;
class Delayed_User_Callback;
template class dictionary<Controller *, Delayed_User_Callback *> ;

class Buffer;
template class list<Buffer *>;

#include <sean/cc/cc_layer.h>
template class list<call_info*> ;
template class list<serv_info*> ;

#include <sean/qsaal/sscop_visitors.h>
template class list<SSCOPVisitor *> ;

#include <sean/daemon/SwitchFabric.h>
template class list<Binding*> ;

#include <sean/api/Signalling_API.h>
template class dictionary<int, FSM_ATM_Call*> ;
template class dictionary<int, FSM_ATM_Service*> ;
template class dictionary<int, ATM_Leaf*> ;

#include <sean/api/Controllers.h>
template class dictionary<int, Controller*> ;
template class dictionary<int,API_Controller_CallbackTimer*> ;
template class dictionary<ATM_Call*,Delayed_User_Callback*> ;
template class dictionary<ATM_Service*,Delayed_User_Callback*> ;
template class dictionary<ATM_Leaf*,Delayed_User_Callback*> ;

#include <sean/cc/cc_layer.h>
template class dictionary<setup_attributes*, Conduit *> ;
template class dictionary<int, appid_and_int *> ;

#include <sean/ipc/abstract_ipc.h>
#include <sean/cc/cc_layer.h>
template class dictionary< const abstract_local_id* , list<call_info*>* > ;
template class dictionary<const abstract_local_id*, abstract_connection*> ;
template class dictionary<int, call_info *>;

#include <sean/uni_fsm/Q93bCall.h>
#include <sean/uni_fsm/Q93bParty.h>
template class dictionary<int, Party *> ;

#include <sean/uni_fsm/Q93bLeaf.h>
template class dictionary<int, Leaf *> ;

template class dictionary<abstract_local_id const *, list<serv_info *> *> ;

template class dictionary<int, list<int> *> ;

#include <sean/uni_fsm/Q93bParty.h>
template class h_array<int, Party *>;

#include <sean/uni_fsm/Q93bLeaf.h>
template class h_array<int, Leaf *>;

template class dictionary<int, int>;
template class list<ATM_Attributes *>;
template class list<Controller *>;

template class dictionary<int, sean_Visitor *>;
template class dictionary<appid_and_int *, Conduit *>;
template class list<ATM_Interface *>;
template class dictionary<abstract_local_id const *, int>;
template class dictionary<int, ATM_Interface *>;

class sean_io;
template class dictionary<int, sean_io *>;

class lij_key;
template class dictionary<lij_key*, Conduit *>;

// Router.h
class VCAlloc;
template class dictionary<int, VCAlloc *> ;
template class dictionary<int, list<Binding *> *> ;
class NSAP_DCC_ICD_addr;
template class dictionary<int, NSAP_DCC_ICD_addr *> ;
template class dictionary<NSAP_DCC_ICD_addr *, int> ;

// --------------------------------------------------------------------
int compare(ATM_Attributes *const &lhs, ATM_Attributes *const &rhs)
{  return (((long)lhs<(long)rhs)? -1 : ((long)lhs>(long)rhs)? 1 : 0); }

int compare(Controller *const &lhs, Controller *const &rhs)
{  return (((long)lhs<(long)rhs)? -1 : ((long)lhs>(long)rhs)? 1 : 0); }

int compare(ATM_Interface *const & lhs, ATM_Interface *const & rhs) 
{  return (((long)lhs<(long)rhs)? -1 : ((long)lhs>(long)rhs)? 1 : 0); }

int compare(Binding *const & lhs, Binding *const & rhs) 
{  return (((long)lhs<(long)rhs)? -1 : ((long)lhs>(long)rhs)? 1 : 0); }

int compare(ATM_Leaf *const &lhs, ATM_Leaf *const &rhs)
{  return (((long)lhs<(long)rhs)? -1 : ((long)lhs>(long)rhs)? 1 : 0); }

int compare(IPC_DataVisitor *const &lhs, IPC_DataVisitor *const &rhs)
{  return (((long)lhs<(long)rhs)? -1 : ((long)lhs>(long)rhs)? 1 : 0); }

int compare(Incoming_ATM_Call *const &lhs, Incoming_ATM_Call *const &rhs)
{  return (((long)lhs<(long)rhs)? -1 : ((long)lhs>(long)rhs)? 1 : 0); }

int compare(Outgoing_ATM_Call *const &lhs, Outgoing_ATM_Call *const &rhs)
{  return (((long)lhs<(long)rhs)? -1 : ((long)lhs>(long)rhs)? 1 : 0); }

int compare(Buffer *const &lhs, Buffer *const &rhs)
{  return (((long)lhs<(long)rhs)? -1 : ((long)lhs>(long)rhs)? 1 : 0); }

int compare(call_info * const & lhs, call_info * const & rhs)
{
  if (lhs && rhs)
    return compare(*lhs, *rhs);
  return (((long)lhs<(long)rhs) ? -1 : ((long)lhs>(long)rhs) ? 1 : 0);
}

int compare(serv_info * const & lhs, serv_info * const & rhs)
{
  if (lhs && rhs)
    return compare(*lhs, *rhs);
  return (((long)lhs<(long)rhs) ? -1 : ((long)lhs>(long)rhs) ? 1 : 0);
}

int compare(NSAP_DCC_ICD_addr *const & lhs, NSAP_DCC_ICD_addr *const & rhs)
{
  if (lhs->equals(rhs))
    return 0;
  return compare((Addr *)lhs, (Addr *)rhs);
}


extern "C" {

  void exit(int foo) {

    for (int fd = 4; fd < FD_SETSIZE; fd++)
      close(fd);

    int pid = getpid();
    char cmd[255];
    sprintf(cmd, "kill -9 %d",pid);
    system(cmd);
  }
  
  void _exit(int foo) {

    for (int fd = 4; fd < FD_SETSIZE; fd++)
      close(fd);

    int pid = getpid();
    char cmd[255];
    sprintf(cmd, "kill -9 %d",pid);
    system(cmd);
  }

};
