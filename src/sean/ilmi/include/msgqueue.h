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

      M S G Q U E U E . H

      CSNMPMessageQueue CLASS DEFINITION

      COPYRIGHT HEWLETT PACKARD COMPANY 1995

      INFORMATION NETWORKS DIVISION

      NETWORK MANAGEMENT SECTION


      VERSION:
	$Revision: 1.10 $

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
	Queue for holing SNMP event sources (outstanding snmp messages)


      COMPILER DIRECTIVES:
	UNIX - For UNIX build

      CHANGE HISTORY:
=====================================================================*/
#ifndef _MSGQUEUE
#define _MSGQUEUE

//----[ includes ]-----------------------------------------------------
extern "C"
{
#include <sys/types.h>		// NOTE:  due to 10.10 bug, order is important
				//   in that all routines must include types.h
				//   and time.h in same order otherwise you
				//   will get conflicting definitions of
				//   "fd_set" resulting in link time errors.
#include <sys/time.h>		// time stuff and fd_set
}
//----[ snmp++ includes ]----------------------------------------------
#include "vb.h"	       // class def for vbs
#include "address.h"   // class def for addresses
#include "target.h"    // class def for targets
#include "pdu.h"
#include "msec.h"
#include "snmp_pp.h"      // class def for snmp
#include "eventlist.h"

//----[ defines ]------------------------------------------------------



//----[ CSNMPMessage class ]-------------------------------------------


  /*-----------------------------------------------------------*/
  /* CSNMPMessage					       */
  /*   a description of a single MIB access operation.	       */
  /*-----------------------------------------------------------*/
class CSNMPMessage {
public:
  CSNMPMessage(unsigned long id,
	       Snmp * snmp,
	       int socket,
	       SnmpTarget &target,
	       Pdu &pdu,
	       unsigned char * rawPdu,
	       size_t rawPduLen,
	       const Address & address,
	       snmp_callback callBack,
	       void * callData);
  ~CSNMPMessage();
  unsigned long GetId();
  void ResetId(const unsigned long newId);
  void SetSendTime();
  void GetSendTime(msec &sendTime);
  int GetSocket();
  int SetPdu(const int reason, const Pdu &pdu);
  int GetPdu(int &reason, Pdu &pdu);
  int GetReceived();
  int ResendMessage();
  int Callback(const int reason);

private:
  unsigned long	  m_uniqueId;
  msec		  m_sendTime;
  Snmp *	  m_snmp;
  int		  m_socket;
  SnmpTarget *	  m_target;
  Pdu		  m_pdu;
  unsigned char * m_rawPdu;
  size_t	  m_rawPduLen;
  Address *	  m_address;
  snmp_callback	  m_callBack;
  void *	  m_callData;
  int		  m_reason;
  int		  m_received;
};

  /*-----------------------------------------------------------*/
  /* CSNMPMessageQueueElt				       */
  /*   a container for a single item on a linked lists of      */
  /*  CSNMPMessages.					       */
  /*-----------------------------------------------------------*/
class CSNMPMessageQueueElt {
  public:
    CSNMPMessageQueueElt(CSNMPMessage *message,
			 CSNMPMessageQueueElt *next,
			 CSNMPMessageQueueElt *previous);

    ~CSNMPMessageQueueElt();
    CSNMPMessageQueueElt *GetNext();
    CSNMPMessage *GetMessage();
    CSNMPMessage *TestId(const unsigned long uniqueId);

  private:
    CSNMPMessage *m_message;
    class CSNMPMessageQueueElt *m_next;
    class CSNMPMessageQueueElt *m_previous;
};

  /*-----------------------------------------------------------*/
  /* CSNMPMessageQueue					       */
  /*   class describing a collection of outstanding SNMP msgs. */
  /*-----------------------------------------------------------*/
class CSNMPMessageQueue: public CEvents {
  public:
    CSNMPMessageQueue();
    ~CSNMPMessageQueue();
    CSNMPMessage *AddEntry(unsigned long id,Snmp * snmp, int socket,
			   SnmpTarget &target, Pdu &pdu, unsigned char * rawPdu,
			   size_t rawPduLen, const Address & address,
			   snmp_callback callBack, void * callData);
    CSNMPMessage *GetEntry(const unsigned long uniqueId);
    int DeleteEntry(const unsigned long uniqueId);
    void DeleteSocketEntry(const int socket);
  // find the next msg that will timeout
    CSNMPMessage *GetNextTimeoutEntry();
  // find the next timeout
    int GetNextTimeout(msec &sendTime);
  // set up parameters for select
    void GetFdSets(int &maxfds, fd_set &readfds, fd_set &writefds,
		  fd_set &exceptfds);
  // return number of outstanding messages
    int GetCount();

  // tell the queue we are looking for an id
    void PushId(const unsigned long id);
    unsigned long PeekId();
    unsigned long PopId();

    int HandleEvents(const int maxfds,
		     const fd_set &readfds,
		     const fd_set &writefds,
		     const fd_set &exceptfds);
    int DoRetries(const msec &sendtime);

    int Done();

  private:
    CSNMPMessageQueueElt m_head;
    int m_msgCount;
    unsigned long *m_idStack;
    int m_stackTop;
};

//--------[ externs ]---------------------------------------------------
extern CSNMPMessageQueue *snmpEventList; // contains all outstanding messages

#endif
