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
static char const _server1_cc_rcsid_[] =
"$Id: server3.cc,v 1.3 1998/08/20 20:25:04 talmage Exp $";
#endif
#include <common/cprototypes.h>

// **************************************************************************
//
// This application is informed whenever ANY incoming connection comes
// in, and upon receiving said notification, it accepts the call.
// These calls are never torn down, and the application assumes a best
// case scenario where all of its requests are granted.  This server
// will echo any data that arrives on an active connection.
//
// NEW: This server notices when a connection becomes inactive.
// **************************************************************************
#include <sean/api/ATM_Interface.h>
#include <sean/api/Call_API.h>
#include <sean/api/Controllers.h>
#include <sean/api/Leaf_API.h>
#include <sean/api/Service_API.h>
#include <sean/daemon/HostFile.h>

class server_controller : public Controller {
public:
  server_controller(void)             { /* nothing perhaps */  }
  virtual ~server_controller(void)    { /* nothing perhaps */  }

private:
  //-------------------------------------------------- interface related callbacks

  // Boot will happen when 'interface' is ready to attend to your signalling needs.
  void Boot(ATM_Interface& interface) {
    cout << "ATM_Interface is ready.\n";

    // make me a service object
    ATM_Service * serv = new ATM_Service(interface);

    // when something happens to the service object 'serv',
    // we'd like a callback to <this> server_controller instance.
    // e.g. a 'Service_Active' callback when 'serv' is 
    // active (ready to receive/send connections).
    serv->Associate_Controller(*this);

    // request to register the service for me.     
    cout << "Requesting service (id=" << serv->Get_Service_ID() << ") to be registered.\n";\
    ATM_Attributes::result err = serv->Register();

    // we assume request was accepted.  A 'Service_Active' 
    // callback will occur when the service is ready.
    assert( err == ATM_Attributes::success );
  }

  //-------------------------------------------------- call related callbacks

  // Call_Active will happen when 'call' is ready to send/receive data.
  void Call_Active(ATM_Call& call) {
    cout << "The call (id=" << call.Get_Call_ID() << ") is now active\n";
  }

  // Call_Inactive will happen when 'call' is released.
  void Call_Inactive(ATM_Call& call) {
    cout << "The call (id=" << call.Get_Call_ID() << ") is now inactive\n";

    // Destroy the call.
    call.Suicide();
  }

  // Incoming_Data will happen when 'call' has data for us to read.
  void Incoming_Data(ATM_Call &call) {
    int pdu_length;
    u_char *buffer;
    int bytes_read;

    //
    // Empty the call's data queue one PDU at a time.  Print each PDU,
    // pretending that each one is a "line" of printable data but not
    // necesarily a zero-terminated string.
    //
    while ((pdu_length = call.Next_PDU_Length()) > 0) {
      buffer = new u_char[pdu_length];
      bytes_read = call.RecvData(buffer, pdu_length);

      for (int i=0; i < bytes_read; i++)
	cout << buffer[i];

      cout << endl;
    }
  }
  //-------------------------------------------------- service related callbacks

  // Service_Active will happen when 'service' is ready to receive calls.
  void Service_Active(ATM_Service& service) {
    cout << "The service (id=" << service.Get_Service_ID() << ") is now active\n";
  }

  // Incoming_Call will happen when 'service' has received an incoming call
  void Incoming_Call(ATM_Service& service) {
    cout << "The service (id=" << service.Get_Service_ID() << 
      ") is notified of an incoming call.\n";

    // get me an Incoming_ATM_Call object that represents the newly arrived call.
    Incoming_ATM_Call* incall = service.Get_Incoming_Call();

    // when something happens to the incoming call object 'incall',
    // we'd like a callback to <this> server_controller instance.
    // e.g. a 'Call_Active' callback when 'incall' is 
    // active (ready to receive/send data).
    incall->Associate_Controller(*this);

    // now accept the incoming call.
    cout << "The service (id=" << service.Get_Service_ID() << 
      ") accepting the incoming call (id=" << incall->Get_Call_ID() << ")\n";
    ATM_Attributes::result err = incall->Accept();

    // we assume success
    assert( err == ATM_Attributes::success );
  }
};

// *** The main program starts here ***
int main(int argc, char** argv) {
  if (argc!=3) {
    cout << "usage: " << argv[0] << " my-name host-file\n";
    exit(1);
  }

  HostFile config(argv[2]);
  int sd_port = config.get_port_byname (argv[1]);
  
  // open a signalling session to sd process, port 'lport'
  ATM_Interface& session = Open_ATM_Interface("/dev/ignored", sd_port);

  // we make an instance of our derived controller object
  server_controller Kontrol;

  // when something happens to the ATM_Interface object 'session',
  // we'd like a callback to the Kontrol server_controller instance.
  // e.g. a 'Boot' callback when 'session' is ready
  // to attend to our signalling needs.
  session.Associate_Controller(Kontrol);

  // hand over control the the API event loop
  SEAN_Run();

  exit(0);
}
