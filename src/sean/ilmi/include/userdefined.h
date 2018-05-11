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

      U S E R D E F I N E D . H   
      
      CUDEventQueue CLASS DEFINITION
       
      COPYRIGHT HEWLETT PACKARD COMPANY 1995
      
      INFORMATION NETWORKS DIVISION
      
      NETWORK MANAGEMENT SECTION
      
      
      VERSION:
        $Revision: 1.8 $
       
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
        input sources
      
      COMPILER DIRECTIVES:
        UNIX - For UNIX build  
      
      CHANGE HISTORY:
=====================================================================*/
#ifndef _USERDEFINED
#define _USERDEFINED

//----[ includes ]----------------------------------------------------- 
extern "C" 
{
#include <sys/types.h>          // NOTE:  due to 10.10 bug, order is important
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

// Modeled after XtInputMask
typedef unsigned long   UdInputMask;
#define UdInputNoneMask         0L
#define UdInputReadMask         (1L<<0)
#define UdInputWriteMask        (1L<<1)
#define UdInputExceptMask       (1L<<2)

typedef unsigned long   UdId;

/* User-defined callback*/
typedef void (*ud_callback)(void * callData, int source, UdId id);


//----[ CUDEvent class ]----------------------------------------------- 

  
class CUDEvent {
public:
  CUDEvent (const UdId uniqueId, const int source, 
	    const UdInputMask condition, const ud_callback callBack, 
	    const void * callData);
  ~CUDEvent();
  UdId          GetId();
  int           GetSource();
  UdInputMask   GetCondition();

  int Callback();

private:
  UdId            m_uniqueId;
  int             m_context;
  int             m_source;
  UdInputMask     m_condition;
  ud_callback     m_callBack;
  void *          m_callData;
};

  /*-----------------------------------------------------------*/
  /* CUDEventQueueElt                                          */
  /*   a container for a single item on a linked lists of      */
  /*  CUDEvents.                                               */
  /*-----------------------------------------------------------*/
class CUDEventQueueElt {
  public:
    CUDEventQueueElt(CUDEvent *udevent, 
                     CUDEventQueueElt *next,
                     CUDEventQueueElt *previous);

    ~CUDEventQueueElt();
    CUDEventQueueElt *GetNext();
    CUDEvent *GetUDEvent();
    CUDEvent *TestId(const UdId uniqueId);

  private:
    CUDEvent *m_udevent;
    class CUDEventQueueElt *m_next;
    class CUDEventQueueElt *m_previous;
};

  /*-----------------------------------------------------------*/
  /* CUDEventQueue                                             */
  /*   class describing a collection of outstanding SNMP msgs. */
  /*-----------------------------------------------------------*/
class CUDEventQueue: public CEvents {
  public:
    CUDEventQueue();
    ~CUDEventQueue();
    UdId AddEntry(const int source, const UdInputMask condition,
                  const ud_callback callBack, const void * callData);
    CUDEvent *GetEntry(const UdId uniqueId);
    void DeleteEntry(const UdId uniqueId);

    UdId MakeId();

    // find the next timeout
    int GetNextTimeout(msec &sendTime);
    // set up parameters for select
    void GetFdSets(int &maxfds, fd_set &readfds, fd_set &writefds, 
                   fd_set &exceptfds);
    // return number of user-defined event handlers 
    int GetCount();

    int HandleEvents(const int maxfds, const fd_set &readfds, 
                     const fd_set &writefds, const fd_set &exceptfds);
    int DoRetries(const msec &sendtime);

    int Done();

  private:
    CUDEventQueueElt m_head;
    int              m_msgCount;
    static UdId      m_id;
#ifdef SNMPX11
    XtInputId        m_inputId;
#endif // SNMPX11
};

//--------[ externs ]---------------------------------------------------
extern CUDEventQueue *udEventList; // contains all user-defined events
extern UdId SNMPAddInput(const int source, 
                         const UdInputMask condition,
                         const ud_callback callBack, 
                         const void * callData);
extern void SNMPRemoveInput(const UdId udId);

#endif
