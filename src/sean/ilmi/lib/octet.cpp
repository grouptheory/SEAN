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



  O C T E T . C P P

  OCTETSTR CLASS IMPLEMENTATION

  VERSION:
  2.6

  DESIGN:
  Peter E Mellquist

  AUTHOR:
  Peter E Mellquist

  LANGUAGE:
  ANSI C++

  OPERATING SYSTEMS:
  MS-WINDOWS Win32
  BSD UNIX

  DESCRIPTION:
  This class is fully contained and does not rely on or any other
  SNMP libraries. This class is portable across any platform
  which supports C++.


=====================================================================*/
char octet_cpp_version[]="@(#)SNMP++ 2.6 $Header: octet.cpp,v 1.18 96/09/11 14:01:57 hmgr Exp $";

#include "octet.h"    // include definition for octet class
#include <ctype.h>    // for isprint() used by get_printable()
#include <stdio.h>    // for sprintf() used by get_printable_hex()


//============[ syntax type ]=========================================
SmiUINT32 OctetStr::get_syntax()
{ return sNMP_SYNTAX_OCTETS; };

//============[ constructor using no arguments ]======================
OctetStr::OctetStr( void): 
  output_buffer(NULL), validity(TRUE)
{
  smival.syntax = sNMP_SYNTAX_OCTETS;
  smival.value.string.ptr = 0;
  smival.value.string.len = 0;
};

//============[ constructor using a  string ]=========================
OctetStr::OctetStr( const char  * string):
  output_buffer(NULL), validity(TRUE)
{
  smival.syntax = sNMP_SYNTAX_OCTETS;
  smival.value.string.ptr = 0;
  smival.value.string.len = 0;

  size_t z;

  // check for null string
  if ( !string || !(z = (size_t) STRLEN( string)))
  {
    return;
  }

  // get mem needed
  smival.value.string.ptr = (SmiLPBYTE) new  unsigned char [z];
  // if new was a success, then fil it up
  if ( smival.value.string.ptr != 0)
  {
    MEMCPY( smival.value.string.ptr, string, (size_t) z);	
    smival.value.string.len = z;
  }
  else
    validity = FALSE;
};


//============[ constructor using an unsigned char * ]================
OctetStr::OctetStr( const unsigned char  * string, unsigned long int size): 
  output_buffer(NULL), validity(TRUE)
{
  smival.syntax = sNMP_SYNTAX_OCTETS;
  smival.value.string.ptr = 0;
  smival.value.string.len = 0;

  // check for zero len
  if ( !string || !size)
  {
    return;
  }

  // get the mem needed
  smival.value.string.ptr = (SmiLPBYTE) new  unsigned char [ size];
  // if successful then fill it up
  if ( smival.value.string.ptr != 0)
  {
    MEMCPY( smival.value.string.ptr, string, (size_t) size);
    smival.value.string.len = size;
  }
  else
    validity = FALSE;
};

//============[ set the data on an already constructed Octet ]============
void OctetStr::set_data( const unsigned char  * string,
			             unsigned long int size)
{
  // free up already used space
  if ( smival.value.string.ptr )
  {
    delete [] smival.value.string.ptr;
    smival.value.string.ptr = NULL;
  }
  smival.value.string.len = 0;

  // check for zero len
  if ( !string || !size)
  {
    validity = TRUE;
    return;
  }

  // get the mem needed
  smival.value.string.ptr = (SmiLPBYTE) new  unsigned char [ size];
  // if successful then fill it up
  if ( smival.value.string.ptr != 0)
  {
    MEMCPY( smival.value.string.ptr, string, (size_t) size);  
    smival.value.string.len = size;
    validity = TRUE;
  }
  else
    validity = FALSE;
};

