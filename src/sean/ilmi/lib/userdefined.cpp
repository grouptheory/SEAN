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

      U S E R D E F I N E D . C P P
      
      CUDEventQueue CLASS DEFINITION
       
      COPYRIGHT HEWLETT PACKARD COMPANY 1995
      
      INFORMATION NETWORKS DIVISION
      
      NETWORK MANAGEMENT SECTION
      
      
      VERSION:
        $Revision: 1.11 $
       
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
char userdefined_version[]="$Header: userdefined.cpp,v 1.11 97/03/26 09:22:21 hmgr Exp $";

// For debuggin cout's
#include <iostream.h>

//-----[ includes ]----------------------------------------------------

//----[ snmp++ includes ]----------------------------------------------      

#include "userdefined.h"	// queue for holding user-defined events
#include "msec.h"		// class for handling time in msecs

//--------[ globals ]---------------------------------------------------
CUDEventQueue *udEventList;

//--------[ forwards ]--------------------------------------------------
//--------[ externs ]---------------------------------------------------

UdId SNMPAddInput(const int source, 
		  const UdInputMask condition,
		  const ud_callback callBack, 
		  const void * callData) 
{
  UdId id;

  id = udEventList->AddEntry(source, condition, callBack, callData);
  return id;
}

void SNMPRemoveInput(const UdId udId)
{
  udEventList->DeleteEntry(udId);
}

//----[ CUDEvent class ]------------------------------------------------ 

CUDEvent::CUDEvent( const UdId uniqueId, const int source, 
		    const UdInputMask condition, const ud_callback callBack, 
		    const void * callData) :
  m_uniqueId(uniqueId), m_source(source), m_condition(condition), 
  m_callBack(callBack), m_callData((void *)callData)
{
  // TM What else?
}

CUDEvent::~CUDEvent() {
  // TM What else?
  
}

UdId CUDEvent::GetId() { 
  return(m_uniqueId);
}

int CUDEvent::GetSource() { 
  return(m_source);
}

UdInputMask CUDEvent::GetCondition() { 
  return(m_condition);
}


int CUDEvent::Callback() {
  if (m_callBack) {
    m_callBack(m_callData, m_source, m_uniqueId);
    return 0;
  }
  else {
    return 1;
  }
}


//----[ CUDEventQueueElt class ]-------------------------------------- 

CUDEventQueueElt::CUDEventQueueElt(CUDEvent *udevent,
				   CUDEventQueueElt *next,
				   CUDEventQueueElt *previous):
  m_udevent(udevent), m_next(next), m_previous(previous)
{
    /*------------------------------------------*/
    /* Finish insertion into doubly linked list */
    /*------------------------------------------*/
  if (m_next)
    m_next->m_previous = this;
  if (m_previous)
    m_previous->m_next = this;
}

CUDEventQueueElt::~CUDEventQueueElt(){
    /*-------------------------------------*/
    /* Do deletion form doubly linked list */
    /*-------------------------------------*/
  if (m_next)
    m_next->m_previous = m_previous;
  if (m_previous)
    m_previous->m_next = m_next;

  if (m_udevent)
    delete m_udevent;
}

CUDEventQueueElt *CUDEventQueueElt::GetNext() { 
  return(m_next);
}

CUDEvent *CUDEventQueueElt::GetUDEvent() { 
  return(m_udevent);
}

CUDEvent *CUDEventQueueElt::TestId(const UdId uniqueId){
  if (m_udevent && (m_udevent->GetId() == uniqueId))
    return(m_udevent);
  else
    return(NULL);
}



//----[ CUDEventQueue class ]-------------------------------------- 

CUDEventQueue::CUDEventQueue():
  m_head(NULL,NULL,NULL), m_msgCount(0)
{
}

CUDEventQueue::~CUDEventQueue(){
  CUDEventQueueElt *leftOver;
    /*--------------------------------------------------------*/
    /* walk the list deleting any elements still on the queue */
    /*--------------------------------------------------------*/
  while (leftOver = m_head.GetNext())
    delete leftOver;
}

