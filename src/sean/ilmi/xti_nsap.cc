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
static char const _xti_nsap_cc_rcsid_[] =
"$Id: xti_nsap.cc,v 1.1 1999/04/16 20:09:07 marsh Exp marsh $";
#endif
#include <common/cprototypes.h>

/*
 * code sections extracted from /opt/FOREatm/examples/xti/common.c
 */
extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/stream.h>
#include <tiuser.h>

#include <fore_xti/xti_user_types.h>
#include <fore_xti/xti.h>
#include <fore_xti/xti_atm.h>
#include <fore_xti/ans.h>
/* missing prototype */
int t_getname(int fd, struct netbuf *name, char nsap[]);

static int get_nsap (unsigned char *nsap)
{
    struct netbuf name;
    ATMSAPAddress local_addr;
    struct t_info info;
    int retval = -1;
    int fd = t_open("/dev/xtisvc0",O_RDWR, &info);

    if (fd < 0)
      {
	perror("open");
      }
    else
      {
	name.maxlen = sizeof(local_addr);
	name.buf = (char *) &local_addr;

	if (t_getname(fd, &name, LOCALNAME) < 0)
	  {
	    t_error("t_getname (local_address)");
	  }
	else
	  {
	    int i;
	    unsigned char *address =
	      (unsigned char *)&local_addr.sap.t_atm_sap_addr.address;
	    for (i=0; i<20; i++)
	      nsap[i] = address[i];
	    retval = 0;
	  }
	t_close(fd);
      }
    if (retval != 0)
      printf("unable to obtain nsap from xti\n");

    return retval;
}
}

/* calls get_nsap and extracts 6 byte esi */
int get_esi (unsigned char *esi)
{
  int retval = -1;
  unsigned char nsap[20];
  if (get_nsap(nsap) == 0)
    {
      int i;
      for (i=0; i<6; i++)
	esi[i] = nsap[13+i];
      retval = 0;
    }
  return retval;
}


