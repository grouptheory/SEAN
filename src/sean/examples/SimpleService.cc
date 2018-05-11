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
 * File: SimpleService.cc
 * Version: $Id: SimpleService.cc,v 1.9 1998/09/28 12:32:27 bilal Exp $
 *
 * Purpose: It's an example of a Controller that provides an
 * ATM_Service.  The service accepts one call at a time.  It waits up
 * to 3 seconds for input from the call.  Then it releases the call.
 * If it receives any input from the call, it cancels the 3-second
 * timer, sends some data to the call, and enables a 2-second timer
 * that releases the call.
 */


#include <sean/api/Signalling_API.h>
#include "SimpleService.h"

SimpleService::SimpleService(ATM_Interface &interface) :
  _interface(interface), _incoming_call(0), _incoming_call_reference(0)
{
  cout << "OK, I'm initialized, baby." << endl;
}


SimpleService::~SimpleService(void)
{
  cout << "OK, I'm destructed, baby." << endl;
}


void SimpleService::PeriodicCallback(int code)
{
  ATM_Attributes::result err;

  cout << "PeriodicCallback()" << endl;

  switch (code) {
  case 1:	// actions for 1
  case 2:	// actions for 2
    //
    // Hangup on the incoming call.  When the call goes inactive,
    // start accepting incoming calls again.
    //
    if (_incoming_call_reference != 0) {
      cout << "Hanging up on call " 
	   << hex <<_incoming_call->Get_crv() << dec;

      if (code == 2)
	cout << " after receiving and sending data";

      cout << endl;

      err = _incoming_call->Set_cause(ie_cause::normal_call_clearing);

      if (err == ATM_Attributes::success)
	cout << "Set cause to Normal Call Clearing" << endl;
      else cout << "Failed to set the cause to Normal Call Clearing" << endl; 

      err = _incoming_call->TearDown();
      if (err != ATM_Attributes::success)
	cout << "Error! TearDown() failed!!" << endl;
    }

    CancelPeriodicCallback(code);
  break;

  default:
  break;
  }
}


//
// The interface is ready.
//
// Creates the service, sets its attributes, and registers it with the
// signalling daemon.  When the signalling daemon has decided what to
// do with the service, he'll notify us through Service_Active() or
// Service_Inactive().
//
void SimpleService::Boot(ATM_Interface &interface)
{
  ATM_Service *service;
  ATM_Attributes::result err;

  cout << "Boot()" << endl;

  // Create an ATM_Service on the interface
  service = new ATM_Service(interface);

  // Associate the ATM_Service with this SimpleService.
  // This SimpleService will keep track of the ATM_Service.
  if (!service->Associate_Controller(*this)) {
    cout << "Associate_Controller() failed!  I'm quitting!" << endl;
    exit(1);
  }

  //
  // Set the service's ATM_Attributes.
  //
  // The signalling daemon will tell this SimpleController about
  // incoming calls that match this set of ATM_Attributes.
  // We'll learn about it when Incoming_Call() is called.
  //
  err = service->Set_td_BE(353207, 16777215);

  if (err == ATM_Attributes::success) {

    err = service->Set_bbc(ie_bbc::BCOB_X,
			ie_bbc::not_clipped,
			ie_bbc::p2p,
			10);

    if (err == ATM_Attributes::success) {

	err = service->Set_qos_param(ie_qos_param::qos0,ie_qos_param::qos0);

	if (err != ATM_Attributes::success) {
	  cout << "Boot(): error setting qos parameter" << endl;
	  service->Suicide();
	  service = 0;
	}
    } else {
      cout << "Boot(): error setting BBC X" << endl;
      service->Suicide();
      service = 0;
    }
  } else {
    cout << "Boot(): error setting Best Effort" << endl;
    service->Suicide();
    service = 0;
  }

  // Register the ATM_Service with the CCD
  // 
  // We'll know that the registration succeeded if Service_Active() is
  // called.
  //
  // We'll know that the registration failed if Service_Inactive() is
  // called.  Registration usually fails because there is already a
  // service with the same set of ATM_Attributes.
  //
  if (service != 0) {
    err = service->Register();

    if (err != ATM_Attributes::success) {
      cout << "Register() failed!  I'm quitting!" << endl;
      exit(1);
    }
  }
}


//
// One of the calls that we accepted in Incoming_Call() is ready to
// talk to.
//
void SimpleService::Call_Active(ATM_Call& call)
{
  ATM_Attributes::result err;
  u_char *buffer;
  u_long buffer_length;
  int bytes_read;
  int bytes_written;
  Addr *source = 0;
  ie_calling_party_num::presentation_indicator source_presentation;
  ie_calling_party_num::screening_indicator source_screening;

  cout << "Call_Active(): cref= " 
       << hex << call.Get_crv() << dec << " from ";

  err = call.Get_calling_party_num(source, source_presentation, 
				   source_screening);

  if (err == ATM_Attributes::success) 
    cout << *source << endl;
  else // No calling party number
    cout << "nowhere" << endl;

  // Notice that we don't delete source.
  // It's not ours to delete.
}


//
// One of the calls that we learned about in Incoming_Call() is not
// active.  Possibly, we rejected it in Incoming_Call() by calling
// TearDown().  Maybe we accepted it, exchanged data with it, and then
// called TearDown().  Maybe the caller released the call himself.
//
void SimpleService::Call_Inactive(ATM_Call& call)
{
  ATM_Attributes::result err;

  cout << "Call_Inactive(): cref= " << hex << call.Get_crv() << dec;

  if (_incoming_call_reference == call.Get_crv()) {
    _incoming_call = 0;
    _incoming_call_reference = 0;
  }

  // Destroy the call
  call.Suicide();
}


