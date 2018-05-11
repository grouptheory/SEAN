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
static char const _ATM_Attributes_cc_rcsid_[] =
"$Id: ATM_Attributes.cc,v 1.16 1999/03/10 18:44:01 mountcas Exp $";
#endif
#include <common/cprototypes.h>

#include "ATM_Attributes.h"
#include "API_fsms.h"
#include "ATM_Interface.h"
#include "Controllers.h"
#include "Signalling_API.h"
#include <sean/cc/sean_Visitor.h>
#include <codec/q93b_msg/q93b_msg.h>
#include <codec/q93b_msg/leaf_setup.h>
#include <codec/q93b_msg/Buffer.h>
#include <FW/basics/diag.h>
#include <sean/xti/sean_io.h>
#include <codec/uni_ie/UNI40_td.h>


//-----------------------------------------------
void ATM_Attributes::protect(void) {
  _protect=true;
}

//-----------------------------------------------
void ATM_Attributes::unprotect(void) {
  _protect=false;
}

//-----------------------------------------------
generic_q93b_msg* ATM_Attributes::make_freeform(Buffer* buf){
  generic_q93b_msg* msg = new q93b_freeform_message( _ie, _cref, _flag, buf);
  if (msg->Valid()) return msg;
  else {
    delete msg;
    return 0;
  }
}

//-----------------------------------------------
generic_q93b_msg* ATM_Attributes::make_setup(Buffer* buf){
  generic_q93b_msg* msg = new q93b_setup_message( _ie, _cref, _flag, buf);
  if (msg->Valid()) return msg;
  else {
    delete msg;
    return 0;
  }
}

//-----------------------------------------------
generic_q93b_msg* ATM_Attributes::make_release(Buffer* buf){
  generic_q93b_msg* msg = new q93b_release_message( _ie, _cref, _flag, buf);
  if (msg->Valid()) return msg;
  else {
    delete msg;
    return 0;
  }
}

//-----------------------------------------------
generic_q93b_msg* ATM_Attributes::make_release_comp(Buffer* buf){
  generic_q93b_msg* msg = new q93b_release_comp_message( _ie, _cref, _flag, buf);
  if (msg->Valid()) return msg;
  else {
    delete msg;
    return 0;
  }
}

//-----------------------------------------------
generic_q93b_msg* ATM_Attributes::make_connect(Buffer* buf){
  generic_q93b_msg* msg = new q93b_connect_message( _ie, _cref, _flag, buf);
  if (msg->Valid()) return msg;
  else {
    delete msg;
    return 0;
  }
}

//-----------------------------------------------
generic_q93b_msg* ATM_Attributes::make_addparty(Buffer* buf){
  generic_q93b_msg* msg = new q93b_add_party_message( _ie, _cref, _flag, buf);
  if (msg->Valid()) return msg;
  else {
    delete msg;
    return 0;
  }
}

//-----------------------------------------------
generic_q93b_msg* ATM_Attributes::make_dropparty(Buffer* buf){
  generic_q93b_msg* msg = new q93b_drop_party_message( _ie, _cref, _flag, buf);
  if (msg->Valid()) return msg;
  else {
    delete msg;
    return 0;
  }
}


//-----------------------------------------------
generic_q93b_msg* ATM_Attributes::make_addparty_ack(Buffer* buf){
  generic_q93b_msg* msg = new q93b_add_party_ack_message( _ie, _cref, _flag, buf);
  if (msg->Valid()) return msg;
  else {
    delete msg;
    return 0;
  }
}


//-----------------------------------------------
generic_q93b_msg* ATM_Attributes::make_addparty_rej(Buffer* buf){
  generic_q93b_msg* msg = new q93b_add_party_rej_message( _ie, _cref, _flag, buf);
  if (msg->Valid()) return msg;
  else {
    delete msg;
    return 0;
  }
}


//-----------------------------------------------
generic_q93b_msg* ATM_Attributes::make_leaf_setup_request(Buffer* buf){
  generic_q93b_msg* msg = new UNI40_leaf_setup_request_message( _ie, _cref, _flag, buf);
  if (msg->Valid()) return msg;
  else {
    delete msg;
    return 0;
  }
}


//-----------------------------------------------
generic_q93b_msg* ATM_Attributes::make_leaf_setup_failure(Buffer* buf){
  generic_q93b_msg* msg = new UNI40_leaf_setup_failure_message( _ie, _cref, _flag, buf);
  if (msg->Valid()) return msg;
  else {
    delete msg;
    return 0;
  }
}


//-----------------------------------------------
bool ATM_Attributes::Authorize(InfoElem::ie_ix attribute) {

  bool ret = true;

  // wrong state

  if (get_read_only( attribute ))
    ret = ATM_Attributes::failure;

  // either min or alt traffic descriptors, but never both

  if (attribute==InfoElem::UNI40_min_traff_desc_ix) {
    if (_ie[InfoElem::UNI40_alt_traff_desc_ix])
      ret = false;   
  }

  if (attribute==InfoElem::UNI40_alt_traff_desc_ix) {
    if (_ie[InfoElem::UNI40_min_traff_desc_ix])
      ret = false;   
  }

  // must set regular td before setting alternate or the minimum td

  if (attribute==InfoElem::UNI40_min_traff_desc_ix) {
    if ( ! _ie[InfoElem::ie_traffic_desc_ix])
      ret = false;   
  }

  if (attribute==InfoElem::UNI40_alt_traff_desc_ix) {
    if ( ! _ie[InfoElem::ie_traffic_desc_ix])
      ret = false;   
  }

  // setting the regular td zeroes the alternate td and the minimum td

  if (attribute==InfoElem::ie_traffic_desc_ix) {
    if (_ie[InfoElem::UNI40_alt_traff_desc_ix]) {
      delete _ie[InfoElem::UNI40_alt_traff_desc_ix];
      _ie[InfoElem::UNI40_alt_traff_desc_ix] = 0;
    }
    if (_ie[InfoElem::UNI40_min_traff_desc_ix]) {
      delete _ie[InfoElem::UNI40_min_traff_desc_ix];
      _ie[InfoElem::UNI40_min_traff_desc_ix] = 0;
    }
  }

  return ret;
}

//-----------------------------------------------
ATM_Attributes::result ATM_Attributes::set_attr(InfoElem::ie_ix attribute, 
						 InfoElem* ie){
  if (attribute != InfoElem::ie_cause_ix) {
    if (((_indirect[attribute]) || (!_modifiable[attribute])) && (_protect))
      return (ATM_Attributes::failure);
  }

  if (_ie[attribute]!=NULL) {
    delete _ie[attribute];
    _ie[attribute]=0L;
  }

  _ie[attribute]=ie;
  return (ATM_Attributes::success);
}

InfoElem* ATM_Attributes::get_attr (InfoElem::ie_ix attribute) const 
{ return (_ie[attribute]); }



//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_cause(ie_cause::CauseValues err){
  if (!Authorize(InfoElem::ie_cause_ix)) return ATM_Attributes::failure;

  ie_cause *thing = new ie_cause((ie_cause::CauseValues)err, ie_cause::user);
  ATM_Attributes::result answer = set_attr(InfoElem::ie_cause_ix, thing);

  if (answer==ATM_Attributes::failure) {
    delete thing;
    thing=0L;
  }
  return answer;
}


//------------------------------------------------------
ie_cause::CauseValues ATM_Attributes::Get_cause(void){
  ie_cause* ie;
  if ((ie = 
       (ie_cause*)
       get_attr(InfoElem::ie_cause_ix))!=NULL)

    { return (ie->get_cause_value()); }
  else return (ie_cause::unspecified_or_unknown);
}


/* ------- below is not yet supported ------- 

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_aal_1(void)
{ 
  return ATM_Attributes::failure;
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_aal_2(void)
{ 
  return ATM_Attributes::failure;
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_aal_34(void)
{ 
  return ATM_Attributes::failure;
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_aal_1 (void)  const 
{ 
  return ATM_Attributes::failure;
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_aal_2 (void)  const 
{ 
  return ATM_Attributes::failure;
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_aal_34 (void) const 
{
  return ATM_Attributes::failure;
}

   ------- above is not yet supported -------  */



//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_aal_5(aal5::aal5_sscs sscs, 
			      int fm_sdu_size, int bm_sdu_size) { 
  if (!Authorize(InfoElem::ie_aal_param_ix)) return ATM_Attributes::failure;

  aal5 *thing = new aal5((aal5::aal5_sscs)sscs, fm_sdu_size, bm_sdu_size);
  ATM_Attributes::result answer = set_attr(InfoElem::ie_aal_param_ix, thing);

  if (answer==ATM_Attributes::failure) {
    delete thing;
    thing=0L;
  }
  return answer;
}

