// -*- C++ -*-
// +++++++++++++++
//    S E A N        ---  Signalling Entity for ATM Networks  ---
// +++++++++++++++
// Version: 1.0.1
// 
// 			  Copyright (c) 1998
// 		 Naval Research Laboratory (NRL/CCS)
// 			       and the
// 	  Defense Advanced Research Projects Agency (DARPA)
// 
// 			 All Rights Reserved.
// 
// Permission to use, copy, and modify this software and its
// documentation is hereby granted, provided that both the copyright notice and
// this permission notice appear in all copies of the software, derivative
// works or modified versions, and any portions thereof, and that both notices
// appear in supporting documentation.
// 
// NRL AND DARPA ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
// DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM
// THE USE OF THIS SOFTWARE.
// 
// NRL and DARPA request users of this software to return modifications,
// improvements or extensions that they make to:
// 
//                 sean-dev@cmf.nrl.navy.mil
//                         -or-
//                Naval Research Laboratory, Code 5590
//                Center for Computation Science
//                Washington, D.C.  20375
// 
// and grant NRL and DARPA the rights to redistribute these changes in
// future upgrades.
//

#ifndef LINT
static char const _ilmiTerm_cc_rcsid_[] =
"$Id: ilmiTerm.cc,v 1.24 1999/04/08 20:10:38 marsh Exp marsh $";
#endif
#include <common/cprototypes.h>
#include <iostream.h>

extern "C" {
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/socket.h>   
#include <netinet/in.h>   
#include <arpa/inet.h>    
#include <sys/types.h>    
#include <sys/time.h>     
#include <errno.h>        
#include <strings.h>
#include <sys/fcntl.h>
#include <fore_xti/xti_user_types.h>
#include <fore_xti/xti.h>
#include <fore_xti/xti_atm.h>
#include <fore_xti/ans.h>
}

#include <snmpmsg.h>
#include <snmp_pp.h>

#define MAX_SNMP_PACKET 2048  // maximum snmp packet len

#define INCOMING   1
#define OUTGOING   2

#include "ilmiTerm.h"
#include "ilmiVisitors.h"

#define DEVICE          "/dev/xtipvc0"
// #define DEBUG 1

DriverReader::DriverReader(ilmiTerm *owner, int fd):InputHandler(owner,fd),_owner(owner){ }

inline void DriverReader::Callback(void)
{
  int ret = _owner->receive_ilmi_response();
  if(ret != SNMP_CLASS_SUCCESS)
    {
      cout << " looks like a BAD  response error = " << ret << endl;
      exit(1);
    }
}

DriverReader::~DriverReader()
{
}


ilmiTerm::ilmiTerm() 
{
  StartVisitor *iv1 = new StartVisitor();
  _go1 = new StartTimer(this,iv1);
  Register(_go1);
  LinkUpVisitor *iv2 = new LinkUpVisitor();
  _go2 = new LinkUpTimer(this,iv2);
  Register(_go2);

  _trap_visitor_type = QueryRegistry(TRAP_VISITOR_NAME);

}


ilmiTerm::ilmiTerm(int vpi, int vci) :_community("ILMI"),_version(version1)
{
  int ilmi_vpi = vpi;
  int ilmi_vci = vci;
  int sig_qos = 0;
  int qos = 0;
  char *options = NULL;
  long optlen = 0;

  struct t_info info;
  _fd = t_open(DEVICE, O_RDWR, &info);
  if(_fd < 0)
    {
      cout << "t_open failed\n";
      exit(1);
    }
  if(t_bind(_fd, (struct t_bind *) NULL, (struct t_bind *) NULL) < 0)
    {
      cout << "t_bind failed\n";
      exit(1);
    }
  struct t_call call_req;
  PVCAddress addr_req;
  addr_req.addressType=AF_ATM_PVC;
  addr_req.vpi = ilmi_vpi;
  addr_req.vci = ilmi_vci;
  memset(&call_req, 0, sizeof(call_req));
  call_req.addr.len = sizeof(addr_req);
  call_req.addr.buf = (char *) &addr_req;
  cout << "Connecting to (" << ilmi_vpi << " , " << ilmi_vci << ") ..... " << endl;
  options = (char *) NULL;
  if(t_connect(_fd, &call_req, (struct t_call *) NULL) < 0)
    {
      cout << "t_connect failed " << endl;
      exit(1);
    }
  cout << "connected " << endl;
  _reader = new DriverReader(this,_fd);
  assert(_reader);
  Register(_reader);
  _trap_visitor_type = QueryRegistry(TRAP_VISITOR_NAME);
}

