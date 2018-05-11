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
#ifndef _SSCOPTIMERS_H_
#define _SSCOPTIMERS_H_

#ifndef LINT
static char const _sscop_timers_h_rcsid_[] =
"$Id: sscop_timers.h,v 1.1 1998/07/20 11:11:52 bilal Exp $";
#endif

#include <FW/kernel/Handlers.h>

class SSCOPconn;

class SSCOPTimer : public TimerHandler {
  friend class SSCOPconn;
public:

  SSCOPTimer(SSCOPconn *c, double ht);
  virtual ~SSCOPTimer();
  bool Active(void);

protected:

  double      _t;
  bool        _active;
  SSCOPconn * _owner;
};

class TimerPOLL : public SSCOPTimer {
public:

  TimerPOLL(SSCOPconn *c, double ht);
  virtual ~TimerPOLL();
  void Callback(void);
};

class TimerCC : public SSCOPTimer {
public:

  TimerCC(SSCOPconn *c, double ht);
  virtual ~TimerCC();
  void Callback(void);
};

class TimerNORESP : public SSCOPTimer {
public:

  TimerNORESP(SSCOPconn *c, double ht);
  virtual ~TimerNORESP();
  void Callback(void);
};

class TimerKEEPALIVE : public SSCOPTimer {
public:

  TimerKEEPALIVE(SSCOPconn *c, double ht);
  virtual ~TimerKEEPALIVE();
  void Callback(void);
};

class TimerIDLE : public SSCOPTimer {
public:

  TimerIDLE(SSCOPconn *c, double ht);
  virtual ~TimerIDLE();
  void Callback(void);
};

#endif // _SSCOPTIMERS_H_