//------------------------------------------------------
ie_aal::AAL_TYPE ATM_Attributes::Get_aal_type (void) const
{ ie_aal* ie=(ie_aal*)get_attr(InfoElem::ie_aal_param_ix);
  if (!ie) 
    return ie_aal::unspecified_or_unknow;
  else 
    return (ie_aal::AAL_TYPE)((ie!=NULL)?(ie->get_aal_type()):0);
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_aal_5 (aal5::aal5_sscs &sscs, 
				 int &fm_sdu_size, int &bm_sdu_size) const 
{
  if (Get_aal_type()!=ie_aal::aal5_type) return ATM_Attributes::failure; 
  aal5* ie =  (aal5*) get_attr(InfoElem::ie_aal_param_ix);
  if (!ie)
    return ATM_Attributes::failure;
  sscs = (aal5::aal5_sscs) ie->get_sscs();
  fm_sdu_size = ie->get_fm_cpcs_sdu_size();
  bm_sdu_size = ie->get_bm_cpcs_sdu_size(); 
  return ATM_Attributes::success; }

//------------------------------------------------------------
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//------------------------------------------------------------

ATM_Attributes::result ATM_Attributes::Set_ep_ref(int x){
  if ( ! Authorize(InfoElem::ie_end_pt_ref_ix))
    return ATM_Attributes::failure;
  
  ie_end_pt_ref* thing2=new ie_end_pt_ref(x);
  if (set_attr(InfoElem::ie_end_pt_ref_ix, thing2)==ATM_Attributes::failure){
    delete thing2; thing2=0L;
    return ATM_Attributes::failure;
  }
  return ATM_Attributes::success;
}

//------------------------------------------------------
int ATM_Attributes::Get_ep_ref(void) const {
  ie_end_pt_ref* thing2=(ie_end_pt_ref*)get_attr(InfoElem::ie_end_pt_ref_ix);
  if (thing2) return thing2->get_epr_value();
  else return NO_EPR;
}

//------------------------------------------------------
int ATM_Attributes::Get_crv(void) const {
  return _cref;
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_bbc(ie_bbc::Bbc_class bbc,
					       ie_bbc::clipping clip,
					       ie_bbc::conn_type ctype, char atc)

{ 
  if ( ! Authorize(InfoElem::ie_broadband_bearer_cap_ix))
    return ATM_Attributes::failure;

  ie_bbc *thing = new ie_bbc((ie_bbc::Bbc_class) bbc, (ie_bbc::clipping)clip, 
			     (ie_bbc::conn_type)ctype, (ie_bbc::trans_caps)atc);
  ATM_Attributes::result answer = set_attr(InfoElem::ie_broadband_bearer_cap_ix, thing);

  if (answer==ATM_Attributes::failure) {
    delete thing;
    thing=0L;
  }
  return answer;
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_bbc(ie_bbc::Bbc_class & bbc,
                 ie_bbc::clipping & clip,
                 ie_bbc::conn_type & ctype, char & atc) const
{
  ie_bbc *ie = ( ie_bbc *)get_attr(InfoElem::ie_broadband_bearer_cap_ix); 
  if (!ie)
    return ATM_Attributes::failure;
  clip  = (ie_bbc::clipping)      ie->get_clipping();
  ctype = (ie_bbc::conn_type)     ie->get_conn_type();
  atc   = (ie_bbc::trans_caps)    ie->get_ATC();
  return ATM_Attributes::success;
}

//------------------------------------------------------
ie_bbc::Bbc_class ATM_Attributes::Get_bbc_type (void) const
{
  ie_bbc * ie =  (ie_bbc *) get_attr(InfoElem::ie_broadband_bearer_cap_ix); 

  return ((ie_bbc::Bbc_class)((ie!=NULL)?
			      (ie->get_qos_class()):
			      ie_bbc::unspecified_or_unknow));
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_conn_type(ie_bbc::conn_type& ctype) const
{
  ie_bbc * ie =  (ie_bbc *) get_attr(InfoElem::ie_broadband_bearer_cap_ix); 
  if (ie==NULL) {
    return ATM_Attributes::failure;
  }
  else {
    ctype = (ie_bbc::conn_type) (ie->get_conn_type());
    return ATM_Attributes::success;
  }
}

//-----------------------------------------------
bool ATM_Attributes::Is_P2P(void){
  ie_bbc::conn_type ctype;
  Get_conn_type(ctype);
  return (ctype==ie_bbc::p2p);
}


//-----------------------------------------------
bool ATM_Attributes::Is_P2MP(void){
  ie_bbc::conn_type ctype;
  Get_conn_type(ctype);
  return (ctype==ie_bbc::p2mp);
}


//------------------------------------------------------------
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//------------------------------------------------------------

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_blli_IP(void){
  if ( ! Authorize(InfoElem::ie_blli_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result answer;
  ie_blli* thing = new ie_blli(layer2::lan_iso_8802, layer3::none);
  answer = set_attr( InfoElem::ie_blli_ix, thing); 
  return answer;  
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_blli_none(void){
  if ( ! Authorize(InfoElem::ie_blli_ix) )
    return ATM_Attributes::failure;

  if (_ie[InfoElem::ie_blli_ix]!=NULL) {
    delete _ie[InfoElem::ie_blli_ix];
    _ie[InfoElem::ie_blli_ix]=0L;
  }
  return ATM_Attributes::success;
}

//------------------------------------------------------------
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//------------------------------------------------------------

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_bhli_iso(unsigned long x){
  ATM_Attributes::result answer;
  char *tmp = new char [25];
  for (int fill=0;fill<8;fill++) tmp[fill]=0;
  sprintf(tmp,"%lu",x);
  answer = Set_bhli_iso((unsigned char*)tmp,((strlen(tmp)>8)?8:strlen(tmp)));
  delete [] tmp;
  tmp=0L;
  return answer;
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_bhli_user(unsigned long x){
  ATM_Attributes::result answer;
  char *tmp = new char [25];
  for (int fill=0;fill<8;fill++) tmp[fill]=0;
  sprintf(tmp,"%lu",x);
  answer = Set_bhli_user((unsigned char*)tmp,((strlen(tmp)>8)?8:strlen(tmp)));
  delete [] tmp;
  tmp=0L;
  return answer;
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_bhli_iso(unsigned char *buffer, int buflen)
{
  if ( ! Authorize(InfoElem::ie_bhli_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result answer = ATM_Attributes::failure;

  if ((!buffer) || (buflen==0)) return answer;

  iso_bhli *thing  = new iso_bhli(buffer, buflen);
  answer = set_attr( InfoElem::ie_bhli_ix, thing); 

  if (answer==ATM_Attributes::failure) {
    delete thing;
    thing=0L;
  }
  return answer;
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_bhli_user(unsigned char *buffer, int buflen)
{
  if ( ! Authorize(InfoElem::ie_bhli_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result answer = ATM_Attributes::failure;

  if ((!buffer) || (buflen==0)) return answer;

  user_bhli *thing = new user_bhli(buffer, buflen);
  answer = set_attr( InfoElem::ie_bhli_ix, thing); 

  if (answer==ATM_Attributes::failure) {
    delete thing;
    thing=0L;
  }
  return answer;
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_bhli_vendor(int oui, int appid)
{
  if ( ! Authorize(InfoElem::ie_bhli_ix) )
    return ATM_Attributes::failure;

  vendor_bhli *thing = new vendor_bhli(oui, appid);
  ATM_Attributes::result answer = set_attr( InfoElem::ie_bhli_ix, thing);

  if (answer==ATM_Attributes::failure) {
    delete thing;
    thing=0L;
  }
  return answer;
}

//------------------------------------------------------
ie_bhli::bhli_type ATM_Attributes::Get_bhli_type(void) const
{ ie_bhli* ie=(ie_bhli*)get_attr(InfoElem::ie_bhli_ix); 
  return (ie_bhli::bhli_type)
    ((ie!=NULL)?(ie->get_bhli_type()):ie_bhli::unspecified_or_unknow);
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_bhli_iso(unsigned char*& buffer, 
				   int& buflen) const
{ if (Get_bhli_type()!=ie_bhli::iso) return ATM_Attributes::failure;
  ie_bhli* ie=(ie_bhli*)get_attr(InfoElem::ie_bhli_ix); 
  if (!ie)
    return ATM_Attributes::failure;
  buffer = ie->get_bhli_buffer();
  buflen = ie->get_bhli_buflen();
  return ATM_Attributes::success; }

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_bhli_user(unsigned char*& buffer, 
				    int& buflen) const
{ if (Get_bhli_type()!=ie_bhli::user_specific) return ATM_Attributes::failure;
  ie_bhli* ie=(ie_bhli*)get_attr(InfoElem::ie_bhli_ix); 
  if (!ie)
    return ATM_Attributes::failure;
  buffer = ie->get_bhli_buffer();
  buflen = ie->get_bhli_buflen();
  return ATM_Attributes::success; }

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_bhli_vendor(unsigned char*& buffer, 
				      int& buflen) const
{ if (Get_bhli_type()!=ie_bhli::vendor_specific_application_id) return ATM_Attributes::failure;
  ie_bhli* ie=(ie_bhli*)get_attr(InfoElem::ie_bhli_ix); 
  if (!ie)
    return ATM_Attributes::failure;
  buffer = ie->get_bhli_buffer();
  buflen = ie->get_bhli_buflen();
  return ATM_Attributes::success; }

//------------------------------------------------------------
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//------------------------------------------------------------

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_called_party_num(Addr* dst_addr)
{
  if ( ! Authorize(InfoElem::ie_called_party_num_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result answer = ATM_Attributes::failure;
  if (!dst_addr) return answer;

  ie_called_party_num *thing = new ie_called_party_num(dst_addr);
  answer = set_attr( InfoElem::ie_called_party_num_ix, thing);

  if (answer==ATM_Attributes::failure) {
    delete thing;
    thing=0L;
  }
  return answer;
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_called_party_num(Addr*& dst_addr) const { 
  ie_called_party_num* ie;
  if ((ie = 
       (ie_called_party_num*)
       get_attr(InfoElem::ie_called_party_num_ix))!=NULL)

    { dst_addr=ie->get_addr()->copy(); return ATM_Attributes::success; }
  else return ATM_Attributes::failure;
}

//------------------------------------------------------------
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//------------------------------------------------------------

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_called_party_subaddr(Addr* dst_subaddr)
{
  if ( ! Authorize(InfoElem::ie_called_party_subaddr_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result answer = ATM_Attributes::failure;
  if (!dst_subaddr) return answer;

  ie_called_party_subaddr *thing = new ie_called_party_subaddr(dst_subaddr);
  answer = set_attr(InfoElem::ie_called_party_subaddr_ix, thing); 

  if (answer==ATM_Attributes::failure) {
    delete thing;
    thing=0L;
  }
  return answer;
}

//------------------------------------------------------
ATM_Attributes::result 
ATM_Attributes::Get_called_party_subaddr(Addr*& dst_subaddr) const { 
  ie_called_party_subaddr* ie;
  if ((ie = 
       (ie_called_party_subaddr*)
       get_attr(InfoElem::ie_called_party_subaddr_ix))!=NULL)

    { dst_subaddr=ie->get_subaddr()->copy(); return ATM_Attributes::success; }
  else return ATM_Attributes::failure;
}

//------------------------------------------------------------
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//------------------------------------------------------------

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_calling_party_num(Addr* src_addr, 
	     ie_calling_party_num::presentation_indicator presentation, 
		ie_calling_party_num::screening_indicator screening)
{
  if ( ! Authorize(InfoElem::ie_calling_party_num_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result answer = ATM_Attributes::failure;
  if (!src_addr) return answer;

  ie_calling_party_num *thing = new ie_calling_party_num
    (src_addr, (ie_calling_party_num::presentation_indicator)presentation,
     (ie_calling_party_num::screening_indicator)screening);

  answer = set_attr( InfoElem::ie_calling_party_num_ix, 
				       thing);

  if (answer==ATM_Attributes::failure) {
    delete thing;
    thing=0L;
  }
  return answer;
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_calling_party_num(Addr*& src_addr, 
     ie_calling_party_num::presentation_indicator& presentation, 
	   ie_calling_party_num::screening_indicator& screening) const
{ ie_calling_party_num* ie;
  if ((ie = 
       (ie_calling_party_num*) 
       get_attr(InfoElem::ie_calling_party_num_ix))!=NULL){

    src_addr     = ie->get_addr()->copy();
    presentation = (ie_calling_party_num::presentation_indicator) 
      ie->get_presentation();
    screening    = (ie_calling_party_num::screening_indicator)    
      ie->get_screening(); 
    return ATM_Attributes::success; }
  else return ATM_Attributes::failure;
}

//------------------------------------------------------------
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//------------------------------------------------------------

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_calling_party_subaddr(Addr * src_subaddr)
{
  if ( ! Authorize(InfoElem::ie_calling_party_subaddr_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result answer = ATM_Attributes::failure;
  if (!src_subaddr) return answer;

  ie_calling_party_subaddr * thing = 
    new ie_calling_party_subaddr(src_subaddr);
  answer = set_attr(InfoElem::ie_calling_party_subaddr_ix, thing);

  if (answer==ATM_Attributes::failure) {
    delete thing;
    thing=0L;
  }
  return answer;
}

//------------------------------------------------------
ATM_Attributes::result 
  ATM_Attributes::Get_calling_party_subaddr(Addr*& src_subaddr) const
{ ie_calling_party_subaddr* ie;
  if ((ie = 
       (ie_calling_party_subaddr*) 
       get_attr(InfoElem::ie_calling_party_subaddr_ix))!=NULL){

    src_subaddr = ie->get_subaddr()->copy();
    return ATM_Attributes::success; }
  else return ATM_Attributes::failure;
}


//------------------------------------------------------------
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//------------------------------------------------------------

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_qos_param(
			 ie_qos_param::qos_indicator fqos_class, 
			 ie_qos_param::qos_indicator bqos_class)
{
  if ( ! Authorize(InfoElem::ie_qos_param_ix) )
    return ATM_Attributes::failure;

  ie_qos_param *thing = 
    new ie_qos_param((ie_qos_param::qos_indicator)fqos_class, 
		     (ie_qos_param::qos_indicator)bqos_class);
  ATM_Attributes::result answer = set_attr(InfoElem::ie_qos_param_ix, thing);

  if (answer==ATM_Attributes::failure) {
    delete thing;
    thing=0L;
  }
  return answer;

}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_qos_param(
	      ie_qos_param::qos_indicator& fqos_class, 
	      ie_qos_param::qos_indicator& bqos_class) const
{ ie_qos_param *ie;
  if ((ie = (ie_qos_param*) get_attr(InfoElem::ie_qos_param_ix)) != NULL) {
    fqos_class = (ie_qos_param::qos_indicator)(ie->get_fqos_class());
    bqos_class = (ie_qos_param::qos_indicator)(ie->get_bqos_class());
    return ATM_Attributes::success; }
  else return ATM_Attributes::failure;
}

//------------------------------------------------------------
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//------------------------------------------------------------

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_td_1(IE_TRAFFIC_DESC::dir d, 
					int pcr_0, int pcr_01){
  if ( ! Authorize(InfoElem::ie_traffic_desc_ix) )
    return ATM_Attributes::failure;

  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::ie_traffic_desc_ix)) == NULL) {
    ie=new IE_TRAFFIC_DESC;
    set_attr((InfoElem::ie_traffic_desc_ix),ie); }

  ie->set_TP2((IE_TRAFFIC_DESC::dir)d, pcr_0, pcr_01);
  return(ATM_Attributes::success);
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_td_3(IE_TRAFFIC_DESC::dir d, 
					int pcr_01, 
					int scr_0, int mbs_0) {
  if ( ! Authorize(InfoElem::ie_traffic_desc_ix) )
    return ATM_Attributes::failure;

  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::ie_traffic_desc_ix)) == NULL) { 
    ie=new IE_TRAFFIC_DESC;
    set_attr((InfoElem::ie_traffic_desc_ix),ie); }

  ie->set_TP3((IE_TRAFFIC_DESC::dir)d,pcr_01,scr_0,mbs_0);
  return(ATM_Attributes::success);
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_td_4(IE_TRAFFIC_DESC::dir d, 
					int pcr_01, 
					int scr_0, int mbs_0) {
  if ( ! Authorize(InfoElem::ie_traffic_desc_ix) )
    return ATM_Attributes::failure;

  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::ie_traffic_desc_ix)) == NULL) { 
    ie=new IE_TRAFFIC_DESC;
    set_attr((InfoElem::ie_traffic_desc_ix),ie); }

  ie->set_TP4((IE_TRAFFIC_DESC::dir)d,pcr_01,scr_0,mbs_0);
  return(ATM_Attributes::success);
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_td_5(IE_TRAFFIC_DESC::dir d,
					int pcr_01){
  if ( ! Authorize(InfoElem::ie_traffic_desc_ix) )
    return ATM_Attributes::failure;

  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::ie_traffic_desc_ix)) == NULL) { 
    ie=new IE_TRAFFIC_DESC;
    set_attr((InfoElem::ie_traffic_desc_ix),ie); }

  ie->set_TP5((IE_TRAFFIC_DESC::dir)d,pcr_01);
  return(ATM_Attributes::success);
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_td_6(IE_TRAFFIC_DESC::dir d, 
					int pcr_01,  
					int scr_01, int mbs_01)
{
  if ( ! Authorize(InfoElem::ie_traffic_desc_ix) )
    return ATM_Attributes::failure;

  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::ie_traffic_desc_ix)) == NULL) { 
    ie=new IE_TRAFFIC_DESC;
    set_attr((InfoElem::ie_traffic_desc_ix),ie); }

  ie->set_TP6((IE_TRAFFIC_DESC::dir)d,pcr_01,scr_01,mbs_01);
  return(ATM_Attributes::success);
} 

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_td_BE(int fpcr_01, int bpcr_01){
  if ( ! Authorize(InfoElem::ie_traffic_desc_ix) )
    return ATM_Attributes::failure;

  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::ie_traffic_desc_ix)) == NULL) { 
    ie=new IE_TRAFFIC_DESC;
    set_attr((InfoElem::ie_traffic_desc_ix),ie); }

  ie->set_BE(fpcr_01,bpcr_01);
  return(ATM_Attributes::success);
}  

//------------------------------------------------------
IE_TRAFFIC_DESC::VALID_TPC ATM_Attributes::Get_td_type(IE_TRAFFIC_DESC::dir d) const {
  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::ie_traffic_desc_ix)) != NULL) 
    switch(d){
    case IE_TRAFFIC_DESC::fw: return ((IE_TRAFFIC_DESC::VALID_TPC)(ie->ftpc())); break;
    case IE_TRAFFIC_DESC::bw: return ((IE_TRAFFIC_DESC::VALID_TPC)(ie->btpc())); break;
    }
  return (IE_TRAFFIC_DESC::VALID_TPC)IE_TRAFFIC_DESC::TPC_UNSPECIFIED;
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_td_1(IE_TRAFFIC_DESC::dir d, int& pcr_0, 
						  int& pcr_01) const {
  if (Get_td_type(d)!=IE_TRAFFIC_DESC::TPC1) return ATM_Attributes::failure;
  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::ie_traffic_desc_ix)) == NULL) return ATM_Attributes::failure;
  switch(d){
  case IE_TRAFFIC_DESC::fw:
    pcr_0 = ie->get_FPCR_0();
    pcr_01 = ie->get_FPCR_01();
    break;
  case IE_TRAFFIC_DESC::bw:
    pcr_0 = ie->get_BPCR_0();
    pcr_01 = ie->get_BPCR_01();
    break;
  }
  return ATM_Attributes::success; 
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_td_2(IE_TRAFFIC_DESC::dir d, int& pcr_0, 
						  int& pcr_01) const {
  if (Get_td_type(d)!=IE_TRAFFIC_DESC::TPC2) return ATM_Attributes::failure;
  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::ie_traffic_desc_ix)) == NULL) return ATM_Attributes::failure;
  switch(d){
  case IE_TRAFFIC_DESC::fw:
    pcr_0 = ie->get_FPCR_0();
    pcr_01 = ie->get_FPCR_01();
    break;
  case IE_TRAFFIC_DESC::bw:
    pcr_0 = ie->get_BPCR_0();
    pcr_01 = ie->get_BPCR_01();
    break;
  }
  return ATM_Attributes::success; 
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_td_3(IE_TRAFFIC_DESC::dir d, int& pcr_01, 
						  int& scr_0, int& mbs_0) const {
  if (Get_td_type(d)!=IE_TRAFFIC_DESC::TPC3) return ATM_Attributes::failure;
  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::ie_traffic_desc_ix)) == NULL) return ATM_Attributes::failure;
  switch(d){
  case IE_TRAFFIC_DESC::fw:
    pcr_01 = ie->get_FPCR_01();
    scr_0 = ie->get_FSCR_0();
    mbs_0 = ie->get_FMBS_0();
    break;
  case IE_TRAFFIC_DESC::bw:
    pcr_01 = ie->get_BPCR_01();
    scr_0 = ie->get_BSCR_0();
    mbs_0 = ie->get_BMBS_0();
    break;
  }
  return ATM_Attributes::success; 
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_td_4(IE_TRAFFIC_DESC::dir d, int& pcr_01, 
						  int& scr_0, int& mbs_0) const {
  if (Get_td_type(d)!=IE_TRAFFIC_DESC::TPC4) return ATM_Attributes::failure;
  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::ie_traffic_desc_ix)) == NULL) return ATM_Attributes::failure;
  switch(d){
  case IE_TRAFFIC_DESC::fw:
    pcr_01 = ie->get_FPCR_01();
    scr_0  = ie->get_FSCR_0();
    mbs_0  = ie->get_FMBS_0();
    break;
  case IE_TRAFFIC_DESC::bw:
    pcr_01 = ie->get_BPCR_01();
    scr_0  = ie->get_BSCR_0();
    mbs_0  = ie->get_BMBS_0();
    break;
  }
  return ATM_Attributes::success; 
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_td_5(IE_TRAFFIC_DESC::dir d, int& pcr_01) const {
  if (Get_td_type(d)!=IE_TRAFFIC_DESC::TPC5) return ATM_Attributes::failure;
  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::ie_traffic_desc_ix)) == NULL) return ATM_Attributes::failure;
  switch(d){
  case IE_TRAFFIC_DESC::fw:
    pcr_01 = ie->get_FPCR_01();
    break;
  case IE_TRAFFIC_DESC::bw:
    pcr_01 = ie->get_BPCR_01();
    break;
  }
  return ATM_Attributes::success; 
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_td_6(IE_TRAFFIC_DESC::dir d, int& pcr_01,  int& scr_01, 
			       int& mbs_01) const
{
  if (Get_td_type(d)!=IE_TRAFFIC_DESC::TPC6) return ATM_Attributes::failure;
  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::ie_traffic_desc_ix)) == NULL) return ATM_Attributes::failure;
  switch(d){
  case IE_TRAFFIC_DESC::fw:
    pcr_01 = ie->get_FPCR_01();
    scr_01 = ie->get_FSCR_01();
    mbs_01 = ie->get_FMBS_01();
    break;
  case IE_TRAFFIC_DESC::bw:
    pcr_01 = ie->get_BPCR_01();
    scr_01 = ie->get_BSCR_01();
    mbs_01 = ie->get_BMBS_01();
    break;
  }
  return ATM_Attributes::success; }

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_td_BE(int& fpcr_01, int& bpcr_01) const { 
  if (Get_td_type(IE_TRAFFIC_DESC::fw)!=IE_TRAFFIC_DESC::TPC7) return ATM_Attributes::failure;
  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::ie_traffic_desc_ix)) == NULL) return ATM_Attributes::failure;
  fpcr_01 = ie->get_FPCR_01();
  bpcr_01 = ie->get_BPCR_01();
  return ATM_Attributes::success; }


