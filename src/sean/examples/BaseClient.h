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

#include <sean/api/ATM_Interface.h>
#include <sean/api/Controllers.h>
#include <sean/api/Call_API.h>
#include <sean/api/Leaf_API.h>

class BaseClient : public Controller {
public:
  BaseClient(ATM_Interface &interface, char *called_party, bool p2mp = false);
  virtual ~BaseClient(void);
  bool IsP2MP();
  void AddParty(Addr *addr);
protected:
  virtual void PeriodicCallback(int code);
  virtual void Boot(ATM_Interface &interface);

  virtual void Call_Active(ATM_Call& call);
  void Leaf_Active(ATM_Leaf &leaf, ATM_Call &call);

  virtual void Call_Inactive(ATM_Call& call);
  virtual void Incoming_Data(ATM_Call& call);

  void CallAgain(ATM_Interface &interface); 

  ATM_Interface &_interface;

  Outgoing_ATM_Call *_outgoing_call;
  Addr *_called_party_number;
  bool _p2mp;
  Addr *_leaf_addr;
};
