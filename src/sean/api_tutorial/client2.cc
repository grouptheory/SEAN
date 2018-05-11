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
static char const _client1_cc_rcsid_[] =
"$Id: client2.cc,v 1.4 1998/09/28 12:31:44 bilal Exp $";
#endif
#include <common/cprototypes.h>

// **************************************************************************
//
// This application makes a call to a command-line specified host
// The call is never torn down, and the application assumes a best case
// scenario where its request succeeds.
//
// NEW: It registers a periodic callback for 20 seconds from now, upon
// which it will exit.
//
// **************************************************************************
#include <sean/api/ATM_Interface.h>
#include <sean/api/Call_API.h>
#include <sean/api/Controllers.h>
#include <sean/api/Leaf_API.h>
#include <sean/api/Service_API.h>
#include <sean/daemon/HostFile.h>

class client_controller : public Controller {
private:
  Addr* _called_party;                 // who we intend to call

public:
  client_controller(Addr* who_to_call) { 
    _called_party = who_to_call;
  }

  virtual ~client_controller(void)    { /* nothing perhaps */  }

private:
  //-------------------------------------------------- interface related callbacks

  // Boot will happen when 'interface' is ready to attend to your signalling needs.
  void Boot(ATM_Interface& interface) {
    cout << "ATM_Interface is ready.\n";

    // make me a call object
    Outgoing_ATM_Call * outcall = new Outgoing_ATM_Call(interface);

    // when something happens to the call object 'outcall',
    // we'd like a callback to <this> client_controller instance.
    // e.g. a 'Call_Active' callback when 'outcall' is 
    // active (ready to receive/send data).
    outcall->Associate_Controller(*this);

    ATM_Attributes::result err;

    //
    // Register a periodic callback for 20 seconds from now.
    //
    RegisterPeriodicCallback(1, 20.0);

    // We must set the QoS parameters for the call prior to 
    // attempting to establish it.
    err = outcall->Set_td_BE(1000, 1000);
    err = outcall->Set_bbc(ie_bbc::BCOB_X,
			   ie_bbc::not_clipped,
			   ie_bbc::p2p,
			   10);

    err = outcall->Set_qos_param(ie_qos_param::qos0,ie_qos_param::qos0);
    err = outcall->Set_called_party_num( _called_party );

    // request to setup the call for me
    cout << "Requesting call (id=" << outcall->Get_Call_ID() << ") to be established.\n";
    err = outcall->Establish();

    // we assume request was accepted.  A 'Call_Active' 
    // callback will occur when the call is ready.
    assert( err == ATM_Attributes::success );
  }

  //-------------------------------------------------- call related callbacks
  void Call_Active(ATM_Call& call) {
    cout << "Call (id=" << call.Get_Call_ID() << ") is active.\n";
  }

  //-------------------------------------------------- time related callbacks
  void PeriodicCallback(int code) {
    cout << "Periodic callback code=" << code << endl;
    SEAN_Halt();	// Stops the program immediately.
  }
};

// *** The main program starts here ***
int main(int argc, char** argv) {

  if (argc!=4) {
    cout << "usage: " << argv[0] << " my-name who-to-call host-file\n";
    exit(1);
  }

  HostFile config(argv[3]);
  int sd_port = config.get_port_byname (argv[1]);
  char* who_to_call_nsap_str = config.get_nsap_byname (argv[2]);
  Addr* who_to_call_addr =  newAddr( who_to_call_nsap_str );

  // open a signalling session to sd process, port 'lport'
  ATM_Interface& session = Open_ATM_Interface("/dev/ignored", sd_port);

  // we make an instance of our derived controller object
  client_controller Kontrol(who_to_call_addr);

  // when something happens to the ATM_Interface object 'session',
  // we'd like a callback to the Kontrol server_controller instance.
  // e.g. a 'Boot' callback when 'session' is ready
  // to attend to our signalling needs.
  session.Associate_Controller(Kontrol);

  // hand over control the the API event loop
  SEAN_Run();

  exit(0);
}
