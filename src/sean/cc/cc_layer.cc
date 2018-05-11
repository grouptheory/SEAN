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
static char const _cc_layer_cc_rcsid_[] =
"$Id: cc_layer.cc,v 1.20 1998/09/30 04:18:40 bilal Exp $";
#endif
#include <common/cprototypes.h>


#include "cc_layer.h"
#include "keys.h"
#include "sean_Visitor.h"
#include "cid2cref.h"

#include <sean/daemon/ccd_config.h>
#include <FW/basics/diag.h>
#include <codec/uni_ie/ie.h>
#include <codec/q93b_msg/generic_q93b_msg.h>
#include <sean/templates/constants.h>
#include <sean/api/API_constants.h>

extern "C" {
#include <stdio.h>
#include <stdlib.h>
};

const VisitorType * serv_creator::_sean_visitor_type              = 0;
const VisitorType * networkside_serv_accessor::_sean_visitor_type = 0;
const VisitorType * userside_serv_accessor::_sean_visitor_type    = 0;

dictionary<setup_attributes*, Conduit *>* networkside_serv_accessor::_ptr;
void mon(void) 
{
  diag("cc_layer.service",DIAG_INFO,"size = %d\n", networkside_serv_accessor::_ptr->size());
}

//-----------------
networkside_serv_accessor::networkside_serv_accessor(void)
{
  if (_sean_visitor_type == 0)
    _sean_visitor_type = QueryRegistry(SEAN_VISITOR_NAME);
  _ptr = &_access_map;
}

//-----------------
Conduit * networkside_serv_accessor::GetNextConduit(Visitor * v) 
{ 
  VisitorType vt = v->GetType();
  Conduit * ret = 0;

  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;
    sean_Visitor::sean_Visitor_Type t = sv->get_op();
    switch(t) {
    case sean_Visitor::release_comp_ind: 
      {
	dic_item d;

	forall_items(d,_access_map) {
	  setup_attributes* profile = _access_map.key(d);
	  int crv = profile->crv();
	  if (crv == sv->get_crv()) {
	    ret = _access_map.inf(d);
	  }
	}
      } 
    break;

    case sean_Visitor::leaf_setup_failure_ind: {
      // Match only on the basis of seqnumber

      generic_q93b_msg * msg = sv->share_message();
      if (msg) {
	int seq_id = -2;
	InfoElem** ies = msg->get_ie_array();
	UNI40_leaf_sequence_num* lseq = (UNI40_leaf_sequence_num*)( ies[InfoElem::UNI40_leaf_sequence_num_ix] );
	assert(lseq);
	seq_id = lseq->getSeqNum();

	dic_item d;
	forall_items(d,_access_map) {
	  setup_attributes* profile = _access_map.key(d);
	  int leafserv = profile->seqnum();
	  if (leafserv == seq_id) {
	    ret = _access_map.inf(d);
	    break;
	  }
	}
      }
    } break;

    case sean_Visitor::leaf_setup_ind: {
      // Match only on the basis of call id

      generic_q93b_msg * msg = sv->share_message();
      if (msg) {
	int call_id = -2;
	InfoElem** ies = msg->get_ie_array();
	UNI40_lij_call_id * lij_cid = (UNI40_lij_call_id *)ies[InfoElem::UNI40_leaf_call_id_ix];
	if (lij_cid) {
	  call_id = lij_cid->GetValue();
	}

	dic_item d;
	forall_items(d,_access_map) {
	  setup_attributes* profile = _access_map.key(d);
	  int rootserv = profile->call_id();
	  if (rootserv == call_id) {
	    ret = _access_map.inf(d);
	    break;
	  }
	}
      }
    } break;

    case sean_Visitor::add_party_ind:
    case sean_Visitor::setup_ind: {
      // Match only on the basis of seqnumber

      generic_q93b_msg * msg = sv->share_message();
      assert (msg);
      int seq_id = -2;
      InfoElem** ies = msg->get_ie_array();
      UNI40_leaf_sequence_num* lseq = (UNI40_leaf_sequence_num*)( ies[InfoElem::UNI40_leaf_sequence_num_ix] );
      if (lseq) {
	seq_id = lseq->getSeqNum();

	dic_item d;
	forall_items(d,_access_map) {
	  setup_attributes* profile = _access_map.key(d);
	  int leafserv = profile->seqnum();
	  if (leafserv == seq_id) {
	    ret = _access_map.inf(d);
	    break;
	  }
	}
      }
      else {
	setup_attributes* incoming_call_attr = sv->get_setup_attributes();
	dic_item d, best=0;
	int max_score = -1;
      
	// find "best match" of incoming setup against registered services
	forall_items(d,_access_map) {
	  setup_attributes* profile = _access_map.key(d);
	  int score = profile->score( incoming_call_attr );
	  if (score>max_score) {
	    best = d;
	    max_score = score;
	  }
	}
	delete incoming_call_attr;
	
	if (max_score > -1)
	  ret = _access_map.inf(best);
      }
    }
    break;

    default:
      break;
    }
  }

  return ret;  
}

