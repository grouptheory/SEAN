#include <iostream.h>
#include <stdlib.h>     // For exit() function prototype
#include "oid.h"
#include "oid_def.h"
#include "address.h"
#include "counter.h"
#include "ctr64.h"
#include "gauge.h"
#include "octet.h"
#include "pdu.h"
#include "snmp_pp.h"
#include "target.h"
#include "vb.h"

// default request oids
#define NUM_SYS_VBS	7
#define sysDescr	"1.3.6.1.2.1.1.1.0"
#define sysObjectID	"1.3.6.1.2.1.1.2.0"
#define sysUpTime	"1.3.6.1.2.1.1.3.0"
#define sysContact	"1.3.6.1.2.1.1.4.0"
#define sysName		"1.3.6.1.2.1.1.5.0"
#define sysLocation	"1.3.6.1.2.1.1.6.0"
#define sysServices	"1.3.6.1.2.1.1.7.0"

// default notification oid
#define coldStart	"1.3.6.1.6.3.1.1.4.3.0.1"

int
main(int argc, char **argv){
  int status;
  char *req_str      = "get";
  char *dflt_req_oid = sysDescr;
  char *dflt_trp_oid = coldStart;
  char *genAddrStr   = "127.0.0.1" ;		  // localhost
  char *oid_str      = NULL;
  Pdu pdu;
  Vb vb;

  if ( argc > 1)
      genAddrStr = argv[1]; 

  if ( argc > 2)
      req_str = argv[2];
      
  if ( argc > 3)
      oid_str = argv[3];
      
  Snmp my_session(status);
  if (status){
    cout << "Failed to create SNMP Session: " << status << endl;
    return(1);
  }
  cout << "Created session successfully" << endl;

  GenAddress genAddr(genAddrStr);
  if (! genAddr.valid()) {
    cout << "Invalid destination: " << genAddrStr << endl;
    return(1);
  }

  CTarget target(genAddr);
  if (! target.valid()) {
    cout << "Invalid target" << endl;
    return(1);
  }

  if ( strcmp(req_str, "get") == 0 ) {

    Vb vbl[NUM_SYS_VBS];
    vbl[0].set_oid(sysDescr);
    vbl[1].set_oid(sysObjectID);
    vbl[2].set_oid(sysUpTime);
    vbl[3].set_oid(sysContact);
    vbl[4].set_oid(sysName);
    vbl[5].set_oid(sysLocation);
    vbl[6].set_oid(sysServices);

    cout << "Send a GET-REQUEST to: " << genAddr.get_printable() << endl;
    if ( ! oid_str ) {
      if ( strcmp(genAddrStr,"localhost" ) == 0 || 
	   strcmp(genAddrStr, "127.0.0.1") == 0 ){
	pdu.set_vblist(vbl, NUM_SYS_VBS);
      } else {
	for (int i=0; i<NUM_SYS_VBS;i++)
	  pdu += vbl[i];
      }
    }
    else {
      Oid req_oid(oid_str);
      if ( ! req_oid.valid() ) {
	cout << "Request oid constructor failed for:" << oid_str << endl;
	return(1);
      }
      vb.set_oid(req_oid);
      pdu += vb;
    }
    status = my_session.get(pdu, target);
    if (status){
      cout << "Failed to issue SNMP Get: (" << status  << ") "
	   << my_session.error_msg(status) << endl;
      return(1);
    }
    else{
      cout << "Issued get successfully" << endl;
      int vbcount = pdu.get_vb_count();
      if ( vbcount == 7 ) {
	pdu.get_vblist(vbl, vbcount);
	for ( int i=0; i<vbcount ; i++ )  {
	  cout << vbl[i].get_printable_oid() << " : " <<
	    vbl[i].get_printable_value() << endl;
	}
      } else {
	for ( int i=0; i<vbcount ; i++ )  {
	  pdu.get_vb(vb, i);
	  cout << vb.get_printable_oid() << " : " <<
	    vb.get_printable_value() << endl;
	} 
      }
    } 
  }
  else if ( strcmp(req_str, "trap") == 0 ) {
    cout << "Send a TRAP to: " << genAddr.get_printable() << endl;
    
    if ( ! oid_str )
      oid_str = dflt_trp_oid;
    
    Oid notify_oid(oid_str);
    if ( ! notify_oid.valid() ) {
      cout << "Notify oid constructor failed for:" << oid_str << endl;
      return(1);
    }
    
    pdu.set_notify_id(notify_oid);
    
    // Use a simple payload
    vb.set_oid(sysLocation);
    vb.set_value("This is a test");
    pdu += vb;
    
    status = my_session.trap(pdu, target);
    
    if (status){
      cout << "Failed to issue SNMP Trap: (" << status  << ") "
	   << my_session.error_msg(status) << endl;
      return(1);
    } else {
      cout << "Success" << endl;
    } 
    
  } 
  else {
    cout << "Invalid SNMP operation: " << req_str  << endl ;
    cout << "Usage: " << argv[0] << " [get | trap]" << endl;
    return(1);
  }

  return(0);
}
