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
      
      CNotifyEventQueue CLASS DEFINITION
       
      COPYRIGHT HEWLETT PACKARD COMPANY 1995
      
      INFORMATION NETWORKS DIVISION
      
      NETWORK MANAGEMENT SECTION
      
      
      VERSION:
        $Revision: 1.12 $
       
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
char notifyqueue_version[]="$Header: notifyqueue.cpp,v 1.12 97/03/26 09:18:43 hmgr Exp $";

// For debuggin cout's
#include <stdio.h>
#include <iostream.h>

#ifdef sun
#include <netinet/in.h>
#endif

//-----[ includes ]----------------------------------------------------
extern "C"
{
#include <errno.h>
}
//----[ snmp++ includes ]----------------------------------------------      

#include "notifyqueue.h"	// queue for holding sessions waiting for async notifications

//--------[ globals ]---------------------------------------------------
CNotifyEventQueue *notifyEventList;

//--------[ forwards ]--------------------------------------------------
//--------[ externs ]---------------------------------------------------

//-----[ macros ]------------------------------------------------------
// should be in snmp.h...
#define SNMP_PORT 161	      // standard port # for SNMP
#define SNMP_TRAP_PORT 162    // standard port # for SNMP traps

//----[ CNotifyEvent class ]------------------------------------------------ 

CNotifyEvent::CNotifyEvent(Snmp * snmp, 
			   const OidCollection &trapids,
			   const TargetCollection &targets, 
			   const AddressCollection &addresses) 
{
  m_snmp = snmp;
  // create new collections using parms passed in
  notify_ids = new OidCollection( trapids);
  notify_targets = new TargetCollection ( targets);
  notify_addresses = new AddressCollection ( addresses); 
}

CNotifyEvent::~CNotifyEvent() {
  // free up local collections
  if ( notify_ids != 0)
    delete notify_ids;
  notify_ids = NULL;

  if ( notify_targets != 0)
    delete notify_targets;
  notify_targets = NULL;
  
  if ( notify_addresses != 0)
    delete notify_addresses;
  notify_addresses = NULL;
}

Snmp * CNotifyEvent::GetId() { 
  return(m_snmp);
}

int CNotifyEvent::notify_filter(const Oid &trapid, const SnmpTarget &target) const
{
  int target_count, has_target = FALSE, target_matches = FALSE;
  int trapid_count, has_trapid = FALSE, trapid_matches = FALSE;
  GenAddress targetaddr, tmpaddr;

  // figure out how many targets, handle empty case as all targets
  if ((notify_targets) && (target_count = notify_targets->size())) {
    CTarget * ctarget = (CTarget *) &target;
    CTarget tmptarget;
    has_target = TRUE;

    ctarget->get_address(targetaddr);
    if (targetaddr.valid()) {
      // loop through all targets in the collection
      for ( int x=1;x<=target_count; x++)       // for all targets
      {
	if (notify_targets->get_element(tmptarget, x-1))
	  continue;
	tmptarget.get_address(tmpaddr);
	if ((tmpaddr.valid()) &&
	    (targetaddr == tmpaddr) &&
	    (!strcmp(ctarget->get_readcommunity(), 
		     tmptarget.get_readcommunity()))) {
	  target_matches = TRUE;
	  break;
	}
      }
    }
  }
  // else no targets means all targets

  // figure out how many trapids, handle empty case as all trapids
  if ((notify_ids) && (trapid_count = notify_ids->size())) {
    Oid tmpoid;
    has_trapid = TRUE;
    // loop through all trapids in the collection
    for ( int y=1;y<=trapid_count; y++)       // for all trapids
    {
      if (notify_ids->get_element(tmpoid, y-1))
	continue;
      if (trapid == tmpoid) {
	trapid_matches = TRUE;
	break;
      }
    }
  }
  // else no trapids means all traps

  // Make the callback if the trap passed the filters
  if ((has_target && !target_matches) || (has_trapid && !trapid_matches)) {
    return FALSE;
  }
  else {
    return TRUE;
  }

}


