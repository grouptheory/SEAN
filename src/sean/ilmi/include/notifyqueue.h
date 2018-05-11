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

      N O T I F Y Q U E U E. H   
      
      CNotifyEventQueue CLASS DEFINITION
       
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
        Queue for holding sessions waiting for notifiactions

      
      COMPILER DIRECTIVES:
        UNIX - For UNIX build  
      
      CHANGE HISTORY:
=====================================================================*/
#ifndef _NOTIFYQUEUE
#define _NOTIFYQUEUE

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
#include "snmp_pp.h"
#include "msec.h"
#include "oid.h"
#include "target.h"
#include "eventlist.h"

//----[ defines ]------------------------------------------------------



//----[ CNotifyEvent class ]------------------------------------------- 

  
  /*----------------------------------------------------------------*/
  /* CNotifyEvent                                                   */
  /*   a description of a sessions waiting for async notifiactions. */
  /*----------------------------------------------------------------*/
class CNotifyEvent {
public:
  
  CNotifyEvent(Snmp* snmp, 
	       const OidCollection &notify_ids,
	       const TargetCollection &notify_targets, 
	       const AddressCollection &notify_addresses);
  ~CNotifyEvent();
  Snmp * CNotifyEvent::GetId();
  int CNotifyEvent::notify_filter(const Oid &trapid, 
				  const SnmpTarget &target) const;
  int Callback(SnmpTarget & target, Pdu & pdu, int status);

private:
  Snmp * m_snmp;
  TargetCollection *notify_targets;
  OidCollection *notify_ids;
  AddressCollection *notify_addresses;
};

  /*-----------------------------------------------------------*/
  /* CNotifyEventQueueElt                                      */
  /*   a container for a single item on a linked lists of      */
  /*  CNotifyEvents.                                           */
  /*-----------------------------------------------------------*/
class CNotifyEventQueueElt {
  public:
    CNotifyEventQueueElt(CNotifyEvent *notifyevent, 
		     CNotifyEventQueueElt *next,
		     CNotifyEventQueueElt *previous);

    ~CNotifyEventQueueElt();
    CNotifyEventQueueElt *GetNext();
    CNotifyEvent *GetNotifyEvent();
    CNotifyEvent *TestId(Snmp * snmp);

  private:
    CNotifyEvent *m_notifyevent;
    class CNotifyEventQueueElt *m_next;
    class CNotifyEventQueueElt *m_previous;
};

  /*-----------------------------------------------------------*/
  /* CNotifyEventQueue                                         */
  /*   class describing a collection of outstanding SNMP msgs. */
  /*-----------------------------------------------------------*/
class CNotifyEventQueue: public CEvents {
  public:
    CNotifyEventQueue();
    ~CNotifyEventQueue();
    int AddEntry(Snmp * snmp, 
		 const OidCollection &trapids,
		 const TargetCollection &targets, 
		 const AddressCollection &addresses);
    CNotifyEvent * GetEntry(Snmp * snmp);
    void DeleteEntry(Snmp * snmp);

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
    CNotifyEventQueueElt m_head;
    int                  m_msgCount;
    int                  m_notify_fd;
#ifdef SNMPX11
    XtInputId            m_inputId;
#endif // SNMPX11
};

//--------[ externs ]---------------------------------------------------
extern CNotifyEventQueue *notifyEventList; // contains all sessions waiting for notifications

#endif // NOTIFYQUEUE
