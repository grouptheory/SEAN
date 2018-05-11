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

      U S E R T I M E O U T . C P P   
      
      CUTEventQueue CLASS DEFINITION
       
      COPYRIGHT HEWLETT PACKARD COMPANY 1995
      
      INFORMATION NETWORKS DIVISION
      
      NETWORK MANAGEMENT SECTION
      
      
      VERSION:
        $Revision: 1.7 $
       
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
char usertimeout_version[]="$Header: usertimeout.cpp,v 1.7 97/03/26 09:22:23 hmgr Exp $";

// For debuggin cout's
#include <iostream.h>

//-----[ includes ]----------------------------------------------------

//----[ snmp++ includes ]----------------------------------------------      

#include "usertimeout.h"	// queue for holding user-defined events
#include "msec.h"		// class for handling time in msecs

//--------[ globals ]---------------------------------------------------
CUTEventQueue *utEventList;

//--------[ forwards ]--------------------------------------------------
//--------[ externs ]---------------------------------------------------


UtId SNMPAddTimeOut(const unsigned long interval,
		    const ut_callback callBack, 
		    const void * callData) 
{
  UtId id;
  msec now;

  now += interval;
  id = utEventList->AddEntry(now, callBack, callData);
  return id;
}

void SNMPRemoveTimeOut(const UtId utId)
{

  utEventList->DeleteEntry(utId);
}

//----[ CUTEvent class ]------------------------------------------------ 

CUTEvent::CUTEvent( const UtId uniqueId, const msec &timeout,
		    const ut_callback callBack, const void * callData) :
  m_uniqueId(uniqueId), m_timeout(timeout), m_callBack(callBack), 
  m_callData((void *)callData)
{
  // TM What else?
}

CUTEvent::~CUTEvent() {
  // TM What else?
  
}

UtId CUTEvent::GetId() { 
  return(m_uniqueId);
}

void CUTEvent::GetTimeout(msec &timeout) { 
  timeout = m_timeout;
}

int CUTEvent::Callback() {
  if (m_callBack) {
    m_callBack(m_callData, m_uniqueId);
    return 0;
  }
  else {
    return 1;
  }
}


//----[ CUTEventQueueElt class ]-------------------------------------- 

CUTEventQueueElt::CUTEventQueueElt(CUTEvent *utevent,
				   CUTEventQueueElt *next,
				   CUTEventQueueElt *previous):
  m_utevent(utevent), m_next(next), m_previous(previous)
{
    /*------------------------------------------*/
    /* Finish insertion into doubly linked list */
    /*------------------------------------------*/
  if (m_next)
    m_next->m_previous = this;
  if (m_previous)
    m_previous->m_next = this;
}

CUTEventQueueElt::~CUTEventQueueElt(){
    /*-------------------------------------*/
    /* Do deletion form doubly linked list */
    /*-------------------------------------*/
  if (m_next)
    m_next->m_previous = m_previous;
  if (m_previous)
    m_previous->m_next = m_next;

  if (m_utevent)
    delete m_utevent;
}

CUTEventQueueElt *CUTEventQueueElt::GetNext() { 
  return(m_next);
}

CUTEvent *CUTEventQueueElt::GetUTEvent() { 
  return(m_utevent);
}

CUTEvent *CUTEventQueueElt::TestId(const UtId uniqueId){
  if (m_utevent && (m_utevent->GetId() == uniqueId))
    return(m_utevent);
  else
    return(NULL);
}



//----[ CUTEventQueue class ]-------------------------------------- 

CUTEventQueue::~CUTEventQueue(){
  CUTEventQueueElt *leftOver;
    /*--------------------------------------------------------*/
    /* walk the list deleting any elements still on the queue */
    /*--------------------------------------------------------*/
  while (leftOver = m_head.GetNext())
    delete leftOver;
}

