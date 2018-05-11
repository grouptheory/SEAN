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
static char const _HostFile_cc_rcsid_[] =
"$Id: HostFile.cc,v 1.9 1998/08/20 22:02:40 talmage Exp $";
#endif
#include <common/cprototypes.h>

#include <iostream.h>
#include <fstream.h>
#include "Switch.h"
#include "Host.h"
#include "HostFile.h"
#include <codec/uni_ie/ie.h>
#include <FW/basics/Conduit.h>
#include <FW/basics/diag.h>
#include <pwd.h>

extern "C" {
#include <stdio.h>
}

// added to HostFile name to store (name to socket) table
static char *extension=".ports";

static char *expand_tilde(char *tilde_file_name);
#define HOST_TYPE 1
#define PORT_TYPE 2
int file_type(FILE * fd);

// constructor -- reads file and stores info in an array of Host_descriptors
// just to make life interesting filename is either a host file or a port file
//
HostFile::HostFile(char *file_name)
{
  // if the uses is running under /bin/sh the host_file_name
  // may contain ~ or ~user. If it does, we need to expand it
  // since the shell will not.
  //
  char *copyof_file_name = 0;
  if ( file_name[0] == '~' )
    copyof_file_name = expand_tilde(file_name);
  else
    copyof_file_name = file_name;
  
  _nfd = fopen(copyof_file_name, "r");

  if (_nfd == 0) {
    cout << "unable to fopen( " << copyof_file_name
	 << ",...) for reading"<< endl;
    exit(1);
  }
  
  if (file_type(_nfd) == HOST_TYPE) { // HOST_TYPE is local to HostFile.cc

    cout << "Reading Host Descriptors from " << copyof_file_name << endl;

    _hosts = count_hosts();
    
    if ( _hosts > 0 ) {
      _hd = new  Host_descriptor *[_hosts];
      parse_host_file (_hd);
      _host_file_name = new char[1+strlen(copyof_file_name)];
      strcpy(_host_file_name, copyof_file_name);
    }
  } else {  // PORT_TYPE
    _hosts = count_ports();
    if ( _hosts > 0 ) {
      _hd = new  Host_descriptor *[_hosts];
      parse_port_file (_hd);
      _port_file_name = new char[1+strlen(copyof_file_name)];
      strcpy(_port_file_name, file_name);
    }
  }    
  fclose(_nfd);
  _nfd = 0;
  _is_switch    = false;	// until make_switch is called
  _have_read_ports = false;	// until .port file read
}

HostFile::~HostFile() { delete [] _hd; } 

int HostFile::host_count() { return _hosts; }

// default file with service port numberts will be
// .<host_file>.<extension>
static char *get_socket_file_name (char * host_file)
{
  // copy the name
  char *host_file_name=new char[strlen(host_file)+1];
  strcpy (host_file_name, host_file);

  // the port file will be at most the original + extension + . + \0
  int len = strlen(host_file_name) + strlen(extension) + 2;
  char *socket_file_name = new char[len];
  socket_file_name[0]='\0';

  // break name into dirname and basename
  char *basename = strrchr(host_file_name,'/');
  char *dirname=0;
  if (basename==0) {
    basename=host_file_name;
  } else {
    basename[0] = '\0';
    basename++;
    dirname=host_file_name;
  }

  // construct the name
  if (dirname) {	// if there are any / in name insert . after last /
    strcat(socket_file_name, dirname);
    strcat(socket_file_name, "/.");
  } else {			// otherwise add . at the beginning
    strcat(socket_file_name,".");
  }
  strcat(socket_file_name, basename);
  strcat(socket_file_name, extension);


  delete [] host_file_name;
  return socket_file_name;
  
}


// we try several possible stategies to open a file to contain
// name + port + nsap lines

