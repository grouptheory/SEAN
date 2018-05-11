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
static char const _sscop_cc_rcsid_[] =
"$Id: sscop.cc,v 1.4 1998/09/08 12:10:52 battou Exp $";
#endif
#include <common/cprototypes.h>
#include <FW/basics/diag.h>
#include <FW/kernel/Kernel.h>
#include "sscop.h"
#include "sscopState.h"
#include "sscop_timers.h"
#include "sscop_visitors.h"
#include <assert.h>
#include <netinet/in.h>


void SSCOPconn::PrintRXBuffer()
{
  SSCOPVisitor *x;
  cout << "RXBUFFER: ";
  list_item itx;
  forall_items(itx,_RXBuffer)
    {
      x = _RXBuffer.contents(itx);
      cout << x->_s << " ";
    }
  cout << "\n";
}

void SSCOPconn::PrintTXBuffer()
{
  SSCOPVisitor *x;
  cout << "TXBUFFER: ";
  list_item itx;
  forall_items(itx,_TXBuffer)
    {
      x = _TXBuffer.contents(itx);
      cout << x->_s << " ";
    }
  cout << "\n";
}

SSCOPconn::SSCOPconn(double timer_cc, double timer_poll, double timer_noresp,
		     double timer_keepalive, double timer_idle)
{
  _cs = sscop_idle::Instance();
  strcpy(_StateName,"sscop_idle");
  _timer_cc = new TimerCC(this, timer_cc);
  _timer_poll = new TimerPOLL(this, timer_poll);
  _timer_noresp = new TimerNORESP(this, timer_noresp);
  _timer_keepalive = new TimerKEEPALIVE(this,timer_keepalive);
  _timer_idle = new TimerIDLE(this, timer_idle);

  _vt_sq = _vr_sq = 0; 
  _vr_mr = MR;
  _cb = 1;
  _MaxCC = MaxCC;
  _MaxPD = MaxPD;
  _MaxSTAT = MaxSTAT;
  _MR = MR;

  _TXBuffer.clear();
  _RXBuffer.clear();
  _TXQueue.clear();
  _RTQueue.clear();

  _last_bgn = _last_end = _last_rs = _last_er = NULL;
  _list = new u_int[MaxSTAT];
  // Set Poll after Retransmission to TRUE (Q.2110 page 64)
  _PollAfterRet = true;
}

SSCOPconn::~SSCOPconn() { }

State * SSCOPconn::Handle(Visitor *v)
{
  const VisitorType * vt2 = QueryRegistry(SSCOP_VISITOR_NAME);
  VisitorType vt1 = v->GetType();
  // get the current state name
  char name[80];
  _cs->Name(name);

  // some spacing
  DIAG("fsm.sscop", DIAG_DEBUG, cout << endl);
  DIAG("fsm.sscop", DIAG_DEBUG, cout << endl);

  // is it a SSCOPVisitor?
  if (vt2 && vt1.Is_A(vt2))
    {
      SSCOPVisitor *sv = (SSCOPVisitor *)v;
      if(sv->GetVT() == SSCOPVisitor::SDInd)
	{
	  DIAG("fsm.sscop", DIAG_DEBUG, cout <<
	       OwnerName() << " received a " << sv->GetType() << 
	       " v = " << sv << 
	       " in state = " << name <<
	       " at time " << theKernel().CurrentTime() << " seqnum = " << sv->GetS() << endl);
	}
      else
	{
	  DIAG("fsm.sscop", DIAG_DEBUG, cout <<
	       OwnerName() << " received a " << sv->GetType() << 
	       " v = " << sv <<
	       " in state = " << name <<
	       " at time " << theKernel().CurrentTime() << endl);
	}
      int ret = sv->Protocol(this);
      if(ret == -1)
	sv->Suicide();
      else
	if(_cs->StateID() == sscop_ready)
	  SendQueuedSD();
    }
  else
    PassThru(v);
  delete vt2;
  return(this);
}

void SSCOPconn::Send2SSCF(SSCOPVisitor *v)
{
  PassVisitorToA(v);
}

void  SSCOPconn::Interrupt(SimEvent *event) { }

void SSCOPconn::GetStateName(char *name)
{
  strcpy(name,_StateName);
}

