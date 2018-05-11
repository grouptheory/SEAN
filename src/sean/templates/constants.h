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


#ifndef __SEAN_CONSTANTS_H__
#define __SEAN_CONSTANTS_H__

// unassigned values for various fields

extern const int NO_CREF;
extern const int NO_FLAG;
extern const int NO_VPI;
extern const int NO_VCI;
extern const int NO_EPR;
extern const int NO_PID;

extern const int NO_CID;
extern const int NO_SID;
extern const int NO_LID;

class abstract_local_id;
extern const abstract_local_id * NO_APPID;

extern const int NO_PORT;

#include <codec/uni_ie/UNI40_td.h>
#include <codec/uni_ie/td.h>
// set this to 
//     ie_traffic_desc
// or
//     UNI40_traffic_desc
//
#define IE_TRAFFIC_DESC    UNI40_traffic_desc

#endif
