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

//-*-C++-*-
#ifndef _Q93BSTATE_H_
#define _Q93BSTATE_H_
#ifndef LINT
static char const _Q93bState_h_rcsid_[] =
"$Id: Q93bState.h,v 1.4 1998/08/27 17:40:45 battou Exp $";
#endif

#include <FW/kernel/Kernel.h>
#include <FW/actors/State.h>
#include <codec/uni_ie/ie.h>

#include "Q93bCall.h"

class generic_q93b_msg;
class Party;
class ie_cause;
class Call;
class sscfVisitor;

class Q93bState {
public:

  Q93bState();
  virtual ~Q93bState();
  virtual void StateName(char *name) = 0;
  virtual ie_call_state::call_state_values StateID() = 0;

  virtual void ExpT301(Call *);
  virtual void ExpT303(Call *);
  virtual void ExpT308(Call *);
  virtual void ExpT310(Call *);
  virtual void ExpT313(Call *);
  virtual void ExpT316(Call *);
  virtual void ExpT317(Call *);

  // page 144 Q.2931
  virtual int NotifyReq(sean_Visitor *qv, Call *c);
  virtual int NotifyInd(sean_Visitor *qv, Call *c);

  // page 136 Q.2971
  virtual int AddPartyRejInd(sean_Visitor *qv, Call *c);
  virtual int DropPartyInd(sean_Visitor *qv, Call *c);
  virtual int DropPartyAckInd(sean_Visitor *qv, Call *c);
  virtual int AddPartyInd(sean_Visitor *qv, Call *c);
  virtual int AddPartyAckInd(sean_Visitor *qv, Call *c);
  virtual int PartyAlertInd(sean_Visitor *qv, Call *c);

  virtual int AlertReq(sean_Visitor *qv, Call *c);
  virtual int CallProcReq(sean_Visitor *qv, Call *c);
  virtual int ConnectReq(sean_Visitor *qv, Call *c);
  virtual int ConnectAckReq(sean_Visitor *qv, Call *c);
  virtual int ReleaseReq(sean_Visitor *qv, Call *c);
  virtual int ReleaseCompReq(sean_Visitor *qv, Call *c);
  virtual int SetupReq(sean_Visitor *qv, Call *c);
  virtual int SetupCompReq(sean_Visitor *qv, Call *c);
  virtual int StatusReq(sean_Visitor *qv, Call *c);
  virtual int StatusEnqReq(sean_Visitor *qv, Call *c);
  virtual int StatusRespReq(sean_Visitor *qv, Call *c);

  virtual int AlertInd(sean_Visitor *qv, Call *c);
  virtual int CallProcInd(sean_Visitor *qv, Call *c);
  virtual int ConnectInd(sean_Visitor *qv, Call *c);
  virtual int ConnectAckInd(sean_Visitor *qv, Call *c);
  
  // redefined in N0 and N12 see page 83 of Q.2971
  virtual int ReleaseInd(sean_Visitor *qv, Call *c);
  // redefined in N0 see page 83 of Q.2971
  virtual int ReleaseCompInd(sean_Visitor *qv, Call *c);

  virtual int SetupInd(sean_Visitor *qv, Call *c);
  virtual int SetupCompInd(sean_Visitor *qv, Call *c);
  virtual int StatusInd(sean_Visitor *qv, Call *c);
  virtual int StatusEnqInd(sean_Visitor *qv, Call *c);
  virtual int StatusRespInd(sean_Visitor *qv, Call *c);

  // global state methods
  virtual int RestartReq(sean_Visitor *qv, Call *c);
  virtual int RestartResp(sean_Visitor *qv, Call *c);
  virtual int RestartAckReq(sean_Visitor *qv, Call *c);

