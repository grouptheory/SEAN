#if defined(MODULEID) && !defined(lint)
static char rcsinfo[]="@(#) ipaddr.c $Revision: 1.2 $ $Date: 97/03/26 08:39:25 $";
#endif

/*--------------------------------------------------------------------
 * filename: ipresolv.cpp
 
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

 *
 * Description: 
 *  This file contains DNS name resolution for IP Addresses.  Should
 *    Probably be built into the general IPAddress class, but currently
 *    just an internal convenience function...
 *
 * History:
 *  05/22/95 jdm    Initial design (Objective SNMP).
 *  09/07/95 jdm    Modify to work with SNMP++ IPAddress class...
 *-------------------------------------------------------------------*/
#include <stdio.h>
#include <iostream.h>
#include "snmp_pp.h"

#ifdef __unix
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
extern int h_errno;  // defined in WinSock header, but not for UX?!
#else
#include <winsock.h>
#endif // __unix

#define LOCAL_HOST_IP "127.0.0.1"
#define MAX_TEMP_BUF 200

#ifndef __unix
static int socksInitialized = 0;

int InitializeSockets(){
  if (socksInitialized)
    return(0);
  
  else{
    WORD versionRequested;
    WSADATA wsaData;
    int sockError;
    
    versionRequested = (1 << 16) + 1;
    sockError = WSAStartup(versionRequested, &wsaData);
    if (sockError){
      return(sockError);
    }
    else{
      socksInitialized = 1;
      return(0);
    }
  }
};
#endif //__unix

char *GetBindError(int error){
  static char errorBuf[100];
  
  switch(error){
    case HOST_NOT_FOUND:
      return("<< Host not found >>");
    case TRY_AGAIN:
      return("<< Temporary resolver failure, try again >>");
    case NO_RECOVERY:
      return("<< Non recoverable error >>");
    case NO_ADDRESS:
      return("<< Name maps to non-IP address >>");
    default:
      sprintf(errorBuf, "<< Unknown bind error: %d >>", error);
      return(errorBuf);
  }
}

IPAddress *IPAddrFromName(char *addrString, int &status){
  hostent *lookupResult;
  char    *namePtr = NULL;
  in_addr ipAddr;

  status = 0;

    /*-----------------------------------------------------------------*/
    /* Try this as a "dotted IP" address first, if failure attempt DNS */
    /* lookup of the value...                                          */
    /*-----------------------------------------------------------------*/
  if ((ipAddr.s_addr = inet_addr(addrString)) == -1){
    lookupResult = gethostbyname(addrString);
    if (lookupResult){
      if (lookupResult->h_length == sizeof(in_addr)){
        memcpy((void *) &ipAddr, (void *) lookupResult->h_addr_list[0],
                sizeof(in_addr));
	return(new IPAddress(inet_ntoa(ipAddr)));
      }
      else{
	  // Got an address, but not of expected size...
	status = NO_ADDRESS;
	return(NULL);
      }
    }
    else{
	// Failed to do the lookup of the hostname...
      status = h_errno;
      return(NULL);
    }
  }
  else
    return(new IPAddress(addrString));
}

char *IPAddrToName(IPAddress &srcIpAddr, int &status){
  hostent *lookupResult;
  static char returnBuf[200];  // better not have any monster names...
  char    *namePtr = NULL;
  in_addr ipAddr;

  status = 0;

  if ((ipAddr.s_addr = inet_addr((char *) srcIpAddr)) == -1){
    status = NO_ADDRESS;
    sprintf(returnBuf, "<invalid IPAddr>");
    return(returnBuf);
  }
  else{
    lookupResult = gethostbyaddr((char *) &ipAddr, sizeof(in_addr), AF_INET);
    if (lookupResult){
      namePtr = lookupResult->h_name;
    }
    else{
      namePtr = inet_ntoa(ipAddr);
    }
    strcpy(returnBuf, namePtr);
    return(returnBuf);
  }
}