ofstream * get_port_ostream (char * host_file)
{
  // copy the name
  char *host_file_name=new char[strlen(host_file)+1];
  strcpy (host_file_name, host_file);

  // the port file will be at most the ../ + . + original + extension + \0
  int len = strlen(host_file_name) + strlen(extension) + 5;
  char *port_file_name = new char[len];
  port_file_name[0]='\0';

  // break name into dirname and basename
  char *basename = strrchr(host_file_name,'/');
  char *dirname=0;
  if (basename==0) {
    basename=host_file_name;
  } else {
    basename[0] = '\0';
    basename++;
    dirname=host_file_name;
  }

  // construct port_file_name = dirname/.basename.ports
  if (dirname) {	// if there are any / in name insert . after last /
    strcat(port_file_name, dirname);
    strcat(port_file_name, "/.");
  } else {			// otherwise add . at the beginning
    strcat(port_file_name,".");
  }
  strcat(port_file_name, basename);
  strcat(port_file_name, extension);
  ofstream *out = new ofstream(port_file_name);

  if (out->bad()) {
    out->close();
    delete out;
    // we can't open port_file_name for write, try ../.<host_file_name>.ports
    strcpy(port_file_name, "../.");
    strcat(port_file_name,basename);
    strcat(port_file_name, extension);
    out = new ofstream(port_file_name);
  }

  if (out->bad()) {
    out->close();
    delete out;
    // we can't open in  ../ for write, try ./.<host_file_name>
    strcpy(port_file_name, "./.");
    strcat(port_file_name,basename);
    strcat(port_file_name, extension);
    out = new ofstream(port_file_name);
  }

  if (out->bad()) {
    out->close();
    delete out;
    // we can't open ./ for, for write try ~/.<host_file_name>
    strcpy(port_file_name, "~/.");
    strcat(port_file_name,basename);
    strcat(port_file_name, extension);
    char *exfn = expand_tilde(port_file_name);
    out = new ofstream(exfn);
    if (out) {
      delete port_file_name;
      port_file_name = exfn;
    }
  }
  
  if (out) {
    cout << "Port File: " << port_file_name << endl;
  } else {
    cout << "unable to open output file to store host-port-nsap for apps\n";
  }
  delete [] host_file_name;
  delete [] port_file_name;
  return out;
}

// we try several possible stategies to open a file containing
// name + port + nsap lines

ifstream * get_port_istream (char * host_file)
{
  // copy the name
  char *host_file_name=new char[strlen(host_file)+1];
  strcpy (host_file_name, host_file);

  // the port file will be at most the ../ + . + original + extension + \0
  int len = strlen(host_file_name) + strlen(extension) + 5;
  char *port_file_name = new char[len];
  port_file_name[0]='\0';

  // break name into dirname and basename
  char *basename = strrchr(host_file_name,'/');
  char *dirname=0;
  if (basename==0) {
    basename=host_file_name;
  } else {
    basename[0] = '\0';
    basename++;
    dirname=host_file_name;
  }

  // construct port_file_name = dirname/.basename.ports
  if (dirname) {	// if there are any / in name insert . after last /
    strcat(port_file_name, dirname);
    strcat(port_file_name, "/.");
  } else {			// otherwise add . at the beginning
    strcat(port_file_name,".");
  }
  strcat(port_file_name, basename);
  strcat(port_file_name, extension);
  ifstream *in = new ifstream(port_file_name);

  if (in->bad()) {
    cout << "Unable to open " << port_file_name << endl;
    in->close();
    delete in;
    // we can't open port_file_name for read, try ../.<host_file_name>.ports
    strcpy(port_file_name, "../.");
    strcat(port_file_name,basename);
    strcat(port_file_name, extension);
    in = new ifstream(port_file_name);
  }

  if (in->bad()) {
    cout << "Unable to open " << port_file_name << endl;
    in->close();
    delete in;
    // we can't open in  ../ for read, try ./.<host_file_name>
    strcpy(port_file_name, "./.");
    strcat(port_file_name,basename);
    strcat(port_file_name, extension);
    in = new ifstream(port_file_name);
  }

  if (in->bad()) {
    cout << "Unable to open " << port_file_name << endl;
    in->close();
    delete in;
    // we can't open ./ for, for read try ~/.<host_file_name>
    strcpy(port_file_name, "~/.");
    strcat(port_file_name,basename);
    strcat(port_file_name, extension);
    char *efn = expand_tilde(port_file_name);
    in = new ifstream(efn);
    if (in) {
      delete port_file_name;
      port_file_name = efn;
    }
  }
  if (in) {
    cout << "Port File: " << port_file_name << endl;
  } else {
    cout << "unable to open output file to read port-host-nsap for apps\n";
  }
  delete [] host_file_name;
  delete [] port_file_name;
  return in;
}

