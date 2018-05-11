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
  purpose. It is provided "AS-IS" without warranty of any kind,either express 
  or implied. User hereby grants a royalty-free license to any and all 
  derivatives based upon this software code base. 



  O I D. C P P
  
  OID CLASS IMPLEMENTATION

  DESIGN:
  Peter E. Mellquist

  AUTHOR:
  Peter E Mellquist

  DATE:
  June 15, 1994

  DESCRIPTION:
  This module contains the implementation of the oid class. This
  includes all protected and public member functions. The oid class
  may be compiled stand alone without the use of any other library.

  LANGUAGE:
  ANSI C++

  OPERATING SYSTEM(S):
  MS-Windows Win32
  BSD UNIX


=====================================================================*/
char oid_cpp_version[]="#(@)SNMP++ 2.6 $Header: oid.cpp,v 1.33 96/09/11 14:02:00 hmgr Exp $";

//---------[ external C libaries used ]--------------------------------
extern "C"
{
#include <stdio.h>                // standard io
#include <memory.h>               // memcpy's
#include <string.h>               // strlen, etc..
#include <stdlib.h>               // standard library
#include <ctype.h>                // isdigit
#include <malloc.h>               // malloc, free
}

#include "oid.h"                  // include def for oid class
#define  SNMPBUFFSIZE 10          // size of scratch buffer
#define  SNMPCHARSIZE 10          // an individual oid instance as a string


//=============[Oid::get_syntax(void)]====================================
SmiUINT32 Oid::get_syntax()
{ return sNMP_SYNTAX_OID; };


//=============[Oid::Oid(void)]============================================
// constructor using no arguments
// initialize octet ptr and string
// ptr to null
Oid::Oid( void)
{
  smival.syntax = sNMP_SYNTAX_OID;
  smival.value.oid.len = 0;
  smival.value.oid.ptr = NULL;
  iv_str=0;
};


//=============[Oid::Oid( const char *dotted_string ]=====================
// constructor using a dotted string
//
// do a string to oid using the string passed in
Oid::Oid( const char * dotted_oid_string)
{
  smival.syntax = sNMP_SYNTAX_OID;
  smival.value.oid.len = 0;
  smival.value.oid.ptr = NULL;
  StrToOid( (char *) dotted_oid_string, &smival.value.oid);
  iv_str = 0;
};


//=============[Oid::Oid( const Oid &oid) ]================================
// constructor using another oid object
//
// do an oid copy using the oid object passed in
Oid::Oid ( const Oid &oid)
{
  smival.syntax = sNMP_SYNTAX_OID;
  smival.value.oid.len = 0;
  smival.value.oid.ptr = NULL;
  iv_str = 0;
  
  // allocate some memory for the oid
  // in this case the size to allocate is the same
  // size as the source oid
  if (oid.smival.value.oid.len) {
    smival.value.oid.ptr = (SmiLPUINT32) new unsigned long [ oid.smival.value.oid.len];
    if ( smival.value.oid.ptr !=0)
      OidCopy( (SmiLPOID) &(oid.smival.value.oid),(SmiLPOID) &smival.value.oid);
  }
};


//=============[Oid::Oid( const unsigned long *raw_oid, int oid_len) ]====
// constructor using raw numeric form
//
// copy the integer values into the private member
Oid::Oid(const unsigned long *raw_oid, int oid_len)
{
  smival.syntax = sNMP_SYNTAX_OID;
  smival.value.oid.len = 0;
  smival.value.oid.ptr = NULL;
  iv_str = 0;
    
  if (raw_oid && oid_len > 0) {
    smival.value.oid.ptr = (SmiLPUINT32) new unsigned long [ oid_len];
    if ( smival.value.oid.ptr) {
      smival.value.oid.len = oid_len;
      for (int i=0; i < oid_len; i++)
	smival.value.oid.ptr[i] = raw_oid[i];
    }
  }
};

//=============[Oid::~Oid]==============================================
// destructor
//
// free up the descriptor space
Oid::~Oid()
{
   // free up the octet deep memory
   if ( smival.value.oid.ptr ) {
     delete [] smival.value.oid.ptr;
     smival.value.oid.ptr = NULL;
   }

   // free up the output string
   if ( iv_str !=0)
     delete [] iv_str;
};


