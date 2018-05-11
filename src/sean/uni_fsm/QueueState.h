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
 * File: QueueState.cc
 * Author: 
 * Version: $Id: QueueState.h,v 1.2 1999/03/25 21:30:18 marsh Exp $
 *
 * Purpose: Implements a queue of depth n.  Every once in a while, the
 * queue sends some data in each direction.  Does not queue
 * QueueOverrider-type Visitors because they model the hardware,
 * not the data that flows on it.  Kills QueueKillMe-type Visitors.
 *
 * 
 * BUGS: 
 */

#ifndef __QUEUE_STATE_H__
#define __QUEUE_STATE_H__
#ifndef LINT
static char const _QueueState_h_rcsid_[] =
"$Id: QueueState.h,v 1.2 1999/03/25 21:30:18 marsh Exp $";
#endif

#include <FW/actors/State.h>
#include <DS/containers/list.h>
#define QUEUE_SERVICE_EVENT 1000
#define QUEUE_OVERRIDER_NAME "QueueOverrider"
#define QUEUE_KILL_ME_NAME "QueueKillMe"

class QueueState: public State {
public:
  static const int DEFAULT_DEPTH = 100;
  QueueState(double service_time, int depth = QueueState::DEFAULT_DEPTH);
  virtual State * Handle(Visitor* v);
  virtual void Interrupt(SimEvent *event);
protected:
  virtual ~QueueState();
  int _depth;
  double _service_time;
  list<Visitor *> _qFromA;	// Visitors that enter on the A side
  list<Visitor *> _qFromB;	// Visitors that enter on the B side
  SimEvent * _serviceEvent;	/* Occurs every _service_time seconds.
				 * Reused throughout the lifetime of
				 * the QueueState.  Destroyed by the
				 * destructor.
				 */
};

#endif







