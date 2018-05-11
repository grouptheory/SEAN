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


  
  P D U . C P P

  PDU CLASS IMPLEMENTATION

  VERSION:
  2.6

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
  Pdu class implementation. Encapsulation of an SMI Protocol
  Data Unit (PDU) in C++.


=====================================================================*/
char pdu_cpp_version[]="#(@)SNMP++ 2.6 $Header: pdu.cpp,v 1.14 96/03/07 08:14:52 hmgr Exp $";

#include "pdu.h"       // include Pdu class definition

//=====================[ constructor no args ]=========================
Pdu::Pdu( void)
{
   // init all instance vars to null and invalid
   vb_count = 0;
   pdu_type = 0;
   validity = TRUE;
   error_status = 0;
   error_index = 0;
   request_id = 0;
   notify_timestamp = 0;
};

//=====================[ constructor with vbs and count ]==============
Pdu::Pdu( Vb* pvbs, const int pvb_count)
{
   int z;  // looping variable

   // Initialize all mv's
   vb_count = 0;
   pdu_type = 0;
   error_status = 0;
   error_index = 0;
   request_id = 0;
   notify_timestamp = 0;

   // zero is ok
   if ( pvb_count == 0) 
   {
      validity = TRUE;
      return;
   }

   // check for over then max
   if ( pvb_count > MAX_VBS) 
   {
       validity = FALSE;
       return;
   }

   // loop through and assign internal vbs
   for (z=0;z<pvb_count;z++) 
   {
     vbs[z] = new Vb( pvbs[z]);
     // check for new fail
     if ( vbs[z] == 0) 
     {
        validity = FALSE;
        return;
     }
   }

   // assign the vb count
   vb_count = pvb_count;

   validity = TRUE;
   return;

};

//=====================[ constructor with another Pdu instance ]========
Pdu::Pdu( const Pdu &pdu)
{
   vb_count = 0;
   *this = pdu;
   return;
};

//=====================[ destructor ]====================================
Pdu::~Pdu()
{
  for ( int z=0;z<vb_count;z++)
     delete vbs[z];
};

//=====================[ assignment to another Pdu object overloaded ]===
Pdu& Pdu::operator=( const Pdu &pdu)
{
   int z;   // looping variable

   // Initialize all mv's
   error_status = pdu.error_status;
   error_index = pdu.error_index;
   request_id = pdu.request_id;
   pdu_type = pdu.pdu_type;
   notify_id = pdu.notify_id;
   notify_timestamp = pdu.notify_timestamp;
   notify_enterprise = pdu.notify_enterprise;
   validity = TRUE;

   // free up old vbs
   for ( z=0;z<vb_count;z++)
     delete vbs[z];
   vb_count = 0;

   // check for zero case
   if ( pdu.vb_count == 0)
   {
      return *this;
   }

   // loop through and fill em up
   for (z=0;z<pdu.vb_count;z++)
   {
     vbs[z] = new Vb ( *(pdu.vbs[z]));
     // new failure
     if ( vbs[z] == 0)
     {
        validity = FALSE;
        return *this;
     }
   }

   vb_count = pdu.vb_count;

   return *this;
};

// append operator, appends a string
Pdu& Pdu::operator+=( Vb &vb)
{

  // do we have room?
  if ( vb_count+1> MAX_VBS)
    return *this;      

  // add the new one
  vbs[vb_count] = new Vb (vb);

  // up the vb count
  vb_count++;

  // set up validity
  validity = TRUE;

  // return self reference
  return *this;

};

//=====================[ extract Vbs from Pdu ]==========================
// how do you know that the caler has enough memory???
// should I self allocate this in here and require the
// caller then to free it up at soem later time
int Pdu::get_vblist( Vb* pvbs, const int pvb_count)
{
   if ((!pvbs) || ( pvb_count < 0) || ( pvb_count > vb_count))
      return FALSE;

   // loop through all vbs and assign to params
   for (int z=0;z<pvb_count;z++)
      pvbs[z] = *vbs[z];

   return TRUE;

};

