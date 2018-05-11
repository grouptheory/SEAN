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



  V B . C P P

  VARIABLE BINDING CLASS IMPLEMENTATION

  DESCRIPTION:
  This module contains the class implementation of the VB class.
  The Vb class is an encapsulation of the snmp variable binding.

  DESIGN:
  Peter E Mellquist

  AUTHOR:
  Peter E Mellquist

  LANGAUGE:
  ANSI C++

  OPERATING SYSTEMS:
  MS-Windows Win32
  BSD UNIX

=====================================================================*/
char vb_cpp_version[]="#(@) SNMP++ 2.6 $Header: vb.cpp,v 1.19 96/09/11 14:02:18 hmgr Exp $";

#include "oid.h"	       // include oid class defs
#include "vb.h"		       // include vb class defs
#include "snmperrs.h"      // error codes

#define IP_ADDR_SIZE  4
#define IPX_ADDR_SIZE 10
#define MAC_ADDR_SIZE 6

//---------------[ Vb::Vb( void) ]--------------------------------------
// constructor with no arguments
// makes an vb, unitialized
Vb::Vb( void)
  { iv_vb_value = NULL;
    exception_status = SNMP_CLASS_SUCCESS; };

//---------------[ Vb::Vb( const Oid &oid) ]-----------------------------
// constructor to initialize the oid
// makes a vb with oid portion initialized
Vb::Vb( const Oid &oid)
{ iv_vb_oid = oid;
  iv_vb_value = NULL;
  exception_status = SNMP_CLASS_SUCCESS;  };

//---------------[ Vb::Vb( const Vb &vb) ]-----------------------------
// copy constructor
Vb::Vb( const Vb &vb)
{  iv_vb_value = NULL;
   *this = vb; 
};

//--------------[ Vb::valid() ]-----------------------------------------
// returns validity of a Vb object
// must have a valid oid and value
int Vb::valid() const
{
#ifdef WHEN_WE_HAVE_SNMPNULL_CLASS
   if ( iv_vb_oid.valid() && iv_vb_value && iv_vb_value->valid() )
#else  // WHEN_WE_HAVE_SNMPNULL_CLASS
   if ( iv_vb_oid.valid() && 
	((iv_vb_value == NULL) || (iv_vb_value && iv_vb_value->valid())) )
#endif // WHEN_WE_HAVE_SNMPNULL_CLASS
     return TRUE;
   else
     return FALSE;
}

//---------------[ Vb::~Vb() ]------------------------------------------
// destructor
// if the vb has a oid or an octect string then
// the associated memory needs to be freed
Vb::~Vb()
{ free_vb(); };


//---------------[ Vb& Vb::operator=( const Vb &vb) ]--------------------
// overloaded assignment allows assigning one Vb to another
// this involves deep memory thus target vb needs to be freed
// before assigning source
Vb& Vb::operator=( const Vb &vb)
{
   free_vb();	 // free up target to begin with

   //-----[ reassign the Oid portion 1st ]
   vb.get_oid( iv_vb_oid);

   //-----[ next set the vb value portion ]
   if (vb.iv_vb_value == NULL){
     iv_vb_value = NULL;
   }
   else{
     iv_vb_value = vb.iv_vb_value->clone();
   }
   exception_status = vb.exception_status;

   return *this; // return self reference
};

//---------------[ Vb::set_oid( const Oid oid ) ]-----------------------
// set value oid only with another oid
void Vb::set_oid( const Oid oid)
{ iv_vb_oid = oid;};

//---------------[ Vb::get_oid( Oid &oid) ]-----------------------------
// get oid portion
void Vb::get_oid( Oid &oid) const
{ oid = iv_vb_oid;};


//----------------[ void Vb::free_vb() ]--------------------------------
// protected method to free memory
// this methos is used to free memory when assigning new vbs
// or destructing
// in the case of oids and octets, we need to do a deep free
void Vb::free_vb()
{
  if (iv_vb_value)
    delete iv_vb_value;
  exception_status = SNMP_CLASS_SUCCESS;
  iv_vb_value = NULL;
};


//---------------[ Vb::set_value( int i) ]-------------------------------
// set the value with an int
// C++ int maps to SMI int
void Vb::set_value( const int i)
{
  free_vb();
  iv_vb_value = new SnmpInt32(i);
};


//----------------[ Vb::set_value( long int i) ]--------------------------
// set the value with a long signed int
// C++ long int maps to SMI int 32
void Vb::set_value( const long int i)
{
  free_vb();
  iv_vb_value = new SnmpInt32(i);
};

 // set a Vb null, if its not already
 void Vb::set_null()
 {
    free_vb();
 };


//-----------------[ Vb::set_value( unsigned long int i) ]------------------
// set the value with an unsigned long int
// C++ unsigned long int maps to SMI UINT32
void Vb::set_value( const unsigned long int i)
{
  free_vb();
  iv_vb_value = new SnmpUInt32(i);
};



//------------[Vb::set_value ( general value) ]----------------------------
void Vb::set_value ( const SnmpSyntax &val)
{
  free_vb();
  iv_vb_value = val.clone();
};

//--------------[ Vb::set_value( char WINFAR * ptr) ]-------------------
// set value on a string
// makes the string an octet
// this must be a null terminates string
void Vb::set_value( const char WINFAR * ptr)
{
  free_vb();
  iv_vb_value = new OctetStr(ptr);
};


//---------------------[ Vb::get_value( int &i) ]----------------------
// get value int
// returns 0 on success and value
int Vb::get_value( int &i)
{
   if (iv_vb_value &&
       iv_vb_value->valid() &&
       (iv_vb_value->get_syntax() == sNMP_SYNTAX_INT ))
   {
     long lval;
     lval = *((SnmpInt32 *)iv_vb_value);// SnmpInt32 includes cast to long,
     i = (int) lval;				    // but not to int.
     return SNMP_CLASS_SUCCESS;
   }
   else
     return SNMP_CLASS_INVALID;
};



