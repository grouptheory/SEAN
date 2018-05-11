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
"$Id: client8.cc,v 1.4 1998/09/28 12:31:44 bilal Exp $";
#endif
#include <common/cprototypes.h>

// **************************************************************************
// This application makes a point-to-multipoint call to a command-line
// specified host.  The application assumes a best case scenario where
// its request succeeds.  It registers periodic callback for 20
// seconds from now, upon which it will tear down the call.  When the
// call becomes inactive, it exits.  No data is exchanged.
//
// NEW: When a leaf asks to join the point-to-multipoint call, it is
// added to the call.  Use this with client 9.
// **************************************************************************
#include <sean/api/ATM_Interface.h>
#include <sean/api/Call_API.h>
#include <sean/api/Controllers.h>
#include <sean/api/Leaf_API.h>
#include <sean/api/Service_API.h>
#include <sean/daemon/HostFile.h>

class client_controller : public Controller {
private:
  Addr* _called_party;			// who we intend to call
  Outgoing_ATM_Call * _outcall;		// a call to _called_party
  ATM_Leaf *_second_leaf;		// the second leaf of the call

public:
  client_controller(Addr* who_to_call) : 
    _outcall(0), _second_leaf(0) { 
    _called_party = who_to_call;
  }

  virtual ~client_controller(void)    { /* nothing perhaps */  }

private:
  //-------------------------------------------------- interface related callbacks

  // Boot will happen when 'interface' is ready to attend to your signalling needs.
  void Boot(ATM_Interface& interface) {
    cout << "ATM_Interface is ready.\n";

    // make me a call object
    _outcall = new Outgoing_ATM_Call(interface);

    // when something happens to the call object '_outcall',
    // we'd like a callback to <this> client_controller instance.
    // e.g. a 'Call_Active' callback when '_outcall' is 
    // active (ready to receive/send data).
    _outcall->Associate_Controller(*this);

    ATM_Attributes::result err;

    //
    // Register a periodic callback for 20 seconds from now.
    //
    RegisterPeriodicCallback(1, 20.0);

    // We must set the QoS parameters for the call prior to 
    // attempting to establish it.
    err = _outcall->Set_td_BE(1000, 1000);

    // This is a point-to-multipoint call.  Observe param #2
    err = _outcall->Set_bbc(ie_bbc::BCOB_X,
			    ie_bbc::not_clipped,
			    ie_bbc::p2mp,
			    10);

    err = _outcall->Set_qos_param(ie_qos_param::qos0,ie_qos_param::qos0);
    err = _outcall->Set_called_party_num( _called_party );

    //
    // Set the LIJ call identifier so that leaves can find this p2mp
    // call.
    //
    err = _outcall->Set_LIJ_call_id(42);

    // request to setup the call for me
    cout << "Requesting call (id=" << _outcall->Get_Call_ID() << ") to be established.\n";
    err = _outcall->Establish();

    // we assume request was accepted.  A 'Call_Active' 
    // callback will occur when the call is ready.
    assert( err == ATM_Attributes::success );
  }

  //-------------------------------------------------- call related callbacks
  void Call_Active(ATM_Call& call) {
    cout << "Call (id=" << call.Get_Call_ID() << ") is active.\n";
  }

  void Call_Inactive(ATM_Call& call) {
    cout << "Call (id=" << call.Get_Call_ID() << ") is inactive.\n";

    // Destroy the call
    call.Suicide();

    // Exit
    SEAN_Halt();
  }

  //-------------------------------------------------- leaf related callbacks
  void Leaf_Active(ATM_Leaf &leaf, ATM_Call &call) {
    cout << "Call (id=" << call.Get_Call_ID() << ") ";
    cout << "has an active leaf(id=" << leaf.Get_Leaf_ID() << ")" << endl;
  }

  void Leaf_Inactive(ATM_Leaf &leaf, ATM_Call &call) {
    cout << "Call (id=" << call.Get_Call_ID() << ") ";
    cout << "has an inactive leaf(id=" << leaf.Get_Leaf_ID() << ")" << endl;

    // Destroy the leaf.
    leaf.Suicide();
  }

  void Incoming_LIJ(Outgoing_ATM_Call& call) {
    ATM_Attributes::result err;

    cout << "A leaf wants to join Call (id=" << call.Get_Call_ID() << ").\n";
    _second_leaf = call.Get_Leaf_Join_Request();

    cout << "Adding leaf (id=" << _second_leaf->Get_Leaf_ID() << ")" << endl;
    err = _second_leaf->Add();
  }

  //-------------------------------------------------- time related callbacks
  void PeriodicCallback(int code) {
    ATM_Attributes::result err;

    cout << "Periodic callback code=" << code << endl;

    switch (code) {
    case 1:		// Tear down the call
      CancelPeriodicCallback(code);

      // First, give the reason for releasing the call
      err = _outcall->Set_cause(ie_cause::normal_call_clearing);
      assert( err == ATM_Attributes::success );

      // Second, release the call
      cout << "Tearing down call (id=" << _outcall->Get_Call_ID() << ").\n";
      err = _outcall->TearDown();
      assert( err == ATM_Attributes::success );

      _outcall = 0;
      break;

    default:
      cout << "Unknown code " << code << endl;
      break;
    }
  }
};

// *** The main program starts here ***
int main(int argc, char** argv) {

  if (argc!=4) {
    cout << "usage: " << argv[0] 
	 << " my-name who-to-call host-file\n";
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
