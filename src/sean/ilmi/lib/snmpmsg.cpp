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


  SNMP++ S N M P M E S S A G E . C P P   
      
  SNMPMESSAGE CLASS DEFINITION
       
  VERSION:
  2.6
  
  RCS INFO:
  $Header: Exp $
       
  DESIGN:
  Peter E Mellquist
                
  AUTHOR:      
  Peter E Mellquist
              
  LANGUAGE:
  ANSI C++ 
      
  OPERATING SYSTEMS:
  Win 32
  BSD UNIX
      
  DESCRIPTION:
  ASN.1	encoding / decoding class
      		 
=====================================================================*/ 

#include "snmpmsg.h"	                // header file for SnmpMessage
#include "asn1.h"                       // ASN.1 header file
#include "stdio.h"                      // standard io file 



//--------------[ well known trap ids ]-----------------------------------
// SMI trap oid def
class snmpTrapsOid: public Oid {
   public: 
   snmpTrapsOid (void):Oid("1.3.6.1.6.3.1.1.5"){};
}; 

// SMI Enterprise Oid
class snmpTrapEnterpriseOid: public Oid {
   public: 
   snmpTrapEnterpriseOid(void):Oid("1.3.6.1.6.3.1.1.4.3.0"){};
}; 

// SMI Cold Start Oid
class coldStartOid: public snmpTrapsOid {
   public: 
   coldStartOid( void){*this+=".1";};
};

// SMI WarmStart Oid
class warmStartOid: public snmpTrapsOid {
   public: 
   warmStartOid( void){*this+=".2";};
};

// SMI LinkDown Oid
class linkDownOid: public snmpTrapsOid {
   public: 
   linkDownOid( void){*this+=".3";};
}; 


// SMI LinkUp Oid
class linkUpOid: public snmpTrapsOid {
   public: 
   linkUpOid( void){*this+=".4";};
};

// SMI Authentication Failure Oid
class authenticationFailureOid: public snmpTrapsOid {
   public: 
   authenticationFailureOid( void){*this+=".5";};
}; 

// SMI egpneighborloss Oid
class egpNeighborLossOid: public snmpTrapsOid {
   public: 
   egpNeighborLossOid( void){*this+=".6";};
};

const coldStartOid coldStart;
const warmStartOid warmStart;
const linkDownOid linkDown;
const linkUpOid linkUp;
const authenticationFailureOid authenticationFailure;
const egpNeighborLossOid egpNeighborLoss;
const snmpTrapEnterpriseOid snmpTrapEnterprise;

//------------[ convert SNMP++ VB to WinSNMP smiVALUE ]----------------
int convertVbToSmival( Vb &tempvb, SmiVALUE *smival )
{
   smival->syntax = tempvb.get_syntax();
   switch ( smival->syntax ) {

	  // case sNMP_SYNTAX_NULL
	  case sNMP_SYNTAX_NULL:
      break;

	  // case sNMP_SYNTAX_INT32:
      case sNMP_SYNTAX_INT:
	  tempvb.get_value(smival->value.sNumber);
	  break;

      //    case sNMP_SYNTAX_UINT32:
      case sNMP_SYNTAX_GAUGE32:
      case sNMP_SYNTAX_CNTR32:
      case sNMP_SYNTAX_TIMETICKS:
	  case sNMP_SYNTAX_UINT32:
	  tempvb.get_value(smival->value.uNumber);
	  break;

	  // case Counter64
      case sNMP_SYNTAX_CNTR64:
      {
	     Counter64 c64;
	     tempvb.get_value(c64);
	     smival->value.hNumber.hipart = c64.high();
	     smival->value.hNumber.lopart = c64.low();
	  }
	  break;

      case sNMP_SYNTAX_BITS:
      case sNMP_SYNTAX_OCTETS:
      case sNMP_SYNTAX_IPADDR:
      {
	     OctetStr os;
	     tempvb.get_value(os);
	     smival->value.string.ptr = NULL;
	     smival->value.string.len = os.len();
	     if ( smival->value.string.len > 0 ) 
		 {
			smival->value.string.ptr = (SmiLPBYTE) new  unsigned char [smival->value.string.len];
	        if ( smival->value.string.ptr ) 
		    {
               for (int i=0; i<(int) smival->value.string.len ; i++)
		          smival->value.string.ptr[i] = os[i];
	        } 
		    else 
		    { 
	           smival->syntax = sNMP_SYNTAX_NULL;  // invalidate the smival
	           return SNMP_CLASS_RESOURCE_UNAVAIL;
	        }
	     }
      }
      break;

      case sNMP_SYNTAX_OID:
	  {
	     Oid oid;
	     tempvb.get_value(oid);
	     smival->value.oid.ptr = NULL;
	     smival->value.oid.len = oid.len();
	     if ( smival->value.oid.len > 0 ) 
		 {
			smival->value.oid.ptr = (SmiLPUINT32) new unsigned long [ smival->value.oid.len];
	        if ( smival->value.oid.ptr ) 
			{
               for (int i=0; i<(int)smival->value.oid.len ; i++)
		          smival->value.oid.ptr[i] = oid[i];
	        } 
			else 
			{
	            smival->syntax = sNMP_SYNTAX_NULL;  // invalidate the smival
	            return SNMP_CLASS_RESOURCE_UNAVAIL;
	        }
	    }
	  }
	  break;

      default:
        return SNMP_CLASS_INTERNAL_ERROR;
   }
   return SNMP_CLASS_SUCCESS;
};

