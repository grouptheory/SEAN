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

#ifndef __API_FSM_H__
#define __API_FSM_H__
#ifndef LINT
static char const _API_fsms_h_rcsid_[] =
"$Id: API_fsms.h,v 1.4 1998/08/10 11:54:47 bilal Exp $";
#endif

#include <FW/basics/diag.h>
#include <FW/actors/State.h>
class VisitorType;
class sean_Visitor;
class ATM_Attributes;
class ATM_Leaf;
class ATM_Call;
class ATM_Interface;
class generic_q93b_msg;
class sean_io;

//------------------------------------------------

class API_FSM : public State {
public:

  enum api_fsm_state {
    no_fsm,
    ccd_dead,
    
    call_preparing, 
    call_underspecified__waiting_for_incoming_LIJ,
    call_outgoing_setup__waiting_for_remote,
    call_incoming_setup__waiting_for_local,
    call_p2p_active,
    call_p2mp_active, 
    call_p2mp_active__LIJ_present,
    call_release_initiated,
    call_released,
    
    service_preparing, 
    service_register_requested,
    service_deregister_requested,
    service_active__no_call_present, 
    service_active__call_present, 
    service_released,
    
    LIJ_preparing,
    LIJ_requested__waiting_for_remote,

    leaf_preparing,
    leaf_join_request__waiting_for_local,
    leaf_addparty_requested__waiting_for_remote,
    leaf_dropparty_requested__waiting_for_remote,
    leaf_active,
    leaf_released
  };

protected:
  API_FSM(int id);
  virtual ~API_FSM();

  void mark_FSM_for_deletion(void);
  bool is_FSM_marked_for_deletion(void);

  void set_crv_in_attributes(unsigned long crv);
  void update_attributes(generic_q93b_msg* m);
  void notify_attributes_of_fsm_failure(int code);

  void associate_attributes(ATM_Attributes* attr);
  virtual void set_queue_to_empty(void);
  API_FSM::api_fsm_state get_state(void) const;

  sean_io* get_datapath(void);  // goes and asks the ATM_Attributes, 
                                // or actual derived class

  void change_state_no_callback(API_FSM::api_fsm_state transition_to);
  void change_state_with_callback(API_FSM::api_fsm_state st, 
				  ATM_Call* call, 
				  ATM_Leaf* leaf);

  API_FSM::api_fsm_state    _state;
  int                       _id;
  ATM_Attributes           *_attr;
  bool                      _mark_for_deletion;
  static const VisitorType* _sean_visitor_type;

  friend class ATM_Attributes;
  friend class ATM_Interface;
  friend class ATM_Leaf;
};

#include "Call_FSM.h"
#include "Service_FSM.h"
#include "Leaf_FSM.h"

void Print_API_FSM_State(API_FSM::api_fsm_state s, ostream& os);

#endif



