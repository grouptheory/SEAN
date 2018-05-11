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
static char const _xtio_cc_rcsid_[] =
"$Id: xtio.cc,v 1.6 1998/08/06 04:04:28 bilal Exp $";
#endif
#include <common/cprototypes.h>
#include <iostream.h>
extern "C" {
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/stream.h>
#include <sys/types.h>
#include <tiuser.h>
#include <fore_xti/xti_user_types.h>
#include <fore_xti/xti.h>
#include <fore_xti/xti_atm.h>
#include <fore_xti/ans.h>
};

#include "sean_io.h"

#define DEVICE          "/dev/xtipvc0"

int xtio_open(int *max_pdu)
{
  struct t_info info;
  int fd = t_open(DEVICE, O_RDWR, &info);
  if(fd < 0) {
      cout << "t_open failed\n";
      return -1;
  } 
  *max_pdu = info.options;
  return fd;
}



int xtio_attach(int _fd, int vpi, int vci, int qos_kb)
{
  char *options = NULL;
  if(t_bind(_fd, (struct t_bind *) NULL, (struct t_bind *) NULL) < 0)
    {
      cout << "t_bind failed\n";
      exit(1);
    }
  // no QoS, don't send TRAFFIC option 
  struct t_call call_req;
  PVCAddress addr_req;
  addr_req.addressType=AF_ATM_PVC;
  addr_req.vpi = vpi;
  addr_req.vci = vci;
  memset(&call_req, 0, sizeof(call_req));
  call_req.addr.len = sizeof(addr_req);
  call_req.addr.buf = (char *) &addr_req;
  cout << "Connecting to (" << vpi << " , " << vci << ") ..... " << endl;
  options = (char *) NULL;
  if(t_connect(_fd, &call_req, (struct t_call *) NULL) < 0)
    {
      cout << "t_connect failed " << endl;
      exit(1);
    }

  sean_io* sio = The_io_table()->Lookup(_fd);
  assert(sio);
  sio->_patrol = new SVC_DataHandler(_fd);

  return _fd;
}

int xtio_read(int fd, caddr_t buffer, int len)
{
  // we want NON BLOCKING READS -so check for data with select
  // return a length of 0 if no data
  static bool error_state=false;
  int bytes_read = 0;
  fd_set readfds;
  struct timeval timeout; timeout.tv_sec=0; timeout.tv_usec=0; // no wait
  FD_ZERO (&readfds);
  FD_SET( fd, &readfds);

  int ready = select (fd+1, &readfds, (fd_set *)0, (fd_set *)0, &timeout);
  
  if (ready == 1) {
    int flags;
    bytes_read = t_rcv(fd, buffer, len, &flags);
    error_state = false;
  } else if (ready == -1) {
    if (!error_state) {
      error_state = true;
      perror("xtio_read():select");
    }
  }
  return bytes_read;
}


int xtio_write(int fd, caddr_t buffer, int len)
{
  return t_snd(fd, buffer, len,0);
}



int xtio_close(int fd)
{
  sean_io* sio = The_io_table()->Lookup(fd);
  assert(sio);
  delete sio->_patrol;
  sio->_patrol = 0;

  if( t_unbind(fd) < 0) {
    t_error("t_unbind");
  }

  int close_val = 0;
  if( (close_val = t_close(fd)) < 0) {
    t_error("t_close");
  }
  return close_val;
}
