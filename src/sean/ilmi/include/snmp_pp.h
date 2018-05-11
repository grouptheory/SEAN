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

     

   SNMP++ S N M P_PP . H

   SNMP CLASS DEFINITION

   VERSION
    2.6

   RCS INFO:
   $Header: snmp.h,v 1.22 96/09/11 14:01:35 hmgr Exp $

   DESCRIPTION:
   SNMP class defintion. The Snmp class provides an object oriented
   approach to SNMP. The SNMP class is an encapsulation of SNMP
   sessions, gets, sets and get nexts. The class manages all SNMP
   resources and provides complete retry and timeout capability.
   This file is portable. It does not rely on any WinSnmp or any
   other snmp libraries. The matching implementation of this file
   is platform specific.

   DESIGN:
   Peter E Mellquist

  AUTHOR:
  Peter E Mellquist

  LANGUAGE:
  ANSI C++

  OPERATING SYSTEMS:
   Win32
   BSD UNIX

=====================================================================*/
#ifndef _SNMP_CLS
#define _SNMP_CLS

//------[ C externals ]------------------------------------------------
extern "C"
{
#include <memory.h>               // memcpy's
#include <string.h>               // strlen, etc..
}

//-----[ snmp++ classes ]------------------------------------------------
#include "oid.h"                // snmp++ oid class
#include "vb.h"                 // snbmp++ vb class
#include "target.h"             // snmp++ target class
#include "pdu.h"                // snmp++ pdu class
#include "snmperrs.h"           // error macros and strings 
#include "address.h"            // snmp++ address class defs

#ifdef __unix
#include /**/ "eventlist.h"
#endif // __unix

//-----[ internally used defines ]----------------------------------------
#define MAXNAME 80                       // maximum name length
#define MAX_ADDR_LEN 10                  // maximume address len, ipx is 4+6
#define SNMP_SHUTDOWN_MSG 0x0400+177     // shut down msg for stoping a blocked message

//-----[ async defines for engine ]---------------------------------------
#define sNMP_PDU_GET_ASYNC       21
#define sNMP_PDU_GETNEXT_ASYNC   22
#define sNMP_PDU_SET_ASYNC       23
#define sNMP_PDU_GETBULK_ASYNC   24
#define sNMP_PDU_INFORM_ASYNC    25

//-----[ trap / notify macros ]-------------------------------------------
#define IP_NOTIFY  162     // IP notification
#define IPX_NOTIFY 0x2121  // IPX notification


//------[ forward declaration of Snmp class ]-----------------------------
class Snmp;

//-----------[ async methods callback ]-----------------------------------
// async methods require the caller to provide a callback
// address of a function with the following typedef
typedef void (*snmp_callback)( int,             // reason
                               Snmp*,           // session handle
                               Pdu &,           // pdu passsed in
                               SnmpTarget &,    // source target
                               void * );        // optional callback data


//------------[ SNMP Class Def ]---------------------------------------------
//
class DLLOPT Snmp {


 public:
  //------------------[ constructor,blocked usage ]---------------------
  Snmp( int &status);                    // construction status

  //-------------------[ destructor ]------------------------------------
  ~Snmp();

  //-------------------[ returns error string ]--------------------------
  char * error_msg( const int c);
  
  //----------------------[ cancel a pending request ]--------------------
  virtual int cancel( const unsigned long rid);

  //------------------------[ get ]---------------------------------------
  virtual int get( Pdu &pdu,             // pdu to get
                   SnmpTarget &target);  // get target

  //------------------------[ get async ]----------------------------------
  virtual int get( Pdu &pdu,                       // pdu to get async
                   SnmpTarget &target,             // destination target
                   const snmp_callback callback,   // async callback to use
                   const void * callback_data=0);  // callback data

  //------------------------[ get next ]-----------------------------------
  virtual int get_next( Pdu &pdu,             // pdu to get_next
                        SnmpTarget &target);  // get target

  //------------------------[ get next async ]-----------------------------
  virtual int get_next( Pdu &pdu,                      // pdu to get_next async
                        SnmpTarget &target,            // destination target
                        const snmp_callback callback,  // async callback to use
                        const void * callback_data=0); // callback data