//
// construct a sim switch
// with hosts using ipc ports in the range lport...lport + host_count()-1
// the channel numbers are stored in ._host_file_name.sockets
// in the form "host_name ipc_port nsap" one per line 
//
void HostFile::make_switch (int lport)
{
  
  Switch* sw;
  Conduit* swcon = make_Switch(_hosts, sw);

  NSAP_DCC_ICD_addr ** addr = new NSAP_DCC_ICD_addr * [_hosts];
  int socket_num = lport;
  int i;
  for (i=0; i<_hosts; i++) {
    addr[i] = new NSAP_DCC_ICD_addr(_hd[i]->Nsap());
    _hd[i]->_port = lport;
    Conduit* h = attach_Host(addr[i], swcon, sw, i, lport++);
    DIAG("daemon",DIAG_INFO,
	 cout << "switch port" << i <<"--> "
	 << _hd[i]->Nsap() <<" \""<< _hd[i]->Name() << "\" ipc-port= " << lport-1 << endl;);

  }

  // if we make it here the switch and hosts are made
  _is_switch = true;

  // write out the socket numbers
  ofstream *out = get_port_ostream (_host_file_name);
  //  ofstream out(port_file_name);
  
  if (out == 0) {
    cout << "Unable to open output port ostream for "<<_host_file_name << endl;
    exit (1);
  }

  for (i=0 ; i < _hosts; i++) {
    *out << socket_num++ <<" "<< _hd[i]->Name()<< " " << _hd[i]->Nsap() <<endl;
  }
  out->close();
  delete out;
}

//
// make a switch with hosts that use application files
// if the application file name was missing from the host_file
// the user is prompted for the app file name for each host
//
void HostFile::make_switch (void)
{
  
  Switch* sw;
  Conduit* swcon = make_Switch(_hosts, sw);

  NSAP_DCC_ICD_addr ** addr = new NSAP_DCC_ICD_addr * [_hosts];
  int i;
  char appin[255];

  for (i=0; i<_hosts; i++) {
    addr[i] = new NSAP_DCC_ICD_addr(_hd[i]->Nsap());
    char *app = _hd[i]->_apps;
    if (!app) {
      cout << "Exec file for "<<_hd[i]->Name()<<" : "; cin >> appin;
      app = new char[strlen(appin)+1];
      strcpy(app,appin);
    }
    Conduit* h = attach_Host(addr[i], swcon, sw, i, app);
    DIAG("daemon.app",DIAG_INFO,
	 cout << "host[" << i <<"] = "
	      << _hd[i]->Nsap() <<" "<< _hd[i]->Name() <<" "<< app << endl;);
  }
  // if we make it here the switch and hosts are made
  _is_switch = true;
}

// return nsap in "47..." string format
char * HostFile::get_nsap_byname(char *hostname)
{
  char *nsap=0;
  int i;
  for (i=0; i<_hosts; i++) {
    if (strcmp(hostname, _hd[i]->Name()) == 0) {
      nsap = new char[strlen(_hd[i]->Nsap())+1];
      strcpy(nsap, _hd[i]->Nsap());
      break;
    }
  }
  return nsap;
}

