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
static char const _sean_io_cc_rcsid_[] =
"$Id: sean_io.cc,v 1.16 1998/08/13 05:55:07 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "sean_io.h"
#include <iostream.h>
#ifdef __FORE_IO__
#include "xtio.h"
#endif

#ifdef __FAKE_IO__
#include "sim_interface.h"
#endif

#include <sean/templates/constants.h>
#include <sean/api/Call_API.h>
#include <sean/api/Callback_Mgmt.h>


long DRIVER_QOS                = (getenv("DRIVER_QOS") ? \
				 atoi(getenv("DRIVER_QOS")) : DEFAULT_DRIVER_QOS);

long MAX_PDU_SIZE              = (getenv("MAX_PDU_SIZE") ? \
				 atoi(getenv("MAX_PDU_SIZE")) : DEFAULT_MAX_PDU_SIZE);

long MAX_PDUS_TO_SUCK          = (getenv("MAX_PDUS_TO_SUCK") ? \
				 atoi(getenv("MAX_PDUS_TO_SUCK")) : DEFAULT_MAX_PDUS_TO_SUCK);

long ACTIVE_NUM_PDUS_TO_SUCK   = (getenv("ACTIVE_NUM_PDUS_TO_SUCK") ? \
				 atoi(getenv("ACTIVE_NUM_PDUS_TO_SUCK")) : DEFAULT_ACTIVE_NUM_PDUS_TO_SUCK);

long CLOSING_MAX_PDUS_TO_SUCK  = (getenv("CLOSING_MAX_PDUS_TO_SUCK") ? \
				 atoi(getenv("CLOSING_MAX_PDUS_TO_SUCK")) : DEFAULT_CLOSING_MAX_PDUS_TO_SUCK);


io_table* io_table::_singleton = 0;


//------------------------------------------------------
sean_io::sean_io() {
  zero();
  diag("api.data",DIAG_INFO,"              DRIVER_QOS =%ld\n",DRIVER_QOS);
  diag("api.data",DIAG_INFO,"            MAX_PDU_SIZE =%ld\n",MAX_PDU_SIZE);
  diag("api.data",DIAG_INFO,"        MAX_PDUS_TO_SUCK =%ld\n",MAX_PDUS_TO_SUCK);
  diag("api.data",DIAG_INFO," ACTIVE_NUM_PDUS_TO_SUCK =%ld\n",ACTIVE_NUM_PDUS_TO_SUCK);
  diag("api.data",DIAG_INFO,"CLOSING_MAX_PDUS_TO_SUCK =%ld\n",CLOSING_MAX_PDUS_TO_SUCK);
}

//------------------------------------------------------
sean_io::~sean_io() { 
  if (_fd != -1) close();
}

//------------------------------------------------------
void sean_io::zero(void) {
  _fd           = -1;
  _max_transfer = 0;
  _vp           = NO_VPI;
  _vc           = NO_VCI;
  _interface    = 0;
  _call         = 0;
  _frobber      = 0;
  _patrol       = 0;
  not_open();
}

//------------------------------------------------------
int sean_io::open(void)
{
#ifdef __FORE_IO__
  _fd = xtio_open(&_max_transfer);
  
  if (_fd != -1) {
    cout << "opened xti interface on fd: = " << _fd <<endl;
    _close  = xtio_close;
    _attach = xtio_attach;
    _read   = xtio_read;
    _write  = xtio_write;
  }
  return _fd;
#endif
#ifdef __FAKE_IO__
  _fd = sim_interface_open();
  
  if (_fd != -1) {
    cout << "opened simulated data interface on fake fd = " << _fd <<endl;
    _close  = sim_interface_close;
    _attach = sim_interface_attach;
    _read   = sim_interface_read;
    _write  = sim_interface_write;
  }
  return _fd;
#endif
}

//------------------------------------------------------
int sean_io::close()
{
  suck(CLOSING_MAX_PDUS_TO_SUCK,MAX_PDU_SIZE);

  int ret= _close(_fd);

  The_io_table()->Deregister(_fd);
  zero();
  return ret;
}

