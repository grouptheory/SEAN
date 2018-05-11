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

// -*- C++ -*-
#ifndef LINT
static char const _ilmid_cc_rcsid_[] =
"$Id: ilmid.cc,v 1.20 1999/04/02 22:49:51 marsh Exp marsh $";
#endif
#include <common/cprototypes.h>
#include <sean/daemon/ccd_config.h>

#define PORT_MUX_NAME     "PortMux"

#include <iostream.h>
#include <FW/basics/Conduit.h>
#include <FW/behaviors/Adapter.h>
#include <FW/behaviors/Protocol.h>
#include <FW/basics/Visitor.h>
#include <FW/kernel/Kernel.h>
#include <FW/basics/diag.h>
#include <FW/basics/FW.h>

#include <snmp_pp.h>

#include "ilmiVisitors.h"
#include "ilmiTimers.h"
#include "ilmiTerm.h"
#include "ilmi.h"

extern "C" {
extern int toupper(char);
}
// prototype for function in xti_nsap.cc
int get_esi (unsigned char *esi); 

int main(int argc, char** argv)
{
  int pid,opt,bg=0;
  extern char *optarg;
  int esi_set=0;
  int  ip_set=0;
  int  debug_set=0;
  char *have_esi;
  char *host=0;
  int host_set=0;
  char filename[80];
  char IpAddr[20];
  char *e = 0L;
  
  while((opt = getopt(argc, argv, "bdh:e:")) != EOF)
    switch (opt)
      {
      case 'd': // debug mode
	debug_set = 1;
        break;

      case 'h': // host name or IP address
	ip_set = 1;
	strcpy(IpAddr,optarg);
	break;

      case 'b': // background mode
        bg = 1;
        break;

      case 'e': // ESI 
	esi_set=1;
	e = optarg;
        break;

      default:
        printf("%s\n",argv[0]);
      }

  if(debug_set == 1)
    {
      DiagLevel("ilmi", DIAG_DEBUG);
      cerr << "Enter the name of the visualizer pipe: ";
      cin >> filename;
      VisPipe(filename);
    }
  else
    VisPipe("/dev/null");

  unsigned char ESIaddr[7];

  if (esi_set==0) {
    if(get_esi(ESIaddr)==0)
      esi_set=2;
  }
  if (esi_set==0 || ip_set == 0)
    {
      printf("Usage: %s [ -e ESI ] -h HostName/IpAddr\n",argv[0]);
      printf("Usage: -e ESI -- not neeeded if FORE ilmi killed after boot\n");
      printf("Usage: you can get ESI with /opt/FOREatm/bin/adinfo\n");
      printf("          -e 00:00:00:00:00:00 -h hostname (esi from /opt/FOREatm/bin/adinfo)\n");
      exit(1);
    }
  // Make an ESI
  if (esi_set == 1) { // got esi from command line -- this always wins
    for(int i=0; i<6; i++, e+=3) 
      {
	ESIaddr[i] =
	  (e[0] <= '9' ? e[0]-'0' : toupper(e[0]) -'A'+10) << 4 |
	  (e[1] <= '9' ? e[1]-'0' : toupper(e[1]) -'A'+10);
      }
  }
  ESIaddr[6]=0;
  printf("ESIaddr = %02x%02x%02x%02x%02x%02x:%02x\n",
         ESIaddr[0],ESIaddr[1],ESIaddr[2],
	 ESIaddr[3],ESIaddr[4],ESIaddr[5],ESIaddr[6]);
  
  // Make a SNMP++ Ip address & check validity of address
  IpAddress ipa(IpAddr);
  if(!ipa.valid())
    {
      cout << "Invalid Address or DNS Name, " << IpAddr << "\n";
      exit(1);
    }
  cout << "IpAddress = " << ipa.get_printable() << endl;
  if(bg)
    {
      pid = fork();
      if(pid < 0)
        {
          exit(1);
        }
      else if(pid > 0)
        exit(0);
    }
  int status;
  OctetStr community("ILMI");                        
  ilmi * i = new ilmi(status,ESIaddr,ipa,community);
  Protocol * ip = new Protocol(i);
  Conduit *ic = new Conduit("ilmi", ip);

  ilmiTerm  * top = new ilmiTerm();
  Adapter   * atop = new Adapter(top);
  Conduit   * ctop = new Conduit("ILMI-TERM-TOP", atop);

  ilmiTerm  * low = new ilmiTerm(0,16);
  Adapter   * alow = new Adapter(low);
  Conduit   * clow = new Conduit("ILMI-TERM-LOW", alow);


  Join(A_half(ctop), A_half(ic));
  Join(B_half(ic), A_half(clow));

  Kernel & controller = theKernel();
  controller.SetSpeed((Kernel::Speed)0);
  controller.Run();
  return 0;
}
