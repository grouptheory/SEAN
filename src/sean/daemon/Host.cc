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
static char const _Host_cc_rcsid_[] =
"$Id: Host.cc,v 1.7 1998/08/06 06:16:01 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "Host.h"
#include "SigStack.h"
#include "Fiber.h"
#include "Application.h"
#include "Switch.h"
#include "SwitchFabric.h"
#include <FW/behaviors/Cluster.h>
#include <FW/behaviors/Adapter.h>
#include <FW/behaviors/Protocol.h>
#include <FW/basics/Conduit.h>
#include <codec/uni_ie/addr.h>

#include <sean/ipc/tcp_ipc.h>
#include <sean/daemon/Streams.h>

//----------------------------------------------------
Host::Host(int listen_port, int switchport) {

  if (switchport==NO_PORT) 
    setup_stack(SigStack::full_stack); // this is 'live'
  else 
    setup_stack(SigStack::sans_aal);   // this is simulation

  tcp_ipc_layer * tcp_term  = new tcp_ipc_layer(listen_port);
  Adapter       * tcp_adapt = new Adapter(tcp_term);
  Conduit       * tcp_con   = new Conduit(IPC_LAYER_NAME, tcp_adapt);

  StreamFilter  * str = new StreamFilter( );
  Protocol      * str_pro = new Protocol(str);
  Conduit       * str_filt = new Conduit(STREAM_FILTER_NAME, str_pro);

  call_control* cc = _userstack->Get_CC();
  str->Inform_Of_Call_Control(cc);

  Join(A_half(tcp_con),  A_half(str_filt));
  Join(B_half(str_filt), A_half(_userstack));

  DefinitionComplete();
}

//-----------------------------------------------------
Host::Host(char* file, int switchport) {

  if (switchport==NO_PORT) 
    abort();                           // this is nonsense
  else 
    setup_stack(SigStack::sans_aal);   // this is simulation

  char appname[80];
  sprintf(appname,"App.%s",file);

  _app = new Application(file, switchport);
  Protocol * app_proto = new Protocol (_app);
  _app_c = new Conduit(appname,app_proto);

  Join(A_half(_userstack_c), B_half(_app_c));

  DefinitionComplete();
}

//-----------------------------------------------------
Host::~Host() { }

//-----------------------------------------------------
void Host::setup_stack(SigStack::type stack_type) {
  
  _userstack = new SigStack(USER_SIDE, 
			    stack_type, 
			    NO_PORT);

  Cluster* userstack_clu = new Cluster(_userstack);
  _userstack_c = new Conduit("Host-Stack",userstack_clu);

  _app_c = 0;
  _app   = 0;
}

//-----------------------------------------------------
Conduit * Host::GetAHalf(void) const {
  if (_app_c) return A_half(_app_c);
  else return A_half(_userstack_c);
}

//-----------------------------------------------------
Conduit * Host::GetBHalf(void) const {
  return B_half(_userstack_c);
}

//-----------------------------------------------------

Conduit* attach_Host(NSAP_DCC_ICD_addr * address, 
		     Conduit* swc, Switch* sw, int switchport,
		     int listenport) {
  Host* h = new Host(listenport,switchport);
  Cluster* clu = new Cluster(h);
  Conduit* c = new Conduit("HOST", clu);

  sw->RegisterAddr( (NSAP_DCC_ICD_addr *)( address->copy() ),
		    switchport);

  Conduit * rval = 0;

  if (swc && c) {
    State    * st = new Fiber(switchport, switchport);
    Protocol * pr = new Protocol(st);
    rval = new Conduit("Link", pr);

    ConnectorVisitor * cv1 = new ConnectorVisitor( 0, switchport );

    if (!Join(B_half(swc), A_half(rval), cv1, 0)) {
      delete rval; rval = 0;
    } else if (!Join(B_half(rval), B_half(c))) {
      delete rval; rval = 0;
    }
  }

  return c;
}


//-----------------------------------------------------
Conduit* attach_Host(NSAP_DCC_ICD_addr * address, 
		     Conduit* swc, Switch* sw, int switchport,
		     char * application_file) {
  Host* h = new Host(application_file,switchport);
  Cluster* clu = new Cluster(h);
  Conduit* c = new Conduit("FILE-HOST", clu);

  sw->RegisterAddr( (NSAP_DCC_ICD_addr *)( address->copy() ),
		    switchport);

  Conduit * rval = 0;

  if (swc && c) {
    State    * st = new Fiber(switchport, switchport);
    Protocol * pr = new Protocol(st);
    rval = new Conduit("Link", pr);

    ConnectorVisitor * cv1 = new ConnectorVisitor( 0, switchport );

    if (!Join(B_half(swc), A_half(rval), cv1, 0)) {
      delete rval; rval = 0;
    } else if (!Join(B_half(rval), B_half(c))) {
      delete rval; rval = 0;
    }
  }

  return c;
}


//-----------------------------------------------------
void Fill_Address(NSAP_DCC_ICD_addr* address, char* namebuf) {
  u_char addrbuf[80];
  int len = address->encode(addrbuf);
  int j=0;
  for (int i=0;i<len;i++) {
    int high = addrbuf[i] >> 4;
    int low = addrbuf[i] & 0x0F;
    if (high>9) { namebuf[j]= 'A'+high-10; j++; }
    else { namebuf[j]= '0'+high; j++; }
    if (low>9) { namebuf[j]= 'A'+low-10; j++; }
    else { namebuf[j]= '0'+low; j++; }
  }
  namebuf[j]=0;
}

