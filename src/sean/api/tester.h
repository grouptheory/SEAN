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
#ifndef __TESTER_H__
#define __TESTER_H__

#include "Controllers.h"
#include <DS/containers/list.h>

class ATM_Interface;
class Incoming_ATM_Call;
class Outgoing_ATM_Call;
class ATM_Service;
class HostFile;

class Kontroller : public Controller {
public:
  Kontroller(char* hostfile, char* cmdfile);
  virtual ~Kontroller();

  void PeriodicCallback(int code);

  void Boot(ATM_Interface& interface);
  void Call_Active(ATM_Call& call);
  void Incoming_LIJ(Outgoing_ATM_Call& call);
  void Call_Inactive(ATM_Call& call);
  void Incoming_Data(ATM_Call& call);
  void Incoming_Call_From_Root(Leaf_Initiated_ATM_Call& lic);

  void Leaf_Active(class ATM_Leaf & leaf, class ATM_Call & c);
  void Leaf_Inactive(class ATM_Leaf & leaf, class ATM_Call & c);

  void Incoming_Call(ATM_Service& service);
  void Service_Active(ATM_Service& service);
  void Service_Inactive(ATM_Service& service);

protected:

  void ReadNextLine(void);
  void OpenFile(char* fname);

  void begin_wait(char* line);
  void end_wait(void);
  void register_service(char* line);
  void teardown(char* line);
  void send_data(char* line);
  void setup_call(char* line);
  void request_call(char* line);
  void add_leaf(char* line);
  void drop_leaf(char* line);
  void identity(char* line);

private:
  const int      MAX = 5;

  char           _id[80];
  Addr*          _myaddress;

  ATM_Interface* _interface;

  Leaf_Initiated_ATM_Call* _licall;
  bool _accept_root_call;

  Outgoing_ATM_Call*       _outcall;  
  char  _dest[80];
  char  _callid[80];
  bool  _teardown_lijs;

  ATM_Leaf*                _leaf[MAX];
  int _next_leaf;
  char _leafid[80];
  char _destleaf[80];

  ATM_Service*             _service;
  bool _reject_calls;
  Incoming_ATM_Call*       _incall;

  char _wait[80];
  ifstream * _commandfile;
  HostFile * _hostfile;
};

#endif
