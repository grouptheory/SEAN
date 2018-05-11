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
 * File: Signalling_API.h
 * @author: bilal
 * @version: $Id: Service_API.h,v 1.2 1998/08/06 00:48:50 bilal Exp $
 */

#ifndef _SERVICE_API_H_
#define _SERVICE_API_H_
#ifndef LINT
static char const _Service_API_h_rcsid_[] =
"$Id: Service_API.h,v 1.2 1998/08/06 00:48:50 bilal Exp $";
#endif

#include "API_fsms.h"
#include "ATM_Attributes.h"

class ATM_Interface;
class Call_Controller;
class Service_Controller;
class ATM_Call;
class ATM_Incoming_Call;
class ATM_Service;
class ATM_Leaf;
class sean_Visitor;

//-----------------------------------------------

class ATM_Service : public ATM_Attributes {
public:

  ATM_Service(ATM_Interface& interface);
  void Suicide(void);

  int Get_Service_ID(void) const;

  enum service_status {
    CCD_DEAD                       =API_FSM::ccd_dead,
    USER_PREPARING_SERVICE         =API_FSM::service_preparing, 
    SERVICE_REGISTER_INITIATED     =API_FSM::service_register_requested, 
    SERVICE_DEREGISTER_INITIATED   =API_FSM::service_deregister_requested, 
    SERVICE_ACTIVE                 =API_FSM::service_active__no_call_present, 
    INCOMING_CALL_PRESENT          =API_FSM::service_active__call_present, 
    SERVICE_INACTIVE               =API_FSM::service_released
  };

  ATM_Attributes::result Register(void);
  ATM_Attributes::result Deregister(void);
  Incoming_ATM_Call* Get_Incoming_Call(void);
  service_status Status(void) const;

protected:

  list<Incoming_ATM_Call*> _incoming_call_queue;

  friend ostream& operator << (ostream& os, const ATM_Service& s);
  void Print(void) const;

private:

  virtual ~ATM_Service();
  bool Ready_To_Die(void);

  virtual void state_has_changed(API_FSM::api_fsm_state from, 
				 API_FSM::api_fsm_state to,
				 ATM_Call* newc,
				 ATM_Leaf* newl);

  friend class Service_Controller;
  friend class Memory_Manager;
};

#endif