//=============[Oid::operator = const char * dotted_string ]==============
// assignment to a string operator overloaded
//
// free the existing oid
// create the new oid from the string
// return this object
Oid& Oid::operator=( const char *dotted_oid_string)
{
  // delete the old value
  if ( smival.value.oid.ptr ) {
    delete [] smival.value.oid.ptr;
    smival.value.oid.ptr = NULL;
  }
  smival.value.oid.len = 0;

  // assign the new value
  StrToOid( (char *) dotted_oid_string, &smival.value.oid);
  return *this;
};


//=============[Oid:: operator = const Oid &oid ]==========================
// assignment to another oid object overloaded
//
// free the existing oid
// create a new one from the object passed in
Oid& Oid::operator=( const Oid &oid)
{
  // protect against assignment from self
  if ( this == &oid )
      return *this;

  // delete the old value
  if ( smival.value.oid.ptr ) {
    delete [] smival.value.oid.ptr;
    smival.value.oid.ptr = NULL;
  }
  smival.value.oid.len = 0;

  // check for zero len on source
  if ( oid.smival.value.oid.len == 0) {
     smival.value.oid.len = 0;
     smival.value.oid.ptr = NULL;
     return *this;
  }

  // allocate some memory for the oid
  smival.value.oid.ptr = (SmiLPUINT32) new unsigned long [ oid.smival.value.oid.len];
  if ( smival.value.oid.ptr !=0)
    OidCopy( (SmiLPOID) &(oid.smival.value.oid),(SmiLPOID) &smival.value.oid);
  return *this;
};


//==============[Oid:: operator += const char *a ]=========================
// append operator, appends a string
//
// allocate some space for a max oid string
// extract current string into space
// concat new string
// free up existing oid
// make a new oid from string
// delete allocated space
Oid& Oid::operator+=( const char *a)
{
   unsigned long n;

   if (!a)
     return *this;

   if ( *a=='.')
	  a++;

   n =  (smival.value.oid.len *SNMPCHARSIZE) + ( smival.value.oid.len) + 1 + STRLEN(a);
   char *ptr = (char *) new char[ n];
   if ( ptr !=0)
   {
     OidToStr(&smival.value.oid, n,ptr);
     if (STRLEN(ptr))
       STRCAT(ptr,".");
     STRCAT(ptr,a);
     if ( smival.value.oid.len !=0) {
       delete [] smival.value.oid.ptr;
       smival.value.oid.len = 0;
     }
     StrToOid( (char *) ptr, &smival.value.oid);
     delete [] ptr;
   }
   return *this;
};

//=============[ int operator == oid,oid ]=================================
// equivlence operator overloaded
int operator==( const Oid &lhs, const Oid &rhs)
{   
   // ensure same len, then use nCompare
   if (rhs.len() != lhs.len()) return 0;
   if( lhs.nCompare( rhs.len(), rhs)==0) return 1; else return 0;
};      
 
//==============[ operator!=( Oid &x,Oid &y) ]============================= 
//not equivlence operator overloaded
int operator!=( const Oid &lhs,const Oid &rhs)
{   
   // just invert ==
   return (!(lhs==rhs));
};  

//==============[ operator<( Oid &x,Oid &y) ]=============================  
// less than < overloaded
int operator<( const Oid &lhs,const Oid &rhs)
{   
  int result;

   // call nCompare with the current
   // Oidx, Oidy and len of Oidx
   if((result = lhs.nCompare( rhs.len(), rhs))<0) return 1; 
   else if (result > 0) return 0;

   else{
     // if here, equivalent substrings, call the shorter one <
     if (lhs.len() < rhs.len()) return 1;
     else return 0;
   }
}; 

//==============[ operator<=( Oid &x,Oid &y) ]=============================  
// less than <= overloaded
int operator<=( const Oid &x,const Oid &y)
{   
   if((x<y) || (x==y)) return 1;
   else return 0;
};    

//==============[ operator>( Oid &x,Oid &y) ]=============================     
// greater than > overloaded
int operator>( const Oid &x,const Oid &y)
{   
  // just invert existing <=
  if (!(x<=y)) return 1;
  else return 0;
}; 

//==============[ operator>=( Oid &x,Oid &y) ]=============================  
// greater than >= overloaded
int operator>=( const Oid &x,const Oid &y)
{   
  // just invert existing <
  if(!(x<y)) return 1;
  else return 0;
};          
  
