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

      E V E N T L I S T . C P P

      CEventList  CLASS DEFINITION

      COPYRIGHT HEWLETT PACKARD COMPANY 1995

      INFORMATION NETWORKS DIVISION

      NETWORK MANAGEMENT SECTION


      VERSION:
        $Revision: 1.17 $

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
char event_list_version[]="$Header: eventlist.cpp,v 1.17 97/03/25 21:26:59 hmgr Exp $";

// For debuggin cout's
#include <iostream.h>

//-----[ includes ]----------------------------------------------------

//----[ snmp++ includes ]----------------------------------------------

#include "eventlist.h"		// queue for holding all event sources
#include "msgqueue.h"		// queue for holding snmp event sources
#include "userdefined.h"        // queue for holding user defined event sources
#include "usertimeout.h"        // queue for holding user defined timeouts
#include "notifyqueue.h"	// queue for holding trap callbacks
#include "msec.h"		// class for handling time in msecs

//--------[ globals ]---------------------------------------------------
CEventList eventList;		// holds all event sources

#ifdef SNMPX11
XtAppContext global_app_context = CONTEXT_NOT_SET;
XtIntervalId global_interval_id = TIMER_NOT_SET;
msec global_next_timeout;
#endif // SNMPX11

//--------[ forwards ]--------------------------------------------------
//--------[ externs ]---------------------------------------------------

#ifdef WU_APP

extern "C" {

#define PeekMessage PeekMessageA
#define DispatchMessage DispatchMessageA

// MS-Windows types needed
typedef int BOOL;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef void*  HWND;
typedef unsigned int UINT;
typedef UINT WPARAM;
typedef long LPARAM;
#define MAKELONG(a,b) ((long)(((WORD)(a))|((DWORD)((WORD)(b)))<<16))
typedef struct tagPOINT
{
   long x;
   long y;
} POINT;

WORD app_hinst;
typedef struct tagMSG {
   HWND hWnd;
   UINT message;
   WPARAM wParam;
   LPARAM lParam;
   DWORD time;
   POINT pt;
} MSG,*LPMSG;

#define PM_NOREMOVE 0x0000
#define PM_REMOVE 0x0001

// prototypes for MS-Windows message pump calls
BOOL PeekMessage( LPMSG lpMsg, HWND hWnd, UINT wMFMin, UINT wMFMAX, UINT wRMsg);
BOOL TranslateMessage( const MSG *lpmsg);
BOOL DispatchMessage( const MSG *lpmsg);
}
#endif

//-------[ blocking MS-Windows Message Pump ]-------
// Pumping messages allows other windows messages
// to be processed.
yield_pump()
{
#ifdef WU_APP
  MSG msg;

  while ( PeekMessage( &msg,NULL,0,0,PM_NOREMOVE))
  {
    // else peek, xlate, and dispatch it
    PeekMessage( &msg, NULL,0,0,PM_REMOVE);
    if ( msg.message == SNMP_SHUTDOWN_MSG ) return SNMP_CLASS_SHUTDOWN;
    TranslateMessage( &msg);
    DispatchMessage( &msg);
  }
#endif


//--------[ X Motif yield pump ]------------------------
// yield for use with X motif only
// no blocking yield implemented at this time

#ifdef X_MOTIF

#endif



  return SNMP_CLASS_SUCCESS;
}

//---------[ Process Pending Events ]-------------------------------
// Pull all available events out of their sockets - do not block
int SNMPProcessPendingEvents()
{
  int maxfds;
  fd_set readfds;
  fd_set writefds;
  fd_set exceptfds;
  int nfound = 0;
  struct timeval fd_timeout;
  msec now;
  int status;

  // do not allow select to block
  fd_timeout.tv_sec = 0;
  fd_timeout.tv_usec = 0;

  do {

    // Set up Select
    eventList.GetFdSets(maxfds, readfds, writefds, exceptfds);

#ifdef _XPG4_EXTENDED
    nfound = select(maxfds, &readfds, &writefds, &exceptfds, &fd_timeout);
#else // _XPG4_EXTENDED
    nfound = select(maxfds, (int *)&readfds, (int *)&writefds,
		    (int *)&exceptfds, &fd_timeout);
#endif // _XPG4_EXTENDED
    now.refresh();

    if (nfound > 0) {

      status = eventList.HandleEvents(maxfds, readfds, writefds, exceptfds);
      // TM should we do anything with bad status?

    } // found something on select
  } while (nfound > 0);

  // go through the message queue and resend any messages
  // which are past the timeout.
  status = eventList.DoRetries(now);

  return status;
}

