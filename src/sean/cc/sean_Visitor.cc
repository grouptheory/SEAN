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
static char const _sean_Visitor_cc_rcsid_[] =
"$Id: sean_Visitor.cc,v 1.15 1998/10/07 05:21:31 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "sean_Visitor.h"
#include "keys.h"

#include <sean/ipc/abstract_ipc.h>

#include <codec/uni_ie/ie.h>
#include <codec/uni_ie/UNI40_bbc.h>
#include <codec/q93b_msg/q93b_msg.h>
#include <codec/q93b_msg/Buffer.h>

#include <sean/templates/constants.h>


sean_Visitor::sean_Visitor(sean_Visitor::sean_Visitor_Type t, 
			   generic_q93b_msg * msg,
			   Buffer * buf)
  : Visitor(_my_type), _crv( NO_CREF ), _flg( NO_FLAG ), _epr( NO_EPR ), 
    _cid( NO_CID ), _sid( NO_SID ), _lid( NO_LID ), 
    _iport( NO_PORT ), _oport( NO_PORT ), 
    _vpi( NO_VPI ), _vci( NO_VCI ), 
    _appid( NO_APPID ), 
    _op(t), _msg(msg), _buf(buf), _p2mp(false)
{
  if (_msg) {
    _crv = _msg->get_crv();
    _flg = _msg->get_crf();

    if ((_msg->type() == header_parser::setup_msg) &&
	(_msg->ie(InfoElem::ie_broadband_bearer_cap_ix))) {
      ie_bbc * bbc = 
	(ie_bbc *)(_msg->ie(InfoElem::ie_broadband_bearer_cap_ix));
      if (bbc->get_conn_type() == ie_bbc::p2mp)
	_p2mp = true;
    }
  }
  else {       // no message, 
    if (buf)  // but yet we are carrying a Buffer!
      assert(t==sean_Visitor::raw_simulation_pdu);
    // then we'd better be shipping raw simulation payloads
  }
}

sean_Visitor::sean_Visitor(Buffer * buf)
  : Visitor(_my_type), _crv( NO_CREF ), _flg( NO_FLAG ), _epr( NO_EPR ),
    _cid( NO_CID ), _sid( NO_SID ), _lid( NO_LID ), 
    _iport( NO_PORT ), _oport( NO_PORT ), 
    _vpi( NO_VPI ), _vci( NO_VCI ), 
    _appid( NO_APPID ), 
    _op(sean_Visitor::unknown), 
    _msg(0), _buf(buf), _p2mp(false)
{
  unsigned char * buffer = buf->data();
  int maxlength = buf->length();
  int index = 0; assert(index + 1 < maxlength);

  // first four bytes of buffer are the op code
  bcopy((const void *)(buffer + index * sizeof(int)), (void *)(&_op), sizeof(int)); 
  index++; assert(index + 1 < maxlength);
  // next four bytes of byffer are the cid
  bcopy((const void *)(buffer + index * sizeof(int)), (void *)(&_cid), sizeof(int)); 
  index++; assert(index+1 < maxlength);
  // following four bytes of buffer are the sid
  bcopy((const void *)(buffer + index * sizeof(int)), (void *)(&_sid), sizeof(int)); 
  index++; assert(index+1 < maxlength);
  // final four bytes of buffer (header) are the lid
  bcopy((const void *)(buffer + index * sizeof(int)), (void *)(&_lid), sizeof(int)); 
  index++; 

  int remaining = maxlength - index * sizeof(int);
  if (remaining > 0) {
    _msg = Parse(buffer + index * sizeof(int), remaining);
  } else _msg = 0;

  if (_msg) {
    _msg->set_external_buffer_on();

    // set call reference value
    _crv = _msg->get_crv();
    _flg = _msg->get_crf();

    // set vpi and vci
    ie_conn_id *cid_ie = 0;
    if (cid_ie = (ie_conn_id *)(_msg->ie(InfoElem::ie_conn_identifier_ix))) {
      _vpi = cid_ie->get_vpi();
      _vci = cid_ie->get_vci();
    }
    // set the epr value
    ie_end_pt_ref *epr_ie = 0;
    if (epr_ie = (ie_end_pt_ref *)(_msg->ie(InfoElem::ie_end_pt_ref_ix)))
      _epr = epr_ie->get_epr_value();

    if ((_msg->type() == header_parser::setup_msg) &&
	(_msg->ie(InfoElem::ie_broadband_bearer_cap_ix))) {
      ie_bbc * bbc = 
	(ie_bbc *)(_msg->ie(InfoElem::ie_broadband_bearer_cap_ix));
      if (bbc->get_conn_type() == ie_bbc::p2mp)
	_p2mp = true;
    }
    
  } else {
    _crv = NO_CREF;
  }
}