//============[ constructor using another octet object ]==============
OctetStr::OctetStr ( const OctetStr &octet): 
  output_buffer(NULL), validity(TRUE)
{
  smival.syntax = sNMP_SYNTAX_OCTETS;
  smival.value.string.ptr = 0;
  smival.value.string.len = 0;

   // check for zero len case
   if ( octet.smival.value.string.len == 0)
   {
      return;
   }

   // must be a valid object
   if ( !octet.validity)
   {
      validity = FALSE;
      return;
   }

   // get the mem needed
   smival.value.string.ptr = (SmiLPBYTE) new  unsigned char [ octet.smival.value.string.len];
   // if successful then fill it up
   if ( smival.value.string.ptr != 0)
   {
      MEMCPY( smival.value.string.ptr,		       
	          octet.smival.value.string.ptr,	   
	          (size_t) octet.smival.value.string.len);  
      smival.value.string.len = octet.smival.value.string.len;
   }
   else
     validity = FALSE;
};

//=============[ destructor ]=========================================
OctetStr::~OctetStr()
{
   // if not empty, free it up
   if ( smival.value.string.ptr )
      delete [] smival.value.string.ptr;
   if (output_buffer != NULL)
      delete [] output_buffer;
};


//=============[ assignment to a string operator overloaded ]=========
OctetStr& OctetStr::operator=( const char  *string)
{
   // get the string size
   size_t nz;

   // free up previous memory if needed
   if ( smival.value.string.ptr )
   {
     delete [] smival.value.string.ptr;
     smival.value.string.ptr = NULL;
     smival.value.string.len = 0;
   }
   
   // if empty then we are done
   if (!string || !(nz = (size_t) STRLEN( string)))
   {
     validity = TRUE;
     return *this;
   }

   // get memory needed
   smival.value.string.ptr = (SmiLPBYTE) new  unsigned char [ nz];
   // if not null, fill it up
   if ( smival.value.string.ptr != 0)
   {
      MEMCPY( smival.value.string.ptr,	      	// dest
	          string,	      		// source
	          (size_t) nz);   		// size
      smival.value.string.len = nz;
      validity = TRUE;
   }
   else
     validity = FALSE;

   return *this;	     // return self reference
};

//=============[ assignment to another oid object overloaded ]========
OctetStr& OctetStr::operator=( const OctetStr &octet)
{
   // protect against assignment from self
   if ( this == &octet )
       return *this;

   // don't assign from invalid objs
   if (!octet.validity)
   {
     return *this;
   }

   // free up previous memory if needed
   if ( smival.value.string.len )
   {
     delete [] smival.value.string.ptr;
     smival.value.string.ptr = NULL;
     smival.value.string.len = 0;
   }

   if (!octet.smival.value.string.len)
   {
     validity = TRUE;
     return *this;
   }

   // get some new memory
   smival.value.string.ptr = (SmiLPBYTE) new  unsigned char [ octet.smival.value.string.len];
   // if not null, fill it up
   if ( smival.value.string.ptr != 0)
   {
      MEMCPY( smival.value.string.ptr, 
	      octet.smival.value.string.ptr,	          
	      (size_t) octet.smival.value.string.len);      
      smival.value.string.len = octet.smival.value.string.len;
      validity = TRUE;
   }
   else
     validity = FALSE;

   return *this;		       // return self reference
};

//==============[ equivlence operator overloaded ]====================
int operator==( const OctetStr &lhs, const OctetStr &rhs)
{
   if( lhs.nCompare( rhs.smival.value.string.len, rhs)==0) return TRUE; else return FALSE;
};

//==============[ not equivlence operator overloaded ]================
int operator!=( const OctetStr &lhs, const OctetStr &rhs)
{
   if( lhs.nCompare( rhs.smival.value.string.len, rhs)!=0) return TRUE; else return FALSE;
};

//==============[ less than < overloaded ]============================
int operator<( const OctetStr &lhs, const OctetStr &rhs)
{
   if( lhs.nCompare( rhs.smival.value.string.len, rhs)<0) 
      return TRUE; 
   else 
      return FALSE;
};