  virtual int RestartInd(sean_Visitor *qv, Call *c);
  virtual int RestartCompInd(sean_Visitor *qv, Call *c);
  virtual int RestartAckInd(sean_Visitor *qv, Call *c);
  // P2MP methods
  virtual int LeafSetupReq(sean_Visitor *qv, Call *c);
  virtual int LeafSetupFailureReq(sean_Visitor *qv, Call *c);
  virtual int AddPartyReq(sean_Visitor *qv, Call *c);
  virtual int AddPartyCompReq(sean_Visitor *qv, Call *c);
  virtual int AddPartyAckReq(sean_Visitor *qv, Call *c);
  virtual int PartyAlertReq(sean_Visitor *qv, Call *c);
  virtual int AddPartyRejReq(sean_Visitor *qv, Call *c);
  virtual int DropPartyReq(sean_Visitor *qv, Call *c);
  virtual int DropPartyCompReq(sean_Visitor *qv, Call *c);
  virtual int DropPartyAckReq(sean_Visitor *qv, Call *c);
  
  virtual int LeafSetupInd(sean_Visitor *qv, Call *c);
  virtual int LeafSetupFailureInd(sean_Visitor *qv, Call *c);
  virtual int AddPartyCompInd(sean_Visitor *qv, Call *c);
  virtual int DropPartyCompInd(sean_Visitor *qv, Call *c);
  // Message manipulations
  virtual int UnexpectedMSG(sean_Visitor *qv, Call *c);
  // timers
  virtual int q93b_t301_timeout(Call *c);
  virtual int q93b_t308_timeout(Call *c);
  virtual int q93b_t310_timeout(Call *c);
  virtual int q93b_t313_timeout(Call *c);
  virtual int q93b_t316_timeout(Call *c);
  virtual int q93b_t317_timeout(Call *c);
  virtual int q93b_t322_timeout(Call *c);

protected:

  void ChangeState(Call *c, Q93bState *s);
};

// redefinition of state-dependent behaviors below
class NullState : public Q93bState {
public:

  virtual ~NullState();
  static Q93bState* Instance();
  void StateName(char *name);
  ie_call_state::call_state_values StateID();

  // pages 61 & 117  Q.2971 -- network side & user side respectively
  int SetupInd(sean_Visitor *qv, Call *c);
  int SetupReq(sean_Visitor *qv, Call *c);
  // don't do page 144 Q.2931
  inline int NotifyReq(sean_Visitor *qv, Call *c);
  inline int NotifyInd(sean_Visitor *qv, Call *c);
  // don't do pages 83 & 138 Q.2971
  inline int ReleaseInd(sean_Visitor *qv, Call *c);
  inline int ReleaseCompInd(sean_Visitor *qv, Call *c);
  // don't do pages 84 139 Q.2971
  inline int DropPartyInd(sean_Visitor *qv, Call *c);
  inline int DropPartyReq(sean_Visitor *qv, Call *c);
  inline int DropPartyAckReq(sean_Visitor *qv, Call *c);
  // don't do pages 85 & 140 Q.2971
  inline int ReleaseReq(sean_Visitor *qv, Call *c);
  // don't do page 144 Q.2971
  inline int StatusEnqReq(sean_Visitor *qv, Call *c);
  inline int StatusEnqInd(sean_Visitor *qv, Call *c);
  // don't do page 145 Q.2971
  inline int StatusInd(sean_Visitor *qv, Call *c);
  // don't do page 147 Q.2971
  inline void ExpT322(Call *);

  int LeafSetupFailureInd(sean_Visitor *qv, Call *c);

  // don't do page 148 Q.2971
  inline int UnexpectedMSG(sean_Visitor *qv, Call *c);
private:

  NullState();
  static Q93bState* _instance;
};

class CallInitiatedState : public Q93bState {
public:

  virtual ~CallInitiatedState();
  static Q93bState* Instance();
  void StateName(char *name);
  ie_call_state::call_state_values StateID();
  // pages 63 for network side
  int ReleaseCompReq(sean_Visitor *qv, Call *c); // same as ReleaseResp()
  int CallProcReq(sean_Visitor *qv, Call *c);
  // pages 119 for user side
  int CallProcInd(sean_Visitor *qv, Call *c);
  void ExpT303(Call *);
  // extensions
  int ConnectReq(sean_Visitor *qv, Call *c);

private:

  CallInitiatedState();
  static Q93bState* _instance;
};


class OutCallProcState : public Q93bState {
public:

