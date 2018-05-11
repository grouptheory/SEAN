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
static char const _gecko_cc_rcsid_[] =
"$Id: gecko.cc,v 1.3 1998/08/06 18:38:10 marsh Exp $";
#endif
#include <common/cprototypes.h>
// -*- C++ -*-
#include <iostream.h>
#include <FW/kernel/Kernel.h>
#include <FW/kernel/KDB.h>

extern "C" {
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>       
#include <arpa/inet.h>
};

class EchoHandler : public InputHandler {
public:

  EchoHandler(SimEntity * se, int fd) :
    InputHandler(se, fd)
  {
    Register(this);

    SendInitMsg();
  }

  virtual ~EchoHandler( ) { close(GetFD()); }

  // Echo whatever is read
  void Callback(void)
  {
    char buf[1024];
    bzero(buf, 1024);

    if (read(GetFD(), buf, 1024) < 0) {
      perror("Unable to read from the socket, but the kernel said I could ... ");
      // fookers
    } else
      cout << buf;

    if (!strcmp(buf, "QUIT, QUIT, NOW YOU BASTARD")) {
      cout << "Received stop message, exiting ..." << endl;
      theKernel().Stop();
    }
  }

  // Send the initialization message to the CCD
  void SendInitMsg(void) 
  {
    char buf[64];
    sprintf(buf, "0000THISISTHESPECIALMESSAGEFROMGECKO");
    int len = strlen(buf) - 4 + 1;
    buf[0] = ((len & 0xFF000000) >> 24);
    buf[1] = ((len & 0x00FF0000) >> 16);
    buf[2] = ((len & 0x0000FF00) >> 8);
    buf[3] = (len  & 0x000000FF);

    int wlen = -1;
    if ((wlen = write(GetFD(), buf, len + 4)) <= 0) {
      perror("Unable to send start up message: ");
      exit(1);
    } else
      cout << "Wrote " << wlen << " bytes.  Len = " << len 
	   << " b[0-4] " << (int)buf[0] << " " << (int)buf[1] 
	   << " " << (int)buf[2] << " " << (int)buf[3] << endl;
  }
};


void main(int argc, char ** argv)
{
  if (argc < 3) {
    cout << "usage: " << argv[0] << " hostname port" << endl;
    exit(1);
  }
  const char * host = argv[1];
  int portno = atoi(argv[2]);

  int sock_fd = -1;
  struct protoent * proto;
  // get the protocol thingy for the socket call
  if ((proto = getprotobyname("tcp")) == 0) {
    perror("Unable to obtain the protocol: ");
    exit(1);
  }
  // Obtain the socket descriptor
  if ((sock_fd = socket(AF_INET, SOCK_STREAM, proto->p_proto)) < 0) {
    perror("Unable to obtain socket descriptor: ");
    exit(1);
  }

  struct sockaddr_in sadder;
  bzero((char *)&sadder, sizeof(sadder));
  sadder.sin_family = AF_INET;
  // Obtain the INET address of the destination host
  struct hostent * hostaddr;
  if (hostaddr = gethostbyname(host))
    bcopy(hostaddr->h_addr, (char *)&sadder.sin_addr, hostaddr->h_length);
  else if ((sadder.sin_addr.s_addr = inet_addr(host)) < 0) {
    perror("Unable to find the host you requested: ");
    exit(1);
  }
  // put the host is network byte order
  if ((sadder.sin_port = htons((u_short)portno)) == 0) {
    perror("Unable to translate the port: ");
    exit(1);
  }

  // Connect it to the remote host:port
  if (connect(sock_fd, (struct sockaddr *)&sadder, sizeof(sadder)) < 0) {
    perror("Unable to connect the socket ... ");
    exit(1);
  }
  cout << "The gecko is listening ..." << endl;

  Kernel & kontrol = theKernel();
  Debugger * kdb   = kontrol.KDB();

  EchoHandler * eh = new EchoHandler(kdb, sock_fd);
  // Sit back and relax ...
  kontrol.Run();
  // clean up the handler
  delete eh;
}