//------------------------------------------------------
int sean_io::attach(int vpi, int vci, int Qos)
{
  _vp        = vpi;
  _vc        = vci;
  _interface = ATM_Call::_current_interface;
  _call      = ATM_Call::_current_call;
  The_io_table()->Register(_fd,this);

  Qos = DRIVER_QOS;

  return _attach(_fd, vpi, vci, Qos);
}

//------------------------------------------------------
int sean_io::read(caddr_t buffer, int len)
{
  diag("api.data",DIAG_INFO,"<<((--beginning read()--\n");
  int bytes;
  if (bytes = peek_incoming_data_buffer() ) {

    list_item li = _incoming_pdus.first();
    Buffer * nextbuf = _incoming_pdus.inf(li);

    if (len >= bytes) {
      bcopy( nextbuf->data(), (void*)buffer, bytes);  // ship the whole PDU out
      _incoming_pdus.pop();
      delete nextbuf;
    }
    else {
      nextbuf->ShrinkHeader(len);
      bcopy( nextbuf->data(), (void*)buffer, len);    // ship out a small chunk
      nextbuf->set_length( bytes - len );
      bytes = len;
    }
  }

  // check to see if the user has a data
  // callback. do we need to cancel it?

  int remaining = peek_incoming_data_buffer();

  if (_frobber) {
    if (remaining==0) {
      theAPISimEntity()->dequeue( _frobber );
      _frobber = 0;
      diag("api.data",DIAG_INFO,"!!!!--removing frobber--\n");
    }
    else {
      diag("api.data",DIAG_INFO,"!!!!--leaving frobber intact()--\n");
    }
  }
  else {
    diag("api.data",DIAG_INFO,"!!!!--no frobber, you probably shouldn't be seeing this--\n");
  }

  diag("api.data",DIAG_INFO,">>))--ending read()--\n");

  return bytes;
}

//------------------------------------------------------
int sean_io::write(caddr_t buffer, int len)
{
  return _write(_fd, buffer, len);
}

//------------------------------------------------------
void sean_io::enqueue_incoming_data_buffer(Buffer* buf) {

  _incoming_pdus.append(buf);
  diag("api.data",DIAG_INFO,"$$$$--enqueuing buffer--\n");

  // check to see if the user has a data
  // callback already; do we need to 
  // make one?

  if (!_frobber) {
    Controller* cntl = _call->_control;
    if (cntl) {
      _frobber = theAPISimEntity()->enqueue(data_cc_c, cntl,_call);
      _frobber->Set_Associated_IO( this );
      diag("api.data",DIAG_INFO,"$$$$--no frobber, registering a callback--\n");
    }
  }
  else {
    diag("api.data",DIAG_INFO,"$$$$--there's already a frobber...-\n");
  }
}


//------------------------------------------------------
int sean_io::peek_incoming_data_buffer(void) {

  if (_incoming_pdus.empty()) {
    diag("api.data",DIAG_INFO,"!!!!--beginning suck inside peek()--\n");
    suck(ACTIVE_NUM_PDUS_TO_SUCK,MAX_PDU_SIZE);
  }

  if (!_incoming_pdus.empty()) {
    int ret = 0;
    list_item li = _incoming_pdus.first();
    Buffer * nextbuf = _incoming_pdus.inf(li);
    return nextbuf->length();
  }
  
  return 0;
}

//------------------------------------------------------
int sean_io::suck(int maxsuck_numpdus, int maxsuck_pdusize) {
  
  int capsize;
  if (maxsuck_pdusize > MAX_PDU_SIZE)
    capsize = MAX_PDU_SIZE;
  else 
    capsize = maxsuck_pdusize;

  u_char* data = _tmp->data();
  int read_so_far = 0;
  int bytes = 0;
  int num_pdus=0;

  diag("api.data",DIAG_INFO,">>>>--beginning suck phase-->>\n");
  do {
    bytes = _read(_fd, (char*)data, capsize);
    if (bytes>0) {
      Buffer* inc = new Buffer(bytes);
      bcopy(data, inc->data(), bytes);
      inc->set_length(bytes);
      enqueue_incoming_data_buffer( inc );
      read_so_far+=bytes;
      num_pdus++;
    }
  }
  while ((bytes>0) && (num_pdus < maxsuck_numpdus));
  diag("api.data",DIAG_INFO,"<<<<--ending suck phase--, got %d pdus<<\n", num_pdus);
  
  return num_pdus;
}


