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
#ifndef __HOST_H__
#define __HOST_H__

#ifndef LINT
static char const _Host_h_rcsid_[] =
"$Id: Host.h,v 1.4 1998/07/31 03:46:50 bilal Exp $";
#endif

#include <FW/actors/Expander.h>
#include <sean/templates/constants.h>

#include "SigStack.h"

class NSAP_DCC_ICD_addr;
class Application;
class Switch;

#define IPC_LAYER_NAME     "IPC"
#define STREAM_FILTER_NAME "StreamFilter"

class Host : public Expander {
public:

  Host(int listenport, int switchport=NO_PORT);
  Host(char * application_file, int switchport);

  Conduit * GetAHalf(void) const;
  Conduit * GetBHalf(void) const;

protected:

  virtual ~Host();

  void setup_stack(SigStack::type stack_type);

private:
  Conduit     * _userstack_c;
  SigStack    * _userstack;

  // used by the Host(int listenport,
  //                  int switchport);
  Conduit     * _ipc;
  Conduit     * _stream_filter;

  // used by the Host(char * application_file,
  //                  int switchport);
  Conduit     * _app_c;  
  Application * _app;
};

Conduit* attach_Host(NSAP_DCC_ICD_addr* address, 
		     Conduit* swc, Switch* sw, int switchport,
		     int listenport);
Conduit* attach_Host(NSAP_DCC_ICD_addr* address, 
		     Conduit* swc, Switch* sw, int switchport,
		     char * application_file);

void Fill_Address(NSAP_DCC_ICD_addr* address, char* namebuf);

#endif