UtId CUTEventQueue::AddEntry(const msec &timeout, 
			     const ut_callback callBack, 
			     const void * callData) {
  
  UtId uniqueId;

  // use a unique ID
  uniqueId = MakeId();
  CUTEvent *newEvent = new CUTEvent(uniqueId, timeout,
				    callBack, callData);

    /*---------------------------------------------------------*/
    /* Insert entry at head of list, done automagically by the */
    /* constructor function, so don't use the return value.    */
    /*---------------------------------------------------------*/
  (void) new CUTEventQueueElt(newEvent, m_head.GetNext(), &m_head);
  m_msgCount++;
#ifdef SNMPX11
// TM: if they have not called SnmpX11Initialize yet then we have a problem...
  SnmpX11SetTimer();
#endif // SNMPX11
  return(uniqueId);
}

CUTEvent *CUTEventQueue::GetEntry(const UtId uniqueId){
  CUTEventQueueElt *msgEltPtr = m_head.GetNext();
  CUTEvent *returnVal = NULL;

  while (msgEltPtr){
    if (returnVal = msgEltPtr->TestId(uniqueId))
      return(returnVal);
    msgEltPtr = msgEltPtr->GetNext();
  }
  return (NULL);
}

void CUTEventQueue::DeleteEntry(const UtId uniqueId){
  CUTEventQueueElt *msgEltPtr = m_head.GetNext();
  CUTEvent *returnVal = NULL;

  while (msgEltPtr){
    if (returnVal = msgEltPtr->TestId(uniqueId)){
      delete msgEltPtr;
      m_msgCount--;
      break;
    }
    msgEltPtr = msgEltPtr->GetNext();
  }
#ifdef SNMPX11
  SnmpX11SetTimer();
#endif // SNMPX11
}

#ifdef sun
UtId CUTEventQueue::m_id = 0L;
#endif

// Moved to userdefined.cpp
UtId CUTEventQueue::MakeId() {
  m_id++;
  while (GetEntry(m_id)) {
    // make sure that it is a unique Id
    m_id++;
  }

  return m_id;
}

CUTEvent * CUTEventQueue::GetNextTimeoutEntry() {
  CUTEventQueueElt *msgEltPtr = m_head.GetNext();
  msec bestTime;
  msec sendTime;
  CUTEvent *msg;
  CUTEvent *bestmsg = NULL;

  if (msgEltPtr) {
    bestmsg = msgEltPtr->GetUTEvent();
    bestmsg->GetTimeout(bestTime);
  }

// This would be much simpler if the queue was an ordered list!
  while (msgEltPtr){
    msg = msgEltPtr->GetUTEvent();
    msg->GetTimeout(sendTime);
    if (bestTime  > sendTime) {
      bestTime = sendTime;
      bestmsg = msg;
    }
    msgEltPtr = msgEltPtr->GetNext();
  }
  return bestmsg;
}

int CUTEventQueue::GetNextTimeout(msec &sendTime) {
  CUTEvent *msg;

  msg = GetNextTimeoutEntry();
  if (msg == NULL) {
    // nothing in the queue...
    return 1;
  }
  else {
    msg->GetTimeout(sendTime);
  }
  return 0;
}


void CUTEventQueue::GetFdSets(int &/*maxfds*/, 
			      fd_set &/*readfds*/, 
			      fd_set &/*writefds*/, 
			      fd_set &/*exceptfds*/) {
  // we never have any event sources
  return;
}

int CUTEventQueue::GetCount() {
  return m_msgCount;
}

int CUTEventQueue::HandleEvents(const int /*maxfds*/, 
				const fd_set &/*readfds*/, 
				const fd_set &/*writefds*/, 
				const fd_set &/*exceptfds*/) {
  msec now;  
  return DoRetries(now);
}

int CUTEventQueue::DoRetries(const msec &sendtime) {
  CUTEvent *msg;
  msec timeout;
  UtId id;

  while (msg = GetNextTimeoutEntry()) {
    msg->GetTimeout(timeout);
    if (timeout <= sendtime) {
      id = msg->GetId();
      msg->Callback();
      DeleteEntry(id);
    }
    else {
      break;
    }
  }
  
  return SNMP_CLASS_SUCCESS;
}


int CUTEventQueue::Done() {
  // we are never done
  return 0;
}