//
// Some call has data for us.  We'll read the data and print it.
// We'll send the caller some data, a string of 10 letter 'B's.
// Then we'll enable PeriodicCallback with code 2.  When it
// expires in 2 seconds, we'll hang up on the caller.
//
//
// Inside any callback, the API guarantees that no new data will appear.
// This callback could do three things:
//
// Option 1: It could read all of the data available, clearing the
// Data Present event.
//
// Option 2: It could choose not to read any data or to read some of
// the data available.  Either choice leaves the Data Present event on
// the event queue.  The next time the callback is invoked, there
// might be more data than there is now.
//
// Option 3: It could use Drain() to tell the API to throw away all of
// the unread data.  This clears the Data Present event.  Drain() is
// compatible with Option 2, above.
//
void SimpleService::Incoming_Data(ATM_Call& call)
{
  u_char *buffer = 0;
  int buffer_length = 0;
  int bytes_read = 0;
  int bytes_written = 0;
  int i = 0;
  cout << "Data!" << endl;
  
  CancelPeriodicCallback(1);

  //
  // Read all of the data in the queue.
  // Print each pdu on its own line.
  //
  while ((buffer_length = call.Next_PDU_Length()) > 0) {
    buffer = new u_char[buffer_length];

    bytes_read = call.RecvData(buffer, buffer_length);

    cout << "Expected: " << buffer_length << " bytes" << endl;
    cout << "Read    : " << bytes_read << " bytes" << endl;
    cout << "Data    : ";
    for (i=0; i < bytes_read; i++) cout << buffer[i];
    cout << endl;
    cout << endl;

    delete [] buffer;
  }

  // Send some data to the other side
  buffer_length = 10;
  buffer = new u_char[buffer_length];

  for (i = 0; i < buffer_length; i++) buffer[i] = 'B';
  bytes_written = call.SendData(buffer, buffer_length);

  delete [] buffer;

  cout << "SendData() tried to write " << buffer_length << endl;
  cout << "SendData() actually wrote " << bytes_written << endl;

  // We could hang up now.  If the calling party us using our API, then 
  // the Controller responsible for the call will be notified of the
  // incoming data even though we've released the call here.
  //
  // Instead of hanging up now, we'll set a timer for two seconds
  // from now.  When the timer expires, we'll hang up.  We do this
  // because we can.
  //
  RegisterPeriodicCallback(2, 2.0);	// code 2 in 2.0 seconds
}


//
// If the Signalling Daemon accepted the service we registered in
// Boot(), he'll inform us by calling Service_Active().
//
void SimpleService::Service_Active(ATM_Service& service)
{
  cout << "Service_Active()" << endl;
  cout << service << endl;
}


//
// When the service we registered in Boot() is not active, either
// because we deregistered it with Deregister() or because the
// Signalling Daemon refused it, the Signalling Daemon notifies us by
// calling Service_Inactive().
//
void SimpleService::Service_Inactive(ATM_Service& service)
{
  cout << "Service_Inactive()" << endl;
  cout << service << endl;

  // Destroy the service
  service.Suicide();
}


/*
 * Somebody called the service we registered in Boot().
 *
 * Accept any incoming call if we're not processing one already.
 * We could handle an arbitrary number of calls at a time, but
 * for this simple example, we're just dealing with one call.
 *
 * If the caller doesn't send us any data within 3 seconds, we will
 * hang up.  PeriodicCallback with code 1 indicates a "silent caller".
 *
 * See Incoming_Data() to learn what we do if the caller sends us some.
 */
void SimpleService::Incoming_Call(ATM_Service& service)
{
  Incoming_ATM_Call *incoming_call;
  ATM_Attributes::result err;

  cout << "Incoming_Call()" << endl;

  // Ask the service for the Incoming_ATM_Call
  incoming_call = service.Get_Incoming_Call();

  // Associate the call with this SimpleService
  incoming_call->Associate_Controller(*this);

  // Decide what to do about it
  bool accept_this_call;

  accept_this_call = (_incoming_call == 0);

  if (accept_this_call) {
    RegisterPeriodicCallback(1, 3.0);	// Hangup in 3 seconds.

    _incoming_call_reference = incoming_call->Get_crv();

    // Tell the call that we accept it
    err = incoming_call->Accept();

    _incoming_call = incoming_call;

    if (err != ATM_Attributes::success) {
      cout << "Error!  Accept() failed!" << endl;
      err = incoming_call->Set_cause(ie_cause::temporary_failure);
      err = incoming_call->TearDown();
      _incoming_call = 0;
      _incoming_call_reference = 0;
    }
  } else {
    // Tell the call that we reject it
    cout << "Rejecting the call" << endl;

    err = incoming_call->Set_cause(ie_cause::user_busy);

    if (err == ATM_Attributes::success)
      cout << "Set cause to User Busy" << endl;
    else cout << "Failed to set the cause to User Busy" << endl;

    err = incoming_call->TearDown();

    if (err != ATM_Attributes::success) {
      cout << "TearDown() failed!  I'm quitting!" << endl;
      exit(1);
    }
  }
}


