/*===================================================================

      U X S N M P . C P P

      UXSNMP CLASS DECLARATION


      Description:
	HPUX version of Snmp class

      Language:
	ANSI C++

      Operating System:
	HP-UX 9.0


      Author:
	Peter E Mellquist
	HP RND R & D

      Date:
	July 27, 1994

      Changes:
=====================================================================*/

char snmp_cpp_version[]="#(@)SNMP++ 2.6 $Header: uxsnmp.cpp,v 1.48 97/03/26 09:22:51 hmgr Exp $";

// For debuggin cout's
#include <iostream.h>

//-----[ includes ]----------------------------------------------------
extern "C"
{
#include <stdio.h>		    // standard io
#include <stdlib.h>		    // need for malloc
#include <unistd.h>		    // unix
#include <sys/socket.h>		// bsd socket stuff
#include <netinet/in.h>		// network types
#include <arpa/inet.h>		// arpa types
#include <sys/types.h>		// system types
#include <sys/time.h>		// time stuff
#include <errno.h>          // ux errs
}


#define _INCLUDE_SNMP_ERR_STRINGS

//----[ snmp++ includes ]----------------------------------------------
#include "oid.h"           // class def for oids
#include "oid_def.h"       // class def for well known trap oids
#include "vb.h"	           // class def for vbs
#include "address.h"       // class def for addresses
#include "snmp_pp.h"       // class def for this module
#include "msgqueue.h"      // message queue
#include "notifyqueue.h"   // notification queue
#include "eventlist.h"     // queue for holding outstanding messages
#include "snmpmsg.h"       // asn serialization class


//-----[ special includes ]-------------------------------------------
extern "C"
{

//------------[ if using Wind-U, then bring in the ms-windows header ]
#ifdef WU_APP
WORD app_hinst;
#else
typedef short WORD;
typedef long DWORD;
#endif

}

#ifdef _DEBUG
#define SNMPDEBUG(s)               printf(s)
#else
#define SNMPDEBUG(s)
#endif

//-----[ macros ]------------------------------------------------------
#define DEFAULT_TIMEOUT 1000  // one second defualt timeout
#define DEFAULT_RETRIES 1     // no retry default
#define SNMP_PORT 161	      // port # for SNMP
#define SNMP_TRAP_PORT 162    // port # for SNMP traps
#define MAX_SNMP_PACKET 2048  // maximum snmp packet len
#define MANY_AT_A_TIME 20     // many at a time size
#define MAX_GET_MANY 51	      // maximum get many size
#define MAX_RID 32767	      // max rid to use
#define MIN_RID 1000	      // min rid to use

//--------[ globals ]---------------------------------------------------
int ref_count = 0;		    // reference count of snmp objects
long int current_rid = MAX_RID;	// current rid to use

//--------------[ well known trap ids ]-----------------------------------
const coldStartOid coldStart;
const warmStartOid warmStart;
const linkDownOid linkDown;
const linkUpOid linkUp;
const authenticationFailureOid authenticationFailure;
const egpNeighborLossOid egpNeighborLoss;
const snmpTrapEnterpriseOid snmpTrapEnterprise;


//--------[ make the pdu request id ]-----------------------------------
// return a unique rid, clock can be too slow , so use current_rid
long MyMakeReqId()
{
  do {
   current_rid++;
   if ( current_rid > MAX_RID) current_rid = MIN_RID;
  } while (snmpEventList->GetEntry(current_rid));

  return (long) current_rid;
}

//---------[ Send SNMP Request ]---------------------------------------
// Send out a snmp request
int send_snmp_request( int sock, unsigned char *send_buf,
		       size_t send_len, Address & address)
{
  struct sockaddr_in agent_addr;  // send socket struct

  // UX only supports UDP type addresses (addr and port) right now 
  if (address.get_type() != type_udp)
  {
    return -1;// unsupported address type
  }
  
  // prepare the destination address
  memset(&agent_addr, 0, sizeof(agent_addr));
  agent_addr.sin_family = AF_INET;
  agent_addr.sin_addr.s_addr = inet_addr(((IpAddress &)address).IpAddress::get_printable());
  agent_addr.sin_port = ((UdpAddress &)address).get_port();

  // send the packet
  // pdu_container->pdu_sent();
  if ( sendto( sock, (char*) send_buf, send_len, 0,
	       (struct sockaddr*) &agent_addr, sizeof(agent_addr)) < 0)
  {
    SNMPDEBUG(strerror(errno));
    return -1; // send error!
  }

  return 0;
}

