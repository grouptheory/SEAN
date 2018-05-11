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

#ifndef __HostFile_H__
#define __HostFile_H__

#ifndef LINT
static char const _HostFile_h_rcsid_[] =
"$Id: HostFile.h,v 1.5 1999/03/25 20:03:20 marsh Exp $";
#endif
extern "C" {
#include <stdio.h>
}
class Host_descriptor;

class HostFile {
public:

  HostFile(char *filename);
  ~HostFile();

  void make_switch(void);            // apps switch
  void make_switch(int port_socket); // ipc switch

  char *get_nsap_byname(char *host);
  int get_port_byname (char *host);  

  int host_count();

private:
  FILE * _nfd;
  int _hosts;
  Host_descriptor **_hd;
  char *_host_file_name;
  char *_port_file_name;
  
  int count_hosts();
  int count_ports();
  void parse_host_file (Host_descriptor ** hd);
  void parse_port_file (Host_descriptor ** hd);

  bool _is_switch;         // true if make_switch called
  bool _have_read_ports;   // false until .ports file read

};

class Host_descriptor {
  friend class HostFile;
public:
  Host_descriptor() : _nsap(0), _name(0), _apps(0), _port(0) { }
  
  Set(char *nsap,char *name,char *apps) {
    _nsap=new char[strlen(nsap)+1];
    strcpy(_nsap,nsap);
    _name=new char[strlen(name)+1];
    strcpy(_name,name);
    if (apps) { // apps is optional
      _apps=new char[strlen(apps)+1];
      strcpy(_apps,apps);
    }
  }
  ~Host_descriptor() { delete _nsap, _name; }

  char *Name()   { return _name;   }
  char *Nsap()   { return _nsap;   }
  int   Port()   { return _port; }
  
private:
  //  int   _port;
  int   _port;
  char *_nsap;
  char *_name;
  char *_apps;
};

#endif