//------------------------------------------------------------
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//------------------------------------------------------------

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_vpi(long& vpi) const { 
  ie_conn_id *ie;
  if ((ie = (ie_conn_id*) 
       get_attr(InfoElem::ie_conn_identifier_ix)) != NULL) {
    vpi = ie->get_vpi();
    return ATM_Attributes::success;
  }
  else return ATM_Attributes::failure; }

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_vci(long& vci) const { 
  ie_conn_id *ie;
  if ((ie = (ie_conn_id*) 
       get_attr(InfoElem::ie_conn_identifier_ix)) != NULL) {
    vci = ie->get_vci();
    return ATM_Attributes::success;
  }
  else return ATM_Attributes::failure; }

//------------------------------------------------------------
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//------------------------------------------------------------

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Set_qos(ie_qos_param::qos_indicator fq, ie_qos_param::qos_indicator bq)
{
  if ( ! Authorize(InfoElem::ie_qos_param_ix) )
    return ATM_Attributes::failure;

  ie_qos_param *thing = new ie_qos_param((ie_qos_param::qos_indicator)fq,
					 (ie_qos_param::qos_indicator)bq);

  ATM_Attributes::result answer = set_attr(InfoElem::ie_qos_param_ix, thing);

  if (answer==ATM_Attributes::failure) {
    delete thing;
    thing=0L;
  }
  return answer;
}

