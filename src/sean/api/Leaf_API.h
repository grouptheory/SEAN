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
 * @version: $Id: Leaf_API.h,v 1.3 1998/08/10 11:54:47 bilal Exp $
 */

#ifndef _LEAF_API_H_
#define _LEAF_API_H_
#ifndef LINT
static char const _Leaf_API_h_rcsid_[] =
"$Id: Leaf_API.h,v 1.3 1998/08/10 11:54:47 bilal Exp $";
#endif

#include "API_fsms.h"
#include "ATM_Attributes.h"

class ATM_Interface;
class Call_Controller;
class Service_Controller;
class ATM_Call;
class ATM_Service;
class ATM_Leaf;
class sean_Visitor;

//-----------------------------------------------

class ATM_Leaf : public ATM_Attributes {
public:

  friend ATM_Leaf* Make_Leaf(ATM_Call& call);
  void Suicide(void);

  int Get_Leaf_ID(void) const;

  enum leaf_status {
    CCD_DEAD            =API_FSM::ccd_dead,
    USER_PREPARING_LEAF =API_FSM::leaf_preparing, 
    INCOMING_LIJ        =API_FSM::leaf_join_request__waiting_for_local,
    ADDPARTY_REQUESTED  =API_FSM::leaf_addparty_requested__waiting_for_remote,
    DROPPARTY_REQUESTED =API_FSM::leaf_dropparty_requested__waiting_for_remote,
    LEAF_READY          =API_FSM::leaf_active, 
    LEAF_RELEASED       =API_FSM::leaf_released 
  };
  leaf_status Status(void) const;

  ATM_Attributes::result Add(void);
  ATM_Attributes::result Drop(void);

  friend ostream& operator << (ostream& os, const ATM_Leaf& s);
  void Print(void) const;

private:
  ATM_Leaf(Outgoing_ATM_Call& call);

  virtual ~ATM_Leaf();
  bool Ready_To_Die(void);

  void state_has_changed(API_FSM::api_fsm_state from, 
			 API_FSM::api_fsm_state to,
			 ATM_Call* newc,
		    ATM_Leaf* newl);

  ATM_Leaf(Outgoing_ATM_Call& call,
	   generic_q93b_msg* incoming_leaf_setup, 
	   int lid);

  void orphaned_by_call(Outgoing_ATM_Call* abandoner);
  Outgoing_ATM_Call*   _owning_call;
  int                  _owning_call_id;

  friend class FSM_ATM_Leaf;
  friend class FSM_ATM_Call;
  friend class Outgoing_ATM_Call;
  friend class Memory_Manager;
};

#endif


