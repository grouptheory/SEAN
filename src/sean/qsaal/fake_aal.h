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

#ifndef __FAKE_AAL_H__
#define __FAKE_AAL_H__
// -*- C++ -*-
#ifndef LINT
static char const _fake_aal_h_rcsid_[] =
"$Id: fake_aal.h,v 1.1 1998/07/20 11:11:52 bilal Exp $";
#endif

// *******************************************************
// *******************************************************
//
//    DO NOT INCLUDE THIS FILE fake_aal.h IN YOUR CODE
// 
//    Use the interface publicly defined in the base 
//    class sscop_aal.h
//
// *******************************************************
// *******************************************************

#include <FW/kernel/Handlers.h>
#include <FW/actors/State.h>

#include "sscop_aal.h"

class SSCOPVisitor;
class SSCOPaal;

class fake_SSCOPaal : public SSCOPaal {
 public:
  fake_SSCOPaal(void);
  virtual ~fake_SSCOPaal();

  State* Handle(Visitor* v);
  void Interrupt(class SimEvent *e);

  // void SetOther(fake_SSCOPaal *other){_other = other;} obsolete
  // fake_SSCOPaal(fake_SSCOPaal *other);

 private:

  int AALread(u_char *pdu, int len);
  int AALwrite(u_char *pdu, int len);
  SSCOPVisitor *DecodePDU(u_char *pdu, int size);

  // fake_SSCOPaal *GetOther(){return _other;}
  // fake_SSCOPaal *_other;                       obsolete
};

#endif



