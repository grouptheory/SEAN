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


#ifndef _ASYNC_H_
#define _ASYNC_H_
#ifndef LINT
static char const _async_h_rcsid_[] =
"$Id: async.h,v 1.1 1998/07/20 11:11:40 bilal Exp $";
#endif

#include <iostream.h>

extern "C" {
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
}

#include <DS/containers/dictionary.h>
#include <FW/kernel/Handlers.h>

class asynchronous_manager;
class tcp_local_id;

class asynchronous_manager {
private:
  static asynchronous_manager* _singleton;

  int _interval;     /* count in unit of useconds */
  int _total_time;
  int _set_type;
  int _sig_type;
  long _pid;
  dictionary<int,InputHandler*> _fds;

  void (*_periodic_func)(int);
  void (*_signalling_func)(int);

  void start_timer(int set_type, int sig_type);
  void reset(int set_type);

  asynchronous_manager(double timeperiod,     // in seconds
		       void (*periodic_f)(int), 
		       void (*signalling_f)(int));

  friend class tcp_ipc_layer;
  friend void process_timeout(int foo);
  friend void periodic_handler(int foo);
  friend void sig_io_handler(int foo);

  ~asynchronous_manager();
  void reset_io_handler(void);
  void handle_sigio(void);

 public:

  friend asynchronous_manager* AsynchronousManager(void);
  void Register_Async_Handler(tcp_local_id* id, InputHandler* ih);
};


#endif