//------------------------------------------------------
void sean_io::drain(void) {
  while( ! _incoming_pdus.empty() ) {
    Buffer* buf = _incoming_pdus.pop();
    delete buf;
  };
  _incoming_pdus.clear();

  if (_frobber) {
    diag("api.data",DIAG_INFO,"%%%%--removing frobber after drain...--\n");
    theAPISimEntity()->dequeue( _frobber );
    _frobber = 0;
  }
  else {
    diag("api.data",DIAG_INFO,"%%%%--no frobber already after drain...--\n");
  }
}


//------------------------------------------------------
// default functions: user before an open() and also
// after a close() or a failed open()
//------------------------------------------------------
void sean_io::not_open() {
  _close  = not_open_close;
  _attach = not_open_attach;
  _read   = not_open_read;
  _write  = not_open_write;
}

int not_open_close(int fd) {
  cout << "Warning sean_io::close() called - not open" <<endl;
  return -1;
}

int not_open_attach(int fd, int vpi, int vci, int Qos) {
  cout << "Warning sean_io::attach() called - not open" <<endl;
  return -1;
}

int not_open_read(int fd, caddr_t buffer, int len) {
  cout << "Warning sean_io::read() called - not open" <<endl;
  return -1;
}

int not_open_write(int fd, caddr_t buffer, int len) {
  cout << "Warning sean_io::write() called - not open" <<endl;
  return -1;
}




//------------------------------------------------------
io_table* The_io_table(void) {
  if (!io_table::_singleton)
    io_table::_singleton=new io_table();
  return io_table::_singleton;
}

//------------------------------------------------------
bool io_table::Register(int fd, sean_io* sio) {
  dic_item di = _fd_data.lookup(fd);
  bool ret=false;
  if (!di) {
    _fd_data.insert(fd,sio);
    ret=true;
  }
  return ret;
}

//------------------------------------------------------
bool io_table::Deregister(int fd) {
  dic_item di = _fd_data.lookup(fd);
  bool ret=false;
  if (di) {
    _fd_data.del_item(di);
    ret=true;
  }
  return ret;
}

//------------------------------------------------------
sean_io* io_table::Lookup(int fd) {
  dic_item di = _fd_data.lookup(fd);
  sean_io* ret=0;
  if (di) {
    ret=_fd_data.inf(di);
  }
  return ret;
}

//------------------------------------------------------
io_table::io_table(void) : _last_fake_fd(6) {
}

//------------------------------------------------------
io_table::~io_table(void) {
  _fd_data.clear();
}

//------------------------------------------------------
int io_table::New_Fake_FD() {
  _last_fake_fd++;
  return _last_fake_fd;
}

//------------------------------------------------------
SVC_DataHandler::SVC_DataHandler(int fd) : InputHandler( theAPISimEntity() ,fd) {
  _sio = The_io_table()->Lookup(fd);
  Register(this);
  
  diag("api.data.svc_datahandler",DIAG_INFO, "oooo--Data patrol is registered for fd=%d.\n", GetFD());
}

//------------------------------------------------------
SVC_DataHandler::~SVC_DataHandler() {
}

//------------------------------------------------------
void SVC_DataHandler::Callback(void) {
  assert(_sio);
  diag("api.data",DIAG_INFO,"oooo--SVC DataHandler begins--\n");
  int sucked = _sio->suck(MAX_PDUS_TO_SUCK,MAX_PDU_SIZE);
  diag("api.data.svc_datahandler",DIAG_INFO, "Data patrol sucked out %d pdus from the SVC (fd=%d)...\n", sucked, GetFD());
  diag("api.data",DIAG_INFO,"oooo--SVC DataHandler ends--\n");
}
