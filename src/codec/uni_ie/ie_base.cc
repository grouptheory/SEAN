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
static char const _ie_base_cc_rcsid_[] =
"$Id: ie_base.cc,v 1.20 1999/04/15 16:37:13 mountcas Exp $";
#endif
#include <common/cprototypes.h>

#include <codec/uni_ie/ie_base.h>
#include <FW/basics/diag.h>

int InfoElem::equals(const E164_addr* x) const { return 0; }
int InfoElem::equals(const NSAP_addr* x) const { return 0; }
int InfoElem::equals(const NSAP_DCC_ICD_addr* x) const { return 0; }
int InfoElem::equals(const NSAP_E164_addr* x) const { return 0; }
int InfoElem::equals(const iso_bhli* x) const { return 0; }
int InfoElem::equals(const user_bhli* x) const { return 0; }
int InfoElem::equals(const vendor_bhli* x) const { return 0; }
int InfoElem::equals(const ie_blli* x) const { return 0; }
int InfoElem::equals(const ie_bbc* x) const { return 0; }
// int InfoElem::equals(const A_Bbc* x) const { return 0; }
// int InfoElem::equals(const C_Bbc* x) const { return 0; }
// int InfoElem::equals(const X_Bbc* x) const { return 0; }
int InfoElem::equals(const aal5 * x) const { return 0; }
int InfoElem::equals(const ie_broadband_repeat_ind* x) const { return 0; }
int InfoElem::equals(const ie_broadband_send_comp_ind* x) const { return 0; }
int InfoElem::equals(const ie_call_state* x) const { return 0; }
int InfoElem::equals(const ie_called_party_num* x) const { return 0; }
int InfoElem::equals(const ie_called_party_subaddr* x) const { return 0; }
int InfoElem::equals(const ie_calling_party_num* x) const { return 0; }
int InfoElem::equals(const ie_calling_party_subaddr* x) const { return 0; }
int InfoElem::equals(const ie_cause* x) const { return 0; }
int InfoElem::equals(const ie_conn_id* x) const { return 0; }
int InfoElem::equals(const ie_end_pt_ref* x) const { return 0; }
int InfoElem::equals(const ie_end_pt_state* x) const { return 0; }
int InfoElem::equals(const ie_qos_param* x) const { return 0; }
int InfoElem::equals(const ie_restart_ind* x) const { return 0; }
int InfoElem::equals(const ie_traffic_desc* x) const { return 0; }
int InfoElem::equals(const ie_transit_net_sel* x) const { return 0; }
int InfoElem::equals(const UNI40_lij_call_id* x) const { return 0; }
int InfoElem::equals(const UNI40_lij_params * x) const { return 0; }
int InfoElem::equals(const UNI40_leaf_sequence_num* x) const { return 0; }
int InfoElem::equals(const UNI40_abr_params * x) const { return 0; }
int InfoElem::equals(const UNI40_abr_setup * x) const { return 0; }
int InfoElem::equals(const UNI40_generic_id * x) const { return 0; }
int InfoElem::equals(const UNI40_e2e_transit_delay * x) const { return 0; }
int InfoElem::equals(const UNI40_alt_traffic_desc * x) const { return 0; }
int InfoElem::equals(const UNI40_min_traffic_desc * x) const { return 0; }
int InfoElem::equals(const PNNI_called_party_soft_pvpc * x) const { return 0;}
int InfoElem::equals(const PNNI_crankback * x) const { return 0; }
int InfoElem::equals(const PNNI_designated_transit_list * x) const { return 0;}
int InfoElem::equals(const PNNI_calling_party_soft_pvpc * x) const { return 0;}
int InfoElem::equals(const ie_security_service * himptr) const { return 0; }

void special_print(const u_char * buf, const int len, ostream & os)
{
  int counter = 0, chcnt = 0;

  while ( counter < len ) {
    // print the offset
    os.width( 6 ); os.fill( '0' );
    os << counter << "  ";
    // print the hex numbers
    os << hex;
    while ( counter < len ) {
      int val = (int)buf[ counter++ ];
      if ( val < 16 ) os << "0";
      os << val << " ";
      chcnt++;

      if ( counter != 0 && counter %  8 == 0 )
	os << " ";

      if ( counter != 0 && counter % 16 == 0 )
	break;
    }
    os << dec;
    int sc = chcnt;
    if ( chcnt < 16 ) {
      if ( chcnt < 8 )
	os << " ";
      os << " ";
      while ( chcnt < 16 ) {
	os << "   ";
	chcnt++;
      }
    }
    os << " " << flush;

    // print the characters
    for ( int i = counter - sc; i < counter; i++ ) {
      char val = (char)buf[ i ];
      if ( isprint( val ) )
	os << val;
      else
	os << '.';
    }
    chcnt = 0;
    os << endl << flush;
  }
}