//---------[ Process Events ]------------------------------------------
// Block until an event shows up - then handle the event(s)
int SNMPProcessEvents()
{
  int maxfds;
  fd_set readfds;
  fd_set writefds;
  fd_set exceptfds;
  int nfound = 0;
  struct timeval fd_timeout;
  msec now;
  msec sendTime;
  int status = 0;

  now.refresh();
  eventList.GetNextTimeout(sendTime);
  now.GetDelta(sendTime, fd_timeout);

  eventList.GetFdSets(maxfds, readfds, writefds, exceptfds);

#ifdef _XPG4_EXTENDED
  nfound = select(maxfds, &readfds, &writefds, &exceptfds, &fd_timeout);
#else // _XPG4_EXTENDED
  nfound = select(maxfds, (int *)&readfds, (int *)&writefds,
		    (int *)&exceptfds, &fd_timeout);
#endif // _XPG4_EXTENDED

  status = SNMPProcessPendingEvents();

  return status;
}

//---------[ Main Loop ]------------------------------------------
// Infinite loop which blocks when there is nothing to do and handles
// any events as they occur.
void SNMPMainLoop()
{
  do {
    SNMPProcessEvents();
  } while (!eventList.Done());
}

//---------[ Exit Main Loop ]---------------------------------------
// Force the SNMP Main Loop to terminate immediately
void SNMPExitMainLoop()
{
   eventList.SetDone();
}

//---------[ Block For Response ]-----------------------------------
// Wait for the completion of an outstanding SNMP event (msg).
// Handle any other events as they occur.
int SNMPBlockForResponse(const unsigned long req_id, Pdu &pdu)
{
  CSNMPMessage *msg;
  int status;

  // tell the msg queue we are looking for something
  snmpEventList->PushId(req_id);

  do {
    yield_pump(); 
    SNMPProcessPendingEvents();
  } while (!snmpEventList->Done());

  // reset the msg queue
  snmpEventList->PopId();

  msg = snmpEventList->GetEntry(req_id);
  if (msg) {
    // we found our response
    msg->GetPdu(status, pdu);

    // Dequeue the message
    snmpEventList->DeleteEntry(req_id);
    return  status;
  }
  else {
    // not in the send queue...must have timed out
    return SNMP_CLASS_TIMEOUT;
  }
}

void SNMPGetFdSets(int &maxfds, fd_set &readfds, fd_set &writefds,
		   fd_set &exceptfds)
{
  eventList.GetFdSets(maxfds, readfds, writefds, exceptfds);
}

Uint32 SNMPGetNextTimeout()
{
  int status;
  msec now, sendTime;

//TM: This function used to have an argument of sendTime and
//    would simply call eventList.GetNextTimeout(sendTime) and
//    return the status.  However, to avoid exposing the msec
//    class we now convert the msec to hundreths of seconds
//    and return that as a unsigned long.
// 25-Jan-96 TM

  sendTime = now;
  status = eventList.GetNextTimeout(sendTime);
  if (sendTime.IsInfinite()) {
    return (MAX_UINT32);
  }
  else {

    // Kludge: When this was first designed the units were millisecs
    // However, later on the units for the target class were changed
    // to hundreths of secs.  Divide millisecs by 10 to create the
    // hundreths of secs which the rest of the objects use.  
    // 25-Jan-96 TM

    sendTime -= now;
    return (((unsigned long) sendTime) / 10);
  }
}


#ifdef SNMPX11
// Required for integration with X11

SNMPX11Initialize( const XtAppContext app_context)
{

  // Save the app_context for future Xt calls.
  global_app_context = app_context;

  return SNMP_CLASS_SUCCESS;

}