bool sean_Visitor::Valid(void) 
{
  switch (_op) {
  case unknown:
  case setup_req:           case setup_ind:
  case call_proc_req:       case call_proc_ind:
  case connect_req:         case connect_ind:
  case release_req:         case release_ind:
  case connect_ack_req:     case connect_ack_ind:
  case setup_comp_req:      case setup_comp_ind:
  case release_comp_req:    case release_comp_ind:
  case status_enq_req:      case status_enq_ind:
  case status_req:          case status_ind:
  case add_party_req:       case add_party_ind:
  case add_party_comp_req:  case add_party_comp_ind:
  case add_party_ack_req:   case add_party_ack_ind:
  case add_party_rej_req:   case add_party_rej_ind:
  case drop_party_req:      case drop_party_ind:
  case drop_party_comp_req: case drop_party_comp_ind:
  case drop_party_ack_req:  case drop_party_ack_ind:
  case leaf_setup_req:      case leaf_setup_ind:
  case restart_req:         case restart_ind:
  case restart_resp:        case restart_comp_ind:
  case restart_ack_req:     case restart_ack_ind:
  case api_setup_ind_response_to_lsr:
    return (_msg && _msg->Valid());
    break;

  case service_register_req:  case service_register_accept:  
                              case service_register_deny:
  case service_dereg_req:     case service_dereg_accept:     
                              case service_dereg_deny:
  case api_call_construction:
  case api_call_destruction:
  case api_service_construction:
  case api_service_destruction:
  case api_leaf_construction:
  case api_leaf_destruction:
  case raw_simulation_pdu:
    return true;
    break;

  default:
    return false;
    break;
  }
}

Buffer * sean_Visitor::take_buffer(void) 
{
  Buffer * answer = _buf;
  _buf = 0;
  if (!answer && _msg) {
    answer = new Buffer( 2048 );
    memcpy(answer->data(), _msg->encoded_buffer(), _msg->encoded_bufferlen());
    answer->set_length( _msg->encoded_bufferlen() );
  }
  return answer;
}

const abstract_local_id * sean_Visitor::get_shared_app_id(void) const { return _appid; }

appid_and_int * sean_Visitor::get_cid_key(void) const 
{
  return new appid_and_int(_appid, _cid);
}

appid_and_int * sean_Visitor::get_sid_key(void) const 
{
  return new appid_and_int(_appid, _sid);
}

appid_and_int * sean_Visitor::get_lid_key(void) const 
{
  return new appid_and_int(_appid, _lid);
}

int sean_Visitor::get_crv(void) const   { return _crv; }
int sean_Visitor::get_flg(void) const   { return _flg; }
int sean_Visitor::get_cid(void) const   { return _cid; }
int sean_Visitor::get_sid(void) const   { return _sid; }
int sean_Visitor::get_lid(void) const   { return _lid; }
int sean_Visitor::get_iport(void) const { return _iport; }
int sean_Visitor::get_oport(void) const { return _oport; }
int sean_Visitor::get_vpi(void) const   { return _vpi; }
int sean_Visitor::get_vci(void) const   { return _vci; }
int sean_Visitor::get_pid(void) const
{
  if (!_msg) return _epr;

  ie_end_pt_ref *epr_ie = 0;
  if (epr_ie = (ie_end_pt_ref *)(_msg->ie(InfoElem::ie_end_pt_ref_ix)))
    return epr_ie->get_epr_value();
  return _epr;
}

int sean_Visitor::get_epr(void) const 
{ 
  return get_pid();
}


