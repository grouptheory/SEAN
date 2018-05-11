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
 * File: BaseService.cc
 * Version: $Id: BaseService.cc,v 1.5 1999/03/30 19:57:59 marsh Exp marsh $
 *
 * Purpose: It's an example of a Controller that provides an
 * ATM_Service.  The service accepts one call at a time.  It waits up
 * to 3 seconds for input from the call.  Then it releases the call.
 * If it receives any input from the call, it cancels the 3-second
 * timer, sends some data to the call, and enables a 2-second timer
 * that releases the call.
 */

#include <sys/time.h>
#include <sean/api/Signalling_API.h>
#include "BaseService.h"

// simple timing macros 
#define GET_TIME(x)	(gettimeofday(&x, (struct timezone *) NULL))
#define SUB_TIME(s,e)	((double) ((e.tv_sec - s.tv_sec) + \
			  ((e.tv_usec - s.tv_usec) / 1000000.0)))

BaseService::BaseService(ATM_Interface &interface) :
  _interface(interface), _incoming_call(0), _incoming_call_reference(0)
{
}


BaseService::~BaseService(void)
{
}


void BaseService::PeriodicCallback(int code)
{
  ATM_Attributes::result err;
  //  cout << "PeriodicCallback()" << endl;
}


//
// The interface is ready.
//
// Creates the service, sets its attributes, and registers it with the
// signalling daemon.  When the signalling daemon has decided what to
// do with the service, he'll notify us through Service_Active() or
// Service_Inactive().
//
void BaseService::Boot(ATM_Interface &interface)
{
  ATM_Service *service;
  ATM_Attributes::result err;

  // create a service
  service = new ATM_Service(interface);

  // attach it to a controller
  if (!service->Associate_Controller(*this))
    {
      cout << "Associate_Controller() failed!  I'm quitting!" << endl;
      exit(1);
    }
  
  // set matching attributes: NONE for this example -- Accept all calls


  // register this service
  if (service != 0)
    {
      err = service->Register();
      if (err != ATM_Attributes::success)
	{
	  cout << "Register() failed!  I'm quitting!" << endl;
	  exit(1);
	}
    }
}


void BaseService::Call_Active(ATM_Call& call)
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
}

static int pdu_cnt = 0;
static struct timeval start, end;  
void Report_Transfer();


void BaseService::Call_Inactive(ATM_Call& call)
{
  ATM_Attributes::result err;
  cout << "Call_Inactive(): cref= " << hex << call.Get_crv() << dec <<endl;
  if (_incoming_call_reference == call.Get_crv())
    {
      _incoming_call = 0;
      _incoming_call_reference = 0;
    }
  if (pdu_cnt) Report_Transfer();

  call.Suicide();
}

void BaseService::Incoming_Data(ATM_Call& call)
{
  u_char *buffer = 0;
  int buffer_length = 8192;
  int bytes_read = 0;
  int i = 0;
  //  cout << "Data!" << endl;
  buffer = new u_char[buffer_length];
  int k = 0;
  if(pdu_cnt == 0)
    GET_TIME(start);
  while ((buffer_length = call.Next_PDU_Length()) > 0)
    {
      bytes_read = call.RecvData(buffer, buffer_length);
      if(bytes_read == 1)
	Report_Transfer();
      pdu_cnt++;
    }
  delete [] buffer;
}

void Report_Transfer()
{
  GET_TIME(end);
  int total_bytes = pdu_cnt * 8192;
  double elapsed = SUB_TIME(start,end);
  printf("Received %d PDUs (%d bytes)\n", pdu_cnt,total_bytes);
  printf("Elapsed time = %.3f seconds, throughput = %.3f Mbits/sec\n\n",
	 elapsed, (total_bytes * 8.0 / (1024*1024.0)) / elapsed);	  
  pdu_cnt= 0;
}


//
// If the Signalling Daemon accepted the service we registered in
// Boot(), he'll inform us by calling Service_Active().
//
void BaseService::Service_Active(ATM_Service& service)
{
  cout << "Service is Registration done." << endl;
}


//
// When the service we registered in Boot() is not active, either
// because we deregistered it with Deregister() or because the
// Signalling Daemon refused it, the Signalling Daemon notifies us by
// calling Service_Inactive().
//
void BaseService::Service_Inactive(ATM_Service& service)
{
  cout << "Service is deregistered !" << endl;
  // Destroy the service
  service.Suicide();
}


// incoming call matched the service
void BaseService::Incoming_Call(ATM_Service& service)
{
  Incoming_ATM_Call *incoming_call;
  ATM_Attributes::result err;
  cout << "Incoming_Call()" << endl;

  // Ask the service for the Incoming_ATM_Call
  incoming_call = service.Get_Incoming_Call();

  // Associate the call with this BaseService
  incoming_call->Associate_Controller(*this);

  // Decide what to do about it
  bool accept_this_call;

  accept_this_call = (_incoming_call == 0);

  if (accept_this_call)
    {
      _incoming_call_reference = incoming_call->Get_crv();
      err = incoming_call->Accept();
      _incoming_call = incoming_call;
      if (err != ATM_Attributes::success)
	{
	  cout << "Error!  Accept() failed!" << endl;
	  err = incoming_call->Set_cause(ie_cause::temporary_failure);
	  err = incoming_call->TearDown();
	  _incoming_call = 0;
	  _incoming_call_reference = 0;
	}
    }
  else 
    {
      // Tell the call that we reject it
      cout << "Rejecting the call" << endl;
      err = incoming_call->Set_cause(ie_cause::user_busy);
      if (err == ATM_Attributes::success)
	cout << "Set cause to User Busy" << endl;
      else
	cout << "Failed to set the cause to User Busy" << endl;
      err = incoming_call->TearDown();
      if (err != ATM_Attributes::success)
	{
	  cout << "TearDown() failed!  I'm quitting!" << endl;
	  exit(1);
	}
    }
}