//==============[ less than <= overloaded ]===========================
int operator<=( const OctetStr &lhs, const OctetStr &rhs)
{
   if(( lhs.nCompare( rhs.smival.value.string.len, rhs)<0) ||
      ( lhs.nCompare( rhs.smival.value.string.len, rhs)==0))
      return TRUE;
   else
      return FALSE;
};

//===============[ greater than > overloaded ]========================
int operator>( const OctetStr &lhs, const OctetStr &rhs)
{
  if( lhs.nCompare( rhs.smival.value.string.len, rhs)>0) 
      return TRUE; 
  else 
      return FALSE;
};

//===============[ greater than >= overloaded ]=======================
int operator>=( const OctetStr &lhs, const OctetStr &rhs)
{
  if(( lhs.nCompare( rhs.smival.value.string.len, rhs)>0) ||
     ( lhs.nCompare( rhs.smival.value.string.len, rhs)==0))
     return TRUE;
  else
     return FALSE;
};

//===============[ equivlence operator overloaded ]===================
int operator==( const OctetStr &lhs,const char  *rhs)
{
   OctetStr to( rhs);
   if( lhs.nCompare( to.smival.value.string.len,to)==0) 
       return TRUE; 
   else 
       return FALSE;
};

//===============[ not equivlence operator overloaded ]===============
int operator!=( const OctetStr &lhs,const char  *rhs)
{
   OctetStr to( rhs);
   if ( lhs.nCompare( to.smival.value.string.len,to)!=0) 
       return TRUE; 
   else 
       return FALSE;
};

//===============[ less than < operator overloaded ]==================
int operator<( const OctetStr &lhs,const char  *rhs)
{
   OctetStr to( rhs);
   if ( lhs.nCompare( to.smival.value.string.len,to)<0) 
       return TRUE; 
   else 
       return FALSE;
};

//===============[ less than <= operator overloaded ]=================
int operator<=( const OctetStr &lhs,char  *rhs)
{
   OctetStr to( rhs);
   if (( lhs.nCompare( to.smival.value.string.len,to)<0) ||
       ( lhs.nCompare( to.smival.value.string.len,to)==0))
      return TRUE;
   else
      return FALSE;
};

//===============[ greater than > operator overloaded ]===============
int operator>( const OctetStr &lhs,const char  *rhs)
{
   OctetStr to( rhs);
   if ( lhs.nCompare( to.smival.value.string.len,to)>0) 
       return TRUE; 
   else 
       return FALSE;
};

//===============[ greater than >= operator overloaded ]==============
int operator>=( const OctetStr &lhs,const char  *rhs)
{
   OctetStr to( rhs);
   if (( lhs.nCompare( to.smival.value.string.len,to)>0) ||
       ( lhs.nCompare( to.smival.value.string.len,to)==0))
      return TRUE;
   else
      return FALSE;
};

//===============[ append operator, appends a string ]================
OctetStr& OctetStr::operator+=( const char  *a)
{
  unsigned char  *tmp;	 // temp pointer
  size_t slen,nlen;

  // get len of string
  if ( !a || ((slen = (size_t) STRLEN( a)) == 0)) 
    return *this;

  // total len of new octet
  nlen =  slen + (size_t) smival.value.string.len;
  // get mem needed
  tmp =  (SmiLPBYTE) new  unsigned char [ nlen];
  // if not null, fill it up
  if ( tmp != 0)
  {
    // copy in the original 1st
    MEMCPY ( tmp, smival.value.string.ptr, (size_t) smival.value.string.len);
    // copy in the string
    MEMCPY( tmp + smival.value.string.len, a, (size_t) slen);
    // delete the original
    if ( smival.value.string.ptr )
      delete [] smival.value.string.ptr;
    // point to the new one
    smival.value.string.ptr = tmp;
    smival.value.string.len = nlen;
  }

  return *this;
};

