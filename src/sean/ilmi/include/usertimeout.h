/* -*-C++-*- */
/*===================================================================

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

      U S E R T I M E O U T . H   
      
      CUTEventQueue CLASS DEFINITION
       
      COPYRIGHT HEWLETT PACKARD COMPANY 1995
      
      INFORMATION NETWORKS DIVISION
      
      NETWORK MANAGEMENT SECTION
      
      
      VERSION:
        $Revision: 1.6 $
       
      DESIGN:
        Tom Murray
                
      AUTHOR:
        Tom Murray      

      DATE:
        Sept 12, 1995
      
      LANGUAGE:
        ANSI C++ 
      
      OPERATING SYSTEMS:
        DOS/WINDOWS 3.1
        BSD UNIX
      
      DESCRIPTION:
        Queue for holding callback associated with user defined 
        timeouts

      
      COMPILER DIRECTIVES:
        UNIX - For UNIX build  
      
      CHANGE HISTORY:
=====================================================================*/
#ifndef _USERTIMEOUT
#define _USERTIMEOUT

//----[ includes ]----------------------------------------------------- 
extern "C" 
{
#include <sys/types.h>          // NOTE: due to 10.10 bug, order is important
                                //   in that all routines must include types.h
                                //   and time.h in same order otherwise you
                                //   will get conflicting definitions of 
                                //   "fd_set" resulting in link time errors.
#include <sys/time.h>           // time stuff and fd_set
}
//----[ snmp++ includes ]----------------------------------------------        
#include "msec.h"
#include "eventlist.h"

//----[ defines ]------------------------------------------------------

typedef unsigned long   UtId;
 
/* User-defined callback */
typedef void (*ut_callback)(void * callData, UtId id);


//----[ CUTEvent class ]------------------------------------------- 

  
  /*-----------------------------------------------------------*/
  /* CUTEvent                                                  */
  /*   a description of a single MIB access operation.         */
  /*-----------------------------------------------------------*/
class CUTEvent {
public:
  CUTEvent(const UtId uniqueId, const msec &timeout,
           const ut_callback callBack, const void * callData);
  ~CUTEvent();
  UtId GetId();
  void GetTimeout(msec &timeout);

  int Callback();

private:
  UtId            m_uniqueId;
  msec            m_timeout;
  ut_callback     m_callBack;
  void *          m_callData;
};

  /*-----------------------------------------------------------*/
  /* CUTEventQueueElt                                          */
  /*   a container for a single item on a linked lists of      */
  /*  CUTEvents.                                               */
  /*-----------------------------------------------------------*/
class CUTEventQueueElt {
  public:
    CUTEventQueueElt(CUTEvent *utevent, 
		     CUTEventQueueElt *next,
		     CUTEventQueueElt *previous);

    ~CUTEventQueueElt();
    CUTEventQueueElt *GetNext();
    CUTEvent *GetUTEvent();
    CUTEvent *TestId(const UtId uniqueId);

  private:
    CUTEvent *m_utevent;
    class CUTEventQueueElt *m_next;
    class CUTEventQueueElt *m_previous;
};

  /*-----------------------------------------------------------*/
  /* CUTEventQueue                                             */
  /*   class describing a collection of outstanding SNMP msgs. */
  /*-----------------------------------------------------------*/
class CUTEventQueue: public CEvents {
  public:
    CUTEventQueue(): m_head(NULL,NULL,NULL), m_msgCount(0) {};
    ~CUTEventQueue();
    UtId AddEntry(const msec &timeout, 
		  const ut_callback callBack, 
		  const void * callData);
    CUTEvent *GetEntry(const UtId uniqueId);
    void DeleteEntry(const UtId uniqueId);

    UtId MakeId();

    // find the next msg that will timeout
    CUTEvent *GetNextTimeoutEntry();
    // find the next timeout
    int GetNextTimeout(msec &timeout);
    // set up parameters for select
    void GetFdSets(int &maxfds, fd_set &readfds, fd_set &writefds, 
		   fd_set &exceptfds);
    // return number of outstanding messages 
    int GetCount();

    int HandleEvents(const int maxfds, 
                     const fd_set &readfds, 
                     const fd_set &writefds, 
                     const fd_set &exceptfds);
    int DoRetries(const msec &sendtime);

    int Done();

  private:
    CUTEventQueueElt m_head;
    int              m_msgCount;
    static UtId      m_id;
};

//--------[ externs ]---------------------------------------------------
extern CUTEventQueue *utEventList; // contains all user-defined timeouts
extern UtId SNMPAddTimeOut(const unsigned long interval,
                           const ut_callback callBack, 
                           const void * callData);
extern void SNMPRemoveTimeOut(const UtId utId);

#endif