//
// find the signalling ipc port being used by an ipc simulation host
//
int HostFile::get_port_byname(char *host)
{
  int i, j;
  // on first call read file and establish data
  if (!_have_read_ports) {

    //    char *socket_file_name = get_socket_file_name (_host_file_name);
    ifstream *in = get_port_istream(_host_file_name);
    //    delete socket_file_name;
    
    for ( i=0; i < _hosts && in->good(); i++ ) {
      int lport = -1;
      char name[256], nsap[256];

      (*in) >> lport >> name >> nsap;

      for (j=0; j<_hosts ; j++) {
	if (strcmp(name, _hd[i]->Name()) == 0) {
	  _hd[i]->_port = lport;
	  break;
	}
      }
    }
    _have_read_ports=true;
    in->close();

    delete in;
  }

  // search for match
  for (i=0 ; i<_hosts; i++) {
    if (strcmp(host, _hd[i]->Name()) == 0) {
      return _hd[i]->_port;
    }
  }
  return 0;
}

// utility routine used in parsing the host file
#define isawhite(c) (*c == ' ' || *c == '\t' || *c == '\n') 
static char * skip_white(char *in)
{
     while (*in && isawhite(in) ) in++;
     return in;
}

// utility routine used in parsing the host file
int remove_comment(char * & input)
{
    char *comment=strchr(input,'#');
    if (comment)  comment[0]='\0';

    // skip leading white space
    input = skip_white(input);

    return strlen(input);
}

#define isadigit(c) ((c >='0' && c<='9'))
static int read_port(char * &n, char *line, int line_num)
{
  n = skip_white(n);
  int len = strlen(n);
  int port = 0;
  
  if (len == 0) {
    cout << "port is missing on line "<< line_num <<endl;
    cout << "last reading:" << line <<endl;
    exit(1);
  }

  while (isadigit(*n)) {
    port = port * 10 + (*n - '0');
    n++;
  }
  return port;
}
static char nsap[51];

// utility routine used to parse nsap in the host file

//
// the second item on a line is the NSAP
// the NSAP will be of the form:
//        0x47.0005.80.ffde00.0000.0000.0104.000000000000.00
// notes: for hex values [A-F]  are not allowed,
//        0x is required
//        0x47 and 0x39 are the only permissible starting values
//        the .'s are optional
//
static char *read_nsap(char * &n, char *line, int line_num)
{
  n = skip_white(n);
  int len = strlen(n);

  if (len == 0) {
    cout << "nsap and name missing on line "<< line_num <<endl;
    cout << "last reading:" << line <<endl;
    exit(1);
  }

  char *nsap_ptr=nsap;
  int nlen=0;   // number of characters in nsap -- including optional .'s 
  int i=0;      // number of hex values in nsap - must == 40

  if (n[0] == '0' && n[1] == 'x' ) n+=2;
  
  if (((n[0] == '4' && n[1] == '7') ||(n[0] == '3' && n[1] == '9'))) {
    bcopy(n, nsap_ptr, 2);
    nsap_ptr += 2;
    i = 2;
    nlen += 2;
    n+=2;
    
#define isnsap(c) ((c >='0' && c<='9') || (c>='a' && c<='f') || (c=='.'))
#define isahex(c) ((c >='0' && c<='9') || (c>='a' && c<='f') )
    while (isnsap (*n)) {	// hex or .
      if (isahex(*n)) {		// hex
	i++;
	*nsap_ptr++ = *n;
      }
      n++;
      nlen++;
    }
    
    if (i!=40) {
      cout << "invalid nsap while reading line " << line_num <<endl;
      cout << "your nsap contains "<<i<<" nibble values, 40 expected"<<endl;
      cout << "valid nsaps are of the form: "<< endl;
      cout << "    0x47.0005.80.ffde00.0000.0000.0104.000000000000.00"<<endl;
      cout << "last reading: "<< line <<endl;
      exit (1);
    }

    if ( *n != ' ' && *n != '\t' ) {
      cout << "invalid character in nsap on line " << line_num << endl;
      cout << "last reading: " << line << endl;
      exit(1);
    }
    
    *nsap_ptr = '\0';
  } else {			// doesn't begin with 0x47 or 0x39
    cout << "invalid nsap while reading line " << line_num <<endl;
    cout << "valid nsaps are of the form: "<< endl;
    cout << "     0x47.0005.80.ffde00.0000.0000.0104.000000000000.00" <<endl;
    cout << "last reading: "<< line <<endl;
    exit (1);
  }
  // nsap should now be 40 bytes : "470500..." 
  return nsap;
}

