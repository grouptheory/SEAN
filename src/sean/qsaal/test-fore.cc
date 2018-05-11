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
#ifndef LINT
static char const _test1_cc_rcsid_[] =
"$Id: test-fore.cc,v 1.2 1998/08/06 04:04:19 bilal Exp $";
#endif
#include <common/cprototypes.h>
#include <sean/daemon/ccd_config.h>

#include <iostream.h>
#include <FW/basics/Conduit.h>
#include <FW/behaviors/Adapter.h>
#include <FW/behaviors/Protocol.h>
#include <FW/kernel/Kernel.h>
#include <FW/basics/diag.h>
#include <FW/basics/FW.h>

#include <sean/uni_fsm/q93b_expander.h>
#include <sean/ipc/tcp_ipc.h>
#include <sean/uni_fsm/Q93bTimers.h>
#include <sean/uni_fsm/Q93bState.h>
#include <sean/uni_fsm/Q93bCall.h>
#include <sean/uni_fsm/Q93bParty.h>
#include <sean/uni_fsm/PartyState.h>
#include <sean/uni_fsm/q93b_expander.h>
#include <sean/uni_fsm/QueueState.h>
#include <sean/uni_fsm/Router.h>
#include <sean/uni_fsm/SSCFVisitor.h>
#include <sean/cc/call_control.h>

#include "sscf.h"
#include "sscop.h"
#include "fore_aal.h"
#include "sscopState.h"
#include "sscop_visitors.h"
#include "qsaal_expander.h"

class StartTimer : public TimerHandler  {
public:

  StartTimer(Terminal * t, Visitor * v) : 
    TimerHandler(t,1.0), _t(t), _v(v) { }
  virtual ~StartTimer() { }
  void Callback(void) { _t->Inject(_v); }

private:

  Terminal * _t;
  Visitor  * _v;
};

class SSCOPTerm : public Terminal {
public:

  SSCOPTerm(int startit, int id) 
    {
      _id = id;
      if (startit)
	{
	  // I need to initiate the SSCOP connetion 
	  sscfVisitor *v = new sscfVisitor(sscfVisitor::EstReq);
	  _go = new StartTimer(this,v);
	  Register(_go);
	}
    }

  ~SSCOPTerm() { }

  void Interrupt(SimEvent * e)
  { cout << "I was Interrupted by event " << e->GetID() << "!\n"; }

  void Absorb(Visitor * v);

private:
  int _id;
  StartTimer * _go;
};


void SSCOPTerm::Absorb(Visitor * v)
{
  v->Suicide();
}

int main(int argc, char** argv)
{
  char filename[80];

  DiagLevel("fsm.sscop", DIAG_DEBUG);

  int speedvalue;
  if (argc > 1)
    speedvalue = atoi(argv[1]);
  else {
    cout << "usage: " << argv[0] << " speed duration r/d vispipe" << endl;
    cout << "Enter log_2 speed (-2,-1,0,1,2) or enter 666 for sim-time: ";
    cin >> speedvalue;
  }

  double simtime;
  if (argc > 2)
    simtime = atof(argv[2]);
  else {
    cout << "How long (seconds) to run the simulation: ";
    cin >> simtime;
  }

  int debug;
  if (argc > 3) {
    if (argv[3][0] == 'r' || argv[3][0] == '0' || argv[3][0] == 'R')
      debug = 0;
    else
      debug = 1;
  } else {
    cout << "(0)= Run, (1)= Debug, Enter 1/r or 0/d: ";
    cin >> filename;
    if (filename[0] == 'r' || filename[0] == 'R' || filename[0] == '0')
      debug = 0;
    else
      debug = 1;
  }

  if (argc > 4)
    VisPipe(argv[4]);
  else {
    cout << "Enter the name of the visualizer pipe: ";
    cin >> filename;
    VisPipe(filename);
  }

  // TERMINAL
  SSCOPTerm * term = new SSCOPTerm(1,1);
  Adapter   * adapter = new Adapter(term);
  Conduit   * ct1 = new Conduit("SSCF", adapter);

  // CCD
  call_control * ccd = new call_control(0,USER_SIDE);
  Protocol * ccdp = new Protocol(ccd);
  Conduit *ccdc = new Conduit("CCD", ccdp);

  // UNI
  q93b_expander * U1 = new q93b_expander(USER_SIDE);
  Cluster * U1cluster = new Cluster(U1);
  Conduit * uc = new Conduit("UNI",U1cluster);

  // QUEUE
  QueueState * q1  = new QueueState(0.01,500);
  Protocol   * pq1 = new Protocol(q1);
  Conduit    * q1c = new Conduit("Queue-1", pq1);

//   // SSCF
//   SSCFconn * sscf_fsm = new SSCFconn();
//   Protocol  * sscf_protocol = new Protocol(sscf_fsm);
//   Conduit   * sscf_conduit = new Conduit("SSCF", sscf_protocol);

//   // SSCOP
//   SSCOPconn * sscop_fsm = new SSCOPconn();
//   Protocol  * sscop_protocol = new Protocol(sscop_fsm);
//   Conduit   * sscop_conduit = new Conduit("SSCOP", sscop_protocol);

//   // AAL
//   fore_SSCOPaal * aal_fsm = new fore_SSCOPaal();
//   Adapter  * aal_adapter = new Adapter(aal_fsm);
//   Conduit  * aal_conduit = new Conduit("AAL",aal_adapter);

  QSAALExpander *qsaal = new  QSAALExpander();
  Cluster * qsaal_cluster = new Cluster(qsaal);
  Conduit * qsaal_c = new Conduit("QSAAL",qsaal_cluster);

  Join(A_half(ct1), A_half(ccdc));
  Join(B_half(ccdc), A_half(uc));
  Join(B_half(uc), A_half(q1c));

  Join(B_half(q1c),A_half(qsaal_c));

  //  Join(B_half(q1c), A_half(sscf_conduit));
  //  Join(B_half(sscf_conduit), A_half(sscop_conduit));
  //  Join(B_half(sscop_conduit),A_half(aal_conduit));

  
  Kernel & controller = theKernel();

  controller.SetSpeed((Kernel::Speed)speedvalue);

  controller.StopIn(simtime);

  if (debug)
    controller.Debug();
  else
    controller.Run();

  return 0;
}
