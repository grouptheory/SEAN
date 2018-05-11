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
"$Id: test_time.cc,v 1.2 1998/08/06 04:04:20 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <iostream.h>
#include <FW/basics/Conduit.h>
#include <FW/behaviors/Adapter.h>
#include <FW/behaviors/Protocol.h>
#include <FW/kernel/Kernel.h>
#include <FW/basics/diag.h>
#include <FW/basics/FW.h>
#include "sscop.h"
#include "sscop_aal.h"
#include "sscopState.h"
#include "sscop_visitors.h"

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
	  // I need to initiate the SSCOP connetion by Injecting a BeginReq visitor
	  SSCOPVisitor *v = new BeginVisitor(SSCOPVisitor::BeginReq,0,0);
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
  SSCOPVisitor *sv = (SSCOPVisitor *)v;
  SSCOPVisitor *s;
  u_char *sdu;
  int i,j, len;
  switch(sv->GetVT())
    {
    case SSCOPVisitor::BeginInd :
      s = new BeginAckVisitor(SSCOPVisitor::BeginAckReq,0,0);
      Inject(s);
      break;

    case SSCOPVisitor::BeginAckInd :
      for(j=0; j < 10; j++)
	{
	  sdu = new u_char[100];
	  for(i = 0; i < 100; i++)
	    sdu[i] = j;
	  s = new SDVisitor(SSCOPVisitor::SDReq,sdu,100);
	  Inject(s);
	}
      break;

    case SSCOPVisitor::EndInd :
      s = new EndAckVisitor(SSCOPVisitor::EndAckReq,0,0);
      Inject(s);
      break;
    case SSCOPVisitor::EndAckInd :
      break;
    case SSCOPVisitor::ResyncInd :
      s = new ResyncAckVisitor(SSCOPVisitor::ResyncAckReq,0,0);
      Inject(s);
      break;
    case SSCOPVisitor::ResyncAckInd :
      break;
    case SSCOPVisitor::BeginRejInd :
      break;
    case SSCOPVisitor::SDInd :
      cout << "SSCF-" << _id << " :  SDU -- seqnum = " << sv->GetS() << " DELIVERED in visitor = " << hex << v << endl;
      cout << dec << endl;
      break;
    case SSCOPVisitor::RecovInd :
      s = new RecovAckVisitor(SSCOPVisitor::RecovAckReq,0,0);
      Inject(s);
      break;

    case SSCOPVisitor::PollInd :
      break;
    case SSCOPVisitor::StatInd :
      break;
    case SSCOPVisitor::UstatInd :
      break;
    case SSCOPVisitor::UDInd :
      break;
    case SSCOPVisitor::MDInd :
      break;
    case SSCOPVisitor::RecovAckInd :
      s = new RecovAckVisitor(SSCOPVisitor::RecovAckReq,0,0);
      Inject(s);
      break;
    case SSCOPVisitor::MAAErrorInd :
      break;
    }
  v->Suicide();
  cout << "SSCF-" << _id << " :  suicide visitor = " << hex << v << endl;
  cout << dec << endl;
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

  // create Terminal 1
  SSCOPTerm * sscf_term = new SSCOPTerm(1,1);
  Adapter   * sscf_adapter = new Adapter(sscf_term);
  Conduit   * sscf_conduit = new Conduit("SSCF", sscf_adapter);

  // create the SSCOP1
  SSCOPconn * sscop_fsm = new SSCOPconn();
  Protocol  * sscop_protocol = new Protocol(sscop_fsm);
  Conduit   * sscop_conduit = new Conduit("SSCOP", sscop_protocol);

  // create the AAL
  SSCOPaal * aal_fsm = new SSCOPaal();
  Adapter  * aal_adapter = new Adapter(aal_fsm);
  Conduit  * aal_conduit = new Conduit("AAL",aal_adapter);

  Join(A_half(sscf_conduit), A_half(sscop_conduit));
  Join(B_half(sscop_conduit),A_half(aal_conduit));

  
  Kernel & controller = theKernel();

  controller.SetSpeed((Kernel::Speed)speedvalue);

  controller.StopIn(simtime);

  if (debug)
    controller.Debug();
  else
    controller.Run();

  return 0;
}
