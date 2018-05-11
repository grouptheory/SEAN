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

      E V E N T L I S T . H   
      
      CEventList  CLASS DEFINITION
       
      COPYRIGHT HEWLETT PACKARD COMPANY 1995
      
      INFORMATION NETWORKS DIVISION
      
      NETWORK MANAGEMENT SECTION
      
      
      VERSION:
        $Revision: 1.14 $
       
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
        Queue for holding all event sources (snmp messages, user 
        defined input sources, user defined timeouts, etc)
      
      COMPILER DIRECTIVES:
        UNIX - For UNIX build  
      
      CHANGE HISTORY:
=====================================================================*/
#ifndef _EVENTLIST
#define _EVENTLIST

//----[ includes ]----------------------------------------------------- 
extern "C" 
{
#include <sys/types.h> // NOTE: due to 10.10 bug, order is important
                       //   in that all routines must include types.h
                       //   and time.h in same order otherwise you
                       //   will get conflicting definitions of 
                       //   "fd_set" resulting in link time errors.
#include <sys/time.h>  // time stuff and fd_set
#include <values.h>

//------------[ if using X11... ] 
#ifdef SNMPX11
#include <X11/Intrinsic.h>
#endif // SNMPX11
}

//----[ snmp++ includes ]----------------------------------------------        
#include "msec.h"
#include "snmp_pp.h"      // class def for snmp

//----[ defines ]------------------------------------------------------

typedef unsigned long Uint32;
#define MAX_UINT32 MAXLONG

//--------[ externs ]--------------------------------------------------
extern int SNMPProcessPendingEvents();
extern int SNMPProcessEvents();
extern int SNMPBlockForResponse(const unsigned long req_id, Pdu &pdu);
extern void SNMPMainLoop();
extern void SNMPExitMainLoop();
extern void SNMPGetFdSets(int &maxfds, fd_set &readfds, fd_set &writefds, 
			  fd_set &exceptfds);
extern Uint32 SNMPGetNextTimeout();

#ifdef SNMPX11
extern int SNMPX11Initialize( const XtAppContext app_context);
#endif // SNMPX11

//----[ CEvents class ]------------------------------------------------ 

  
class CEvents {
  public:

  // allow destruction of derived classes
  virtual ~CEvents() {};

  // find the next timeout
  virtual int GetNextTimeout(msec &sendTime) = 0;

  // set up parameters for select
  virtual void GetFdSets(int &maxfds, fd_set &readfds, fd_set &writefds, 
			   fd_set &exceptfds) = 0;

  // return number of outstanding messages 
  virtual int GetCount() = 0;

  // process events pending on the active file descriptors
  virtual int HandleEvents(const int maxfds, 
			   const fd_set &readfds, 
			   const fd_set &writefds, 
			   const fd_set &exceptfds) = 0;

  // process any timeout events
  virtual int DoRetries(const msec &sendtime) = 0;

  // check to see if there is a termination condition
  virtual int Done() = 0;
};


class CEventListElt {
  public:
    CEventListElt(CEvents *events, 
		  CEventListElt *next,
		  CEventListElt *previous);

    ~CEventListElt();
    CEventListElt *GetNext();
    CEvents *GetEvents();

  private:
    CEvents *m_events;
    class CEventListElt *m_next;
    class CEventListElt *m_previous;
};

class CEventList {
  public:
    CEventList();
    ~CEventList();

  // add an event source to the list 
  CEvents *AddEntry(CEvents *events);

  // tell main_loop to exit after one pass
  void SetDone();
  // see if main loop should terminate
  int GetDone();

  // find the time of the next event that will timeout
  int GetNextTimeout(msec &sendTime);

  // set up paramters for select
  void GetFdSets(int &maxfds, fd_set &readfds, fd_set &writefds, 
		 fd_set &exceptfds);

  // return number of outstanding messages 
  int GetCount();
  
  // process events pending on the active file descriptors
  int HandleEvents(const int maxfds, 
		   const fd_set &readfds, 
		   const fd_set &writefds, 
		   const fd_set &exceptfds);

  // process any timeout events
  int DoRetries(const msec &sendtime);

  // check to see if there is a termination condition
  int Done();

  private:
    CEventListElt m_head;
    int m_msgCount;
    int m_done;
};

//--------[ externs ]---------------------------------------------------
extern CEventList eventList; // contains all expected events

#ifdef SNMPX11
#define TIMER_NOT_SET ((unsigned long) -1)
#define CONTEXT_NOT_SET ((XtAppContext) NULL)

// these are not user-callable functions
extern int SnmpX11AddInput(int inputFd, XtInputId &inputId); 
extern int SnmpX11RemoveInput(XtInputId &inputId); 
extern void SnmpX11TimerCallback(XtPointer client_data, XtIntervalId *id);
extern void SnmpX11InputCallback(XtPointer client_data, int *source, XtInputId *id);
extern void SnmpX11SetTimer();
#endif // SNMPX11

#endif