//------------------------------------------------------
ATM_Attributes::result ATM_Attributes::Get_qos(IE_TRAFFIC_DESC::dir d, ie_qos_param::qos_indicator& qos) const {
  ie_qos_param *ie;
  if ((ie = (ie_qos_param*) 
       get_attr(InfoElem::ie_qos_param_ix)) != NULL) {
    switch(d){
    case IE_TRAFFIC_DESC::fw: qos = (ie_qos_param::qos_indicator)(ie->get_fqos_class()); break;
    case IE_TRAFFIC_DESC::bw: qos = (ie_qos_param::qos_indicator)(ie->get_bqos_class()); break;
    }
    return ATM_Attributes::success;
  }
  else return ATM_Attributes::failure;
}

//-----------------------------------------------
ATM_Interface& ATM_Attributes::Get_interface(void) const {
  return *_interface;
}


//-----------------------------------------------
bool ATM_Attributes::Associate_Controller(Controller& c) {
  Divorce_Controller();
  bool ret = true;
  _control=&c;

  switch(_fsmtype) {
  case ATM_Attributes::Call_FSM:
    _control->adopt_managed_call(this);
    break;

  case ATM_Attributes::Service_FSM:
    _control->adopt_managed_service(this);
    break;

  default:
    ret = false;
    break;
  }
  return ret;
}

//-----------------------------------------------
bool ATM_Attributes::Divorce_Controller(void) {
  bool ret = true;
  switch(_fsmtype) {
  case ATM_Attributes::Call_FSM:
    if (_control) 
      _control->abandon_managed_call(this);
    break;

  case ATM_Attributes::Service_FSM:
    if (_control) 
      _control->abandon_managed_service(this);
    break;

  default:
    ret = false;
    break;
  }
  return ret;
}

//------------------------------------------------------------
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//------------------------------------------------------------

API_FSM::api_fsm_state ATM_Attributes::get_fsm_state(void) const {
  if (_fsm) {
    return(_fsm->get_state());
  }
  else return(API_FSM::no_fsm);
}


//-----------------------------------------------
// our calls and service

ATM_Attributes::ATM_Attributes(ATM_Interface& interface, fsm_type t)
  : _interface(&interface), _control(0), _protect(true)
{
  diag("api.attributes",DIAG_DEBUG,"ATM_Attributes-- begin constructing\n");
  for (int i=0;i<num_ie;i++) {
    _ie[i]=(InfoElem *)0; 
    set_inherited((InfoElem::ie_ix)i,false);
  }
  reset_modifiable_mask( true );

  diag("api.attributes",DIAG_DEBUG,"ATM_Attributes-- done constructing\n");
  
  _fsm=0;
  _id=0;
  set_crv( 0 );

  switch(t) {
  case ATM_Attributes::Call_FSM:
    _fsm = interface.make_call(_id); break;
  case ATM_Attributes::Service_FSM:
    _fsm = interface.make_service(_id); break;
  default:
    break;
  }
  if (_fsm) {
    _fsm->associate_attributes(this);
    _fsmtype=t;
    set_modifiable_bits( _fsm->get_state() );
  }
  else _fsmtype=ATM_Attributes::unknown_FSM;
}


//-----------------------------------------------
// our leaves, made locally

ATM_Attributes::ATM_Attributes(ATM_Interface& interface, 
			       Outgoing_ATM_Call& call, fsm_type t)
  : _interface(&interface), _control(0), _protect(true)
{
  diag("api.attributes",DIAG_DEBUG,"ATM_Attributes-- begin constructing\n");
  for (int i=0;i<num_ie;i++) {
    InfoElem* ie=call._ie[(InfoElem::ie_ix)i];

    if (ie) _ie[i]=ie->copy();
    else _ie[i]=(InfoElem *)0; 

    set_inherited((InfoElem::ie_ix)i,false);
  }
  diag("api.attributes",DIAG_DEBUG,"ATM_Attributes-- done constructing\n");
  reset_modifiable_mask( true );

  _fsm=0;

  switch(t) {
  case ATM_Attributes::Leaf_FSM: {
    set_crv( call.Get_crv() );

    _id = call.Get_Next_EPR();
    _fsm = interface.make_leaf( call.Get_Call_ID() , _id); 
    unprotect();
    ATM_Attributes::result res = Set_ep_ref(_id);
    protect();
    assert( res == ATM_Attributes::success);
  } break;

  default:
    break;
  }
  if (_fsm) {
    _fsm->associate_attributes(this);
    _fsmtype=t;
    set_modifiable_bits( _fsm->get_state() );
  }
  else _fsmtype=ATM_Attributes::unknown_FSM;
}


//-----------------------------------------------
// incoming calls

ATM_Attributes::ATM_Attributes(ATM_Interface& interface, 
			       generic_q93b_msg* msg, 
			       int cid) 
  : _interface(&interface), _control(0), _protect(true)
{
  diag("api.attributes",DIAG_DEBUG,"ATM_Attributes-- begin constructing\n");

  for (int i=0;i<num_ie;i++) {

    InfoElem* ie=msg->ie((InfoElem::ie_ix)i);

    if (ie) {
      _ie[i]=ie->copy();

#if __CRAP__
      // we need to redo the EPR flip as the message is rebuilt
      // from buffer after IPC.
      // PS: The EPR flip is only done at the IE level
      // not the encoded message.
      // currently it is done in 2 places: sscf (qsaal) and here
    
      if(i == InfoElem::ie_end_pt_ref_ix) 
	{
	  int epr = ((ie_end_pt_ref *) _ie)->get_epr_value();
	  epr ^= 0x8000;
	  ((ie_end_pt_ref *) _ie)->set_epr_value(epr);
	}
#endif
    }
    else _ie[i]=(InfoElem *)0; 

    set_inherited((InfoElem::ie_ix)i,false);
  }
  reset_modifiable_mask( true );

  _fsm=0;
  _id=cid;

#if __CRAP__
  set_crv( msg->get_crv() | (msg->get_crf() << 23) );
#endif

  _fsm = interface.make_call(_id);
  if (_fsm) {
    _fsm->change_state_no_callback(API_FSM::call_incoming_setup__waiting_for_local);
    _fsm->associate_attributes(this);
    _fsmtype=ATM_Attributes::Call_FSM;
    set_modifiable_bits( _fsm->get_state() );
  }
  else _fsmtype=ATM_Attributes::unknown_FSM;

  diag("api.attributes",DIAG_DEBUG,"ATM_Attributes-- done constructing\n");
}

//------------------------------------------------------------
// leaf made due to incoming leaf_setup

ATM_Attributes::ATM_Attributes(ATM_Interface& interface, ATM_Call& call, 
	       generic_q93b_msg* msg, fsm_type t, int lid)
  : _interface(&interface), _control(0), _protect(true)
{
  diag("api.attributes",DIAG_DEBUG,"ATM_Attributes-- begin constructing\n");
  for (int i=0;i<num_ie;i++) {
    InfoElem* ie=call._ie[(InfoElem::ie_ix)i];

    if (ie) _ie[i]=ie->copy();
    else _ie[i]=(InfoElem *)0; 

    set_inherited((InfoElem::ie_ix)i,false);
  }
  diag("api.attributes",DIAG_DEBUG,"ATM_Attributes-- done constructing\n");
  reset_modifiable_mask( true );

  _fsm=0;
  _id=lid;

#ifdef __CRAP__
  set_crv( msg->get_crv() | (msg->get_crf() << 23) );
#endif

  unprotect();
  ATM_Attributes::result res = Set_ep_ref(lid);
  protect();

  switch(t) {
  case ATM_Attributes::Leaf_FSM:
    _fsm = interface.make_leaf(call.Get_Call_ID(), lid ); break;
  default:
    break;
  }
  if (_fsm) {
    _fsm->change_state_no_callback(API_FSM::leaf_join_request__waiting_for_local);
    _fsm->associate_attributes(this);
    _fsmtype=t;
    set_modifiable_bits( _fsm->get_state() );
  }
  else _fsmtype=ATM_Attributes::unknown_FSM;
}

//------------------------------------------------------------
ATM_Attributes::~ATM_Attributes(void)
{
  diag("api.attributes",DIAG_DEBUG,"ATM_Attributes-- begin deleting\n");
  for (int i=0;i<num_ie;i++) 
    if ((_ie[i]!=NULL) && (!get_read_only((InfoElem::ie_ix)i))) {
      delete _ie[i];_ie[i]=(InfoElem *)0;
    }
  diag("api.attributes",DIAG_DEBUG,"ATM_Attributes-- done deleting\n");
}

//-----------------------------------------------
bool ATM_Attributes::get_read_only(InfoElem::ie_ix ix){
  return(_indirect[(int)ix] || (!_modifiable[(int)ix]));
}

//-----------------------------------------------
void ATM_Attributes::set_read_only(InfoElem::ie_ix ix, bool value){
  _modifiable[(int)ix]=value;
}

///-----------------------------------------------
void ATM_Attributes::set_inherited(InfoElem::ie_ix ix, bool value){
  _indirect[(int)ix]=value;
}

//-----------------------------------------------
/*
 * Prints all of the information elements.
 */
ostream& ATM_Attributes::print_ies(ostream& os)
{
  for (int i=0; i < num_ie; i++)
    if (_ie[i] != 0) 
      os << *(_ie[i]) << endl;

  return (os);
}

//-----------------------------------------------
int ATM_Attributes::get_id(void) const {
  return _id;
}

//-----------------------------------------------
void ATM_Attributes::set_crv(int crv) {
  _cref = crv;
  _flag = crv & 0x800000;
}

//-----------------------------------------------
void ATM_Attributes::fsm_failure(int code) {
  _fsm_failed = code;
}

//-----------------------------------------------
void ATM_Attributes::update_from_message(generic_q93b_msg* msg) {
  diag("api.attributes",DIAG_DEBUG,"Updating user API object based on incoming q93b_msg...\n");
  for (int i=0;i<num_ie;i++) {
    InfoElem* ie=msg->ie((InfoElem::ie_ix)i);
    if (ie) {
      if (_ie[i] != ie) { // guard against deletions in response 
	                  // to self-referential messages originated 
	                  // in make_* methods
	delete _ie[i];
	_ie[i] = msg->ie((InfoElem::ie_ix)i)->copy();
	diag("api.attributes",DIAG_DEBUG,"*** updating ie# %d based on q93b_message\n", i);
      }                   
    }
  }
}

//-----------------------------------------------
bool ATM_Attributes::push_FSM (sean_Visitor* sv) {
  if (_fsm) {
    bool ret;
    _fsm_failed = 0;
    _interface->inject_visitor(sv);
    if (_fsm_failed) ret= false;
    else ret= true;
    _fsm_failed = 0;
    return ret;
  }
  else
    sv->Suicide();
  return false;
}

//-----------------------------------------------
void ATM_Attributes::notify_FSM_that_queue_is_empty(void) {
  if (_fsm) _fsm->set_queue_to_empty();  
}


//-----------------------------------------------
void ATM_Attributes::kill_api_call_datastructure(int cid) {
  _interface->kill_call(cid);
}

//-----------------------------------------------
void ATM_Attributes::kill_api_service_datastructure(int sid) {
  _interface->kill_service(sid);
}

//-----------------------------------------------
void ATM_Attributes::kill_api_leaf_datastructure(int cid, int lid) {
  _interface->kill_leaf(cid, lid);
}

//-----------------------------------------------
void ATM_Attributes::reset_modifiable_mask(bool v) {
  for (int i=0;i<num_ie;i++)
    _modifiable[i]= v;
}

