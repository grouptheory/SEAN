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
// File: SimpleService.h
// Version $Id: SimpleService.h,v 1.2 1998/08/17 17:35:23 talmage Exp $
//
#include <sean/api/ATM_Interface.h>
#include <sean/api/Controllers.h>

class SimpleService : public Controller {
public:
  SimpleService(ATM_Interface &interface);
  virtual ~SimpleService(void);

protected:

  virtual void PeriodicCallback(int code);
  virtual void Boot(ATM_Interface &interface);

  virtual void Call_Active(ATM_Call& call);
  virtual void Call_Inactive(ATM_Call& call);
  virtual void Incoming_Data(ATM_Call& call);

  virtual void Service_Active(ATM_Service& service);
  virtual void Service_Inactive(ATM_Service& service);
  virtual void Incoming_Call(ATM_Service& service);

  ATM_Interface &_interface;	// Use this when creating calls and services
  Incoming_ATM_Call *_incoming_call;
  int _incoming_call_reference;
};
