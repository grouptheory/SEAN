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

/* -*- C++ -*-
 * File: SimpleCaller.cc
 * Version: $Id: SimpleCaller.cc,v 1.12 1998/09/28 12:32:27 bilal Exp $
 *
 * Purpose: SimpleCaller is an example of a Controller that calls an
 * ATM_Service.  It places one call and waits up to 3 seconds for the
 * call to become active.  If the call becomes active, it writes data
 * to the service and waits up to 3 more seconds for data from the
 * service before hanging up.
 *
 * If the call doesn't become active, SimpleCaller hangs up and exits.
 */


#include <sean/api/Signalling_API.h>
#include "SimpleCaller.h"

SimpleCaller::SimpleCaller(ATM_Interface &interface,
			   char *called_party) :
  _interface(interface), _outgoing_call(0), _called_party_number(0)
{
  if (called_party != 0) {
    _called_party_number = newAddr(called_party);
  }
  cout << "OK, I'm initialized, baby." << endl;
}


SimpleCaller::~SimpleCaller(void)
{
  delete _called_party_number;
  cout << "OK, I'm destructed, baby." << endl;
}


void SimpleCaller::PeriodicCallback(int code)
{
  ATM_Attributes::result err;

  cout << "PeriodicCallback()" << endl;

  switch (code) {
  case 1:	// actions for 1
  case 2:	// actions for 2
    cout << "Interrupted with code " << code << " -- ";
    if (_outgoing_call != 0) {
      cout << "Hanging up" << endl;
      err = _outgoing_call->Set_cause(ie_cause::normal_call_clearing);
      err = _outgoing_call->TearDown();
    } else {
      SEAN_Halt();
    }
    CancelPeriodicCallback(code);
  break;

  default:
  break;
  }
}


//
// OK, the interface is up, so we can make calls on it.  Call the
// address given to the constructor.
//
void SimpleCaller::Boot(ATM_Interface &interface)
{
  cout << "Boot()" << endl;

  CallAgain(interface);
}


//
// Yay!  The service accepted our call.  Send some data to the service.
//
void SimpleCaller::Call_Active(ATM_Call& call)
{
  ATM_Attributes::result err;
  u_char *buffer = 0;
  u_long buffer_length = 0;
  int bytes_read = 0;
  int bytes_written = 0;
  Addr *destination = 0;
  int i = 0;

  CancelPeriodicCallback(1);
  cout << "Call_Active(): cref= " << hex << call.Get_crv() << dec << " to ";
  err = call.Get_called_party_num(destination);
  if (err == ATM_Attributes::success) 
    cout << *destination << endl;
  else 
    cout << "nowhere" << endl;

  // Notice that we don't delete destination.  It's not ours to
  // delete.

  cout << "Call_Active(): Writing to it." << endl;

  // Send some data to the other side
  buffer = new u_char[10];
  buffer_length = 10;
  for (i = 0; i < buffer_length; i++) buffer[i] = 'A';

  bytes_written = call.SendData(buffer, buffer_length);

  delete [] buffer;

  cout << "SendData() tried to write " << buffer_length << endl;
  cout << "SendData() actually wrote " << bytes_written << endl;

  //
  // Wait up to 6 seconds for data from the service.
  // Hangup regardless.
  //
  RegisterPeriodicCallback(2, 6.0);
}


//
// Somebody released the call.  It could be the called party, the
// calling pary (i.e. me!), or the network.  It doesn't matter who,
// because it's reported here regardless.
void SimpleCaller::Call_Inactive(ATM_Call& call)
{
  ATM_Attributes::result err;
  ATM_Interface &interface = call.Get_interface();

  cout << "Call_Inactive(): cref= " << hex << call.Get_crv() << dec << endl;
  
  _outgoing_call = 0;

  // Destroy the call
  call.Suicide();
  
  RegisterPeriodicCallback(2, 1.0);
}