sean_Visitor::sean_Visitor_Type sean_Visitor::get_op(void) const { return _op; }

generic_q93b_msg * sean_Visitor::take_message(void) 
{
  generic_q93b_msg* ans = _msg;
  _msg = 0;
  return ans;
}

generic_q93b_msg * sean_Visitor::share_message(void) 
{
  generic_q93b_msg* ans = _msg;
  return ans;
}

void sean_Visitor::give_message(generic_q93b_msg *& new_msg)
{
  _msg = new_msg;

  if ((_msg->type() == header_parser::setup_msg) &&
      (_msg->ie(InfoElem::ie_broadband_bearer_cap_ix))) {
    ie_bbc * bbc = 
      (ie_bbc *)(_msg->ie(InfoElem::ie_broadband_bearer_cap_ix));
    if (bbc->get_conn_type() == ie_bbc::p2mp)
      _p2mp = true;
  }

  if (_buf) {
    _buf->set_length( new_msg->encoded_bufferlen() );
  }

  new_msg = 0;
}

setup_attributes * sean_Visitor::get_setup_attributes(void) const 
{
  if ((!_msg) || 
      ((_op!=sean_Visitor::setup_req) &&                  // root sensing service
       (_op!=sean_Visitor::setup_ind) &&               
       (_op!=sean_Visitor::service_register_req) &&
       (_op!=sean_Visitor::leaf_setup_req) &&
       (_op!=sean_Visitor::leaf_setup_ind) &&
       (_op!=sean_Visitor::leaf_setup_failure_req) &&
       (_op!=sean_Visitor::leaf_setup_failure_ind) &&
       (_op!=sean_Visitor::release_comp_ind))) return 0;

  return new setup_attributes( _msg->get_ie_array(), get_crv() );
}


void sean_Visitor::set_crv(const int x) 
{ 
  _crv = x;
  if (_msg) 
    _msg->set_crv(x);

  if (x & 0x800000)
    _flg = 1;
  else
    _flg = 0;
}

void sean_Visitor::set_flg(const int x) 
{ 
  _flg = x; 
  if (_msg) 
    _msg->set_crf(x);
  
  if (x)
    _crv = _crv | 0x800000;
  else
    _crv = _crv & 0x7FFFFF;
}

void sean_Visitor::set_cid(const int x)   { _cid = x; }
void sean_Visitor::set_sid(const int x)   { _sid = x; }
void sean_Visitor::set_lid(const int x)   { _lid = x; }
void sean_Visitor::set_iport(const int x) { _iport = x; }
void sean_Visitor::set_oport(const int x) { _oport = x; }
void sean_Visitor::set_vpi(const int x)   { _vpi = x; }
void sean_Visitor::set_vci(const int x)   { _vci = x; }
void sean_Visitor::set_pid(const int x)   { set_epr(x); }
void sean_Visitor::set_epr(const int x)   { 
  _epr = x; 
}

void sean_Visitor::set_shared_app_id(const abstract_local_id* x) { _appid = x; }

const VisitorType sean_Visitor::GetType(void) const
{   return VisitorType(GetClassType());  }

sean_Visitor::~sean_Visitor()
{
  SimEvent * se = GetReturnEvent();
  if (se) {
    // crap ...
  }
}

const vistype & sean_Visitor::GetClassType(void) const
{
  return _my_type;
}

bool sean_Visitor::is_call_related(void) const 
{
  switch (_op) {
  case leaf_setup_req:            case leaf_setup_ind:

  case leaf_setup_failure_req:
  case leaf_setup_failure_ind:
    
  case setup_req:             
  case call_proc_req:       case call_proc_ind: 
  case connect_req:         case connect_ind:
  case release_req:         case release_ind:
  case connect_ack_req:     case connect_ack_ind:
  case setup_comp_req:      case setup_comp_ind:
  case release_comp_req:    case release_comp_ind:

  case add_party_req:       case add_party_ind:
  case add_party_comp_req:  case add_party_comp_ind:
  case add_party_ack_req:   case add_party_ack_ind:
  case add_party_rej_req:   case add_party_rej_ind:
  case drop_party_req:      case drop_party_ind:
  case drop_party_comp_req: case drop_party_comp_ind:
  case drop_party_ack_req:  case drop_party_ack_ind:

  case restart_req:         case restart_ind:
  case restart_resp:        case restart_comp_ind:
  case restart_ack_req:     case restart_ack_ind:

  case status_enq_req:      case status_enq_ind:
  case status_req:          case status_ind:

  case api_call_construction:  case api_call_destruction:
  case api_leaf_construction:  case api_leaf_destruction:

  case raw_simulation_pdu:
  case api_setup_ind_response_to_lsr:

    return true;
    break;
  default:
    return false;
  }
}

