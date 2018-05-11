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
static char const _UNIExpander_cc_rcsid_[] =
"$Id: q93b_expander.cc,v 1.8 1998/08/21 15:35:56 mountcas Exp $";
#endif
#include <common/cprototypes.h>

#include <sean/daemon/ccd_config.h>

#include <FW/basics/Conduit.h>
#include <FW/basics/diag.h>
#include <FW/behaviors/Factory.h>
#include <FW/behaviors/Mux.h>
#include <FW/behaviors/Protocol.h>
#include <codec/q93b_msg/generic_q93b_msg.h>
#include <sean/cc/sean_Visitor.h>
#include <sean/cc/cc_layer.h>
#include <sean/cc/keys.h>

#include "q93b_expander.h"
#include "Q93bCall.h"

extern "C" {
#include <stdio.h>
#include <stdlib.h>
};

const VisitorType * CallCreator::_sean_type = 0;
const VisitorType * cref_accessor::_sean_type = 0;
const VisitorType * cid_accessor::_sean_visitor_type=0;

cref_accessor::cref_accessor(void)
{
  if (!_sean_type)
    _sean_type = QueryRegistry(SEAN_VISITOR_NAME);
}

Conduit * cref_accessor::GetNextConduit(Visitor * v)
{ 
  bool insert = false;
  Conduit * c = 0;

  if (v->GetType().Is_A(_sean_type)) {
    sean_Visitor * sv = (sean_Visitor *)v;

    switch (sv->get_op()) {
      case sean_Visitor::setup_ind: 
	
	insert = true; // fall through
	
      case sean_Visitor::leaf_setup_failure_ind: 
	{
	  lij_key* key = new lij_key(sv, lij_key::incoming_message );
	  
	  dic_item di;
	  if (di = _temporary_map.lookup(key)) {
	    Conduit * call = _temporary_map.inf(di);
	    c = call;
	  }
	}
      break;
	
      
    case sean_Visitor::leaf_setup_req:
    case sean_Visitor::leaf_setup_ind: 
      break;

    default: 
      {
	int cref = sv->get_crv();
	dic_item d;
	if (d = _access_map.lookup(cref))
	  c = _access_map.inf(d);
	break;
      }
    }
  }

  return c;
}

cref_accessor::~cref_accessor() { }

bool cref_accessor::Broadcast(Visitor * v)
{
  // not supported
  return false;
}

bool cref_accessor::Add(Conduit * c, Visitor * v)
{
  bool rval = false;

  if (v->GetType().Is_A(_sean_type)) {
    sean_Visitor * sv = (sean_Visitor *)v;

    switch (sv->get_op()) {
      case sean_Visitor::leaf_setup_req:
	{    
	  lij_key* key = new lij_key(sv, lij_key::outgoing_message );

	  // temporary already contains conduit with same key
	  if (_temporary_map.lookup(key))
	    abort();
	  _temporary_map.insert(key, c);
	  rval = true;
	}
	break;

      default:
	{
	  int cref = sv->get_crv();
	  if (_access_map.lookup(cref))
	    return false;
	  _access_map.insert(cref, c);
	  rval = true;
	}
	break;
    }
  }
  return rval;
}

bool cref_accessor::Del(Conduit * c)
{
  dic_item it;
  forall_items(it, _access_map) {
    if (_access_map.inf(it) == c) {
      _access_map.del_item(it);
      return true;
    }
  }
  return false;

  // not found in access map so try temporary
  forall_items(it, _temporary_map) {
    if (_temporary_map.inf(it) == c) {
      _temporary_map.del_item(it);
      return true;
    }
  }
  return false;
}

bool cref_accessor::Del(Visitor * v) {  return false; }

CallCreator::CallCreator(int s) : _side(s)
{
  if (!_sean_type)
    _sean_type = QueryRegistry(SEAN_VISITOR_NAME);
}


void cref_accessor::rebind( lij_key* key, int crv ) {

  dic_item di = _temporary_map.lookup(key);
  assert(di);

  Conduit* call = _temporary_map.inf(di);
  cout << "REBINDING TO crv="<< crv << endl;

  // remove it from the temporary map and insert it into the permanent map.
  _temporary_map.del_item(di);
  
  // invariant: the cref is not already in use
  if (_access_map.lookup(crv))
    abort();
  _access_map.insert(crv, call);
}





Conduit * CallCreator::Create(Visitor * v)
{
  Call     * s = 0;
  Protocol * p = 0;
  Conduit  * c = 0;
  char buf[64];
  
  if (v->GetType().Is_A(_sean_type)) {
    sean_Visitor * sv = (sean_Visitor *)v;

    switch (sv->get_op()) {
      case sean_Visitor::leaf_setup_failure_ind:
      case sean_Visitor::leaf_setup_failure_req:
      case sean_Visitor::leaf_setup_ind:
	c = 0; // will pass thru the factory
	break;

      case sean_Visitor::leaf_setup_req:
	if (_side == USER_SIDE) {
	  s = new Call(USER_SIDE);
	  sprintf(buf, "LeafSetup-UNI");
	}
	break;
      case sean_Visitor::setup_ind:
      case sean_Visitor::setup_req:
	if (_side == NETWORK_SIDE) {
	  s = new Call(NETWORK_SIDE);
	  sprintf(buf, "NetSide-UNI");
	} else {
	  s = new Call(USER_SIDE);
	  sprintf(buf, "UserSide-UNI");
	}
	s->SetIdentity(c);
	break;
      default:
	c = 0;
	v->Suicide();
	break;
    }
  }
  if (s) {
    p = new Protocol(s);
    Register(c = new Conduit(buf, p));
    if (!strcmp(buf, "LeafSetup-UNI"))
      s->SetIdentity(c);
  }
  return c;
}

