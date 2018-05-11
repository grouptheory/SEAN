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

#ifndef SSCOP_H
#define SSCOP_H
#ifndef LINT
static char const _sscop_h_rcsid_[] =
"$Id: sscop.h,v 1.2 1998/08/13 04:31:47 battou Exp $";
#endif

#include <sys/types.h>
#include <DS/containers/list.h>
#include <FW/kernel/Kernel.h>
#include <FW/actors/State.h>
#include <FW/kernel/Handlers.h>

const int MaxCC = 10;
const int MaxPD = 100;
const int MaxSTAT = 77;
const int MR = 30;

class SSCOPTimer;
class SSCOPVisitor;

class SSCOPconn : public State {
  // states need access to protocol control block data
  friend class sscopState;
  friend class sscop_idle;
  friend class sscop_outconn;
  friend class sscop_inconn;
  friend class sscop_outdisc;
  friend class sscop_outres;
  friend class sscop_inres;
  friend class sscop_outrec;
  friend class sscop_recresp;
  friend class sscop_inrec;
  friend class sscop_ready;
  // visitors need access to protocol control block data
  friend class SSCOPVisitor;
  friend class BeginVisitor;
  friend class BeginAckVisitor;
  friend class BeginRejVisitor;
  friend class EndVisitor;
  friend class EndAckVisitor;
  friend class ResyncVisitor;
  friend class ResyncAckVisitor;
  friend class RecovVisitor;
  friend class RecovAckVisitor;
  friend class SDVisitor;
  friend class PollVisitor;
  friend class StatVisitor;
  friend class UStatVisitor;
  friend class UDVisitor;
  friend class MDVisitor;
  friend class RetrieveVisitor;
  friend class MAAErrorVisitor;
public:
  enum PDUtypes {
    SSCOP_BAD      =  0,
    SSCOP_BGN      =  1,
    SSCOP_BGAK     =  2,
    SSCOP_BGREJ    =  7,
    SSCOP_END      =  3,
    SSCOP_ENDAK    =  4,
    SSCOP_RS       =  5,
    SSCOP_RSAK     =  6,
    SSCOP_ER       =  9,
    SSCOP_ERAK     = 15,
    SSCOP_SD       =  8,
    SSCOP_POLL     = 10,
    SSCOP_STAT     = 11,
    SSCOP_USTAT    = 12,
    SSCOP_UD       = 13,
    SSCOP_MD       = 14
  };
  enum sscop_states {
    sscop_idle,
    sscop_outconn,
    sscop_inconn,
    sscop_outdisc,
    sscop_outres,
    sscop_inres,
    sscop_outrec,
    sscop_recresp,
    sscop_inrec,
    sscop_ready
  } ;

  // see table 4/Q.2130 on page 14 for timer values
  SSCOPconn(double timer_cc = 1.0, double timer_poll = 0.75, double timer_noresp = 7.0,
	    double timer_keepalive = 2.0, double timer_idle = 15.0);
  ~SSCOPconn();
  
  State * Handle(Visitor *v);
  void    Interrupt(SimEvent *e);

  void Send2SSCF(SSCOPVisitor *v);
  void Send2Peer(SSCOPVisitor *v);
  SSCOPVisitor *SearchRXBuffer(int s);
  SSCOPVisitor *SearchTXBuffer(int s);
  SSCOPVisitor *SearchRTQueue(int s);
  SSCOPVisitor *GetFromRXBuffer(int s);
  int RemFromRXBuffer(int from_s, int to_s);
  int RemFromTXBuffer(int from_s, int to_s);
  void PrintRXBuffer();
  void PrintTXBuffer();
  // Connection control 
  void establish_req(SSCOPVisitor *v);
  void establish_resp(SSCOPVisitor *v);
  void release_req(SSCOPVisitor *v);
  void resync_req(SSCOPVisitor *v);
  void resync_resp(SSCOPVisitor *v);
  void recover_resp(SSCOPVisitor *v);
  void unitdata_req(SSCOPVisitor *v);
  void maa_data_req(SSCOPVisitor *v);

