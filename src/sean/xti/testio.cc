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
static char const _testio_cc_rcsid_[] =
"$Id: testio.cc,v 1.3 1998/08/06 04:04:27 bilal Exp $";
#endif
#include <common/cprototypes.h>
#include <iostream.h>
#include "sean_io.h"
//can't be any more simple or less usefull

main()
{
  char buffer[1024];
  int maxpdu;
  int bytes_read;
#if 0  
  int fd = xtio_open(&maxpdu);
  (void)xtio_attach(fd, 0, 216, 0);
  
  bytes_read = xtio_read(fd, buffer, 1024);
  cout << "read "<< bytes_read <<" on (0,16)"<<endl;
#else
  sean_io *s = new sean_io();
  cout << "fd = " << s->open() << endl;
  cout << "attach = " << s->attach(0, 229, 0) <<endl;
  s->write(buffer, 20);
  s->close();
#endif  
}