bool sean_Visitor::is_service_related(void) const 
{
  switch(_op) {
  case service_register_req:      case service_register_accept:  
  case service_register_deny:     case service_dereg_req:     
  case service_dereg_accept:      case service_dereg_deny:

  case api_service_construction:  case api_service_destruction:

  case setup_ind:
    return true;
    break;
  default:
    return false;
    break;
  }
}

sean_Visitor * sean_Visitor::Clone(sean_Visitor_Type t) 
{
  if (!_msg) return 0;
  InfoElem ** ie = _msg->get_ie_array();
  InfoElem ** new_ies = new InfoElem * [ num_ie ];
  int crv  = _crv;
  int flag = _msg->get_crf();
  generic_q93b_msg * spawn;

  for (int i = 0; i < num_ie; i++)
    if (ie[i])
      new_ies[i] = ie[i]->copy();
    else
      new_ies[i] = 0;
  Buffer * buf = new Buffer( 4096 );  // must use some ridiculously large value since 
                                      // the buffer doesn't dynamically resize ...

  switch(t) {
  case sean_Visitor::setup_req:
  case sean_Visitor::setup_ind:
  case api_setup_ind_response_to_lsr:
      spawn = new q93b_setup_message(new_ies, crv, flag, buf);
      break;
    case sean_Visitor::call_proc_req:
    case sean_Visitor::call_proc_ind:
      spawn = new q93b_call_proceeding_message(new_ies, crv, flag, buf);
      break;
    case sean_Visitor::connect_req:
    case sean_Visitor::connect_ind:
      spawn = new q93b_connect_message(new_ies, crv, flag, buf);
      break;
    case sean_Visitor::release_req:
    case sean_Visitor::release_ind:
      spawn = new q93b_release_message(new_ies, crv, flag, buf);
      break;
    case sean_Visitor::connect_ack_req:
    case sean_Visitor::connect_ack_ind:
      spawn = new q93b_connect_ack_message(new_ies, crv, flag, buf);
      break;
    case sean_Visitor::release_comp_req:
    case sean_Visitor::release_comp_ind:
      spawn = new q93b_release_comp_message(new_ies, crv, flag, buf);
      break;
    case sean_Visitor::status_enq_req:
    case sean_Visitor::status_enq_ind:
      spawn = new q93b_status_enq_message(new_ies, crv, flag, buf);
      break;
    case sean_Visitor::status_req:
    case sean_Visitor::status_ind:
      spawn = new q93b_status_message(new_ies, crv, flag, buf);
      break;
    case sean_Visitor::add_party_req:
    case sean_Visitor::add_party_ind:
      spawn = new q93b_add_party_message(new_ies, crv, flag, buf);
      break;
    case sean_Visitor::add_party_ack_req:
    case sean_Visitor::add_party_ack_ind:
      spawn = new q93b_add_party_ack_message(new_ies, crv, flag, buf);
      break;
    case sean_Visitor::add_party_rej_req:
    case sean_Visitor::add_party_rej_ind:
      spawn = new q93b_add_party_rej_message(new_ies, crv, flag, buf);
      break;
    case sean_Visitor::drop_party_req:
    case sean_Visitor::drop_party_ind:
      spawn = new q93b_drop_party_message(new_ies, crv, flag, buf);
      break;
    case sean_Visitor::drop_party_ack_req:
    case sean_Visitor::drop_party_ack_ind:
      spawn = new q93b_drop_party_ack_message(new_ies, crv, flag, buf);
      break;
    case sean_Visitor::leaf_setup_req:
    case sean_Visitor::leaf_setup_ind:
      spawn = new UNI40_leaf_setup_request_message(new_ies, crv, flag, buf);
      break;
    case sean_Visitor::restart_req:
    case sean_Visitor::restart_ind:
      spawn = new q93b_restart_message(new_ies, crv, flag, buf);
      break;
    case sean_Visitor::restart_ack_req:
    case sean_Visitor::restart_ack_ind:
      spawn = new q93b_restart_ack_message(new_ies, crv, flag, buf);
      break;

    case raw_simulation_pdu:
      return 0;
      break;

    default: 
      break;
  }
  // When I pass in new_ies does it make a copy or take the whole array?
  // If it makes a copy I need to delete [] new_ies, and perhaps iterate
  // over all the ies and delete them as well ...
  // Please advise - mountcas
  
  sean_Visitor * clone = 0;
  if (spawn->Valid()) {
    buf->set_length( spawn->encoded_bufferlen() );
    clone = new sean_Visitor(t, spawn, buf);
    clone->set_cid( this->get_cid() );
    clone->set_sid( this->get_sid() );
    clone->set_lid( this->get_lid() );
    clone->set_crv( this->get_crv() );
    clone->set_shared_app_id( this->get_shared_app_id() );
    
    clone->set_iport( this->get_iport() );
    clone->set_oport( this->get_oport() );
    clone->set_vpi( this->get_vpi() );
    clone->set_vci( this->get_vci() );
    clone->set_pid( this->get_pid() );
    clone->set_epr( this->get_epr() );
  } else {
    delete buf;
    delete spawn;
    for (int j = 0; j < num_ie; j++)
      if (new_ies[j])
	delete new_ies[j];
    delete [] new_ies;
  }
  return clone;
}

