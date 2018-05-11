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
static char const _keys_cc_rcsid_[] =
"$Id: keys.cc,v 1.8 1998/08/25 21:21:30 mountcas Exp $";
#endif
#include <common/cprototypes.h>

#include "keys.h"
#include <sean/ipc/abstract_ipc.h>
#include <FW/basics/diag.h>
#include <codec/uni_ie/UNI40_leaf.h>

extern "C" {
  #include<assert.h>
};


appid_and_int::appid_and_int(const abstract_local_id * id, int x) 
  : _id(id), _x(x) { }

appid_and_int::~appid_and_int() { }

int appid_and_int::get_int(void) const
{  return _x;  }

const abstract_local_id * appid_and_int::get_id(void) const
{  return _id;  }

int appid_and_int::equals(const appid_and_int & rhs)
{
  const appid_and_int * const & val = &rhs;
  return !compare(this, (appid_and_int *const &)val);
}

int compare(appid_and_int *const & a, appid_and_int *const & b) 
{
  int res1 = 0;
  if (a->_id && b->_id) {
    res1 = a->_id->compare(b->_id);
  }
  else if (a->_id && !(b->_id)) res1 = 1;
  else if (b->_id && !(a->_id)) res1 = -1;
  
  if (res1==0) {
    if (a->_x < b->_x) return -1;
    else if (a->_x > b->_x) return +1;
    else return 0;
  }
  else return res1;
}

//------------------------------------------------------------------

setup_attributes::setup_attributes(InfoElem** iearray, int crv) {
  for (int i=0;i<num_ie;i++)
    if (iearray[i]) _ie[i]=iearray[i]->copy();
    else _ie[i]=0;

  _crv = crv;
}

setup_attributes::~setup_attributes() 
{
  for (int i=0;i<num_ie;i++)
    if (_ie[i]) delete _ie[i];
}

// second argument is the service ie
int compare(setup_attributes *const & a, setup_attributes *const & b) 
{  
  for (int i=0;i<num_ie;i++) {
    if (( i == InfoElem::ie_conn_identifier_ix) ||
	( i == InfoElem::UNI40_leaf_sequence_num_ix) ||
	( i == InfoElem::ie_end_pt_ref_ix) ||
	( i == InfoElem::ie_broadband_send_comp_ind_ix) ||

	( i == InfoElem::ie_traffic_desc_ix) ||
	( i == InfoElem::ie_transit_net_sel_ix) ||
	( i == InfoElem::ie_qos_param_ix) ||
	( i == InfoElem::UNI40_alt_traff_desc_ix) ||
	( i == InfoElem::UNI40_min_traff_desc_ix) ||
	( i == InfoElem::UNI40_abr_setup_ix) ||
	( i == InfoElem::UNI40_abr_params_ix))
      continue;
  
    if (!b->_ie[i]) 
      continue; // no constraint

    if ((!a->_ie[i]) ||
	(!a->_ie[i]->equals(b->_ie[i]))) {
      if (!a->_ie[i])
	cout << "setup has no ie #" << i << "\n";
      if (!a->_ie[i]->equals(b->_ie[i])) {
	diag("cc_layer.service",DIAG_DEBUG,"differ in ie#%d\n",i);
	a->_ie[i]->PrintSpecific(cout);
	b->_ie[i]->PrintSpecific(cout);
      }
      
      return 1;
    }
  }
  return 0;
}

