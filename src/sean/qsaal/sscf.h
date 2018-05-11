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

#ifndef SSCF_H
#define SSCF_H
#ifndef LINT
static char const _sscf_h_rcsid_[] =
"$Id: sscf.h,v 1.1 1998/07/20 11:11:52 bilal Exp $";
#endif

#include <sys/types.h>
#include <FW/kernel/Kernel.h>
#include <FW/actors/State.h>
#include <FW/kernel/Handlers.h>

class SSCOPVisitor;

#define SSCF_VISITOR_NAME "sscfVisitor"

class SSCFconn : public State {
public:
  SSCFconn();
  virtual ~SSCFconn();
  State * Handle(Visitor *v);
  void    Interrupt(SimEvent *e);
private:
  static const VisitorType* _uni_visitor_type;
  static const VisitorType* _sscf_visitor_type;
  static const VisitorType* _sscop_visitor_type;
};

#endif



