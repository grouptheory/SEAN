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

#ifndef _UNI_TIMERS_H_
#define _UNI_TIMERS_H_

#ifndef LINT
static char const _Q93bTimers_h_rcsid_[] =
"$Id: Q93bTimers.h,v 1.3 1998/08/26 12:01:43 battou Exp $";
#endif

#include <FW/kernel/Handlers.h>



class Party;
class Call;
class Coordinator;

// timers are expressed in msec's

//--------------- Call Timers ----------------------------

class CallTimer: public TimerHandler {
  friend class Call;
public:
  enum CallTimerType {
    timer_t301,
    timer_t302,
    timer_t303,
    timer_t304,
    timer_t306,
    timer_t308,
    timer_t309,
    timer_t310,
    timer_t313,
    timer_t316,
    timer_t317,
    timer_t322
  };
  enum CallTimers {
    t301   = 180000, // 3 min
    t302   = 15000, // not used
    t303   = 4000,
    t304   = 20000, // not used
    t306   = 30000, // not used
    t308   = 30000,
    t309   = 10000,
    t310   = 120000, // 30 to 120 sec in UNI-4.0
    t313   = 4000,
    t316   = 120000,
    t317   = 100000,
    t322   = 4000
  };

  CallTimer(CallTimers t, State *ca, int max_retries = 1);
  virtual  ~CallTimer();
  virtual CallTimerType Timer_type() = 0; 
  //  void Callback();
  int SetTimer();
  int StopTimer();
  Call *get_call();
  bool IsActive();
  void SetActiveON();
  void SetActiveOFF();
 protected:
  bool _active;
  char _timer_diag[3];
  int _retries;
  int _max_retries;
  State *_call;
};


  
class Q93b_t301 : public CallTimer{
public:
  Q93b_t301(State *ca, int max_retries = 1);
  void Callback();
  CallTimerType Timer_type();
};

class Q93b_t303 : public CallTimer{
public:
  Q93b_t303(State *ca, int max_retries = 1);
  void Callback();
  CallTimerType Timer_type();
};

class Q93b_t308 : public CallTimer{
public:
  Q93b_t308(State *ca, int max_retries = 1);
  void Callback();
  CallTimerType Timer_type();
};


class Q93b_t309 : public CallTimer{
public:
  Q93b_t309(State *ca, int max_retries = 1);
  void Callback();
  CallTimerType Timer_type();
};

class Q93b_t310 : public CallTimer{
public:
  Q93b_t310(State *ca, int max_retries = 1);
  void Callback();
  CallTimerType Timer_type();
};

class Q93b_t313 : public CallTimer{
public:
  Q93b_t313(State *ca, int max_retries = 1);
  void Callback();
  CallTimerType Timer_type();
};

class Q93b_t316 : public CallTimer{
public:
  Q93b_t316(State *ca, int max_retries = 1);
  void Callback();
  CallTimerType Timer_type();
};

class Q93b_t317 : public CallTimer{
public:
  Q93b_t317(State *ca, int max_retries = 1);
  void Callback();
  CallTimerType Timer_type();
};

class Q93b_t322 : public CallTimer{
public:
  Q93b_t322(State *ca, int max_retries = 1);
  void Callback();
  CallTimerType Timer_type();
};


//----------------- Party Timers ----------------------------

class PartyTimer: public TimerHandler {
public:
  enum PartyTimerType {
    timer_t397,
    timer_t398,
    timer_t399
  };
  enum PartyTimers {
    t397   = 180000, // 3 min same as t301
    t398   = 4000,
    t399   = 124000 // 34 to 124 sec in uni-4.0
  };
  PartyTimer(PartyTimers t, Party *p, int max_retries = 1);
  virtual  ~PartyTimer();
  virtual PartyTimerType Timer_type() = 0; 
  //  void Callback();
  int SetTimer();
  int StopTimer();
  Party *GetParty();
 protected:
  char _timer_diag[3];
  Party *_party;
  int _max_retries;
};






class Q93b_t397 : public PartyTimer{
public:
  Q93b_t397(Party *p, int max_retries = 1);
  void Callback();
  PartyTimerType Timer_type();
};

class Q93b_t398 : public PartyTimer{
public:
  Q93b_t398(Party *p, int max_retries = 1);
  void Callback();
  PartyTimerType Timer_type();
};

class Q93b_t399 : public PartyTimer{
public:
  Q93b_t399(Party *p, int max_retries = 1);
  void Callback();
  PartyTimerType Timer_type();
};


//----------------- Leaf  Timers ----------------------------

class Leaf;

class LeafTimer: public TimerHandler {
public:
  enum LeafTimers {
    t331   = 60000 // 60 secs
  };
  LeafTimer(LeafTimers t, Leaf *l, int max_retries = 1);
  virtual  ~LeafTimer();
  //  void Callback();
  int SetTimer();
  int StopTimer();
  Leaf *GetLeaf();
 protected:
  char _timer_diag[3];
  Leaf *_Leaf;
  int _max_retries;
};






class Q93b_t331 : public LeafTimer{
public:
  Q93b_t331(Leaf *l, int max_retries = 1);
  void Callback();
};

#endif