const char * id2name( InfoElem::ieid id )
{
  const char * rval = 0;

  switch ( id ) {
    case InfoElem::ie_cause_id:
      rval = "ie_cause";
      break;
    case InfoElem::UNI40_e2e_transit_delay_id:
      rval = "UNI40_e2e_transit_delay";
      break;
    case InfoElem::ie_call_state_id:
      rval = "ie_call_state";
      break;
    case InfoElem::UNI40_notification_ind_id:
      rval = "UNI40_notification_ind";
      break;
    case InfoElem::ie_end_pt_ref_id:
      rval = "ie_end_pt_ref";
      break;
    case InfoElem::ie_end_pt_state_id:
      rval = "ie_end_pt_state";
      break;
    case InfoElem::ie_aal_param_id:
      rval = "ie_aal_param";
      break;
    case InfoElem::ie_traffic_desc_id:
      rval = "ie_traffic_desc";
      break;
    case InfoElem::ie_conn_identifier_id:
      rval = "ie_conn_identifier";
      break;
    case InfoElem::ie_qos_param_id:
      rval = "ie_qos_param";
      break;
    case InfoElem::ie_bhli_id:
      rval = "ie_bhli";
      break;
    case InfoElem::ie_broadband_bearer_cap_id:
      rval = "ie_broadband_bearer_cap";
      break;
    case InfoElem::ie_blli_id:
      rval = "ie_blli";
      break;
    case InfoElem::ie_broadband_locking_shift_id:
      rval = "ie_broadband_locking_shift";
      break;
    case InfoElem::ie_broadband_nlocking_shift_id:
      rval = "ie_broadband_nlocking_shift";
      break;
    case InfoElem::ie_broadband_send_comp_ind_id:
      rval = "ie_broadband_send_comp_ind";
      break;
    case InfoElem::ie_broadband_repeat_ind_id:
      rval = "ie_broadband_repeat_ind";
      break;
    case InfoElem::ie_calling_party_num_id:
      rval = "ie_calling_party_num";
      break;
    case InfoElem::ie_calling_party_subaddr_id:
      rval = "ie_calling_party_subaddr";
      break;
    case InfoElem::ie_called_party_num_id:
      rval = "ie_called_party_num";
      break;
    case InfoElem::ie_called_party_subaddr_id:
      rval = "ie_called_party_subaddr";
      break;
    case InfoElem::ie_transit_net_sel_id:
      rval = "ie_transit_net_sel";
      break;
    case InfoElem::ie_restart_ind_id:
      rval = "ie_restart_ind";
      break;
    case InfoElem::UNI40_generic_id_id:
      rval = "UNI40_generic_id";
      break;
    case InfoElem::UNI40_min_traff_desc_id:
      rval = "UNI40_min_traff_desc";
      break;
    case InfoElem::UNI40_alt_traff_desc_id:
      rval = "UNI40_alt_traff_desc";
      break;
    case InfoElem::UNI40_abr_setup_id:
      rval = "UNI40_abr_setup";
      break;
    case InfoElem::PNNI_called_party_soft_pvpc_id:
      rval = "PNNI_called_party_soft_pvpc";
      break;
    case InfoElem::PNNI_crankback_id:
      rval = "PNNI_crankback";
      break;
    case InfoElem::PNNI_designated_transit_list_id:
      rval = "PNNI_designated_transit_list";
      break;
    case InfoElem::PNNI_calling_party_soft_pvpc_id:
      rval = "PNNI_calling_party_soft_pvpc";
      break;
    case InfoElem::UNI40_abr_params_id:
      rval = "UNI40_abr_params";
      break;
    case InfoElem::UNI40_leaf_call_id:
      rval = "UNI40_leaf_call";
      break;
    case InfoElem::UNI40_leaf_params_id:
      rval = "UNI40_leaf_params";
      break;
    case InfoElem::UNI40_leaf_sequence_num_id:
      rval = "UNI40_leaf_sequence_num";
      break;
    case InfoElem::UNI40_conn_scope_id:
      rval = "UNI40_conn_scope";
      break;
    case InfoElem::UNI40_xqos_param_id:
      rval = "UNI40_xqos_param";
      break;
    case InfoElem::ie_security_service_id:
      rval = "ie_security_service";
      break;
    case InfoElem::ie_unknown_id:
    default:
      rval = "ie_unknown";
      break;
  }
  return rval;
}

#include <codec/uni_ie/PNNI_called_party_pvpc.h>
#include <codec/uni_ie/PNNI_calling_party_pvpc.h>
#include <codec/uni_ie/PNNI_crankback.h>
#include <codec/uni_ie/PNNI_designated_transit_list.h>
#include <codec/uni_ie/UNI40_abr.h>
#include <codec/uni_ie/UNI40_bbc.h>
#include <codec/uni_ie/UNI40_conn_scope.h>
#include <codec/uni_ie/UNI40_e2e_trans_delay.h>
#include <codec/uni_ie/UNI40_generic.h>
#include <codec/uni_ie/UNI40_leaf.h>
#include <codec/uni_ie/UNI40_notif.h>
#include <codec/uni_ie/UNI40_td.h>
#include <codec/uni_ie/UNI40_xqos.h>
#include <codec/uni_ie/aal.h>
#include <codec/uni_ie/addr.h>
#include <codec/uni_ie/bhli.h>
#include <codec/uni_ie/blli.h>
#include <codec/uni_ie/broadband_repeat_ind.h>
#include <codec/uni_ie/broadband_send_comp_ind.h>
#include <codec/uni_ie/cache.h>
#include <codec/uni_ie/call_state.h>
#include <codec/uni_ie/called_party_num.h>
#include <codec/uni_ie/called_party_subaddr.h>
#include <codec/uni_ie/calling_party_num.h>
#include <codec/uni_ie/calling_party_subaddr.h>
#include <codec/uni_ie/cause.h>
#include <codec/uni_ie/conn_id.h>
#include <codec/uni_ie/e2e_transit_delay.h>
#include <codec/uni_ie/end_pt_ref.h>
#include <codec/uni_ie/end_pt_state.h>
#include <codec/uni_ie/errmsg.h>
#include <codec/uni_ie/qos_param.h>
#include <codec/uni_ie/restart_ind.h>
#include <codec/uni_ie/security_service.h>
#include <codec/uni_ie/subaddr.h>
#include <codec/uni_ie/td.h>
#include <codec/uni_ie/transit_net_sel.h>