//---------[ receive a snmp response ]---------------------------------
// Receive a response from the specified socket.
// This function does not set the request id in the pdu if
// any error occur in receiving or parsing.  This is important 
// because the caller initializes this to zero and checks it to 
// see whether it has been changed to a valid value.  The 
// return value is the normal PDU status or SNMP_CLASS_SUCCESS.
// when we are successful in receiving a pdu.  Otherwise it
// is an error status.

int receive_snmp_response( int sock, Pdu &pdu)
{
  unsigned char receive_buffer[MAX_SNMP_PACKET];
  long receive_buffer_len; // len of received data
  struct sockaddr_in from_addr;
  int fromlen = sizeof(from_addr);


  // do the read
  do {
      receive_buffer_len = (long) recvfrom(sock,
					  (char *) receive_buffer,
					  MAX_SNMP_PACKET,
					  0,
					  (struct sockaddr*) &from_addr,
					  (int*) &fromlen);      
  } while (receive_buffer_len < 0 && EINTR == errno);

  if (receive_buffer_len <= 0 )		// error or no data pending
    return SNMP_CLASS_TL_FAILED;

  SnmpMessage snmpmsg;
  if ( snmpmsg.load( receive_buffer, receive_buffer_len) != SNMP_CLASS_SUCCESS)
	 return SNMP_CLASS_ERROR;

  OctetStr community_name;
  snmp_version version;
  if ( snmpmsg.unload( pdu, community_name, version) != SNMP_CLASS_SUCCESS)
	 return SNMP_CLASS_ERROR;


  //-----[ check for error status stuff..]
  // an error status is a valid pdu,
  // the caller needs to know about it
  if ( pdu.get_error_status() != 0)
	 return pdu.get_error_status();
  
  return SNMP_CLASS_SUCCESS;   // Success! return
}


//---------[ receive a snmp trap ]---------------------------------
// Receive a trap from the specified socket
int receive_snmp_notification( int sock, Pdu &pdu, CTarget &target)
{
  unsigned char receive_buffer[MAX_SNMP_PACKET];
  long receive_buffer_len; // len of received data
  struct sockaddr_in from_addr;
  int fromlen = sizeof(from_addr);

  // do the read
  do {
      receive_buffer_len = (long) recvfrom(sock,
					  (char *) receive_buffer,
					  MAX_SNMP_PACKET,
					  0,
					  (struct sockaddr*)&from_addr,
					  (int*) &fromlen);      
  } while (receive_buffer_len < 0 && EINTR == errno);

  if (receive_buffer_len <= 0 )		// error or no data pending
    return SNMP_CLASS_TL_FAILED;

  if (from_addr.sin_family != AF_INET) // sanity check
    return SNMP_CLASS_TL_FAILED;

  SnmpMessage snmpmsg;
  if ( snmpmsg.load( receive_buffer, receive_buffer_len) != SNMP_CLASS_SUCCESS)
	 return SNMP_CLASS_ERROR;

  OctetStr community_name;
  snmp_version version;
  if ( snmpmsg.unload( pdu, community_name, version) != SNMP_CLASS_SUCCESS)
    return SNMP_CLASS_ERROR;

  IpAddress ipa( (char*) inet_ntoa( from_addr.sin_addr));
  target.set_address( ipa);
  target.set_readcommunity( community_name);
  target.set_writecommunity( community_name);

  return SNMP_CLASS_SUCCESS;   // Success! return
}


//--------[ map action ]------------------------------------------------
// map the snmp++ action to a SMI pdu type
void map_action( unsigned short action, unsigned short & pdu_action)
{
      switch( action)
      {
         case sNMP_PDU_GET:
         case sNMP_PDU_GET_ASYNC:
           pdu_action = sNMP_PDU_GET;
         break;

         case sNMP_PDU_SET:
         case sNMP_PDU_SET_ASYNC:
           pdu_action = sNMP_PDU_SET;
         break;

         case sNMP_PDU_GETNEXT:
         case sNMP_PDU_GETNEXT_ASYNC:
           pdu_action = sNMP_PDU_GETNEXT;
         break;

         case sNMP_PDU_GETBULK:
         case sNMP_PDU_GETBULK_ASYNC:
           pdu_action = sNMP_PDU_GETBULK;
         break;

         default:
           pdu_action = sNMP_PDU_GET;  // TM ?? error ??
	 break;

      };  // end switch

}