//-----------------
networkside_serv_accessor::~networkside_serv_accessor() { }

//-----------------
bool networkside_serv_accessor::Broadcast(Visitor * v) 
{
  // not supported
  return false;
}

//-----------------
bool networkside_serv_accessor::Add(Conduit * c, Visitor * v)
{
  VisitorType vt = v->GetType();
  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;
    sean_Visitor::sean_Visitor_Type t = sv->get_op();

    bool good = true;

    switch(t) {
    case sean_Visitor::setup_req: 
    case sean_Visitor::leaf_setup_req: 
    case sean_Visitor::service_register_req:  
      {
	setup_attributes* incoming_call_attr = sv->get_setup_attributes();
	dic_item d, best=0;
	int max_score = -1;

	// make sure there isn't an identical service already

	forall_items(d,_access_map) {
	  setup_attributes* profile = _access_map.key(d);
	  good = profile->is_distinguishable_from( incoming_call_attr );
	  if (!good) break;
	}
	
	if (good)
	  _access_map.insert(incoming_call_attr,c);
	else {
	  diag("cc_layer.service",DIAG_WARNING,"service registration fails, networkside_serv_accessor::Add\n");
	  delete incoming_call_attr;
	}
	return good;
      }
    break;

    default:
      return false;
      break;
    }

    if (!good) {
      switch(t) {
      case sean_Visitor::setup_req: 
	// FIX: 
	// send release complete

      case sean_Visitor::leaf_setup_req: 
	// FIX: 
	// send leaf_setup_failure

      case sean_Visitor::service_register_req:
	// FIX: 
	// send service_register_deny
      
      default:
	break;
      }
    }
  }
  return false;
}

//-----------------
bool networkside_serv_accessor::Del(Conduit * c)
{
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
bool networkside_serv_accessor::Del(Visitor * v)
{
  // not supported
  return false;
}


//-------------------------------------------------------------
// NOTE: userside_serv_accessor is also instantiated in the API
//-------------------------------------------------------------

userside_serv_accessor::userside_serv_accessor(void)
{
  if (_sean_visitor_type == 0)
    _sean_visitor_type = QueryRegistry(SEAN_VISITOR_NAME);
}

//-----------------
Conduit * userside_serv_accessor::GetNextConduit(Visitor * v) 
{ 
  VisitorType vt = v->GetType();
  
  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;
    sean_Visitor::sean_Visitor_Type t = sv->get_op();
    switch(t) {
    case sean_Visitor::setup_ind:

    case sean_Visitor::api_service_construction:
    case sean_Visitor::api_service_destruction:

    case sean_Visitor::service_dereg_accept: 
    case sean_Visitor::service_register_accept: 

    case sean_Visitor::service_dereg_deny: 
    case sean_Visitor::service_register_deny: 

    case sean_Visitor::service_dereg_req: 
    case sean_Visitor::service_register_req: {
      appid_and_int *id=sv->get_sid_key();
      Conduit* ret=0;
      dic_item d;
      if (d = _access_map.lookup(id))
	ret = _access_map.inf(d);
      delete id;
      return ret;
      } break;

    default:
      return 0;
      break;
    }
  }
  return 0;
}