int CNotifyEvent::Callback(SnmpTarget & target, Pdu & pdu, 
			   int status) {
  Oid trapid;
  int reason;
  pdu.get_notify_id(trapid);

  // Make the callback if the trap passed the filters
  if ((m_snmp) && (notify_filter(trapid, target))) {
      
    if (SNMP_CLASS_TL_FAILED == status) {
      reason = SNMP_CLASS_TL_FAILED;
    } else {
      reason = SNMP_CLASS_NOTIFICATION;
    }

    //------[ call into the callback function ]-------------------------
    (m_snmp->get_notify_callback())( 
      reason,
      m_snmp,			// snmp++ session who owns the req
      pdu,			// trap pdu
      target,			// target
      m_snmp->get_notify_callback_data()); // callback data
  }
  
  return SNMP_CLASS_SUCCESS;
}


//----[ CNotifyEventQueueElt class ]-------------------------------------- 

CNotifyEventQueueElt::CNotifyEventQueueElt(CNotifyEvent *notifyevent,
					   CNotifyEventQueueElt *next,
					   CNotifyEventQueueElt *previous):
  m_notifyevent(notifyevent), m_next(next), m_previous(previous)
{
    /*------------------------------------------*/
    /* Finish insertion into doubly linked list */
    /*------------------------------------------*/
  if (m_next)
    m_next->m_previous = this;
  if (m_previous)
    m_previous->m_next = this;
}

CNotifyEventQueueElt::~CNotifyEventQueueElt(){
    /*-------------------------------------*/
    /* Do deletion form doubly linked list */
    /*-------------------------------------*/
  if (m_next)
    m_next->m_previous = m_previous;
  if (m_previous)
    m_previous->m_next = m_next;

  if (m_notifyevent)
    delete m_notifyevent;
}

CNotifyEventQueueElt *CNotifyEventQueueElt::GetNext() { 
  return(m_next);
}

CNotifyEvent *CNotifyEventQueueElt::GetNotifyEvent() { 
  return(m_notifyevent);
}

CNotifyEvent *CNotifyEventQueueElt::TestId(Snmp *snmp){
  if (m_notifyevent && (m_notifyevent->GetId() == snmp))
    return(m_notifyevent);
  else
    return(NULL);
}



//----[ CNotifyEventQueue class ]-------------------------------------- 

CNotifyEventQueue::CNotifyEventQueue(): 
  m_head(NULL,NULL,NULL), m_msgCount(0), m_notify_fd(-1) {
//TM: could do the trap registration setup here but seems better to
//wait until the app actually requests trap receives by calling
//notify_register().
}

CNotifyEventQueue::~CNotifyEventQueue(){
  CNotifyEventQueueElt *leftOver;
    /*--------------------------------------------------------*/
    /* walk the list deleting any elements still on the queue */
    /*--------------------------------------------------------*/
  while (leftOver = m_head.GetNext())
    delete leftOver;
}

int CNotifyEventQueue::AddEntry(Snmp * snmp, 
				const OidCollection &trapids,
				const TargetCollection &targets,
				const AddressCollection &addresses) { 

  if (!m_msgCount) {
    // This is the first request to receive notifications
    // Set up the socekt for the snmp trap port (162)
    struct sockaddr_in mgr_addr;

    // open a socket to be used for the session
    if (( m_notify_fd = (int) socket( AF_INET, SOCK_DGRAM,0)) < 0)
      return SNMP_CLASS_TL_FAILED;
    else
    {
      // set up the manager socket attributes
      memset(&mgr_addr, 0, sizeof(mgr_addr));
      mgr_addr.sin_family = AF_INET;
      mgr_addr.sin_addr.s_addr = htonl( INADDR_ANY);
      mgr_addr.sin_port = htons( SNMP_TRAP_PORT);

      // bind the socket
      if ( bind( (int) m_notify_fd,
#ifdef sun
		 (struct sockaddr *) &mgr_addr,
#else
		 &mgr_addr,
#endif
		 sizeof(mgr_addr)) < 0) {
//perror("bind");
fprintf(stderr,"bind error\n");
	// TM: should probably check this...
	return SNMP_CLASS_TL_IN_USE;
      }

#ifdef SNMPX11
      // Tell X11 to watch our file descriptor
      SnmpX11AddInput( m_notify_fd, m_inputId);
#endif // SNMPX11
    }
  }

    
  CNotifyEvent *newEvent = new CNotifyEvent(snmp, trapids, targets, addresses);

    /*---------------------------------------------------------*/
    /* Insert entry at head of list, done automagically by the */
    /* constructor function, so don't use the return value.    */
    /*---------------------------------------------------------*/
  (void) new CNotifyEventQueueElt(newEvent, m_head.GetNext(), &m_head);
  m_msgCount++;
  return SNMP_CLASS_SUCCESS;
}