//------[ Snmp Class Constructor ]--------------------------------------
Snmp::Snmp( int &status)		      // status of construction
{
  struct sockaddr_in mgr_addr;

  // intialize all the trap receiving member variables
  notify_targets = 0;
  notify_ids = 0;
  listen_addresses = 0;
  notifycallback = 0;
  notifycallback_data = 0;
#ifdef HPUX
  int errno = 0;
#endif

  // open a socket to be used for the session
  if (( iv_snmp_session = (int) socket( AF_INET, SOCK_DGRAM,0)) < 0) { 

    if (EMFILE == errno || ENOBUFS == errno || ENFILE == errno) {
	status = SNMP_CLASS_RESOURCE_UNAVAIL;	  
    } else if (EHOSTDOWN == errno) {
	status = SNMP_CLASS_TL_FAILED;  
    } else {
	status = SNMP_CLASS_TL_UNSUPPORTED;	  
    }
  } else {
    // set up the manager socket attributes
    memset(&mgr_addr, 0, sizeof(mgr_addr));
    mgr_addr.sin_family = AF_INET;
    mgr_addr.sin_addr.s_addr = htonl( INADDR_ANY);
    mgr_addr.sin_port = htons( 0);
    // bind the socket
    if ( bind( (int) iv_snmp_session,
	       (struct sockaddr*) &mgr_addr,
	       sizeof(mgr_addr)) < 0) {
      if (EADDRINUSE  == errno) {
	  status = SNMP_CLASS_TL_IN_USE;	  
      } else if (ENOBUFS == errno) {
	  status = SNMP_CLASS_RESOURCE_UNAVAIL;  
      } else if (errno == EAFNOSUPPORT){
	  status = SNMP_CLASS_TL_UNSUPPORTED;	  
      } else if (errno == ENETUNREACH || errno == EACCES){
	  status = SNMP_CLASS_TL_FAILED;	  
      } else {
	  status = SNMP_CLASS_INTERNAL_ERROR;
      }
    } else {
      ref_count++;
      status = SNMP_CLASS_SUCCESS;
      if ( ref_count == 1)
      {
	// pdu_container = new Pdu_Container();
#ifdef PDU_CONTAINER
	if ( pdu_container == NULL)
	{
	  status = SNMP_CLASS_RESOURCE_UNAVAIL;
	  close( (int) iv_snmp_session);
	}
#endif // PDU_CONTAINER
      }
    }
  }

#ifdef SNMPX11
  // Tell X11 to watch our file descriptor
  status = SnmpX11AddInput( (int)iv_snmp_session, pdu_handler);
#endif // SNMPX11

  construct_status = status;
  return;
};


//---------[ Snmp Class Destructor ]----------------------------------
Snmp::~Snmp()
{
  // if we failed during construction then don't try 
  // to free stuff up that was not allocated
  if ( construct_status != SNMP_CLASS_SUCCESS)
     return;

  close( (int) iv_snmp_session);    // close the dynamic socket
  ref_count--;
  // extra check for protection
  if ( ref_count < 0)
    ref_count = 0;
  
  if ( ref_count == 0)
    ;  // delete pdu_container;


   // go through the snmpEventList and delete any outstanding
   // events on this socket
   snmpEventList->DeleteSocketEntry((int)iv_snmp_session);

   // shut down trap reception if used
   notify_unregister();

#ifdef SNMPX11
  // Tell X11 to stop watching our file descriptor
   SnmpX11RemoveInput(pdu_handler);
#endif // SNMPX11
};


//-------------------[ returns error string ]--------------------------
char * Snmp::error_msg( const int c)
{
   return ((c<0)?
           ((c<MAX_NEG_ERROR)?nErrs[-(MAX_NEG_ERROR)+1]:nErrs[-c]):
            ((c>MAX_POS_ERROR)?pErrs[MAX_POS_ERROR+1]:pErrs[c]));
}


//------------------------[ get ]---------------------------------------
int Snmp::get( Pdu &pdu,	    // pdu to use
	       SnmpTarget &target)  // get target
{
   pdu.set_type( sNMP_PDU_GET);
   return snmp_engine( pdu,	  // get pdu
		       0,	  // max repeaters
		       0,	  // non repeaters
		       target,	  // target
		       NULL,	  // callback for async only
		       0);	  // callback data
};

