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
static char const _CODEC_TMPL_CC_rcsid_[] =
"$Id: codec_tmpl.cc,v 1.31 1999/04/16 17:17:20 talmage Exp $";
#endif
#include <common/cprototypes.h>

#include <codec/templates/codec_tmpl.h>
#include <DS/containers/list.cc>
template class list<long> ;
struct AddrPrefixCont;
template class list<struct AddrPrefixCont *> ;
class ig_resrc_avail_info;
template class list<ig_resrc_avail_info *> ;
class ig_transit_net_id;
template class list<ig_transit_net_id *> ;
class HLinkCont;
template class list<HLinkCont *> ;
#include <codec/pnni_ig/lgn_horizontal_link_ext.h>
template class list<ig_lgn_horizontal_link_ext::HLinkCont *> ; 
#include <codec/pnni_ig/nodal_hierarchy_list.h>
template class list<Level *> ;
class AckContainer;
template class list<AckContainer *> ;
class SumContainer;
template class list<SumContainer *> ;
class InfoGroup;
template class list<InfoGroup *> ;
class ReqContainer;
template class list<ReqContainer *> ;
class ig_nodal_ptse_summary;
template class list<ig_nodal_ptse_summary *> ;
class ig_nodal_ptse_ack;
template class list<ig_nodal_ptse_ack *> ;
class ig_req_ptse_header;
template class list<ig_req_ptse_header *> ;
class ig_ptse;
template class list<ig_ptse *> ;
class DTLContainer;
template class list<DTLContainer *> ;
class security_association;
template class list<security_association *> ;
class security_service_opt;
template class list<security_service_opt *> ; 
class security_service_alg;
template class list<security_service_alg *> ;
class FIPSSecurityTag;
template class list<FIPSSecurityTag *> ;
class FIPSNamedTagSet;
template class list<FIPSNamedTagSet *> ;
class security_agent_id;
template class list<security_agent_id *> ;
class security_service_spec;
template class list<security_service_spec *> ;

class DBKey;
#include <DS/containers/dictionary.cc>
template class dictionary<DBKey *, ig_ptse *> ;

#include <DS/containers/sortseq.cc>
template class sortseq<DBKey *, ig_ptse *> ;

class ig_horizontal_links;
class ig_uplinks;
class ig_nodal_state_params;
class PNNI_crankback;
class PTSEReqPkt;
// ------- compares --------
int compare(DTLContainer *const & x, DTLContainer *const & y)
{ return (x > y ? 1 : y > x ? -1 : 0); }
int compare(HLinkCont *const & x, HLinkCont *const & y)
{ return (x > y ? 1 : y > x ? -1 : 0); }
int compare(ig_resrc_avail_info *const & x, ig_resrc_avail_info *const & y)
{ return (x > y ? 1 : y > x ? -1 : 0); }
int compare(ig_transit_net_id *const & x, ig_transit_net_id *const & y)
{ return (x > y ? 1 : y > x ? -1 : 0); }
int compare(ig_horizontal_links *const & x, ig_horizontal_links *const & y)
{ return (x > y ? 1 : y > x ? -1 : 0); }
int compare(ig_uplinks *const & x, ig_uplinks *const & y)
{ return (x > y ? 1 : y > x ? -1 : 0); }
int compare(ig_nodal_state_params * const & x, ig_nodal_state_params * const & y)
{ return (x > y ? 1 : y > x ? -1 : 0); }
int compare(PNNI_crankback * const & x, PNNI_crankback * const & y)
{ return (x > y ? 1 : y > x ? -1 : 0); }
int compare(PTSEReqPkt *const & x, PTSEReqPkt *const & y)
{ return (x > y ? 1 : y > x ? -1 : 0); }
int compare(int const & x, int const & y)
{ return (x > y ? 1 : y > x ? -1 : 0); }
int compare(long const & x, long const & y)
{ return (x > y ? 1 : y > x ? -1 : 0); }

#include <codec/uni_ie/security_service.h>
int compare (security_association * const & x, security_association * const & y)
{ 
  int xrid = x->getRelativeId(), yrid = y->getRelativeId();
  // decending order
  return ( xrid > yrid ? -1 : yrid > xrid ? 1 : 0 );
}

int compare (security_service_spec * const & x, security_service_spec * const & y)
{
  return (x > y ? 1 : y > x ? -1 : 0);
}

int compare (security_service_opt * const & x, security_service_opt * const & y)
{
  return (x > y ? 1 : y > x ? -1 : 0);
}

int compare (security_service_alg * const & x, security_service_alg * const & y)
{
  return (x > y ? 1 : y > x ? -1 : 0);
}

int compare (FIPSSecurityTag * const & x, FIPSSecurityTag * const & y)
{ return (x > y ? 1 : y > x ? -1 : 0); }

int compare (FIPSNamedTagSet * const & x, FIPSNamedTagSet * const & y)
{ return (x > y ? 1 : y > x ? -1 : 0); }

int compare (security_agent_id * const & x, security_agent_id * const & y)
{ return (x > y ? 1 : y > x ? -1 : 0); }

#include <codec/pnni_ig/ptse.h>

int compare(ig_ptse * const & lhs, ig_ptse * const & rhs)
{
  if ((lhs->_identifier == rhs->_identifier) &&
      (lhs->_seq_num == rhs->_seq_num) &&
      (((lhs->_remaining_lifetime == DBKey::ExpiredAge) &&
	(rhs->_remaining_lifetime == DBKey::ExpiredAge)) ||
       ((lhs->_remaining_lifetime != DBKey::ExpiredAge) &&
	(rhs->_remaining_lifetime != DBKey::ExpiredAge)))) 
    return 0;
  else if (((lhs->_identifier) < (rhs->_identifier)) ||
	   ((lhs->_identifier == rhs->_identifier) &&
	    (lhs->_seq_num < rhs->_seq_num)) ||
	   ((lhs->_identifier == rhs->_identifier) &&
	    (lhs->_seq_num == rhs->_seq_num) &&
	    ((lhs->_remaining_lifetime != DBKey::ExpiredAge) &&
	     (rhs->_remaining_lifetime == DBKey::ExpiredAge)))) 
    return -1;
  return 1;
}

