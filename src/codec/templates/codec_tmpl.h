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
#ifndef __CODEC_TMPL_H__
#define __CODEC_TMPL_H__

#ifndef LINT
static char const _codec_tmpl_h_rcsid_[] =
"$Id: codec_tmpl.h,v 1.12 1999/04/16 17:17:09 talmage Exp $";
#endif
#include <FW/templates/FW_tmpl.h>

class  ig_ptse;
class  PeerID;
class  ig_horizontal_links;
class  ig_uplinks;
class  ig_nodal_ptse_ack;
class  ig_req_ptse_header;
class  ig_nodal_ptse_summary;
class  ig_nodal_state_params;
class  DBKey;
class  PNNI_crankback;
class  PTSPPkt;
class  PTSEReqPkt;
class  Addr;
class  AddrPrefixCont;
class  ReqContainer;
class  SumContainer;
class  AckContainer;
class  security_association;
class  security_service_alg;
class  security_service_opt;
class  security_service_spec;
class  FIPSSecurityTag;
class  FIPSNamedTagSet;
class  security_agent_id;

extern int compare (ig_ptse * const &, ig_ptse * const &);
extern int compare (PeerID * const &, PeerID * const &);
extern int compare (ig_horizontal_links * const &, ig_horizontal_links * const &);
extern int compare (ig_uplinks * const &, ig_uplinks * const &);
extern int compare (ig_nodal_ptse_ack * const &, ig_nodal_ptse_ack * const &);
extern int compare (ig_req_ptse_header * const &, ig_req_ptse_header * const &);
extern int compare (ig_nodal_ptse_summary * const &, ig_nodal_ptse_summary * const &);
extern int compare (ig_nodal_state_params * const &, ig_nodal_state_params * const &);
extern int compare (DBKey * const &, DBKey * const &);
extern int compare (PNNI_crankback * const &, PNNI_crankback * const &);
extern int compare (PTSPPkt * const &, PTSPPkt * const &);
extern int compare (PTSEReqPkt * const &, PTSEReqPkt * const &);
extern int compare (Addr * const &, Addr * const &);
extern int compare (AddrPrefixCont * const &, AddrPrefixCont * const &);
extern int compare (ReqContainer * const &, ReqContainer * const &);
extern int compare (SumContainer * const &, SumContainer * const &);
extern int compare (AckContainer * const &, AckContainer * const &);
extern int compare (security_association * const &, security_association * const &);
extern int compare (security_service_opt * const & x, security_service_opt * const & y);
extern int compare (security_service_spec * const & x, security_service_spec * const & y);
extern int compare (security_service_alg * const & x, security_service_alg * const & y);
extern int compare (FIPSSecurityTag * const &, FIPSSecurityTag * const &);
extern int compare (FIPSNamedTagSet * const &, FIPSNamedTagSet * const &);
extern int compare (security_agent_id * const &, security_agent_id * const &);

#endif