int SSCOPconn::IncVTCC(void) { _vt_cc++; return _vt_cc; }
// _vt_ps must be Modulo24 increment TOFIX
int SSCOPconn::IncVTPS(void) { _vt_ps++; return _vt_ps; }
// _vt_sq must be Modulo8 increment TOFIX
int SSCOPconn::IncVTSQ(void)
{
  _vt_sq = (_vt_sq + 1) & 0xff; return _vt_sq;
}

int SSCOPconn::TXBase(int x) { return ((x - _vt_a) & 0xffffff);}
int SSCOPconn::RXBase(int x) { return ((x - _vr_r) & 0xffffff);}

void SSCOPconn::ChangeState(sscopState *s, char *StateName)
{
  DIAG("fsm.sscop", DIAG_DEBUG, cout << OwnerName() << ": +++++++++++++  from = " << _StateName << " to = " << StateName << endl);
  _cs = s;
  strcpy(_StateName, StateName);
}

void SSCOPconn::SetTimerCC(void)
{ _timer_cc->_active = 1; Register(_timer_cc); }

void SSCOPconn::SetTimerPOLL(void)
{ _timer_poll->_active = 1; Register(_timer_poll); }

void SSCOPconn::SetTimerNORESP(void)
{ _timer_noresp->_active = 1; Register(_timer_noresp); }

void SSCOPconn::SetTimerKEEPALIVE(void)
{ _timer_keepalive->_active = 1; Register(_timer_keepalive); }

void SSCOPconn::SetTimerIDLE(void)
{ _timer_idle->_active = 1; Register(_timer_idle); }

void SSCOPconn::StopTimerCC(void)
{ _timer_cc->_active = 0; Cancel(_timer_cc); }

void SSCOPconn::StopTimerPOLL(void)
{ _timer_poll->_active = 0; Cancel(_timer_poll); }

void SSCOPconn::StopTimerNORESP(void)
{ _timer_noresp->_active = 0; Cancel(_timer_noresp); }

void SSCOPconn::StopTimerKEEPALIVE(void)
{ _timer_keepalive->_active = 0; Cancel(_timer_keepalive); }

void SSCOPconn::StopTimerIDLE(void)
{ _timer_idle->_active = 0; Cancel(_timer_idle); }

sscopState * SSCOPconn::GetCS(void) { return _cs; }

void SSCOPconn::establish_req(SSCOPVisitor *v)
{  _cs->AA_EstablishReq(this,v);  }

void SSCOPconn::establish_resp(SSCOPVisitor *v)
{  _cs->AA_EstablishResp(this,v);  }

void SSCOPconn::release_req(SSCOPVisitor *v)
{  _cs->AA_ReleaseReq(this,v);  }

void SSCOPconn::resync_req(SSCOPVisitor *v)
{  _cs->AA_ResyncReq(this,v);  }

void SSCOPconn::resync_resp(SSCOPVisitor *v)
{  _cs->AA_ResyncResp(this,v);  }

void SSCOPconn::recover_resp(SSCOPVisitor *v)
{  _cs->AA_RecoverResp(this,v);  }

void SSCOPconn::unitdata_req(SSCOPVisitor *v)
{  _cs->AA_UNITDATAReq(this,v);  }

void SSCOPconn::maa_data_req(SSCOPVisitor *v)
{  _cs->MAA_UNITDATAReq(this,v);  }

void SSCOPconn::establish_ind(SSCOPVisitor *v)
{ }

void SSCOPconn::establish_conf(SSCOPVisitor *v)
{ }
 
void SSCOPconn::release_ind(SSCOPVisitor *v)
{  cout << " Lost connection\n";  }

void SSCOPconn::release_conf(SSCOPVisitor *v)
{ }

void SSCOPconn::resync_ind(SSCOPVisitor *v)
{ }

void SSCOPconn::resync_conf(SSCOPVisitor *v)
{ }

void SSCOPconn::recover_ind(SSCOPVisitor *v)
{ }

void SSCOPconn::udata_ind(SSCOPVisitor *v)
{ }

void SSCOPconn::retreive_ind(SSCOPVisitor *v)
{ }

void SSCOPconn::retreive_comp(SSCOPVisitor *v)
{ }