//-----------------
userside_serv_accessor::~userside_serv_accessor() { }

//-----------------
bool userside_serv_accessor::Broadcast(Visitor * v) 
{
  // not supported
  return false;
}

//-----------------
bool userside_serv_accessor::Add(Conduit * c, Visitor * v)
{
  VisitorType vt = v->GetType();
  
  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor * sv = (sean_Visitor*)v;
    sean_Visitor::sean_Visitor_Type t = sv->get_op();
    switch(t) {
    case sean_Visitor::setup_req: {
      generic_q93b_msg * msg = sv->share_message();
      assert (msg->type() == header_parser::setup_msg);
      InfoElem** ies = msg->get_ie_array();
      if( ! ies[InfoElem::UNI40_leaf_call_id_ix] ) {
	return false;
      }
    }
    
    // fall through

    case sean_Visitor::leaf_setup_req: 
    case sean_Visitor::api_service_construction:
    case sean_Visitor::api_service_destruction:
    case sean_Visitor::service_dereg_req: 
    case sean_Visitor::service_register_req: {
      appid_and_int *id=sv->get_sid_key();
      bool ret = false;
      if (_access_map.lookup(id)) {
	diag("cc_layer.service",DIAG_WARNING,"service registration fails, userside_serv_accessor::Add\n");
	ret=false;
      }
      else {
	_access_map.insert(id,c);
	ret=true;
      }
      if (!ret) delete id;
      return ret;
      } break;
    default:
      return false;
      break;
    }
  }
  return false;
}