//------------------------[ get async ]----------------------------------
int Snmp::get( Pdu &pdu,		    // pdu to use
	       SnmpTarget &target,	    // destination target
	       const snmp_callback callback,	    // async callback
	       const void * callback_data)        // callback data
{
   pdu.set_type( sNMP_PDU_GET_ASYNC);
   return snmp_engine( pdu,	       // get async pdu
		       0,	       // max repeaters
		       0,	       // non repeaters
		       target,	       // target
		       callback,       // callback to use
		       callback_data); // callback data
};

//------------------------[ get next ]-----------------------------------
int Snmp::get_next( Pdu &pdu,		  // pdu to use
		    SnmpTarget &target)	  // get target
{
   pdu.set_type( sNMP_PDU_GETNEXT);
   return snmp_engine( pdu,	   // pdu to get next
		       0,	   // max repeaters
		       0,	   // non repeaters
		       target,	   // target
		       NULL,	   // callback for async only
		       0);	   // callback data for async only
};

//------------------------[ get next async ]-----------------------------
int Snmp::get_next( Pdu &pdu,			  // pdu to use
		    SnmpTarget &target,		  // destination target
		    const snmp_callback callback,	  // callback to use
		    const void * callback_data)         // callback data
{
   pdu.set_type( sNMP_PDU_GETNEXT_ASYNC);
   return snmp_engine( pdu,		 // pdu to get next
		       0,		 // max repeaters
		       0,		 // non repeaters
		       target,		 // target
		       callback,	 // callback
		       callback_data);	 // callback data
};

//-------------------------[ set ]---------------------------------------
int Snmp::set( Pdu &pdu,	    // pdu to use
	       SnmpTarget &target)  // target address
{
   pdu.set_type( sNMP_PDU_SET);
   return snmp_engine( pdu,	    // pdu to set
		       0,	    // max repeaters
		       0,	    // non repeaters
		       target,	    // target
		       NULL,	    // callback for async only
		       0);	    // callback data
};

//------------------------[ set async ]----------------------------------
int Snmp::set( Pdu &pdu,		     // pdu to use
	       SnmpTarget &target,	     // destination target
	       const snmp_callback callback,	     // callback to use
	       const void * callback_data)         // callback data
{
   pdu.set_type( sNMP_PDU_SET_ASYNC);
   return snmp_engine( pdu,		  // pdu to set
		       0,		  // max repeaters
		       0,		  // non repeaters
		       target,		  // target
		       callback,	  // callback function
		       callback_data);	  // callback data
};

//-----------------------[ get bulk ]------------------------------------
int Snmp::get_bulk( Pdu &pdu,	     // pdu to use
		    SnmpTarget &target,	     // destination target
		    const int non_repeaters, // number of non repeaters
		    const int max_reps)	     // maximum number of repetitions
{
     pdu.set_type( sNMP_PDU_GETBULK);
     return snmp_engine( pdu,		    // pdu to use
			 non_repeaters,	    // # of non repeaters
			 max_reps,	    // max repititions
			 target,	    // target
			 NULL,		    // callback for async only
			 0);		    // callback data
};

//-----------------------[ get bulk async ]------------------------------
int Snmp::get_bulk( Pdu &pdu,		  // pdu to use
		    SnmpTarget &target,		  // destination target
		    const int non_repeaters,  // number of non repeaters
		    const int max_reps,		  // maximum number of repetitions
		    const snmp_callback callback,	  // callback to use
		    const void * callback_data)         // callback data
{
  pdu.set_type( sNMP_PDU_GETBULK_ASYNC);
  return snmp_engine( pdu,		      // pdu to use
		      non_repeaters,	      // # of non repeaters
		      max_reps,		      // max repititions
		      target,		      // target
		      callback,		      // callback function
		      callback_data);	      // callback data
};


//-----------------------[ cancel ]--------------------------------------
int Snmp::cancel( const unsigned long request_id)
{
  int status = snmpEventList->DeleteEntry(request_id);

  return(status);
};


//----------------------[ blocking inform, V2 only]------------------------
int Snmp::inform( Pdu &pdu,                // pdu to send
                  SnmpTarget &target)      // destination target
{
   pdu.set_type( sNMP_PDU_INFORM);
   return snmp_engine( pdu,
					   0,
					   0,
					   target,
					   NULL,
					   0);
					   
};