void SSCOPconn::maa_data_ind(SSCOPVisitor *v)
{ }


void SSCOPconn::ReSendVisitorToPeer(SSCOPVisitor *sv)
{
  char name[40];
  _cs->Name(name);
  if(sv == 0)
    {
      DIAG("fsm.sscop", DIAG_DEBUG, cout << OwnerName() << ": ********  Trying to ReSend v = 0" << endl);
      return;
    }
  if(sv->GetVT() == SSCOPVisitor::SDReq)
    {
      DIAG("fsm.sscop", DIAG_DEBUG, cout <<
	   OwnerName() << " ReSending a " << sv->GetType() << 
	   " in state = " << name <<
	   " at time " << theKernel().CurrentTime() << " seqnum = " << sv->GetS() << endl);
    }
  else
    {
      DIAG("fsm.sscop", DIAG_DEBUG, cout <<
	   OwnerName() << " ReSending a " << sv->GetType() << 
	   " in state = " << name <<
	   " at time " << theKernel().CurrentTime() << endl);
    }
  Send2Peer(sv);
}

void SSCOPconn::Send2Peer(SSCOPVisitor *sv)
{
  assert(sv != 0);
  char name[40];
  _cs->Name(name);

  if(sv->GetVT() == SSCOPVisitor::SDReq)
    {
      DIAG("fsm.sscop", DIAG_DEBUG, cout <<
	   OwnerName() << " Sending a " << sv->GetType() << 
	   " in state = " << name <<
	   " at time " << theKernel().CurrentTime() << " seqnum = " << sv->GetS() << endl);
    }
  else
    {
      DIAG("fsm.sscop", DIAG_DEBUG, cout <<
	   OwnerName() << " Sending a " << sv->GetType() << 
	   " in state = " << name <<
	   " at time " << theKernel().CurrentTime() << endl);
    }
  switch(sv->GetVT())
    {
    case SSCOPVisitor::BeginReq:
      _last_bgn = new BeginVisitor(SSCOPVisitor::BeginReq,0,0);
      _last_bgn->_mr = sv->_mr;
      _last_bgn->_sq = sv->_sq;
      break;
    case SSCOPVisitor::EndReq:
      _last_end = new EndVisitor(SSCOPVisitor::EndReq,0,0);
      _last_end->_sbit = sv->_sbit;
      break;
    case SSCOPVisitor::ResyncReq:
      _last_rs = new ResyncVisitor(SSCOPVisitor::ResyncReq,0,0);
      _last_rs->_mr = sv->_mr;
      _last_rs->_sq = sv->_sq;
      break;
    case SSCOPVisitor::RecovReq:
      _last_er = new RecovVisitor(SSCOPVisitor::RecovReq,0,0);
      _last_er->_mr = sv->_mr;
      _last_er->_sq = sv->_sq;
      break;
    default:
      break;
    }
  // need to mapping from sscop to visitor/pdu of some fields
  sv->MapFields(this);
  sv->PackPDU();
  PassVisitorToB(sv);
}

void SSCOPconn::SendQueuedSD(void)
{
  // I must be in ready state to forward SDs
  if(_cs->StateID() != sscop_ready)
    return;
  while(!_RTQueue.empty() || _vt_s < _vt_ms || _timer_idle->Active())
    {
      if(_RTQueue.empty())
	{
	  
	  if(!_TXQueue.empty())
	    {
	      // Transmission queue is NOT empty
	      if(_vt_s < _vt_ms)
		{
		  SSCOPVisitor *v1 = _TXQueue.pop();
		  SSCOPVisitor *v2 = v1->Copy();
		  v2->SetS(_vt_s);
		  Send2Peer(v1);
		  v2->_saved_ps = _vt_ps;
		  _TXBuffer.append(v2);
		  _vt_s++; // FIX to modulo24
		  DoLabelB();
		}
	      else
		{
		  StopTimerIDLE();
		  SetTimerNORESP();
		  DoLabelA();
		}
	    }
	  else
	    return;
	}
      else
	{
	  // Retransmission queue is NOT empty
	  SSCOPVisitor *x = _RTQueue.pop();
	  int x_seqnum = x->GetS();
	  ReSendVisitorToPeer(x);
	  // we need to update the _save_ps in the TXBuffer
	  if(!(x = SearchTXBuffer(x_seqnum)))
	    {
	      cout << "can't update StoredPS in TXBuffer seqnum = " << x_seqnum << endl;
	      abort();
	    }
	  x->_saved_ps = _vt_ps;
	  if(_PollAfterRet)
	    {
	      if(_RTQueue.empty())
		{
		  DoLabelA();
		}
	    }
	  // otherwise we do label B in both cases
	  DoLabelB();
	}
    } //eow
}

