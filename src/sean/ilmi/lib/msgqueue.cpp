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

      M S G Q U E U E . C P P

      MSG QUEUE CLASS DECLARATION


      Description:
	HPUX version of Snmp class

      Language:
	ANSI C++

      Operating System:
	HP-UX 9.0

      Modules Used:
	- Requires use of SR's Library
	- Optional, Bristol technologies WIND-U
	- Optional, X-Windows Motif

      Author:
	Peter E Mellquist
	HP RND R & D

      Compiler Directives:
		RTRMON_REV - compiles for router monitor use.
		This turns off use of timeticks, counter and gauge SNMP++
		objects.
	WU_APP - compiles for use with Bristol Technologies WIND-U
				 MS-Windows to UNIX porting tools.
	X_MOTIF - compiles for use with X Windows motif API.

      Date:
	July 27, 1994

      Changes:
	 PEM Sept 14, 1994 , added timeout and get/set many mf's
	 PEM MAY  15, 1995 , added async mode snmp objects
=====================================================================*/
char msgqueue_version[]="#(@)$Header: msgqueue.cpp,v 1.19 97/03/26 08:58:53 hmgr Exp $";

// For debuggin cout's
#include <iostream.h>

//-----[ includes ]----------------------------------------------------

//----[ snmp++ includes ]----------------------------------------------

#include "msgqueue.h"		// queue for holding outstanding messages
#include "msec.h"		// class for handling time in msecs

#define SNMP_PORT 161 

//--------[ globals ]---------------------------------------------------
CSNMPMessageQueue *snmpEventList;

//--------[ forwards ]--------------------------------------------------
//--------[ externs ]---------------------------------------------------
extern int send_snmp_request( int sock, unsigned char *send_buf,
			      size_t send_len, Address &address);

//----[ CSNMPMessage class ]-------------------------------------------

CSNMPMessage::CSNMPMessage(unsigned long id,
			   Snmp * snmp,
			   int socket,
			   SnmpTarget &target,
			   Pdu &pdu,
			   unsigned char * rawPdu,
			   size_t rawPduLen,
			   const Address & address,
			   snmp_callback callBack,
			   void * callData):
  m_uniqueId(id), m_snmp(snmp), m_socket(socket), m_pdu(pdu),
  m_rawPduLen(rawPduLen), m_callBack(callBack), m_callData(callData),
  m_reason(0), m_received(0)
{

  // reset pdu mvs
  set_error_index(&m_pdu, 0);
  set_error_status(&m_pdu, 0);
  set_request_id(&m_pdu, m_uniqueId);

  m_rawPdu = new unsigned char [rawPduLen];
  memcpy(m_rawPdu,rawPdu, rawPduLen);
  m_address = (Address *)address.clone();
  m_target = target.clone();

  SetSendTime();
}

CSNMPMessage::~CSNMPMessage() {
  delete [] m_rawPdu;
  delete m_address;
  delete m_target;
}

unsigned long CSNMPMessage::GetId() {
  return(m_uniqueId);
}

void CSNMPMessage::ResetId(const unsigned long newId) {
  m_uniqueId = newId;
}

void CSNMPMessage::SetSendTime() {
  m_sendTime.refresh();

  // Kludge: When this was first designed the units were millisecs
  // However, later on the units for the target class were changed
  // to hundreths of secs.  Multiply the hundreths of secs by 10
  // to create the millisecs which the rest of the objects use.
  // 11-Dec-95 TM
  m_sendTime += (m_target->get_timeout() * 10);
}

void CSNMPMessage::GetSendTime(msec &sendTime) {
  sendTime = m_sendTime;
}

int CSNMPMessage::GetSocket() {
  return m_socket;
}

int CSNMPMessage::GetPdu(int &reason, Pdu &pdu) {
  pdu = m_pdu;
  reason = m_reason;

  return 0;
}

