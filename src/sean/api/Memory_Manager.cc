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
static char const _MEMORY_MANAGER_CC_rcsid_[] =
"$Id: Memory_Manager.cc,v 1.2 1998/08/06 04:03:26 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "ATM_Interface.h"
#include "Signalling_API.h"
#include "Memory_Manager.h"

Memory_Manager* Memory_Manager::_singleton = 0;

//--------------------------------------
Memory_Manager& Recyclable::The_Recycle_Bin(void) {
  if ( ! Memory_Manager::_singleton )
    Memory_Manager::_singleton = new Memory_Manager();
  return *Memory_Manager::_singleton;
};

//--------------------------------------
Recyclable::Recyclable(void) {
  _user_accessible = true;
}

//--------------------------------------
Recyclable::~Recyclable() {
}

//--------------------------------------
bool Recyclable::User_Accessible(void) const {
  return _user_accessible;
}


//--------------------------------------
void Recyclable::Mark_Inaccessible(void) {
  _user_accessible = true;
}

//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------

void Memory_Manager::Digest_Call(ATM_Call* x) {
  assert(x);
  x->Mark_Inaccessible();
  if (!_call_trash.search(x)) {
    diag("api.memory",DIAG_INFO,"Digesting ATM_Call %x\n",x);
    _call_trash.insert(x);
    Dump();
  }
}

//--------------------------------------
void Memory_Manager::Digest_Service(ATM_Service* x) {
  assert(x);
  x->Mark_Inaccessible();
  if (!_service_trash.search(x)) {
    diag("api.memory",DIAG_INFO,"Digesting ATM_Service %x\n",x);
    _service_trash.insert(x);
    Dump();
  }
}

//--------------------------------------
void Memory_Manager::Digest_Leaf(ATM_Leaf* x) {
  assert(x);
  x->Mark_Inaccessible();
  if (!_leaf_trash.search(x)) {
    diag("api.memory",DIAG_INFO,"Digesting ATM_Leaf %x\n",x);
    _leaf_trash.insert(x);
    Dump();
  }
}

//--------------------------------------
void Memory_Manager::Digest_Interface(ATM_Interface* x) {
  assert(x);
  x->Mark_Inaccessible();
  if (!_interface_trash.search(x)) {
    diag("api.memory",DIAG_INFO,"Digesting ATM_Interface %x\n",x);
    _interface_trash.insert(x);
    Dump();
  }
}


//--------------------------------------
void Memory_Manager::Dump(void) {

  int killed;

  do {
    _call_ready_to_die.clear();
    _service_ready_to_die.clear();
    _leaf_ready_to_die.clear();
    _interface_ready_to_die.clear();

    list_item li; killed=0;

    forall_items(li,_call_trash) {
      ATM_Call* c = _call_trash.inf(li);
      if (c->Ready_To_Die())
	_call_ready_to_die.insert(c);
    }

    forall_items(li,_service_trash) {
      ATM_Service* c = _service_trash.inf(li);
      if (c->Ready_To_Die())
	_service_ready_to_die.insert(c);
    }

    forall_items(li,_leaf_trash) {
      ATM_Leaf* c = _leaf_trash.inf(li);
      if (c->Ready_To_Die())
	_leaf_ready_to_die.insert(c);
    } 

    forall_items(li,_interface_trash) {
      ATM_Interface* c = _interface_trash.inf(li);
      if (c->Ready_To_Die())
	_interface_ready_to_die.insert(c);
    }
    
    forall_items(li, _call_ready_to_die) {
      ATM_Call* c = _call_ready_to_die.inf(li);
      list_item extract = _call_trash.search(c);
      assert(extract);
      _call_trash.del_item(extract);
      diag("api.memory",DIAG_INFO,"Destroying ATM_Call %x\n",c);
      delete c;
      killed ++;
    }

    forall_items(li, _service_ready_to_die) {
      ATM_Service* c = _service_ready_to_die.inf(li);
      list_item extract = _service_trash.search(c);
      assert(extract);
      _service_trash.del_item(extract);
      diag("api.memory",DIAG_INFO,"Destroying ATM_Service %x\n",c);
      delete c;
      killed ++;
    }
    
    forall_items(li, _leaf_ready_to_die) {
      ATM_Leaf* c = _leaf_ready_to_die.inf(li);
      list_item extract = _leaf_trash.search(c);
      assert(extract);
      _leaf_trash.del_item(extract);
      diag("api.memory",DIAG_INFO,"Destroying ATM_Leaf %x\n",c);
      delete c;
      killed ++;
    }

    forall_items(li, _interface_ready_to_die) {
      ATM_Interface* c = _interface_ready_to_die.inf(li);
      list_item extract = _interface_trash.search(c);
      assert(extract);
      _interface_trash.del_item(extract);
      diag("api.memory",DIAG_INFO,"Destroying ATM_Interface %x\n",c);
      delete c;
      killed ++;
    }
  }
  while (killed > 0);

  _call_ready_to_die.clear();
  _service_ready_to_die.clear();
  _leaf_ready_to_die.clear();
  _interface_ready_to_die.clear();
}

//--------------------------------------
Memory_Manager::Memory_Manager() {
}

//--------------------------------------
Memory_Manager::~Memory_Manager() {
}