//----------------------[ asynch inform, V2 only]------------------------
int Snmp::inform( Pdu &pdu,                // pdu to send
                  SnmpTarget &target,      // destination target
                  const snmp_callback callback,  // callback function
                  const void * callback_data)    // callback data
{
    pdu.set_type( sNMP_PDU_INFORM);
	return snmp_engine( pdu,
                        0,
                        0,
                        target,
                        callback,
                        callback_data);
};


//---------------------[ send a trap ]-----------------------------------
int Snmp::trap( Pdu &pdu,                        // pdu to send
				SnmpTarget &target)             // destination target
{
  OctetStr my_get_community;
  OctetStr my_set_community;
  GenAddress address;
  unsigned long my_timeout;
  int my_retry;
  unsigned char version;
  int status;

  SNMPDEBUG("++ SNMP++, Send a Trap\n");
  //---------[ make sure pdu is valid ]---------------------------------
  if ( !pdu.valid())
  {
	SNMPDEBUG("-- SNMP++, PDU Object Invalid\n");
	return  SNMP_CLASS_INVALID_PDU;
  }

  //---------[ make sure target is valid ]------------------------------
  if ( !target.valid())
  {
	SNMPDEBUG("-- SNMP++, Target Object Invalid\n");
	return SNMP_CLASS_INVALID_TARGET;
  }								  

  //---------[ try to resolve it as a v1 target ]------------------------
  if ( !target.resolve_to_C( my_get_community,
                             my_set_community,
							 address,
							 my_timeout,
							 my_retry,
							 version))
  {
     SNMPDEBUG("-- SNMP++, Resolve Fail\n"); 
	 return SNMP_CLASS_UNSUPPORTED;
  }

  //--------[ determine request id to use ]------------------------------
  set_request_id(&pdu, MyMakeReqId());

  //--------[ check timestamp, if null use system time ]-----------------
  TimeTicks timestamp;
  pdu.get_notify_timestamp( timestamp);
  if (timestamp <= 0) {
    struct timeval tp;

    gettimeofday(&tp, NULL);
	tp.tv_sec -= 642816000;   // knock off 20years worth of seconds
	timestamp = (tp.tv_sec * 100) + (tp.tv_usec / 10000);
	pdu.set_notify_timestamp( timestamp);
  }

  //------[ validate address to use ]-------------------------------------
  if (!address.valid()) {
     SNMPDEBUG("-- SNMP++, Bad address\n");
     return SNMP_CLASS_INVALID_TARGET;
  }

  if ((address.get_type() != type_ip) &&
	  (address.get_type() != type_udp) )
  {
     SNMPDEBUG("-- SNMP++, Bad address type\n");
     return SNMP_CLASS_TL_UNSUPPORTED;
  }

  UdpAddress udp_address(address);
  if (!udp_address.valid()) {
     SNMPDEBUG("-- SNMP++, copy address failed\n");
     return SNMP_CLASS_RESOURCE_UNAVAIL;
  }

  //----------[ choose the target address port ]-----------------------
  if ((address.get_type() == type_ip) || !udp_address.get_port())
     udp_address.set_port(SNMP_TRAP_PORT);


  //----------[ based on the target type, choose v1 or v1 trap type ]-----
  if ( version == version1)
	 pdu.set_type( sNMP_PDU_V1TRAP);
  else
	 pdu.set_type( sNMP_PDU_TRAP);


  //------[ build the snmp message ]----------------------------------------
  SnmpMessage snmpmsg;
  status = snmpmsg.load( pdu, my_get_community, (snmp_version) version);
  if ( status != SNMP_CLASS_SUCCESS)
	 return status;


  //------[ send the trap ]
  status = send_snmp_request((int) iv_snmp_session,      
                                   snmpmsg.data(),              
								   (size_t)snmpmsg.len(), 
								   udp_address);       
  if (status != 0)
    return SNMP_CLASS_TL_FAILED;

  return SNMP_CLASS_SUCCESS;

};


//-----------------------[ access the trap reception info ]---------------
snmp_callback Snmp::get_notify_callback()
{ return notifycallback; };

void * Snmp::get_notify_callback_data()
{ return notifycallback_data; };


//-----------------------[ read the notification filters ]----------------
int Snmp::get_notify_filter( OidCollection &trapids,
			     TargetCollection &targets)
{
   if ( notify_ids != 0)
	  trapids = *notify_ids;
   if ( notify_targets != 0)
	  targets = *notify_targets;

   return SNMP_CLASS_SUCCESS;
};



