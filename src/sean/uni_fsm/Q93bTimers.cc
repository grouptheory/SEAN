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

#ifndef LINT
static char const _Q93bTimers_cc_rcsid_[] =
"$Id: Q93bTimers.cc,v 1.2 1998/08/06 04:04:25 bilal Exp $";
#endif
#include <common/cprototypes.h>

#ifndef _UNI_TIMERS_CC_
#define _UNI_TIMERS_CC_

#include "Q93bTimers.h"
#include "Q93bCall.h"
#include "Q93bParty.h"
#include "Q93bLeaf.h"
#include "Coordinator.h"

CallTimer::CallTimer(CallTimers t, State *ca, int max_retries):TimerHandler((double )t),
  _call(ca),_max_retries(max_retries)
{
  _timer_diag[0] = '0';
  _timer_diag[1] = '0';
  _timer_diag[2] = '0';
}

CallTimer::~CallTimer() { }

Call * CallTimer::get_call()
{
  return (Call *)_call;
}


int CallTimer::SetTimer()
{
  _active = true;
  Register(this);
  return 1;
}

int CallTimer::StopTimer()
{
  _active = false;
  Cancel(this);
  return 1;

}

void CallTimer::SetActiveON() {_active = true;}
void CallTimer::SetActiveOFF() {_active = false;}

bool CallTimer::IsActive() {return _active;}


//------------- T301: Alerting message  ----------------
// started after sending an Alerting  message

Q93b_t301::Q93b_t301(State *ca, int max_retries): CallTimer(t301,ca,max_retries) {
  _timer_diag[0] = '3';
  _timer_diag[1] = '0';
  _timer_diag[2] = '1';
}

CallTimer::CallTimerType Q93b_t301::Timer_type()
{
  return timer_t301;
}

void Q93b_t301::Callback()
{
  assert(_call!=0);
  ((Call *)_call)->q93b_t301_timeout();
}


//------------- T303: setup message  ----------------
// started after sending a setup  message

Q93b_t303::Q93b_t303(State *ca, int max_retries): CallTimer(t303,ca,max_retries) {
  _timer_diag[0] = '3';
  _timer_diag[1] = '0';
  _timer_diag[2] = '3';
}

CallTimer::CallTimerType Q93b_t303::Timer_type()
{
  return timer_t303;
}

void Q93b_t303::Callback()
{
  assert(_call!=0);
  ((Call *)_call)->q93b_t303_timeout();
}


//------------- T308: release message  ----------------
// started after sending a RELEASE  message

Q93b_t308::Q93b_t308(State *ca, int max_retries): CallTimer(t308,ca,max_retries) {
  _timer_diag[0] = '3';
  _timer_diag[1] = '0';
  _timer_diag[2] = '8';
}
CallTimer::CallTimerType Q93b_t308::Timer_type()
{
  return timer_t308;
}

void Q93b_t308::Callback()
{
  assert(_call!=0);
  ((Call *)_call)->q93b_t308_timeout();
}

//------------- T309 ----------------
// started on receipt of: AAL_RELEASE_INDICATION from SSCF

Q93b_t309::Q93b_t309(State *ca, int max_retries): CallTimer(t309,ca,max_retries) {
  _timer_diag[0] = '3';
  _timer_diag[1] = '0';
  _timer_diag[2] = '9';
}
CallTimer::CallTimerType Q93b_t309::Timer_type()
{
  return timer_t309;
}

void Q93b_t309::Callback()
{
  assert(_call!=0);
  ((Coordinator *)_call)->ExpT309();
}

//------------- T310 : call_proceeding  ----------------
// started after sending a CALL_PROCEEDING  message

Q93b_t310::Q93b_t310(State *ca, int max_retries): CallTimer(t310,ca,max_retries) {
  _timer_diag[0] = '3';
  _timer_diag[1] = '1';
  _timer_diag[2] = '0';
}
CallTimer::CallTimerType Q93b_t310::Timer_type(){return timer_t310;}

void Q93b_t310::Callback()
{
  assert(_call!=0);
  ((Call *)_call)->q93b_t310_timeout();
}

//------------- T313 : connect message  ----------------
// started after sending a CONNECT  message

Q93b_t313::Q93b_t313(State *ca, int max_retries): CallTimer(t313,ca,max_retries) {
  _timer_diag[0] = '3';
  _timer_diag[1] = '1';
  _timer_diag[2] = '3';
}
CallTimer::CallTimerType Q93b_t313::Timer_type(){return timer_t313;}


void Q93b_t313::Callback()
{
  assert(_call!=0);
  ((Call *)_call)->q93b_t313_timeout();
}

//------------- T316 ----------------
// started after sending a RESTART message