//==============[ operator==( Oid &x,char *) ]=============================         
// equivlence operator overloaded
int operator==( const Oid &x,const char *dotted_oid_string)
{    
   // create a temp oid object
   Oid to( dotted_oid_string);
   // compare using existing operator
   if(x == to) return 1; else return 0;
};         

//==============[ operator!=( Oid &x,char*) ]=============================    
// not equivlence operator overloaded
int operator!=( const Oid &x,const char *dotted_oid_string)
{    
   // create a temp oid object
   Oid to( dotted_oid_string);
   // compare using existing operator
   if(x != to) return 1; else return 0;
};             

//==============[ operator<( Oid &x,char*) ]=============================     
// less than < operator overloaded
int operator<( const Oid &x,const char *dotted_oid_string)
{    
   // create a temp oid object
   Oid to( dotted_oid_string);
   // compare using existing operator
   if(x < to) return 1; else return 0;
}; 

//==============[ operator<=( Oid &x,char *) ]=============================  
// less than <= operator overloaded
int operator<=( const Oid &x,char *dotted_oid_string)
{    
   // create a temp oid object
   Oid to( dotted_oid_string);
   // compare using existing operator
   if(x <= to) return 1; else return 0;
};             

//==============[ operator>( Oid &x,char* ]=============================  
// greater than > operator overloaded
int operator>( const Oid &x,const char *dotted_oid_string)
{    
   // create a temp oid object
   Oid to( dotted_oid_string);
   // compare using existing operator
   if(x > to) return 1; else return 0;
}; 

//==============[ operator>=( Oid &x,char*) ]=============================  
// greater than >= operator overloaded
int operator>=( const Oid &x,const char *dotted_oid_string)
{    
   // create a temp oid object
   Oid to( dotted_oid_string);
   // compare using existing operator
   if(x >= to) return 1; else return 0;
};            

//===============[Oid::oidval ]=============================================
// return the WinSnmp oid part
SmiLPOID Oid::oidval()
{
  return (SmiLPOID) &smival.value.oid;
};

//===============[Oid::set_data ]==---=====================================
// copy data from raw form...
void Oid::set_data( const unsigned long *raw_oid,
                    const unsigned int oid_len)
{
  if (smival.value.oid.len < oid_len){
    if ( smival.value.oid.ptr) {
      delete [] smival.value.oid.ptr;
      smival.value.oid.ptr = NULL;
      smival.value.oid.len = 0;
    }
    smival.value.oid.ptr = (SmiLPUINT32) new unsigned long [ oid_len];
    if ( smival.value.oid.ptr ==0){
      return;
    }
  }
  MEMCPY((SmiLPBYTE) smival.value.oid.ptr,
         (SmiLPBYTE) raw_oid,
         (size_t) (oid_len*sizeof(SmiUINT32)));
  smival.value.oid.len = oid_len;
};


//===============[Oid::len ]================================================
// return the len of the oid
unsigned long Oid::len() const
{
   return smival.value.oid.len;
};

//===============[Oid::trim( unsigned int) ]============================
// trim off the n leftmost values of an oid
// Note!, does not adjust actual space for
// speed
void Oid::trim( const unsigned long n)
{
  // verify that n is legal
  if ((n<=smival.value.oid.len)&&(n>0)) {
    smival.value.oid.len -= n;
    if (smival.value.oid.len == 0) {
      delete [] smival.value.oid.ptr;
      smival.value.oid.ptr = NULL;
    }
  }
};

//===============[Oid::operator += const unsigned int) ]====================
// append operator, appends an int
//
// allocate some space for a max oid string
// extract current string into space
// concat new string
// free up existing oid
// make a new oid from string
// delete allocated space
Oid& Oid::operator+=( const unsigned long i)
{
   unsigned long n;
   n = (smival.value.oid.len *SNMPCHARSIZE) + ( smival.value.oid.len -1) + 1 + 4;  // extra for
   char buffer[SNMPBUFFSIZE];
   // allocate some temporary space
   char *ptr = (char *) new char[ n];
   if ( ptr != 0)
   {
     OidToStr(&smival.value.oid,n,ptr);
     if (STRLEN(ptr))
       STRCAT(ptr,".");
     sprintf( buffer,"%ld",i);
     STRCAT(ptr,buffer);
     if ( smival.value.oid.ptr ) {
       delete [] smival.value.oid.ptr;
       smival.value.oid.ptr = NULL;
       smival.value.oid.len = 0;
     }
     StrToOid( (char *) ptr, &smival.value.oid);
     delete [] ptr;
   }
   return *this;

}