void SnmpX11TimerCallback(XtPointer /*client_data*/, XtIntervalId */*id*/)
{ 
  // We have been called because one of our timers popped
  // The timer is automatically unregistered from X11 after it is fired
  global_interval_id = TIMER_NOT_SET;

  // Handle the event and any timeouts
  SNMPProcessPendingEvents();

  // Set a timer for the next retransmission
  SnmpX11SetTimer();
}

void SnmpX11InputCallback(XtPointer /*client_data*/, 
			  int */*source*/, XtInputId */*id*/)
{
  // We have been called because there is activity on one of our fds
  // Handle the event and any timeouts
  SNMPProcessPendingEvents();

  // Set a timer for the next retransmission
  SnmpX11SetTimer();
}

void SnmpX11SetTimer()
{
  msec nextTimeout;

  // if they have not yet initialized there is nothing we can do
  if (global_app_context == CONTEXT_NOT_SET)
    return;

  // Before returning control set a timer with X11 in case we
  // don't get any input before the next retransmission time
  eventList.GetNextTimeout(nextTimeout);
  if (global_interval_id != TIMER_NOT_SET) {
    // we already have a timer set
    if (global_next_timeout != nextTimeout) {
      // The timeout registered with X11 is no longer what we want
      XtRemoveTimeOut(global_interval_id);
    }
    else {
      // The current timeout is still valid
      return;
    }
  }
  if (nextTimeout.IsInfinite()) {
    // nothing left to wait for
    global_interval_id = TIMER_NOT_SET;
  }
  else {
    timeval alarm;
    msec now;
    unsigned long millisec;
    // calcuate and set the next timer
    now.GetDelta(nextTimeout, alarm);
    millisec = (alarm.tv_sec * 1000) + (alarm.tv_usec / 1000);
    global_next_timeout = nextTimeout;
    global_interval_id = XtAppAddTimeOut(global_app_context, millisec,
					 (XtTimerCallbackProc)SnmpX11TimerCallback, 0);
  }
  return;  
}

int SnmpX11AddInput(int inputFd, XtInputId &inputId)
{
  if (global_app_context == CONTEXT_NOT_SET) {
    // They have not called SnmpX11Initialize yet!
//TM: Need better error define
    return SNMP_CLASS_ERROR;
  }
  // Tell X11 to call us back for any activity on our file descriptor
  inputId = XtAppAddInput(global_app_context, inputFd,
			  (XtPointer) (XtInputReadMask | XtInputExceptMask),
			  (XtInputCallbackProc)SnmpX11InputCallback, 0);
  SnmpX11SetTimer();
  return SNMP_CLASS_SUCCESS;
}

int SnmpX11RemoveInput(XtInputId &inputId)
{ 
  // Tell X11 to stop watching our file descriptor
  XtRemoveInput(inputId);
  SnmpX11SetTimer();
  return SNMP_CLASS_SUCCESS;
}
#endif // SNMPX11

//----[ CSNMPMessageQueueElt class ]--------------------------------------

CEventListElt::CEventListElt(CEvents *events,
			     CEventListElt *next,
			     CEventListElt *previous):
  m_events(events), m_next(next), m_previous(previous)
{
    /*------------------------------------------*/
    /* Finish insertion into doubly linked list */
    /*------------------------------------------*/
  if (m_next)
    m_next->m_previous = this;
  if (m_previous)
    m_previous->m_next = this;
}

CEventListElt::~CEventListElt(){
    /*-------------------------------------*/
    /* Do deletion form doubly linked list */
    /*-------------------------------------*/
  if (m_next)
    m_next->m_previous = m_previous;
  if (m_previous)
    m_previous->m_next = m_next;

  if (m_events)
    delete m_events;
}

CEventListElt *CEventListElt::GetNext() {
  return(m_next);
}

CEvents *CEventListElt::GetEvents() {
  return(m_events);
}



//----[ CEventList class ]--------------------------------------

