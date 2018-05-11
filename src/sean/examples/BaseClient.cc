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

//* -*- C++ -*-

#include <sean/api/Signalling_API.h>
#include "BaseClient.h"

void SendData(ATM_Call &call)
{

  int NUM_PDU  =  256;
  int BUF_SIZE = 4096;
  
  u_char *buffer = new u_char[BUF_SIZE];
  int buffer_length = BUF_SIZE;
  int bytes_written = 0;

  cout << "Sending DATA" << endl;
  for (int i = 0; i < NUM_PDU; i++)
    bytes_written = call.SendData(buffer, buffer_length);
  // Signal the EOD  with a PDU of length 1
  // if a lot of data is being lost this indicator will likely fail
  bytes_written = call.SendData(buffer, 1);
  delete [] buffer;
  cout << "sent " << NUM_PDU  << " PDUs of "<< BUF_SIZE
       << " bytes for a total of "
       << NUM_PDU * BUF_SIZE/(1024*1024) << " Mbytes " << endl;
}


BaseClient::BaseClient(ATM_Interface &interface, char *called_party, bool p2mp) :
  _interface(interface), _outgoing_call(0), _called_party_number(0),_p2mp(p2mp)
{
  if (called_party != 0)
    _called_party_number = newAddr(called_party);
}


BaseClient::~BaseClient(void)
{
  delete _called_party_number;
}

bool BaseClient::IsP2MP() { return _p2mp;}


void BaseClient::AddParty(Addr *addr)
{
  _leaf_addr = addr;
}

void BaseClient::PeriodicCallback(int code)
{
  ATM_Attributes::result err;
  //  cout << "PeriodicCallback()" << endl;
}


void BaseClient::Boot(ATM_Interface &interface)
{
  cout << "Boot()" << endl;
  CallAgain(interface);
}

void BaseClient::Leaf_Active(ATM_Leaf &leaf, ATM_Call &call)
{
  cout << "Call (id=" << call.Get_Call_ID() << ") ";
  cout << "has an active leaf(id= " << leaf.Get_Leaf_ID() << ")" << endl;
  SendData(call);
  leaf.Set_cause(ie_cause::normal_call_clearing);
  leaf.Drop();
  call.TearDown(); // release the call
  call.Suicide();
  //  SEAN_Halt();
}

void BaseClient::Call_Active(ATM_Call& call)
{
  cout << "Call (id=" << call.Get_Call_ID() << ") is active.\n";

  ATM_Attributes::result err;
  Addr *destination = 0;
  int i = 0;

  cout << "Call_Active(): cref= " << hex << call.Get_crv() << dec << " to ";
  err = call.Get_called_party_num(destination);
  if (err == ATM_Attributes::success) 
    cout << *destination << endl;
  else 
    cout << "nowhere" << endl;


  if(!call.Is_P2MP())
    {
      SendData(call);
      call.TearDown(); // release the call
      call.Suicide();
      SEAN_Halt();
    }
  ATM_Leaf *leaf = 0;
  // Add a leaf now
  if(_leaf_addr)
    {
      leaf = Make_Leaf(call);
      err = leaf->Set_called_party_num(_leaf_addr);
      assert(err == ATM_Attributes::success);
      err = leaf->Add();
      assert(err == ATM_Attributes::success);
    }
}


void BaseClient::Call_Inactive(ATM_Call& call)
{
  ATM_Attributes::result err;
  ATM_Interface &interface = call.Get_interface();

  cout << "Call_Inactive(): cref= " << hex << call.Get_crv() << dec << endl;
  
  _outgoing_call = 0;
  call.Suicide();
  SEAN_Halt();
}

void BaseClient::Incoming_Data(ATM_Call& call)
{
  u_char *buffer = 0;
  int buffer_length = 0;
  int bytes_read = 0;
  int i = 0;

  //  cout << "Data!" << endl;
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


void BaseClient::CallAgain(ATM_Interface &interface)
{
  Outgoing_ATM_Call *call;
  ATM_Attributes::result err;
  Addr *destination = _called_party_number;
  
  call = new Outgoing_ATM_Call(interface);
  if (!call->Associate_Controller(*this))
    {
      cout << "Associate_Controller() failed!  I'm quitting!" << endl;
      exit(1);
    }
  // Set its Attributes to Best Effort, QoS 0, BBC X, Called Party
  // Number as indicated in the constructor.
  err = call->Set_td_BE(353207,0);

  if (err == ATM_Attributes::success)
    {
      if(_p2mp)
	err = call->Set_bbc(ie_bbc::BCOB_X,ie_bbc::not_clipped,ie_bbc::p2mp,10);
      else
	err = call->Set_bbc(ie_bbc::BCOB_X,ie_bbc::not_clipped,ie_bbc::p2p,10);
      if (err == ATM_Attributes::success)
	{
	  err = call->Set_called_party_num(_called_party_number);
	  if (err == ATM_Attributes::success)
	    {
	      err = call->Set_qos_param(ie_qos_param::qos0,ie_qos_param::qos0);
	      if (err == ATM_Attributes::success)
		call->Associate_Controller(*this);
	      else
		{
		  cout << "CallAgain(): error setting qos paramete" << endl;
		  call->Suicide();
		  call = 0;
		}
	    }
	  else
	    {
	      cout << "CallAgain(): error setting called party number" << endl;
	      call->Suicide();
	      call = 0;
	    }
	}
      else
	{
	  cout << "CallAgain(): error setting BBC X" << endl;
	  call->Suicide();
	  call = 0;
	}
    }
  else
    {
      cout << "CallAgain(): error setting Best Effort" << endl;
      call->Suicide();
      call = 0;
    }
  
  if (call != 0)
    {
      cout << "Calling" << endl;
      err = call->Establish();
      if (err == ATM_Attributes::success)
	_outgoing_call = call;
      else
	{
	  cout << "Error!  Establish() failed!" << endl;
	  call->Suicide();
	}
    }
}