//===============[Oid::operator += const Oid) ]========================
// append operator, appends an Oid
//
// allocate some space for a max oid string
// extract current string into space
// concat new string
// free up existing oid
// make a new oid from string
// delete allocated space
Oid& Oid::operator+=( const Oid &o)
{
  SmiLPUINT32 new_oid;

  if (o.smival.value.oid.len == 0)
    return *this;

  new_oid = (SmiLPUINT32) new unsigned long [ smival.value.oid.len + o.smival.value.oid.len];
  if ( new_oid == 0 ){
    if ( smival.value.oid.ptr ) {
        delete [] smival.value.oid.ptr;
        smival.value.oid.ptr = NULL;
        smival.value.oid.len = 0;
    }
    return *this;
  }
  
  if (smival.value.oid.ptr){
    MEMCPY((SmiLPBYTE) new_oid,
	   (SmiLPBYTE) smival.value.oid.ptr,
	   (size_t) (smival.value.oid.len*sizeof(SmiUINT32)));

    delete [] smival.value.oid.ptr;
  }
    // out with the old, in with the new...
  smival.value.oid.ptr = new_oid;

  MEMCPY((SmiLPBYTE) &new_oid[smival.value.oid.len],
         (SmiLPBYTE) o.smival.value.oid.ptr,
         (size_t) (o.smival.value.oid.len*sizeof(SmiUINT32)));

  smival.value.oid.len += o.smival.value.oid.len;

  return *this;
}


//=============[Oid::get_printable ]====================================
// return string value
//
// return string portion of the oid
//
char * Oid::get_printable()
{
   unsigned long n;
   // the worst case char len of an oid can be..
   // oid.len*3 + dots in between if each oid is XXXX
   // so.. size = (len*4) + (len-1) + 1 , extra for a null

   n = (smival.value.oid.len *SNMPCHARSIZE) + ( smival.value.oid.len -1) + 1 ;
   if (n==0) n=1; // need at least 1 byte for a null string

   // adjust the len of output array in case size was adjusted
   if ( iv_str!=0)
     delete [] iv_str;
   // allocate some space for the output string
   iv_str = (char *) new char[ n];
   // convert to an output string
   if ( iv_str !=0)
     OidToStr(&smival.value.oid,n,iv_str);
   return iv_str;
};


//==========[Oid::get_printable( unsigned int n) ]=========================
// overloaded get_printable to get the n left most values
// as a dotted string
char * Oid::get_printable( const unsigned long n)
{
  unsigned long index = 0;
  unsigned long start;
  unsigned totLen = 0;
  char szNumber[SNMPBUFFSIZE];
  unsigned long nz;

  nz = (smival.value.oid.len *SNMPCHARSIZE) + ( smival.value.oid.len -1) + 1 ;
  if (nz==0) nz=1; // need at least one byte for a null string

  // delete the previous output string
  if ( iv_str !=0)
    delete [] iv_str;

    // allocate some space for the output string
  iv_str = (char *) new char[ nz];
  if ( iv_str == 0)
    return iv_str;

    // init the string
  iv_str[totLen] = 0;

  // cannot ask for more then there is..
  if (n>smival.value.oid.len)
    return iv_str;

  start = smival.value.oid.len - n;

  // loop through and build up a string
  for (index=start; index<smival.value.oid.len; index++)
  {

    // convert data element to a string
    sprintf( szNumber,"%ld",smival.value.oid.ptr[index]);

    // verify len is not over
    if (totLen+STRLEN(szNumber)+1>=nz)
      return iv_str;

    // if not at end, pad with a dot
    if (totLen!=0)
      iv_str[totLen++] = '.';

    // copy the string token into the main string
    STRCPY(iv_str+totLen, szNumber);

    // adjust the total len
    totLen += STRLEN(szNumber);
  }
  return iv_str;

};


