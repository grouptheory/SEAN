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
static char const _tcp_ipc_cc_rcsid_[] =
"$Id: tcp_ipc.cc,v 1.8 1998/09/28 12:32:40 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include "tcp_ipc.h"
#include "IPC_Visitors.h"

#include <FW/basics/Visitor.h>
#include <FW/basics/Conduit.h>
#include <FW/basics/diag.h>
#include <FW/actors/Terminal.h>
#include <codec/q93b_msg/Buffer.h>
#include "async.h"

extern "C" {
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>       
#include <arpa/inet.h>

#ifdef __SunOS5__ 
#include <sys/filio.h>
#endif

int close(int fd);
};

/* maximum connection queue length */
#define QLEN               5
#define BUFSIZE         4096

#ifndef INADDR_NONE
#define INADDR_NONE     0xffffffff
#endif

//----------------------------------------------------------
tcp_listener::tcp_listener(Terminal * manager, int lis_portnumber) 
  : abstract_listener(new tcp_endpoint(lis_portnumber)), 
    InputHandler(manager, ::socket(PF_INET, SOCK_STREAM, _ppe->p_proto)),
    _manager(manager), _portnumber(lis_portnumber) 
{
  int maxloop=1;
  char * sockscan = getenv("SEAN_SOCKET_SCAN");
  if (sockscan) maxloop=1000;
    
  bool done=false;
  for (int off=0; ((off<=maxloop) && (!done)); off++) {

    int attempt_port = _portnumber = lis_portnumber+off;

    int s = GetFD();
    if (s < 0)
      diag("ipc.listener.tcp",DIAG_FATAL,"tcp_listener can't create socket");
  
    int one = 1;
    if (ioctl(s, FIONBIO, (char *)&one))
      diag("ipc.listener.tcp",DIAG_FATAL,"tcp_listener can't make socket nonblocking\n");
  
    struct sockaddr_in sin; /* an Internet endpoint address         */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    
    /* Map service name to port number */
    if ( (sin.sin_port = htons((u_short)attempt_port)) == 0 ) {
      cerr << "tcp_listener: can't get " << attempt_port << " service entry:";
      perror("");
      exit(1);
    }
  
    /* Bind the socket */
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
      if (off==maxloop)
	diag("ipc.listener.tcp",DIAG_FATAL,"tcp_listener bind error on port %d.", _portnumber);
    }
    else
      done = true;
  }
  
  if (sockscan) 
    diag("ipc.listener.tcp",DIAG_INFO,"tcp_listener is using port %d", _portnumber);
}

tcp_listener::~tcp_listener()
{
  int s = GetFD();
  close(s);
}

bool tcp_listener::start_listening(int max_queue)
{
  if (listen(GetFD(), max_queue) < 0)
    diag("ipc.listener.tcp",DIAG_FATAL,"tcp_listener listen error");
  return true;
}

abstract_connection * tcp_listener::get_incoming_call(void)
{
  int accepted_sock;
  int alen;
  struct sockaddr_in fsin;
  tcp_connection * tcon = 0;
  
  alen = sizeof(struct sockaddr_in);
  int sock = GetFD();

  if((accepted_sock = accept(sock, (struct sockaddr *)&fsin,&alen)) < 0)
    diag("ipc.listener.tcp",DIAG_ERROR,"tcp_listener bad error on accept");
  else {
    int one = 1;
    if (ioctl(sock, FIONBIO, (char *)&one))
      diag("ipc.listener.tcp",DIAG_FATAL,"tcp_listener can't make socket nonblocking\n");

    tcon = new tcp_connection(_manager, accepted_sock);
  }
  return tcon;
}

void tcp_listener::Callback(void) 
{
  abstract_connection* incoming = get_incoming_call();
  if (incoming) {
    abstract_local_id *alid = incoming->my_local_id();
    assert(TYPECHECK(alid,tcp_sockets));
    
    tcp_local_id* lid = (tcp_local_id*) alid;
    
    IPC_SignallingVisitor * v = 
      make_IPC_SV(IPC_SignallingVisitor::establish_by_peer,
		  lid,
		  incoming);
    _manager->Absorb(v);
  }
}

void tcp_listener::Print(ostream& os) const 
{
  os << "tcp_listener< port " << _portnumber << " >";
}


//----------------------------------------------------------
tcp_endpoint::tcp_endpoint(int portnumber, char* host) 
  : abstract_endpoint(abstract_ipc_mechanism::tcp_sockets), 
    _id(portnumber) 
{
  if (!host) strcpy(_host,"");
  else strncpy(_host,host,254);
}

