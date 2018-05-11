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

    
  C O U N T E R 6 4. C P P   
      
  COUNTER64 CLASS IMPLEMENTATION
       
  VERSION 2.6
      

  DESIGN:
  Peter E. Mellquist          
          
  AUTHOR:   
  Peter E Mellquist
        
  DATE:  
  June 15, 1994
        
  DESCRIPTION:
  Implementation for
  Counter64 ( 64 bit counter class).
         
        
  LANGUAGE:
  ANSI C++
        
  OPERATING SYSTEM(S):
  MS-Windows Win32
  BSD UNIX
        
=====================================================================*/ 
char counter64_cpp_version[]="@(#) SNMP++ 2.6 $Header: ctr64.cpp,v 1.8 96/09/11 14:01:49 hmgr Exp $";
 
#include "ctr64.h"
#include <stdio.h>   // for pretty printing...

#define MAX32 4294967295

//-----------[ syntax type ]----------------------------------------------
SmiUINT32 Counter64::get_syntax()
{ return sNMP_SYNTAX_CNTR64; };

//------------------[ constructor with no value ]------------------------ 
Counter64::Counter64( void)
{
  smival.syntax = sNMP_SYNTAX_CNTR64;
  smival.value.hNumber.hipart = 0;
  smival.value.hNumber.lopart = 0;
};

//------------------[ constructor with values ]-------------------------- 
Counter64::Counter64( unsigned long hiparm, unsigned long loparm)
{
  smival.syntax = sNMP_SYNTAX_CNTR64;
  smival.value.hNumber.hipart = hiparm;
  smival.value.hNumber.lopart = loparm;
};

//------------------[ constructor with low value only ]------------------ 
Counter64::Counter64( unsigned long loparm )
{
  smival.syntax = sNMP_SYNTAX_CNTR64;
  smival.value.hNumber.hipart = 0;
  smival.value.hNumber.lopart = loparm;
};

//------------------[ copy constructor ]--------------------------------- 
Counter64::Counter64( const Counter64 &ctr64 )
{
  smival.syntax = sNMP_SYNTAX_CNTR64;
  smival.value.hNumber.hipart = ctr64.high();
  smival.value.hNumber.lopart = ctr64.low();
};

//------------------[ destructor ]--------------------------------- 
Counter64::~Counter64()
{};


//------------------[ Counter64::high() ]------------------------------ 
// return the high part   
unsigned long int Counter64::high() const
{ 
  return smival.value.hNumber.hipart; 
};


//------------------[ Counter64::low() ]-------------------------------        
// return the low part   
unsigned long int Counter64::low() const
{ 
  return smival.value.hNumber.lopart;
};

//------------------[ set_high( const unsigned long int h) ]-----------        
// set the high part   
void Counter64::set_high( const unsigned long int h)
{ smival.value.hNumber.hipart = h; };
   
//------------------[ set_low( const unsigned long int l) ]------------        
// set the low part   
void Counter64::set_low( const unsigned long int l)
{ smival.value.hNumber.lopart = l; }; 
        
//------------------[ operator=( const Counter64 &ctr64) ]-------------     
// assign a ctr64 to a ctr64 
Counter64& Counter64::operator=( const Counter64 &ctr64) 
{ 
   smival.value.hNumber.hipart = ctr64.high();
   smival.value.hNumber.lopart = ctr64.low();
   return *this;
};         

//-------------------[ operator=( const unsigned long int i) ]---------
// assign a ul to a ctr64, clears the high part
// and assugns the low part
Counter64& Counter64::operator=( const unsigned long int i) 
{ 
   smival.value.hNumber.hipart = 0;
   smival.value.hNumber.lopart = i;
   return *this;
};


//-----------[ c64_to_ld( Counter64 c64) ]-----------------------------
// convert a Counter 64 to a long double
long double Counter64::c64_to_ld( Counter64 &c64)
{
  long double ld = c64.high();
  ld *= MAX32;
  ld += c64.low();
  return (ld);
};


//-----------[ ld_to_c64( long double ld) ]----------------------------      
// convert a long double to a Counter64
Counter64 Counter64::ld_to_c64( long double ld)
{
     unsigned long h;
     unsigned long l;
     h = (unsigned long)(ld / MAX32);
     l = (unsigned long)(ld-h);
     return ( Counter64( h,l));
};  

//----------[ Counter64::operator+( const Counter64 &c) ]---------------
// add two Counter64s
Counter64 Counter64::operator+( const Counter64 &c)
{
   long double ld1, ld2, ldsum;
   Counter64 result;
   Counter64 temp( c);
   ld1 = c64_to_ld( *this);
   ld2 = c64_to_ld( temp);
   ldsum = ld1+ld2;
   result = ld_to_c64( ldsum);
   
   return ( Counter64( result.high(),result.low() ));
}; 

// add a unsigned long and a Counter64
Counter64 operator+( unsigned long ul, const Counter64 &c64)
{  
   return   Counter64( ul) + c64;
};    
     
//------------[ Counter64::operator-( const Counter64 &c) ]-------------
// subtract two Counter64s
Counter64 Counter64::operator-( const Counter64 &c)
{  
   long double ld1, ld2, ldsum;
   Counter64 result;
   Counter64 temp( c);
   ld1 = c64_to_ld( *this);
   ld2 = c64_to_ld( temp);
   ldsum = ld1-ld2;
   result = ld_to_c64( ldsum);
   
   return ( Counter64( result.high(),result.low() ));

}; 