// free a SMI value
void freeSmivalDescriptor( SmiVALUE *smival )
{
   switch ( smival->syntax ) {
     case sNMP_SYNTAX_OCTETS:
     case sNMP_SYNTAX_OPAQUE:
     case sNMP_SYNTAX_IPADDR:
     case sNMP_SYNTAX_BITS:		    // obsoleted in SNMPv2 Draft Std
          delete [] smival->value.string.ptr;	   
       break;

     case sNMP_SYNTAX_OID:
		  delete [] smival->value.oid.ptr;
       break;
   }
   smival->syntax = sNMP_SYNTAX_NULL;
};


// load up a SnmpMessage
int SnmpMessage::load( Pdu pdu,                      // Pdu object to load
					   OctetStr community,           // community to use
					   snmp_version version)         // version 1 or 2
{
	int status;

	// create a raw pdu
	snmp_pdu *raw_pdu;
	raw_pdu = snmp_pdu_create( (int) pdu.get_type());
	
	Oid enterprise;

	// make sure pdu is valid
	if ( !pdu.valid()) 
		SNMP_CLASS_INVALID_PDU;

	// load it up
	raw_pdu->reqid = pdu.get_request_id();
	raw_pdu->errstat= (unsigned long) pdu.get_error_status();
	raw_pdu->errindex= (unsigned long) pdu.get_error_index();

	// if its a V1 trap then load up other values
	// for v2, use normal pdu format
	if ( raw_pdu->command == sNMP_PDU_V1TRAP)
	{
	   // forget about the v1 trap agent address
	   // use transport address to determine where
	   // it came from

	   //-----[ compute generic trap value ]-------------------------------
       // determine the generic value
       // 0 - cold start
       // 1 - warm start
       // 2 - link down
       // 3 - link up
       // 4 - authentication failure
       // 5 - egpneighborloss
       // 6 - enterprise specific
	   Oid trapid;
	   pdu.get_notify_id( trapid);
	   if ( !trapid.valid() || trapid.len() < 2 ) 
	   {
 	       snmp_free_pdu( raw_pdu);
	       return SNMP_CLASS_INVALID_NOTIFYID;
       }
	   raw_pdu->specific_type=0;
       if ( trapid == coldStart)
          raw_pdu->trap_type = 0;  // cold start
       else if ( trapid == warmStart)
          raw_pdu->trap_type = 1;  // warm start
       else if( trapid == linkDown)
          raw_pdu->trap_type = 2;  // link down
       else if ( trapid == linkUp)
          raw_pdu->trap_type = 3;  // link up
       else if ( trapid == authenticationFailure )
          raw_pdu->trap_type = 4;  // authentication failure
       else if ( trapid == egpNeighborLoss)
          raw_pdu->trap_type = 5;  // egp neighbor loss
       else {
	      raw_pdu->trap_type = 6;     // enterprise specific
	                                  // last oid subid is the specific value
	                                  // if 2nd to last subid is "0", remove it 
			                          // enterprise is always the notify oid prefix
	      raw_pdu->specific_type = (int) trapid[(int) (trapid.len()-1)]; 
          trapid.trim(1);
	      if ( trapid[(int)(trapid.len()-1)] == 0 )
	      trapid.trim(1);
          enterprise = trapid;      
       }

	   if ( raw_pdu->trap_type !=6)
	      pdu.get_notify_enterprise( enterprise);
	   if ( enterprise.len() >0) {
		   // note!!
		   // these are hooks into an SNMP++ oid
		   // and therefor the raw_pdu enterprise
		   // should not free them. null them out!!
		   SmiLPOID rawOid;
		   rawOid = enterprise.oidval();
		   raw_pdu->enterprise = rawOid->ptr;
		   raw_pdu->enterprise_length = (int) rawOid->len;
	   }

	   // timestamp
	   TimeTicks timestamp;
	   pdu.get_notify_timestamp( timestamp);
	   raw_pdu->time = ( unsigned long) timestamp;
	   
	}

	// if its a v2 trap then we need to make a few adjustments
	// vb #1 is the timestamp
	// vb #2 is the id, represented as an Oid
	if ( raw_pdu->command == sNMP_PDU_TRAP)
	{
		Pdu temppdu;
		Vb tempvb;

		// vb #1 is the timestamp
		TimeTicks timestamp;
		tempvb.set_oid("1.3.6.1.2.1.1.3.0");   // sysuptime
		pdu.get_notify_timestamp( timestamp);
		tempvb.set_value ( timestamp);
		temppdu += tempvb;               	 

		// vb #2 is the id
		Oid trapid;
		tempvb.set_oid("1.3.6.1.6.3.1.1.5");
		pdu.get_notify_id( trapid);
		tempvb.set_value( trapid);
		temppdu += tempvb;

		// append the remaining vbs
		for ( int z=0;z<pdu.get_vb_count();z++) {
			pdu.get_vb( tempvb,z);
			temppdu += tempvb;
		}

        pdu = temppdu;          // reassign the pdu to the temp one
	}
	// load up the payload
	// for all Vbs in list, add them to the pdu
	int vb_count;
	Vb tempvb;
	Oid tempoid;
	SmiLPOID smioid;
	SmiVALUE smival;

	vb_count = pdu.get_vb_count();
	for (int z=0;z<vb_count;z++) {
		pdu.get_vb( tempvb,z);
		tempvb.get_oid( tempoid);
		smioid = tempoid.oidval();
		// clear the value portion, in case its
		// not already been done so by the app
		// writer
		// only do it in the case its a get,next or bulk
		if ((raw_pdu->command == sNMP_PDU_GET) ||
			(raw_pdu->command == sNMP_PDU_GETNEXT) ||
            (raw_pdu->command == sNMP_PDU_GETBULK))
			tempvb.set_null();
		status = convertVbToSmival( tempvb, 
			                        &smival );
		if ( status != SNMP_CLASS_SUCCESS)
			return status;

		// add the vb to the raw pdu
	    snmp_add_var( raw_pdu, 
			          smioid->ptr, 
			          (int) smioid->len,
			          &smival);

		freeSmivalDescriptor( &smival);

	}

	// ASN1 encode the pdu
	status = snmp_build( raw_pdu,
	                     databuff,
						 (int *) &bufflen,
						 version,
						 community.data(),
						 (int) community.len());


	if ( status != 0) {
		valid_flag = FALSE;
		return SNMP_ERROR_WRONG_ENCODING;
	}
	valid_flag = TRUE;

	// prevention of SNMP++ Enterprise Oid death
	if ( enterprise.len() >0) {
		raw_pdu->enterprise = 0;
		raw_pdu->enterprise_length=0;
	}
	
	snmp_free_pdu( raw_pdu);

	return SNMP_CLASS_SUCCESS;
};