int CSNMPMessage::SetPdu(const int reason, const Pdu &pdu) {
  m_received = 1;
  m_pdu = pdu;
  m_reason = reason;

  return 0;
}

int CSNMPMessage::GetReceived(){
  return m_received;
}

int CSNMPMessage::ResendMessage() {
  int status;

  if (m_received) {
    // Don't bother to resend if we already have the response
    SetSendTime();
    return SNMP_CLASS_SUCCESS;
  }

  if (m_target->get_retry() <= 0) {
    // This message has timed out

    // perform callback with the error
    Callback(SNMP_CLASS_TIMEOUT);

    return SNMP_CLASS_TIMEOUT;
  }

  m_target->set_retry(m_target->get_retry() - 1);
  SetSendTime();
  status = send_snmp_request(m_socket, m_rawPdu, m_rawPduLen, *m_address);
  if (status != 0) {
    return SNMP_CLASS_TL_FAILED;
  }

  return SNMP_CLASS_SUCCESS;
}

int CSNMPMessage::Callback(const int reason) {
  snmp_callback	  tmp_callBack;
  if (m_callBack) {
    // prevent callbacks from using this message
    tmp_callBack = m_callBack;
    m_callBack = NULL;

    tmp_callBack(reason, m_snmp, m_pdu, *m_target, m_callData);
    return 0;
  }
  else {
    return 1;
  }
}


//----[ CSNMPMessageQueueElt class ]--------------------------------------

CSNMPMessageQueueElt::CSNMPMessageQueueElt(CSNMPMessage *message,
					   CSNMPMessageQueueElt *next,
					   CSNMPMessageQueueElt *previous):
  m_message(message), m_next(next), m_previous(previous)
{
    /*------------------------------------------*/
    /* Finish insertion into doubly linked list */
    /*------------------------------------------*/
  if (m_next)
    m_next->m_previous = this;
  if (m_previous)
    m_previous->m_next = this;
}

CSNMPMessageQueueElt::~CSNMPMessageQueueElt(){
    /*-------------------------------------*/
    /* Do deletion form doubly linked list */
    /*-------------------------------------*/
  if (m_next)
    m_next->m_previous = m_previous;
  if (m_previous)
    m_previous->m_next = m_next;

  if (m_message)
    delete m_message;
}

CSNMPMessageQueueElt *CSNMPMessageQueueElt::GetNext() {
  return(m_next);
}

CSNMPMessage *CSNMPMessageQueueElt::GetMessage() {
  return(m_message);
}

CSNMPMessage *CSNMPMessageQueueElt::TestId(const unsigned long uniqueId){
  if (m_message && (m_message->GetId() == uniqueId))
    return(m_message);
  else
    return(NULL);
}



//----[ CSNMPMessageQueue class ]--------------------------------------

CSNMPMessageQueue::CSNMPMessageQueue():
  m_head(NULL,NULL,NULL), m_msgCount(0), m_idStack(NULL), m_stackTop(0)
{
  PushId(0);
}

CSNMPMessageQueue::~CSNMPMessageQueue(){
  CSNMPMessageQueueElt *leftOver;
    /*--------------------------------------------------------*/
    /* walk the list deleting any elements still on the queue */
    /*--------------------------------------------------------*/
  while (leftOver = m_head.GetNext())
    delete leftOver;

//  if (m_stackTop)
    delete [] m_idStack;
}

CSNMPMessage * CSNMPMessageQueue::AddEntry(unsigned long id,
					   Snmp * snmp,
					   int socket,
					   SnmpTarget &target,
					   Pdu &pdu,
					   unsigned char * rawPdu,
					   size_t rawPduLen,
					   const Address & address,
					   snmp_callback callBack,
					   void * callData) {

  CSNMPMessage *newMsg = new CSNMPMessage(id, snmp, socket, target, pdu,
					  rawPdu, rawPduLen, address,
					  callBack, callData);

    /*---------------------------------------------------------*/
    /* Insert entry at head of list, done automagically by the */
    /* constructor function, so don't use the return value.    */
    /*---------------------------------------------------------*/
  (void) new CSNMPMessageQueueElt(newMsg, m_head.GetNext(), &m_head);
  m_msgCount++;
#ifdef SNMPX11
  SnmpX11SetTimer();
#endif // SNMPX11
  return(newMsg);
}