//-----------------------[ register to get traps]-------------------------
int Snmp::notify_register( const OidCollection     &trapids,
			   const TargetCollection  &targets,
			   const AddressCollection &addresses,
			   const snmp_callback      callback,
                           const void              *callback_data) 
{ 
  int status;

   //-------------------------------------------------------------------------
   //-----------[ assign new trap filtering info ]----------------------------
   //-------------------------------------------------------------------------

#if 0
// TM: Note since the Snmp class does not allow me to get access to
// the notify_targets and notify_ids I carry them in the notify queue
// instead of adding them into the Snmp object.  This should be
// changed when Snmp allows extraction of these values.

   // delete current target collection if defined
   if (notify_targets != 0)
      delete notify_targets;
   // create a new collection using param passed in
   notify_targets = new TargetCollection ( targets);
   // delete current trapid collection if defined
   if (notify_ids != 0)
      delete notify_ids;
   // create a new collection using param passed in
   notify_ids = new OidCollection( trapids);
#endif // 0

   // assign callback and callback data info
   notifycallback = callback;
   notifycallback_data = (void *)callback_data;

   // remove any previous filters for this session
   notifyEventList->DeleteEntry(this);

   // add to the notify queue 
   status = notifyEventList->AddEntry(this, trapids, targets, addresses);

   return status;
};

// alternate form for local listen specification
//-----------------------[ register to get traps]-------------------------
int Snmp::notify_register( const OidCollection    &trapids,
			   const TargetCollection &targets,
			   const snmp_callback     callback,
                           const void             *callback_data) 
{
  AddressCollection addresses;
  
  return notify_register(trapids,
			 targets,
			 addresses,
			 callback,
			 callback_data);
}

//-----------------------[ un-register to get traps]----------------------
int Snmp::notify_unregister() 
{ 
#if 0
//TM: see comment in notify_register
   // free up local collections
   if ( notify_targets != 0)
      delete notify_targets;

   if ( notify_ids != 0)
      delete notify_ids;
#endif // 0

   // null out callback information
   notifycallback = 0;
   notifycallback_data = 0;

   // remove from the notify queue
   notifyEventList->DeleteEntry(this);

   return SNMP_CLASS_SUCCESS; 
};

//---------[ get / set engine ]-----------------------------------------
// The main snmp engine used for all requests
// async requests return out early and don't wait in here for
// the response
int Snmp::snmp_engine( Pdu &pdu,		 // pdu to use
		       long int non_reps,   	 // # of non repititions
		       long int max_reps,   	 // # of max repititions
		       SnmpTarget &target,  	 // from this target
		       const snmp_callback cb,	 // callback for async calls
		       const void * cbd)    	 // callback data