// add a unsigned long and a Counter64
Counter64 operator-( unsigned long ul, const Counter64 &c64)
{  
   return   Counter64( ul) - c64;
};    
        
//------------[ Counter64::operator*( const Counter64 &c) ]-------------
// multiply two Counter64s
Counter64 Counter64::operator*( const Counter64 &c)
{
   long double ld1, ld2, ldsum;
   Counter64 result;
   Counter64 temp( c);
   ld1 = c64_to_ld( *this);
   ld2 = c64_to_ld( temp);
   ldsum = ld1*ld2;
   result = ld_to_c64( ldsum);
   
   return ( Counter64( result.high(),result.low() )); 

}; 

// add a unsigned long and a Counter64
Counter64 operator*( unsigned long ul, const Counter64 &c64)
{  
   return   Counter64( ul) * c64;
};  
     
//------------[ Counter64::operator/( const Counter64 &c) ]--------------     
// divide two Counter64s
Counter64 Counter64::operator/( const Counter64 &c)
{
   long double ld1, ld2, ldsum;
   Counter64 result;
   Counter64 temp( c);
   ld1 = c64_to_ld( *this);
   ld2 = c64_to_ld( temp);
   ldsum = ld1/ld2;
   result = ld_to_c64( ldsum);
   
   return ( Counter64( result.high(),result.low() ));
};

// add a unsigned long and a Counter64
Counter64 operator/( unsigned long ul, const Counter64 &c64)
{  
   return   Counter64( ul) / c64;
};  
     
//-------[ overloaded equivlence test ]----------------------------------
int operator==( Counter64 &lhs, Counter64 &rhs)
{
   if (( lhs.high() == rhs.high()) && ( lhs.low() == rhs.low()))
      return TRUE;
   else
      return FALSE;       
};
     
//-------[ overloaded not equal test ]-----------------------------------
int operator!=( Counter64 &lhs, Counter64 &rhs)
{
   if (( lhs.high() != rhs.high()) || ( lhs.low() != rhs.low()))
      return TRUE;
   else
      return FALSE;
};             
     
//--------[ overloaded less than ]---------------------------------------
int operator<( Counter64 &lhs, Counter64 &rhs)
{    
   long double ld_lhs, ld_rhs;
   ld_lhs = lhs.c64_to_ld( lhs);
   ld_rhs = rhs.c64_to_ld( rhs);
   if ( ld_lhs < ld_rhs)
      return TRUE;
   else
      return FALSE;   
};

//---------[ overloaded less than or equal ]-----------------------------
int operator<=( Counter64 &lhs, Counter64 &rhs)
{  
   long double ld_lhs, ld_rhs;
   ld_lhs = lhs.c64_to_ld( lhs);
   ld_rhs = rhs.c64_to_ld( rhs);
   if ( ld_lhs <= ld_rhs)
      return TRUE;
   else
      return FALSE;   
};
     
//---------[ overloaded greater than ]-----------------------------------
int operator>( Counter64 &lhs, Counter64 &rhs)
{  
   long double ld_lhs, ld_rhs;
   ld_lhs = lhs.c64_to_ld( lhs);
   ld_rhs = rhs.c64_to_ld( rhs);
   if ( ld_lhs > ld_rhs)
      return TRUE;
   else
      return FALSE;   
};
     
//----------[ overloaded greater than or equal ]-------------------------
int operator>=( Counter64 &lhs, Counter64 &rhs)
{
   long double ld_lhs, ld_rhs;
   ld_lhs = lhs.c64_to_ld( lhs);
   ld_rhs = rhs.c64_to_ld( rhs);
   if ( ld_lhs >= ld_rhs)
      return TRUE;
   else
      return FALSE;     
};

//----------[ return ASCII format ]-------------------------
// TODO:  Fix up to do real 64bit decimal value printing... 
//        For now, print > 32-bit values in hex
char * Counter64::get_printable() 
{ 
  if ( high() != 0 )
    sprintf(output_buffer, "0x%X%08X", high(), low());
  else
    sprintf(output_buffer, "%d", low());
  return output_buffer;
};


//----------------[ general Value = operator ]---------------------
SnmpSyntax& Counter64::operator=( SnmpSyntax &val)
{
  // protect against assignment from itself
  if ( this == &val )
      return *this;

  smival.value.hNumber.lopart = 0;	// pessimsitic - assume no mapping
  smival.value.hNumber.hipart = 0;

  // try to make assignment valid
  if (val.valid()){
    switch (val.get_syntax()){
      case sNMP_SYNTAX_CNTR64:
	smival.value.hNumber.hipart = 
		((Counter64 &)val).smival.value.hNumber.hipart;
	smival.value.hNumber.lopart = 
		((Counter64 &)val).smival.value.hNumber.lopart;
	break;

      case sNMP_SYNTAX_CNTR32:
      case sNMP_SYNTAX_TIMETICKS:
      case sNMP_SYNTAX_GAUGE32:
   // case sNMP_SYNTAX_UINT32:		.. indistinguishable from GAUGE32
      case sNMP_SYNTAX_INT32:
	// take advantage of union...
	smival.value.hNumber.lopart = ((Counter64 &)val).smival.value.uNumber;
	smival.value.hNumber.hipart = 0;
	break;
    }
  }
  return *this;
};

//----------------[ Counter64::clone() ]-----------------------------------
// create a new instance of this Value
SnmpSyntax* Counter64::clone() const 
{ return ( SnmpSyntax *) new Counter64(*this); };

//----------------[ Counter64::valid() ]-------------------------------------
int Counter64::valid() const
{ return 1; };