void CallCreator::Interrupt(SimEvent * e) { }

CallCreator::~CallCreator(void) { }

q93b_expander::q93b_expander(int user) : _user(user)
{
  // upper mux
#if 0
  if (user == USER_SIDE) {
    cid_accessor * accA = new cid_accessor( );
    Mux * mA = new Mux(accA);
    _upper_crefmux = new Conduit(Q93B_UPMUX_NAME_USER, mA);
  }
  else {
#endif
    cref_accessor * accA = new cref_accessor( );
    Mux * mA = new Mux(accA);
    _upper_crefmux = new Conduit(Q93B_UPMUX_NAME, mA);
    //  }

  // factory
  CallCreator * cr;
  if (user == USER_SIDE) 
    cr = new CallCreator(USER_SIDE);
  else
    cr = new CallCreator(NETWORK_SIDE);

  Factory * f = new Factory(cr);
  _q93b_factory = new Conduit(Q93B_FACTORY_NAME, f);

  // lower mux
  cref_accessor * accB = new cref_accessor();
  Mux * mB = new Mux(accB);
  _lower_crefmux = new Conduit (Q93B_DOWNMUX_NAME,mB);

  Join(A_half(_q93b_factory), B_half(_upper_crefmux));
  Join(B_half(_q93b_factory), B_half(_lower_crefmux));

  DefinitionComplete();
}

q93b_expander::~q93b_expander(void)
{
  delete _upper_crefmux;
  delete _lower_crefmux;
  delete _q93b_factory;
}

Conduit * q93b_expander::GetAHalf(void) const { return A_half(_upper_crefmux); }
Conduit * q93b_expander::GetBHalf(void) const { return A_half(_lower_crefmux); }

//-----------------
cid_accessor::cid_accessor(void){
  if (_sean_visitor_type == 0)
    _sean_visitor_type = QueryRegistry(SEAN_VISITOR_NAME);
}


Conduit * cid_accessor::GetNextConduit(Visitor * v) {
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


cid_accessor::~cid_accessor() {
}


bool cid_accessor::Broadcast(Visitor * v) {
  // not supported
  return false;
}


bool cid_accessor::Add(Conduit * c, Visitor * v){
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


bool cid_accessor::Del(Conduit * c){
  dic_item it;
  forall_items(it,_access_map) {
    if (_access_map.inf(it)==c) {
      _access_map.del_item(it);
      return true;
    }
  }
  return false;
}


bool cid_accessor::Del(Visitor * v){
  // not supported;
  return false;
}


//----------------------------------------------------------------------


lij_key::lij_key(sean_Visitor* sv, lij_key::dir d) {

  _dest = 0;
  _src  = 0;
  _seqnum = -1;
  _cid    = -1;
  _setup  = false;

  generic_q93b_msg * msg = sv->share_message();
  InfoElem ** ies = 0;
  if (msg) {
    ies = msg->get_ie_array();
    
    UNI40_lij_call_id* cid = (UNI40_lij_call_id*) ( ies[InfoElem::UNI40_leaf_call_id_ix] );
    if (cid)
      _cid = cid->GetValue();

    UNI40_leaf_sequence_num * sn = (UNI40_leaf_sequence_num*) ( ies[InfoElem::UNI40_leaf_sequence_num_ix] );
    if (sn)
      _seqnum = sn->getSeqNum();

    if ((sv->get_op() == sean_Visitor::setup_req) ||
	(sv->get_op() == sean_Visitor::setup_ind)) 
      _setup = true;
    
    // src is ALWAYS the src of the LIJ request
    // dest is ALWAYS the root of the call

    ie_called_party_num *number = (ie_called_party_num *)(msg->ie(InfoElem::ie_called_party_num_ix));
    if (number) {
      if (d==lij_key::outgoing_message)
	_dest = number->get_addr()->copy();
      else
	_src = number->get_addr()->copy();
    }

    ie_calling_party_num *number2 = (ie_calling_party_num *)(msg->ie(InfoElem::ie_calling_party_num_ix));
    if (number2) {
      if (d==lij_key::outgoing_message)
	_src = number2->get_addr()->copy();
      else
	_dest = number2->get_addr()->copy();
    }
  }
}


lij_key::~lij_key() {
  if (_src) delete _src;
  if (_dest) delete _dest;
}


int compare(lij_key *const & c1, lij_key *const & c2) {
  int same = 0;

  cout << "MATCHING BEGIN\n";

  if (c1->_cid != c2->_cid) 
    same++;
  else {
    cout << c1->_cid << " call id == " <<  c2->_cid << "call id\n";
  }

  if (c1->_seqnum != c2->_seqnum) 
    same++;
  else {
    cout << c1->_seqnum << " seqnum == " <<  c2->_seqnum << "seqnum\n";
  }

  cout << "MATCHING END\n";

  return same;
}
