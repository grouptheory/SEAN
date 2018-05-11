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

// -*- C++ -*-
#ifndef LINT
static char const _echo_client_cc_rcsid_[] =
"$Id: echo_client.cc,v 1.11 1998/08/06 04:06:21 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/basics/Conduit.h>
#include <sean/daemon/HostFile.h>
#include "PrintFunctions.h"
#include "echo_client.h"
#include "EchoService.h"

#ifdef ECHO_S
#define SERVICE_ECHO true
#else
#define SERVICE_ECHO false
#endif

echo_client::echo_client(ATM_Interface & interface, double stop_time,
			 char *called_party) 
  : BaseClient("echo_client", interface, called_party), 
    _call_active(false), _call(0)
{ 
  if (stop_time == -1.0)
    stop_time = 10.0;

  RegisterPeriodicCallback( 666, stop_time );
}

echo_client::~echo_client() { }

void echo_client::Call_Active(ATM_Call & call)
{
  diag(_diag_key, DIAG_INFO, 
       "The call has become active ...\n");
  _call        = (Outgoing_ATM_Call *)&call;
  _call_active = true;

  // data processing callback
  RegisterPeriodicCallback( 100, 1.0 );
}

void echo_client::Call_Inactive(ATM_Call & call)
{
  if (_call_active)
    diag(_diag_key, DIAG_INFO,
	 "The call has become inactive, the test was a success.  Exiting ...\n");
  else
    diag(_diag_key, DIAG_INFO,
	 "The call has become inactive, the test was a failure.  Exiting ...\n");

  exit( 1 );
}

void echo_client::PeriodicCallback(int code)
{
  u_char buf[1024];
  int rd_len = 0;

  if (code == 666) {
    diag(_diag_key, DIAG_INFO, "%s has exited normally, the test has failed.\n", _diag_key);
    exit(0);
  } else if (code == 100) {
    if (!_call) return;
   
    // write some misc data to the call
    sprintf((char *)buf, "Lalalalalalalalalalalalalalalalalala");
    if ((rd_len = _call->SendData(buf, strlen((char *)buf))) < 
	strlen((char *)buf)) 
      diag(_diag_key, DIAG_INFO, 
	   "Error writing data to the call, return code is %d.\n", rd_len);
    else
      diag(_diag_key, DIAG_INFO, 
	   "Successfully wrote some data to the call: %d.\n", rd_len);
  } else
    diag(_diag_key, DIAG_INFO, 
	 "%s received an unsupported code %d.\n", _diag_key, code);
}


void echo_client::Incoming_Data(ATM_Call& call) {

  DIAG(_diag_key, DIAG_INFO, 
       cout << endl;
       cout << _diag_key << " (Info): ";
       cout << "Incoming Data to BHLI #";
       PrintBHLIUser(call);
       cout << endl; );

  int bytes_expected = 0;
  int    rd_len = -1;
  u_char *buf = 0;

  while ((bytes_expected = call.Next_PDU_Length()) > 0) {
    // read and write data to the call
    buf = new u_char[bytes_expected];

    if ((rd_len = _call->RecvData(buf, bytes_expected)) < 0) 
      diag(_diag_key, DIAG_INFO, 
	   "Error reading from the call, return code is %d.\n", rd_len);
    else if (rd_len > 0) {
      // Print out the data to the screen
      cout << "Expected: " << bytes_expected << endl;
      cout << "Read    : " << rd_len << endl;
      cout << "Received '";
      for (int i = 0; i < rd_len; i++)
	cout << hex << (int)buf[i] << " ";
      cout << "'" << dec <<  endl;
    }

  }
}



void Usage(char *name)
{
  cerr << "usage: " << name
       << " s my-name [host-file [stoptime [visualizer-pipe]]]" 
       << endl;
  cerr << "usage: " << name
       << " c my-name server-name [host-file [stoptime [visualizer-pipe]]]" 
       << endl;
}


int StartClient(int argc, char **argv)
{
  int answer = 0;
  double stop_time = 10.0;

  if (argc >= 4) {
    char * host_file_name = "../daemon/host.file";

    if ((argc >= 5) && (strlen(argv[4]) > 0))
      host_file_name = argv[4];

    if (argc >= 6)
      stop_time = atof(argv[5]);

    HostFile name_service(host_file_name);
    int port = name_service.get_port_byname(argv[2]);

    if (port > 0) {
      ATM_Interface &interface = Open_ATM_Interface("/dev/foo", port);
      echo_client *caller = 0;
      char *called_party = 0;

      called_party = name_service.get_nsap_byname(argv[3]);

      if (called_party != 0) {
	cout << "called party name = " << argv[3] 
	     << " ATM Address  = " << called_party << endl;

	caller = new echo_client(interface, stop_time, called_party);
	interface.Associate_Controller(*caller);
    
	if (argc >= 7) {
	  cout << "vispipe = " << argv[6] << endl;
	  VisPipe(argv[6]);
	} else {
	  cout << "vispipe = echo_client_client.output" << endl;
	  VisPipe("echo_client_client.output");
	}
	
	SEAN_Run();

      } else {
	cout << argv[0] << ": Can't find host " << argv[3] << endl;
	answer = 1;
      }
    } else {
      cout << argv[0] << ": Can't find host " << argv[2] 
	   << " or can't find port file" << endl;
      answer = 1;
    }
  } else {
    Usage(argv[0]);
    answer = 1;
  } 
  return answer;
}

int StartServer(int argc, char **argv)
{
  int answer = 0;
  double stop_time = 10.0;
  char * host_file_name = "../daemon/host.file";
  
  if ((argc >= 4) && (strlen(argv[3]) > 0))
    host_file_name = argv[3];

  if (argc >= 5)
    stop_time = atof(argv[4]);

  HostFile name_service(host_file_name);
  int port = name_service.get_port_byname(argv[2]);

  if (port > 0) {
    ATM_Interface &interface = Open_ATM_Interface("/dev/una0", port);
    EchoService *service = 
      new EchoService("echo_client", interface, stop_time);

    interface.Associate_Controller(*service);

    if (argc >= 5) {
      cout << "vispipe = " << argv[5] << endl;
      VisPipe(argv[4]);
    } else {
      cout << "vispipe = echo_client_server.output" << endl;
      VisPipe("echo_client_server.output");
    }

    SEAN_Run();

  } else {
    cout << argv[0] << ": Can't find host " << argv[2]
	 << " or can't find port file" << endl;
    answer = 1;
  }

  return answer;
}


int main(int argc, char **argv)
{
  int answer = 0;
  EchoService * service = 0;
  echo_client * caller = 0;

  DiagLevel("api", DIAG_DEBUG);
  DiagLevel("echo_client", DIAG_DEBUG);
  DiagLevel("ipc", DIAG_ENV);


  if (argc >= 3) {
    if (argv[1][0] == 'c') answer = StartClient(argc, argv);
    else if (argv[1][0] == 's') answer = StartServer(argc, argv);
    else { 
      Usage(argv[0]);
      answer = 1;
    }
  } else {
    Usage(argv[0]);
    answer = 1;
  }

  return answer;
}


