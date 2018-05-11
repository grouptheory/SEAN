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

//
// Simple Call to a Service. 
//
#ifndef LINT
static char const _caller_cc_[] =
"$Id: caller.cc,v 1.3 1998/09/28 12:31:44 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <sean/api/ATM_Interface.h>
#include <sean/api/Call_API.h>
#include <sean/api/Controllers.h>
#include <sean/api/Leaf_API.h>
#include <sean/api/Service_API.h>
#include <sean/daemon/HostFile.h>

class simple_call : public Controller {
public:

  simple_call(Addr* called_party) {
    _called_party = called_party;
  }
  virtual ~simple_call(void) {
  }

private:
  Addr* _called_party;

  void PeriodicCallback(int code) {
    cout << "PeriodicCallback\n";
  }
  
  void Boot(ATM_Interface& interface) {
    cout << "Boot\n";

    Outgoing_ATM_Call* call = new Outgoing_ATM_Call(interface);
    // we should handle all the state changes for this call
    call->Associate_Controller(*this);

    ATM_Attributes::result err;

    // fill in the required IEs
    err = call->Set_td_BE(353207, 16777215);
    err = call->Set_bbc(ie_bbc::BCOB_X,
			ie_bbc::not_clipped,
			ie_bbc::p2p,
			10);

    err = call->Set_qos_param(ie_qos_param::qos0,ie_qos_param::qos0);
    call->Set_called_party_num( _called_party );

    assert (call->Establish() != ATM_Attributes::failure);
  }

 //-------------------------------------------------- call related callbacks
  void Call_Active(ATM_Call& call) {
    cout << "Call_Active\n";
    // successful
    // can send data here or in PeriodicCallback
    
  }
  void Call_Inactive(ATM_Call& call) {
    cout << "Call_Inactive\n";
    // failure
  }
  void Incoming_Data(ATM_Call& call) {
    cout << "Incoming_Data\n";
  }

};


void usage(char *program)
{
  cout <<"Usage: "<< program << " port host "<< endl;
  exit(1);
}
int main(int argc, char** argv) {

  DiagLevel("api",DIAG_ENV);
  DiagLevel("ipc",DIAG_ENV);

  HostFile nameserver("/etc/xti_hosts");

  int port = atoi(argv[1]);
  
  ATM_Interface& sig = Open_ATM_Interface("/dev/ignored", port);

  if (argv[2] == 0) {
      usage(argv[0]);
  }
  
  char *nsap = nameserver.get_nsap_byname(argv[2]);
  if (nsap == 0) {
    cout << "Host: "<< argv[0] << " not in /etc/xti_hosts" <<endl;
    exit(1);
  }
  
//   Addr* addr =
//     newAddr("47.0005.80.ffde00.0000.0000.0b01.0020480691b9.00"); // uno
  Addr* addr = newAddr(nsap);
  
  cout << "calling "<<argv[2]<< " at " << nsap << endl;
  
  simple_call my_simple_call(addr);
  sig.Associate_Controller(my_simple_call);

  SEAN_Run();
}
