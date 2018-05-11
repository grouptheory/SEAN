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

#ifndef _PARTY_CC_
#define _PARTY_CC_

#ifndef LINT
static char const _Q93bParty_cc_rcsid_[] =
"$Id: Q93bParty.cc,v 1.10 1998/08/06 04:04:24 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/basics/diag.h>
#include <FW/kernel/Kernel.h>
#include <FW/actors/State.h>
#include <FW/kernel/SimEvent.h>

#include <sys/types.h>
#include <codec/uni_ie/ie.h>

#include "Q93bState.h"
#include "Q93bCall.h"
#include "Q93bParty.h"
#include "PartyState.h"
#include "Q93bTimers.h"



Party::Party(Call *ca, ie_called_party_num *cpn, ie_end_pt_ref  *epr) :_ca(ca),_dest(cpn),_epr_ie(epr)
{
  _eps_ie = new ie_end_pt_state(ie_end_pt_state::p_null);
  _ps = P0_Null::Instance();
  _dest_subaddr=0L;
  _t397 = new Q93b_t397(this);
  _t398 = new Q93b_t398(this);
  _t399 = new Q93b_t399(this);
}


Party::~Party()
{
  // do not delete _ps !!!
  if(_t398)
    {
      _t398->StopTimer();
      delete _t398;
    }
  if(_t399)
    {
      _t399->StopTimer();
      delete _t399;
    }
  if(_epr_ie)
    delete _epr_ie;
  if(_eps_ie)
    delete _eps_ie;
}

void Party::ChangeState(PartyState *s)
{
  _ps = s;
  set_eps(_ps->StateID());
}


PartyState *Party::GetPS()
{
  return _ps;
}

void Party::SetOwner(Call *c)
{
  _ca = c;
}

Call *Party::GetOwner(void)
{
  return  _ca;
}

void Party::set_called_number(ie_called_party_num *cn_ie){
  // Don't clobber _dest if it's the same as cn_ie.
  if (_dest && _dest != cn_ie) delete _dest;
  _dest=cn_ie;
}
 
void Party::set_called_subaddr(ie_called_party_subaddr *cs_ie){
  // Don't clobber _dest if it's the same as cn_ie.
  if (_dest_subaddr && _dest_subaddr != cs_ie) delete _dest_subaddr;
  _dest_subaddr=cs_ie;
}

InfoElem* Party::get_called_number(void){
  return _dest;
}

InfoElem* Party::get_called_subaddr(void){
  return _dest_subaddr;
}

ie_end_pt_state::party_state Party::get_eps()
{
  return(_eps_ie->get_epr_state());
}

void Party::set_eps(ie_end_pt_state::party_state state) {
  _eps_ie->set_epr_state(state);
}

int Party::get_epr() {
  return(_epr_ie->get_epr_value());
}

void  Party::set_epr(int epr) {
  _epr_ie->set_epr_value(epr);
}

void  Party::set_seqnum(int seqnum) {
  _leaf_seqnum = seqnum;
}

int  Party::get_seqnum() {
  return  _leaf_seqnum ;
}

void  Party::set_ls(Party::LeafStates ls) {
  _ls = ls;
}

Party::LeafStates  Party::get_ls() {
  return  _ls;
}



InfoElem * Party::get_epr_ie()
{
  return _epr_ie;
}

InfoElem * Party::get_eps_ie()
{
  return _eps_ie;
}

void Party::set_epr_ie(InfoElem *epr_ie) {
  _epr_ie = (ie_end_pt_ref *) epr_ie;
}


void Party::set_eps_ie(InfoElem *eps_ie) {
  _eps_ie = (ie_end_pt_state *)eps_ie;
}

void  Party::q93b_t397_timeout()
{
  u_char timer_diag[3] = {'3','9','7'};
  int ret = GetPS()->party_t397_timeout(this);
  if(ret == -2)
    _ca->KillParty(this);
}

void  Party::q93b_t398_timeout()
{
  u_char timer_diag[3] = {'3','9','8'};
  int ret = GetPS()->party_t398_timeout(this);
  if(ret == -2)
    _ca->KillParty(this);
}

void  Party::q93b_t399_timeout()
{
  u_char timer_diag[3] = {'3','9','9'};
  int ret = GetPS()->party_t399_timeout(this);
  if(ret == -2)
    _ca->KillParty(this);
}


void Party::SetT397()
{
  _active_timer =_t397;
  _ca->SetT397(this);
}

void Party::SetT398()
{
  _active_timer =_t398;
  _ca->SetT398(this);
}

void Party::SetT399()
{
  _active_timer =_t399;
  _ca->SetT399(this);
}

void Party::StopTimer()
{
  if(_active_timer)
    {
      _ca->StopTimer(this);
      _active_timer = 0L;
    }
}

void Party::StopT397()
{
  _active_timer =_t397;
  _ca->StopT397(this);
}

void Party::StopT398()
{
  _active_timer =_t398;
  _ca->StopT398(this);
}

void Party::StopT399()
{
  _active_timer =_t399;
  _ca->StopT399(this);
}


#endif







