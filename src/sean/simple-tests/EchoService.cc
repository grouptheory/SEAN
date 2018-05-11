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
static char const _EchoService_cc_rcsid_[] =
"$Id: EchoService.cc,v 1.3 1998/08/06 04:06:16 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/basics/Conduit.h>
#include <FW/basics/diag.h>

#include "BaseController.h"
#include "EchoService.h"
#include "PrintFunctions.h"

#ifdef ECHO_S
#define ECHO_SCREEN true
#else
#define ECHO_SCREEN false
#endif

EchoService::EchoService(const char * diag_key, 
			 ATM_Interface & interface,
			 double lifetime) :
  AcceptingOneService(diag_key, interface, lifetime)
{
  RegisterPeriodicCallback( 666, lifetime );
}

EchoService::~EchoService(void) { }

void EchoService::PeriodicCallback(int code)
{
  if ( code == 666 ) {
    diag(_diag_key, DIAG_INFO, "Program is exiting normally ...\n");
    exit( 1 );
  } else if ( code == 100 ) {
    // ping call
    if (!_accepted_call) return;

    if (_accepted_call->Status() == ATM_Call::CALL_RELEASED) {
      _accepted_call = 0;
      diag(_diag_key, DIAG_INFO, "The call was released somehow ...\n");
      return;
    }
  } else
    diag(_diag_key, DIAG_INFO, "%s received an unsupported callback with code %d.\n", _diag_key, code);
}

void EchoService::Incoming_Call(ATM_Service & service)
{
  ATM_Attributes::result err;

  DIAG(_diag_key, DIAG_INFO, 
       cout << endl << "EchoService (Info): "
            << "Incoming call on Service #";
       PrintBHLIUser(service);
       cout << endl);

  // Sorry only one call at a time
  if (_accepted_call) return;

  diag(_diag_key, DIAG_INFO, "About to Get_Incoming_Call()\n");

  _accepted_call = service.Get_Incoming_Call();

  if (_accepted_call) {
    diag(_diag_key, DIAG_INFO, 
         "Associating the call with this controller.\n");

    if (_accepted_call->Associate_Controller(*this)) {
      diag(_diag_key, DIAG_INFO, 
           "Associate_Controller() Succeeded!\n");

      DIAG(_diag_key, DIAG_INFO, 
           cout << "EchoService (Info): "
                <<  "beginning establish on incoming call..."
                << _accepted_call << endl);

      diag(_diag_key, DIAG_INFO, "About to accept the call\n");

      err = _accepted_call->Accept();

      DIAG(_diag_key, DIAG_INFO, 
           cout << "EchoService (Info): ";
           PrintResult(err);
           cout << endl);

      diag(_diag_key, DIAG_INFO, 
           "establish of incoming call complete\n");
      // Check for data every second
      //      RegisterPeriodicCallback( 100, 1.0 );
    } else {
      diag(_diag_key, DIAG_INFO, "Associate_Controller() failed\n");
    }
  }  else {
    diag(_diag_key, DIAG_INFO, "call ptr is 0\n");
  }
}


void EchoService::Incoming_Data(ATM_Call &call)
{
  int wr_len = 0;
  int    rd_len = -1;
  int   exp_len = 0;
  u_char *buf = 0;


  //
  // Drain the data queue one PDU at a time.
  //
  while ((exp_len = call.Next_PDU_Length()) > 0) {

    buf = new u_char[exp_len];

    if ((rd_len = call.RecvData(buf, exp_len)) < 0) {
      diag(_diag_key, DIAG_INFO, 
	   "Error reading data from call, return rval is %d.\n", rd_len);
    } else
      diag(_diag_key, DIAG_INFO, "Read %d bytes from call ...\n", rd_len);

    if (rd_len > 0) {
      if (ECHO_SCREEN) {
	cout << "Received '" << hex;
	for (int i = 0; i < rd_len; i++)
	  cout << (int)buf[i] << " ";
	cout << "'" << dec << endl;
      } else {
	diag(_diag_key, DIAG_INFO, 
	     "Echoing the data back to the client ...\n");
	wr_len = call.SendData(buf, rd_len);
	diag(_diag_key, DIAG_INFO, "Tried to write %d\n", rd_len);
	diag(_diag_key, DIAG_INFO, "Actually wrote %d\n", wr_len);
      }
    }

    delete [] buf;
  }
}