  virtual ~OutCallProcState();
  static Q93bState* Instance();
  void StateName(char *name);
  ie_call_state::call_state_values StateID();
  // page 64 Q.2971 (network side)
  int AlertReq(sean_Visitor *qv, Call *c);
  int ConnectReq(sean_Visitor *qv, Call *c); //same as SetupResp
  // page 120 Q.2971 (user side)
  int AlertInd(sean_Visitor *qv, Call *c);
  int ConnectInd(sean_Visitor *qv, Call *c); 
  void ExpT310(Call *c);

private:

  OutCallProcState();
  static Q93bState* _instance;
};


class CallDeliveredState : public Q93bState {
public:

  virtual ~CallDeliveredState();
  static Q93bState* Instance();
  void StateName(char *name);
  ie_call_state::call_state_values StateID();
  // page 65 for network side
  int ConnectReq(sean_Visitor *qv, Call *c);
  int PartyAlertReq(sean_Visitor *qv, Call *c);
  int AddPartyRejReq(sean_Visitor *qv, Call *c);
  int AddPartyInd(sean_Visitor *qv, Call *c);
  // page 122 for user side
  int PartyAlertInd(sean_Visitor *qv, Call *c);
  int AddPartyReq(sean_Visitor *qv, Call *c);
  int AddPartyRejInd(sean_Visitor *qv, Call *c);
  int ConnectInd(sean_Visitor *qv, Call *c);

private:

  CallDeliveredState();
  static Q93bState* _instance;
};


class CallPresentState : public Q93bState {
public:

  virtual ~CallPresentState();
  static Q93bState* Instance();
  void StateName(char *name);
  ie_call_state::call_state_values StateID();
  // pages 66-68 for network side
  int AlertInd(sean_Visitor *qv, Call *c);
  int ConnectInd(sean_Visitor *qv, Call *c);
  int CallProcInd(sean_Visitor *qv, Call *c);
  void ExpT303(Call *c);
  // pages 125-126 for user side
  int ReleaseCompReq(sean_Visitor *qv, Call *c); // same as ReleaseResp
  int CallProcReq(sean_Visitor *qv, Call *c);
  int AlertReq(sean_Visitor *qv, Call *c);
  int ConnectReq(sean_Visitor *qv, Call *c); // same SetupResp

private:

  CallPresentState();
  static Q93bState* _instance;
};


class CallReceivedState : public Q93bState {
public:

  virtual ~CallReceivedState();
  static Q93bState* Instance();
  void StateName(char *name);
  ie_call_state::call_state_values StateID();
  // pages 69-71 for network side
  int PartyAlertInd(sean_Visitor *qv, Call *c);
  int AddPartyReq(sean_Visitor *qv, Call *c);
  int AddPartyRejInd(sean_Visitor *qv, Call *c);
  void ExpT301(Call *c);
  int ConnectInd(sean_Visitor *qv, Call *c);
  // page 127 for user side
  int ConnectReq(sean_Visitor *qv, Call *c); // same as setupResp
  int PartyAlertReq(sean_Visitor *qv, Call *c);
  int AddPartyRejReq(sean_Visitor *qv, Call *c);
  int AddPartyInd(sean_Visitor *qv, Call *c);

private:

  CallReceivedState();
  static Q93bState* _instance;
};


class ConnReqState : public Q93bState {
public:

  virtual ~ConnReqState();
  static Q93bState* Instance();
  void StateName(char *name);
  ie_call_state::call_state_values StateID();
  // pages 72-73 for network side
  int ConnectAckReq(sean_Visitor *qv, Call *c); // same as SetupCompReq
  int AddPartyRejInd(sean_Visitor *qv, Call *c);
  int AddPartyAckInd(sean_Visitor *qv, Call *c);
  int PartyAlertInd(sean_Visitor *qv, Call *c);
  // pages 128-129 for user side
  void ExpT313(Call *c);
  int ConnectAckInd(sean_Visitor *qv, Call *c);
  int PartyAlertReq(sean_Visitor *qv, Call *c);
  int AddPartyRejReq(sean_Visitor *qv, Call *c);
  int AddPartyInd(sean_Visitor *qv, Call *c);

private:

