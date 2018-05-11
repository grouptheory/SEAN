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
static char const _BaseController_cc_rcsid_[] =
"$Id: BaseController.cc,v 1.9 1998/10/14 17:55:50 marsh Exp $";
#endif
#include <common/cprototypes.h>

#include "BaseController.h"
#include "PrintFunctions.h"

BaseController::BaseController(const char * diag_key, 
			       ATM_Interface &interface) : 
  _interface(interface)
{
  _diag_key = strdup(diag_key);
}


BaseController::~BaseController(void)
{
  free(_diag_key);
}


void BaseController::PeriodicCallback(int code)
{
  DIAG(_diag_key, DIAG_ERROR, 
       cout << endl;
       cout << _diag_key << " (Error): ";
       cout << "BaseController::PeriodicCallback(code=" << code;
       cout << endl; );
}


void BaseController::Boot(void)
{
}


void BaseController::Call_Active(ATM_Call& call) {
  DIAG(_diag_key, DIAG_ERROR, 
       cout << endl;
       cout << _diag_key << " (Error): ";
       cout << "Call Active to BHLI #";
       PrintBHLIUser(call);
       cout << endl; );
}


void BaseController::Incoming_LIJ(ATM_Call& call) {
  DIAG(_diag_key, DIAG_ERROR, 
       cout << endl;
       cout << _diag_key << " (Error): ";
       cout << "Incoming LIJ to BHLI #";
       PrintBHLIUser(call);
       cout << endl; );
}


void BaseController::Call_Inactive(ATM_Call& call) {
  DIAG(_diag_key, DIAG_ERROR, 
       cout << endl;
       cout << _diag_key << " (Error): ";
       cout << "Call Inactive to BHLI #";
       PrintBHLIUser(call);
       cout << endl; );
}


void BaseController::Incoming_Data(ATM_Call& call) {
  DIAG(_diag_key, DIAG_ERROR, 
       cout << endl;
       cout << _diag_key << " (Error): ";
       cout << "Incoming Data to BHLI #";
       PrintBHLIUser(call);
       cout << endl; );
}


void BaseController::Leaf_Active(ATM_Leaf &leaf, ATM_Call& call) {
  DIAG(_diag_key, DIAG_ERROR, 
       cout << endl;
       cout << _diag_key << " (Error): ";
       cout << "Leaf Active to BHLI #";
       PrintBHLIUser(call);
       cout << endl; );
}


void BaseController::Leaf_Inactive(ATM_Leaf &leaf, ATM_Call& call) {
  DIAG(_diag_key, DIAG_ERROR, 
       cout << endl;
       cout << _diag_key << " (Error): ";
       cout << "Leaf Inactive to BHLI #";
       PrintBHLIUser(call);
       cout << endl; );
}


void BaseController::Service_Active(ATM_Service& service) {
  DIAG(_diag_key, DIAG_ERROR, 
       cout << endl;
       cout << _diag_key << " (Error): ";
       cout << "Service Active to BHLI #";
       PrintBHLIUser(service);
       cout << endl; );
}


void BaseController::Service_Inactive(ATM_Service& service) {
  DIAG(_diag_key, DIAG_ERROR, 
       cout << endl;
       cout << _diag_key << " (Error): ";
       cout << "Service Inactive to BHLI #";
       PrintBHLIUser(service);
       cout << endl; );
}


void BaseController::Incoming_Call(ATM_Service& service) {
  DIAG(_diag_key, DIAG_ERROR, 
       cout << endl;
       cout << _diag_key << " (Error): ";
       cout << "Incoming Call to BHLI #";
       PrintBHLIUser(service);
       cout << endl; );
}


Outgoing_ATM_Call *BaseController::NewCall(void)
{
  ATM_Attributes::result err;
  Addr* addr;
  Outgoing_ATM_Call* call = new Outgoing_ATM_Call(_interface);

  err = call->Set_td_BE(353207, 16777215);

  if (err == ATM_Attributes::success) {

    err = call->Set_bbc(ie_bbc::BCOB_X, ie_bbc::not_clipped,ie_bbc::p2p,10);

    if (err == ATM_Attributes::success) {

	err = call->Set_qos_param(ie_qos_param::qos0,ie_qos_param::qos0);

	if (err == ATM_Attributes::success) {

	  call->Associate_Controller(*this);

	} else {
	  diag(_diag_key, DIAG_INFO, 
	       "NewCall(): error setting qos parameter\n");
	  call->Suicide();
	  call = 0;
	}
    } else {
      diag(_diag_key, DIAG_INFO, 
	   "NewCall(): error setting BBC X\n");
      call->Suicide();
      call = 0;
    }
  } else {
    diag(_diag_key, DIAG_INFO, 
	 "NewCall(): error setting Best Effort\n");
    call->Suicide();
    call = 0;
  }
  return call;
}