CSNMPMessage *CSNMPMessageQueue::GetEntry(const unsigned long uniqueId){
  CSNMPMessageQueueElt *msgEltPtr = m_head.GetNext();
  CSNMPMessage *returnVal = NULL;

  while (msgEltPtr){
    if (returnVal = msgEltPtr->TestId(uniqueId))
      return(returnVal);
    msgEltPtr = msgEltPtr->GetNext();
  }
  return (NULL);
}

int CSNMPMessageQueue::DeleteEntry(const unsigned long uniqueId){
  CSNMPMessageQueueElt *msgEltPtr = m_head.GetNext();
  CSNMPMessage *returnVal = NULL;

  while (msgEltPtr){
    if (returnVal = msgEltPtr->TestId(uniqueId)){
      delete msgEltPtr;
      m_msgCount--;
#ifdef SNMPX11
      SnmpX11SetTimer();
#endif // SNMPX11
      return SNMP_CLASS_SUCCESS;
    }
    msgEltPtr = msgEltPtr->GetNext();
  }
  return SNMP_CLASS_INVALID_REQID;
}

void CSNMPMessageQueue::DeleteSocketEntry(const int socket){
  CSNMPMessageQueueElt *msgEltPtr = m_head.GetNext();
  CSNMPMessageQueueElt *tmp_msgEltPtr;
  CSNMPMessage *msg = NULL;

  while (msgEltPtr){
    msg = msgEltPtr->GetMessage();
    if (socket == msg->GetSocket()) {
      // Make a callback with an error
      (void) msg->Callback(SNMP_CLASS_SESSION_DESTROYED);
      tmp_msgEltPtr = msgEltPtr;
      msgEltPtr = tmp_msgEltPtr->GetNext();
      // delete the entry
      delete tmp_msgEltPtr;
    }
    else
      msgEltPtr = msgEltPtr->GetNext();
  }
}

CSNMPMessage * CSNMPMessageQueue::GetNextTimeoutEntry() {
  CSNMPMessageQueueElt *msgEltPtr = m_head.GetNext();
  msec bestTime;
  msec sendTime;
  CSNMPMessage *msg;
  CSNMPMessage *bestmsg = NULL;

  if (msgEltPtr) {
    bestmsg = msgEltPtr->GetMessage();
    bestmsg->GetSendTime(bestTime);
  }

// This would be much simpler if the queue was an ordered list!
  while (msgEltPtr){
    msg = msgEltPtr->GetMessage();
    msg->GetSendTime(sendTime);
    if (bestTime  > sendTime) {
      bestTime = sendTime;
      bestmsg = msg;
    }
    msgEltPtr = msgEltPtr->GetNext();
  }
  return bestmsg;
}

int CSNMPMessageQueue::GetNextTimeout(msec &sendTime) {
  CSNMPMessage *msg;

  msg = GetNextTimeoutEntry();
  if (msg == NULL) {
    // nothing in the queue...
    return 1;
  }
  else {
    msg->GetSendTime(sendTime);
  }
  return 0;
}


void CSNMPMessageQueue::GetFdSets(int &maxfds, fd_set &readfds,
				  fd_set &, fd_set &) {
  CSNMPMessageQueueElt *msgEltPtr = m_head.GetNext();
  int sock;

  while (msgEltPtr){
    sock = msgEltPtr->GetMessage()->GetSocket();
    FD_SET(sock, &readfds);
    if (maxfds < (sock+1))
      maxfds = sock+1;
    msgEltPtr = msgEltPtr->GetNext();
  }
}

int CSNMPMessageQueue::GetCount() {
  return m_msgCount;
}