Q93b_t316::Q93b_t316(State *ca, int max_retries): CallTimer(t316,ca,max_retries) {
  _timer_diag[0] = '3';
  _timer_diag[1] = '1';
  _timer_diag[2] = '6';
}
CallTimer::CallTimerType Q93b_t316::Timer_type(){return timer_t316;}

void Q93b_t316::Callback()
{
  ((Call *)_call)->q93b_t316_timeout();
}

//------------- T317 ----------------
// started on receipt of a RESTART message

Q93b_t317::Q93b_t317(State *ca, int max_retries): CallTimer(t317,ca,max_retries) {
  _timer_diag[0] = '3';
  _timer_diag[1] = '1';
  _timer_diag[2] = '7';
}
CallTimer::CallTimerType Q93b_t317::Timer_type(){return timer_t317;}

void Q93b_t317::Callback()
{
  ((Call *)_call)->q93b_t317_timeout();
}

//------------- T322: status_enq message  ----------------
// started after sending a STATUS_ENQ  message

Q93b_t322::Q93b_t322(State *ca, int max_retries): CallTimer(t322,ca,max_retries) {
  _timer_diag[0] = '3';
  _timer_diag[1] = '2';
  _timer_diag[2] = '2';
}
CallTimer::CallTimerType Q93b_t322::Timer_type(){return timer_t322;}

void Q93b_t322::Callback()
{
  ((Call *)_call)->q93b_t322_timeout();
}

//====================== PARTY TIMERS ==================================================

PartyTimer::PartyTimer(PartyTimers t, Party *p, int max_retries):TimerHandler((double )t),
  _party(p),_max_retries(max_retries)
{
  _timer_diag[0] = '0';
  _timer_diag[1] = '0';
  _timer_diag[2] = '0';
}

PartyTimer::~PartyTimer() { }

int PartyTimer::SetTimer()
{
  Register(this);
  return 1;
}

int PartyTimer::StopTimer()
{
  Cancel(this);
  return 1;

}

Party  * PartyTimer::GetParty()
{
  return _party;
}

//------------- T397: Alerting or AddPartyAlerting  message  ----------------
// started after receiving an ALERTING or ADD_PARTY_ALERTING  message

Q93b_t397::Q93b_t397(Party *p, int max_retries): PartyTimer(t397,p) {
  _timer_diag[0] = '3';
  _timer_diag[1] = '9';
  _timer_diag[2] = '7';
}

PartyTimer::PartyTimerType Q93b_t397::Timer_type(){return timer_t397;}

void Q93b_t397::Callback()
{
  assert(_party != 0);
  _party->q93b_t397_timeout();
}

//------------- T398: drop_party message  ----------------
// started after sending a DROP_PARTY  message

Q93b_t398::Q93b_t398(Party *p, int max_retries): PartyTimer(t398,p,max_retries) {
  _timer_diag[0] = '3';
  _timer_diag[1] = '9';
  _timer_diag[2] = '8';
}

PartyTimer::PartyTimerType Q93b_t398::Timer_type(){return timer_t398;}

void Q93b_t398::Callback()
{
  assert(_party != 0);
  _party->q93b_t398_timeout();
}

//------------- T399: add_party  message  ----------------
// started after sending an ADD_PARTY  message

Q93b_t399::Q93b_t399(Party  *p, int max_retries): PartyTimer(t399,p,max_retries) {
  _timer_diag[0] = '3';
  _timer_diag[1] = '9';
  _timer_diag[2] = '9';
}

PartyTimer::PartyTimerType Q93b_t399::Timer_type(){return timer_t399;}

void Q93b_t399::Callback()
{
  assert(_party != 0);
  _party->q93b_t399_timeout();
}

//-------------------- Leaf Timers ---------------------------

LeafTimer::LeafTimer(LeafTimers t, Leaf *l, int max_retries):TimerHandler((double )t),
  _Leaf(l),_max_retries(max_retries)
{
  _timer_diag[0] = '0';
  _timer_diag[1] = '0';
  _timer_diag[2] = '0';
}

LeafTimer::~LeafTimer() { }

int LeafTimer::SetTimer()
{
  Register(this);
  return 1;
}

int LeafTimer::StopTimer()
{
  Cancel(this);
  return 1;

}

Leaf  * LeafTimer::GetLeaf()
{
  return _Leaf;
}


Q93b_t331::Q93b_t331(Leaf *l, int max_retries): LeafTimer(t331,l,max_retries) {
  _timer_diag[0] = '3';
  _timer_diag[1] = '3';
  _timer_diag[2] = '1';
}

void Q93b_t331::Callback()
{
  assert(_Leaf != 0);
  _Leaf->ExpT331();
}


#endif