// utility routine used to parse the host file

#define isaname(c) ((c>='a'  &&  c<='z') || \
                    (c>='A'  &&  c<='Z') || \
		    (c>='0'  &&  c<='9') || \
		    (c=='_') || (c=='.') || \
		    (c=='+') || (c=='-') || \
		    (c=='@') || (c=='=') || \
		    (c=='/') || (c==',') || \
		    (c==':') || (c=='~')    \
		   )

static char host_name[1024];
// hostname can contain [a-zA-Z0-9_.]
static char *read_hostname (char * &n, char *line, int line_num)
{
  n = skip_white(n);
  int len = strlen(n);
  if (len == 0) {
    cout << "missing hostname while reading line " << line_num <<endl;
    cout << "last reading: "<< line <<endl;
    exit (1);
  }
  char *hn=host_name;

  while (isaname(*n)) *hn++ = *n++;
  *hn='\0';
  // n should now be pointing at white space or the end of the line
  if ( *n=='\0' || isawhite(n)) ;
  else {
    cout << "invalid host name while reading line " <<line_num<<endl;
    cout << "error reading host name - illegal character[ "
	 << *n << " = 0x"<< hex<<(int)*n<<dec <<" ]encountered" <<endl;
    cout << "only characters in the set [a-zA-Z0-9_.] are allowed" <<endl;
    cout << "last reading: "<< line <<endl;
    exit(1);
  }
  return host_name;
}

// utility routine used to parse the host file
// differs from read_hostname in that app_name is optional so there is no
//      error if it is missing.

static char app_name[1024];
static char *read_appname (char * &n, char *line, int line_num)
{
  n = skip_white(n);
  int len = strlen(n);
  if (len == 0) return 0;

  char *an=app_name;

  while (isaname(*n)) *an++ = *n++;
  *an='\0';
  
  // n should now be pointing at white space or the end of the line
  // app file names are optional but if present must be valid
  if ( *n=='\0' || isawhite(n)) ;
  else {
    cout << "invalid app file name while reading line " <<line_num<<endl;
    cout << "error reading app file name - illegal character[ "
	 << *n << " = 0x"<< hex<<(int)*n<<dec <<" ]encountered" <<endl;
    cout << "only characters in the set [a-zA-Z0-9_.+-@=/,] allowed" <<endl;
    cout << "last reading: "<< line <<endl;
    exit(1);
  }
  return app_name;
}
  
// network descriptor file valid input
// # anything beyond a # is discarded
// blank line
//
// # nsap                                    host_name [optional app file name]
//   0x47.0005.80.ffde00.0000.0000.0104.000000000000.00  foo app
//   0x47.0005.80.ffde00.0000.0000.0104.000000011111.00  bar

int HostFile:: count_hosts( )
{
  // this program counts the number of valid host lines in the file
  // invalid lines are noted and the program exits
  rewind(_nfd);
  int len;
  char line[1024];
  char *name=0;
  char *nsap=0;
  char *apps=0;
  int line_num=0;
  int host_count = 0;
  
  while (!feof(_nfd)) {
    line[0] = '\0';
    fgets(line, 1024, _nfd);
    line_num++;

    char *input = line;
    if (remove_comment(input) == 0) continue;

    nsap = read_nsap(input, line, line_num);
    
    name = read_hostname(input, line, line_num);

    apps = read_appname(input, line, line_num);

    host_count++;
  }
  return host_count;
}

int HostFile:: count_ports( )
{
  // this program counts the number of lines
  // no validity checking is done
  rewind(_nfd);
  char line[1024];
  int line_num=0;
  
  while (!feof(_nfd)) {
    fgets(line, 1024, _nfd);
    line_num++;
  }
  return line_num;
  
}

