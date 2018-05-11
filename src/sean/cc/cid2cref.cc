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
static char const _cid2cref_cc_rcsid_[] =
"$Id: cid2cref.cc,v 1.8 1998/09/30 04:18:40 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "cid2cref.h"
#include <sean/cc/keys.h>
#include <sean/cc/sean_Visitor.h>
#include <FW/basics/diag.h>
#include <codec/q93b_msg/generic_q93b_msg.h>
#include <codec/uni_ie/end_pt_ref.h>
#include <codec/uni_ie/ie_base.h>
#include <codec/uni_ie/UNI40_leaf.h>

// -----------------------------------------------------------
cid2crefSimEvent::cid2crefSimEvent(SimEntity * to, SimEntity * from, 
				   int cref, const abstract_local_id * appid, int cid)
  : SimEvent(to, from, CID2CREF_CODE), 
    _cref(cref), 
    _cid(cid),
    _appid((abstract_local_id *)appid)
{ }

cid2crefSimEvent::~cid2crefSimEvent(void) { }

int cid2crefSimEvent::get_cid(void) const { return _cid; }

int cid2crefSimEvent::get_crv(void) const { return _cref; }

const abstract_local_id * cid2crefSimEvent::get_appid(void) const { return _appid; }

void cid2crefSimEvent::set_cid(int cid) { _cid = cid; }

void cid2crefSimEvent::set_crv(int cref) { _cref = cref; }

void cid2crefSimEvent::set_appid(const abstract_local_id * appid) 
{ _appid = (abstract_local_id *)appid; }

// -----------------------------------------------------------

const VisitorType * cid2cref::_sean_vistype = 0;

cid2cref::cid2cref(void) : 
  _next_cid(1)
{
  if (!_sean_vistype)
    _sean_vistype = QueryRegistry(SEAN_VISITOR_NAME);
}

cid2cref::~cid2cref() { }