//==============[Oid::get_printable( unsigned int start, n) ]=============
// return a dotted string starting at start,
// going n positions to the left
// NOTE, start is 1 based ( the first id is at position #1)
char * Oid::get_printable( const unsigned long start,
                           const unsigned long n)
{
  unsigned long index = 0;
  unsigned long totLen = 0;
  char szNumber[SNMPBUFFSIZE];
  unsigned long nz;
  unsigned long my_start;

  my_start = start;

  nz = (smival.value.oid.len *SNMPCHARSIZE) + ( smival.value.oid.len -1) + 1 ;
  if (nz==0) nz=1; // need at least one byte for a null string

  // clean up the previous output string
  if ( iv_str !=0)
    delete [] iv_str;

    // allocate some space for the output string
  iv_str = (char *) new char[ nz];
  if ( iv_str == 0)
    return iv_str;

    // init the string
  iv_str[totLen] = 0;

  my_start -=1;

        // cannot ask for more then there is..
  if ((my_start+n-1)>smival.value.oid.len)
    return iv_str;


        // loop through and build up a string
  for (index=my_start; index<(my_start+n); index++)
  {

    // convert data element to a string
    sprintf( szNumber,"%ld",smival.value.oid.ptr[index]);

    // verify len is not over
    if (totLen+STRLEN(szNumber)+1>=nz)
      return iv_str;

    // if not at end, pad with a dot
    if (totLen!=0)
      iv_str[totLen++] = '.';

    // copy the string token into the main string
    STRCPY(iv_str+totLen, szNumber);

    // adjust the total len
    totLen += STRLEN(szNumber);
  }
  return iv_str;

};


//=============[Oid::StrToOid( char *string, SmiLPOID dst) ]==============
// convert a string to an oid
int Oid::StrToOid( const char *string,
                   SmiLPOID dstOid)
{
  unsigned long index = 0;
  unsigned long number = 0;

  // make a temp buffer to copy the data into first
  SmiLPUINT32 temp;
  unsigned long nz;

  if (string && *string) 
  {
     nz = STRLEN( string);
  }
  else {
     dstOid->len = 0;
     dstOid->ptr = NULL;
     return -1;
  }
  temp = (SmiLPUINT32) new unsigned long [ nz];
  // return if can't get the mem
  if ( temp == 0) return -1;

  while (*string!=0 && index<nz)
  {
    // init the number for each token
    number = 0;
    // skip over the dot
    if (*string=='.') string++;

            // grab a digit token and convert it to a long int
    while (isdigit(*string))
      number=number*10 + *(string++)-'0';

                // check for invalid chars
    if (*string!=0 && *string!='.')
      // Error: Invalid character in string
    {
      delete [] temp;
      return -1;
    }

    // stuff the value into the array
    temp[index] = number;
    index++;  // bump the counter
  }


  // get some space for the real oid
  dstOid->ptr = (SmiLPUINT32) new unsigned long [ index];
  // return if can't get the mem needed
  if( dstOid->ptr == 0)
  {
    delete [] temp;
    return -1;
  }

  // copy in the temp data
  MEMCPY((SmiLPBYTE) dstOid->ptr,
         (SmiLPBYTE) temp,
         (size_t) (index*sizeof(SmiUINT32)));

  // set the len of the oid
  dstOid->len = index;

  // free up temp data
  delete [] temp;

  return (int) index;
};


//===============[Oid::OidCopy( source, destination) ]====================
// Copy an oid
int Oid::OidCopy( SmiLPOID srcOid,     // source oid
                  SmiLPOID dstOid)     // destination oid
{
  // check source len ! zero
  if (srcOid->len==0)
    return -1;

  // copy source to destination
  MEMCPY((SmiLPBYTE) dstOid->ptr,
         (SmiLPBYTE) srcOid->ptr,
         (size_t) (srcOid->len*sizeof(SmiUINT32)));

  //set the new len
  dstOid->len = srcOid->len;
  return (int) srcOid->len;
};