//=====================[ deposit Vbs ]===================================
int Pdu::set_vblist( Vb* pvbs, const int pvb_count)
{

   // if invalid then don't destroy
   if ((!pvbs) || ( pvb_count < 0) || ( pvb_count > MAX_VBS))
     return FALSE;

   // free up current vbs
   for ( int z=0;z<vb_count;z++)
     delete vbs[z];
   vb_count = 0;

   // check for zero case
   if ( pvb_count == 0)
   {  
      validity = TRUE;
      error_status = 0;
      error_index = 0;
      request_id = 0;
      return FALSE;
   }
     

   // loop through all vbs and reassign them
   for ( z=0;z<pvb_count;z++)
   {
     vbs[z] = new Vb (pvbs[z]);
     // check for new fail
     if ( vbs[z] == 0)
     {
         validity = FALSE;
         return FALSE;
     }
   }
   
   vb_count = pvb_count;

   // clear error status and index since no longer valid
   // request id may still apply so don't reassign it
   error_status = 0;
   error_index = 0;
   validity = TRUE;

   return TRUE;
};

//===================[ get a particular vb ]=============================
// here the caller has already instantiated a vb object
// index is zero based
int Pdu::get_vb( Vb &vb, const int index) const
{
   // can't have an index less than 0
   if ( index < 0) 
     return FALSE;

   // can't ask for something not there
   if ( index > (vb_count-1))
      return FALSE;

   // asssign it
   vb = *vbs[index];

   return TRUE;
};

//===================[ set a particular vb ]=============================
int Pdu::set_vb( Vb &vb, const int index)
{
   // can't set a vb at index less than 0
   if ( index < 0)
     return FALSE;

   // can't ask for something not there
   if ( index > (vb_count-1))
      return FALSE;

   // delete what is there
   delete vbs[index];

   // assign it
   vbs[index] = new Vb (vb);

   return TRUE;

};

//=====================[ return number of vbs ]==========================
int Pdu::get_vb_count() const
{
   return vb_count;
};

//=====================[ return the error status ]=======================
int Pdu::get_error_status()
{
   return error_status;
};

//=====================[ set the error status ]==========================
// friend
void set_error_status( Pdu *pdu, const int status)
{
   if (pdu)
     pdu->error_status = status;
};

//=====================[ return the error index ]========================
int Pdu::get_error_index()
{
   return error_index;
};

//=====================[ set the error index ]===========================
// friend
void set_error_index( Pdu *pdu, const int index)
{
   if (pdu)
     pdu->error_index = index;
};

//=====================[ clear error status ]=============================
void clear_error_status( Pdu *pdu)
{
   if (pdu)
     pdu->error_status = 0;
};

//=====================[ clear error index ]==============================
void clear_error_index( Pdu *pdu)
{
   if (pdu)
     pdu->error_index = 0;
};

//=====================[ return the request id ]==========================
unsigned long Pdu::get_request_id()
{
   return request_id;
};

//=====================[ set the request id ]=============================
// friend function
void set_request_id( Pdu *pdu, const unsigned long rid)
{
   if (pdu)
     pdu->request_id = rid;
};

//=====================[ returns validity of Pdu instance ]===============
int Pdu::valid() const 
{
   return validity;
};

//=====================[ get the pdu type ]===============================
unsigned short Pdu::get_type()
{
   return pdu_type;
};

// set the pdu type
void Pdu::set_type( unsigned short type)
{
   pdu_type = type;
};


// trim off the last vb
int Pdu::trim(const int p)
{
   int lp = p;

   // verify that lp is legal
   if ( lp < 0 || lp > vb_count)
     return FALSE;

   while ( lp !=0)   
   {
     if ( vb_count > 0)
     {
         delete vbs[vb_count-1];
         vb_count--;
     }
     lp--;
   }
   return TRUE;
};



// delete a Vb anywhere within the Pdu
int Pdu::delete_vb( const int p)
{
   // position has to be in range
   if (( p<0) || (p>(vb_count-1)))
      return FALSE;

   // safe to remove it
   delete vbs[ p];

   for ( int z=p;z < (vb_count-1);z++)
   {
      vbs[z] = vbs[z+1];
   }  
   vb_count--;

   return TRUE;
};


// set notify timestamp
void Pdu::set_notify_timestamp( const TimeTicks & timestamp)
{
   notify_timestamp = timestamp;
};


// get notify timestamp
void Pdu::get_notify_timestamp( TimeTicks & timestamp)
{
    timestamp = notify_timestamp;
};

// set the notify id
void Pdu::set_notify_id( const Oid id)
{
    notify_id = id;
};

// get the notify id
void Pdu::get_notify_id( Oid &id)
{
   id = notify_id;
};

// set the notify enterprise
void Pdu::set_notify_enterprise( const Oid &enterprise)
{
   notify_enterprise = enterprise;
};

// get the notify enterprise
void Pdu::get_notify_enterprise( Oid & enterprise)
{
    enterprise = notify_enterprise;
};