State * cid2cref::Handle(Visitor * v)
{
  if (v->GetType().Is_A(_sean_vistype)) {
    sean_Visitor  * sv = (sean_Visitor *)v;
    appid_and_int * cid_key = sv->get_cid_key();

    switch (sv->get_op()) {
      case sean_Visitor::setup_req:
	{
	  // api chose the cid/appid
	  appid_and_int * cid_app = sv->get_cid_key();
	  int cid = cid_app->get_int();

	  // call control set the cref
	  int crv = sv->get_crv();
	  add_binding(crv, cid_app);
	  diag("cc_layer.cid2cref", DIAG_INFO, "setup_req caused binding of cref=%lx, cid=%lx\n", crv, cid);
	}
        break;

      case sean_Visitor::leaf_setup_failure_req:
	  diag("cc_layer.cid2cref", DIAG_INFO, 
	       "Letting leaf_setup_failure_ind flow through downwards...\n");
	break;

      case sean_Visitor::leaf_setup_failure_ind:
	{
	  generic_q93b_msg * msg = sv->share_message();
	  InfoElem ** ies = 0;
	  assert(msg);
	  ies = msg->get_ie_array();

	  UNI40_leaf_sequence_num* lseq = (UNI40_leaf_sequence_num*)( ies[InfoElem::UNI40_leaf_sequence_num_ix] );
	  assert(lseq);

	  int seqnum = lseq->getSeqNum();
	  dic_item di = _seqnum2cid.lookup(seqnum);
	  if (di) {
	    _seqnum2cid.del_item(di);
	    diag("cc_layer.cid2cref", DIAG_INFO, "Noted, leaf_setup_failure for leaf_setup_req seq=%x\n", seqnum);
	  }
	  else {
	    diag("cc_layer.cid2cref", DIAG_ERROR, "passing on leaf_setup_failure with seq number %d\n",seqnum);
	  }
	  break;
	}

    case sean_Visitor::setup_ind:
	{
	  // Visitor came from below, it should have the cref set properly
	  int crv = sv->get_crv();

	  int simevent_cid = -1;

	  generic_q93b_msg * msg = sv->share_message();
	  InfoElem ** ies = 0;
	  assert(msg);
	  ies = msg->get_ie_array();
	  UNI40_leaf_sequence_num* lseq = (UNI40_leaf_sequence_num*)( ies[InfoElem::UNI40_leaf_sequence_num_ix] );

	  if (lseq) {
	    int seqnum = lseq->getSeqNum();
	    dic_item di = _seqnum2cid.lookup(seqnum);
	    assert(di);

	    if (di) {
	      appid_and_int * cid_app = _seqnum2cid.inf(di);
	      simevent_cid = cid_app->get_int();
	      _seqnum2cid.del_item(di);
	      add_binding(crv, cid_app);
	      diag("cc_layer.cid2cref", DIAG_INFO, "setup_ind for leaf_setup_req seq=%x, caused binding of cref=%lx, cid=%lx\n", 
		   seqnum, crv, simevent_cid);	      
	    }
	    else {
	      diag("cc_layer.cid2cref", DIAG_ERROR, "foreign setup_ind with seq number %d\n",seqnum);
	    }
	  }
	  else 
	    {
	      sv->set_cid( _next_cid );
	      appid_and_int * cid_app = sv->get_cid_key();
	      add_binding(crv, cid_app);
	      simevent_cid = _next_cid;
	      _next_cid += 2;
	    }

	  cid2crefSimEvent * ccse = new cid2crefSimEvent(this, this, crv, 0, simevent_cid );
	  if (sv->GetReturnEvent()) {
	    diag("cc_layer.cid2cref", DIAG_ERROR, "%s already contains a SimEvent at cid2cref mapper!\n",
		 v->GetType().Name());
	  }
	  sv->SetReturnEvent(ccse);
	  diag("cc_layer.cid2cref", DIAG_INFO, "setup_ind caused attachment of simevent, incoming crv=%x, cid=%x\n", crv, simevent_cid);

	}
        break;
	
      case sean_Visitor::release_comp_req: 
	{
	  int cref = lookup(cid_key);
	  if (cref) 
	    sv->set_crv(cref);
	  else {
	    diag("cc_layer.cid2cref", DIAG_WARNING, "%s: No cref associated with cid key for %s %s!\n",
		 OwnerName(), v->GetType().Name(), sv->op_name());
	    diag("cc_layer.cid2cref", DIAG_WARNING, 
		 "%s: This is okay if its in response to a setup no app was interested in!\n",
		 OwnerName());
	  }
	  remove_binding(cid_key);
	  diag("cc_layer.cid2cref", DIAG_INFO, "release_comp_req unbinding of cref=%lx, cid=%lx\n", 
	       sv->get_crv(), sv->get_cid());
	}
        break;

      case sean_Visitor::release_comp_ind:
	{
	  appid_and_int * ai = lookup(sv->get_crv());    // map cref -> cid
	  if (ai) {
	    sv->set_cid( ai->get_int() );
	    sv->set_shared_app_id( ai->get_id() );
	  } else {
	    diag("cc_layer.cid2cref", DIAG_WARNING, 
		 "No cid/appid association with crv %d seen in release/release_comp indication.\n", 
		 sv->get_crv());
	  }
	  remove_binding(sv->get_crv());  // unbind cref entry
	  diag("cc_layer.cid2cref", DIAG_INFO, "release_comp_ind unbinding of cref=%lx, cid=%lx\n", 
	       sv->get_crv(), sv->get_cid());
	}
        break;
	
      case sean_Visitor::leaf_setup_req:
	{
	  // api chose the cid/appid
	  appid_and_int * cid_app = sv->get_cid_key();
	  int cid = cid_app->get_int();

	  generic_q93b_msg * msg = sv->share_message();
	  InfoElem ** ies = 0;
	  assert(msg);

	  ies = msg->get_ie_array();
	  UNI40_leaf_sequence_num* lseq = (UNI40_leaf_sequence_num*)( ies[InfoElem::UNI40_leaf_sequence_num_ix] );
	  assert(lseq);
	  int seqnum = lseq->getSeqNum();

	  // call control set the sequence number
	  assert( ! _seqnum2cid.lookup(seqnum) );
	  _seqnum2cid.insert(seqnum, cid_app);

	  diag("cc_layer.cid2cref", DIAG_INFO, "leaf_setup_req caused binding of cref=%lx, cid=%lx\n", seqnum, cid);
	}
        break;

      case sean_Visitor::leaf_setup_ind:
	  diag("cc_layer.cid2cref", DIAG_INFO, 
	       "Letting leaf_setup_ind flow through upwards...\n");
	break;

      default:
	if (sv->is_ind()) {
	  // If it is an indication obtain the cid and slam it in
	  appid_and_int * cid_key = lookup( sv->get_crv() );
	  // we better have a binding for this cref
	  diag("cc_layer.cid2cref", DIAG_DEBUG, "%s: Checking for binding for crv %lx to %lx.", 
	       OwnerName(), sv->get_crv(), cid_key);
	  // assert(cid_key);
	  if (cid_key) {
	    sv->set_cid( cid_key->get_int() );
	    sv->set_shared_app_id( cid_key->get_id() );
	  }
	} else if (sv->is_req()) {
	  // It's a req so assert that it's cref matches what we have in the map for that cid
	  int cref = lookup(cid_key);
	  diag("cc_layer.cid2cref", DIAG_INFO, "incoming cref=%lx, stored cref=%lx\n", sv->get_crv(), cref);
	  if( sv->get_crv() != cref ) {
	    diag("cc_layer.cid2cref", DIAG_WARNING, "incoming cref=%lx != stored cref=%lx\n", sv->get_crv(), cref);
	    abort();
	  }
	} else {
	  diag("cc_layer.cid2cref", DIAG_WARNING, "%s: No cref associated with cid key for %s %s!\n",
	       OwnerName(), v->GetType().Name(), sv->op_name());
	}
	break;
    }
    delete cid_key;

    generic_q93b_msg * msg = sv->share_message();
    InfoElem ** ies = msg->get_ie_array();
    if(ies[InfoElem::ie_end_pt_ref_ix])  // P2MP indeed.
      {
	int epr = ((ie_end_pt_ref *)ies[InfoElem::ie_end_pt_ref_ix])->get_epr_value();
	sv->set_lid(epr);
	sv->set_pid(epr);
      }
  }
  
  PassThru(v);
  return this;
}