void SSCOPconn::MAAError(char *name, char code, SSCOPVisitor *v)
{
  // report to management thru and MAAErrorVisitor
  cout << "State(" << name << "): reporting error code  " << code << " to management " << endl;
}



void SSCOPconn::SendUSTAT(int first,int last)
{
  u_int buffer[2];

  buffer[0] = htonl(first);
  buffer[1] = htonl(last);
  //  SendPDU(SSCOP_USTAT,buffer,8);
}

// macros from pages 49-51 of Q.2110

void SSCOPconn::ReleaseBuffers() 
{
  _TXQueue.clear();
  _TXBuffer.clear();
  _RTQueue.clear();
  _RXBuffer.clear();
}

void SSCOPconn::PrepareRetrieval() 
{
  if(_cb)
    {
      _TXQueue.clear();
      _TXBuffer.clear();
    }
  _RTQueue.clear();
  _RXBuffer.clear();
}

void SSCOPconn::PrepareRecovery()
{
  if (_cb)
    {
      _TXQueue.clear();
      _TXBuffer.clear();
    }
  _RTQueue.clear();
}

void SSCOPconn::ClearTransmitter()
{
  if (!_cb)
    {
      _TXQueue.clear();
      _TXBuffer.clear();
    }
}

void SSCOPconn::DeliverData()
{
  SSCOPVisitor *v = 0;
  if (!_cb)
    {
      // deliver the data in sequence
      DIAG("fsm.sscop", DIAG_DEBUG, cout << OwnerName() <<  "SSCOPconn::DeliverData()" << endl);
      if(!_RXBuffer.empty())
	_RXBuffer.sort();
      PrintRXBuffer();
      while(!_RXBuffer.empty())
	{
	  v = _RXBuffer.pop();
	  Send2SSCF(v);
	}
    }
  else
    _RXBuffer.clear();
}

void SSCOPconn::InitStateVariables() // 20.49 
{
  _vt_s = _vt_ps = _vt_a = 0;
  _vt_pa = 1;
  _vt_pd = 0;
  _Credit = 1;
  _vr_r = _vr_h = 0;
}

// TODO
void SSCOPconn::DataRetrieval()
{
}


int SSCOPconn::DetectRetransmission(int sq) 
{
  if(_vr_sq == sq)
    return 1;
  _vr_sq = sq;
  return 0;
}

void SSCOPconn::SetPollTimer() 
{
  if (_TXQueue.empty() && _vt_s == _vt_a)
    SetTimerKEEPALIVE();
  else
    SetTimerPOLL();
}

void SSCOPconn::ResetDataTransferTimers()
{
  StopTimerPOLL();
  StopTimerKEEPALIVE();
  StopTimerNORESP();
  StopTimerIDLE();
}

void SSCOPconn::SetDataTransferTimers()
{
  SetTimerPOLL();
  SetTimerNORESP();
}

void SSCOPconn::InitVRMR(){ _vr_mr = _MR;}


void SSCOPconn::ClearBuffers(int br)
{
  _RXBuffer.clear();
  _TXBuffer.clear();
}


int SSCOPconn::Modulo24(int x) { return ((x) & ((1 << 24)-1)); }



void SSCOPconn::UpdateVRMR() { _vr_mr = Modulo24(_vr_r+MR); }


SSCOPVisitor *SSCOPconn::SearchRXBuffer(int s)
{
  SSCOPVisitor *v = 0;
  list_item itv;
  forall_items(itv,_RXBuffer)
    {
      v = _RXBuffer.contents(itv);
      if(v && (v->GetS() == s))
	return v;
    }
  return (SSCOPVisitor *)0;
}