CEventList::CEventList():
  m_head(NULL,NULL,NULL), m_msgCount(0), m_done(0)
{

  // Automaticly add the SNMP message queue
  snmpEventList = new CSNMPMessageQueue;
  AddEntry((CEvents *)snmpEventList);

  // Automatically add the SNMP notification queue
  notifyEventList = new CNotifyEventQueue;
  AddEntry((CEvents *)notifyEventList);

  // Automaticly add the user-defined event queue
  udEventList = new CUDEventQueue;
  AddEntry((CEvents *)udEventList);

  // Automaticly add the user-defined timeout queue
  utEventList = new CUTEventQueue;
  AddEntry((CEvents *)utEventList);

}

CEventList::~CEventList(){
  CEventListElt *leftOver;
    /*--------------------------------------------------------*/
    /* walk the list deleting any elements still on the queue */
    /*--------------------------------------------------------*/
  while (leftOver = m_head.GetNext())
    delete leftOver;
}

CEvents * CEventList::AddEntry(CEvents *events) {

    /*---------------------------------------------------------*/
    /* Insert entry at head of list, done automagically by the */
    /* constructor function, so don't use the return value.    */
    /*---------------------------------------------------------*/
  (void) new CEventListElt(events, m_head.GetNext(), &m_head);
  m_msgCount++;
  return(events);
}

void CEventList::SetDone() {
  m_done += 1;
}

int CEventList::GetDone() {
  return m_done;
}

int CEventList::GetNextTimeout(msec &sendTime) {
  CEventListElt *msgEltPtr = m_head.GetNext();
  msec tmpTime = sendTime;

  sendTime.SetInfinite();	// set sendtime out into future
  while (msgEltPtr) {
    if (msgEltPtr->GetEvents()->GetCount() &&
	!msgEltPtr->GetEvents()->GetNextTimeout(tmpTime)) {
      if (sendTime > tmpTime)
	sendTime = tmpTime;
    }

    msgEltPtr = msgEltPtr->GetNext();
  }

  return 0;
}


void CEventList::GetFdSets(int &maxfds, fd_set &readfds, fd_set &writefds,
			   fd_set &exceptfds) {
  CEventListElt *msgEltPtr = m_head.GetNext();

  maxfds = 0;
  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  FD_ZERO(&exceptfds);
  while (msgEltPtr) {
    if (msgEltPtr->GetEvents()->GetCount()) {
      msgEltPtr->GetEvents()->GetFdSets(maxfds, readfds, writefds, exceptfds);
    }
    msgEltPtr = msgEltPtr->GetNext();
  }
}


int CEventList::GetCount() {
  return m_msgCount;
}


int CEventList::HandleEvents(const int maxfds,
			     const fd_set &readfds,
			     const fd_set &writefds,
			     const fd_set &exceptfds) {
  CEventListElt *msgEltPtr = m_head.GetNext();
  int status = SNMP_CLASS_SUCCESS;
  while (msgEltPtr){
    if (msgEltPtr->GetEvents()->GetCount()) {
      status = msgEltPtr->GetEvents()->HandleEvents(maxfds, readfds, writefds,
						    exceptfds);
    }
    msgEltPtr = msgEltPtr->GetNext();
  }
  return status;
}

int CEventList::DoRetries(const msec &sendtime) {
  CEventListElt *msgEltPtr = m_head.GetNext();
  int status = SNMP_CLASS_SUCCESS;
  while (msgEltPtr){
    if (msgEltPtr->GetEvents()->GetCount()) {
      status = msgEltPtr->GetEvents()->DoRetries(sendtime);
    }
    msgEltPtr = msgEltPtr->GetNext();
  }
  return status;
}

int CEventList::Done() {
  CEventListElt *msgEltPtr = m_head.GetNext();
  int status = SNMP_CLASS_SUCCESS;

  if (m_done) {
    m_done--;
    return 1;
  }

  while (msgEltPtr){
    if (msgEltPtr->GetEvents()->GetCount()) {
      status = msgEltPtr->GetEvents()->Done();
      if (status)
	break;
    }
    msgEltPtr = msgEltPtr->GetNext();
  }
  return status;
}