void sean_Visitor::Transform_into_req(void)
{
  assert (_op < 0);
  _op = (sean_Visitor_Type)(-_op);  // change the sign
}

void sean_Visitor::Transform_into_ind(void)
{
  assert (_op > 0);
  _op = (sean_Visitor_Type)(-_op);
}

const char * sean_Visitor::op_name(void) const
{
  switch (_op) {
  case sean_Visitor::api_setup_ind_response_to_lsr:
    return "api_setup_ind_response_to_lsr";
  case sean_Visitor::setup_req:
    return "setup_req";
  case sean_Visitor::setup_ind:
    return "setup_ind";
  case sean_Visitor::call_proc_req:
    return "call_proc_req";
  case sean_Visitor::call_proc_ind:
    return "call_proc_ind";
  case sean_Visitor::connect_req:
    return "connect_req";
  case sean_Visitor::connect_ind:
    return "connect_ind";
  case sean_Visitor::release_req:
    return "release_req";
  case sean_Visitor::release_ind:
    return "release_ind";
  case sean_Visitor::connect_ack_req:
    return "connect_ack_req";
  case sean_Visitor::connect_ack_ind:
    return "connect_ack_ind";
  case sean_Visitor::setup_comp_req:
    return "setup_comp_req";
  case sean_Visitor::setup_comp_ind:
    return "setup_comp_ind";
  case sean_Visitor::release_comp_req:
    return "release_comp_req";
  case sean_Visitor::release_comp_ind:
    return "release_comp_ind";
  case sean_Visitor::status_enq_req:
    return "status_enq_req";
  case sean_Visitor::status_enq_ind:
    return "status_enq_ind";
  case sean_Visitor::status_req:
    return "status_req";
  case sean_Visitor::status_ind:
    return "status_ind";
  case sean_Visitor::add_party_req:
    return "add_party_req";
  case sean_Visitor::add_party_ind:
    return "add_party_ind";
  case sean_Visitor::add_party_comp_req:
    return "add_party_comp_req";
  case sean_Visitor::add_party_comp_ind:
    return "add_party_comp_ind";
  case sean_Visitor::add_party_ack_req:
    return "add_party_ack_req";
  case sean_Visitor::add_party_ack_ind:
    return "add_party_ack_ind";
  case sean_Visitor::add_party_rej_req:
    return "add_party_rej_req";
  case sean_Visitor::add_party_rej_ind:
    return "add_party_rej_ind";
  case sean_Visitor::drop_party_req:
    return "drop_party_req";
  case sean_Visitor::drop_party_ind:
    return "drop_party_ind";
  case sean_Visitor::drop_party_comp_req:
    return "drop_party_comp_req";
  case sean_Visitor::drop_party_comp_ind:
    return "drop_party_comp_ind";
  case sean_Visitor::drop_party_ack_req:
    return "drop_party_ack_req";
  case sean_Visitor::drop_party_ack_ind:
    return "drop_party_ack_ind";
  case sean_Visitor::leaf_setup_req:
    return "leaf_setup_req";
  case sean_Visitor::leaf_setup_ind:
    return "leaf_setup_ind";
  case sean_Visitor::leaf_setup_failure_req:
    return "leaf_setup_failure_req";
  case sean_Visitor::leaf_setup_failure_ind:
    return "leaf_setup_failure_ind";
  case sean_Visitor::restart_req:
    return "restart_req";
  case sean_Visitor::restart_ind:
    return "restart_ind";
  case sean_Visitor::restart_resp:
    return "restart_resp";
  case sean_Visitor::restart_comp_ind:
    return "restart_comp_ind";
  case sean_Visitor::restart_ack_req:
    return "restart_ack_req";
  case sean_Visitor::restart_ack_ind:
    return "restart_ack_ind";
  case sean_Visitor::service_register_req:
    return "service_register_req";
  case sean_Visitor::service_register_accept:
    return "service_register_accept";
  case sean_Visitor::service_register_deny:
    return "service_register_deny";
  case sean_Visitor::service_dereg_req:
    return "service_dereg_req";
  case sean_Visitor::service_dereg_accept:
    return "service_dereg_accept";
  case sean_Visitor::service_dereg_deny:
    return "service_dereg_deny";
  case sean_Visitor::api_call_construction:
    return "api_call_construction";
  case sean_Visitor::api_service_construction:
    return "api_service_construction";
  case sean_Visitor::api_service_destruction:
    return "api_service_destruction";
  case sean_Visitor::api_leaf_construction:
    return "api_leaf_construction";
  case sean_Visitor::api_leaf_destruction:
    return "api_leaf_destruction";
  case raw_simulation_pdu:
    return "raw_simulation_pdu";
  case sean_Visitor::unknown:
  default:
    return "unknown";
    break;
  }
}

