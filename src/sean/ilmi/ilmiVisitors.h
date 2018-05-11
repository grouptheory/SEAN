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

#ifndef _ILMIVISITORS
#define _ILMIVISITORS
#ifndef LINT
static char const _ilmiVisitors_h_rcsid_[] =
"$Id: ilmiVisitors.h,v 1.13 1998/09/01 22:12:55 marsh Exp $";
#endif

#include <FW/basics/Visitor.h>

#include <snmp_pp.h>

#define ILMI_VISITOR_NAME "ilmiVisitor"
#define TRAP_VISITOR_NAME "TrapVisitor"
#define GET_VISITOR_NAME "GetVisitor"
#define RESP_VISITOR_NAME "RespVisitor"
#define LINKUP_VISITOR_NAME "LinkUpVisitor"
#define START_VISITOR_NAME "StartVisitor"
#define SET_VISITOR_NAME "SetVisitor"

class ilmiVisitor : public Visitor
{
public:
  ilmiVisitor();
  ilmiVisitor(Pdu pdu, int id);
  const VisitorType GetType() const;
  Pdu GetPDU();
  void SetPDU(Pdu);
  int GetID();
  void SetID(int);
protected:
  ilmiVisitor(const ilmiVisitor &);
  ilmiVisitor(vistype &);
  const vistype &GetClassType() const;
  virtual ~ilmiVisitor();
private:
  Pdu _pdu;
  int _id;
  static vistype _my_type;
};


class GetVisitor : public ilmiVisitor {
public:
  GetVisitor(Pdu pdu, int id);
  GetVisitor(const GetVisitor & rhs);
  virtual ~GetVisitor();
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual ilmiVisitor * dup(void) const;
private:
  static vistype _my_type;
};

class RespVisitor : public ilmiVisitor {
public:
  RespVisitor(Pdu pdu, int id);
  RespVisitor(const RespVisitor & rhs);
  virtual ~RespVisitor();
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual ilmiVisitor * dup(void) const;
private:
  static vistype _my_type;
};


class TrapVisitor : public ilmiVisitor {
public:
  TrapVisitor(Pdu pdu, int id);
  TrapVisitor(const TrapVisitor & rhs);
  virtual ~TrapVisitor();
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual ilmiVisitor * dup(void) const;
private:
  static vistype _my_type;
};


class SetVisitor : public ilmiVisitor {
public:
  SetVisitor(Pdu pdu, int id);
  SetVisitor(const SetVisitor & rhs);
  virtual ~SetVisitor();
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual ilmiVisitor * dup(void) const;
private:
  static vistype _my_type;
};


class LinkUpVisitor : public Visitor {
public:
  LinkUpVisitor();
  LinkUpVisitor(const LinkUpVisitor & rhs);
  virtual ~LinkUpVisitor();
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};


class StartVisitor : public Visitor {
public:
  StartVisitor();
  StartVisitor(const StartVisitor & rhs);
  virtual ~StartVisitor();
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};


#endif



