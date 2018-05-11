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

/* -*- C++ -*-
 * File: ATM_Attributes.h
 * @author: bilal
 * @version: $Id: ATM_Attributes.h,v 1.10 1998/09/28 12:31:16 bilal Exp $
 */
#ifndef __ATM_Attributes_H__
#define __ATM_Attributes_H__
#ifndef LINT
static char const _ATM_Attributes_h_rcsid_[] =
"$Id: ATM_Attributes.h,v 1.10 1998/09/28 12:31:16 bilal Exp $";
#endif

#include <codec/uni_ie/ie_base.h>
class generic_q93b_msg;
class API_FSM;
class ATM_Interface;
class ATM_Call;
class Outgoing_ATM_Call;
class sean_Visitor;
class Buffer;
class Controller;
class sean_io;

#include "API_fsms.h"
#include "Memory_Manager.h"
#include <codec/uni_ie/ie.h>
#include <sean/templates/constants.h>


class ATM_Attributes : public Recyclable {

public:
  
  enum result {
    success = 0,
    failure = 1
  };
  
  result Set_cause(ie_cause::CauseValues err);
  ie_cause::CauseValues Get_cause(void);
  
  // result Set_aal_1(void);
  // result Set_aal_2(void);
  // result Set_aal_34(void);
  result Set_aal_5(aal5::aal5_sscs sscs, 
		   int fm_sdu_size, 
		   int bm_sdu_size);
  
  ie_aal::AAL_TYPE Get_aal_type (void) const;
  
  // result Get_aal_1 (void)  const;
  // result Get_aal_2 (void)  const;
  // result Get_aal_34 (void) const;
  result Get_aal_5 (aal5::aal5_sscs& sscs, 
		    int &fm_sdu_size, 
		    int &bm_sdu_size) const;
  
  result Set_bbc(ie_bbc::Bbc_class bbc,
		 ie_bbc::clipping clip,
		 ie_bbc::conn_type ctype, char atc);

  result Get_bbc(ie_bbc::Bbc_class & bbc,
		 ie_bbc::clipping & clip,
		 ie_bbc::conn_type & ctype, char & atc) const;

  ie_bbc::Bbc_class Get_bbc_type(void) const;
  result Get_conn_type(ie_bbc::conn_type& ctype) const;
  
  bool Is_P2P(void);
  bool Is_P2MP(void);
  
  result Set_bhli_iso(unsigned long x);
  result Set_bhli_iso(unsigned char *buffer, int buflen);
  result Set_bhli_user(unsigned long x);
  result Set_bhli_user(unsigned char *buffer, int buflen);
  result Set_bhli_vendor(int oui, int appid);
  
  ie_bhli::bhli_type Get_bhli_type(void) const;
  
  result Get_bhli_iso(unsigned char*& buffer, 
		      int& buflen) const;
  result Get_bhli_user(unsigned char*& buffer, 
		       int& buflen) const;
  result Get_bhli_vendor(unsigned char*& buffer, 
			 int& buflen) const;
  
  result Set_blli_IP(void);
  result Set_blli_none(void);
  
  result Set_called_party_num(Addr* dst_addr);
  result Get_called_party_num(Addr*& dst_addr) const;
  
  result Set_called_party_subaddr(Addr* dst_subaddr);
  result Get_called_party_subaddr(Addr*& dst_subaddr) const;
  
  result Set_calling_party_num(Addr* src_addr, 
			       ie_calling_party_num::presentation_indicator presentation,     
			       ie_calling_party_num::screening_indicator screening);
  result Get_calling_party_num(Addr*& src_addr, 
			       ie_calling_party_num::presentation_indicator& presentation, 
			       ie_calling_party_num::screening_indicator& screening) const;
  
  result Set_calling_party_subaddr(Addr * src_subaddr);
  result Get_calling_party_subaddr(Addr*& src_subaddr) const;
  
  result Set_qos_param(ie_qos_param::qos_indicator fqos_class, 
		       ie_qos_param::qos_indicator bqos_class);
  result Get_qos_param(ie_qos_param::qos_indicator& fqos_class, 
		       ie_qos_param::qos_indicator& bqos_class) const;
  
