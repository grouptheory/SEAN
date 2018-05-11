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
static char const _linktst_cc_rcsid_[] =
"$Id: example.cc,v 1.13 1998/09/28 12:31:16 bilal Exp $";
#endif

#include <FW/basics/Conduit.h>
#include <FW/basics/diag.h>
#include <codec/uni_ie/ie.h>

#include "ATM_Interface.h"
#include "Signalling_API.h"

#include "Controllers.h"
#include "example.h"


#define MAXCOUNT 100


My_Call_Manager::My_Call_Manager(ATM_Interface& i, bool silent) 
  : _interface(&i), _counter(0), _silent(silent) {
}

My_Call_Manager::~My_Call_Manager() {
}

void My_Call_Manager::PeriodicCallback(int code) {
  cout << "My_Call_Manager: Call. Periodic Callback #" << _counter <<", code " << code << endl;
  _counter++;
  if (_counter> MAXCOUNT) {
    cout << "My_Call_Manager: Cancelling Periodic Callback.\n";
    CancelPeriodicCallback( _MYCODE );
    cout << "My_Call_Manager: Releasing Calls...\n";

    list_item li;
    while (!_outgoing_calls.empty()) {
      li = _outgoing_calls.first();
      Outgoing_ATM_Call* call = _outgoing_calls.inf(li);
      call->Set_cause( ie_cause::normal_call_clearing );
      call->TearDown();
      _outgoing_calls.del_item(li);
    }

    cout << "My_Call_Manager: Done.\n";
  }
}

void My_Call_Manager::Boot(ATM_Interface& interface) {
  if (_silent) return;

  ATM_Attributes::result err;
  Addr* addr;

  //--------------------------------------------------------
  Outgoing_ATM_Call* call = new Outgoing_ATM_Call(*_interface);
  call->Associate_Controller(*this);

  err = call->Set_td_BE(1000,1000);
  err = call->Set_bbc(ie_bbc::BCOB_X,
		      ie_bbc::not_clipped,
		      ie_bbc::p2mp,
		      10);

  addr = newAddr("0x47.0005.80.ffde00.0000.0000.0104.0020480620eb.01"); // uno
  err = call->Set_called_party_num(addr); 
  err = call->Set_qos_param(ie_qos_param::qos0,ie_qos_param::qos0);

  call->Associate_Controller(*this);
    
  err = call->Establish();
  ATM_Call::call_status c_status = call->Status();
  cout << "My_Call_Manager: Call status" << c_status << "." << endl;

  _outgoing_calls.append(call);
}

void My_Call_Manager::Call_Active(ATM_Call& call) {
  cout << "My_Call_Manager: Call is active\n";
  cout << "My_Call_Manager: Registering Periodic Callback at 1 second interval.\n";
  RegisterPeriodicCallback( _MYCODE , 1.0);
}

void My_Call_Manager::Incoming_LIJ(ATM_Call& call) {
  cout << "My_Call_Manager: Incoming LIJ\n";
}

void My_Call_Manager::Call_Inactive(ATM_Call& call) {
  cout << "My_Call_Manager: Call is Inactive\n";
}

void My_Call_Manager::Incoming_Data(ATM_Call& call) {
  cout << "My_Call_Manager: Incoming Data\n";
}


void My_Call_Manager::Leaf_Active(class ATM_Leaf & leaf, class ATM_Call & c) {
  cout << "My_Call_Manager: Leaf Active\n";
}

void My_Call_Manager::Leaf_Inactive(class ATM_Leaf & leaf, class ATM_Call & c) {
  cout << "My_Call_Manager: Leaf Inactive\n";
}

//-----------------------------------------------------------------

My_Service_Manager::My_Service_Manager(ATM_Interface& i)
  : _interface(&i), _counter(0) {
}

My_Service_Manager::~My_Service_Manager() {
}

void My_Service_Manager::PeriodicCallback(int code) {
  cout << "My_Service_Manager: Service. Periodic Callback #" << _counter <<", code " << code << endl;
  _counter++;
  if (_counter> MAXCOUNT) {
    cout << "My_Service_Manager: Cancelling Periodic Callback.\n";
    CancelPeriodicCallback( _MYCODE );

    cout << "My_Service_Manager: Deregistering Service...\n";
    _service->Deregister();
    cout << "My_Service_Manager: Done.\n";
  }
}

void My_Service_Manager::Boot(ATM_Interface& interface) {
  ATM_Attributes::result err;
  Addr* addr;

  // ------------------------------------------------------
  ATM_Service* service = new ATM_Service(*_interface);
  err = service->Set_td_BE(1000,1000);
  err = service->Set_bbc(ie_bbc::BCOB_X,
			 ie_bbc::not_clipped,
			 ie_bbc::p2mp,
			 10);

  addr = newAddr("0x47.0005.80.ffde00.0000.0000.0104.0020480620eb.01"); // uno
  err = service->Set_called_party_num(addr);
  err = service->Set_qos_param(ie_qos_param::qos0,ie_qos_param::qos0);

  service->Associate_Controller(*this);

  service->Register();
  ATM_Service::service_status s_status = service->Status();
  cout << "My_Service_Manager: service status " << s_status << "." << endl;

  _service = service;
}

void My_Service_Manager::Service_Active(ATM_Service& service) {
  cout << "My_Service_Manager: Service Active\n";
  cout << "My_Service_Manager: Registering Periodic Callback at 1 second interval.\n";
  RegisterPeriodicCallback( _MYCODE, 1.0);
}

void My_Service_Manager::Service_Inactive(ATM_Service& service) {
  cout << "My_Service_Manager: Service Inactive\n";
}

void My_Service_Manager::Incoming_Call(ATM_Service& service) {
  cout << "My_Service_Manager: Incoming Call\n";
  Incoming_ATM_Call* c = service.Get_Incoming_Call();
  if (c) {
    c->Associate_Controller(*this);
    cout << "My_Service_Manager: Controller has been set\n";

    cout << "My_Service_Manager: Beginning establish on incoming call...";
    cout << "CallID = " << c->Get_Call_ID() << endl;
    c->Accept();
    if (c->Status() == ATM_Call::CALL_P2P_READY) {
      cout << "My_Service_Manager: Call ready\n";
    }
    cout << "My_Service_Manager: Establish of incoming call complete\n";
  }
  else cout << "My_Service_Manager: Call ptr is ZERO\n";
}

//-----------------------------------------------------------------

int main(int argc, char** argv) {

  DiagLevel("ipc",DIAG_DEBUG);
  DiagLevel("api",DIAG_DEBUG);
  DiagLevel("ccd",DIAG_DEBUG);

  if ((argc!=3)&&(argc!=2)) {
    cout << "Usage: " << argv[0] << " s|c ipc-port" << endl;
    exit(1);
  }
  char type, machine[255];
  int port=atoi(argv[2]);

  char ch = argv[1][0];
  if (ch == 'c') {
    VisPipe("client.output");
    My_Call_Manager *cmanager;
    ATM_Interface& host_nic = Open_ATM_Interface (argv[1],port);
    cmanager = new My_Call_Manager(host_nic);
    host_nic.Associate_Controller(*cmanager);
  }


  if (ch == 's') {
    VisPipe("server.output");

    My_Call_Manager *cmanager;
    ATM_Interface& host_nic = Open_ATM_Interface (argv[1],port);
    cmanager = new My_Call_Manager(host_nic, true); // silent

    My_Service_Manager *smanager;
    smanager = new My_Service_Manager(host_nic);
    host_nic.Associate_Controller(*smanager);
  }

  SEAN_Run();
  exit(0);
}