UdId CUDEventQueue::AddEntry(const int source, 
			     const UdInputMask condition,
			     const ud_callback callBack, 
			     const void * callData) {
  
  UdId uniqueId;

  // use a unique ID
  uniqueId = MakeId();
  
  CUDEvent *newEvent = new CUDEvent(uniqueId, source, condition, 
				    callBack, callData);

    /*---------------------------------------------------------*/
    /* Insert entry at head of list, done automagically by the */
    /* constructor function, so don't use the return value.    */
    /*---------------------------------------------------------*/
  (void) new CUDEventQueueElt(newEvent, m_head.GetNext(), &m_head);
  m_msgCount++;
#ifdef SNMPX11
  // Tell X11 to watch our file descriptor
  SnmpX11AddInput( source, m_inputId);
#endif // SNMPX11
  return(uniqueId);
}

CUDEvent *CUDEventQueue::GetEntry(const UdId uniqueId){
  CUDEventQueueElt *msgEltPtr = m_head.GetNext();
  CUDEvent *returnVal = NULL;

  while (msgEltPtr){
    if (returnVal = msgEltPtr->TestId(uniqueId))
      return(returnVal);
    msgEltPtr = msgEltPtr->GetNext();
  }
  return (NULL);
}

void CUDEventQueue::DeleteEntry(const UdId uniqueId){
  CUDEventQueueElt *msgEltPtr = m_head.GetNext();
  CUDEvent *returnVal = NULL;

  while (msgEltPtr){
    if (returnVal = msgEltPtr->TestId(uniqueId)){
      delete msgEltPtr;
      m_msgCount--;
#ifdef SNMPX11
      // Tell X11 to stop watching our file descriptor
      SnmpX11RemoveInput(m_inputId);
#endif // SNMPX11
      break;
    }
    msgEltPtr = msgEltPtr->GetNext();
  }
}

#ifdef sun
UdId CUDEventQueue::m_id = 0L;
#endif

UdId CUDEventQueue::MakeId() {
  m_id++;
  while (GetEntry(m_id)) {
    // make sure that it is a unique Id
    m_id++;
  }

  return m_id;
}

int CUDEventQueue::GetNextTimeout(msec &/*sendTime*/) {

  // We never have a timeout
  return SNMP_CLASS_INVALID_OPERATION;
}

void CUDEventQueue::GetFdSets(int &maxfds, fd_set &readfds, 
			      fd_set &writefds, fd_set &exceptfds) {
  CUDEventQueueElt *msgEltPtr = m_head.GetNext();
  int source;
  UdInputMask condition;

  while (msgEltPtr){
    source = msgEltPtr->GetUDEvent()->GetSource();
    condition = msgEltPtr->GetUDEvent()->GetCondition();
    if (condition & UdInputReadMask)
      FD_SET(source, &readfds);
    if (condition & UdInputWriteMask)
      FD_SET(source, &writefds);
    if (condition & UdInputExceptMask)
      FD_SET(source, &exceptfds);
    if (maxfds < (source+1))
      maxfds = source+1;
    msgEltPtr = msgEltPtr->GetNext();
  }
}

int CUDEventQueue::GetCount() {
  return m_msgCount;
}

int CUDEventQueue::HandleEvents(const int maxfds, 
				const fd_set &readfds, 
				const fd_set &writefds, 
				const fd_set &exceptfds) {
  CUDEventQueueElt *msgEltPtr;
  int source;
  
  for (int fd = 0; fd < maxfds; fd++) {
    if (FD_ISSET(fd, &readfds)  || 
	FD_ISSET(fd, &writefds) ||
	FD_ISSET(fd, &exceptfds)) {

      // each fd may have multiple callbacks registered
      msgEltPtr = m_head.GetNext();
      while (msgEltPtr){
	source = msgEltPtr->GetUDEvent()->GetSource();
	if (source == fd) {
	  msgEltPtr->GetUDEvent()->Callback();
	}
	msgEltPtr = msgEltPtr->GetNext();
      } 
    }
  }
  
  return SNMP_CLASS_SUCCESS;
}

int CUDEventQueue::DoRetries(const msec &/*sendTime*/) {
  // no timeouts, so just return
  return SNMP_CLASS_SUCCESS;
}


int CUDEventQueue::Done() {
  // we are never done
  return 0;
}