InfoElem * buildIE( InfoElem::ieid id )
{
  InfoElem * rval = 0;

  switch ( id ) {
    case InfoElem::ie_cause_id:
      rval = new ie_cause( );
      break;
    case InfoElem::UNI40_e2e_transit_delay_id:
      rval = new UNI40_e2e_transit_delay( );
      break;
    case InfoElem::ie_call_state_id:
      rval = new ie_call_state( );
      break;
    case InfoElem::UNI40_notification_ind_id:
      rval = new UNI40_notification_indicator( );
      break;
    case InfoElem::ie_end_pt_ref_id:
      rval = new ie_end_pt_ref( );
      break;
    case InfoElem::ie_end_pt_state_id:
      rval = new ie_end_pt_state( );
      break;
    case InfoElem::ie_aal_param_id:
      rval = new aal5( );
      break;
    case InfoElem::ie_traffic_desc_id:
      rval = new ie_traffic_desc( );
      break;
    case InfoElem::ie_conn_identifier_id:
      rval = new ie_conn_id( );
      break;
    case InfoElem::ie_qos_param_id:
      rval = new ie_qos_param( );
      break;
    case InfoElem::ie_bhli_id:
      rval = new iso_bhli( );
      break;
    case InfoElem::ie_blli_id:
      rval = new ie_blli( );
      break;
    case InfoElem::ie_broadband_send_comp_ind_id:
      rval = new ie_broadband_send_comp_ind( );
      break;
    case InfoElem::ie_broadband_repeat_ind_id:
      rval = new ie_broadband_repeat_ind( );
      break;
    case InfoElem::ie_calling_party_num_id:
      rval = new ie_calling_party_num( );
      break;
    case InfoElem::ie_calling_party_subaddr_id:
      rval = new ie_calling_party_subaddr( );
      break;
    case InfoElem::ie_called_party_num_id:
      rval = new ie_called_party_num( );
      break;
    case InfoElem::ie_called_party_subaddr_id:
      rval = new ie_called_party_subaddr( );
      break;
    case InfoElem::ie_transit_net_sel_id:
      rval = new ie_transit_net_sel( );
      break;
    case InfoElem::ie_restart_ind_id:
      rval = new ie_restart_ind( );
      break;
    case InfoElem::UNI40_min_traff_desc_id:
      rval = new UNI40_min_traffic_desc( );
      break;
    case InfoElem::UNI40_alt_traff_desc_id:
      rval = new UNI40_alt_traffic_desc( );
      break;
    case InfoElem::UNI40_abr_setup_id:
      rval = new UNI40_abr_setup( );
      break;
    case InfoElem::PNNI_called_party_soft_pvpc_id:
      rval = new PNNI_called_party_soft_pvpc( );
      break;
    case InfoElem::PNNI_crankback_id:
      rval = new PNNI_crankback( );
      break;
    case InfoElem::PNNI_designated_transit_list_id:
      rval = new PNNI_designated_transit_list( );
      break;
    case InfoElem::PNNI_calling_party_soft_pvpc_id:
      rval = new PNNI_calling_party_soft_pvpc( );
      break;
    case InfoElem::UNI40_abr_params_id:
      rval = new UNI40_abr_param( );
      break;
    case InfoElem::UNI40_leaf_call_id:
      rval = new UNI40_lij_call_id( );
      break;
    case InfoElem::UNI40_leaf_params_id:
      rval = new UNI40_lij_params( );
      break;
    case InfoElem::UNI40_leaf_sequence_num_id:
      rval = new UNI40_leaf_sequence_num( );
      break;
    case InfoElem::UNI40_conn_scope_id:
      rval = new UNI40_conn_scope_sel( );
      break;
    case InfoElem::UNI40_xqos_param_id:
      rval = new UNI40_xqos_param( );
      break;
    case InfoElem::ie_security_service_id:
      rval = new ie_security_service( );
      break;
    case InfoElem::ie_unknown_id:
    default:
      diag( "codec.uni_ie", DIAG_FATAL,
	    "buildIE received invalid InfoElem ID %x!\n",
	    id );
      break;
  }
  return rval;
}
