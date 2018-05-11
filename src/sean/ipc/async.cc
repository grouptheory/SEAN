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

#ifndef LINT
static char const _async_cc_rcsid_[] =
"$Id: async.cc,v 1.7 1998/08/25 21:30:52 mountcas Exp $";
#endif
#include <common/cprototypes.h>

#include "async.h"
#include <iostream.h>
#include <FW/basics/diag.h>
#include <sean/ipc/tcp_ipc.h>


extern "C" {
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>     
#include <sys/types.h>    
#if defined(__SunOS5__) || defined(__IRIX64__)
#include <sys/filio.h>
#else
#ifdef __Linux__
#include <asm/ioctls.h>
#endif
#endif
#include <unistd.h>
#include <fcntl.h>
}

// in seconds
#define PERIODIC_ASYNC_WAKEUP 2.0


class asynchronous_manager;

asynchronous_manager* AsynchronousManager(void);

void periodic_handler(int foo);
void sig_io_handler(int foo);
void process_timeout(int foo);

void process_timeout(int foo){
  asynchronous_manager::_singleton->_total_time += asynchronous_manager::_singleton->_interval;
  (*(asynchronous_manager::_singleton->_periodic_func))(foo);
  /* register the handler using signal for the type of time you want */
  signal(asynchronous_manager::_singleton->_sig_type, process_timeout);
}

void asynchronous_manager::start_timer(int set_type, int sig_type){
  struct itimerval time_value;
  _set_type = set_type;
  _sig_type = sig_type;
  _total_time = 0;
  if(!_interval)
    return;
  /* register the handler using signal for the type of time you want */
  signal(sig_type, process_timeout);
  /* now init the struct time_value */
  time_value.it_value.tv_sec  = _interval/1000000;
  time_value.it_value.tv_usec = _interval%1000000;
  time_value.it_interval.tv_sec  = _interval/1000000;
  time_value.it_interval.tv_usec = _interval%1000000;
  setitimer(set_type, &time_value, NULL);
}

void asynchronous_manager::reset(int set_type){
  struct itimerval time_value;
  
  time_value.it_value.tv_sec  = 0;
  time_value.it_value.tv_usec = 0;
  time_value.it_interval.tv_sec  = 0;
  time_value.it_interval.tv_usec = 0;
  
  if(!setitimer(set_type, &time_value, NULL))
    printf("could not reset timer\n");
}


void periodic_handler(int foo){
  cout << "Periodic Async " << foo << endl;;
}

void sig_io_handler(int foo){
  AsynchronousManager()->reset_io_handler();
  AsynchronousManager()->handle_sigio();
  AsynchronousManager()->reset_io_handler();
}

asynchronous_manager::asynchronous_manager(double timeperiod, 
			       void (*periodic_f)(int), 
					   void (*signalling_f)(int)) {
  _interval = (int)(timeperiod * 1000000.0);
  _periodic_func=periodic_f;
  _signalling_func=signalling_f;
  _pid = getpid();

  // start_timer(ITIMER_VIRTUAL, SIGVTALRM);
  start_timer(ITIMER_REAL, SIGALRM);
  signal(SIGIO, _signalling_func);
#if !defined(__NetBSD__)
  signal(SIGPOLL, _signalling_func);
#endif
  asynchronous_manager::_singleton=this;
}

asynchronous_manager::~asynchronous_manager(){
  reset(ITIMER_VIRTUAL);
  asynchronous_manager::_singleton=0L;
}

void asynchronous_manager::Register_Async_Handler(tcp_local_id* id, InputHandler* ih) {
  int fd = id->_s;
  int one = 1;

  int rval;
  if ((rval = ioctl(fd, FIOASYNC, (char *)&one)) < 0) {
    diag("api.async",DIAG_ERROR,"Unable to make connection to signalling agent asynchronous... (FIOASYNC failed)");
  }

  if (rval = fcntl(fd,F_SETOWN,_pid) < 0) {
    diag("api.async",DIAG_ERROR,"Unable to make connection to signalling agent asynchronous... (F_SETOWN failed)");
  }

  dic_item di = _fds.lookup(fd);
  if (di) {
    _fds.del_item(di);
  }
  _fds.insert(fd,ih);
}

asynchronous_manager* AsynchronousManager(void) {
  if (!asynchronous_manager::_singleton) {
    if (getenv("SEAN_ASYNC_MODE")) {                  // asynchronous mode is not 
      asynchronous_manager::_singleton = new          // yet supported.
	asynchronous_manager( PERIODIC_ASYNC_WAKEUP,
			      periodic_handler,
			      sig_io_handler );
    }
  }
  return asynchronous_manager::_singleton;
}


void asynchronous_manager::reset_io_handler(void) {
  signal(SIGIO, _signalling_func);
#if !defined(__NetBSD__)
  signal(SIGPOLL, _signalling_func);
#endif
}


void asynchronous_manager::handle_sigio(void) {
  dic_item di;
  forall_items(di,_fds) {
    InputHandler * ih = _fds.inf(di);
    int fd = _fds.key(di);
    diag("api.async",DIAG_INFO,"!!! Polling connection to signalling agent on fd %d\n",fd);
  }
    
  double poll = 0.0;
  theKernel().WaitForInput( poll );
  diag("api.async",DIAG_INFO,"!!! Done polling connections to signalling agents...\n");
}