//--------------[ Vb::get_value( long int &i) ]-------------------------
// get the signed long int
// returns 0 on success and a value
int Vb::get_value( long int &i)
{
   if (iv_vb_value &&
       iv_vb_value->valid() &&
       (iv_vb_value->get_syntax() == sNMP_SYNTAX_INT32 ))
   {
     i = *((SnmpInt32 *)iv_vb_value);	// SnmpInt32 includes cast to long
     return SNMP_CLASS_SUCCESS;
   }
   else
    return SNMP_CLASS_INVALID;
};


//-----------------[  Vb::get_value( unsigned long int &i) ]--------------
// get the unsigned long int
// returns 0 on success and a value
int Vb::get_value( unsigned long int &i)
{
   if (iv_vb_value &&
       iv_vb_value->valid() &&
       ((iv_vb_value->get_syntax() == sNMP_SYNTAX_UINT32 ) ||
	(iv_vb_value->get_syntax() == sNMP_SYNTAX_CNTR32 ) ||
	(iv_vb_value->get_syntax() == sNMP_SYNTAX_GAUGE32 ) ||
	(iv_vb_value->get_syntax() == sNMP_SYNTAX_TIMETICKS )))
   {
     i = *((SnmpUInt32 *)iv_vb_value);	// SnmpUint32 has includes to ulong
     return SNMP_CLASS_SUCCESS;
   }
   else
     return SNMP_CLASS_INVALID;
};


//--------------[ Vb::get_value( unsigned char WINFAR * ptr, unsigned long &len)
// get a unsigned char string value
// destructive, copies into given ptr
// also returned is the length
//
// Note!! The user must provide a target string
// which is big enough to hold the string
int Vb::get_value( unsigned char WINFAR * ptr, unsigned long &len)
{
   if (iv_vb_value &&
       iv_vb_value->valid() &&
       ( iv_vb_value->get_syntax() == sNMP_SYNTAX_OCTETS))
   {
     OctetStr *p_os = (OctetStr *)iv_vb_value;
     len = p_os->len();
     for ( int i=0; i<(int)len ; i++ ) {
	   ptr[i] = (*p_os)[i];		// use OctetStr[] for "raw" access
     }
     return SNMP_CLASS_SUCCESS;
   }
   else
   {
     ptr[0] = '0';
     len = 0;
     return SNMP_CLASS_INVALID;
   }
};



//---------------[ Vb::get_value ]-------------------------------------
// get an unsigned char array
// caller specifies max len of target space
int Vb::get_value( unsigned char WINFAR * ptr,	// pointer to target space
		   unsigned long &len,		// returned len
		   unsigned long maxlen)	// max len of target space
{
   if ( iv_vb_value &&
        iv_vb_value->valid() &&
	(iv_vb_value->get_syntax() == sNMP_SYNTAX_OCTETS))
   {
     OctetStr *p_os = (OctetStr *)iv_vb_value;
     len = p_os->len();
     if (len > maxlen) len = maxlen;
     for ( int i=0; i<(int)len ; i++ ) {
	   ptr[i] = (*p_os)[i];		// use OctetStr[] for "raw" access
     }
     return SNMP_CLASS_SUCCESS;
   }
   else
   {
     ptr[0] = '0';
     len = 0;
     return SNMP_CLASS_INVALID;
   }
};


//---------------[ Vb::get_value( Value &val) ]--------
int Vb::get_value( SnmpSyntax &val)
{
  if (iv_vb_value){
    val = *iv_vb_value;
    if (val.valid())
	return SNMP_CLASS_SUCCESS;
    else
	return SNMP_CLASS_INVALID;
  }
  else
  {
//TM: should set val to be invalid
    return SNMP_CLASS_INVALID;
  }
};

//--------------[ Vb::get_value( char WINFAR *ptr) ]-------------------
// get a char * from an octet string
// the user must provide space or
// memory will be stepped on
int Vb::get_value( char WINFAR *ptr)
{
   if (iv_vb_value &&
       iv_vb_value->valid() &&
       ( iv_vb_value->get_syntax() == sNMP_SYNTAX_OCTETS))
   {
     OctetStr *p_os = (OctetStr *)iv_vb_value;
     unsigned long len = p_os->len();
     for ( int i=0; i<(int)len ; i++ ) {
	   ptr[i] = (*p_os)[i];		// use OctetStr[] for "raw" access
     }
     ptr[len] = 0;
     return SNMP_CLASS_SUCCESS;
   }
   else
     return SNMP_CLASS_INVALID;
};



//-----[ misc]--------------------------------------------------------

// return the current syntax
// This method violates Object Orientation but may be useful if
// the caller has a vb object and does not know what it is.
// This would be useful in the implementation of a browser.
SmiUINT32 Vb::get_syntax()
{ 
	if ( exception_status != SNMP_CLASS_SUCCESS)
		return exception_status;
	else
        return ( iv_vb_value ? iv_vb_value->get_syntax() : sNMP_SYNTAX_NULL);
};


static char blank_string[] = "";

// return the printabel value
char WINFAR *Vb::get_printable_value()
{
  if (iv_vb_value)
    return iv_vb_value->get_printable();
  else
    return blank_string;
}

// return the printable oid
char WINFAR *Vb::get_printable_oid()
{ return iv_vb_oid.get_printable(); };

// friend function to set exception status
void set_exception_status( Vb *vb, const SmiUINT32 status)
{
   vb->exception_status = status;
};

