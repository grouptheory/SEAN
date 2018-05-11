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



  T A R G E T . C P P   
      
  VERSION:
  2.6
       
  DESIGN:
  Peter E Mellquist
                
  AUTHOR:      
  Peter E Mellquist
              
  LANGUAGE:
  ANSI C++ 
      
  OPERATING SYSTEMS:
  MS-WINDOWS WIN32
  BSD UNIX
      
  DESCRIPTION:
  Target class defines target SNMP agents. 
      
=====================================================================*/
char target_cpp_version[]="#(@) SNMP++ 2.6 $Header: target.cpp,v 1.18 96/09/11 14:02:10 hmgr Exp $"; 
#include "target.h"
                                         
#define PUBLIC "public"

inline int MIN(int a, int b) { return a<b?a:b;} 

// class variables for default behavior control                             
unsigned long default_timeout=100;
int default_retries=1; 

//----------------------------------------------------------------------
//--------[ Abstract SnmpTarget Member Functions ]----------------------
//----------------------------------------------------------------------

// allow destruction of derived classes
SnmpTarget::~SnmpTarget() 
{};

// change the default timeout   
void SnmpTarget::set_default_timeout( const unsigned long t)
{  default_timeout = t; };
                                 
// change the default retries                             
void SnmpTarget::set_default_retries( const int r)
{ default_retries = r; };

// return validity of target
int SnmpTarget::valid() const
{ return validity;}; 

// set the retry value       
void SnmpTarget::set_retry( const int r)
{ retries = r; };   
       
// get the retry value
int SnmpTarget::get_retry()
{ return retries; };
       
// set the timeout   
void SnmpTarget::set_timeout( const unsigned long t)
{ timeout = t; };
       
// get the timeout
unsigned long SnmpTarget::get_timeout()
{ return timeout; };  


//---------------------------------------------------------------------- 
//--------[ CTarget Member Functions ]----------------------------------
//----------------------------------------------------------------------

//---------[ CTarget::CTarget( void) ]---------------------------------- 
// CTarget constructor no args
CTarget::CTarget( void)
{
  read_community = PUBLIC;
  write_community = PUBLIC;
  timeout = default_timeout;   
  retries = default_retries;
  version = version1;     
  validity = FALSE;
};

//-----------[ CTarget:: CTarget ]-------------------------------------
// CTarget constructor with args
CTarget:: CTarget(  const Address &address,
                    const char *read_community_name,
                    const char *write_community_name)
{
   GenAddress tmpaddr(address);
   read_community = read_community_name;
   write_community = write_community_name;
   timeout = default_timeout;   
   retries = default_retries;     
   version = version1;
   
   // initialize local address object
   my_address = tmpaddr;
   if ( my_address.valid())
      validity = TRUE;
   else
      validity = FALSE;      
};

//-----------[ CTarget:: CTarget ]-----------------------------------
// CTarget constructor with args
CTarget:: CTarget(  const Address &address,
                    const OctetStr& read_community_name,
                    const OctetStr& write_community_name)
{
   GenAddress tmpaddr(address);
   read_community = read_community_name;
   write_community = write_community_name;
   timeout = default_timeout;   
   retries = default_retries;     
   version = version1;
   
   // initialize local address object
   my_address = tmpaddr;
   if ( my_address.valid())
      validity = TRUE;
   else
      validity = FALSE;      
};


//-----------[ CTarget:: CTarget( Address &address) ]--------------
// CTarget constructor with args
CTarget:: CTarget( const Address &address)
{   
   GenAddress tmpaddr(address);
   read_community =  PUBLIC;
   write_community = PUBLIC;
   // initialize local address object
   
   timeout = default_timeout;   
   retries = default_retries;  
   my_address = tmpaddr;
   version = version1;
      
   if ( my_address.valid())
      validity = TRUE;
   else
      validity = FALSE;      
};

//-----------[ CTarget:: CTarget( const CTarget &target) ]-------
// CTarget constructor with args
CTarget:: CTarget( const CTarget &target)
{   
   GenAddress tmpaddr(target.my_address);
   read_community = target.read_community;
   write_community = target.write_community;
   my_address = tmpaddr;
   timeout = target.timeout;   
   retries = target.retries;  
   version = version1;
   validity = target.validity;
};
  
