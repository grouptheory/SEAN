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

#ifndef _PARTY_H_
#define _PARTY_H_
#ifndef LINT
static char const _Q93bParty_h_rcsid_[] =
"$Id: Q93bParty.h,v 1.5 1998/07/27 20:08:49 battou Exp $";
#endif

#include <sys/types.h>
#include <codec/uni_ie/ie.h>
#include <FW/kernel/Kernel.h>
#include <FW/actors/State.h>
// #include "Q93bTimers.h"

class Call;
class Q93bState;
class PartyState;

class PartyTimer;
class LeafTimer;

class Q93b_t331;
class Q93b_t397;
class Q93b_t398;
class Q93b_t399;

class Party {
friend class Call;
friend class PartyState;
public:
  enum LeafStates {
    leaf_join_initiated = 0,
    leaf_null = 1
  };
  Party(Call *ca, ie_called_party_num *cpn, ie_end_pt_ref  *epr);
  ~Party();
  ie_end_pt_state::party_state get_eps();
  void set_eps(ie_end_pt_state::party_state state);
  void set_ls(LeafStates ls);
  LeafStates get_ls();
  int get_seqnum();
  void set_seqnum(int seqnum);
  int get_epr();
  void set_epr(int epr);
  void set_epr_ie(InfoElem *epr_ie);
  void set_eps_ie(InfoElem *eps_ie);
  void set_called_number(ie_called_party_num *cn_ie);
  void set_called_subaddr(ie_called_party_subaddr *cs_ie);
  InfoElem *get_epr_ie();
  InfoElem *get_eps_ie();
  InfoElem* get_called_number(void);
  InfoElem* get_called_subaddr(void);
  // timers
  void  q93b_t331_timeout();
  void  q93b_t397_timeout();
  void  q93b_t398_timeout();
  void  q93b_t399_timeout();
  void SetT331();
  void SetT397();
  void SetT398();
  void SetT399();
  void StopT331();
  void StopT397();
  void StopT398();
  void StopT399();
  void StopTimer(); // stop the active timer
  PartyState *GetPS();
  Call *GetOwner(void);
  void SetOwner(Call *c);
protected:
  void ChangeState(PartyState *s);
  PartyState *_ps;
  Call * _ca;
  int _epr;
  int _leaf_seqnum;
  LeafStates _ls;
  LeafTimer  * _t331;
  PartyTimer * _t397;
  PartyTimer * _t398;
  PartyTimer * _t399;

  PartyTimer * _active_timer;

  u_int _t331_retries;
  u_int _t397_retries;
  u_int _t398_retries;
  u_int _t399_retries;

  u_int _t331_max_retries;
  u_int _t397_max_retries;
  u_int _t398_max_retries;
  u_int _t399_max_retries;

  ie_called_party_num *_dest;
  ie_called_party_subaddr *_dest_subaddr;
  ie_end_pt_ref   *_epr_ie;
  ie_end_pt_state *_eps_ie;
};
#endif





