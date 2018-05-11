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
#ifndef _PARTY_STATE_H_
#define _PARTY_STATE_H_

#ifndef LINT
static char const _Q93bPartyState_h_rcsid_[] =
"$Id: PartyState.h,v 1.14 1998/07/27 23:14:25 battou Exp $";
#endif

#include <sys/types.h>

#include <codec/uni_ie/ie.h>

#include "Q93bTimers.h"

class Party;
class sean_Visitor;

class PartyState {
public:
  virtual ~PartyState();
  virtual void StateName(char *name) = 0;
  virtual ie_end_pt_state::party_state StateID() = 0;
  // [SigList-7] Recommendation Q.2971 page 57
  // Signals related to primitives
  virtual int SetupReq(sean_Visitor *qv, Party *p);
  virtual int AlertReq(sean_Visitor *qv, Party *p);
  virtual int ConnectReq(sean_Visitor *qv, Party *p); // same as SetupResp
  virtual int ConnectInd(sean_Visitor *qv, Party *p);
  virtual int ConnectAckInd(sean_Visitor *qv, Party *p);
  virtual int SetupCompReq(sean_Visitor *qv, Party *p); 
  virtual int ReleaseReq(sean_Visitor *qv, Party *p);
  virtual int ReleaseRespReq(sean_Visitor *qv, Party *p);
  virtual int ReleaseCompReq(sean_Visitor *qv, Party *p);
  virtual int AddPartyReq(sean_Visitor *qv, Party *p);
  virtual int AddPartyAckReq(sean_Visitor *qv, Party *p);
  virtual int AddPartyRejReq(sean_Visitor *qv, Party *p);
  virtual int PartyAlertReq(sean_Visitor *qv, Party *p);
  virtual int DropPartyReq(sean_Visitor *qv, Party *p);
  virtual int DropPartyAckReq(sean_Visitor *qv, Party *p);
  virtual int PartyStatusEnqReq(sean_Visitor *qv, Party *p); 
  virtual int StatusEnqReq(sean_Visitor *qv, Party *p);
  // Messages
  virtual int SetupInd(sean_Visitor *qv, Party *p);
  virtual int AlertInd(sean_Visitor *qv, Party *p);
  virtual int ReleaseInd(sean_Visitor *qv, Party *p);
  virtual int ReleaseCompInd(sean_Visitor *qv, Party *p);
  virtual int StatusEnqInd(sean_Visitor *qv, Party *p);
  virtual int StatusInd(sean_Visitor *qv, Party *p);
  virtual int AddPartyInd(sean_Visitor *qv, Party *p);
  virtual int AddPartyAckInd(sean_Visitor *qv, Party *p);
  virtual int AddPartyRejInd(sean_Visitor *qv, Party *p);
  virtual int PartyAlertInd(sean_Visitor *qv, Party *p);
  virtual int DropPartyInd(sean_Visitor *qv, Party *p);
  virtual int DropPartyAckInd(sean_Visitor *qv, Party *p);
  virtual int UnexpectedMSG(sean_Visitor *qv, Party *p);
  // timers
  virtual int party_t397_timeout(Party *p);
  virtual int party_t398_timeout(Party *p);
  virtual int party_t399_timeout(Party *p);
protected:
  PartyState();
  void ChangeState(Party *p, PartyState *s);
};


class P0_Null : public PartyState {
public:
  virtual ~P0_Null();
  void StateName(char *name);
  ie_end_pt_state::party_state StateID();
  static PartyState *Instance();
  int SetupReq(sean_Visitor *qv, Party *p);
  int AddPartyReq(sean_Visitor *qv, Party *p);
  int SetupInd(sean_Visitor *qv, Party *p);
  int AddPartyInd(sean_Visitor *qv, Party *p);
  // from page 108 Q.2971
  int PartyStatusEnqReq(sean_Visitor *qv, Party *p);
  int StatusEnqInd(sean_Visitor *qv, Party *p);
  int StatusInd(sean_Visitor *qv, Party *p);
  // redefined to avoid getting the default of page 104/161 in Q.2971
  virtual int DropPartyInd(sean_Visitor *qv, Party *p);
  // redefined to avoid getting the default of page 103/160 in Q.2971
  int DropPartyAckInd(sean_Visitor *qv, Party *p);
  // page 106/163 in Q.2971
  int UnexpectedMSG(sean_Visitor *qv, Party *p);
  int ReleaseInd(sean_Visitor *qv, Party *p);
  int ReleaseCompInd(sean_Visitor *qv, Party *p);
  int ReleaseRespReq(sean_Visitor *qv, Party *p);
protected:
  P0_Null();
  static PartyState *_instance;
};