//-----------------------------------------------
void ATM_Attributes::set_modifiable_bits(API_FSM::api_fsm_state state) {
  reset_modifiable_mask(false);
  switch (state) {
  case API_FSM::call_preparing:       

    // legal IEs in setup 

    _modifiable[InfoElem::ie_traffic_desc_ix]            = true;
    _modifiable[InfoElem::ie_broadband_bearer_cap_ix]    = true;
    _modifiable[InfoElem::ie_called_party_num_ix]        = true;
    _modifiable[InfoElem::ie_qos_param_ix]               = true;
    _modifiable[InfoElem::ie_aal_param_ix]               = true;
    _modifiable[InfoElem::ie_bhli_ix]                    = true;
    _modifiable[InfoElem::ie_broadband_repeat_ind_ix]    = true;
    _modifiable[InfoElem::ie_blli_ix]                    = true;
    _modifiable[InfoElem::ie_called_party_subaddr_ix]    = true;
    _modifiable[InfoElem::ie_calling_party_num_ix]       = true;
    _modifiable[InfoElem::ie_calling_party_subaddr_ix]   = true;
    _modifiable[InfoElem::ie_conn_identifier_ix]         = true;
    _modifiable[InfoElem::ie_broadband_send_comp_ind_ix] = true;
    _modifiable[InfoElem::ie_transit_net_sel_ix]         = true;
    _modifiable[InfoElem::UNI40_leaf_call_id_ix]         = true;
    _modifiable[InfoElem::UNI40_leaf_params_ix]          = true;
    _modifiable[InfoElem::UNI40_leaf_sequence_num_ix]    = true;
    _modifiable[InfoElem::UNI40_conn_scope_ix]           = true;
    _modifiable[InfoElem::UNI40_alt_traff_desc_ix]       = true;
    _modifiable[InfoElem::UNI40_min_traff_desc_ix]       = true;
    _modifiable[InfoElem::UNI40_e2e_transit_delay_ix]    = true;
    _modifiable[InfoElem::UNI40_xqos_param_ix]           = true;
    _modifiable[InfoElem::ie_qos_param_ix]               = true;
    _modifiable[InfoElem::UNI40_abr_setup_ix]            = true;
    _modifiable[InfoElem::UNI40_abr_params_ix]           = true;
    _modifiable[InfoElem::UNI40_generic_id_ix]           = true;
    _modifiable[InfoElem::UNI40_notification_ind_ix]     = true;
 // _modifiable[InfoElem::ie_end_pt_ref_ix]              = true;
    break;

  case API_FSM::call_outgoing_setup__waiting_for_remote:

    // legal IEs in release_complete
    
    _modifiable[InfoElem::ie_cause_ix]         = true;
    _modifiable[InfoElem::UNI40_generic_id_ix] = true;
    break;

  case API_FSM::call_incoming_setup__waiting_for_local:

    // legal IEs in connect
    
    _modifiable[InfoElem::ie_aal_param_ix]            = true;
    _modifiable[InfoElem::ie_blli_ix]                 = true;
    _modifiable[InfoElem::ie_conn_identifier_ix]      = true;
    _modifiable[InfoElem::ie_traffic_desc_ix]         = true;
    _modifiable[InfoElem::UNI40_e2e_transit_delay_ix] = true;
    _modifiable[InfoElem::UNI40_xqos_param_ix]        = true;
    _modifiable[InfoElem::UNI40_abr_setup_ix]         = true;
    _modifiable[InfoElem::UNI40_abr_params_ix]        = true;
    _modifiable[InfoElem::UNI40_notification_ind_ix]  = true;
    _modifiable[InfoElem::UNI40_generic_id_ix]        = true;
//  _modifiable[InfoElem::ie_end_pt_ref_ix]           = true;
    break;
    

  case API_FSM::call_p2p_active:
  case API_FSM::call_p2mp_active:
  case API_FSM::call_p2mp_active__LIJ_present:

    // legal IEs in release
    
    _modifiable[InfoElem::ie_cause_ix]               = true;
    _modifiable[InfoElem::UNI40_notification_ind_ix] = true;
    _modifiable[InfoElem::UNI40_generic_id_ix]       = true;
    break;

  case API_FSM::call_release_initiated:
  case API_FSM::call_released:
    break;

    //------------------------------------------------------

  case API_FSM::service_preparing:

    // legal IEs in setup 

    _modifiable[InfoElem::ie_traffic_desc_ix]            = true;
    _modifiable[InfoElem::ie_broadband_bearer_cap_ix]    = true;
    _modifiable[InfoElem::ie_called_party_num_ix]        = true;
    _modifiable[InfoElem::ie_qos_param_ix]               = true;
    _modifiable[InfoElem::ie_aal_param_ix]               = true;
    _modifiable[InfoElem::ie_bhli_ix]                    = true;
    _modifiable[InfoElem::ie_broadband_repeat_ind_ix]    = true;
    _modifiable[InfoElem::ie_blli_ix]                    = true;
    _modifiable[InfoElem::ie_called_party_subaddr_ix]    = true;
    _modifiable[InfoElem::ie_calling_party_num_ix]       = true;
    _modifiable[InfoElem::ie_calling_party_subaddr_ix]   = true;
    _modifiable[InfoElem::ie_conn_identifier_ix]         = true;
    _modifiable[InfoElem::ie_broadband_send_comp_ind_ix] = true;
    _modifiable[InfoElem::ie_transit_net_sel_ix]         = true;
    _modifiable[InfoElem::UNI40_leaf_call_id_ix]         = true;
    _modifiable[InfoElem::UNI40_leaf_params_ix]          = true;
    _modifiable[InfoElem::UNI40_leaf_sequence_num_ix]    = true;
    _modifiable[InfoElem::UNI40_conn_scope_ix]           = true;
    _modifiable[InfoElem::UNI40_alt_traff_desc_ix]       = true;
    _modifiable[InfoElem::UNI40_min_traff_desc_ix]       = true;
    _modifiable[InfoElem::UNI40_e2e_transit_delay_ix]    = true;
    _modifiable[InfoElem::UNI40_xqos_param_ix]           = true;
    _modifiable[InfoElem::ie_qos_param_ix]               = true;
    _modifiable[InfoElem::UNI40_abr_setup_ix]            = true;
    _modifiable[InfoElem::UNI40_abr_params_ix]           = true;
    _modifiable[InfoElem::UNI40_generic_id_ix]           = true;
    _modifiable[InfoElem::UNI40_notification_ind_ix]     = true;
//  _modifiable[InfoElem::ie_end_pt_ref_ix]              = true;
    break;

  case API_FSM::service_register_requested:
  case API_FSM::service_deregister_requested:
  case API_FSM::service_active__no_call_present:
  case API_FSM::service_active__call_present:
  case API_FSM::service_released:
    break;

    //------------------------------------------------------

  case API_FSM::leaf_preparing:
  case API_FSM::leaf_join_request__waiting_for_local:

    // legal IEs in add_party
    
    _modifiable[InfoElem::ie_aal_param_ix]               = true;
    _modifiable[InfoElem::ie_bhli_ix]                    = true;
    _modifiable[InfoElem::ie_blli_ix]                    = true;
    _modifiable[InfoElem::ie_called_party_num_ix]        = true;
    _modifiable[InfoElem::ie_called_party_subaddr_ix]    = true;
    _modifiable[InfoElem::ie_calling_party_num_ix]       = true;
    _modifiable[InfoElem::ie_calling_party_subaddr_ix]   = true;
    _modifiable[InfoElem::ie_broadband_send_comp_ind_ix] = true;
    _modifiable[InfoElem::ie_transit_net_sel_ix]         = true;
    _modifiable[InfoElem::UNI40_notification_ind_ix]     = true;
    _modifiable[InfoElem::UNI40_e2e_transit_delay_ix]    = true;
    _modifiable[InfoElem::UNI40_leaf_sequence_num_ix]    = true;
    _modifiable[InfoElem::UNI40_generic_id_ix]           = true;
//  _modifiable[InfoElem::ie_end_pt_ref_ix]              = true;
    break;


  case API_FSM::leaf_addparty_requested__waiting_for_remote:
  case API_FSM::leaf_active:

    // Legal IEs in drop_party

    _modifiable[InfoElem::ie_cause_ix]               = true;
    _modifiable[InfoElem::UNI40_notification_ind_ix] = true;
    _modifiable[InfoElem::UNI40_generic_id_ix]       = true;
//  _modifiable[InfoElem::ie_end_pt_ref_ix]          = true;
    break;

  case API_FSM::leaf_released:
    break;

    //------------------------------------------------------

  case API_FSM::LIJ_preparing:

    // legal IEs in leaf_setup

    _modifiable[InfoElem::ie_transit_net_sel_ix]       = true;
    _modifiable[InfoElem::ie_calling_party_num_ix]     = true;
    _modifiable[InfoElem::ie_calling_party_subaddr_ix] = true;
    _modifiable[InfoElem::ie_called_party_num_ix]      = true;
    _modifiable[InfoElem::ie_called_party_subaddr_ix]  = true;
    _modifiable[InfoElem::UNI40_leaf_call_id_ix]       = true;
    _modifiable[InfoElem::UNI40_leaf_sequence_num_ix]  = true;
    break;

  case API_FSM::LIJ_requested__waiting_for_remote:
    
    // legal IEs in release_complete
    
    _modifiable[InfoElem::ie_cause_ix]         = true;
    _modifiable[InfoElem::UNI40_generic_id_ix] = true;
    break;

  default:
    break;
  }
}

//-----------------------------------------------
    
void ATM_Attributes::orphaned_by_controller(Controller* abandoner) {
  assert (abandoner==_control);
  _control = 0;
}


//-----------------------------------------------

void ATM_Attributes::disable_controller_callbacks(void) {
  switch(_fsmtype) {
  case ATM_Attributes::Call_FSM:
    if (_control) 
      _control->abandon_managed_call(this);
    break;
    
  case ATM_Attributes::Service_FSM:
    if (_control) 
      _control->abandon_managed_service(this);
    break;

  default:  
    break;
  }
  _control = 0;
}

//-----------------------------------------------

void ATM_Attributes::force_leaf_to_active_state(void) {
  assert(_fsm);
  switch(_fsmtype) {
  case ATM_Attributes::Leaf_FSM: 
    _fsm->change_state_no_callback(API_FSM::leaf_active);
    break;

  default:
    break;
  }
}

//-----------------------------------------------

void ATM_Attributes::force_leaf_to_dead_state(void) {
  assert(_fsm);
  switch(_fsmtype) {
  case ATM_Attributes::Leaf_FSM: 
    _fsm->change_state_no_callback(API_FSM::leaf_released);
    break;

  default:
    break;
  }
}


//-----------------------------------------------

void ATM_Attributes::force_call_to_dead_state(void) {
  assert(_fsm);
  switch(_fsmtype) {
  case ATM_Attributes::Call_FSM: 
    _fsm->change_state_no_callback(API_FSM::call_released);
    break;

  default:
    break;
  }
}


// redefined in Outgoing_ATM_Call
//-----------------------------------------------
sean_io* ATM_Attributes::get_datapath(void) {
  return 0;
}



//************************ U N I  version 4.0 ************************/
//************************ specific features  ************************/



// ---------------------------
// UNI-4.0 specific attributes
/* exclusive VPI, VCI */


