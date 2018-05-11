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
static char const _BaseClient_cc_rcsid_[] =
"$Id: BaseClient.cc,v 1.10 1998/08/06 04:06:15 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "BaseClient.h"
#include "PrintFunctions.h"

BaseClient::BaseClient(const char * diag_key, 
		       ATM_Interface & interface,
		       char *called_party,
		       char *bhli) 
  : BaseController(diag_key, interface), _called_party_number(0), _bhli(0)
{
  if (bhli != 0)
    _bhli = strdup(bhli);
  else _bhli = strdup(diag_key);

  if (called_party != 0) {
    _called_party_number = newAddr(called_party);
  } 
}

BaseClient::~BaseClient()
{
  delete _called_party_number;
  free(_bhli);
}

void BaseClient::Boot(ATM_Interface &interface) 
{
  ATM_Attributes::result err;

  // -------------------------------------------------------------
  Outgoing_ATM_Call * call = 
    NewCall(_called_party_number, _bhli); 

  if (call) {
    call->Associate_Controller(*this);

    diag(_diag_key, DIAG_INFO, "Calling service.\n");

    err = call->Establish();

    DIAG(_diag_key, DIAG_INFO, 
	 cout << _diag_key << " (Info): ";
	 cout << "Result of call->Establish: ";
	 PrintResult(err);
	 cout << endl; );

    ATM_Call::call_status c_status = call->Status();

    DIAG(_diag_key, DIAG_INFO, 
	 cout << _diag_key << " (Info): ";
	 cout << "call status:";
	 PrintStatus(c_status);
	 cout << "." << endl; );
  } else {
    diag(_diag_key, DIAG_INFO, 
	 "BaseClient::Boot(): Couldn't create a call.  Aborting\n");
    exit(1);
  }
}