  result Set_qos(ie_qos_param::qos_indicator fq, 
		 ie_qos_param::qos_indicator bq);
  result Get_qos(IE_TRAFFIC_DESC::dir dir, 
		 ie_qos_param::qos_indicator& qos) const;
  
  result Set_td_1(IE_TRAFFIC_DESC::dir dir, 
		  int pcr_0, 
		  int pcr_01);	
  result Set_td_2(IE_TRAFFIC_DESC::dir dir, 
		  int pcr_0, 
		  int pcr_01);
  result Set_td_3(IE_TRAFFIC_DESC::dir dir, 
		  int pcr_01, 
		  int scr_0, 
		  int mbs_0);
  result Set_td_4(IE_TRAFFIC_DESC::dir dir, 
		  int pcr_01, 
		  int scr_0, 
		  int mbs_0);
  result Set_td_5(IE_TRAFFIC_DESC::dir dir, 
		  int pcr_01);
  result Set_td_6(IE_TRAFFIC_DESC::dir dir, 
		  int pcr_01, 
		  int scr_01, 
		  int mbs_01);
  result Set_td_BE(int fpcr_01, 
		   int bpcr_01);
  
  IE_TRAFFIC_DESC::VALID_TPC Get_td_type(IE_TRAFFIC_DESC::dir d) const;
  
  result Get_td_1(IE_TRAFFIC_DESC::dir dir, 
		  int& pcr_0, 
		  int& pcr_01) const;
  result Get_td_2(IE_TRAFFIC_DESC::dir dir, 
		  int& pcr_0, 
		  int& pcr_01) const;
  result Get_td_3(IE_TRAFFIC_DESC::dir dir, 
		  int& pcr_01, 
		  int& scr_0, 
		  int& mbs_0) const;
  result Get_td_4(IE_TRAFFIC_DESC::dir dir, 
		  int& pcr_01, 
		  int& scr_0, 
		  int& mbs_0) const;
  result Get_td_5(IE_TRAFFIC_DESC::dir dir, 
		  int& pcr_01) const;
  result Get_td_6(IE_TRAFFIC_DESC::dir dir, 
		  int& pcr_01, 
		  int& scr_01, 
		  int& mbs_01) const;
  result Get_td_BE(int& fpcr_01, 
		   int& bpcr_01) const;
  
  result Get_vpi(long& vpi) const;
  result Get_vci(long& vci) const;

  // Set_ep_ref(int x) is not exported.
  int Get_ep_ref(void) const;



  // ---------------------------
  // UNI-4.0 specific attributes
  /* exclusive VPI, VCI */

  result Set_exclusive_vpi_vci(long vpi, long vci);
  result Set_exclusive_vpi_any_vci(long vpi); 
  result Set_exclusive_vpi_no_vci(long vpi);

  // ---------------------------
  // UNI-4.0 specific attributes
  /* Available Bit Rate */

  result Set_ABR(int ficr, int bicr, int ftbe, int btbe, int crtt, 
		 int frif, int brif, int frdf, int brdf);
  result Get_ABR(int& ficr, int& bicr, int& ftbe, int& btbe, int& crtt, 
		 int& frif, int& brif, int& frdf, int& brdf);

  // ---------------------------
  // UNI-4.0 specific attributes
  /* End-to-End delay requirements */

  result Set_e2e_transit_delay(int cumulative, int maximum);
  result Get_e2e_transit_delay(int& cumulative, int& maximum) const;

  // ---------------------------
  // UNI-4.0 specific attributes
  /* Best Effort with Max Cell Rate */

  result Set_td_BE_with_MCR(int fpcr_01, 
			    int bpcr_01,
			    int fmcr_01,
			    int bmcr_01);

  result Get_td_BE_with_MCR(int& fpcr_01, 
			    int& bpcr_01,
			    int& fmcr_01,
			    int& bmcr_01) const;

  // ---------------------------
  // UNI-4.0 specific attributes
  /* QoS negotiation: Minimum traffic descriptor */

