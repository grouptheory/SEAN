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

#ifndef __FORE_AAL_H__
#define __FORE_AAL_H__
// -*- C++ -*-
#ifndef LINT
static char const _fore_aal_h_rcsid_[] =
"$Id: fore_aal.h,v 1.1 1998/07/20 11:11:52 bilal Exp $";
#endif

// *******************************************************
// *******************************************************
//
//    DO NOT INCLUDE THIS FILE fore_aal.h IN YOUR CODE
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
class sean_io;

class fore_SSCOPaal;

class DriverReader : public InputHandler {
public:
  DriverReader(fore_SSCOPaal *owner, int fd);
  virtual ~DriverReader();
  void Callback(void);
private:
  fore_SSCOPaal *_owner;
};

class fore_SSCOPaal : public SSCOPaal {
friend class DriverReader;
 public:
  fore_SSCOPaal(void);
  virtual ~fore_SSCOPaal();

  State* Handle(Visitor* v);
  void Interrupt(class SimEvent *e);

 private:
  void Read(void);
  SSCOPVisitor *DecodePDU(u_char *pdu, int size);

  int _max_TSDU;
  InputHandler *_reader;
  int _fd;
};

#endif