int setup_attributes::score( setup_attributes* incoming_call_attributes ) {
  int total_score = 0;
  bool violation = false;

  for (int i=0; ((!violation) && (i<num_ie)); i++) {

    if (!_ie[i]) // because that means this service places
      continue;  // no constraint on the value of this IE

    // else, we do place a constraint on the value of this IE

    int points = 1;

    switch (i) {
     case InfoElem::ie_conn_identifier_ix:        // these are always changing
    case InfoElem::UNI40_leaf_sequence_num_ix:    // and so are irrelevant to
    case InfoElem::ie_end_pt_ref_ix:              // matching setups to services
    case InfoElem::ie_broadband_send_comp_ind_ix:
      break;

    case InfoElem::ie_traffic_desc_ix:        // Someday we may deal with these
    case InfoElem::ie_transit_net_sel_ix:     // in the daemon... for now, the 
    case InfoElem::ie_qos_param_ix:           // end application must decide
    case InfoElem::UNI40_alt_traff_desc_ix:
    case InfoElem::UNI40_min_traff_desc_ix:
    case InfoElem::UNI40_abr_setup_ix:
    case InfoElem::UNI40_abr_params_ix: 
      break;

    case InfoElem::UNI40_leaf_call_id_ix:     // the leaf call ID must match exactly
    default:
      if (( ! incoming_call_attributes->_ie[i]) ||                 // the setup doesn't specify any value, or
	  ( ! incoming_call_attributes->_ie[i]->equals(_ie[i])))   // an unequal value
	{
	  diag("cc_layer.service",DIAG_INFO,">> ie-match violation in ie# %d\n",i);
	  points      = 0;
	  violation   = true;
	  total_score = 0;

	  if ( ! incoming_call_attributes->_ie[i] )
	    cout << "--score-- setup has no ie #" << i << "\n";
	  else
	  if (!incoming_call_attributes->_ie[i]->equals(_ie[i])) {
	    diag("cc_layer.service",DIAG_DEBUG,"differ in ie#%d\n",i);
	    incoming_call_attributes->_ie[i]->PrintSpecific(cout);
	    _ie[i]->PrintSpecific(cout);
	  }
	}

      if ( (i==InfoElem::UNI40_leaf_call_id_ix) &&
	   (incoming_call_attributes->_ie[i]->equals(_ie[i])) )
	points=100;

      break;
    }

    if (!violation) 
      total_score += points;
  }

  if (violation) return -1;
  else return total_score;
}


bool setup_attributes::is_distinguishable_from( setup_attributes* incoming_call_attributes ) {
  bool identical = true;

  for (int i=0; ((identical) && (i<num_ie)); i++) {

    switch (i) {
    case InfoElem::ie_conn_identifier_ix:         // these are always changing
    case InfoElem::UNI40_leaf_sequence_num_ix:    // and so are irrelevant to
    case InfoElem::ie_end_pt_ref_ix:              // matching setups to services
    case InfoElem::ie_broadband_send_comp_ind_ix:

    case InfoElem::ie_traffic_desc_ix:        // Someday we may deal with these
    case InfoElem::ie_transit_net_sel_ix:     // in the daemon... for now, the 
    case InfoElem::ie_qos_param_ix:           // end application must decide
    case InfoElem::UNI40_alt_traff_desc_ix:
    case InfoElem::UNI40_min_traff_desc_ix:
    case InfoElem::UNI40_abr_setup_ix:
    case InfoElem::UNI40_abr_params_ix: 
      break;
    
    default:
      if (( ! _ie[i]) && ( ! incoming_call_attributes->_ie[i])) 
	break;

      if ((_ie[i]) && (incoming_call_attributes->_ie[i]) &&
	  (_ie[i]->equals( incoming_call_attributes->_ie[i] )))
	break;

      // otherwise...
      
      identical=false;
      break;
    }
  }

  return identical;
}


int setup_attributes::call_id(void) {
  int call_id = -1;

  UNI40_lij_call_id * lij_cid = (UNI40_lij_call_id *)_ie[InfoElem::UNI40_leaf_call_id_ix];
  if (lij_cid) {
    call_id = lij_cid->GetValue();
  }
  return call_id;
}


int setup_attributes::seqnum(void) {
  int seq_id = -1;

  UNI40_leaf_sequence_num* lseq = (UNI40_leaf_sequence_num*)( _ie[InfoElem::UNI40_leaf_sequence_num_ix] );
  if (lseq) {
    seq_id = lseq->getSeqNum();
  }

  return seq_id;
}