  //-------------------------[ set ]---------------------------------------
  virtual int set( Pdu &pdu,            // pdu to set
                   SnmpTarget &target); // target address

  //------------------------[ set async ]----------------------------------
  virtual int set( Pdu &pdu,                       // pdu to set async
                   SnmpTarget &target,             // destination target
                   const snmp_callback callback,   // async callback
                   const void * callback_data=0);  // callback data

  //-----------------------[ get bulk ]------------------------------------
  virtual int get_bulk( Pdu &pdu,                // pdu to get_bulk
                        SnmpTarget &target,      // destination target
                        const int non_repeaters, // number of non repeaters
                        const int max_reps);     // maximum number of repetitions

  //-----------------------[ get bulk async ]------------------------------
  virtual int get_bulk( Pdu &pdu,                      // pdu to get_bulk async
                        SnmpTarget &target,            // destination target
                        const int non_repeaters,       // number of non repeaters
                        const int max_reps,            // max repeaters
                        const snmp_callback callback,        // async callback
                        const void *callback_data=0);        // callback data


  //-----------------------[ send a trap ]----------------------------------
  virtual int trap( Pdu &pdu,                     // pdu to send
                    SnmpTarget &target);          // destination target

  
  //----------------------[ blocking inform, V2 only]------------------------
  virtual int inform( Pdu &pdu,                // pdu to send
                      SnmpTarget &target);     // destination target

  //----------------------[ asynch inform, V2 only]------------------------
  virtual int inform( Pdu &pdu,                      // pdu to send
                      SnmpTarget &target,            // destination target
                      const snmp_callback callback,  // callback function
                      const void * callback_data=0); // callback data


  //-----------------------[ register to get informs]-------------------------
  virtual int notify_register( const OidCollection    &trapids,  // ids to listen for
                               const TargetCollection &targets,  // targets to listen for
                               const snmp_callback callback,     // callback to use
                               const void *callback_data=0);     // callback data

  //-----------------------[ register to get informs]-------------------------
  virtual int notify_register( const OidCollection    &trapids,           // ids to listen for
                               const TargetCollection &targets,           // targets to listen for
                               const AddressCollection &listen_addresses, // interfaces to listen on
                               const snmp_callback callback,              // callback to use
                               const void *callback_data=0);              // callback data


  //-----------------------[ un-register to get traps]----------------------
  virtual int notify_unregister();

  //-----------------------[ get notify register info ]---------------------
  virtual int get_notify_filter( OidCollection &trapids, TargetCollection &targets);


  //-----------------------[ access the trap reception info ]---------------
  snmp_callback get_notify_callback();

  void * get_notify_callback_data();


 protected:
   //---[ instance variables ]
   HANDLE iv_snmp_session;             // session handle
   HANDLE pdu_handler;                 // pdu handler win proc
   int construct_status;               // status of construction

   // inform receive member variables
   TargetCollection *notify_targets;
   OidCollection *notify_ids;
   AddressCollection *listen_addresses;
   snmp_callback  notifycallback;
   void * notifycallback_data;

   //-----------[ Snmp Engine ]----------------------------------------
   // gets, sets and get nexts go through here....
   // This mf does all snmp sending (using sendMsg) and reception
   // except for traps which are sent using trap().
   int snmp_engine( Pdu &pdu,                  // pdu to use
                    long int non_reps,         // get bulk only
                    long int max_reps,         // get bulk only
                    SnmpTarget &target,        // destination target
                    const snmp_callback cb,    // async callback function
                    const void *cbd);          // callback data

   //-----------[ Snmp sendMsg ]---------------------------------
   // send a message using whatever underlying stack is available.
   // 
   int sendMsg( SnmpTarget &target,      // target of send
                Pdu &pdu,                // the pdu to send
				long int non_reps,       // # of non repititions
                long int max_reps,       // # of max repititions
    	    	long int request_id);    // id to use in send
};

#endif //_SNMP_CLS