void cid2cref::Interrupt(SimEvent * se)
{
  if (se->GetCode() == CID2CREF_CODE) {
    cid2crefSimEvent * ccse = (cid2crefSimEvent *)se;

    int cid = ccse->get_cid();
    int cref = ccse->get_crv();
    
    diag("cc_layer.cid2cref", DIAG_INFO, "Interrupt specifies binding of: cref=%lx, cid=%lx\n", cref, cid);

    // zero if it passed through the service factory
    const abstract_local_id * ali = ccse->get_appid();
    if (ali != 0) {
      appid_and_int * info = new appid_and_int(ali, cid);
      change_binding(cref, info);
    } else
      remove_binding( cref );      
    // clean up the SimEvent since we originally made it
    delete se;
  }
}

int cid2cref::lookup(appid_and_int * cid_appid) const
{
  int rval = 0;  // zero is an illegal cref
  
  dic_item di;
  forall_items(di, _cref2cid) {
    appid_and_int * info = _cref2cid.inf(di);
    if (!compare(info, cid_appid)) {
      rval = _cref2cid.key(di);
      break;
    }
  }
  return rval;
}

appid_and_int * cid2cref::lookup(int cref) const
{
  appid_and_int * rval = 0;

  dic_item di;
  if (di = _cref2cid.lookup(cref))
    rval = _cref2cid.inf(di);

  return rval;
}

void cid2cref::add_binding(int cref, appid_and_int * cid_appid)
{
  _cref2cid.insert(cref, cid_appid);
}

void cid2cref::change_binding(int cref, appid_and_int * cid_appid)
{
  dic_item di;

  if (di = _cref2cid.lookup(cref))
    _cref2cid.change_inf(di, cid_appid);
  else
    add_binding(cref, cid_appid);
}

void cid2cref::remove_binding(int cref)
{
  dic_item di;
  if (di = _cref2cid.lookup(cref))
    _cref2cid.del_item(di);
    
}
void cid2cref::remove_binding(appid_and_int * cid_appid)
{
  dic_item di;
  forall_items(di, _cref2cid) {
    if (!compare(_cref2cid.inf(di), cid_appid)) {
      _cref2cid.del_item(di);
      break;
    }
  }
}
