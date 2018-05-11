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



  C O U N T E R. C P P

  COUNTER32 CLASS IMPLEMENTATION

  VERSION:
  2.6

  DESIGN:
  Peter E Mellquist

  AUTHOR:
  Peter E Mellquist

  DATE:
  June 15, 1994

  LANGUAGE:
  ANSI C++

  OPERATING SYSTEMS:
  MS-Windows Win32
  BSD UNIX

  DESCRIPTION:
  Class implementation for SMI Counter32 class.


=====================================================================*/
char counter_cpp_version[]="@(#) SNMP++ 2.6 $Header: counter.cpp,v 1.10 96/09/11 14:01:47 hmgr Exp $";

#include "counter.h"

// constructor no value
Counter32::Counter32( void):SnmpUInt32()
   { smival.syntax = sNMP_SYNTAX_CNTR32; };

// constructor with a value
Counter32::Counter32( const unsigned long i):SnmpUInt32(i)
   { smival.syntax = sNMP_SYNTAX_CNTR32; };

// copy constructor
Counter32::Counter32( const Counter32 &c)
   { this->smival.value.uNumber = c.smival.value.uNumber;
     smival.syntax = sNMP_SYNTAX_CNTR32;
     valid_flag = 1;
   };

// syntax type
SmiUINT32 Counter32::get_syntax()
{ return sNMP_SYNTAX_CNTR32; };

// general assignment from any Value
SnmpSyntax& Counter32::operator=( SnmpSyntax &in_val){
  if ( this == &in_val )	// handle assignement from itself
      return *this;

  valid_flag = 0;		// will get set true if really valid
  if (in_val.valid())
  {  
    switch (in_val.get_syntax())
    {
      case sNMP_SYNTAX_UINT32:
   // case sNMP_SYNTAX_GAUGE32:    	.. indistinquishable from UINT32
      case sNMP_SYNTAX_CNTR32:
      case sNMP_SYNTAX_TIMETICKS:
      case sNMP_SYNTAX_INT32:		// implied cast int -> uint 
	  this->smival.value.uNumber = 
		((Counter32 &)in_val).smival.value.uNumber;  
  	  valid_flag = 1;
	  break;
    }
  }
  return *this;
};

// overloaded assignment
Counter32& Counter32::operator=( const unsigned long int i)
  { this->smival.value.uNumber=i; return *this;};

// overloaded assignment
Counter32& Counter32::operator=( const Counter32 &uli)
  { this->smival.value.uNumber = uli.smival.value.uNumber;
    return *this;};

// otherwise, behave like an unsigned long int
Counter32::operator unsigned long()
  { return this->smival.value.uNumber; };

// clone
SnmpSyntax * Counter32::clone() const
  { return ( SnmpSyntax *) new Counter32(*this); };