  ConnReqState();
  static Q93bState* _instance;
};

class InCallProcState : public Q93bState {
public:

  virtual ~InCallProcState();
  static Q93bState* Instance();
  void StateName(char *name);
  ie_call_state::call_state_values StateID();
  // page 74 for network side
  int AlertInd(sean_Visitor *qv, Call *c);
  int ConnectInd(sean_Visitor *qv, Call *c);
  void ExpT310(Call *c);
  // page 130 user side
  int AlertReq(sean_Visitor *qv, Call *c);
  int ConnectReq(sean_Visitor *qv, Call *c);

private:

  InCallProcState();
  static Q93bState* _instance;
};


class ActiveState : public Q93bState {
public:

  virtual ~ActiveState();
  static Q93bState* Instance();
  void StateName(char *name);
  ie_call_state::call_state_values StateID();
  // page 75 Q.2971
  int ConnectAckInd(sean_Visitor *qv, Call *c);
  // page 77 & 132 Q.2971
  int LinkReleaseInd(sscfVisitor *lsv, Call *);
  int LinkEstablishError(sscfVisitor *lsv, Call *);
  int LinkEstablishInd(sscfVisitor *lsv, Call *);
  int LinkEstablishConf(sscfVisitor *lsv, Call *);
  // page 78 & 134 Q.2971
  int AddPartyReq( sean_Visitor *,  Call *);
  int AddPartyAckReq( sean_Visitor *,  Call *);
  int AddPartyRejReq( sean_Visitor *,  Call *);
  int PartyAlertReq( sean_Visitor *,  Call *);
  int DropPartyReq( sean_Visitor *,  Call *);
  int DropPartyAckReq( sean_Visitor *,  Call *);
  int AddPartyAckInd( sean_Visitor *,  Call *);
  int PartyAlertInd( sean_Visitor *,  Call *);
  int AddPartyInd( sean_Visitor *,  Call *);
  int AddPartyRejInd( sean_Visitor *,  Call *);
  int DropPartyInd( sean_Visitor *,  Call *);
  int DropPartyAckInd( sean_Visitor *,  Call *);

private:

  ActiveState();
  static Q93bState* _instance;
};


class ReleaseReqState : public Q93bState {
public:

  virtual ~ReleaseReqState();
  static Q93bState* Instance();
  void StateName(char *name);
  ie_call_state::call_state_values StateID();
  // page 84 Q.2971
  inline int DropPartyReq(class sean_Visitor *, class Call *);
  inline int DropPartyAckReq(class sean_Visitor *, class Call *);
  // page 85 Q.2971
  int ReleaseReq(sean_Visitor *qv, Call *c); 
  // page 86 Q.2971
  int ReleaseCompReq(sean_Visitor *qv, Call *c); // same as ReLeaseResp
  inline int DropPartyInd(class sean_Visitor *, class Call *);
  inline int DropPartyAckInd(class sean_Visitor *, class Call *);
  int StatusInd(sean_Visitor *qv, Call *c);
  // page 141 Q.2971
  int ReleaseInd(sean_Visitor *qv, Call *c);
  void ExpT308(Call *c);

private:

  ReleaseReqState();
  static Q93bState* _instance;
};


class ReleaseIndState : public Q93bState {
public:

  virtual ~ReleaseIndState();
  static Q93bState* Instance();
  void StateName(char *name);
  ie_call_state::call_state_values StateID();
  // page 84 Q.2971
  inline int DropPartyReq(class sean_Visitor *, class Call *);
  inline int DropPartyAckReq(class sean_Visitor *, class Call *);
  // page 85 Q.2971
  int ReleaseReq(sean_Visitor *qv, Call *c); 
  // pages 87,88, and 143 Q.2971
  int ReleaseInd(sean_Visitor *qv, Call *c);
  inline int DropPartyInd(class sean_Visitor *, class Call *);
  void ExpT308(Call *c);
  int StatusInd(sean_Visitor *qv, Call *c);
  int ReleaseCompReq(sean_Visitor *qv, Call *c); // same as ReLeaseResp

private:

  ReleaseIndState();
  static Q93bState* _instance;
};

#endif