Outgoing_ATM_Call *BaseController::NewCall(Addr *called_party_atm_address)
{
  ATM_Attributes::result err;
  Outgoing_ATM_Call* call = NewCall();

  if (call != 0) {

    err = call->Set_called_party_num(called_party_atm_address);

    if (err != ATM_Attributes::success) {
      diag(_diag_key, DIAG_INFO, 
	   "NewCall(): error setting called party number\n");
      call->Suicide();
      call = 0;
    }
  }

  return call;
}


Outgoing_ATM_Call *BaseController::NewCall(char *called_party_atm_address)
{
  ATM_Attributes::result err;
  Addr* addr;
  Outgoing_ATM_Call *call = 0;

  addr = newAddr(called_party_atm_address);

  if (addr != 0) {

    call = NewCall(addr);

  }

  return call;
}


Outgoing_ATM_Call *BaseController::NewCall(char *called_party_atm_address, 
					   int bhli)
{
  ATM_Attributes::result err;
  Outgoing_ATM_Call *call = NewCall(called_party_atm_address);

  if (call != 0) {
    diag(_diag_key, DIAG_INFO, 
	 "Setting bhli user to %d\n", bhli);

    err = call->Set_bhli_user(bhli);

    if (err != ATM_Attributes::success) {
      diag(_diag_key, DIAG_INFO, 
	   "NewCall(): error setting bhli user\n");
      call->Suicide();
      call = 0;
    }
  }

  return call;
}


Outgoing_ATM_Call *BaseController::NewCall(Addr *called_party_atm_address, 
					   int bhli)
{
  ATM_Attributes::result err;
  Outgoing_ATM_Call *call = NewCall(called_party_atm_address);

  if (call != 0) {
    diag(_diag_key, DIAG_INFO, 
	 "Setting bhli user to %d\n", bhli);

    err = call->Set_bhli_user(bhli);

    if (err != ATM_Attributes::success) {
      diag(_diag_key, DIAG_INFO, 
	   "NewCall(): error setting bhli user\n");
      call->Suicide();
      call = 0;
    }
  }

  return call;
}


Outgoing_ATM_Call *BaseController::NewCall(char *called_party_atm_address, 
					   char *bhli)
{
  ATM_Attributes::result err;
  Outgoing_ATM_Call *call = NewCall(called_party_atm_address);

  if (call != 0) {
    diag(_diag_key, DIAG_INFO, 
	 "Setting bhli user to %s\n", bhli);

    err = call->Set_bhli_user((unsigned char *)bhli, 
			      (int)(strlen(bhli)));

    if (err != ATM_Attributes::success) {
      diag(_diag_key, DIAG_INFO, 
	   "NewCall(): error setting bhli user\n");
      call->Suicide();
      call = 0;
    }
  }

  return call;
}


Outgoing_ATM_Call *BaseController::NewCall(Addr *called_party_atm_address, 
					   char *bhli)
{
  ATM_Attributes::result err;
  Outgoing_ATM_Call *call = NewCall(called_party_atm_address);

  if (call != 0) {
    diag(_diag_key, DIAG_INFO, 
	 "Setting bhli user to %s\n", bhli);

    err = call->Set_bhli_user((unsigned char *)bhli, 
			      (int)(strlen(bhli)));

    if (err != ATM_Attributes::success) {
      diag(_diag_key, DIAG_INFO, 
	   "NewCall(): error setting bhli user\n");
      call->Suicide();
      call = 0;
    }
  }

  return call;
}