void CSNMPMessageQueue::PushId(const unsigned long id) {
  unsigned long *newStack;

//TM: this is really dumb!  Fix it!
  newStack = new unsigned long [m_stackTop+1];

  for (int i=0; i< m_stackTop; i++)
    newStack[i] = m_idStack[i];

  newStack[m_stackTop] = id;
  m_stackTop++;

  delete [] m_idStack;
  m_idStack = newStack;
}

unsigned long CSNMPMessageQueue::PeekId() {
  return m_idStack[m_stackTop - 1];
}

unsigned long CSNMPMessageQueue::PopId() {
  unsigned long temp=0;

  if (m_stackTop) {
    m_stackTop--;
    temp = m_idStack[m_stackTop];
    m_idStack[m_stackTop] = 0;
  }
  return temp;
}

// forward
extern int receive_snmp_response( int sock, Pdu &pdu);
int CSNMPMessageQueue::HandleEvents(const int maxfds,
				    const fd_set &readfds,
				    const fd_set &,
				    const fd_set &) {
  CSNMPMessage *msg;
  Pdu tmppdu;
  unsigned long temp_req_id;
  int status;
  int recv_status;
  fd_set snmp_readfds, snmp_writefds, snmp_errfds;
  int tmp_maxfds = maxfds;

  // Only read from our own fds
  FD_ZERO(&snmp_readfds);
  FD_ZERO(&snmp_writefds);
  FD_ZERO(&snmp_errfds);
  GetFdSets(tmp_maxfds, snmp_readfds, snmp_writefds, snmp_errfds);

  for (int fd = 0; fd < maxfds; fd++) {
    if ((FD_ISSET(fd, &snmp_readfds)) &&
	(FD_ISSET(fd, &readfds))) {
      set_request_id(&tmppdu, 0);

      // get the response and put it into a Pdu
      recv_status = receive_snmp_response(fd, tmppdu);

      // find the corresponding msg in the message queue
      temp_req_id = tmppdu.get_request_id();
      msg = GetEntry(temp_req_id);
      if (!msg) {
	     // the sent message is gone!
	     // probably was canceled, ignore it
	     continue;
      }

      if (tmppdu.get_request_id()) {
	     // we correctly received the pdu

	     // save it back into the message
	     status = msg->SetPdu(recv_status, tmppdu);

	     // Do the callback
	     status = msg->Callback(SNMP_CLASS_ASYNC_RESPONSE);
	
	     if (!status) {
	       // this is an asynch response and the callback is done.
	       // no need to keep this message around;
	       // Dequeue the message
	       DeleteEntry(temp_req_id);
	     }
	  }
    } // if socket has data
  } // for all sockets

  return SNMP_CLASS_SUCCESS;
}

int CSNMPMessageQueue::DoRetries(const msec &now) {
  CSNMPMessage *msg;
  msec sendTime;
  int status = SNMP_CLASS_SUCCESS;
  while (msg = GetNextTimeoutEntry()) {

    msg->GetSendTime(sendTime);

    if (sendTime <= now) {

      // send out the message again
      status = msg->ResendMessage();
      if (status != 0) {
	if (status == SNMP_CLASS_TIMEOUT) {
	  // Dequeue the message
	   DeleteEntry(msg->GetId());
	}
	else {
	  // Some other send error
	  // should we dequeue the message?
	  // do we really want to return without processing the rest?
	  return status;
	}
      }
    }
    else {
      // the next timeout is still in the future...so we are done
      break;
    }
  }
  return status;
}


int CSNMPMessageQueue::Done() {
  unsigned long id;
  CSNMPMessage *msg;

  if ( id = PeekId()) {
    // we were looking for a req_id.  Did we find it?

    msg = GetEntry(id);
    if (msg) {
      return(msg->GetReceived());
    }
    else {
      // the message is not in the queue...must have timed out
      return 1;
    }
  }

  return 0;
}