SSCOPVisitor *SSCOPconn::SearchRTQueue(int s)
{
  SSCOPVisitor *b = 0;
  list_item itb;
  forall_items(itb,_RTQueue)
    {
      b = _RTQueue.contents(itb);
      if(b && (b->GetS() == s))
	return b;
    }
  return (SSCOPVisitor *)0;
}

/* Removes all SDUs whose seqnum is between
 * from_s and to_s inclusive and returns the number
 * of removed SDUs
 */

int SSCOPconn::RemFromTXBuffer(int from_s, int to_s)
{
  if(to_s < from_s || _TXBuffer.length() == 0)
    return 0;
  DIAG("fsm.sscop", DIAG_DEBUG, cout << OwnerName() << "SSCOPconn::RemFromTXBuffer(" << from_s << "," << to_s << ")" <<  endl);
  DIAG("fsm.sscop", DIAG_DEBUG, cout << OwnerName() << "    TXBuffer has " << _TXBuffer.length()  << " PDUs" << endl);
  int count = 0;
  SSCOPVisitor *b = 0;
  list_item cit = _TXBuffer.first(); // current list_item
  list_item nit; // // next list_item
  while(nit = _TXBuffer.succ(cit))
    {
      b = _TXBuffer.inf(cit);
      int seqnum = b->GetS();
      if(seqnum >= from_s && seqnum <= to_s)
	{
	  b = _TXBuffer.del_item(cit);
	  DIAG("fsm.sscop", DIAG_DEBUG, cout << "   removed PDU = " << seqnum << endl);
	  b->Suicide();
	  count++;
	}
      cit = nit;
    }
  // still have the last one that does not have a successor
  b = _TXBuffer.inf(cit);
  int seqnum = b->GetS();
  if(seqnum >= from_s && seqnum <= to_s)
    {
      b = _TXBuffer.del_item(cit);
      DIAG("fsm.sscop", DIAG_DEBUG, cout << "   removed PDU = " << seqnum << endl);
      b->Suicide();
      count++;
    }
  return count;
}

/* Removes all SDUs whose seqnum is between
 * from_s and to_s inclusive and returns the number
 * of removed SDUs
 */

int SSCOPconn::RemFromRXBuffer(int from_s, int to_s)
{
  if(to_s < from_s || _RXBuffer.length() == 0)
    return 0;
  int count = 0;
  SSCOPVisitor *b = 0;
  list_item cit = _RXBuffer.first(); // current list_item
  list_item nit; // // next list_item
  while(nit = _RXBuffer.succ(cit))
    {
      b = _RXBuffer.inf(cit);
      int seqnum = b->GetS();
      if(seqnum >= from_s && seqnum <= to_s)
	{
	  b = _RXBuffer.del_item(cit);
	  b->Suicide();
	  count++;
	}
      cit = nit;
    }
  // still have the last one that does not have a successor
  b = _RXBuffer.inf(cit);
  int seqnum = b->GetS();
  if(seqnum >= from_s && seqnum <= to_s)
    {
      b = _RXBuffer.del_item(cit);
      DIAG("fsm.sscop", DIAG_DEBUG, cout  << "   removed PDU = " << seqnum << endl);
      b->Suicide();
      count++;
    }
  return count;
}




SSCOPVisitor *SSCOPconn::GetFromRXBuffer(int s)
{
  SSCOPVisitor *v;
  list_item it;
  bool found = false;
  forall_items(it, _RXBuffer)
    {
      v = _RXBuffer.inf(it);
      if(v->GetS() == s)
	{
	  found = true;
	  break;
	}
    }
  if(found)
    {
      _RXBuffer.del_item(it);
      return v;
    }
  return (SSCOPVisitor *)0;
}


SSCOPVisitor *SSCOPconn::SearchTXBuffer(int s)
{
  SSCOPVisitor *b = 0;

  list_item itb;
  forall_items(itb,_TXBuffer)
    {
      b = _TXBuffer.contents(itb);
      if(b && (b->GetS() == s))
	return b;
    }
  return (SSCOPVisitor *)0;
}

void SSCOPconn::DoLabelA()
{
  _vt_ps++;
  SSCOPVisitor *sv = new PollVisitor(SSCOPVisitor::PollReq,0,0);
  Send2Peer(sv);
  _vt_pd = 0;
  SetTimerPOLL();
  return;
}