ATM_Attributes::result ATM_Attributes::Set_exclusive_vpi_vci(long vpi, 
							     long vci)
{
  if ( ! Authorize(InfoElem::ie_conn_identifier_ix) )
    return ATM_Attributes::failure;

  int associated = 1;
  int exclusivity = 0;
  ie_conn_id *thing = new ie_conn_id(vpi,vci,associated,exclusivity);
  ATM_Attributes::result answer = set_attr( InfoElem::ie_conn_identifier_ix, 
					    thing);

  if (answer==ATM_Attributes::failure) {
    delete thing;
    thing=0L;
  }
  return answer;
}

ATM_Attributes::result ATM_Attributes::Set_exclusive_vpi_any_vci(long vpi)
{
  if ( ! Authorize(InfoElem::ie_conn_identifier_ix) )
    return ATM_Attributes::failure;

  int associated = 1;
  int exclusivity = 1;
  int vci_ignored = 0;
  ie_conn_id *thing = new ie_conn_id(vpi,vci_ignored,associated,exclusivity);
  ATM_Attributes::result answer = set_attr( InfoElem::ie_conn_identifier_ix, 
					    thing);

  if (answer==ATM_Attributes::failure) {
    delete thing;
    thing=0L;
  }
  return answer;
}

ATM_Attributes::result ATM_Attributes::Set_exclusive_vpi_no_vci(long vpi)
{
  if ( ! Authorize(InfoElem::ie_conn_identifier_ix) )
    return ATM_Attributes::failure;

  int associated = 1;
  int exclusivity = 4;
  int vci_ignored = 0;
  ie_conn_id *thing = new ie_conn_id(vpi,vci_ignored,associated,exclusivity);
  ATM_Attributes::result answer = set_attr( InfoElem::ie_conn_identifier_ix, 
					    thing);

  if (answer==ATM_Attributes::failure) {
    delete thing;
    thing=0L;
  }
  return answer;
}


// ---------------------------
// UNI-4.0 specific attributes
/* Available Bit Rate */


ATM_Attributes::result ATM_Attributes::Set_ABR(int ficr, int bicr, int ftbe, int btbe, int crtt, 
			       int frif, int brif, int frdf, int brdf)
{
  if ( ! Authorize(InfoElem::UNI40_abr_setup_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result ret = ATM_Attributes::success;
  UNI40_abr_setup *ie;
  if ((ie = (UNI40_abr_setup*) 
       get_attr(InfoElem::UNI40_abr_setup_ix)) != NULL) { 
    set_attr((InfoElem::UNI40_abr_setup_ix),0);              // will delete old IE
  }

  ie=new UNI40_abr_setup(ficr, bicr, ftbe, btbe, crtt, 
			 frif, brif, frdf, brdf, 0);
  set_attr((InfoElem::UNI40_abr_setup_ix),ie); 
  return ret;
}

ATM_Attributes::result ATM_Attributes::Get_ABR(int& ficr, int& bicr, int& ftbe, int& btbe, int& crtt, 
			       int& frif, int& brif, int& frdf, int& brdf)
{
  ATM_Attributes::result ret = ATM_Attributes::failure;
  UNI40_abr_setup *ie;
  if ((ie = (UNI40_abr_setup*) 
       get_attr(InfoElem::UNI40_abr_setup_ix)) != NULL) {

    ficr =ie-> getForwardICR();
    bicr = ie->getBackwardICR();
    ftbe = ie->getForwardTBE();
    btbe = ie->getBackwardTBE();
    crtt = ie->getCumulativeRTT();
    frif = ie->getForwardRIF();
    brif = ie->getBackwardRIF();
    frdf = ie->getForwardRDF();
    brdf = ie->getBackwardRDF();

    return ATM_Attributes::success;
  }
  return ret;
}

// ---------------------------
// UNI-4.0 specific attributes
/* End-to-End delay requirements */


ATM_Attributes::result ATM_Attributes::Set_e2e_transit_delay(int cumulative, int maximum)
{
  if ( ! Authorize(InfoElem::UNI40_e2e_transit_delay_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result ret = ATM_Attributes::failure;
  return ret;
}

ATM_Attributes::result ATM_Attributes::Get_e2e_transit_delay(int& cumulative, int& maximum) const
{
  ATM_Attributes::result ret = ATM_Attributes::failure;
  return ret;
}

// ---------------------------
// UNI-4.0 specific attributes
/* Best Effort with Max Cell Rate */


ATM_Attributes::result ATM_Attributes::Set_td_BE_with_MCR(int fpcr_01, 
					  int bpcr_01,
					  int fmcr_01,
					  int bmcr_01)
{
  if ( ! Authorize(InfoElem::ie_traffic_desc_ix) )
    return ATM_Attributes::failure;

  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::ie_traffic_desc_ix)) == NULL) { 
    ie=new IE_TRAFFIC_DESC;
    set_attr((InfoElem::ie_traffic_desc_ix),ie); }

  ie->set_BE(fpcr_01,bpcr_01,fmcr_01,bmcr_01);
  return(ATM_Attributes::success);
}


ATM_Attributes::result ATM_Attributes::Get_td_BE_with_MCR(int& fpcr_01, 
					  int& bpcr_01,
					  int& fmcr_01,
					  int& bmcr_01) const
{
  if (Get_td_type(IE_TRAFFIC_DESC::fw)!=IE_TRAFFIC_DESC::TPC7) return ATM_Attributes::failure;
  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::ie_traffic_desc_ix)) == NULL) return ATM_Attributes::failure;
  fpcr_01 = ie->get_FPCR_01();
  bpcr_01 = ie->get_BPCR_01();
  fmcr_01 = ie->get_FMCR_01();
  bmcr_01 = ie->get_BMCR_01();
  return ATM_Attributes::success;
}


// ---------------------------
// UNI-4.0 specific attributes
/* QoS negotiation: Alternate traffic descriptor */


