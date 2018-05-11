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
static char const _parse_cc_rcsid_[] =
"$Id: parse.cc,v 1.1 1999/03/25 18:43:25 battou Exp $";
#endif
#include <iostream.h>     // cout, <<
#include <fstream.h>      // instream, is_open(), eof(), clear(), seekg(),  >> 
#include <assert.h> 
#include <snmpmsg.h>
#include <snmp_pp.h>

#define MAX_SNMP_PACKET 2048  // maximum snmp packet len

#define INCOMING 1
#define OUTGOING 2

void Print(Pdu & pdu, int dir)
{
  char buffer[1024];
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
      sprintf(buffer,"%02x UNKNOWN", (unsigned char)pdu.get_type());
      break;
    }
  int rid = pdu.get_request_id();
  if(dir == INCOMING)
    cout << " : <== " << buffer << " req_id = " << rid << endl;
  else
    cout << " : ==> " << buffer << " req_id = " << rid << endl;
  int k;
  int vb_cnt = pdu.get_vb_count();
  for(k = 0; k < vb_cnt; k++)
    {
      Vb ivb;
      pdu.get_vb(ivb,k);
      cout << "         Oid = " << ivb.get_printable_oid();
      if(pdu.get_type() == sNMP_PDU_RESPONSE || pdu.get_type() == sNMP_PDU_SET)
	cout << "      Value = " << ivb.get_printable_value() << endl;
      else
	cout << endl;
    }
}

#define value(x,y) ((x) > '9' ? ((x)-'A'+10) : (x)-'0')<<4 |  ((y) > '9' ? ((y)-'A'+10) : (y)-'0')
main()
{
  Pdu pdu;
  unsigned char receive_buffer[MAX_SNMP_PACKET];
  long receive_buffer_len;
  char line[3000];
  int dir;

  ifstream inStream("ilmi_fore.log");
  assert(inStream.is_open());
  int m = 0;
  for (;;)
    {
      inStream.getline(line,3000,'\n'); // direction - data
      if (inStream.eof()) exit(0);
      if(line[0] == 'R')
	dir = INCOMING;
      else if (line[0] == 'S')
	dir = OUTGOING;
      else if (line[0] == 'F')
	exit(0);

      int lleft  = line[6];
      int lright = line[7];
      int left   = line[8];
      int right  = line[9];
      
      int len = value(lleft,lright) * 256 + value(left,right) + 4;
      //      cout << " line has " << len << " bytes in length field and is " << strlen(line) << " bytes long"<<endl;
      int i;   // byte in output
      int j=2; // position on line
      for (i=0; i<len; i++) 
	{
	  if (line[j] == ' ') j++;
	  receive_buffer[i] = value(line[j], line[j+1]);
	  j +=2;
	}
#if 0
      cout << line << endl;
       {
 	cout << line[0];
 	int n;
 	char nibbles[3];
 	nibbles[2]=0;
 	for (n=0; n<len; n++) {
 	  sprintf(nibbles,"%02x",(unsigned int)receive_buffer[n]);
 	  if (n%4 ==0) cout << " ";
 	  cout << nibbles;
 	}
       }
       exit(1);
#endif
      receive_buffer_len = len;
      SnmpMessage snmpmsg;
      if(snmpmsg.load(receive_buffer, receive_buffer_len) != SNMP_CLASS_SUCCESS)
	cout << "return SNMP_CLASS_ERROR" << endl;

      OctetStr community_name;
      snmp_version version;
      if(snmpmsg.unload(pdu,community_name, version) != SNMP_CLASS_SUCCESS)
	cout << "return SNMP_CLASS_ERROR" << endl;

      Print(pdu,dir);
      if(pdu.get_error_status() != 0)
	cout << "pdu.get_error_status() = " << pdu.get_error_status() << endl;
    }
}