tcp_endpoint::~tcp_endpoint() { }

int tcp_endpoint::compare(const abstract_endpoint * other) const 
{
  if (other->get_ipc_scheme_type() == this->get_ipc_scheme_type()) {
    if (((tcp_endpoint*)other)->portnumber() == this->portnumber()) {
      return (strcmp(((tcp_endpoint*)other)->host() , this->host()));
    }
    else if (((tcp_endpoint*)other)->portnumber() > this->portnumber())
      return 1;
    else return -1;
  } else if (other->get_ipc_scheme_type() > this->get_ipc_scheme_type()) 
    return 1;
  else return -1;
}

int tcp_endpoint::portnumber(void) const 
{
  return _id;
}

const char* tcp_endpoint::host(void) const 
{
  return _host;
}

void tcp_endpoint::Print(ostream& os) const 
{
  os << "tcp_endpoint< " << _host << "." << _id << " >";
}

//----------------------------------------------------------
tcp_local_id::tcp_local_id(int socketnumber) : 
  abstract_local_id(abstract_ipc_mechanism::tcp_sockets), _s(socketnumber) 
{ }

tcp_local_id::~tcp_local_id() { }

int tcp_local_id::compare(const abstract_local_id* other) const 
{
  if (other->get_ipc_scheme_type() == this->get_ipc_scheme_type()) {
    if (((tcp_local_id*)other)->socketnumber() == this->socketnumber()) {
      return 0;
    } else if (((tcp_local_id*)other)->socketnumber() > this->socketnumber())
      return 1;
    else return -1;
  } else if (other->get_ipc_scheme_type() > this->get_ipc_scheme_type())
    return 1;
  else return -1;
}

int tcp_local_id::socketnumber(void) const 
{
  return _s;
}

void tcp_local_id::Print(ostream& os) const 
{
  os << "tcp_local_id< " << _s << " >";
}

//----------------------------------------------------------
tcp_connection::tcp_connection(Terminal * manager, int sock)
  : abstract_connection(abstract_ipc_mechanism::tcp_sockets,
			new tcp_local_id(sock)), 
    InputHandler(manager, sock), _ignore(false), _first(true),
    _manager(manager), _sock(sock)
{
  set_state(abstract_connection::alive);
}

tcp_connection::~tcp_connection(void)
{
  int sock = GetFD();
  close(sock);
}

abstract_local_id* tcp_connection::my_local_id(void) 
{
  tcp_local_id* lid = new tcp_local_id(_sock);  
  return lid;
}

int tcp_connection::socket(void)
{
  return _sock;
}

abstract_connection::result tcp_connection::send_to(Buffer* buf)
{
  if (get_state() != abstract_connection::alive) 
    return abstract_connection::failure;
  else {
    int length = buf->length();
    buf->GrowHeader(4);
    bcopy(&length,(void*)( buf->data() ),4);
    
    if (::send(GetFD(),(const char *)buf->data(),buf->length(),0) == -1) {
      diag("ipc.connection.tcp",DIAG_ERROR,"tcp_connection error writing to socket %d\n",
	   GetFD());
      return abstract_connection::failure;
    }
  }
  return abstract_connection::success;
}

abstract_connection::result tcp_connection::recv_from(Buffer* buf, int& len)
{
  if (get_state() != abstract_connection::alive) 
    return abstract_connection::failure;

  int bytes;
  int sock = GetFD();
  int expected = len;

  bytes = ::recv(sock,(char *)(buf->data()), buf->length(), 0);
  if (bytes <= 0)  return abstract_connection::death;
  if (bytes < len) return abstract_connection::underflow;
  if (bytes > len) return abstract_connection::overflow;

  len = bytes;
  return abstract_connection::success;
}

abstract_connection::result tcp_connection::next_pdu_length(int & length) 
{
  if (get_state() != abstract_connection::alive) 
    return abstract_connection::failure;

  int bytes;
  int sock = GetFD();

  bytes = ::recv(sock,(char*) &length, 4, 0);
  if (bytes<=0) return abstract_connection::death;
  if (bytes<4) return abstract_connection::underflow;
  if (bytes>4) return abstract_connection::overflow;

  return abstract_connection::success;
}