  void establish_ind(SSCOPVisitor *v);
  void establish_conf(SSCOPVisitor *v);
  void release_ind(SSCOPVisitor *v);
  void release_conf(SSCOPVisitor *v);
  void resync_ind(SSCOPVisitor *v);
  void resync_conf(SSCOPVisitor *v);
  void recover_ind(SSCOPVisitor *v);
  void udata_ind(SSCOPVisitor *v);
  void maa_data_ind(SSCOPVisitor *v);
  void retreive_ind(SSCOPVisitor *v);
  void retreive_comp(SSCOPVisitor *v);
  void MAAError(char *name, char code,SSCOPVisitor *v);
  sscopState *GetCS();

  void SetTimerCC();
  void SetTimerPOLL();
  void SetTimerNORESP();
  void SetTimerKEEPALIVE();
  void SetTimerIDLE();

  void StopTimer();
  void StopTimerCC();
  void StopTimerPOLL();
  void StopTimerNORESP();
  void StopTimerKEEPALIVE();
  void StopTimerIDLE();

  void ReSendVisitorToPeer(SSCOPVisitor *v);
  void SendQueuedSD();
  void DataSD(SSCOPVisitor *v);
  void DataPOLL(SSCOPVisitor *v);
  void DataUSTAT(SSCOPVisitor *v);
  void DataSTAT(SSCOPVisitor *v);
  void SendUSTAT(int first,int last);
  void DoLabelA();
  void DoLabelB();
  void DoLabelD();
  void DoLabelL();
  //macros pages 49-51
  void ReleaseBuffers();
  void PrepareRetrieval();
  void PrepareRecovery();
  void ClearTransmitter();
  void DeliverData();
  void InitStateVariables();
  void DataRetrieval();
  int DetectRetransmission(int sq);
  void SetPollTimer();
  void ResetDataTransferTimers();
  void SetDataTransferTimers();
  void InitVRMR();

  // not in Q.2110
  void UpdateVRMR();
  void ClearBuffers(int br);
  int IncVTCC();
  int IncVTSQ();
  int IncVTPS();
  int RXBase(int x);
  int TXBase(int x);
  int Modulo24(int x);
  void GetStateName(char *name);
private:
  void  ChangeState(sscopState *s, char *StateName);
  sscopState *_cs;
  char _StateName[80];
  bool _PollAfterRet;
  int _MaxCC;
  int _MaxPD;
  int _MaxSTAT;
  int _MR;
  // Transmitter state variables
  int _vt_s; 
  int _vt_ps;
  int _vt_a; 
  int _vt_pa;
  int _vt_ms;
  int _vt_pd;
  int _vt_cc;
  int _vt_sq;
  // Receiver state variables
  int _vr_r; 
  int _vr_h; 
  int _vr_mr;
  int _vr_sq;
  // Other variables 
  int _cb;
  int _Credit;
  // Timers 
  SSCOPTimer *_active_timer;
  SSCOPTimer *_timer_cc; // 1 sec
  SSCOPTimer *_timer_poll; // 2 secs
  SSCOPTimer *_timer_noresp; // 7 secs
  SSCOPTimer *_timer_keepalive; // 750 msecs
  SSCOPTimer *_timer_idle; // 15 secs
  // we need queues and buffers for retransmission and STAT construction list 
  list<SSCOPVisitor *> _TXBuffer;
  list<SSCOPVisitor *> _RXBuffer;
  list<SSCOPVisitor *> _TXQueue;
  list<SSCOPVisitor *> _RTQueue;
  // for retransmission 
  SSCOPVisitor *_last_bgn;
  SSCOPVisitor *_last_end;
  SSCOPVisitor *_last_rs;
  SSCOPVisitor *_last_er;
  // STAT construction list 
  u_int *_list;
};

#endif




