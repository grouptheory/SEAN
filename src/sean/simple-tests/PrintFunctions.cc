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
// Version: $Id: PrintFunctions.cc,v 1.3 1998/08/06 04:06:19 bilal Exp $
//

#ifndef LINT
static char const _PrintFunctions_cc_rcsid_[] =
"$Id: PrintFunctions.cc,v 1.3 1998/08/06 04:06:19 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "PrintFunctions.h"

void PrintResult(ATM_Attributes::result the_result)
{
  switch (the_result) {
  case ATM_Attributes::success:
    cout << "success";
    break;
  case ATM_Attributes::failure:
    cout << "failure";
    break;
  default:
    cout << "NEW VALUE of ATM_Attributes::result " 
	 << (int) the_result;
    break;
  }
}


void PrintStatus(ATM_Call::call_status c_status)
{
  switch (c_status) {
  case ATM_Call::CCD_DEAD:
    cout << "CCD_DEAD";
    break;

  case ATM_Call::USER_PREPARING_CALL:
    cout << "USER_PREPARING_CALL";
    break;

  case ATM_Call::OUTGOING_CALL_INITIATED:
    cout << "OUTGOING_CALL_INITIATED";
    break;

  case ATM_Call::INCOMING_CALL_WAITING:
    cout << "INCOMING_CALL_WAITING";
    break;

  case ATM_Call::CALL_P2P_READY:
    cout << "CALL_P2P_READY";
    break;

  case ATM_Call::CALL_P2MP_READY:
    cout << "CALL_P2MP_READY";
    break;

  case ATM_Call::CALL_LEAF_JOIN_PRESENT:
    cout << "CALL_LIJ_PRESENT";
    break;

  case ATM_Call::CALL_RELEASED:
    cout << "CALL_RELEASED" ;
    break;

  default:
    cout << "NEW call_status: " << (int)c_status;
    break;
  }
}


void PrintStatus(ATM_Service::service_status s_status)
{
  switch (s_status) {
  case ATM_Service::CCD_DEAD:
    cout << "CCD_DEAD";
    break;

  case ATM_Service::USER_PREPARING_SERVICE:
    cout << "USER_PREPARING_SERVICE";
    break;

  case ATM_Service::SERVICE_REGISTER_INITIATED:
    cout << "SERVICE_REGISTER_INITIATED";
    break;

  case ATM_Service::SERVICE_DEREGISTER_INITIATED:
    cout << "SERVICE_DEREGISTER_INITIATED";
    break;

  case ATM_Service::SERVICE_ACTIVE:
    cout << "SERVICE_ACTIVE";
    break;

  case ATM_Service::INCOMING_CALL_PRESENT:
    cout << "INCOMING_CALL_PRESENT";
    break;

  case ATM_Service::SERVICE_INACTIVE:
    cout << "SERVICE_INACTIVE";
    break;

  default:
    cout << "NEW service_status: " << (int)s_status;
    break;
  }
}


void PrintBHLIUser(ATM_Call & call)
{
  unsigned char *buffer = 0;
  int buffer_length = 0;
  ATM_Attributes::result err;
  err = call.Get_bhli_user(buffer, buffer_length);

  if (buffer_length > 0) {
    
    for (int i = 0; i < buffer_length; i ++) 
      cout << buffer[i];

    delete [] buffer;
  } else {
    cout << "*** no bhli_user ***";
  }

}


void PrintBHLIUser(ATM_Service & service)
{
  unsigned char *buffer = 0;
  int buffer_length = 0;
  ATM_Attributes::result err;
  err = service.Get_bhli_user(buffer, buffer_length);

  if (buffer_length > 0) {
    
    for (int i = 0; i < buffer_length; i ++) 
      cout << buffer[i];

    delete [] buffer;
  } else {
    cout << "*** no bhli_user ***";
  }

}