void ilmiTerm::SendColdStart()
{
  DIAG("ilmi", DIAG_DEBUG, cout << OwnerName() << " Sending a COLDSTART "  << endl;);
  int len = 56;
  unsigned char data[56] = {
    0x30, 0x82, 0x00, 0x34, 0x02, 0x01, 0x00, 0x04, 0x04, 0x49, 0x4C, 0x4D, 0x49, 0xA4, 0x29, 0x06, 
    0x08, 0x2B, 0x06, 0x01, 0x04, 0x01, 0x03, 0x01, 0x01, 0x40, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02, 
    0x01, 0x00, 0x02, 0x01, 0x00, 0x43, 0x01, 0x00, 0x30, 0x0E, 0x30, 0x0C, 0x06, 0x08, 0x2B, 0x06, 
    0x01, 0x02, 0x01, 0x01, 0x03, 0x00, 0x05, 0x00 };
  send_ilmi_request(data,len);
}


ilmiTerm::~ilmiTerm() { }
  
void ilmiTerm::Interrupt(SimEvent * e)
{
  cout << "I was Interrupted by event " << endl;
}

void ilmiTerm::Absorb(Visitor* v)
{
  VisitorType vt1 = v->GetType();
  if (_trap_visitor_type && vt1.Is_A(_trap_visitor_type))
    {
      //  cout << OwnerName() << " Received a TrapVisitor " << endl;
      // we do not send the Trap pdu in the visitor because
      // it is an snmpV2 and ILMI use snmpV1, so we hardwired one.
      SendColdStart();
    }
  else
    {
      //  cout << OwnerName() << " received a " << v->GetType() << endl;
      ilmiVisitor *iv = (ilmiVisitor *)v;
      Pdu pdu = iv->GetPDU();
      Print(pdu,OUTGOING);
      SnmpMessage *snmpmsg  = new SnmpMessage();         
      snmpmsg->load(pdu,_community,_version);
      send_ilmi_request(snmpmsg->data(),snmpmsg->len());
      delete snmpmsg;
    }
  v->Suicide();
}

void ilmiTerm::DumpPkt(char *dir, char *receive_buffer, int receive_buffer_len)
{
  printf("%s",dir);
  for(int i=0; i<receive_buffer_len; i++) 
    {
      printf("%02x",(unsigned char)receive_buffer[i]);
      if (((i+1)%4)==0) printf(" ");
      if (((i+1)%24)==0) printf("\n    ");
    }
  printf("\n\n");
}

int ilmiTerm::send_ilmi_request(unsigned char *send_buf, size_t send_len)
{
  t_snd(_fd,(char*) send_buf,send_len,0);
#ifdef DEBUG
  DumpPkt("vvv ",(char *)send_buf,send_len);
#endif
  return 0;
}

