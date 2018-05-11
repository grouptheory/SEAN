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
static char const _SSCFVisitor_cc_rcsid_[] =
"$Id: SSCFVisitor.cc,v 1.4 1998/08/25 21:33:35 mountcas Exp $";
#endif
#include <common/cprototypes.h>

#include "SSCFVisitor.h"


sscfVisitor::sscfVisitor(sscfVisitorType vt) :
  Visitor(_my_type),_vt(vt)
{
  SetLoggingOn();
}

sscfVisitor::sscfVisitor(vistype& child_type, sscfVisitorType vt) : 
  Visitor(child_type.derived_from(_my_type)),_vt(vt)
{
  SetLoggingOn();
}

sscfVisitor::~sscfVisitor() { }

void sscfVisitor::Name(char *name)
{
  switch (GetVT()) {
    case EstErr:
      strcpy(name,"Establish Error Visitor");
      break;
    case EstReq:
      strcpy(name,"Establish Request Visitor");
      break;
    case EstInd:
      strcpy(name,"Establish Indication Visitor");
      break;
    case EstResp:
      strcpy(name,"Establish Response Visitor");
      break;
    case EstConf:
      strcpy(name,"Establish Confirm Visitor");
      break;
    case RelReq:
      strcpy(name,"Release Request Visitor");
      break;
    case RelInd:
      strcpy(name,"Release Indication Visitor");
      break;
    case RelResp:
      strcpy(name,"Release Response Visitor");
      break;
    case RelConf:
      strcpy(name,"Release confirm Visitor");
      break;
    case DataReq:
      strcpy(name,"Data Request Visitor");
      break;
    case DataInd:
      strcpy(name,"Data Indication Visitor");
      break;
  }
}


const vistype & sscfVisitor::GetClassType(void) const
{
  return _my_type;
}

const VisitorType sscfVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

sscfVisitor::sscfVisitorType sscfVisitor::GetVT(void) const { return _vt; }

void sscfVisitor::SetVT(sscfVisitor::sscfVisitorType vt) { _vt = vt; }

void sscfVisitor::TurnIntoReq(void)
{
  if (_vt < 0)
    _vt = (sscfVisitorType)-_vt;
}

void sscfVisitor::TurnIntoInd(void)
{
  if (_vt > 0)
    _vt = (sscfVisitorType)-_vt;
}

bool sscfVisitor::IsReq(void) const
{
  if (_vt > 0)
    return true;
  return false;
}

bool sscfVisitor::IsInd(void) const
{
  if (_vt < 0)
    return true;
  return false;
}

Visitor * sscfVisitor::dup(void) const
{
  return new sscfVisitor(_vt);
}