// load up a SnmpMessage
int SnmpMessage::load( unsigned char *data, 
					   unsigned long len)
{
   if ( len <= SNMP_MSG_LENGTH) {
	   memcpy( (unsigned char *) databuff,
		       (unsigned char *) data,
			   (unsigned int) len);
	   bufflen = len;
	   valid_flag = TRUE;
   }
   else
	   return SNMP_ERROR_WRONG_LENGTH;

   return SNMP_CLASS_SUCCESS;	
};


// unload the data into SNMP++ objects
int SnmpMessage::unload( Pdu &pdu,                 // Pdu object
						 OctetStr &community,      // community object
						 snmp_version &version)    // SNMP version #
{
   unsigned char community_name[255];
   unsigned long community_len;

   Pdu tmppdu;

   pdu = tmppdu;

   if ( !valid_flag)
	   return SNMP_CLASS_INVALID;

   snmp_pdu *raw_pdu;
   raw_pdu = snmp_pdu_create(0);

   int status = snmp_parse( raw_pdu,
                            databuff,
							community_name,
							community_len,
							version,
                            (int) bufflen);
   
   if ( status != 0) 
	   return SNMP_CLASS_INTERNAL_ERROR;

   
   // load up the SNMP++ variables
   community.set_data( community_name,
		               community_len);
   set_request_id( &pdu,raw_pdu->reqid);
   set_error_status( &pdu,(int) raw_pdu->errstat);
   set_error_index( &pdu,(int) raw_pdu->errindex);
   pdu.set_type( raw_pdu->command);

   // deal with trap parameters
   TimeTicks timestamp;
   timestamp = raw_pdu->time;
   pdu.set_notify_timestamp( timestamp);

   // vbs
   Vb tempvb;
   Oid tempoid;
   struct   variable_list *vp;
   for(vp = raw_pdu->variables; vp; vp = vp->next_variable) {

	 // extract the oid portion
	 tempoid.set_data( (unsigned long *)vp->name,
                       ( unsigned int) vp->name_length);
	 tempvb.set_oid( tempoid);

	 // extract the value portion
	 switch(vp->type){

	 // octet string
	 case sNMP_SYNTAX_OCTETS:
     case sNMP_SYNTAX_OPAQUE:
	 {
	    OctetStr octets( (unsigned char *) vp->val.string,
                         (unsigned long) vp->val_len);
	    tempvb.set_value( octets);
	 }
	 break;

	 // object id
	 case sNMP_SYNTAX_OID:
	 {
	   	Oid oid( (unsigned long*) vp->val.objid,
			     (int) vp->val_len);
		tempvb.set_value( oid);
	 }
     break;

	 // timeticks
	 case sNMP_SYNTAX_TIMETICKS:
	 {
		 TimeTicks timeticks( (unsigned long) *(vp->val.integer));
		 tempvb.set_value( timeticks);
	 }
	 break;

	 // 32 bit counter
	 case sNMP_SYNTAX_CNTR32:
	 {
	     Counter32 counter32( (unsigned long) *(vp->val.integer));
		 tempvb.set_value( counter32);
	 }
	 break;

	 // 32 bit gauge
	 case sNMP_SYNTAX_GAUGE32:
	 {
	     Gauge32 gauge32( (unsigned long) *(vp->val.integer));
		 tempvb.set_value( gauge32);
	 }
	 break;

	 // ip address
	 case sNMP_SYNTAX_IPADDR:
	 {
		 char buffer[20];
		 sprintf( buffer,"%d.%d.%d.%d",
			      vp->val.string[0],
				  vp->val.string[1],
				  vp->val.string[2],
				  vp->val.string[3]);
	     IpAddress ipaddress( buffer);
		 tempvb.set_value( ipaddress);
	 }
	 break;

	 // 32 bit integer
	 case sNMP_SYNTAX_INT:
	 {
	    SnmpInt32 int32( (long) *(vp->val.integer));
		tempvb.set_value( int32);
	 }
	 break;

	 // 32 bit unsigned integer
	 case sNMP_SYNTAX_UINT32:
	 {
		SnmpUInt32 uint32( (unsigned long) *(vp->val.integer));
		tempvb.set_value( uint32);
	 }
     break;

	 // v2 counter 64's
	 case sNMP_SYNTAX_CNTR64:
     break;

 	 case sNMP_SYNTAX_NULL:
	   tempvb.set_null();
	 break;

	 // v2 vb exceptions
	 case sNMP_SYNTAX_NOSUCHOBJECT:
	 case sNMP_SYNTAX_NOSUCHINSTANCE:
	 case sNMP_SYNTAX_ENDOFMIBVIEW:
		 set_exception_status( &tempvb, vp->type);
     break;

	 default:
		 tempvb.set_null();

    } // end switch

	 // append the vb to the pdu
	 pdu += tempvb;
   }
   
   snmp_free_pdu( raw_pdu);

   return SNMP_CLASS_SUCCESS;	
};

