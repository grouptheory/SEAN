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
#ifndef __PKT_INCL_H__
#define __PKT_INCL_H__
#if !defined(LINT)
static char const _pkt_incl_h_rcsid_[] =
"$Id: pkt_incl.h,v 1.4 1998/06/30 15:44:58 mountcas Exp $";
#endif

class PNNIPkt;

#include <iostream.h>
#include <iomanip.h>
#include <DS/containers/list.h>
#include <codec/pnni_ig/ig_incl.h>
#include <codec/pnni_ig/pnni_errmsg.h>
#include <codec/pnni_pkt/pnni_pkt.h>
#include <codec/pnni_pkt/database_sum.h>
#include <codec/pnni_pkt/hello.h>
#include <codec/pnni_pkt/pkt_incl.h>
#include <codec/pnni_pkt/pnni_pkt.h>
#include <codec/pnni_pkt/ptse_ack.h>
#include <codec/pnni_pkt/ptse_req.h>
#include <codec/pnni_pkt/ptsp.h>

#include <common/cprototypes.h>

extern "C" {
#include <sys/types.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <netinet/in.h>
};

typedef short int sh_int;

void PktHexDump(const u_char * buffer, int len, ostream& os);

//@Include: pnni_pkt.h hello.h database_sum.h ptse_ack.h ptse_req.h ptsp.h

#endif // __PKT_INCL_H__







