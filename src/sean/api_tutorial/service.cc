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

#ifndef LINT
static char const _service_cc_[] =
"$Id: service.cc,v 1.3 1998/09/28 12:31:44 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <sean/api/ATM_Interface.h>
#include <sean/api/Call_API.h>
#include <sean/api/Controllers.h>
#include <sean/api/Leaf_API.h>
#include <sean/api/Service_API.h>

class simple_service : public Controller {
public:

  simple_service(void) {
  }

  virtual ~simple_service(void) {
  }

private:
  void PeriodicCallback(int code) {
    cout << "PeriodicCallback\n";
  }
  
  void Boot(ATM_Interface& interface) {
    cout << "Boot\n";

    ATM_Service * service = new ATM_Service(interface);
    // we should handle all the state changes for this service
    service->Associate_Controller(*this);

    ATM_Attributes::result err;
    err = service->Set_td_BE(353207, 16777215);
    err = service->Set_bbc(ie_bbc::BCOB_X,
			   ie_bbc::not_clipped,
			   ie_bbc::p2p,
			   10);

    err = service->Set_qos_param(ie_qos_param::qos0,ie_qos_param::qos0);
    // service->Set_called_party_num( _called_party );

    // here we might set the bhli
    service->Register();
  }

  //-------------------------------------------------- call related callbacks
  void Call_Active(ATM_Call& call) {
    cout << "Call_Active\n";
    // call is ready
    // send data here 
  }

  void Call_Inactive(ATM_Call& call) {
    cout << "Call_Inactive\n";
  }
  void Incoming_Data(ATM_Call& call) {
    cout << "Incoming_Data\n";
    // throw data away
    call.Drain();
  }

  //-------------------------------------------------- service related callbacks
  void Service_Active(ATM_Service& service) {
    cout << "Service_Active\n";
  }
  void Service_Inactive(ATM_Service& service) {
    cout << "Service_Inactive\n";
  }
  void Incoming_Call(ATM_Service& service) {
    cout << "Incoming_Call\n";
    // a setup has arrived, extract call
    Incoming_ATM_Call* call = service.Get_Incoming_Call();
    // here we might look at the attributes of this call
    // we accept the call
    call->Accept();
  }
};



int main(int argc, char** argv) {

  DiagLevel("api",DIAG_ENV);
  DiagLevel("ipc",DIAG_ENV);

  int port = atoi(argv[1]);
  ATM_Interface& sig = Open_ATM_Interface("/dev/ignored", port);
  
  simple_service my_simple_service;
  sig.Associate_Controller(my_simple_service);

  SEAN_Run();
}