{
   long req_id;			// pdu request id
   int status;			// send status
   unsigned short pdu_action;	// type of pdu to build
   unsigned short action;	// type of pdu to build
   unsigned long my_timeout;	// target specific timeout
   int my_retry;		// target specific retry
   unsigned char send_buf[MAX_SNMP_PACKET];
   size_t send_len = MAX_SNMP_PACKET;
   OctetStr my_get_community;
   OctetStr my_set_community;
   GenAddress address;
   unsigned char version;

   //---------[ make sure pdu is valid ]--------------------------
   if ( !pdu.valid())
     return  SNMP_CLASS_INVALID_PDU;

   //---------[ depending on user action, map the correct pdu action]
   action = pdu.get_type();
   map_action(action, pdu_action);

   //---------[ check for correct mode ]---------------------------
   // if the class was constructed as a blocked model, callback=0
   // and async calls are attempted, an error is returned
   if (( cb == 0) &&
       ((action == sNMP_PDU_GET_ASYNC) ||
	(action == sNMP_PDU_SET_ASYNC) ||
	(action == sNMP_PDU_GETNEXT_ASYNC) ||
	(action == sNMP_PDU_GETBULK_ASYNC)))
     return SNMP_CLASS_INVALID_CALLBACK;

   //---------[ more mode checking ]--------------------------------
   // if the class was constructed as an async model, callback = something
   // and blocked calls are attempted, an error is returned
   if (( cb != 0) &&
       ((action == sNMP_PDU_GET) ||
	(action == sNMP_PDU_SET) ||
	(action == sNMP_PDU_GETNEXT) ||
	(action == sNMP_PDU_GETBULK)))
     return SNMP_CLASS_INVALID_CALLBACK;

   //---------[ make sure target is valid ]-------------------------
   // make sure that the target is valid
   if ( ! target.valid())
     return SNMP_CLASS_INVALID_TARGET;

   // try to resolve it as a v1 target instead
   if ( !target.resolve_to_C( my_get_community,
			      my_set_community,
			      address,
			      my_timeout,
			      my_retry,
			      version))
   {
     SNMPDEBUG("-- SNMP++, Target is not Community-based\n");
     return SNMP_CLASS_INVALID_TARGET;
   }

   if (!address.valid())
   {
     SNMPDEBUG("-- SNMP++, Target contains invalid address\n");
     return SNMP_CLASS_INVALID_TARGET;
   }

   //----------[ validate the target address ]--------------------------
   if ((address.get_type() != type_ip) &&
       (address.get_type() != type_udp) )
   {
     SNMPDEBUG("-- SNMP++, Bad address type\n");
     return SNMP_CLASS_TL_UNSUPPORTED; 
   }

   UdpAddress udp_address(address);
   if (!udp_address.valid())
   {
     SNMPDEBUG("-- SNMP++, Bad address\n");
     return SNMP_CLASS_RESOURCE_UNAVAIL;
   }

  
   //----------[ choose the target address port ]-----------------------
   if ((address.get_type() == type_ip) || !udp_address.get_port())
   {
     udp_address.set_port(SNMP_PORT);
   }
   // otherwise port was already set

   OctetStr community_string;
   //----------[ use the appropriate community string ]-----------------
   if (( action == sNMP_PDU_GET) ||
       ( action == sNMP_PDU_GET_ASYNC) ||
       ( action == sNMP_PDU_GETNEXT) ||
       ( action == sNMP_PDU_GETNEXT_ASYNC) ||
       ( action == sNMP_PDU_GETBULK) ||
       ( action == sNMP_PDU_GETBULK_ASYNC))
   {
     community_string = my_get_community;
   }
   else	 // got to be a set
   {
     community_string = my_set_community;
   }

   // set error index to none
   set_error_index(&pdu, 0);

   // determine request id to use
   req_id = MyMakeReqId();
   set_request_id(&pdu, req_id);

   //---------[ map GetBulk over v1 to GetNext ]-------------------------
   if (( pdu_action == sNMP_PDU_GETBULK)&&( (snmp_version)version== version1)) 
      pdu_action = sNMP_PDU_GETNEXT; 
   if ( pdu_action == sNMP_PDU_GETBULK) {
       set_error_status( &pdu,(int) non_reps);
	   set_error_index( &pdu,(int) max_reps);
   }

   pdu.set_type( pdu_action);

   SnmpMessage snmpmsg;
   status = snmpmsg.load( pdu, community_string,(snmp_version) version);
   if ( status != SNMP_CLASS_SUCCESS) {
	 printf("snmp message load error !\n");
     return status; 
   }

   //------[ send the request ]
   status = send_snmp_request((int) iv_snmp_session, // socket descriptor
			                  snmpmsg.data(), 
							  (size_t) snmpmsg.len(),// pdu to send
			                  udp_address);          // address to send to
   if ( status != 0)
   {
     return SNMP_CLASS_TL_FAILED;
   }

   // Add the message to the message queue
   snmpEventList->AddEntry( req_id, 
						    this, 
							(int)iv_snmp_session, 
							target, 
							pdu,
			                snmpmsg.data(), 
							(size_t) snmpmsg.len(), 
							udp_address, 
							cb, 
							(void *)cbd);

   //----[ if an async mode request then return success ]-----
   if (( action == sNMP_PDU_GET_ASYNC) ||
       ( action == sNMP_PDU_SET_ASYNC) ||
       ( action == sNMP_PDU_GETNEXT_ASYNC) ||
       ( action == sNMP_PDU_GETBULK_ASYNC))
   {
     // call the async handler in case someone is
     // firing multiple requests without yielding

     // TM seems kind of unfriendly...
     // SNMPProcessPendingEvents();

     return SNMP_CLASS_SUCCESS;
   }

   // Now wait for the response (or timeout) for our message.
   // This handles any necessary retries.
   status = SNMPBlockForResponse(req_id, pdu);

   return status;
}
