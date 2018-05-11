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
// Version: $Id: PrintFunctions.h,v 1.1 1998/07/15 06:38:50 talmage Exp $

#ifndef LINT
static char const _PrintFunctions_h_rcsid_[] =
"$Id: PrintFunctions.h,v 1.1 1998/07/15 06:38:50 talmage Exp $";
#endif

#ifndef __PrintFunctions_h__
#define __PrintFunctions_h__

#include <sean/api/ATM_Interface.h>
#include <sean/api/Signalling_API.h>

extern void PrintResult(ATM_Attributes::result the_result);
extern void PrintStatus(ATM_Call::call_status c_status);
extern void PrintStatus(ATM_Service::service_status s_status);

extern void PrintBHLIUser(ATM_Call & call);
extern void PrintBHLIUser(ATM_Service & service);

#endif