//-----------[ CTarget::~CTarget() ]--------------------------------
CTarget::~CTarget() 
{};

//-----------[ CTarget::clone() ]--------------------------------
SnmpTarget * CTarget::clone() const
{ return (SnmpTarget *) new CTarget(*this); };


//-----------[ CTarget::get_version() ]--------------------------------
snmp_version CTarget::get_version()
{ return version;};

//-----------[ CTarget::set_version() ]--------------------------------
void CTarget::set_version( const snmp_version v)
{ version = v; };

//----------[ CTarget::get_readcommunity() ]----------------------  
// get the read community name
char * CTarget::get_readcommunity()
{
   return (char *) read_community.get_printable();
};   
   
//----------[ CTarget::get_readcommunity( OctetStr& read_community)]--   
// get the read community name as an unsigned char and len
void CTarget::get_readcommunity( OctetStr& read_community_oct)
{  
   read_community_oct = read_community; 
};   
 
//------[ CTarget::set_writecommunity( const char * new_get_community)]----    
// set the write community name
void CTarget::set_readcommunity( const char * new_read_community)
{ 
   read_community = new_read_community;
}; 


//---------[ CTarget::set_getcommunity ]---------------------------------
// set the read community name
void CTarget::set_readcommunity( const OctetStr& new_read_community)
{
   read_community = new_read_community;
}; 


//--------[ CTarget::get_writecommunity() ]--------------------------        
// get the write community
char * CTarget::get_writecommunity()
{
   return (char *) write_community.get_printable();
};

//---------[ CTarget::get_writecommunity ]----------------------------
// get the write community
void CTarget::get_writecommunity( OctetStr &write_community_oct)
{
   write_community_oct = write_community;
};

//----------[ CTarget::set_writecommunity ]---------------------------
// set the write community
void CTarget::set_writecommunity( const char * new_write_community)
{
   write_community = new_write_community;
};
 
//-----------[ CTarget::set_writecommunity ]--------------------------- 
// set the write community
void CTarget::set_writecommunity( const OctetStr& write_community_oct)
{
   write_community = write_community_oct;
};
 
//------------[ Address& CTarget::get_address() ]---------------------    
// get the address
int CTarget::get_address( GenAddress &address)
{  
   address = my_address;
   return TRUE;
};

//-------------[ CTarget::set_address ]--------------------------------    
// set the address
int CTarget::set_address( Address &address)
{  
   my_address = address;
   if ( my_address.valid())
      validity = TRUE;
   else
      validity = FALSE;      

   return validity;
};  
 
//----------[ CTarget::resolve_to_V1 ]--------------------------------- 
// resolve entity
// common interface for Community based targets
int CTarget::resolve_to_C ( OctetStr &read_comm,
                            OctetStr &write_comm,
                            GenAddress &address,
                            unsigned long &t,
                            int &r,
                            unsigned char &v) 
{  
   // if the target is invalid then return false    
   if ( !validity)
     return FALSE;  

   read_comm = read_community;
   write_comm = write_community;

   address = my_address;  
             
   t = timeout;       
   r = retries;      
   v = version;
   
   return TRUE;
};                             


// overloaded assignment    
CTarget& CTarget::operator=( const CTarget& target)
{
   GenAddress tmpaddr(target.my_address);
   timeout = target.timeout;
   retries = target.retries;
   read_community = target.read_community;
   write_community = target.write_community;
   validity = target.validity;
   my_address = tmpaddr;
   
   return *this;
}; 

//=============[ int operator == CTarget, CTarget ]==========================
// equivlence operator overloaded
int operator==( const CTarget &lhs,const CTarget &rhs)
{   
   // need to compare all the members of a CTarget
   if ( lhs.read_community != rhs.read_community)
      return 0;  // !=

   if ( lhs.write_community != rhs.write_community)
      return 0;  // != 

   if ( lhs.my_address != rhs.my_address)
      return 0;

   if ( lhs.timeout != rhs.timeout)
      return 0;

   if ( lhs.retries != rhs.retries)
      return 0;


   return 1;  // they are equal

};  
