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

#ifndef __sean_IO_H__
#define __sean_IO_H__

#ifndef LINT
static char const _sean_io_h_rcsid_[] =
"$Id: sean_io.h,v 1.8 1998/08/13 05:55:07 bilal Exp $";
#endif

#include <sys/types.h>
#include <DS/containers/list.h>
#include <DS/containers/dictionary.h>
#include <FW/kernel/Handlers.h>
#include <codec/q93b_msg/Buffer.h>

class ATM_Interface;
class ATM_Call;
class Delayed_User_Callback;
class SVC_DataHandler;

//********************************************************************

#define DEFAULT_DRIVER_QOS 100

// The maximum t_read done internal to sean_io
#define DEFAULT_MAX_PDU_SIZE              65535

// The maximum number of PDUs read in, in response to the API
// SVC datahandler callback.  More data will have to wait
// for another round of the Kernel::ProcessNextEvent
#define DEFAULT_MAX_PDUS_TO_SUCK          128

// The maximum number of PDUs we read in when someone
// "peeks" for incoming data e.g. when we do an application
// RecvData(), and discover that there is no data in the API
// Buffer queue
#define DEFAULT_ACTIVE_NUM_PDUS_TO_SUCK   16

// The maximum number of PDUs we try and read when something happens
// that causes the fd to close.
#define DEFAULT_CLOSING_MAX_PDUS_TO_SUCK  1024

//********************************************************************

class sean_io {
public:

  sean_io(void);
  ~sean_io();

  int fd();

  int open();

  // the pointers to the following methods are set by open()
  int close();
  int attach(int vpi, int vci, int Qos);
  int read(caddr_t buffer, int len);
  int write(caddr_t buffer, int len);

  void enqueue_incoming_data_buffer(Buffer* buf);
  int  peek_incoming_data_buffer(void);
  int  suck(int maxsuck_numpdus, int maxsuck_pdusize);
  void drain(void);

private:
  
  void zero(void);

  // the pointers to the following methods are set by open()
  // and used by the equivalent generic function, they will point
  // at functions from a particular interface
  int (*_close)(int fd);
  int (*_attach)(int fd, int vpi, int vci, int Qos);
  int (*_read)(int fd, caddr_t buffer, int len);
  int (*_write)(int fd, caddr_t buffer, int len);

  // default function values
  void not_open();

  // data members
  int            _fd;
  int            _max_transfer;
  int            _vp;
  int            _vc;
  ATM_Interface* _interface;
  ATM_Call*      _call;

  Delayed_User_Callback * _frobber;

  SVC_DataHandler       * _patrol;

  list<Buffer*>  _incoming_pdus;  // the queue

  static Buffer* _tmp;

  friend int sim_interface_write(int fd, caddr_t buffer, int len);
  friend int xtio_attach(int _fd, int vpi, int vci, int qos_kb);
  friend int xtio_close(int fd);

  friend class SVC_DataHandler;
};

int not_open_close(int fd);
int not_open_attach(int fd, int vpi, int vci, int Qos);
int not_open_read(int fd, caddr_t buffer, int len);
int not_open_write(int fd, caddr_t buffer, int len);


//------------------------------------------------------
class io_table;
class io_table {
public:
  friend io_table* The_io_table(void);

  bool Register(int fd, sean_io* sio);
  bool Deregister(int fd);
  sean_io* Lookup(int fd);

  int New_Fake_FD(void);

private:
  io_table(void);
  ~io_table(void);

  int                      _last_fake_fd;

  dictionary<int,sean_io*> _fd_data;
  static io_table*         _singleton;
};

//------------------------------------------------------
class SVC_DataHandler: public InputHandler {
public:
  SVC_DataHandler(int fd);
  virtual ~SVC_DataHandler();

  void Callback(void);

private:
  sean_io* _sio;
};

#endif