void tcp_connection::process(void) {
  
  if (get_state() != abstract_connection::alive) 
    return;

  int bytes;
  int sock = GetFD();

  fd_set sense, ignore;
  FD_ZERO(&sense);
  FD_ZERO(&ignore);

  FD_SET(sock,&sense);
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  bytes = ::select(FD_SETSIZE,&sense,&ignore,&ignore,&tv);
  if (FD_ISSET(sock,&sense)) {
    Callback();
  }
}

void tcp_connection::Callback(void) 
{
  if (_ignore) return;

  result err1, err2;
  int msg_len;
  Buffer * buf;
  const unsigned char * payload;

  err1 = next_pdu_length(msg_len);

  switch (err1) {
  case success: 
    buf = new Buffer(msg_len);
    payload = buf->data(); 

    diag("ipc.connection.tcp",DIAG_DEBUG,"Message length is %d\n", msg_len);
    err2 = recv_from(buf, msg_len);
    switch (err2) {
      case success:
	DIAG("ipc.connection.tcp",DIAG_DEBUG, 
	     printf("tcp_connection ");
	     Print(cout);
	     int i;
	     printf(" recvd %d bytes of data:\n",msg_len);
	     for (i = 0; i < msg_len; i++) 
	       printf(" %02x",payload[i]); 
	     printf("\n");
	     );
  
	if (_first) {
	  _first = false;
	  if (!strcmp((const char *)buf->data(), "THISISTHESPECIALMESSAGEFROMGECKO")) {
	    _ignore = true;
	    // set up the diag to output to this socket ...
	    ofstream * ofs = new ofstream(GetFD());
	    DiagStream( *ofs );
	    // forget about the ofstream ...
	    //   so we leak a bit of memory.
	    return;
	  }
	}
	if (_manager) {
	  IPC_DataVisitor * v = IPC_DV_make_incoming(new tcp_local_id(_sock),buf);
	  _manager->Inject(v);
	} else {
	  diag("ipc.connection.tcp", DIAG_FATAL,
	       "tcp_ipc_connection recv'd %d bytes of unforwardable data.\n",msg_len);
	}
	break;
      case failure:
	diag("ipc.connection.tcp",DIAG_WARNING,"tcp_ipc_connection read failure (2).\n");
	break;
      case overflow:
	diag("ipc.connection.tcp",DIAG_FATAL,"tcp_ipc_connection read overflow (2).\n");
	break;
      case underflow:
	diag("ipc.connection.tcp",DIAG_FATAL,"tcp_ipc_connection read underflow (2).\n");
	break;
      case death:
        die();
	break;
      default:
	break;
    }
    break;
  case failure:
    diag("ipc.connection.tcp",DIAG_WARNING,"tcp_ipc_connection read failure (1).\n");
    break;
  case overflow:
    diag("ipc.connection.tcp",DIAG_FATAL,"tcp_ipc_connection read overflow (1).\n");
    break;
  case underflow:
    diag("ipc.connection.tcp",DIAG_FATAL,"tcp_ipc_connection read underflow (1).\n");
    break;
  case death:
    die();
    break;
  default:
    break;
  }
}

void tcp_connection::die(void) 
{
  set_state(abstract_connection::dead);
  diag("ipc.connection.tcp", DIAG_DEBUG, "tcp_ipc_connection death detected.\n");

  if (_manager) {
    tcp_local_id* lid = new tcp_local_id(_sock);
    IPC_SignallingVisitor * v = 
      make_IPC_SV(IPC_SignallingVisitor::release_by_peer, lid);
    _manager->Absorb(v);
  }
}

void tcp_connection::Print(ostream& os) const 
{
  os << "tcp_connection < sock=" << _sock << " >" << flush;
}

//----------------------------------------------------------
const VisitorType * tcp_ipc_layer::_ipc_data_vistype = 0;
const VisitorType * tcp_ipc_layer::_ipc_sig_vistype  = 0;

tcp_ipc_layer::tcp_ipc_layer(int lis_portnumber) : 
  abstract_ipc_layer(new tcp_endpoint(lis_portnumber)), Terminal() 
{
  if (lis_portnumber>0) {
    _lis = new tcp_listener(this, lis_portnumber);
    _lis->start_listening(5);
    Register(_lis);
  } else
    _lis = 0;

  if (!_ipc_data_vistype)
    _ipc_data_vistype = QueryRegistry(IPC_DATA_VISITOR_NAME);
  if (!_ipc_sig_vistype)
    _ipc_sig_vistype = QueryRegistry(IPC_SIGNALLING_VISITOR_NAME);
}