//-----------------
bool userside_serv_accessor::Del(Conduit * c)
{
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
bool userside_serv_accessor::Del(Visitor * v)
{
  // not supported
  return false;
}


//-------------------------------------------------------
//-------------------------------------------------------
serv_creator::serv_creator(void)
{
  if (_sean_visitor_type == 0)
    _sean_visitor_type = QueryRegistry(SEAN_VISITOR_NAME);
}

//-----------------
Conduit * serv_creator::Create(Visitor * v)
{
  Conduit * ret = 0;

  VisitorType vt = v->GetType();
  if (vt.Is_A(_sean_visitor_type)){
    sean_Visitor* sv = (sean_Visitor*)v;
    sean_Visitor::sean_Visitor_Type t = sv->get_op();
    switch(t) {
    case sean_Visitor::service_register_req: {
      sean_Service * serv_state = new sean_Service(sv);
      Protocol* serv_proto = new Protocol(serv_state);
      Conduit* serv_cond = new Conduit("ATM_Service",serv_proto);
      ret = serv_cond;
    } 
    break;
      
    case sean_Visitor::setup_ind: {
      sv->set_shared_app_id(0);  // so the call_control will reject this setup

      SimEvent * se = sv->GetReturnEvent();
      if (se && se->GetCode() == CID2CREF_CODE) {
	cid2crefSimEvent * ccse = (cid2crefSimEvent*)se;
	ccse->set_appid(0);
	DeliverSynchronous(ccse);
      }
    } 
    break;
      
    // This code will only get executed in the cc_layer.
    // its purpose is to prepare for possible leaf_setup_ind messages

    case sean_Visitor::setup_req: {
      generic_q93b_msg * msg = sv->share_message();
      assert (msg->type() == header_parser::setup_msg);
      InfoElem** ies = msg->get_ie_array();
      if(ies[InfoElem::UNI40_leaf_call_id_ix]) {
	  
	// This setup has specified call_id, so we
	// hope to deal with leaf_setup_ind messages
	// from the network.
	
	LIJ_Root_Sensing_Service* serv_state = new LIJ_Root_Sensing_Service(sv);
	Protocol* serv_proto = new Protocol(serv_state);
	Conduit* serv_cond = new Conduit("Root_LIJ_Service",serv_proto);
	ret = serv_cond;
      }
    }
    break;

      // This code will only get executed in the cc_layer.
      // its purpose is to prepare for possible leaf_setup_failure/setup messages
      // in response to a leaf_setup_request.

    case sean_Visitor::leaf_setup_req: {
      generic_q93b_msg * msg = sv->share_message();
      assert (msg->type() == header_parser::leaf_setup_request_msg);
      InfoElem** ies = msg->get_ie_array();
      assert (ies[InfoElem::UNI40_leaf_call_id_ix]);
	  
      // This setup has specified call_id, so we
      // hope to deal with leaf_setup_ind messages
      // from the network.

      LIJ_Leaf_Sensing_Service* serv_state = new LIJ_Leaf_Sensing_Service(sv);
      Protocol* serv_proto = new Protocol(serv_state);
      Conduit* serv_cond = new Conduit("Leaf_LIJ_Service",serv_proto);
      ret = serv_cond;
    }
    break;

    default:
      break;
    }
  }
  if (ret)
    Register(ret);
  return ret;
}

//-----------------
void serv_creator::Interrupt(SimEvent* e) {  }

//-----------------
serv_creator::~serv_creator(void) { }

//-------------------------------------------------------
//-------------------------------------------------------
cc_layer_expander::cc_layer_expander(int side, int portnumber)
  : _side(side)
{
  Accessor* a;    Mux* m;
  Creator* c;     Factory* f;
  State* s;       Protocol* p;

  if (side == USER_SIDE) {
    a = new networkside_serv_accessor();
    m=new Mux(a);
    _attributes_mux=new Conduit(ATTRIBUTESMUX_NAME,m);

    a = new userside_serv_accessor();
    m=new Mux(a);
    _sid_appid_mux=new Conduit(SIDAPPIDMUX_NAME,m);
  
    c = new serv_creator();
    f=new Factory(c);
    _serv_factory=new Conduit(SERVFACTORY_NAME,f);

    s = new cid2cref();
    p = new Protocol(s);
    _cid2cref_map = new Conduit(CID2CREF_NAME, p);
  }

  call_control *cs = 
    new call_control(side, portnumber);
  _cc_state = cs;

  p=new Protocol(cs);
  _call_control=new Conduit(CALLCONTROL_NAME,p);


  if (side == USER_SIDE) {
    Join(B_half(_call_control), A_half(_sid_appid_mux));
    Join(A_half(_serv_factory), B_half(_sid_appid_mux));
    Join(B_half(_attributes_mux), B_half(_serv_factory));
    Join(A_half(_attributes_mux), A_half(_cid2cref_map));
    Join(B_half(_call_control), A_half(_cid2cref_map));
  }

  if (side == NETWORK_SIDE) {
    // nothing to join, call_control is all there is
  }

  DefinitionComplete();
}

//-----------------
call_control * cc_layer_expander::Get_CC(void) {
  return _cc_state;
}

//-----------------
Conduit * cc_layer_expander::GetAHalf(void) const
{
  return A_half(_call_control);
}

//-----------------
Conduit * cc_layer_expander::GetBHalf(void )const
{
  if (_side == USER_SIDE)
    return B_half(_cid2cref_map);
  else
    return B_half(_call_control);
}

//-----------------
cc_layer_expander::~cc_layer_expander(void) 
{  
  if (_side == USER_SIDE) {
    delete _attributes_mux;
    delete _sid_appid_mux;
    delete _serv_factory;
    delete _cid2cref_map;
  }
  
  delete _call_control;
}


//-------------------------------------------------------
const VisitorType * sean_Service::_sean_type = 0;

//-------------------------------------------------------
sean_Service::sean_Service(sean_Visitor* birther) // queueing
  : _active(false), _initial_joining_phase(true)
{
  if (_sean_type == 0)
    _sean_type = QueryRegistry(SEAN_VISITOR_NAME);

  _sid = birther->get_sid();
  _appid = birther->get_shared_app_id();
}

//-----------------
bool sean_Service::Will_You_Accept(Visitor* examine) {
  return false;
}

//-----------------
State * sean_Service::Handle(Visitor* v) 
{
  _initial_joining_phase = false;

  VisitorType vt = v->GetType();

  if (vt.Is_A(_sean_type)) {
    sean_Visitor* sv = (sean_Visitor*)v;

    if (sv->Valid()) {

      sean_Visitor::sean_Visitor_Type t = sv->get_op();
      switch (t) {
	
      case sean_Visitor::service_register_req: {
	diag("cc_layer.service",DIAG_INFO,"service registration complete\n");
	sean_Visitor * acc = new sean_Visitor(sean_Visitor::service_register_accept);
	acc->set_sid( sv->get_sid() );
	acc->set_shared_app_id( sv->get_shared_app_id() );
	sv->Suicide();
	_active = true;
	PassVisitorToA(acc);
      } break;
	
      case sean_Visitor::service_dereg_req: {
	sean_Visitor* acc= new sean_Visitor(sean_Visitor::service_dereg_accept);
	acc->set_sid( sv->get_sid() );
	acc->set_shared_app_id( sv->get_shared_app_id() );
	PassVisitorToA(acc);
      }
      // Fall through

      case sean_Visitor::api_service_destruction: {
	diag("cc_layer.service",DIAG_INFO,"service deregistration complete\n");
	Free();
	sv->Suicide();
      } break;

      case sean_Visitor::setup_ind: 
	if (!_active) 
	  return this;
	{
	  sv->set_sid(_sid);
	  sv->set_shared_app_id(_appid);

	  SimEvent * se = sv->GetReturnEvent();
	  if (se->GetCode() == CID2CREF_CODE) {
	    cid2crefSimEvent * ccse = (cid2crefSimEvent*)se;
	    ccse->set_appid(_appid);
	    DeliverSynchronous(ccse);
	  }
	  PassThru(sv);
	} break;
	
      default:
	sv->Suicide();
	break;
      }
    }
  }
  else
    PassThru(v);

  return this;
}

//-----------------
void sean_Service::Interrupt(class SimEvent *e) { }

//-----------------
sean_Service::~sean_Service() 
{
  on_destruction();
} 

//-----------------
void sean_Service::on_destruction(void) {
  if ((!_active) && (_initial_joining_phase)) {
    diag("cc_layer.service",DIAG_INFO,"Service sending up service_register_deny... (dtor).\n");
    sean_Visitor* acc= new sean_Visitor(sean_Visitor::service_register_deny);
    acc->set_sid( _sid );
    acc->set_shared_app_id( _appid );
    PassVisitorToA(acc);
  }
}

//----------------
const VisitorType * LIJ_Root_Sensing_Service::_sean_type = 0;

LIJ_Root_Sensing_Service::LIJ_Root_Sensing_Service(sean_Visitor* birther)
  : sean_Service(birther), _delay_setup_visitor(0), _delay_setup_msg(0)
{
  if (_sean_type == 0)
    _sean_type = QueryRegistry(SEAN_VISITOR_NAME);

  _cid = birther->get_cid();
}

bool LIJ_Root_Sensing_Service::Will_You_Accept(Visitor* examine) {
  return false;
}

State* LIJ_Root_Sensing_Service::Handle(Visitor* v)
{
  _initial_joining_phase = false;

  // make dummy called party

  Addr* dummy_addr = newAddr( DELAYED_SETUP_ADDRESS_TOKEN );
  Addr* dest;

  VisitorType vt = v->GetType();

  if (vt.Is_A(_sean_type)) {
    sean_Visitor* sv = (sean_Visitor*)v;
    sean_Visitor::sean_Visitor_Type t = sv->get_op();
    InfoElem ** ies = 0;
    generic_q93b_msg * msg = sv->share_message();
    msg = sv->share_message();
    if (msg) {
      ies = msg->get_ie_array();
    
      switch (t) {

      case sean_Visitor::leaf_setup_ind:

	if (_delay_setup_visitor) {
	  dest = ((ie_calling_party_num*)ies[InfoElem::ie_calling_party_num_ix]) -> get_addr();   // now we know who to call!
	  InfoElem ** ies_saved = _delay_setup_msg->get_ie_array();

	  delete ies_saved[InfoElem::ie_called_party_num_ix];
	  ies_saved[InfoElem::ie_called_party_num_ix] = new ie_called_party_num(dest->copy());
	  _delay_setup_msg->re_encode();

	  UNI40_leaf_sequence_num* lseq = (UNI40_leaf_sequence_num*)( ies[InfoElem::UNI40_leaf_sequence_num_ix] );
	  delete ies_saved[InfoElem::UNI40_leaf_sequence_num_ix];
	  ies_saved[InfoElem::UNI40_leaf_sequence_num_ix] = lseq->copy();

	  assert( _delay_setup_msg->Valid() );     // setup should be complete and accurate now!
	  PassVisitorToB ( _delay_setup_visitor );
	  
	  _delay_setup_msg = 0;
	  _delay_setup_visitor = 0;
	}
	// and the leaf_setup_ind goes on towards the API...
	sv->set_sid(_sid);
	sv->set_cid(_cid);
	sv->set_shared_app_id(_appid);
	break;

      case sean_Visitor::setup_req:

	// we see the setup_req that caused this LIJ_Root_Sensing_Service,
	// for example

	if (_delay_setup_visitor) {
	  _delay_setup_visitor->Suicide();
	  delete _delay_setup_msg;
	  _delay_setup_visitor = 0;
	  _delay_setup_msg = 0;
	}

	dest =  ((ie_called_party_num*)ies[InfoElem::ie_called_party_num_ix]) -> get_addr();
	if (compare(dest,dummy_addr) == 0 ) {
	  _delay_setup_msg = sv->share_message();  
	  _delay_setup_visitor = sv;   
	  sv = 0;
	}
	break;

      case sean_Visitor::release_comp_req:
      case sean_Visitor::release_comp_ind:
	Free();
	break;  // pass visitor up

      default:
	break;
      }
    }

    if (sv) PassThru(sv);
  }
  return this;
}

void LIJ_Root_Sensing_Service::Interrupt(class SimEvent *e)
{
}

LIJ_Root_Sensing_Service::~LIJ_Root_Sensing_Service()
{
}

void LIJ_Root_Sensing_Service::on_destruction(void) {
  if (_initial_joining_phase) {
    diag("cc_layer.service",DIAG_INFO,"LIJ_Root_Sensing_Service sending up ... (dtor).\n");
  }
}

//----------------
const VisitorType * LIJ_Leaf_Sensing_Service::_sean_type = 0;

LIJ_Leaf_Sensing_Service::LIJ_Leaf_Sensing_Service(sean_Visitor* birther)
  : sean_Service(birther)
{
  if (_sean_type == 0)
    _sean_type = QueryRegistry(SEAN_VISITOR_NAME);

  _cid = birther->get_cid();
}

bool LIJ_Leaf_Sensing_Service::Will_You_Accept(Visitor* examine) {
  return false;
}

State* LIJ_Leaf_Sensing_Service::Handle(Visitor* v)
{
  _initial_joining_phase = false;

  VisitorType vt = v->GetType();

  if (vt.Is_A(_sean_type)) {
    sean_Visitor* sv = (sean_Visitor*)v;
    sean_Visitor::sean_Visitor_Type t = sv->get_op();
    switch (t) {
    case sean_Visitor::setup_ind: 
      {
	SimEvent * se = sv->GetReturnEvent();
	assert(se);
	if (se && se->GetCode() == CID2CREF_CODE) {
	  cid2crefSimEvent * ccse = (cid2crefSimEvent*)se;
	  ccse->set_appid(_appid);
	  DeliverSynchronous(ccse);
	}
	Free();
	sv->mark_setup_ind_as_response_to_lsr();
	sv->set_sid(_sid);
	sv->set_cid(_cid);
	sv->set_shared_app_id(_appid);
      }
    break;

    case sean_Visitor::leaf_setup_failure_ind: 
      {
	Free();
	sv->set_shared_app_id(_appid);
	sv->set_sid(_sid);
	sv->set_cid(_cid);
      }
    break;

    default:
      break;
    }

    PassThru(sv);
  }
  
  return this;
}

void LIJ_Leaf_Sensing_Service::Interrupt(class SimEvent *e)
{
}

LIJ_Leaf_Sensing_Service::~LIJ_Leaf_Sensing_Service()
{
}

void LIJ_Leaf_Sensing_Service::on_destruction(void) {
  if (_initial_joining_phase) {
    diag("cc_layer.service",DIAG_INFO,"LIJ_Leaf_Sensing_Service sending up ... (dtor).\n");
  }
}
