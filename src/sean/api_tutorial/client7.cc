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
"$Id: client7.cc,v 1.3 1998/09/28 12:31:44 bilal Exp $";
#endif
#include <common/cprototypes.h>

// **************************************************************************
// This application makes a call to a command-line specified hosts.
// The application assumes a best case scenario where its request
// succeeds.  It registers a periodic callback for 10 seconds from
// now, upon which it will cancel the 10 second periodic callback and
// send some data on the connection. It registers periodic callback
// for 20 seconds from now, upon which it will tear down the call but
// not exit.  When the call becomes active, it adds the second
// command-line specified host to the point-to-multipoint call.
//
// NEW: It registers a callback for 15 seconds from now that drops the
// initial leaf.
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
  Addr* _second_leaf_addr;		// leaf we intend to add
  Outgoing_ATM_Call * _outcall;		// a call to _called_party
  ATM_Leaf *_second_leaf;		// the second leaf of the call

public:
  client_controller(Addr* who_to_call, Addr* who_to_add) : 
    _outcall(0), _second_leaf(0) { 
    _called_party = who_to_call;
    _second_leaf_addr = who_to_add;
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

    //
    // Register another periodic callback for 10 seconds from now.
    //
    RegisterPeriodicCallback(2, 10.0);

    //
    // Register another periodic callback for 15 seconds from now.
    //
    RegisterPeriodicCallback(3, 15.0);

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

    // Create a leaf on this point-to-multipoint call
    _second_leaf = Make_Leaf(call);

    // Supply the ATM address of the leaf
    ATM_Attributes::result err = 
      _second_leaf->Set_called_party_num(_second_leaf_addr);

    assert(err == ATM_Attributes::success);

    cout << "Made leaf(id=" << _second_leaf->Get_Leaf_ID() 
	 << ") to " << *_second_leaf_addr << endl;

    // Add the leaf to the call.
    err = _second_leaf->Add();
    cout << "Added leaf to call" << endl;

    assert(err == ATM_Attributes::success);
  }

  void Call_Inactive(ATM_Call& call) {
    cout << "Call (id=" << call.Get_Call_ID() << ") is inactive.\n";

    // Destroy the call
    call.Suicide();
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

  //-------------------------------------------------- time related callbacks
  void PeriodicCallback(int code) {
    char *buffer = "I am Sam.  Sam I am.  I do not like green eggs and ham.";
    u_long buffer_length = strlen(buffer);
    u_long bytes_written = 0;
    ATM_Attributes::result err;
    ATM_Leaf *initial_leaf = 0;

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

    case 2:		// Cancel the callback and send some data
      CancelPeriodicCallback(code);
      bytes_written = _outcall->SendData((u_char *)buffer, buffer_length);
      cout << "Wrote " << bytes_written << " out of " << buffer_length << endl;
      break;

    case 3:		// Drop the initial leaf
      CancelPeriodicCallback(code);
      initial_leaf = _outcall->Get_Initial_Leaf();

      cout << "Dropping Call (id=" << _outcall->Get_Call_ID() << ") ";
      cout << " initial leaf(id=" << initial_leaf->Get_Leaf_ID() << ")" 
	   << endl;

      // First, give the reason for dropping the leaf
      err = initial_leaf->Set_cause(ie_cause::normal_call_clearing);
      assert( err == ATM_Attributes::success );

      // Second, drop the leaf
      err = initial_leaf->Drop();
      assert( err == ATM_Attributes::success );

      break;

    default:
      cout << "Unknown code " << code << endl;
      break;
    }
  }
};

// *** The main program starts here ***
int main(int argc, char** argv) {

  if (argc!=5) {
    cout << "usage: " << argv[0] 
	 << " my-name who-to-call who-to-add host-file\n";
    exit(1);
  }

  HostFile config(argv[4]);
  int sd_port = config.get_port_byname (argv[1]);
  char* who_to_call_nsap_str = config.get_nsap_byname (argv[2]);
  Addr* who_to_call_addr =  newAddr( who_to_call_nsap_str );
  char* who_to_add_nsap_str =  config.get_nsap_byname (argv[3]);
  Addr* who_to_add_addr = newAddr( who_to_add_nsap_str );

  // open a signalling session to sd process, port 'lport'
  ATM_Interface& session = Open_ATM_Interface("/dev/ignored", sd_port);

  // we make an instance of our derived controller object
  client_controller Kontrol(who_to_call_addr, who_to_add_addr);

  // when something happens to the ATM_Interface object 'session',
  // we'd like a callback to the Kontrol server_controller instance.
  // e.g. a 'Boot' callback when 'session' is ready
  // to attend to our signalling needs.
  session.Associate_Controller(Kontrol);

  // hand over control the the API event loop
  SEAN_Run();

  exit(0);
}
