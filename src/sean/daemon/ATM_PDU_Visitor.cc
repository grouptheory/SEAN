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
static char const _ATM_PDU_Visitor_cc_rcsid_[] =
"$Id: ATM_PDU_Visitor.cc,v 1.7 1998/08/21 16:15:24 mountcas Exp $";
#endif
#include <common/cprototypes.h>

#include "ATM_PDU_Visitor.h"
#include <codec/q93b_msg/Buffer.h>
extern "C" {
#include <string.h>
};

ATM_PDU_Visitor::ATM_PDU_Visitor(int vpi, int vci, u_char * data, int datalen)
  : Visitor(_my_type), _vpi(vpi), _vci(vci), _port(-1)
{
  _buf = new Buffer(2048);
  memcpy(_buf->data(), data, datalen);
  _buf->set_length( datalen );
}

ATM_PDU_Visitor::ATM_PDU_Visitor(int vpi, int vci, Buffer * buf)
  : Visitor(_my_type), _vpi(vpi), _vci(vci), _port(-1), _buf(buf) { }

ATM_PDU_Visitor::ATM_PDU_Visitor(const ATM_PDU_Visitor & rhs)
  : Visitor(rhs), _vpi(rhs._vpi), _vci(rhs._vci), _port(rhs._port)
{ 
  // How do we handle the buffer here?
  _buf = new Buffer(2048);
  memcpy(_buf->data(), rhs._buf->data(), rhs._buf->length());
  _buf->set_length( rhs._buf->length() );
}

ATM_PDU_Visitor::~ATM_PDU_Visitor() 
{ 
  delete _buf;
}

int ATM_PDU_Visitor::GetVPI(void) const  { return _vpi; }
int ATM_PDU_Visitor::GetVCI(void) const  { return _vci; }
int ATM_PDU_Visitor::GetPort(void) const { return _port; }

void ATM_PDU_Visitor::SetVPI(int vp)  { _vpi = vp; }
void ATM_PDU_Visitor::SetVCI(int vc)  { _vci = vc; }
void ATM_PDU_Visitor::SetPort(int p)  { _port = p; }

Buffer * ATM_PDU_Visitor::TakeBuffer(void)
{
  Buffer * rval = _buf;
  _buf = 0;
  return rval;
}

const VisitorType ATM_PDU_Visitor::GetType(void) const
{
  return VisitorType( GetClassType() );
}

const vistype & ATM_PDU_Visitor::GetClassType(void) const
{
  return _my_type;
}

Visitor * ATM_PDU_Visitor::dup(void) const
{
  return new ATM_PDU_Visitor(*this);
}