  result Set_minimum_FPCR_0(int fpcr0);
  result Set_minimum_BPCR_0(int bpcr0);
  result Set_minimum_FPCR_01(int fpcr01);
  result Set_minimum_BPCR_01(int bpcr01);
  result Set_minimum_ForwardABR(int fabr);
  result Set_minimum_BackwardABR(int babr);

  result Get_minimum_FPCR_0(int& fpcr0) const;
  result Get_minimum_BPCR_0(int& bpcr0) const;
  result Get_minimum_FPCR_01(int& fpcr01) const;
  result Get_minimum_BPCR_01(int& bpcr01) const;
  result Get_minimum_ForwardABR(int& fabr) const;
  result Get_minimum_BackwardABR(int& babr) const;


  // ---------------------------
  // UNI-4.0 specific attributes
  /* QoS negotiation: Alternate traffic descriptor */
  // Currently, only a single alternate traffic 
  // descriptor is supported

  result Set_alt_td_1(UNI40_traffic_desc::dir d, int pcr_0, int pcr_01);    
  result Set_alt_td_2(UNI40_traffic_desc::dir d, int pcr_0, int pcr_01);    
  result Set_alt_td_3(UNI40_traffic_desc::dir d, int pcr_01, int scr_0,  int mbs_0);    
  result Set_alt_td_4(UNI40_traffic_desc::dir d, int pcr_01, int scr_0,  int  mbs_0);    
  result Set_alt_td_5(UNI40_traffic_desc::dir d, int pcr_01);    
  result Set_alt_td_6(UNI40_traffic_desc::dir d, int pcr_0, int scr_01, int mbs_01);    
  result Set_alt_td_BE(int fpcr_01, int bpcr_01);
  result Set_alt_td_BE_with_MCR(int fpcr_01, int bpcr_01, int fmcr_01, int bmcr_01);

  result Get_alt_td_1(UNI40_traffic_desc::dir d, int& pcr_0, int& pcr_01);    
  result Get_alt_td_2(UNI40_traffic_desc::dir d, int& pcr_0, int& pcr_01);    
  result Get_alt_td_3(UNI40_traffic_desc::dir d, int& pcr_01, int& scr_0,  int& mbs_0);    
  result Get_alt_td_4(UNI40_traffic_desc::dir d, int& pcr_01, int& scr_0,  int&  mbs_0);    
  result Get_alt_td_5(UNI40_traffic_desc::dir d, int& pcr_01);    
  result Get_alt_td_6(UNI40_traffic_desc::dir d, int& pcr_0, int& scr_01, int& mbs_01);    
  result Get_alt_td_BE(int& fpcr_01, int& bpcr_01);
  result Get_alt_td_BE_with_MCR(int& fpcr_01, int& bpcr_01, int& fmcr_01, int& bmcr_01);

  IE_TRAFFIC_DESC::VALID_TPC Get_alt_td_type(IE_TRAFFIC_DESC::dir d) const;


  // ---------------------------
  // UNI-4.0 specific attributes
  /* Generic identifier supporting
     - Digital Storage Media Command & Control 
     - Recommendation H.245 */

  // Only one of these is in effect, 
  // repeated calls clobber previous results

  result Set_DSMCC_generic_session_id(u_char* buf, int len);
  result Set_DSMCC_generic_resource_id(u_char* buf, int len);
  result Set_H245_generic_resource_id(u_char* buf, int len);
  result Set_H245_generic_session_id(u_char* buf, int len);

  result Get_generic_id(unsigned char*& buffer, int& buflen) const;
  bool Is_DSMCC_generic_id_present(void) const;
  bool Is_H245_generic_id_present(void) const;


  // ---------------------------
  // UNI-4.0 specific attributes
  /* Leaf Initiated Join sequence number */

  // Set_LIJ_sequence_number(int sequence) is not exported.
  result Get_LIJ_sequence_number(int& sequence) const;



  // ---------------------------
  // UNI-4.0 specific attributes
  /* Leaf Initiated Join Parameters */

  result Set_LIJ_net_decides(void);
  result Set_LIJ_root_decides(void);


  // ---------------------------
  // UNI-4.0 specific attributes
  /* P2MP Call identifier */

