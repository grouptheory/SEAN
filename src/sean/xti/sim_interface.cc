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
static char const _sim_interface_cc_rcsid_[] =
"$Id: sim_interface.cc,v 1.7 1998/08/06 04:04:27 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "sean_io.h"
#include "sim_interface.h"
#include <FW/actors/Terminal.h>
#include <sean/api/Call_API.h>
#include <sean/api/ATM_Interface.h>
#include <sean/ipc/IPC_Visitors.h>
#include <sean/cc/sean_Visitor.h>
#include <codec/q93b_msg/Buffer.h>
#include <sean/ipc/tcp_ipc.h>




//------------------------------------------------------
int sim_interface_open(void) {
  diag("api.sim_interface", DIAG_INFO, "Performing open...\n");
  return The_io_table()->New_Fake_FD();
}

//------------------------------------------------------
int sim_interface_close(int fd)
{
  diag("api.sim_interface", DIAG_INFO, "Performing close...\n");
  return 0;
}

//------------------------------------------------------
int sim_interface_attach(int fd, int vpi, int vci, int Qos)
{
  diag("api.sim_interface", DIAG_INFO, "Performing attach...\n");
  return 0;
}

//------------------------------------------------------
int sim_interface_read(int fd, caddr_t buffer, int len)
{
  diag("api.sim_interface", DIAG_INFO, "Performing read...\n");
  return 0;
}

//------------------------------------------------------
int sim_interface_write(int fd, caddr_t buffer, int len)
{
  diag("api.sim_interface", DIAG_INFO, "Performing write...\n");
  sean_io* sio = The_io_table()->Lookup(fd);
  assert(sio);

  int vp                   = sio->_vp;
  int vc                   = sio->_vc;
  int ignore               = -1;
  ATM_Interface* interface = sio->_interface;
  assert(interface);

  Buffer * buf = new Buffer(len + 16);
  unsigned char* data = buf->data();
  sean_Visitor::sean_Visitor_Type op = sean_Visitor::raw_simulation_pdu;

  int index=0;
  // first four bytes of data are the op code
  bcopy((const void *)(&op), (void *)(data + index * sizeof(int)), sizeof(int)); 
  index++; 
  // next four bytes of data (header) are the cid, which will 
  // be ignored when the visitor enters the StreamFilter from A
  bcopy((const void *)(&ignore), (void *)(data + index * sizeof(int)), sizeof(int)); 
  index++; 
  // next four bytes of data are the vpi
  bcopy((const void *)(&vp), (void *)(data + index * sizeof(int)), sizeof(int)); 
  index++; 
  // following four bytes of data are the vci
  bcopy((const void *)(&vc), (void *)(data + index * sizeof(int)), sizeof(int)); 
  index++; 
  // now the payload
  bcopy((const void *)buffer, (void *)(data + index * sizeof(int)), len); 

  buf->set_length(len+16);

  assert(interface->_connection_id);
  IPC_DataVisitor * adv = new IPC_DataVisitor( interface->_connection_id, buf );
  interface->_ipc_injector_terminal->Absorb(adv);

  return len;
}