void SSCOPconn::DoLabelB()
{
  _vt_pd++;
  if(_timer_poll->Active())
    {
      if(_vt_pd >= _MaxPD)
	DoLabelA();
      return;
    }
  // TimerPOLL not active
  if(_timer_idle->Active())
    {
      SetTimerIDLE();
      SetTimerNORESP();
    }
  else
    SetTimerKEEPALIVE();
  if(_vt_pd >= _MaxPD)
    DoLabelA();
  else
    SetTimerPOLL();
  return;
}

void SSCOPconn::DoLabelD()
{
  _vt_cc = 1;
  _vt_sq++; // Modulo ?
  // init VR(MR)
  _vr_mr = _MR;
  SSCOPVisitor *sv = new RecovVisitor(SSCOPVisitor::RecovReq,0,0);
  assert(sv);
  Send2Peer(sv);
  PrepareRecovery();
  SetTimerCC();
  ChangeState(sscop_outrec::Instance(),"sscop_outrec");
}

void SSCOPconn::DoLabelL()
{
  char name[40];
  _cs->Name(name);
  if(_vt_s < _vt_ms)
    {
      if(_Credit == 0)
	{
	  _Credit = 1;
	  MAAError(name,'X',0);
	}
    }
  else
    {
      if(_Credit)
	{
	  _Credit = 0;
	  MAAError(name,'W',0);
	}
    }
  if(_timer_poll->Active())
    SetTimerNORESP();
  else
    if(!_timer_idle->Active())
      {
	StopTimerKEEPALIVE();
	StopTimerNORESP();
	SetTimerIDLE();
      }
}



/*
 * The code closely follows the SDL charts in page 66
 * 1. Save SD PDU is receive buffer means append a SSCOPVisitor to _RXBuffer
 *    list and incrementing modulo 24 _vr_mr.
 *
 * 2. Receiver buffer available means BufferCreate return a
 *    non-zero pointer to a SSCOPVisitor
 */

// March 16, 98

void SSCOPconn::DataSD(SSCOPVisitor *v)
{
  assert(v);
  int s = v->GetS();
  if(s < _vr_mr)
    { // SD.N(S) < VR(MR)
      if(s == _vr_r)
	{ // SD.N(S) == VR(R)
	  if(s == _vr_h)
	    { // SD.N(S) == VR(H)
	      PassThru(v);
	      _vr_r = s+1;
	      _vr_h = s+1;
	      return;
	    }
	  // SD.N(S) != VR(H)
	  // look thru the RXBuffer and keep delivering
	  // SD PDUs whose s is equal to _vr_r
	  do {
	    PassThru(v);
	    _vr_r += 1;
	    if(_RXBuffer.length() == 0)
	      return;
	  } while(v = GetFromRXBuffer(_vr_r));
	  return;
	}
      // case SD.N(S) != VR(R)
      if(s == _vr_h)
	{
	  // SD.N(S) = VR(H)
	  _RXBuffer.append(v);
	  PrintRXBuffer();
	  _vr_mr = Modulo24(_vr_r + MR);
	  _vr_h++;
	  return;
	}
      if(_vr_h < s)
	{
	  // SD.N(S) > VR(H)
	  _RXBuffer.append(v);
	  PrintRXBuffer();
	  _vr_mr = Modulo24(_vr_r + MR);
	  // send an USTAT PDU with list elem 1 = _vr_h and list elem 2 = s
	  SendUSTAT(_vr_h,s);
	  _vr_h = s + 1;
	  return;
	}
      // SD.N(S) < VR(H)
      if(SearchRXBuffer(v->GetS()))
	{
	  // SD.N(s) is in RXBuffer
	  ResetDataTransferTimers();
	  char name[40];
	  _cs->Name(name);
	  MAAError(name,'Q',v);
	  DoLabelD();
	  v->Suicide();
	  return;
	}
      // SD.N(s) is not in RXBuffer
      // this test is not in the SDL charts
      if(s < _vr_r)
	{
	  DIAG("fsm.sscop", DIAG_DEBUG, cout  << " seqnum BELOW VR(R): seqnum = " << s << " VR(R) = " << _vr_r << endl);
	  DIAG("fsm.sscop", DIAG_DEBUG, cout  << " suicide visitor = " << hex << v << endl);
	  cout << dec << endl;
	  v->Suicide();
	  return;
	}
      _RXBuffer.append(v);
      PrintRXBuffer();
      _vr_mr = Modulo24(_vr_r + MR);
      return;
    }
  // SD.N(S) >= VR(MR)
  if(_vr_h < _vr_mr)
    {
      SendUSTAT(_vr_h,_vr_mr);
      _vr_h = _vr_mr;
    }
}



