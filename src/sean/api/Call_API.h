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
 * @version: $Id: Call_API.h,v 1.10 1998/08/25 21:10:51 mountcas Exp $
 */
#ifndef _CALL_API_H_
#define _CALL_API_H_

#ifndef LINT
static char const _Call_API_h_rcsid_[] =
"$Id: Call_API.h,v 1.10 1998/08/25 21:10:51 mountcas Exp $";
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
class sean_io;

//-----------------------------------------------
class ATM_Leaf;

class ATM_Call : public ATM_Attributes {
public:

  void Suicide(void);

  enum call_status {
    CCD_DEAD                =API_FSM::ccd_dead,
    USER_PREPARING_CALL     =API_FSM::call_preparing, 
    CALL_WAITING_FOR_LIJ    =API_FSM::call_underspecified__waiting_for_incoming_LIJ,
    INCOMING_CALL_WAITING   =API_FSM::call_incoming_setup__waiting_for_local,
    OUTGOING_CALL_INITIATED =API_FSM::call_outgoing_setup__waiting_for_remote,
    CALL_P2P_READY          =API_FSM::call_p2p_active, 
    CALL_P2MP_READY         =API_FSM::call_p2mp_active, 
    CALL_LEAF_JOIN_PRESENT  =API_FSM::call_p2mp_active__LIJ_present,
    CALL_RELEASE_INITIATED  =API_FSM::call_release_initiated, 
    CALL_RELEASED           =API_FSM::call_released,

    LIJ_PREPARING           =API_FSM::LIJ_preparing,
    LIJ_INITIATED           =API_FSM::LIJ_requested__waiting_for_remote
  };

  int Get_Call_ID(void) const;

  call_status Status(void) const;

  ATM_Attributes::result TearDown(void);

  virtual int SendData(u_char * buf, u_long length);
  int RecvData(u_char * buf, u_long maxlength);
  void Drain(void);
  int Next_PDU_Length(void);

  friend ostream & operator << (ostream & os, const ATM_Call & s);
  virtual void Print(void) const;

  enum call_type {
    unknown,
    incoming,
    outgoing,
    leaf_initiated
  };

  call_type Type(void);

protected:

  call_type  _t;

  void make_datapath(int QoS, bool data_sending);

  ATM_Call(ATM_Interface& interface, call_type t);

  ATM_Call(ATM_Interface& interface, 
	   generic_q93b_msg* incoming_setup, 
	   int cid, 
	   call_type t);

  virtual ~ATM_Call();
  bool Ready_To_Die(void);

  virtual bool cleanup_leaves_at_time_of_destructor(void);
  virtual int  cleanup_leaves_at_time_of_teardown(void);

  sean_io* get_datapath(void);
  void kill_datapath(void);

  virtual bool leaf_is_dying(ATM_Leaf* leaf);
  virtual void induced_teardown(void);

private:
  sean_io*              _datapath;
  bool                  _data_sending;
  static ATM_Interface* _current_interface;
  static ATM_Call*      _current_call;

  virtual void state_has_changed(API_FSM::api_fsm_state from, 
				 API_FSM::api_fsm_state to,
				 ATM_Call* newc,
				 ATM_Leaf* newl);
  friend class Controller;
  friend class ATM_Leaf;
  friend class FSM_ATM_Service;
  friend class Memory_Manager;
  friend class sean_io;
};

//-----------------------------------------------

class Incoming_ATM_Call : public ATM_Call {
public:

  ATM_Attributes::result Accept(void);

protected:
  Incoming_ATM_Call(ATM_Interface& interface, 
		    generic_q93b_msg* incoming_setup, int cid);
  Incoming_ATM_Call(ATM_Interface& interface, call_type t);

  virtual ~Incoming_ATM_Call();
private:

  void state_has_changed(API_FSM::api_fsm_state from, 
			 API_FSM::api_fsm_state to,
			 ATM_Call* newc,
			 ATM_Leaf* newl);

  friend class FSM_ATM_Service;
};

//-----------------------------------------------

class Leaf_Initiated_ATM_Call : public Incoming_ATM_Call {
public:
  Leaf_Initiated_ATM_Call(ATM_Interface& interface);

  ATM_Attributes::result Request(void);
  
  virtual ~Leaf_Initiated_ATM_Call();
private:

  void state_has_changed(API_FSM::api_fsm_state from, 
			 API_FSM::api_fsm_state to,
			 ATM_Call* newc,
			 ATM_Leaf* newl);

  friend class FSM_ATM_Service;
};

//-----------------------------------------------

class Outgoing_ATM_Call : public ATM_Call {
public:

  Outgoing_ATM_Call(ATM_Interface& interface);

  ATM_Leaf* Get_Initial_Leaf(void);

  ATM_Attributes::result Establish(void);
  ATM_Leaf* Get_Leaf_Join_Request(void);
	
protected:

  virtual bool leaf_is_dying(ATM_Leaf* leaf);
  virtual void induced_teardown(void);
   
  virtual ~Outgoing_ATM_Call();
private:

  virtual bool cleanup_leaves_at_time_of_destructor(void);
  virtual int  cleanup_leaves_at_time_of_teardown(void);

  void state_has_changed(API_FSM::api_fsm_state from, 
			 API_FSM::api_fsm_state to,
			 ATM_Call* newc,
			 ATM_Leaf* newl);
  bool adopt_leaf(ATM_Leaf* lf);
  bool abandon_leaf(ATM_Leaf* lf);

  list<ATM_Leaf*> _incoming_LIJ_queue;
  list<ATM_Leaf*> _leaves;
  ATM_Leaf*       _leaf0;

  int             _next_leaf_id;
  int Get_Next_EPR(void);

  Controller* get_call_controller(void);

  friend class ATM_Attributes;
  friend class FSM_ATM_Service;
  friend class ATM_Leaf;
  friend class FSM_ATM_Call;
};

#endif
