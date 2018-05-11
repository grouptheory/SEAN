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
#ifndef _Q93BCALL_H_
#define _Q93BCALL_H_

#ifndef LINT
static char const _Q93bCall_h_rcsid_[] =
"$Id: Q93bCall.h,v 1.20 1998/10/14 12:59:10 bilal Exp $";
#endif

#include <iostream.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sean/daemon/ccd_config.h>

#include<DS/containers/list.h>
#include<DS/containers/dictionary.h>

#include <FW/kernel/Kernel.h>
#include <FW/actors/State.h>
#include <codec/uni_ie/ie.h>

class Q93bState;
class Party;
class sean_Visitor;
class ie_cause;
class CallTimer;
class generic_q93b_msg;
class abstract_local_id;

// these are more convenient that
// the ie_action_ind in the InfoElem class
const int OK   =  1;
const int YES  =  1;
const int RAI  = -1;
const int RAP  = -2;
const int I    = -3;
const int CLR  = -4;
const int CLP  = -5;
const int NOPC = -6;


class Call : public State {
  friend class Party;
  friend class Leaf;
  friend class CallTimer;
  friend class Q93bState;
public:

  enum call_type {
    p2p           = 0,
    p2mp_lij_root = 1,
    p2mp_lij_net  = 2
  };

  enum action_type {
    release_ind = 1,
    add_party_ind,
    add_party_ack_ind,
    add_party_rej_ind,
    drop_party_ack_ind,
    drop_party_ind,
    restart_ind,
    restart_ack_ind,
    setup_req,
    call_proc_req,
    call_proc_ind,
    release_req,
    release_comp_req,
    status_resp_req,
    status_req,
    status_enq_req,
    connect_req,
    connect_ack_req,
    add_party_req,
    add_party_ack_req,
    add_party_rej_req,
    drop_party_req,
    drop_party_ack_req,
    restart_req,
    restart_ack_req
  };

  Call(int user = USER_SIDE);
  virtual ~Call();

  virtual State * Handle(Visitor *v);
  virtual void    Interrupt(SimEvent *e);
  int VerifyMessage(ie_cause::CauseValues  &, Visitor *v);
  int ValidateEPR(ie_cause::CauseValues  &, Visitor *v);
  bool AnyParty(ie_end_pt_state::party_state ps, int epr);
  bool IsCallBusy(int epr);
  bool IsCallIdle(int epr);
  void Send2Peer(Visitor *v);
  void SendSetupToPeer();
  void SendStatusEnqToPeer();
  void SendLeafSetupToPeer(int leaf_seqnum);
  void SendCallStatusToPeer(ie_call_state::call_state_values cs,
			    ie_cause::CauseValues cv);
  void SendPartyStatusToPeer(ie_cause::CauseValues cv, ie_end_pt_state::party_state ps, int epr);
  void SendReleaseToPeer(ie_cause::CauseValues cv);
  void SendReleaseCompToPeer(ie_cause::CauseValues cv);
  void SendConnectAckToPeer();

  void SendDPToPeer(int epr, ie_cause::CauseValues cv);
  void SendDPAKToPeer(int epr, ie_cause::CauseValues cv);
  void SendDPAKToCCD(int epr, ie_cause::CauseValues cv);
  void SendDPIndToCCD(int epr, ie_cause::CauseValues cv);

  void SendAPRejToPeer(int epr, ie_cause::CauseValues cv);

  void Send2CCD(Visitor *qv);
  void SendReleaseIndToCCD(ie_cause::CauseValues cv);
  void SendReleaseCompIndToCCD(ie_cause::CauseValues cv);

  void DoLabelD(ie_cause::CauseValues cv);
  void DoUM(sean_Visitor *qv);
  void ResetAllTimers();
  void AlertParties();
  void ReleaseAllParties(ie_cause::CauseValues cv);
  void DropAllParties(ie_cause::CauseValues cv);
  void ConnAckParty();
  Party *Init(sean_Visitor *qv);
  Leaf  *InitLeaf(sean_Visitor *qv);
  Party *InitParty(sean_Visitor *qv);
  int IsP2P();
  int IsP2MP();
  int IsNetLIJ();
  int IsRootLIJ();
  int IsCallNegotiating();
  // timers
  int q93b_t301_timeout();
  int q93b_t303_timeout();
  int q93b_t308_timeout();
  int q93b_t310_timeout();
  int q93b_t313_timeout();
  int q93b_t316_timeout();
  int q93b_t317_timeout();
  int q93b_t322_timeout();

  void SetT301(void);
  void SetT303(void);
  void SetT308(void);
  void SetT309(void);
  void SetT310(void);
  void SetT313(void);
  void SetT316(void);
  void SetT317(void);
  void SetT322(void);
  void SetT397(Party * p);
  void SetT398(Party * p);
  void SetT399(Party * p);
  void SetT331(Leaf  * l);

  void StopTimer(void); // stop the active timer
  void StopTimer(Party *p); // stop the active timer

  void StopT301(void);
  void StopT303(void);
  void StopT308(void);
  void StopT309(void);
  void StopT310(void);
  void StopT313(void);
  void StopT316(void);
  void StopT317(void);
  void StopT322(void);
  void StopT397(Party * p);
  void StopT398(Party * p);
  void StopT399(Party * p);
  void StopT331(Leaf  * l);

   Q93bState *GetCS();
  Party *GetParty(int epr);
  Leaf  *GetLeaf(int seqnum);
  void suicide(void);
  void KillLeaf(int leaf_seqnum);
  void KillParty(Party *p);
  void SetIdentity(Conduit* c);
  Conduit* get_me(void);
   void SetVCI(int vci);
   int GetVCI(void);
   void SetVPI(int vpi);
   int GetVPI(void);
   ie_call_state::call_state_values GetCallState();
   void SetCallState(ie_call_state::call_state_values call_state);
   ie_cause::Location  StackType();

  // primitives to PartyControl
  void SendPartyStatusEnqReq(bool active_parties_only = false);
  void RCNonActiveParties();
  void RCParties();
  int  NumParties();
  int  NumLeaves();
  bool IsTimer322Active();
  void Save_CID(int cid);
  void Save_APPID(const abstract_local_id* appid);
  void Stamp_CID(sean_Visitor* qv) const;
  void Stamp_APPID(sean_Visitor* qv) const;
  void zero_ids(void);

  int  CallProtocol(sean_Visitor * sv);
  void PartyProtocol(sean_Visitor * sv);
  void PrintPartyMap(void);

private:
  int  _user;
  void ChangeState(Q93bState  *s);
  Q93bState  * _cs;
  int _crv;
  int _vpi;
  int _vci;
  int _port;
  int _cid;
  int _leaf_cnt;
  const abstract_local_id* _appid;
  call_type _call_type;
  ie_cause::Location _un;
  ie_call_state::call_state_values  _call_state;
  dictionary<int, Party *> _party_map;
  dictionary<int, Leaf  *> _leaf_map;
  InfoElem * _ie_array[num_ie];
  CallTimer  *_active_timer;
  CallTimer  * _t301;
  CallTimer  * _t303;
  CallTimer  * _t308;
  CallTimer  * _t309;
  CallTimer  * _t310;
  CallTimer  * _t313;
  CallTimer  * _t316;
  CallTimer  * _t317;
  CallTimer  * _t322;
  Conduit * _me;
};

#endif