void op_to_str(char* buf, sean_Visitor::sean_Visitor_Type t) 
{
  switch(t) {
  case sean_Visitor::unknown:                   sprintf(buf,"unknown"); break;
  case sean_Visitor::api_setup_ind_response_to_lsr:   sprintf(buf,"api_setup_ind_response_to_lsr"); break;
  case sean_Visitor::setup_req:                 sprintf(buf,"setup_req"); break;
  case sean_Visitor::setup_ind:                 sprintf(buf,"setup_ind"); break;
  case sean_Visitor::call_proc_req:             sprintf(buf,"call_proc_req"); break;
  case sean_Visitor::call_proc_ind:             sprintf(buf,"call_proc_ind"); break;
  case sean_Visitor::connect_req:               sprintf(buf,"connect_req"); break;
  case sean_Visitor::connect_ind:               sprintf(buf,"connect_ind"); break;
  case sean_Visitor::release_req:               sprintf(buf,"release_req"); break;
  case sean_Visitor::release_ind:               sprintf(buf,"release_ind"); break;
  case sean_Visitor::connect_ack_req:           sprintf(buf,"connect_ack_req"); break;
  case sean_Visitor::connect_ack_ind:           sprintf(buf,"connect_ack_ind"); break;
  case sean_Visitor::setup_comp_req:            sprintf(buf,"setup_comp_req"); break;
  case sean_Visitor::setup_comp_ind:            sprintf(buf,"setup_comp_ind"); break;
  case sean_Visitor::release_comp_req:          sprintf(buf,"release_comp_req"); break;
  case sean_Visitor::release_comp_ind:          sprintf(buf,"release_comp_ind"); break;
  case sean_Visitor::status_enq_req:            sprintf(buf,"status_enq_req"); break;
  case sean_Visitor::status_enq_ind:            sprintf(buf,"status_enq_ind"); break;
  case sean_Visitor::status_req:                sprintf(buf,"status_req"); break;
  case sean_Visitor::status_ind:                sprintf(buf,"status_ind"); break;
  case sean_Visitor::add_party_req:             sprintf(buf,"add_party_req"); break;
  case sean_Visitor::add_party_ind:             sprintf(buf,"add_party_ind"); break;
  case sean_Visitor::add_party_comp_req:        sprintf(buf,"add_party_comp_req"); break;
  case sean_Visitor::add_party_comp_ind:        sprintf(buf,"add_party_comp_ind"); break;
  case sean_Visitor::add_party_ack_req:         sprintf(buf,"add_party_ack_req"); break;
  case sean_Visitor::add_party_ack_ind:         sprintf(buf,"add_party_ack_ind"); break;
  case sean_Visitor::add_party_rej_req:         sprintf(buf,"add_party_rej_req"); break;
  case sean_Visitor::add_party_rej_ind:         sprintf(buf,"add_party_rej_ind"); break;
  case sean_Visitor::drop_party_req:            sprintf(buf,"drop_party_req"); break;
  case sean_Visitor::drop_party_ind:            sprintf(buf,"drop_party_ind"); break;
  case sean_Visitor::drop_party_comp_req:       sprintf(buf,"drop_party_comp_req"); break;
  case sean_Visitor::drop_party_comp_ind:       sprintf(buf,"drop_party_comp_ind"); break;
  case sean_Visitor::drop_party_ack_req:        sprintf(buf,"drop_party_ack_req"); break;
  case sean_Visitor::drop_party_ack_ind:        sprintf(buf,"drop_party_ack_ind"); break;
  case sean_Visitor::leaf_setup_req:            sprintf(buf,"leaf_setup_req"); break;
  case sean_Visitor::leaf_setup_ind:            sprintf(buf,"leaf_setup_ind"); break;
  case sean_Visitor::leaf_setup_failure_req:    sprintf(buf,"leaf_setup_failure_req"); break;
  case sean_Visitor::leaf_setup_failure_ind:    sprintf(buf,"leaf_setup_failure_ind"); break;
  case sean_Visitor::restart_req:               sprintf(buf,"restart_req"); break;
  case sean_Visitor::restart_ind:               sprintf(buf,"restart_ind"); break;
  case sean_Visitor::restart_resp:              sprintf(buf,"restart_resp"); break;
  case sean_Visitor::restart_comp_ind:          sprintf(buf,"restart_comp_ind"); break;
  case sean_Visitor::restart_ack_req:           sprintf(buf,"restart_ack_req"); break;
  case sean_Visitor::restart_ack_ind:           sprintf(buf,"restart_ack_ind"); break;
  case sean_Visitor::service_register_req:      sprintf(buf,"service_register_req"); break;
  case sean_Visitor::service_register_accept:   sprintf(buf,"service_register_accept"); break;
  case sean_Visitor::service_register_deny:     sprintf(buf,"service_register_deny"); break;
  case sean_Visitor::service_dereg_req:         sprintf(buf,"service_dereg_req"); break;
  case sean_Visitor::service_dereg_accept:      sprintf(buf,"service_dereg_accept"); break;
  case sean_Visitor::service_dereg_deny:        sprintf(buf,"service_dereg_deny"); break;
  case sean_Visitor::api_call_construction:     sprintf(buf,"api_call_construction"); break;
  case sean_Visitor::api_service_construction:  sprintf(buf,"api_service_construction"); break;
  case sean_Visitor::api_service_destruction:   sprintf(buf,"api_service_destruction"); break;
  case sean_Visitor::api_leaf_construction:     sprintf(buf,"api_leaf_construction"); break;
  case sean_Visitor::api_leaf_destruction:      sprintf(buf,"api_leaf_destruction"); break;
  case sean_Visitor::raw_simulation_pdu:        sprintf(buf,"raw_simulation_pdu"); break;
  default:
    break;
  };
}