int ilmiTerm::receive_ilmi_response()
{
  Pdu pdu;
  unsigned char receive_buffer[MAX_SNMP_PACKET];
  long receive_buffer_len;
  do {
    int flags;
    if ((receive_buffer_len=(long)t_rcv(_fd, (char *)receive_buffer, MAX_SNMP_PACKET, &flags))<=0)
      {
	perror("Problem with t_rcv\n");
	exit(1);
      }
  } while (receive_buffer_len < 0 && EINTR == errno);
#ifdef DEBUG  
  DumpPkt("^^^ ",(char *)receive_buffer,receive_buffer_len);
#endif
  if (receive_buffer_len <= 0)
    return SNMP_CLASS_TL_FAILED;
  SnmpMessage snmpmsg;
  if(snmpmsg.load(receive_buffer, receive_buffer_len) != SNMP_CLASS_SUCCESS)
    return SNMP_CLASS_ERROR;
  OctetStr community_name;
  snmp_version version;
  if(snmpmsg.unload(pdu,community_name, version) != SNMP_CLASS_SUCCESS)
    return SNMP_CLASS_ERROR;
  Print(pdu,INCOMING);
  if(pdu.get_error_status() != 0)
    return pdu.get_error_status();
  ilmiVisitor *iv;
  switch(pdu.get_type())
    {
    case sNMP_PDU_GET:
    case sNMP_PDU_GETNEXT:
      iv = new GetVisitor(pdu,pdu.get_request_id());
      break;
    case sNMP_PDU_RESPONSE:
      iv = new RespVisitor(pdu,pdu.get_request_id());
      break;
    case sNMP_PDU_SET:
      iv = new SetVisitor(pdu,pdu.get_request_id());
      break;
    case sNMP_PDU_V1TRAP:
    case sNMP_PDU_TRAP:
      iv = new TrapVisitor(pdu,pdu.get_request_id());
      break;
    default:
      cout << "UNKNOWN PDU !!!!!!!!!!!!!!!" << endl;
      break;
    }
  assert(iv != 0);
  Inject(iv);
  return SNMP_CLASS_SUCCESS;
}


void ilmiTerm::Print(Pdu & pdu, int dir)
{
  char buffer[100];
  switch(pdu.get_type())
    {
    case sNMP_PDU_GET:
      strcpy(buffer,"sNMP_PDU_GET");
      break;
    case sNMP_PDU_GETNEXT:
      strcpy(buffer,"sNMP_PDU_GETNEXT");
      break;
    case sNMP_PDU_RESPONSE:
      strcpy(buffer,"sNMP_PDU_RESPONSE");
      break;
    case sNMP_PDU_SET:
      strcpy(buffer,"sNMP_PDU_SET");
      break;
    case sNMP_PDU_V1TRAP:
      strcpy(buffer,"sNMP_PDU_V1TRAP");
      break;
    case sNMP_PDU_GETBULK:
      strcpy(buffer,"sNMP_PDU_GETBULK");
      break;
    case sNMP_PDU_INFORM:
      strcpy(buffer,"sNMP_PDU_INFORM");
      break;
    case sNMP_PDU_TRAP:
      strcpy(buffer,"sNMP_PDU_TRAP");
      break;
    default:
      strcpy(buffer,"UNKNOWN");
      break;
    }
  int rid = pdu.get_request_id();
  if(dir == INCOMING)
    DIAG("ilmi", DIAG_DEBUG, cout << OwnerName() << " : <== " << buffer << " req_id = " << rid << endl;)
  else
    DIAG("ilmi", DIAG_DEBUG, cout << OwnerName() << " : ==> " << buffer << " req_id = " << rid << endl;)

  int k;
  int vb_cnt = pdu.get_vb_count();
  for(k = 0; k < vb_cnt; k++)
    {
      Vb ivb;
      pdu.get_vb(ivb,k);
      DIAG("ilmi", DIAG_DEBUG, cout << "         Oid = " << ivb.get_printable_oid(););
      if(pdu.get_type() == sNMP_PDU_RESPONSE || pdu.get_type() == sNMP_PDU_SET)
	DIAG("ilmi", DIAG_DEBUG, cout << "      Value = " << ivb.get_printable_value() << endl;)
      else
	DIAG("ilmi", DIAG_DEBUG, cout << endl;)
    }
}