//================[ append one OctetStr to another ]==================
OctetStr& OctetStr::operator+=( const OctetStr& octetstr)
{
  unsigned char  *tmp;	 // temp pointer
  size_t slen,nlen;

  if (!octetstr.validity || 
      !(slen = (size_t)octetstr.len()))
    return *this;

  // total len of new octet
  nlen =  slen + (size_t) smival.value.string.len;
  // get mem needed
  tmp =  (SmiLPBYTE) new  unsigned char [ nlen];
  // if not null, fill it up
  if ( tmp != 0)
  {
    // copy in the original 1st
    MEMCPY ( tmp, smival.value.string.ptr, (size_t) smival.value.string.len);
    // copy in the string
    MEMCPY( tmp + smival.value.string.len, octetstr.data(), (size_t) slen);
    // delete the original
    if ( smival.value.string.ptr )
      delete [] smival.value.string.ptr;
    // point to the new one
    smival.value.string.ptr = tmp;
    smival.value.string.len = nlen;
  }

   return *this;
};

//================[ appends an int ]==================================
OctetStr& OctetStr::operator+=( const unsigned char c)
{
    unsigned char  *tmp;
    // get the memory needed plus one extra byte
    tmp =  (SmiLPBYTE) new  unsigned char [ smival.value.string.len + 1];
    // if not null, fill it up
    if ( tmp != 0)
    {
       MEMCPY ( tmp, 				   // dest
		smival.value.string.ptr,	   // source
		(size_t) smival.value.string.len); // len of original
       tmp[ smival.value.string.len ] = c; 	// assign in new byte

       if ( smival.value.string.ptr )	// delete the original
           delete [] smival.value.string.ptr;  

       smival.value.string.ptr = tmp;	// point to new one
       smival.value.string.len++;	   	// up the len
    }

    return *this;		   		        // return self reference
};


//================[ compare n elements of an Octet ]==================
int OctetStr::nCompare( const unsigned long n,
		        const OctetStr &o) const
{

   unsigned long z,w;	

   // both are empty, they are equal
   if (( smival.value.string.len == 0) &&
	   ( o.smival.value.string.len == 0))
	   return 0;  // equal 
   
   // self is empty and param has something
   if (( smival.value.string.len == 0) &&
	   ( o.smival.value.string.len >0) &&
	   (n>0))
	   return -1; 

   // self has something and param has nothing
   if (( smival.value.string.len > 0) &&
	   ( o.smival.value.string.len ==0) &&
	   (n>0)) 
	   return 1; 

   // special case
   if (( smival.value.string.len == 0) &&
	   ( o.smival.value.string.len > 0) &&
	   ( n == 0))
	   return 0;  
   
   // pick the Min of n, this and the param len
   // this is the maximum # to iterate a search
   w = smival.value.string.len < o.smival.value.string.len
	   ? smival.value.string.len : o.smival.value.string.len;
   if (n<w) w=n;

   z=0;
   while( z<w)
   {
	  if ( smival.value.string.ptr[z] < o.smival.value.string.ptr[z])
	    return -1;				// less than
      if ( smival.value.string.ptr[z] > o.smival.value.string.ptr[z])
	    return 1;				// greater than
      z++;
   }

   if (( z == 0) &&
	   ( smival.value.string.len == 0) &&
	   ( o.smival.value.string.len > 0))
	   return -1;

   if (( z == 0) &&
	   ( o.smival.value.string.len == 0) &&
	   ( smival.value.string.len > 0))
	   return 1;

   return 0;
};

//================[ return the len of the oid ]=======================
unsigned long OctetStr::len() const
{
   return smival.value.string.len;
};

//================[ operator[]: access as if array ]==================
unsigned char& OctetStr::operator[]( int position)
{ 
    return  smival.value.string.ptr[position];
};

//===============[ reuturns pointer to internal data ]===============
unsigned char  * OctetStr::data() const
{
   return smival.value.string.ptr;
};