void sean_Visitor::Fill_Subtype_Name(char* buf) const 
{
  op_to_str(buf,_op);
}

void sean_Visitor::Name(char* buf) const 
{
  op_to_str(buf,_op);
}

bool sean_Visitor::is_req(void) 
{
  switch(_op) {
  case sean_Visitor::setup_req:
  case sean_Visitor::call_proc_req:
  case sean_Visitor::connect_req:
  case sean_Visitor::release_req:
  case sean_Visitor::connect_ack_req:
  case sean_Visitor::setup_comp_req:
  case sean_Visitor::release_comp_req:
  case sean_Visitor::status_enq_req:
  case sean_Visitor::status_req:
  case sean_Visitor::add_party_req:
  case sean_Visitor::add_party_comp_req:
  case sean_Visitor::add_party_ack_req:
  case sean_Visitor::add_party_rej_req:
  case sean_Visitor::drop_party_req:
  case sean_Visitor::drop_party_comp_req:
  case sean_Visitor::drop_party_ack_req:
  case sean_Visitor::leaf_setup_req:
  case sean_Visitor::leaf_setup_failure_req:
  case sean_Visitor::restart_req:
  case sean_Visitor::restart_resp:
  case sean_Visitor::restart_ack_req:
  case sean_Visitor::service_register_req:
  case sean_Visitor::service_dereg_req:
  case sean_Visitor::api_call_construction:
  case sean_Visitor::api_service_construction:
  case sean_Visitor::api_service_destruction:
  case sean_Visitor::api_leaf_construction:
  case sean_Visitor::api_leaf_destruction:
    return true;
  default:
    return false;
    break;
  };
}

