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
static char const _constants_cc_rcsid_[] =
"$Id: constants.cc,v 1.3 1998/08/06 04:04:20 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "constants.h"

// fake switch stuff

const int NO_PORT                    = 0xFFFFFFFF;

// signalling daemon stuff

const int NO_CREF    = 0x00FFFFFF;  // 7FFFFF;
const int NO_FLAG    = 0x00000001;
const int NO_VPI     = 0x0000FFFF;
const int NO_VCI     = 0x0000FFFF;
const int NO_EPR     = 0x0000FFFF;
const int NO_PID     = 0x0000FFFF;

// api stuff

const int NO_CID     = 0xFFFFFFFF;
const int NO_SID     = 0xFFFFFFFF;
const int NO_LID     = 0xFFFFFFFF;

// ipc stuff

class abstract_local_id;
const abstract_local_id * NO_APPID   = ((abstract_local_id *)0L);