  result Set_LIJ_call_id(int value);
  result Get_LIJ_call_id(int& value);




  int Get_crv(void) const;

  // these only work if your interface is still around
  ATM_Interface& Get_interface(void) const;
  bool Associate_Controller(Controller& c);
  bool Divorce_Controller(void);

protected:
  enum fsm_type {
    unknown_FSM,
    Call_FSM,
    Service_FSM,
    Leaf_FSM
  };

  void protect(void);
  void unprotect(void);


  ATM_Attributes(ATM_Interface& interface, fsm_type t);

  ATM_Attributes(ATM_Interface& interface,            // adding a leaf
		 Outgoing_ATM_Call& call, fsm_type t);

  ATM_Attributes(ATM_Interface& interface, 
		 generic_q93b_msg* msg, int cid);     // incoming call

  ATM_Attributes(ATM_Interface& interface, ATM_Call& call, 
		 generic_q93b_msg* msg, fsm_type t,
		 int lid);  // leaf_setup_ind

  virtual ~ATM_Attributes(void);

  int get_id(void) const;

  bool push_FSM(sean_Visitor* sv);

  generic_q93b_msg* make_freeform(Buffer* buf);
  generic_q93b_msg* make_setup(Buffer* buf);
  generic_q93b_msg* make_release(Buffer* buf);
  generic_q93b_msg* make_release_comp(Buffer* buf);
  generic_q93b_msg* make_connect(Buffer* buf);
  generic_q93b_msg* make_addparty(Buffer* buf);
  generic_q93b_msg* make_dropparty(Buffer* buf);
  generic_q93b_msg* make_addparty_ack(Buffer* buf);
  generic_q93b_msg* make_addparty_rej(Buffer* buf);
  generic_q93b_msg* make_leaf_setup_request(Buffer* buf);
  generic_q93b_msg* make_leaf_setup_failure(Buffer* buf);

  virtual void state_has_changed(API_FSM::api_fsm_state from, 
				 API_FSM::api_fsm_state to,
				 ATM_Call* newc,
				 ATM_Leaf* newl) = 0;
  void fsm_failure(int code = -1);

  void notify_FSM_that_queue_is_empty(void);

  API_FSM::api_fsm_state get_fsm_state(void) const;
  void set_crv(int crv);
  void update_from_message(generic_q93b_msg* m);

  void kill_api_call_datastructure(int cid);
  void kill_api_service_datastructure(int sid);
  void kill_api_leaf_datastructure(int cid, int lid);

  void reset_modifiable_mask( bool v );
  void set_modifiable_bits(API_FSM::api_fsm_state state);

  void disable_controller_callbacks(void);
  void orphaned_by_controller(Controller* abandoner);
  Controller*    _control;

  virtual sean_io* get_datapath(void);

  void initialize_leaf_initiated_call(void);
  void set_dummy_LIJ_sequence_number(void);

  void swap_called_and_calling_party(void);
  result Set_ep_ref(int x);

private:

  result Set_LIJ_sequence_number(int sequence);

  bool Authorize(InfoElem::ie_ix attribute);

  result set_attr(InfoElem::ie_ix attribute, InfoElem* ie);
  InfoElem* get_attr (InfoElem::ie_ix attribute) const;

  bool get_read_only(InfoElem::ie_ix ix);
  void set_read_only(InfoElem::ie_ix ix, bool value);
  void set_inherited(InfoElem::ie_ix ix, bool value);

  ostream& print_ies(ostream& os);

  ATM_Interface* _interface;
  API_FSM*       _fsm;
  fsm_type       _fsmtype;
  int            _fsm_failed;

  InfoElem* _ie[num_ie];
  bool      _indirect[num_ie];
  u_long    _cref;
  u_int     _flag;

  int       _id;
  bool      _modifiable[num_ie];
  bool      _protect;

  friend class Outgoing_ATM_Call;
  void force_leaf_to_active_state(void);
  void force_leaf_to_dead_state(void);
  void force_call_to_dead_state(void);

  friend class API_FSM;
  friend class Controller;
  friend class sean_io;
};

#endif


