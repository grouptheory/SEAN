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
#ifndef __ATM_PDU_VISITOR_H__
#define __ATM_PDU_VISITOR_H__

#ifndef LINT
static char const _ATM_PDU_Visitor_h_rcsid_[] =
"$Id: ATM_PDU_Visitor.h,v 1.5 1998/08/21 16:01:29 mountcas Exp $";
#endif

#include <FW/basics/Visitor.h>

#define ATM_PDU_VISITOR_NAME "ATM_PDU"

class Buffer;

class ATM_PDU_Visitor : public Visitor {
public:

  ATM_PDU_Visitor(int vpi, int vci, u_char * data, int dlen);
  ATM_PDU_Visitor(int vpi, int vci, Buffer * buf);
  ATM_PDU_Visitor(const ATM_PDU_Visitor & rhs);

  int GetVPI(void) const;
  int GetVCI(void) const;
  int GetPort(void) const;

  void SetVPI(int vp);
  void SetVCI(int vc);
  void SetPort(int p);

  Buffer * TakeBuffer(void);

  const VisitorType GetType(void) const;

protected:

  virtual ~ATM_PDU_Visitor();
  virtual Visitor * dup(void) const;

  int      _vpi;
  int      _vci;
  int      _port;
  Buffer * _buf;
  //  u_char * _data;
  //  int      _dlen;

  const vistype & GetClassType(void) const;
  static vistype _my_type;
};

#endif