void HostFile::parse_host_file ( Host_descriptor ** hd)
{
  // this reads the network file descriptor file and fills in array of pointers
  // error chacking is assumed to have been done in count_hosts()
  rewind(_nfd);
  int len;
  char line[1024];
  char *name=0;
  char *nsap=0;
  char *apps=0;
  int line_num=0;
  int host_count = 0;
  
  while (!feof(_nfd)) {
    line[0] = '\0';
    fgets(line, 1024, _nfd);
    line_num++;
    char *input = line;
    if (remove_comment(input) == 0 ) continue;
    
    nsap = read_nsap(input, line, line_num);
    
    name = read_hostname(input, line, line_num);

    // the forth [optional] item is the name of the apps file
    apps = read_appname(input, line, line_num);

    hd[host_count] = new Host_descriptor();
    hd[host_count]->Set(nsap, name,apps);
    host_count++;
  }

}

void HostFile::parse_port_file ( Host_descriptor ** hd)
{
  // this reads the port file and fills in array of pointers
  // error chacking is assumed to have been done in count_ports()
  rewind(_nfd);
  int len;
  char line[1024];
  int port;
  char *name=0;
  char *nsap=0;
  char *apps=0;
  int line_num=0;
  int host_count = 0;
  
  while (!feof(_nfd)) {
    line[0] = '\0';
    fgets(line, 1024, _nfd);
    line_num++;
    char *input = line;

    port = read_port(input, line, line_num);
    
    name = read_hostname(input, line, line_num);

    nsap = read_nsap(input, line, line_num);

    hd[host_count] = new Host_descriptor();
    hd[host_count]->Set(nsap, name, 0);
    hd[host_count]->_port = port;
    host_count++;
  }
  _have_read_ports = true;
}

// expand file names beginning with ~
static char *expand_tilde(char *tilde_file_name)
{
  // two types
  // 1)  ~ or ~/ expand to $HOME
  // 2)  ~user or ~user/
  char *expanded_file_name = tilde_file_name;
  
  if (tilde_file_name[0] == '~' ) {
  
    if (tilde_file_name[1] == '/' || tilde_file_name[1] == '\0') { // raw ~
      char *home = getenv("HOME");

      if (home != 0) { 
	int len = strlen(home) + strlen(tilde_file_name);
	expanded_file_name = new char[len];
	strcpy(expanded_file_name, home);
	strcat(expanded_file_name, tilde_file_name+1);
      }

    } else { // ~user

      char *user_end = strchr(tilde_file_name,'/')-1;

      if (user_end == 0)
	user_end = tilde_file_name + strlen(tilde_file_name);

      int user_len = user_end - tilde_file_name;
      char *user = new char[user_len+1];
      user = strncpy(user,tilde_file_name+1,user_len);

      struct passwd *user_info = getpwnam(user);
      if (user_info->pw_dir) {
	int len = strlen(user_info->pw_dir) + strlen(tilde_file_name);
	expanded_file_name = new char[len];
	strcpy(expanded_file_name, user_info->pw_dir);
	strcat(expanded_file_name, tilde_file_name+user_len+1);
      }
    }

  }
  return expanded_file_name;
}

// what type is open on this fd
// rules: it is either PORT_TYPE or a HOST_TYPE
// PORT_TYPE files ALWAYS are of the form
// int name nsap
// where int is a integer
//       name is almost any string of characters
//       nsap is EXACTLY 40 hex characters
// the compoents are separated by exactly ONE space
//
int file_type(FILE * fd)
{
  rewind(fd);
  char line[1000];
  fgets(line, 1000, fd);
  rewind(fd);
  // now see if this is a valid port file line

  // 45 = 2 for int+name + 2 spaces  +40nsap+one\n
  if (strlen(line) < 45) return HOST_TYPE;

  char *n = line;
  // all characters before first space must be decimal integers

  while (*n && *n != ' ' && isadigit(*n) ) n++;
  if (*n != ' ') return HOST_TYPE;

  // it is either a port file or something else entirely
  return PORT_TYPE;
}
  