CNotifyEvent *CNotifyEventQueue::GetEntry(Snmp * snmp){
  CNotifyEventQueueElt *msgEltPtr = m_head.GetNext();
  CNotifyEvent *returnVal = NULL;

  while (msgEltPtr){
    if (returnVal = msgEltPtr->TestId(snmp))
      return(returnVal);
    msgEltPtr = msgEltPtr->GetNext();
  }
  return (NULL);
}

void CNotifyEventQueue::DeleteEntry(Snmp * snmp){
  CNotifyEventQueueElt *msgEltPtr = m_head.GetNext();
  CNotifyEvent *returnVal = NULL;

  while (msgEltPtr){
    if (returnVal = msgEltPtr->TestId(snmp)){
      delete msgEltPtr;
      m_msgCount--;
      break;
    }
    msgEltPtr = msgEltPtr->GetNext();
  }

  // shut down the trap socket (if valid) if not using it.
  if ((m_msgCount <= 0) && (m_notify_fd >= 0)) {
    close(m_notify_fd);
    m_notify_fd = 0;
#ifdef SNMPX11
    // Tell X11 to stop watching our file descriptor
    SnmpX11RemoveInput(m_inputId);
#endif // SNMPX11
  }
}


int CNotifyEventQueue::GetNextTimeout(msec &/*sendTime*/) {

  // we never have any timeouts
  return 1;
}


void CNotifyEventQueue::GetFdSets(int &maxfds, 
				  fd_set &readfds, 
				  fd_set &/*writefds*/, 
				  fd_set &/*exceptfds*/) {
  if (m_notify_fd >= 0) {
    FD_SET(m_notify_fd, &readfds);
    if (maxfds < (m_notify_fd+1))
      maxfds = m_notify_fd+1;
  }
  return;
}

int CNotifyEventQueue::GetCount() {
  return m_msgCount;
}


extern int receive_snmp_notification(int sock,Pdu &pdu, CTarget &target); 
int CNotifyEventQueue::HandleEvents(const int /*maxfds*/, 
				    const fd_set &readfds, 
				    const fd_set &/*writefds*/, 
				    const fd_set &/*exceptfds*/) {
  Pdu pdu;
  CTarget target;
  CNotifyEventQueueElt *notifyEltPtr = m_head.GetNext();
  int status = SNMP_CLASS_SUCCESS;

  // pull the notifiaction off the socket
  if (FD_ISSET(m_notify_fd, &readfds)) {
    status = receive_snmp_notification(m_notify_fd, pdu,target);

    if (SNMP_CLASS_SUCCESS == status ||
	SNMP_CLASS_TL_FAILED == status) {
      // If we have transport layer failure, the app will want to
      // know about it.
      // Go through each snmp object and check the filters, making
      // callbacks as necessary 
      while (notifyEltPtr){
 
	notifyEltPtr->GetNotifyEvent()->Callback(target, pdu, 
						 status);
	notifyEltPtr = notifyEltPtr->GetNext();   
      } // for each snmp object
    }

  }

  return status;
}

int CNotifyEventQueue::DoRetries(const msec &/*sendtime*/) {
  // nothing to retry
  return 0;
}

int CNotifyEventQueue::Done() {
  // we are never done
  return 0;
}
