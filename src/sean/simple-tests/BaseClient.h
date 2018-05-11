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
#ifndef __BASECLIENT_H__
#define __BASECLIENT_H__

#ifndef LINT
static char const _BaseClient_h_rcsid_[] =
"$Id: BaseClient.h,v 1.4 1998/07/29 21:56:55 talmage Exp $";
#endif

#include <sean/simple-tests/BaseController.h>

class BaseClient : public BaseController {
public:

  BaseClient(const char * diag_key, ATM_Interface & interface,
	     char *called_party, char *bhli = 0);
  virtual ~BaseClient();

protected:
  char * _bhli;
  Addr * _called_party_number;

private:

  // Makes one call
  virtual void Boot(ATM_Interface &interface);
  //  virtual void PeriodicCallback(int code);

  //  virtual void Call_Active(ATM_Call   & call);
  //  virtual void Incoming_LIJ(ATM_Call  & call);
  //  virtual void Call_Inactive(ATM_Call & call);

};

#endif // __BASECLIENT_H__
