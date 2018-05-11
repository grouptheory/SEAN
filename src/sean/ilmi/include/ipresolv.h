/*
  Copyright (c) 1996
  Hewlett-Packard Company

  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  Permission to use, copy, modify, distribute and/or sell this software
  and/or its documentation is hereby granted without fee. User agrees
  to display the above copyright notice and this license notice in all
  copies of the software and any documentation of the software. User
  agrees to assume all liability for the use of the software; Hewlett-Packard
  makes no representations about the suitability of this software for any
  purpose. It is provided "AS-IS without warranty of any kind,either express
  or implied. User hereby grants a royalty-free license to any and all
  derivatives based upon this software code base.

*/
// ipresolv.h
//
//        OBSOLETE OBSOLETE OBSOLETE OBSOLETE
//     friendly naming has been added to the IPAddress class
//     rendering this file obsolete.  Just use address.h
//
//   Functions to use friendly names (DNS) with the IPAddress class
//
//

#include "OBSOLETE"

#ifndef _IPRESOLVE_H
#define _IPRESOLVE_H

// Report error in accessing these functions.
char *GetBindError(int error);

// Create a new IPAddress from a string (caller must delete())
IPAddress *IPAddrFromName(char *addrString, int &status);

// Get a friendly name out of an IPAddress object
char *IPAddrToName(IPAddress &srcIpAddr, int &status);

#endif //_IPRESOLVE_H