//===============[Oid::nCompare( n, Oid) ]=================================
// compare the n leftmost values of two oids ( left-to_right )
// 
// self == Oid then return 0, they are equal
// self < Oid then return -1, <
// self > Oid then return 1,  >
int Oid::nCompare( const unsigned long n, 
                   const Oid &o) const
{             
   unsigned long z;
   unsigned long len = n;
   int reduced_len = 0;
   
   // 1st case they both are null
   if (( len==0)&&( this->smival.value.oid.len==0)) return 0;  // equal
   
   // verify that n is valid, must be >= 0
   if ( len <=0) return 1;                         // ! equal

   // only compare for the minimal length
   if (len > this->smival.value.oid.len){
     len = this->smival.value.oid.len;
     reduced_len = 1;
   }
   if (len > o.smival.value.oid.len){
     len = o.smival.value.oid.len;
     reduced_len = 1;
   }
   
   z=0;
   while(z<len)
   {
      if ( this->smival.value.oid.ptr[z] < o.smival.value.oid.ptr[z])
        return -1;                              // less than
      if ( this->smival.value.oid.ptr[z] > o.smival.value.oid.ptr[z])
        return 1;                               // greater than
      z++;    
   }      
   
     // if we truncated the len then these may not be equal
   if (reduced_len){
     if (this->smival.value.oid.len < o.smival.value.oid.len)
       return -1;
     if (this->smival.value.oid.len > o.smival.value.oid.len)
       return 1;
   }
   return 0;                                 // equal
      
};


//===============[Oid::nCompare( n, Oid) ]=================================
// compare the n rightmost bytes (right-to-left)
// returns 0, equal
// returns -1, <
// returns 1 , >
int Oid::RnCompare( const unsigned long n, const Oid &o) const
{
   // oid to compare must have at least the same number
   // of sub-ids to comparison else the argument Oid is 
   // less than THIS
   if ( o.len() < n)
      return -1;

   // also can't compare argument oid for sub-ids which
   // THIS does not have
   if ( this->len() < n)
      return -1;

   int start = (int) this->len();
   int end = (int) start - (int) n;
   for ( int z=start;z< end;z--)
   {
       if ( o.smival.value.oid.ptr[z] < this->smival.value.oid.ptr[z])
          return -1;
       if ( o.smival.value.oid.ptr[z] > this->smival.value.oid.ptr[z])
          return 1;
   }

   return 0;   // they are equal

}; 


//================[ Oid::valid() ]========================================
// is the Oid object valid
// returns validity
int Oid::valid() const
{
   return ( smival.value.oid.ptr ? TRUE : FALSE ) ;
};

//================[Oid::OidToStr ]=========================================
// convert an oid to a string
int Oid::OidToStr( SmiLPOID srcOid,            // source oid
                   unsigned long size,         // size of string
                   char *string)               // pointer to string
{
  unsigned long index = 0;
  unsigned totLen = 0;
  char szNumber[SNMPBUFFSIZE];

  // init the string
  string[totLen] = 0;

  // verify there is something to copy
  if (srcOid->len==0)
    return -1;

  // loop through and build up a string
  for (index=0; index<srcOid->len; index++)
  {

    // convert data element to a string
    sprintf( szNumber,"%ld", srcOid->ptr[index]);

    // verify len is not over
    if (totLen+STRLEN(szNumber)+1>=size)
      return -2;

    // if not at end, pad with a dot
    if (totLen!=0)
      string[totLen++] = '.';

    // copy the string token into the main string
    STRCPY(string+totLen, szNumber);

    // adjust the total len
    totLen += STRLEN(szNumber);
  }
  return totLen+1;
};


//================[ general Value = operator ]========================
SnmpSyntax& Oid::operator=( SnmpSyntax &val)
{
  // protect against assignment from self
  if ( this == &val )
      return *this;

  // blow away old value
  smival.value.oid.len = 0;
  if (smival.value.oid.ptr) {
      delete [] smival.value.oid.ptr;
      smival.value.oid.ptr = NULL;
  }

  // assign new value
  if (val.valid()){
    switch (val.get_syntax()){
      case sNMP_SYNTAX_OID:
        set_data( ((Oid &)val).smival.value.oid.ptr, 
		  (unsigned int)((Oid &)val).smival.value.oid.len);
        break;
    }
  }
  return *this;
};

//================[ [] operator ]=====================================
unsigned long& Oid::operator[](int position)
{
  return smival.value.oid.ptr[position];
}

//================[ clone ]===========================================
SnmpSyntax *Oid::clone() const
{ 
    return (SnmpSyntax *) new Oid(*this); 
};