ATM_Attributes::result ATM_Attributes::Set_alt_td_1(UNI40_traffic_desc::dir d, int pcr_0, int pcr_01)
{
  if ( ! Authorize(InfoElem::UNI40_alt_traff_desc_ix) )
    return ATM_Attributes::failure;

  UNI40_alt_traffic_desc *ie;
  if ((ie = (UNI40_alt_traffic_desc*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) == NULL) {
    ie=new UNI40_alt_traffic_desc;
    set_attr((InfoElem::UNI40_alt_traff_desc_ix),ie); }
    
  ie->set_TP1((UNI40_traffic_desc::dir)d, pcr_0, pcr_01);
  return(ATM_Attributes::success);
}    

ATM_Attributes::result ATM_Attributes::Set_alt_td_2(UNI40_traffic_desc::dir d, int pcr_0, int pcr_01)
{
  if ( ! Authorize(InfoElem::UNI40_alt_traff_desc_ix) )
    return ATM_Attributes::failure;

  UNI40_alt_traffic_desc *ie;
  if ((ie = (UNI40_alt_traffic_desc*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) == NULL) {
    ie=new UNI40_alt_traffic_desc;
    set_attr((InfoElem::UNI40_alt_traff_desc_ix),ie); }

  ie->set_TP2((UNI40_traffic_desc::dir)d, pcr_0, pcr_01);
  return(ATM_Attributes::success);
}    

ATM_Attributes::result ATM_Attributes::Set_alt_td_3(UNI40_traffic_desc::dir d, int pcr_01, int scr_0,  int mbs_0)
{
  if ( ! Authorize(InfoElem::UNI40_alt_traff_desc_ix) )
    return ATM_Attributes::failure;

  UNI40_alt_traffic_desc *ie;
  if ((ie = (UNI40_alt_traffic_desc*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) == NULL) { 
    ie=new UNI40_alt_traffic_desc;
    set_attr((InfoElem::UNI40_alt_traff_desc_ix),ie); }

  ie->set_TP3((UNI40_traffic_desc::dir)d,pcr_01,scr_0,mbs_0);
  return(ATM_Attributes::success);
}    

ATM_Attributes::result ATM_Attributes::Set_alt_td_4(UNI40_traffic_desc::dir d, int pcr_01, int scr_0,  int  mbs_0)
{
  if ( ! Authorize(InfoElem::UNI40_alt_traff_desc_ix) )
    return ATM_Attributes::failure;

  UNI40_alt_traffic_desc *ie;
  if ((ie = (UNI40_alt_traffic_desc*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) == NULL) { 
    ie=new UNI40_alt_traffic_desc;
    set_attr((InfoElem::UNI40_alt_traff_desc_ix),ie); }

  ie->set_TP4((UNI40_traffic_desc::dir)d,pcr_01,scr_0,mbs_0);
  return(ATM_Attributes::success);
}    

ATM_Attributes::result ATM_Attributes::Set_alt_td_5(UNI40_traffic_desc::dir d, int pcr_01)
{
  if ( ! Authorize(InfoElem::UNI40_alt_traff_desc_ix) )
    return ATM_Attributes::failure;

  UNI40_alt_traffic_desc *ie;
  if ((ie = (UNI40_alt_traffic_desc*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) == NULL) { 
    ie=new UNI40_alt_traffic_desc;
    set_attr((InfoElem::UNI40_alt_traff_desc_ix),ie); }

  ie->set_TP5((UNI40_traffic_desc::dir)d,pcr_01);
  return(ATM_Attributes::success);
}    

ATM_Attributes::result ATM_Attributes::Set_alt_td_6(UNI40_traffic_desc::dir d, int pcr_0, int scr_01, int mbs_01)
{
  if ( ! Authorize(InfoElem::UNI40_alt_traff_desc_ix) )
    return ATM_Attributes::failure;

  UNI40_alt_traffic_desc *ie;
  if ((ie = (UNI40_alt_traffic_desc*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) == NULL) { 
    ie=new UNI40_alt_traffic_desc;
    set_attr((InfoElem::UNI40_alt_traff_desc_ix),ie); }

  ie->set_TP6((UNI40_traffic_desc::dir)d,pcr_0,scr_01,mbs_01);
  return(ATM_Attributes::success);
}    

ATM_Attributes::result ATM_Attributes::Set_alt_td_BE(int fpcr_01, int bpcr_01)
{
  if ( ! Authorize(InfoElem::UNI40_alt_traff_desc_ix) )
    return ATM_Attributes::failure;

  UNI40_alt_traffic_desc *ie;
  if ((ie = (UNI40_alt_traffic_desc*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) == NULL) { 
    ie=new UNI40_alt_traffic_desc;
    set_attr((InfoElem::UNI40_alt_traff_desc_ix),ie); }

  ie->set_BE(fpcr_01,bpcr_01);
  return(ATM_Attributes::success);
}

ATM_Attributes::result ATM_Attributes::Set_alt_td_BE_with_MCR(int fpcr_01, int bpcr_01, int fmcr_01, int bmcr_01)
{
  if ( ! Authorize(InfoElem::UNI40_alt_traff_desc_ix) )
    return ATM_Attributes::failure;

  UNI40_alt_traffic_desc *ie;
  if ((ie = (UNI40_alt_traffic_desc*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) == NULL) { 
    ie=new UNI40_alt_traffic_desc;
    set_attr((InfoElem::UNI40_alt_traff_desc_ix),ie); }

  ie->set_BE(fpcr_01,bpcr_01,fmcr_01,bmcr_01);
  return(ATM_Attributes::success);
}


ATM_Attributes::result ATM_Attributes::Get_alt_td_1(UNI40_traffic_desc::dir d, int& pcr_0, int& pcr_01)
{
  if (Get_alt_td_type(d)!=UNI40_alt_traffic_desc::TPC1) return ATM_Attributes::failure;
  UNI40_alt_traffic_desc *ie;
  if ((ie = (UNI40_alt_traffic_desc*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) == NULL) return ATM_Attributes::failure;
  switch(d){
  case UNI40_alt_traffic_desc::fw:
    pcr_0 = ie->get_FPCR_0();
    pcr_01 = ie->get_FPCR_01();
    break;
  case UNI40_alt_traffic_desc::bw:
    pcr_0 = ie->get_BPCR_0();
    pcr_01 = ie->get_BPCR_01();
    break;
  }
  return ATM_Attributes::success; 
}    

ATM_Attributes::result ATM_Attributes::Get_alt_td_2(UNI40_traffic_desc::dir d, int& pcr_0, int& pcr_01)
{
  if (Get_alt_td_type(d)!=UNI40_alt_traffic_desc::TPC2) return ATM_Attributes::failure;
  UNI40_alt_traffic_desc *ie;
  if ((ie = (UNI40_alt_traffic_desc*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) == NULL) return ATM_Attributes::failure;
  switch(d){
  case UNI40_alt_traffic_desc::fw:
    pcr_0 = ie->get_FPCR_0();
    pcr_01 = ie->get_FPCR_01();
    break;
  case UNI40_alt_traffic_desc::bw:
    pcr_0 = ie->get_BPCR_0();
    pcr_01 = ie->get_BPCR_01();
    break;
  }
  return ATM_Attributes::success; 
}    

ATM_Attributes::result ATM_Attributes::Get_alt_td_3(UNI40_traffic_desc::dir d, int& pcr_01, int& scr_0,  int& mbs_0)
{
  if (Get_alt_td_type(d)!=UNI40_alt_traffic_desc::TPC3) return ATM_Attributes::failure;
  UNI40_alt_traffic_desc *ie;
  if ((ie = (UNI40_alt_traffic_desc*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) == NULL) return ATM_Attributes::failure;
  switch(d){
  case UNI40_alt_traffic_desc::fw:
    pcr_01 = ie->get_FPCR_01();
    scr_0 = ie->get_FSCR_0();
    mbs_0 = ie->get_FMBS_0();
    break;
  case UNI40_alt_traffic_desc::bw:
    pcr_01 = ie->get_BPCR_01();
    scr_0 = ie->get_BSCR_0();
    mbs_0 = ie->get_BMBS_0();
    break;
  }
  return ATM_Attributes::success; 
}    

ATM_Attributes::result ATM_Attributes::Get_alt_td_4(UNI40_traffic_desc::dir d, int& pcr_01, int& scr_0,  int&  mbs_0)
{
  if (Get_alt_td_type(d)!=UNI40_alt_traffic_desc::TPC4) return ATM_Attributes::failure;
  UNI40_alt_traffic_desc *ie;
  if ((ie = (UNI40_alt_traffic_desc*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) == NULL) return ATM_Attributes::failure;
  switch(d){
  case UNI40_alt_traffic_desc::fw:
    pcr_01 = ie->get_FPCR_01();
    scr_0  = ie->get_FSCR_0();
    mbs_0  = ie->get_FMBS_0();
    break;
  case UNI40_alt_traffic_desc::bw:
    pcr_01 = ie->get_BPCR_01();
    scr_0  = ie->get_BSCR_0();
    mbs_0  = ie->get_BMBS_0();
    break;
  }
  return ATM_Attributes::success; 
}    

ATM_Attributes::result ATM_Attributes::Get_alt_td_5(UNI40_traffic_desc::dir d, int& pcr_01)
{
  if (Get_alt_td_type(d)!=UNI40_alt_traffic_desc::TPC5) return ATM_Attributes::failure;
  UNI40_alt_traffic_desc *ie;
  if ((ie = (UNI40_alt_traffic_desc*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) == NULL) return ATM_Attributes::failure;
  switch(d){
  case UNI40_alt_traffic_desc::fw:
    pcr_01 = ie->get_FPCR_01();
    break;
  case UNI40_alt_traffic_desc::bw:
    pcr_01 = ie->get_BPCR_01();
    break;
  }
  return ATM_Attributes::success; 
}    

ATM_Attributes::result ATM_Attributes::Get_alt_td_6(UNI40_traffic_desc::dir d, int& pcr_0, int& scr_01, int& mbs_01)
{
  if (Get_alt_td_type(d)!=UNI40_alt_traffic_desc::TPC6) return ATM_Attributes::failure;
  UNI40_alt_traffic_desc *ie;
  if ((ie = (UNI40_alt_traffic_desc*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) == NULL) return ATM_Attributes::failure;
  switch(d){
  case UNI40_alt_traffic_desc::fw:
    pcr_0 = ie->get_FPCR_0();
    scr_01 = ie->get_FSCR_01();
    mbs_01 = ie->get_FMBS_01();
    break;
  case UNI40_alt_traffic_desc::bw:
    pcr_0 = ie->get_BPCR_0();
    scr_01 = ie->get_BSCR_01();
    mbs_01 = ie->get_BMBS_01();
    break;
  }
  return ATM_Attributes::success;
}    

ATM_Attributes::result ATM_Attributes::Get_alt_td_BE(int& fpcr_01, int& bpcr_01)
{
  if (Get_alt_td_type(UNI40_alt_traffic_desc::fw)!=UNI40_alt_traffic_desc::TPC7) return ATM_Attributes::failure;
  UNI40_alt_traffic_desc *ie;
  if ((ie = (UNI40_alt_traffic_desc*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) == NULL) return ATM_Attributes::failure;
  fpcr_01 = ie->get_FPCR_01();
  bpcr_01 = ie->get_BPCR_01();
  return ATM_Attributes::success;
}

ATM_Attributes::result ATM_Attributes::Get_alt_td_BE_with_MCR(int& fpcr_01, int& bpcr_01, int& fmcr_01, int& bmcr_01)
{
  if (Get_alt_td_type(UNI40_alt_traffic_desc::fw)!=UNI40_alt_traffic_desc::TPC7) return ATM_Attributes::failure;
  UNI40_alt_traffic_desc *ie;
  if ((ie = (UNI40_alt_traffic_desc*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) == NULL) return ATM_Attributes::failure;
  fpcr_01 = ie->get_FPCR_01();
  bpcr_01 = ie->get_BPCR_01();
  fmcr_01 = ie->get_FMCR_01();
  bmcr_01 = ie->get_BMCR_01();
  return ATM_Attributes::success;
}


IE_TRAFFIC_DESC::VALID_TPC ATM_Attributes::Get_alt_td_type(IE_TRAFFIC_DESC::dir d) const {
  IE_TRAFFIC_DESC *ie;
  if ((ie = (IE_TRAFFIC_DESC*) 
       get_attr(InfoElem::UNI40_alt_traff_desc_ix)) != NULL) 
    switch(d){
    case IE_TRAFFIC_DESC::fw: return ((IE_TRAFFIC_DESC::VALID_TPC)(ie->ftpc())); break;
    case IE_TRAFFIC_DESC::bw: return ((IE_TRAFFIC_DESC::VALID_TPC)(ie->btpc())); break;
    }
  return (IE_TRAFFIC_DESC::VALID_TPC)IE_TRAFFIC_DESC::TPC_UNSPECIFIED;
}



// ---------------------------
// UNI-4.0 specific attributes
/* QoS negotiation: Minumum traffic descriptor */


ATM_Attributes::result ATM_Attributes::Set_minimum_FPCR_0(int fpcr0)
{
  if ( ! Authorize(InfoElem::UNI40_min_traff_desc_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result ret = ATM_Attributes::success;
  UNI40_min_traffic_desc *ie;
  if ((ie = (UNI40_min_traffic_desc*) 
       get_attr(InfoElem::UNI40_min_traff_desc_ix)) == NULL) { 
    ie=new UNI40_min_traffic_desc;
    set_attr((InfoElem::UNI40_min_traff_desc_ix),ie); 
  }

  ie->set_FPCR_0(fpcr0);
  return ret;
}

ATM_Attributes::result ATM_Attributes::Set_minimum_BPCR_0(int bpcr0)
{
  if ( ! Authorize(InfoElem::UNI40_min_traff_desc_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result ret = ATM_Attributes::success;
  UNI40_min_traffic_desc *ie;
  if ((ie = (UNI40_min_traffic_desc*) 
       get_attr(InfoElem::UNI40_min_traff_desc_ix)) == NULL) { 
    ie=new UNI40_min_traffic_desc;
    set_attr((InfoElem::UNI40_min_traff_desc_ix),ie); 
  }

  ie->set_BPCR_0(bpcr0);
  return ret;
}

ATM_Attributes::result ATM_Attributes::Set_minimum_FPCR_01(int fpcr01)
{
  if ( ! Authorize(InfoElem::UNI40_min_traff_desc_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result ret = ATM_Attributes::success;
  UNI40_min_traffic_desc *ie;
  if ((ie = (UNI40_min_traffic_desc*) 
       get_attr(InfoElem::UNI40_min_traff_desc_ix)) == NULL) { 
    ie=new UNI40_min_traffic_desc;
    set_attr((InfoElem::UNI40_min_traff_desc_ix),ie); 
  }

  ie->set_FPCR_01(fpcr01);
  return ret;
}

ATM_Attributes::result ATM_Attributes::Set_minimum_BPCR_01(int bpcr01)
{
  if ( ! Authorize(InfoElem::UNI40_min_traff_desc_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result ret = ATM_Attributes::success;
  UNI40_min_traffic_desc *ie;
  if ((ie = (UNI40_min_traffic_desc*) 
       get_attr(InfoElem::UNI40_min_traff_desc_ix)) == NULL) { 
    ie=new UNI40_min_traffic_desc;
    set_attr((InfoElem::UNI40_min_traff_desc_ix),ie); 
  }

  ie->set_BPCR_01(bpcr01);
  return ret;
}

ATM_Attributes::result ATM_Attributes::Set_minimum_ForwardABR(int fabr)
{
  if ( ! Authorize(InfoElem::UNI40_min_traff_desc_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result ret = ATM_Attributes::success;
  UNI40_min_traffic_desc *ie;
  if ((ie = (UNI40_min_traffic_desc*) 
       get_attr(InfoElem::UNI40_min_traff_desc_ix)) == NULL) { 
    ie=new UNI40_min_traffic_desc;
    set_attr((InfoElem::UNI40_min_traff_desc_ix),ie); 
  }

  ie->set_ForwardABR(fabr);
  return ret;
}

ATM_Attributes::result ATM_Attributes::Set_minimum_BackwardABR(int babr)
{
  if ( ! Authorize(InfoElem::UNI40_min_traff_desc_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result ret = ATM_Attributes::success;
  UNI40_min_traffic_desc *ie;
  if ((ie = (UNI40_min_traffic_desc*) 
       get_attr(InfoElem::UNI40_min_traff_desc_ix)) == NULL) { 
    ie=new UNI40_min_traffic_desc;
    set_attr((InfoElem::UNI40_min_traff_desc_ix),ie); 
  }

  ie->set_ForwardABR(babr);
  return ret;
}



ATM_Attributes::result ATM_Attributes::Get_minimum_FPCR_0(int& fpcr0) const
{
  ATM_Attributes::result ret = ATM_Attributes::failure;
  UNI40_min_traffic_desc *ie;
  if ((ie = (UNI40_min_traffic_desc*) 
       get_attr(InfoElem::UNI40_min_traff_desc_ix)) != NULL) {
    fpcr0 = ie->get_FPCR_0();
    return ATM_Attributes::success;
  }
  return ret;
}

ATM_Attributes::result ATM_Attributes::Get_minimum_BPCR_0(int& bpcr0) const
{
  ATM_Attributes::result ret = ATM_Attributes::failure;
  UNI40_min_traffic_desc *ie;
  if ((ie = (UNI40_min_traffic_desc*) 
       get_attr(InfoElem::UNI40_min_traff_desc_ix)) != NULL) {
    bpcr0 = ie->get_BPCR_0();
    return ATM_Attributes::success;
  }
  return ret;
}

ATM_Attributes::result ATM_Attributes::Get_minimum_FPCR_01(int& fpcr01) const
{
  ATM_Attributes::result ret = ATM_Attributes::failure;
  UNI40_min_traffic_desc *ie;
  if ((ie = (UNI40_min_traffic_desc*) 
       get_attr(InfoElem::UNI40_min_traff_desc_ix)) != NULL) {
    fpcr01 = ie->get_FPCR_01();
    return ATM_Attributes::success;
  }
  return ret;
}

ATM_Attributes::result ATM_Attributes::Get_minimum_BPCR_01(int& bpcr01) const
{
  ATM_Attributes::result ret = ATM_Attributes::failure;
  UNI40_min_traffic_desc *ie;
  if ((ie = (UNI40_min_traffic_desc*) 
       get_attr(InfoElem::UNI40_min_traff_desc_ix)) != NULL) {
    bpcr01 = ie->get_BPCR_01();
    ret = ATM_Attributes::success;
  }
  return ret;
}

ATM_Attributes::result ATM_Attributes::Get_minimum_ForwardABR(int& fabr) const
{
  ATM_Attributes::result ret = ATM_Attributes::failure;
  UNI40_min_traffic_desc *ie;
  if ((ie = (UNI40_min_traffic_desc*) 
       get_attr(InfoElem::UNI40_min_traff_desc_ix)) != NULL) {
    fabr = ie->get_ForwardABR();
    ret = ATM_Attributes::success;
  }
  return ret;
}

ATM_Attributes::result ATM_Attributes::Get_minimum_BackwardABR(int& babr) const
{
  ATM_Attributes::result ret = ATM_Attributes::failure;
  UNI40_min_traffic_desc *ie;
  if ((ie = (UNI40_min_traffic_desc*) 
       get_attr(InfoElem::UNI40_min_traff_desc_ix)) != NULL) {
    babr = ie->get_BackwardABR();
    ret = ATM_Attributes::success;
  }
  return ret;
}



// ---------------------------
// UNI-4.0 specific attributes
/* Generic identifier supporting
   - Digital Storage Media Command & Control 
   - Recommendation H.245 */


ATM_Attributes::result ATM_Attributes::Set_DSMCC_generic_session_id(u_char* buf, int len)
{
  if ( ! Authorize(InfoElem::UNI40_generic_id_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result ret = ATM_Attributes::success;
  UNI40_generic_id *ie;
  if ((ie = (UNI40_generic_id*) 
       get_attr(InfoElem::UNI40_generic_id_ix)) != NULL) { 
    set_attr((InfoElem::UNI40_generic_id_ix),0); // will delete existing one
  }
  ie=new UNI40_generic_id(UNI40_generic_id::DSMCC,
			  UNI40_generic_id::Session, 
			  buf, len);
  set_attr((InfoElem::UNI40_generic_id_ix),ie); 
  return ret;
}

ATM_Attributes::result ATM_Attributes::Set_H245_generic_session_id(u_char* buf, int len)
{
  if ( ! Authorize(InfoElem::UNI40_generic_id_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result ret = ATM_Attributes::success;
  UNI40_generic_id *ie;
  if ((ie = (UNI40_generic_id*) 
       get_attr(InfoElem::UNI40_generic_id_ix)) != NULL) { 
    set_attr((InfoElem::UNI40_generic_id_ix),0); // will delete existing one
  }
  ie=new UNI40_generic_id(UNI40_generic_id::H245,
			  UNI40_generic_id::Session, 
			  buf, len);
  set_attr((InfoElem::UNI40_generic_id_ix),ie); 
  return ret;
}

ATM_Attributes::result ATM_Attributes::Set_DSMCC_generic_resource_id(u_char* buf, int len)
{
  if ( ! Authorize(InfoElem::UNI40_generic_id_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result ret = ATM_Attributes::success;
  UNI40_generic_id *ie;
  if ((ie = (UNI40_generic_id*) 
       get_attr(InfoElem::UNI40_generic_id_ix)) != NULL) { 
    set_attr((InfoElem::UNI40_generic_id_ix),0); // will delete existing one
  }
  ie=new UNI40_generic_id(UNI40_generic_id::DSMCC,
			  UNI40_generic_id::Resource, 
			  buf, len);
  set_attr((InfoElem::UNI40_generic_id_ix),ie); 
  return ret;
}

ATM_Attributes::result ATM_Attributes::Set_H245_generic_resource_id(u_char* buf, int len)
{
  if ( ! Authorize(InfoElem::UNI40_generic_id_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result ret = ATM_Attributes::success;
  UNI40_generic_id *ie;
  if ((ie = (UNI40_generic_id*) 
       get_attr(InfoElem::UNI40_generic_id_ix)) != NULL) { 
    set_attr((InfoElem::UNI40_generic_id_ix),0); // will delete existing one
  }
  ie=new UNI40_generic_id(UNI40_generic_id::H245,
			  UNI40_generic_id::Resource, 
			  buf, len);
  set_attr((InfoElem::UNI40_generic_id_ix),ie); 
  return ret;
}

ATM_Attributes::result ATM_Attributes::Get_generic_id(unsigned char*& buffer, int& buflen) const
{
  ATM_Attributes::result ret = ATM_Attributes::failure;
  UNI40_generic_id *ie;
  if ((ie = (UNI40_generic_id*) 
       get_attr(InfoElem::UNI40_generic_id_ix)) != NULL) {
    buflen = ie->Length();
    buffer = new u_char[buflen];
    memcpy(buffer, ie->getValue(), buflen);
    return ATM_Attributes::success;
  }
  return ret;
}

bool ATM_Attributes::Is_DSMCC_generic_id_present(void) const
{
  bool ret = false;
  UNI40_generic_id *ie;
  if ((ie = (UNI40_generic_id*) 
       get_attr(InfoElem::UNI40_generic_id_ix)) != NULL) {
    ret = (UNI40_generic_id::DSMCC == ie->getRelated());
  }
  return ret;
}

bool ATM_Attributes::Is_H245_generic_id_present(void) const
{
  bool ret = false;
  UNI40_generic_id *ie;
  if ((ie = (UNI40_generic_id*) 
       get_attr(InfoElem::UNI40_generic_id_ix)) != NULL) {
    ret = (UNI40_generic_id::H245 == ie->getRelated());
  }
  return ret;
}


// ---------------------------
// UNI-4.0 specific attributes
/* Leaf Initiated Join sequence number */

ATM_Attributes::result ATM_Attributes::Set_LIJ_sequence_number(int sequence) {
  if ( ! Authorize(InfoElem::UNI40_leaf_sequence_num_ix) )
    return ATM_Attributes::failure;


  ATM_Attributes::result ret = ATM_Attributes::success;
  UNI40_leaf_sequence_num *ie;
  if ((ie = (UNI40_leaf_sequence_num*) 
       get_attr(InfoElem::UNI40_leaf_sequence_num_ix)) == NULL) { 
    ie = new UNI40_leaf_sequence_num(sequence);
    set_attr((InfoElem::UNI40_leaf_sequence_num_ix),ie); 
  }
  ie->setSeqNum(sequence);
  return ret;
}

ATM_Attributes::result ATM_Attributes::Get_LIJ_sequence_number(int& sequence) const
{
  ATM_Attributes::result ret = ATM_Attributes::failure;
  UNI40_leaf_sequence_num *ie;
  if ((ie = (UNI40_leaf_sequence_num*) 
       get_attr(InfoElem::UNI40_leaf_sequence_num_ix)) != NULL) {
    sequence = ie->getSeqNum();
    return ATM_Attributes::success;
  }
  return ret;
}



// ---------------------------
// UNI-4.0 specific attributes
/* Leaf Initiated Join Parameters */


ATM_Attributes::result ATM_Attributes::Set_LIJ_net_decides(void)
{
  if ( ! Authorize(InfoElem::UNI40_leaf_params_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result ret = ATM_Attributes::success;
  UNI40_lij_params* ie = new UNI40_lij_params(0);
  set_attr((InfoElem::UNI40_leaf_params_ix),ie); 
  return ret;
}

ATM_Attributes::result ATM_Attributes::Set_LIJ_root_decides(void)
{
  if ( ! Authorize(InfoElem::UNI40_leaf_params_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result ret = ATM_Attributes::success;
  set_attr((InfoElem::UNI40_leaf_params_ix),0);
  return ret;
}


// ---------------------------
// UNI-4.0 specific attributes
/* P2MP Call identifier */


ATM_Attributes::result ATM_Attributes::Set_LIJ_call_id(int value)
{
  if ( ! Authorize(InfoElem::UNI40_leaf_call_id_ix) )
    return ATM_Attributes::failure;

  ATM_Attributes::result ret = ATM_Attributes::success;
  UNI40_lij_call_id *ie;
  if ((ie = (UNI40_lij_call_id*) 
       get_attr(InfoElem::UNI40_leaf_call_id_ix)) != NULL) { 
    set_attr((InfoElem::UNI40_leaf_call_id_ix),0); // deletes old
  }

  ie=new UNI40_lij_call_id( 0 , value ); // 0 means the root assigned
                                         // the call id originally
  set_attr((InfoElem::UNI40_leaf_call_id_ix),ie); 
  return ret;
}

ATM_Attributes::result ATM_Attributes::Get_LIJ_call_id(int& value)
{
  ATM_Attributes::result ret = ATM_Attributes::failure;
  UNI40_lij_call_id *ie;
  if ((ie = (UNI40_lij_call_id*) 
       get_attr(InfoElem::UNI40_leaf_call_id_ix)) != NULL) {
    value = ie->GetValue();
    return ATM_Attributes::success;
  }
  return ret;
}


void ATM_Attributes::initialize_leaf_initiated_call(void) {
  if (_fsm) {
    _fsm->change_state_no_callback(API_FSM::LIJ_preparing);
    _fsm->associate_attributes(this);
    set_modifiable_bits( _fsm->get_state() );
  }
}


void ATM_Attributes::set_dummy_LIJ_sequence_number(void) {
  Set_LIJ_sequence_number( -1 );
}

void ATM_Attributes::swap_called_and_calling_party(void) {
  Addr* src;
  Addr* dest;
  ie_calling_party_num::presentation_indicator pres;
  ie_calling_party_num::screening_indicator scn;

  assert(Get_called_party_num(dest)           == ATM_Attributes::success);
  assert(Get_calling_party_num(src,pres,scn)  == ATM_Attributes::success);
  unprotect();
  assert(Set_called_party_num(src)            == ATM_Attributes::success);
  assert(Set_calling_party_num(dest,pres,scn) == ATM_Attributes::success);
  protect();
}