// March 16, 98
void SSCOPconn::DataPOLL(SSCOPVisitor *v)
{
  int ps = v->_ps;
  int s = v->_s;
  DIAG("fsm.sscop", DIAG_DEBUG, cout << OwnerName() << "Received a POLL: ps = " << ps << " s = " << s << endl);
  int i;
  int ListLen;

  if(_vr_h > s)
    {
      ResetDataTransferTimers();
      char name[40];
      _cs->Name(name);
      MAAError(name,'Q',v);
      DoLabelD();
      v->Suicide();
      return;
    }
  if(_vr_mr < s)
    _vr_h = _vr_mr;
  else
    _vr_h = s;
  // Do Label K here 
  // Start by updating VR(MR) just in case it has not been done yet
  _vr_mr = Modulo24(_vr_r+MR); 
  i = _vr_r;
  // nothing in the list (list is an array of ints of size MaxSTAT)
  ListLen = 0; 
  // cases when i >= _vr_h are bundled together and 
  // done last at the end of the while loop
  while(i < _vr_h)
    {
      if(SearchRXBuffer(i))
	i++;
      else
	{
	  _list[ListLen++] = i;
	  if(ListLen >= _MaxSTAT)
	    {
	      SSCOPVisitor *s = new StatVisitor(SSCOPVisitor::StatReq,0,0);
	      s->_ps = ps;
	      s->_list = new int[ListLen];
	      s->_llen = ListLen;
	      for(int k=0; k < ListLen; k++)
		s->_list[k] = _list[k];
	      Send2Peer(s);
	      // start a new list
	      ListLen = 0;
	      _list[ListLen++] = i;
	    }
	  do
	    i++;
	  while(i < _vr_h && !SearchRXBuffer(i));
	  _list[ListLen++] = i;
	}
    }
  if(i > _vr_h)
    _list[ListLen++] = i;
  SSCOPVisitor *sv = new StatVisitor(SSCOPVisitor::StatReq,0,0);
  sv->_ps = ps;
  sv->_list = new int[ListLen];
  sv->_llen = ListLen;
  for(int k=0; k < ListLen; k++)
    sv->_list[k] = _list[k];
  Send2Peer(sv);
  v->Suicide();
}

void SSCOPconn::DataUSTAT(SSCOPVisitor *v)
{
  int r = v->GetR();
  int mr = v->GetMR();
  
  SSCOPVisitor *b = 0;
  char name[40];
  _cs->Name(name);

  if(_vt_a <= r && r < _vt_s)
    {
      // remove from TXBuffer all SD PDUs whose sequence number is
      // between _vt_a and r-1.
      RemFromTXBuffer(_vt_a,r-1);
      _vt_a = r;
      _vt_ms = mr;
      int seq1 = v->_list[0];
      int seq2 = v->_list[1];
      if(_vt_a <= seq1 && seq2 < _vt_s)
	{ // Do label G
	  while(b = SearchTXBuffer(seq1))
	    {
	      _RTQueue.append(b->Copy());
	      seq1++;
	      if(seq1 == seq2)
		{
		  MAAError(name,'V',v);
		  v->Suicide();
		  return;
		}
	    }
	  ResetDataTransferTimers();
	  // I have noway of passing count = seq1-seq2
	  // maybe somewhere in the visitor to FIX.
	  MAAError(name,'T',v); 
	  DoLabelD();
	}
      else
	{ // Do label F
	  ResetDataTransferTimers();
	  MAAError(name,'I',v);
	  DoLabelD();
	}
    }
  else
    { // Do Label F here
      ResetDataTransferTimers();
      MAAError(name,'I',v);
      DoLabelD();
    }
  v->Suicide();
}


