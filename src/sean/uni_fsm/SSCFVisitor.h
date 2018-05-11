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

//* -*- C++ -*-
#ifndef LINT
static char const _SSCFVisitor_h_rcsid_[] =
"$Id: SSCFVisitor.h,v 1.3 1998/08/25 21:33:36 mountcas Exp $";
#endif

#ifndef _SSCFV_H_
#define _SSCFV_H_


#include <FW/basics/Visitor.h>
#include <FW/kernel/Kernel.h>
#include <FW/actors/State.h>

#define SSCF_VISITOR_NAME           "sscfVisitor"


class sscfVisitor : public Visitor {
public:
  enum sscfVisitorType {
    EstErr  =  0,
    EstReq  =  1,
    EstInd  = -1,
    EstResp =  2,
    EstConf = -2,
    RelReq  =  3,
    RelInd  = -3,
    RelResp =  4,
    RelConf = -4,
    DataReq =  5,
    DataInd = -5
};

  sscfVisitor(sscfVisitorType vt);
  virtual ~sscfVisitor();
  void Name(char *name);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;

  sscfVisitorType GetVT(void) const;
  void SetVT(sscfVisitorType vt);
  void TurnIntoReq(void);
  void TurnIntoInd(void);
  bool IsReq(void) const;
  bool IsInd(void) const;

protected:

  virtual Visitor * dup(void) const;

private:

  sscfVisitor(vistype& child_type, sscfVisitorType vt);
  static vistype _my_type;
  sscfVisitorType _vt;
};

#endif