class P1_AddPartyInit : public PartyState {
public:
  virtual ~P1_AddPartyInit();
  static PartyState *Instance();
  void StateName(char *name);
  ie_end_pt_state::party_state StateID();
  int SetupCompReq(sean_Visitor *qv, Party *p);
  int AddPartyAckInd(sean_Visitor *qv, Party *p);
  int AddPartyRejInd(sean_Visitor *qv, Party *p);
  int party_t399_timeout(Party *p);
  int PartyAlertInd(sean_Visitor *qv, Party *p);
  int DropPartyReq(sean_Visitor *qv, Party *p);
  // page 156 Q.2971
  int ConnectInd(sean_Visitor *qv, Party *p);
  // page 157 Q.2971
  int AlertInd(sean_Visitor *qv, Party *p);
protected:
  P1_AddPartyInit();
  static PartyState *_instance;
};


class P2_AddPartyRcv : public PartyState {
public:
  virtual ~P2_AddPartyRcv();
  static PartyState *Instance();
  void StateName(char *name);
  ie_end_pt_state::party_state StateID();
  // page 101 Q.2971
  int ConnectReq(sean_Visitor *qv, Party *p); // same SetupResp
  int AddPartyAckReq(sean_Visitor *qv, Party *p);
  int AddPartyRejReq(sean_Visitor *qv, Party *p);
  int AddPartyInd(sean_Visitor *qv, Party *p);
  int PartyAlertReq(sean_Visitor *qv, Party *p);
  int AlertReq(sean_Visitor *qv, Party *p);
  int ConnectAckInd(sean_Visitor *qv, Party *p);
  int UnexpectedMSG(sean_Visitor *qv, Party *p);
protected:
  P2_AddPartyRcv();
  static PartyState *_instance;
};


class P3_PartyAlertDel : public PartyState {
public:
  virtual ~P3_PartyAlertDel();
  static PartyState *Instance();
  void StateName(char *name);
  ie_end_pt_state::party_state StateID();
  int ConnectAckInd(sean_Visitor *qv, Party *p);
  int ConnectReq(sean_Visitor *qv, Party *p);
  int AddPartyAckReq(sean_Visitor *qv, Party *p);
  int DropPartyReq(sean_Visitor *qv, Party *p);
protected:
  P3_PartyAlertDel();
  static PartyState *_instance;
};


class P4_PartyAlertRcv : public PartyState {
public:
  virtual ~P4_PartyAlertRcv();
  static PartyState *Instance();
  void StateName(char *name);
  ie_end_pt_state::party_state StateID();
  int SetupCompReq(sean_Visitor *qv, Party *p);
  int party_t397_timeout(Party *p);
  int AddPartyAckInd(sean_Visitor *qv, Party *p);
  int DropPartyReq(sean_Visitor *qv, Party *p);
  int ConnectInd(sean_Visitor *qv, Party *p);
protected:
  P4_PartyAlertRcv();
  static PartyState *_instance;
};


class P5_DropPartyInit : public PartyState {
public:
  virtual ~P5_DropPartyInit();
  static PartyState *Instance();
  void StateName(char *name);
  ie_end_pt_state::party_state StateID();
  int AddPartyRejInd(sean_Visitor *qv, Party *p);
  int party_t398_timeout(Party *p);
  // from bottom page 105 Q.2971
  int StatusInd(sean_Visitor *qv, Party *p);
  // from page 106 Q.2971
  int ReleaseReq(sean_Visitor *qv, Party *p);
  // redefined to avoid getting the default of page 104 in Q.2971
  int DropPartyInd(sean_Visitor *qv, Party *p);
  int ReleaseInd(sean_Visitor *qv, Party *p);
protected:
  P5_DropPartyInit();
  static PartyState *_instance;
};


class P6_DropPartyRcv : public PartyState {
public:
  virtual ~P6_DropPartyRcv();
  static PartyState *Instance();
  void StateName(char *name);
  ie_end_pt_state::party_state StateID();
  // page 161 Q.2971
  int DropPartyAckReq(sean_Visitor *qv, Party *p);
protected:
  P6_DropPartyRcv();
  static PartyState *_instance;
};


class P7_PartyActive : public PartyState {
public:
  virtual ~P7_PartyActive();
  static PartyState *Instance();
  void StateName(char *name);
  ie_end_pt_state::party_state StateID();
  int DropPartyReq(sean_Visitor *qv, Party *p);
protected:
  P7_PartyActive();
  static PartyState *_instance;
};

#endif