//================[ returns validity ]================================
int OctetStr::valid() const
{
   return validity;
};

//================[ clone() ]=========================================
SnmpSyntax * OctetStr::clone() const 
{ 
    return ( SnmpSyntax *) new OctetStr(*this); 
};

//================[ ASCII format return ]=============================
char  * OctetStr::get_printable()
{
  for ( unsigned long i=0; i < smival.value.string.len; i++){
    if (( smival.value.string.ptr[i] != '\r')&& 
		( smival.value.string.ptr[i] != '\n')&&
		(isprint((int) (smival.value.string.ptr[i]))==0))
         return(get_printable_hex());
  }

  if ( output_buffer != NULL)
	  delete [] output_buffer;

  output_buffer = new char[smival.value.string.len + 1];	 

  if (smival.value.string.len)
    MEMCPY(output_buffer, smival.value.string.ptr, (unsigned int) smival.value.string.len);
  output_buffer[smival.value.string.len] = '\0';
  return(output_buffer);
}


//================[ general Value = operator ]========================
SnmpSyntax& OctetStr::operator=( SnmpSyntax &val)
{
   // protect against assignment from self
   if ( this == &val )
       return *this;

  // blow away the old value
  if (smival.value.string.ptr) {
    delete [] smival.value.string.ptr;
    smival.value.string.ptr = NULL;
  }
  smival.value.string.len = 0;
  validity=FALSE;

  if (val.valid()){
    switch (val.get_syntax()){
      case sNMP_SYNTAX_OCTETS:
      case sNMP_SYNTAX_IPADDR:
	set_data( ((OctetStr &)val).smival.value.string.ptr, 
		  ((OctetStr &)val).smival.value.string.len);
	break;
    }
  }
  return *this;
};

//================[ format the output into hex ]========================
char *OctetStr::get_printable_hex()
{
  int cnt;
  char char_buf[80];              // holds ASCII representation of data 
  char *buf_ptr;                  // pointer into ASCII listing	    
  char *line_ptr;                 // pointer into Hex listing 
  int  storageNeeded;             // how much space do we need ?
  int  local_len = (int) smival.value.string.len;
  unsigned char *bytes = smival.value.string.ptr;

  
  storageNeeded = (int) ((smival.value.string.len/16)+1) * 72 + 1;

  if ( output_buffer != NULL)
	  delete [] output_buffer;

  output_buffer = new char[storageNeeded];
  
  line_ptr = output_buffer;

    /*----------------------------------------*/
    /* processing loop for entire data buffer */
    /*----------------------------------------*/
  while (local_len > 0) {
    cnt	     = 16;	  /* print 16 bytes per line */
    buf_ptr  = char_buf;
    sprintf(line_ptr, "  ");
    line_ptr += 2;  /* indent */

      /*-----------------------*/
      /* process a single line */
      /*-----------------------*/
    while (cnt-- > 0 && local_len-- > 0) {
      sprintf(line_ptr, "%2.2X ", *bytes);

      line_ptr +=3;   /* the display of a byte always 3 chars long */
      if (isprint(*bytes))
	sprintf(buf_ptr, "%c", *bytes);
      else
	sprintf(buf_ptr, ".");
      bytes++;
      buf_ptr++;
    }
    cnt++;

       /*----------------------------------------------------------*/
       /* this is to make sure that the ASCII displays line up for */
       /* incomplete lines of hex				   */
       /*----------------------------------------------------------*/
    while (cnt-- > 0){
      sprintf(line_ptr,"   ");
      line_ptr += 3;
    }

      /*------------------------------------------*/
      /* append the ASCII display to the Hex line */
      /*------------------------------------------*/
#ifndef __unix
    sprintf(line_ptr,"   %s\n", char_buf);
#else
    sprintf(line_ptr,"   %s\r\n", char_buf);
#endif // __unix
    line_ptr += 3 + strlen(char_buf);
  }

  return(output_buffer);
}