tcp_ipc_layer::~tcp_ipc_layer(void) { }

abstract_connection* tcp_ipc_layer::initiate_connection(abstract_endpoint* target_ep) 
{
  assert(TYPECHECK(target_ep,tcp_sockets));
  int target_portnumber = ((tcp_endpoint*)target_ep)->portnumber();
  const char* host = ((tcp_endpoint*)target_ep)->host();
  if (!host) host=getenv("HOST");

  char service[15];
  sprintf(service,"%d",target_portnumber);

  struct protoent *ppe;   /* protocol information entry */
  struct hostent  *phe;   /* pointer to host information entry     */
  struct sockaddr_in sin; /* an Internet endpoint address          */

  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  
  if ( (sin.sin_port = htons((u_short)target_portnumber)) == 0 )
    diag("ipc.layer.tcp",DIAG_FATAL,"tcp_ipc_layer can't locate service\n");
  
  /* Map host name to IP address, allowing for dotted decimal */
  if ( phe = gethostbyname(host) )
    bcopy(phe->h_addr, (char *)&sin.sin_addr, phe->h_length);
  else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
    diag("ipc.layer.tcp",DIAG_FATAL,"tcp_ipc_layer can't get host entry\n");

  if ( (ppe = getprotobyname("tcp")) == 0)
    diag("ipc.layer.tcp",DIAG_FATAL,"tcp_ipc_layer can't get tcp entry\n");
  
  int sock = ::socket(PF_INET, SOCK_STREAM, ppe->p_proto);
  
  if (sock < 0)
    diag("ipc.layer.tcp",DIAG_FATAL,"tcp_ipc_layer can't create socket\n");
  
  /* Connect the socket */
  if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) <0)
    diag("ipc.layer.tcp",DIAG_FATAL,"tcp_ipc_layer can't connect to %d\n",
	 target_portnumber);
      
  int one = 1;
  if(ioctl(sock, FIONBIO, (char *)&one))
    diag("ipc.layer.tcp",DIAG_FATAL,"tcp_ipc_layer can't make socket nonblocking\n");

  return new tcp_connection(this,sock);
}

