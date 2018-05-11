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
static char const _ilmiVisitors_cc_rcsid_[] =
"$Id: ilmiVisitors.cc,v 1.13 1998/09/01 22:12:42 marsh Exp battou $";
#endif
#include <common/cprototypes.h>
#include "ilmiVisitors.h"

vistype ilmiVisitor::_my_type(ILMI_VISITOR_NAME);
vistype TrapVisitor::_my_type(TRAP_VISITOR_NAME);
vistype GetVisitor::_my_type(GET_VISITOR_NAME);
vistype RespVisitor::_my_type(RESP_VISITOR_NAME);
vistype LinkUpVisitor::_my_type(LINKUP_VISITOR_NAME);
vistype StartVisitor::_my_type(START_VISITOR_NAME);
vistype SetVisitor::_my_type(SET_VISITOR_NAME);

ilmiVisitor::ilmiVisitor() : Visitor(_my_type)
{
  SetLoggingOn();  
}

ilmiVisitor::ilmiVisitor(Pdu pdu, int id) : Visitor(_my_type)
{
  _pdu = pdu;
  _id = id;
  SetLoggingOn();  
}

ilmiVisitor::ilmiVisitor(const ilmiVisitor &rhs) : Visitor(_my_type) { }

ilmiVisitor::ilmiVisitor(vistype &child_type) : Visitor(child_type.derived_from(_my_type))
{
}

ilmiVisitor::~ilmiVisitor()
{
}

Pdu ilmiVisitor::GetPDU() 
{
  return(_pdu);
}

void ilmiVisitor::SetPDU(Pdu pdu)
{
  _pdu = pdu;
}


int ilmiVisitor::GetID() 
{
  return(_id);
}

void ilmiVisitor::SetID(int id)
{
  _id = id;
}

const VisitorType ilmiVisitor::GetType() const
{
  return(VisitorType(_my_type));
}

const vistype &ilmiVisitor::GetClassType() const
{
  return(_my_type);
}


//----------------------------------------------------------------

TrapVisitor::TrapVisitor(Pdu pdu, int id) : 
  ilmiVisitor(pdu, id)
{
  SetLoggingOn();  
}

TrapVisitor::TrapVisitor(const TrapVisitor & rhs) :  ilmiVisitor(rhs) { }

TrapVisitor::~TrapVisitor() { }

const vistype & TrapVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType TrapVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

ilmiVisitor *TrapVisitor::dup(void) const { return new TrapVisitor(*this); }


//----------------------------------------------------------------

GetVisitor::GetVisitor(Pdu pdu, int id) : 
  ilmiVisitor(pdu, id)
{
  SetLoggingOn();  
}

GetVisitor::GetVisitor(const GetVisitor & rhs) :  ilmiVisitor(rhs) { }

GetVisitor::~GetVisitor() { }

const vistype & GetVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType GetVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

ilmiVisitor *GetVisitor::dup(void) const { return new GetVisitor(*this); }


//----------------------------------------------------------------

RespVisitor::RespVisitor(Pdu pdu, int id) : 
  ilmiVisitor(pdu, id)
{
  SetLoggingOn();  
}

RespVisitor::RespVisitor(const RespVisitor & rhs) :  ilmiVisitor(rhs) { }

RespVisitor::~RespVisitor() { }

const vistype & RespVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType RespVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

ilmiVisitor *RespVisitor::dup(void) const { return new RespVisitor(*this); }


//----------------------------------------------------------------

LinkUpVisitor::LinkUpVisitor() : Visitor(_my_type)
{
  SetLoggingOn();  
}

LinkUpVisitor::LinkUpVisitor(const LinkUpVisitor & rhs) :  Visitor(rhs) { }

LinkUpVisitor::~LinkUpVisitor() { }

const vistype & LinkUpVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType LinkUpVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

Visitor *LinkUpVisitor::dup(void) const { return new LinkUpVisitor(*this); }


//----------------------------------------------------------------

StartVisitor::StartVisitor() : Visitor(_my_type)
{
  SetLoggingOn();  
}

StartVisitor::StartVisitor(const StartVisitor & rhs) :  Visitor(rhs) { }

StartVisitor::~StartVisitor() { }

const vistype & StartVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType StartVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

Visitor *StartVisitor::dup(void) const { return new StartVisitor(*this); }


//----------------------------------------------------------------

SetVisitor::SetVisitor(Pdu pdu, int id) : 
  ilmiVisitor(pdu, id)
{
  SetLoggingOn();  
}

SetVisitor::SetVisitor(const SetVisitor & rhs) :  ilmiVisitor(rhs) { }

SetVisitor::~SetVisitor() { }

const vistype & SetVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType SetVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

ilmiVisitor *SetVisitor::dup(void) const { return new SetVisitor(*this); }





