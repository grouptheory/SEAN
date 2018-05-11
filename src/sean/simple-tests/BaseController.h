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
#ifndef __BASECONTROLLER_H__
#define __BASECONTROLLER_H__

#ifndef LINT
static char const _BaseController_h_rcsid_[] =
"$Id: BaseController.h,v 1.6 1998/07/29 22:01:22 talmage Exp $";
#endif


#include <sean/api/ATM_Interface.h>
#include <sean/api/Call_API.h>
#include <sean/api/Controllers.h>
#include <sean/api/Leaf_API.h>
#include <sean/api/Service_API.h>

class BaseController : public Controller {
public:

  BaseController(const char * diag_key, ATM_Interface &interface);
  virtual ~BaseController(void);

protected:
  char * _diag_key;
  ATM_Interface &_interface;

  Outgoing_ATM_Call *NewCall(void);
  Outgoing_ATM_Call *NewCall(char *called_party_atm_address);
  Outgoing_ATM_Call *NewCall(char *called_party_atm_address, int bhli);
  Outgoing_ATM_Call *NewCall(char *called_party_atm_address, char *bhli);

  Outgoing_ATM_Call *NewCall(Addr *called_party_atm_address);
  Outgoing_ATM_Call *NewCall(Addr *called_party_atm_address, int bhli);
  Outgoing_ATM_Call *NewCall(Addr *called_party_atm_address, char *bhli);

  //
  // called_party_address is always "optional" for a service.
  //
  ATM_Service *NewService(char *called_party_atm_address);
  ATM_Service *NewService(char *called_party_atm_address, int bhli);
  ATM_Service *NewService(char *called_party_atm_address, char *bhli);

  void RegisterService(ATM_Service *service);
  void DeregisterService(ATM_Service *service);

private:

  virtual void PeriodicCallback(int code);
  virtual void Boot(void);

  virtual void Call_Active(ATM_Call& call);
  virtual void Incoming_LIJ(ATM_Call& call);
  virtual void Call_Inactive(ATM_Call& call);
  virtual void Incoming_Data(ATM_Call& call);

  virtual void Leaf_Active(ATM_Leaf& leaf, ATM_Call& call);
  virtual void Leaf_Inactive(ATM_Leaf& leaf, ATM_Call& call);

  virtual void Service_Active(ATM_Service& service);
  virtual void Service_Inactive(ATM_Service& service);
  virtual void Incoming_Call(ATM_Service& service);

};

#endif