void tcp_ipc_layer::Absorb(Visitor* v) 
{
  VisitorType vt = v->GetType();
  dic_item di;
  abstract_connection * con;
  tcp_connection * tc;

  if (vt.Is_A( _ipc_data_vistype )) {
    IPC_DataVisitor* iv = (IPC_DataVisitor*)v;
    if (iv->direction() == IPC_DataVisitor::outgoing) {
      int length = iv->get_length();
      Buffer* packet_buffer = iv->take_packet_and_zero_length();
      const unsigned char* packet = packet_buffer->data();
      diag("ipc.layer.tcp",DIAG_DEBUG,"tcp-layer got a packet of size %d to send out:\n",length);
      DIAG("ipc.layer.tcp",DIAG_DEBUG, 
	   { int i;
	   for (i=0;i<length;i++) printf(" %02x",packet[i]); 
	   printf("\n");
	   });
      
      const abstract_local_id* lid = iv->share_local_id();
      assert(TYPECHECK(lid,tcp_sockets));

      con = lookup_connection(lid);
      if (con) {
	assert(TYPECHECK(con,tcp_sockets));

	tc = (tcp_connection*) con;
	if (tc) {
	  diag("ipc.layer.tcp",DIAG_DEBUG,"tcp_ipc_layer sending data...\n");
	  abstract_connection::result res;
	  res = tc->send_to(packet_buffer);
	  print_result(res);
	}
      }
      else DIAG("ipc.layer.tcp",DIAG_WARNING,
		printf("tcp_ipc_layer sees req to send on unbound ");
		lid->Print(cout);
		printf("\n"););
    }
    iv->Suicide();
  } else if (vt.Is_A( _ipc_sig_vistype )) {
    IPC_SignallingVisitor* sv = (IPC_SignallingVisitor*)v;

    const abstract_local_id* lid = 0;
    const tcp_local_id* tcp_lid = 0;
    abstract_connection* ac = 0;
    tcp_connection* tc = 0;
    abstract_endpoint* ep = 0;
    abstract_local_id** id_pp = 0;
    SimEvent* ack = 0;
    int sock = -1;
    dic_item di;

    switch (sv->get_IPC_opcode()) {

    case IPC_SignallingVisitor::establish_by_peer_op:
      diag("ipc.layer.tcp",DIAG_DEBUG,"tcp_ipc_layer receives incoming connection.\n");

      ac = IPC_SV_take_connection(sv);
      if (!ac) {
	v->Suicide();
	return;
      }
      assert(TYPECHECK(ac,tcp_sockets));
      tc = (tcp_connection*)ac;

      lid = sv->share_local_id();
      assert(TYPECHECK(lid,tcp_sockets));
      tcp_lid = (tcp_local_id*)lid;

      DIAG("ipc.layer.tcp",DIAG_DEBUG, 
	   { printf("tcp_ipc_layer binding (incoming) ");
	   tc->Print(cout);
	   printf("\n");} );

      bind_connection(lid,tc);
      Register(tc);

      Inject(sv);
      break;

    case IPC_SignallingVisitor::establish_by_local_op:
      diag("ipc.layer.tcp",DIAG_DEBUG,"tcp_ipc_layer gets local connect request.\n");

      ep = IPC_SV_take_destination_ep(sv);
      assert(TYPECHECK(ep,tcp_sockets));

      ac = initiate_connection(ep);
      assert(TYPECHECK(ac,tcp_sockets));
      tc = (tcp_connection*)ac;

      id_pp = IPC_SV_take_id_pp(sv);
      ack = IPC_SV_take_simevent(sv);

      if (id_pp) {
	if ((ac) && (ac->get_state() == abstract_connection::alive)) {
	  abstract_local_id * alid = tc->my_local_id();
	  assert(TYPECHECK(alid,tcp_sockets));
	  tcp_local_id* tcp_lid = (tcp_local_id*) alid;

	  *id_pp = tcp_lid;
	  
	  DIAG("ipc.layer.tcp",DIAG_DEBUG, 
	       { printf("tcp_ipc_layer binding (outgoing) ");
	       tc->Print(cout);
	       printf("\n");} );

	  bind_connection(tcp_lid,tc);
	  bool succ = Register(tc);

	  if (ack && succ) {
	    asynchronous_manager* am = AsynchronousManager();
	    if (am) {
	      diag("ipc",DIAG_ERROR, "*******     !!!! WARNING !!!!     ******* ");
	      diag("ipc",DIAG_ERROR, "*******  ENTERING UNTESTED MODE   ******* ");
	      diag("ipc",DIAG_ERROR, "*******    'SEAN_ASYNC_MODE'      ******* ");
	      am->Register_Async_Handler(tcp_lid,tc);
	    }

	    ReturnToSenderSynchronous(ack);
	  }
	}
	else {
	  *id_pp = 0;
	}
      }

      sv->Suicide();
      break;

    case IPC_SignallingVisitor::release_by_peer_op:
      diag("ipc.layer.tcp",DIAG_DEBUG,"tcp_ipc_layer sees connection hangup by peer.\n");

      lid = sv->share_local_id();
      assert(TYPECHECK(lid,tcp_sockets));

      ac = lookup_connection(lid);
      assert(TYPECHECK(ac,tcp_sockets));
      tc = (tcp_connection*) ac;

      if (tc) {
	Cancel(tc);
	unbind_connection(lid);
	DIAG("ipc.layer.tcp",DIAG_DEBUG, 
	     { printf("tcp_ipc_layer unbinding ");
	     tc->Print(cout);
	     printf("\n");} );
	tc->Suicide();

	Inject(sv);
      }
      else { v->Suicide(); return; }

      break;

    case IPC_SignallingVisitor::release_by_local_op:
      diag("ipc.layer.tcp",DIAG_DEBUG,"tcp_ipc_layer gets local request for hangup.\n");

      lid = sv->share_local_id();
      assert(TYPECHECK(lid,tcp_sockets));

      ac = lookup_connection(lid);
      assert(TYPECHECK(ac,tcp_sockets));
      tc = (tcp_connection*) ac;

      if (tc) {
	Cancel(tc);
	unbind_connection(lid);
	DIAG("ipc.layer.tcp",DIAG_DEBUG, 
	     { printf("tcp_ipc_layer unbinding ");
	     tc->Print(cout);
	     printf("\n");} );
	tc->Suicide();
      }
      else { v->Suicide(); return; }

      sv->Suicide();
      break;

    default:
      break;
    }
  }
  else v->Suicide();
}

void tcp_ipc_layer::Interrupt(SimEvent* e) { }

void tcp_ipc_layer::Print(ostream& os) const 
{
  os << "tcp_layer< >";
}