/* pages 70-72
 * The charts are missing the generation of an SD PDU queued up
 * in the case when ( i > 1) is FALSE after doing label L:
 * so if you send W PDUs and they all get there with no errors you
 * will receive a STAT with no list (i = 0) and you will not
 * attempt to send more PDUs. To fix this I added a call to SendQueuedSD()
 */

void SSCOPconn::DataSTAT(SSCOPVisitor *v)
{
  int ps = v->GetPS();
  int r = v->GetR();
  int mr = v->GetMR();
  int *list = v->_list;
  SSCOPVisitor *b = 0;
  
  /* Receiver says:
   * a) This is a response to POLL ps
   * b) I can accept PDUs up to mr-1
   * c) the next expected PDU is r
   */
  DIAG("fsm.sscop", DIAG_DEBUG, cout << OwnerName() << "Received a STAT: ps = " << ps << " mr = " << mr << " r = " << r);
  if(v->_llen)
    {
      DIAG("fsm.sscop", DIAG_DEBUG, cout << "  LIST = {");
      for(int k = 0; k < v->_llen-1; k++)
	DIAG("fsm.sscop", DIAG_DEBUG, cout << list[k] << ",");
      DIAG("fsm.sscop", DIAG_DEBUG, cout << list[v->_llen - 1] << "}" << endl);
    }
  else
    DIAG("fsm.sscop", DIAG_DEBUG, cout << endl);

  char name[40];
  _cs->Name(name);
  if(_vt_pa <= ps && ps <= _vt_ps)
    {
      if(_vt_a <= r && r <= _vt_s)
	{
	  // Remove from _TXBuffer all SD PDUs whose
	  // seqnum is between _vt_a and r-1 
	  RemFromTXBuffer(_vt_a,r-1);
	  _vt_a = r;
	  _vt_pa = ps;
	  _vt_ms = mr;
	  // get the number of elements in the STAT list
	  int i = v->_llen;
	  int count = 0;
	  int seq1, seq2;
	  if(i > 1)
	    {
	      seq1 = *list++; 
	      i--;
	      if(seq1 < _vt_s)
		{
		  // do I here
		  do {
		    seq2 = *list++;
		    i--;
		    if(seq1 < seq2 && seq2 <= _vt_s)
		      {
			do {
			  if(!(b = SearchTXBuffer(seq1)))
			    {
			      // do H here
			      ResetDataTransferTimers();
			      MAAError(name,'S',v);
			      DoLabelD();
			    }
			  else
			    {
			      // we avoid the Remove and Save 
			      int StoredPS = b->GetSavedPS();
			      if(StoredPS < ps &&  ps <= _vt_ps && !SearchRTQueue(seq1))
				{
				  // stick it in the RTQueue
				  _RTQueue.append(b->Copy());
				  count++;
				  SendQueuedSD(); // April 8,98
				}
			      seq1++;
			    }
			} while(seq1 != seq2);
			// do J
			if( i > 0)
			  {
			    seq2 = *list++;
			    i--;
			    if(seq1 < seq2 && seq2 <= _vt_s)
			      {
				do {
				  if(_cb)
				    RemFromTXBuffer(seq1,seq1);
				  seq1++;
				} while(seq1 != seq2);
			      }
			    else
			      {
				// do H
				ResetDataTransferTimers();
				MAAError(name,'S',v);
				DoLabelD();
			      }
			  }
			else
			  {
			    MAAError(name,'V',v);
			    DoLabelL();
			  }
		      }
		    else
		      {
			// do H
			ResetDataTransferTimers();
			MAAError(name,'S',v);
			DoLabelD();
		      }
		  } while (i > 0);
		  // no way of passing count again to FIX
		  MAAError(name,'V',v);
		  DoLabelL();
		}
	      else
		{
		  // do H here
		  ResetDataTransferTimers();
		  MAAError(name,'S',v);
		  DoLabelD();
		}
	    }
	  else
	    {
	      DoLabelL(); // do L 
	      SendQueuedSD(); // April 8,98 missing in the SDL charts
	    }
	}
      else
	{
	  ResetDataTransferTimers();
	  MAAError(name,'S',v);
	  DoLabelD();
	}
    }
  else
    {
      ResetDataTransferTimers();
      MAAError(name,'R',v);
      DoLabelD();
    }
  v->Suicide();
}












