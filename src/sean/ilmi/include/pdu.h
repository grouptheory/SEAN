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

		
  SNMP++ P D U . H

  PDU CLASS DEFINITION

  VERSION:
  2.6

  RCS INFO:
  $Header: pdu.h,v 1.13 96/06/05 19:10:25 hmgr Exp $

  DESIGN:
  Peter E Mellquist

  AUTHOR:
  Peter E Mellquist

  LANGUAGE:
  ANSI C++

  OPERATING SYSTEMS:
  MS-Windows Win32
  BSD UNIX

  DESCRIPTION:
  Pdu class definition. Encapsulation of an SMI Protocol
  Data Unit (PDU) in C++.


=====================================================================*/

#ifndef _PDU_CLS
#define _PDU_CLS


#define MAX_VBS 25
#include "vb.h"	      // include Vb class definition

//=======================================================================
//		     Pdu Class
//=======================================================================
class DLLOPT Pdu {


//-----------[ public member functions ]---------------------------------
public:

  // constructor no args
  Pdu( void);

  // constructor with vbs and count
  Pdu( Vb* pvbs, const int pvb_count);

  // constructor with another Pdu instance
  Pdu( const Pdu &pdu);

  // destructor
  ~Pdu();

  // assignment to another Pdu object overloaded
  Pdu& operator=( const Pdu &pdu);

  // append a vb to the pdu
  Pdu& operator+=( Vb &vb);

  // extract all Vbs from Pdu
  int get_vblist( Vb* pvbs, const int pvb_count);

  // deposit all Vbs to Pdu
  int set_vblist( Vb* pvbs, const int pvb_count);

  // get a particular vb
  // where 0 is the first vb
  int get_vb( Vb &vb, const int index) const;

  // set a particular vb
  // where 0 is the first vb
  int set_vb( Vb &vb, const int index);

  // return number of vbs
  int get_vb_count() const;

  // return the error status
  int get_error_status();

  // set the error status
  DLLOPT friend void set_error_status( Pdu *pdu, const int status);

  // return the error index
  int get_error_index();

  // set the error index
  DLLOPT friend void set_error_index( Pdu *pdu, const int index);

  // clear error status
  DLLOPT friend void clear_error_status( Pdu *pdu);

  // clear error index
  DLLOPT friend void clear_error_index( Pdu *pdu);

  // return the request id
  unsigned long get_request_id();

  // set the request id
  DLLOPT friend void set_request_id( Pdu *pdu, const unsigned long rid);

  // get the pdu type
  unsigned short get_type();

  // set the pdu type
  void set_type( unsigned short type);

  // returns validity of Pdu instance
  int valid() const;

  // trim off the last vb, if present
  int trim(const int position=1);

  // delete a Vb anywhere within the Pdu
  int delete_vb( const int position);

  // set notify timestamp
  void set_notify_timestamp( const TimeTicks & timestamp);

  // get notify timestamp
  void get_notify_timestamp( TimeTicks & timestamp);

  // set the notify id
  void set_notify_id( const Oid id);

  // get the notify id
  void get_notify_id( Oid &id);

  // set the notify enterprise
  void set_notify_enterprise( const Oid &enterprise);

  // get the notify enterprise
  void get_notify_enterprise( Oid & enterprise);



  //-------------[ protected instance variables ]--------------------------
  protected:
    Vb *vbs[MAX_VBS];	         // pointer to array of Vbs
    int vb_count;		         // count of Vbs
    int error_status;		     // SMI error status
    int error_index;		     // SMI error index
    int validity;		         // valid boolean
    unsigned long request_id;	 // SMI request id
    unsigned short pdu_type;	 // derived at run time based
				                 // on request type
    // for notify Pdu objects only
    // traps & notifies
    TimeTicks notify_timestamp;      // a timestamp associated with an infor
    Oid notify_id;                   // an id 
    Oid notify_enterprise;
};
#endif //_PDU_CLS