//
// Read and print the data that the service sends us.
//
// Inside any callback, the API guarantees that no new data will appear.
// This callback could do three things:
//
// Option 1: It could read all of the data available, clearing the
// Incoming Data event.
//
// Option 2: It could choose not to read any data or to read some of
// the data available.  Either choice leaves the Incoming Data event
// on the event queue.  The next time the callback is invoked, there
// might be more data than there is now.
//
// Option 3: It could use Drain() to tell the API to throw away all of
// the unread data.  This clears the Incoming Data event.  Drain() is
// compatible with Option 2, above.
//
void SimpleCaller::Incoming_Data(ATM_Call& call)
{
  u_char *buffer = 0;
  int buffer_length = 0;
  int bytes_read = 0;
  int i = 0;

  cout << "Data!" << endl;
  
  //
  // Read all of the data in the queue.
  // Print each pdu on its own line.
  //
  while ((buffer_length = call.Next_PDU_Length()) > 0) {
    buffer = new u_char[buffer_length];

    bytes_read =call.RecvData(buffer, buffer_length);

    cout << "Expected: " << buffer_length << " bytes" << endl;
    cout << "Read    : " << bytes_read << " bytes" << endl;
    cout << "Data    : ";
    for (i=0; i < bytes_read; i++) cout << buffer[i];
    cout << endl;
    cout << endl;

    delete [] buffer;
  }
}


//
// Place a call to the address given to the constructor.  Use
// RegisterPeriodicCallback() to set a 3-second timer.  When the timer
// expires, PeriodicCallback() will hang up the call.
//
void SimpleCaller::CallAgain(ATM_Interface &interface)
{
  Outgoing_ATM_Call *call;
  ATM_Attributes::result err;
  Addr *destination = _called_party_number;
  

  // Create an Outgoing_ATM_Call to some remote host
  call = new Outgoing_ATM_Call(interface);

  // Associate the Outgoing_ATM_Call with this SimpleCaller.
  // The SimpleCaller will keep track of the Outgoing_ATM_Call.
  if (!call->Associate_Controller(*this)) {
    cout << "Associate_Controller() failed!  I'm quitting!" << endl;
    exit(1);
  }

  // Set its Attributes to Best Effort, QoS 0, BBC X, Called Party
  // Number as indicated in the constructor.
  err = call->Set_td_BE(353207, 16777215);

  if (err == ATM_Attributes::success) {

    err = call->Set_bbc(ie_bbc::BCOB_X,
			ie_bbc::not_clipped,
			ie_bbc::p2p,
			10);

    if (err == ATM_Attributes::success) {

      err = call->Set_called_party_num(_called_party_number);

      if (err == ATM_Attributes::success) {
	err = call->Set_qos_param(ie_qos_param::qos0,ie_qos_param::qos0);

	if (err == ATM_Attributes::success) {

	  call->Associate_Controller(*this);

	} else {
	  cout << "CallAgain(): error setting qos paramete" << endl;
	  call->Suicide();
	  call = 0;
	}
      } else {
	cout << "CallAgain(): error setting called party number" << endl;
	call->Suicide();
	call = 0;
      }
    } else {
      cout << "CallAgain(): error setting BBC X" << endl;
      call->Suicide();
      call = 0;
    }
  } else {
    cout << "CallAgain(): error setting Best Effort" << endl;
    call->Suicide();
    call = 0;
  }

  // Place the call if and only if we were able to set all of the
  // ATM_Attributes.  Schedule a PeriodicCallback for 6 seconds from
  // now.  When it expires, release the call.
  if (call != 0) {
    cout << "Calling" << endl;
    err = call->Establish();
    if (err == ATM_Attributes::success) {
      _outgoing_call = call;
    } else {
      cout << "Error!  Establish() failed!" << endl;
      call->Suicide();
    }
  }

  RegisterPeriodicCallback(1, 6.0);
}