bool sean_Visitor::is_ind(void) 
{
  switch (_op) {
  case sean_Visitor::api_setup_ind_response_to_lsr:
  case sean_Visitor::setup_ind:
  case sean_Visitor::call_proc_ind:
  case sean_Visitor::connect_ind:
  case sean_Visitor::release_ind:
  case sean_Visitor::connect_ack_ind:
  case sean_Visitor::setup_comp_ind:
  case sean_Visitor::release_comp_ind:
  case sean_Visitor::status_enq_ind:
  case sean_Visitor::status_ind:
  case sean_Visitor::add_party_ind:
  case sean_Visitor::add_party_comp_ind:
  case sean_Visitor::add_party_ack_ind:
  case sean_Visitor::add_party_rej_ind:
  case sean_Visitor::drop_party_ind:
  case sean_Visitor::drop_party_comp_ind:
  case sean_Visitor::drop_party_ack_ind:
  case sean_Visitor::leaf_setup_ind:
  case sean_Visitor::leaf_setup_failure_ind:
  case sean_Visitor::restart_ind:
  case sean_Visitor::restart_comp_ind:
  case sean_Visitor::restart_ack_ind:
  case sean_Visitor::service_register_accept:
  case sean_Visitor::service_register_deny:
  case sean_Visitor::service_dereg_accept:
  case sean_Visitor::service_dereg_deny:
    return true;
  default:
    return false;
    break;
  };
}

bool sean_Visitor::IsP2MP(void) const
{ return _p2mp; }

void sean_Visitor::SetP2MP(bool mp)
{ _p2mp = mp; }


void sean_Visitor::mark_setup_ind_as_response_to_lsr(void) {
  assert (_op== sean_Visitor::setup_ind);
  _op=sean_Visitor::api_setup_ind_response_to_lsr;
}

Visitor * sean_Visitor::dup(void) const
{
  return new sean_Visitor(*this);
}

sean_Visitor::sean_Visitor(const sean_Visitor & rhs)
  : Visitor(rhs), _crv(rhs._crv), _flg(rhs._flg), _cid(rhs._cid), _sid(rhs._sid),
    _lid(rhs._lid), _iport(rhs._iport), _oport(rhs._oport), _vpi(rhs._vpi),
    _vci(rhs._vci), _epr(rhs._epr), _p2mp(rhs._p2mp), _appid(rhs._appid),
    _op(rhs._op), _msg(0), _buf(0)
{
  if (rhs._msg)
    _msg = rhs._msg->copy();
  // What the hell do we do about the Buffer?
}
