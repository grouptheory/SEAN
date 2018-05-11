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
static char const _QSAALExpander_cc_rcsid_[] =
"$Id: qsaal_expander.cc,v 1.4 1998/08/21 15:34:42 mountcas Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/basics/Conduit.h>
#include <FW/basics/diag.h>
#include <FW/behaviors/Protocol.h>
#include <FW/kernel/Handlers.h>
#include <FW/behaviors/Adapter.h>
#include <FW/actors/Terminal.h>

#include "sscop.h"
#include "sscf.h"

#ifdef __FORE_AAL__
#include "fore_aal.h"
#else
#include "fake_aal.h"
#endif

#include "qsaal_expander.h"


QSAALExpander::QSAALExpander()
{
  SSCFconn *sscf_fsm  = new SSCFconn();
  Protocol * sscf_protocol = new Protocol(sscf_fsm);
  _sscf = new Conduit("SSCF", sscf_protocol);

  SSCOPconn * sscop_fsm = new SSCOPconn();
  Protocol  * sscop_protocol = new Protocol(sscop_fsm);
  _sscop = new Conduit("SSCOP", sscop_protocol);

#ifdef __FORE_AAL__
  SSCOPaal * aal_fsm = new fore_SSCOPaal();
#else
  SSCOPaal * aal_fsm = new fake_SSCOPaal();
#endif

  Protocol  * aal_proto = new Protocol(aal_fsm);
  _aal = new Conduit("AAL",aal_proto);

  Join(B_half(_sscf), A_half(_sscop));
  Join(B_half(_sscop), A_half(_aal));

  DefinitionComplete();
}

QSAALExpander::~QSAALExpander(void)
{
  delete _sscf;
  delete _sscop;
  delete _aal;
}


Conduit * QSAALExpander::GetAHalf(void) const
{
  return A_half(_sscf);
}


Conduit * QSAALExpander::GetBHalf(void) const
{
  return B_half(_aal);
}