ATM_Service *BaseController::NewService(char *called_party_atm_address)
{
  ATM_Attributes::result err;
  Addr* addr;
  ATM_Service* service = new ATM_Service(_interface);

  err = service->Set_td_BE(353207, 16777215);

  if (err == ATM_Attributes::success) {

    err = service->Set_bbc(ie_bbc::BCOB_X,ie_bbc::not_clipped,ie_bbc::p2p,10);

    if (err == ATM_Attributes::success) {

      //
      // A service need not have a called party address.
      //
      if (called_party_atm_address != 0) {
	addr = newAddr(called_party_atm_address);
      
	err = service->Set_called_party_num(addr);
      
      } else err = ATM_Attributes::success;

      if (err == ATM_Attributes::success) {
	err = service->Set_qos_param(ie_qos_param::qos0,ie_qos_param::qos0);

	if (err == ATM_Attributes::success) {

	  service->Associate_Controller(*this);

	} else {
	  diag(_diag_key, DIAG_INFO, 
	       "NewService(): error setting qos parameter\n");
	  service->Suicide();
	  service = 0;
	}
      } else {
	diag(_diag_key, DIAG_INFO, 
	     "NewService(): error setting called party number\n");
	service->Suicide();
	service = 0;
      }
    } else {
      diag(_diag_key, DIAG_INFO, 
	   "NewService(): error setting BBC X\n");
      service->Suicide();
      service = 0;
    }
  } else {
    diag(_diag_key, DIAG_INFO, 
	 "NewService(): error setting Best Effort\n");
    service->Suicide();
    service = 0;
  }

  return service;
}


ATM_Service *BaseController::NewService(char *called_party_atm_address, 
					int bhli)
{
  ATM_Attributes::result err;
  ATM_Service *service = NewService(called_party_atm_address);

  if (service != 0) {
    diag(_diag_key, DIAG_INFO, 
	 "Setting bhli user to %d\n", bhli);

    err = service->Set_bhli_user(bhli);

    if (err != ATM_Attributes::success) {
      diag(_diag_key, DIAG_INFO, 
	   "NewService(): error setting bhli user\n");
      service->Suicide();
      service = 0;
    }
  }

  return service;
}


ATM_Service *BaseController::NewService(char *called_party_atm_address, 
					   char *bhli)
{
  ATM_Attributes::result err;
  ATM_Service *service = NewService(called_party_atm_address);

  if (service != 0) {
    diag(_diag_key, DIAG_INFO, 
	 "Setting bhli user to %s\n", bhli);

    err = service->Set_bhli_user((unsigned char *)bhli, 
			      (int)(strlen(bhli)));

    if (err != ATM_Attributes::success) {
      diag(_diag_key, DIAG_INFO, 
	   "NewService(): error setting bhli user\n");
      service->Suicide();
      service = 0;
    }
  }

  return service;
}



void BaseController::RegisterService(ATM_Service *service)
{
  ATM_Attributes::result err;

  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Registering the service";
       cout << endl; );

  err = service->Register();

  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Result of Register(): ";
       PrintResult(err);
       cout << endl; );

  if (err == ATM_Attributes::success) {
    DIAG(_diag_key, DIAG_INFO, 
	 cout << _diag_key << " (Info): ";
	 cout << "For service #";
	 PrintBHLIUser(*service);
	 cout <<"  REGISTER TEST PASSED";
	 cout << endl; );

  } else {
    DIAG(_diag_key, DIAG_INFO, 
	 cout << _diag_key << " (Info): ";
	 cout << "For service #";
	 PrintBHLIUser(*service);
	 cout <<"  Aborting.  REGISTER TEST FAILED";
	 cout << endl; );

    exit(1);
  }
}


void BaseController::DeregisterService(ATM_Service *service)
{
  ATM_Attributes::result err;

  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Deregistering service #";
       PrintBHLIUser(*service);
       cout << endl; );

  err = service->Deregister();
  
  DIAG(_diag_key, DIAG_INFO, 
       cout << _diag_key << " (Info): ";
       cout << "Result of Deregister(): ";
       PrintResult(err);
       cout << endl; );

  if (err == ATM_Attributes::success) {
    DIAG(_diag_key, DIAG_INFO, 
	 cout << _diag_key << " (Info): ";
	 cout << "For service #";
	 PrintBHLIUser(*service);
	 cout << " DEREGISTER TEST PASSED";
	 cout << endl; );

  } else {
    DIAG(_diag_key, DIAG_INFO, 
	 cout << _diag_key << " (Info): ";
	 cout << "For service #";
	 PrintBHLIUser(*service);
	 cout << " DEREGISTER TEST FAILED";
	 cout << endl; );
     
    exit(1);
  }
}
