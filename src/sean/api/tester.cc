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
static char const _linktst_cc_rcsid_[] =
"$Id: tester.cc,v 1.1 1998/09/28 12:31:16 bilal Exp $";
#endif
#include <common/cprototypes.h>

#include <FW/basics/Conduit.h>
#include <FW/basics/diag.h>
#include <codec/uni_ie/ie.h>

#include "ATM_Interface.h"
#include "Signalling_API.h"

#include "Controllers.h"
#include "tester.h"

#include <sean/daemon/HostFile.h>

#define STEP   0.0
#define BOGUS "0x47.0005.80.ffde00.0000.0000.0104.000000006666.00"
#define DELAYED "delayed"


void Kontroller::ReadNextLine(void) {
  char line[1024];
  _commandfile->getline(line, 1024);

  switch (line[0]) {
  case '#': // comment
    cout << _id << ": " << "*** !!! " << line+1 << endl;
    break;
  case 'i': // identity
    identity(line);
    break;
  case 'w': // wait
    begin_wait(line);
    break;
  case 's':                  // register service that rejects calls
    _reject_calls=true;
    register_service(line);
    break;
  case 'S':                  // register service that accepts calls
    _reject_calls=false;
    register_service(line);
    break;
  case 't':                  // teardown call
    teardown(line);
    break;
  case '*':                  // send data
    send_data(line);
    break;
  case 'c':                  // make a call that rejects LIJs
    _teardown_lijs = true;
    setup_call(line);
    break;
  case 'C':                  // make a call that accepts LIJs
    _teardown_lijs = false;
    setup_call(line);
    break;
  case 'a':                  // add a party
    add_leaf(line);
    break;
  case 'd':                  // drop a party
    drop_leaf(line);
    break;
  case 'R':                  // request a call, then accept root
    _accept_root_call = true;
    request_call(line);
    break;
  case 'r':                  // request a call, then reject root
    _accept_root_call = false;
    request_call(line);
    break;
  case 'x':                  // exit
    exit(0);
    break;
  default:
    break;
  }
  
}

//---------------------------------------------------------------------------
void Kontroller::begin_wait(char* line) {
  char ignore[80];
    sscanf(line,"%s %s",ignore, _wait);
    double t = atof(_wait);
    cout << "*** Kontroller: " << _id << ": " << " begins wait for " << t << " seconds.\n";
    CancelPeriodicCallback(1);
    RegisterPeriodicCallback(2,t);
}

void Kontroller::end_wait(void) {
    cout << "*** Kontroller: " << _id << ": " << " ends the wait\n";
    CancelPeriodicCallback(2);
    RegisterPeriodicCallback(1,STEP);
}

//---------------------------------------------------------------------------
void Kontroller::register_service(char* line) {
  cout << "*** Kontroller: " << _id << ": " << " registering service: `" << line << "'\n";
  _service = new ATM_Service(*_interface);
  ATM_Attributes::result err;
  assert((err = _service->Set_td_BE(1000,1000)) == ATM_Attributes::success);
  assert((err = _service->Set_bbc(ie_bbc::BCOB_X,
				  ie_bbc::not_clipped,
				  ie_bbc::p2mp,
				  10)) == ATM_Attributes::success);
  assert((err = _service->Set_called_party_num( _myaddress )) == ATM_Attributes::success);
  _service->Associate_Controller(*this);
  assert((err = _service->Register()) == ATM_Attributes::success);
}

//---------------------------------------------------------------------------
void Kontroller::teardown(char* line) {
  ATM_Attributes::result err;
  if (_incall) {
    cout << "*** Kontroller: " << _id << ": " << " tearing down incall: `" << line << "'\n";
    assert((err = _incall->Set_cause( ie_cause::normal_call_clearing )) == ATM_Attributes::success);
    assert((err = _incall->TearDown()) == ATM_Attributes::success);
    _incall=0;
  }
  if (_outcall) {
    cout << "*** Kontroller: " << _id << ": " << " tearing down outcall: `" << line << "'\n";
    assert((err = _outcall->Set_cause( ie_cause::normal_call_clearing )) == ATM_Attributes::success);
    assert((err = _outcall->TearDown()) == ATM_Attributes::success);
    _incall=0;
  }
  if (_licall) {
    cout << "*** Kontroller: " << _id << ": " << " tearing down licall: `" << line << "'\n";
    assert((err = _licall->Set_cause( ie_cause::normal_call_clearing )) == ATM_Attributes::success);
    assert((err = _licall->TearDown()) == ATM_Attributes::success);
    _licall=0;
  }
}

//---------------------------------------------------------------------------
void Kontroller::send_data(char* line) {
  if (_incall) {
    cout << "*** Kontroller: " << _id << ": " << " sending data on incall: `" << line << "'\n";
    u_char buf[10];
    for (int i=0;i<10;i++) buf[i]=i;
    cout << "*** " << _id << ": " << " Sent to incall: " << (_incall->SendData(buf,10)) << " bytes.\n";
  }
  if (_outcall) {
    cout << "*** Kontroller: " << _id << ": " << " sending data on outcall: `" << line << "'\n";
    u_char buf[10];
    for (int i=0;i<10;i++) buf[i]=i;
    cout << "*** " << _id << ": " << " Sent to outcall: " << (_outcall->SendData(buf,10)) << " bytes.\n";
  }
  if (_licall) {
    cout << "*** Kontroller: " << _id << ": " << " sending data on licall: `" << line << "'\n";
    u_char buf[10];
    for (int i=0;i<10;i++) buf[i]=i;
    cout << "*** " << _id << ": " << " Sent to licall: " << (_licall->SendData(buf,10)) << " bytes.\n";
  }
}

//---------------------------------------------------------------------------
void Kontroller::setup_call(char* line) {
  cout << "*** Kontroller: " << _id << ": " << " setting up call: `" << line << "'\n";
  char ignore[80];
  sscanf(line,"%s %s %s",ignore, _dest, _callid);
  _outcall = new Outgoing_ATM_Call(*_interface);
  _outcall->Associate_Controller(*this);
  ATM_Attributes::result err;

  assert((err = _outcall->Set_td_BE(1000,1000)) == ATM_Attributes::success);
  assert((err = _outcall->Set_bbc(ie_bbc::BCOB_X,
				  ie_bbc::not_clipped,
				  ie_bbc::p2mp,
				  10)) == ATM_Attributes::success);
  if (strcmp(_dest,"bogus")==0)
    assert((err = _outcall->Set_called_party_num( newAddr(BOGUS) )) == ATM_Attributes::success);
  else if (strcmp(_dest,DELAYED) != 0)
    assert((err = _outcall->Set_called_party_num( newAddr(_hostfile->get_nsap_byname(_dest)) )) == ATM_Attributes::success);

  assert((err = _outcall->Set_qos_param(ie_qos_param::qos0,
				     ie_qos_param::qos0)) == ATM_Attributes::success);
  assert((err = _outcall->Set_LIJ_call_id( atoi(_callid) )) == ATM_Attributes::success);
      
  _outcall->Associate_Controller(*this);
  assert((err = _outcall->Establish()) == ATM_Attributes::success);
}

//---------------------------------------------------------------------------
void Kontroller::request_call(char* line) {
  cout << "*** Kontroller: " << _id << ": " << " requesting call: `" << line << "'\n";
  char ignore[80];
  sscanf(line,"%s %s %s",ignore, _dest, _callid);
  _licall = new Leaf_Initiated_ATM_Call(*_interface);
  _licall->Associate_Controller(*this);
  ATM_Attributes::result err;

  assert((err = _licall->Set_calling_party_num(_myaddress,
					     ie_calling_party_num::presentation_allowed,
					     ie_calling_party_num::user_provided_not_screened)) == ATM_Attributes::success);
  if (strcmp(_dest,"bogus")==0)
    assert((err = _licall->Set_called_party_num( newAddr(BOGUS) )) == ATM_Attributes::success);
  else if (strcmp(_dest,DELAYED) != 0)
      assert((err = _licall->Set_called_party_num( newAddr(_hostfile->get_nsap_byname(_dest)) )) == ATM_Attributes::success);
  assert((err = _licall->Set_LIJ_call_id( atoi(_callid) )) == ATM_Attributes::success);
      
  _licall->Associate_Controller(*this);
  assert((err = _licall->Request()) == ATM_Attributes::success);
}

//---------------------------------------------------------------------------
void Kontroller::add_leaf(char* line) {
  char ignore[80];
  sscanf(line,"%s %s",ignore, _destleaf);
  int id = _next_leaf;
  _next_leaf++;

  _leaf[id] = Make_Leaf(*_outcall);
  assert( _leaf[id] );
  ATM_Attributes::result err;
      
  if (strcmp(_destleaf,"bogus")==0)
    assert((err = _leaf[id]->Set_called_party_num( newAddr(BOGUS) )) == ATM_Attributes::success);
  else if (strcmp(_destleaf,DELAYED) != 0)
    assert((err = _leaf[id]->Set_called_party_num( newAddr( _hostfile->get_nsap_byname(_destleaf)) )) == ATM_Attributes::success);

  _leaf[id]->Associate_Controller(*this);
      
  assert((err = _leaf[id]->Add()) == ATM_Attributes::success);
}

//---------------------------------------------------------------------------
void Kontroller::drop_leaf(char* line) {
  char ignore[80];
  sscanf(line,"%s %s",ignore, _leafid);
  ATM_Attributes::result err;
  int id = atoi(_leafid);

  assert((err = _leaf[id]->Set_cause( ie_cause::normal_call_clearing )) == ATM_Attributes::success);
  assert((err = _leaf[id]->Drop()) == ATM_Attributes::success);
  _leaf[id]=0;
};

//---------------------------------------------------------------------------
void Kontroller::OpenFile(char* fname) {
  ifstream * commandfile = new ifstream(fname);
  if (!commandfile || commandfile->bad()) {
    cerr << "ERROR: Unable to open '" << fname << "'" << endl;
    exit(0);
  }
  _commandfile = commandfile;
}

//---------------------------------------------------------------------------
void Kontroller::identity(char* line) {
  char ignore[80];
  sscanf(line,"%s %s",ignore,_id);
  _myaddress = newAddr( _hostfile->get_nsap_byname(_id) );
  cout << "*** my identity is " << _id << " ==> " << _hostfile->get_nsap_byname(_id) << endl;

  _interface = & ( Open_ATM_Interface("/dev/sp0", _hostfile->get_port_byname(_id)) );
  _interface->Associate_Controller(*this);

  char log[80];
  sprintf(log,"__%s.output",_id);
  VisPipe(log);
}

//---------------------------------------------------------------------------
Kontroller::Kontroller(char* hostfile, char* cmdfile) {
  _hostfile = new HostFile(hostfile);
  _myaddress = 0;
  _interface = 0;

  _licall = 0;
    
  _outcall = 0;
  strcpy(_dest,"");
  strcpy(_callid,"");
  _teardown_lijs = false;

  for (int i=0;i<MAX;i++)
    _leaf[i]=0;
  _next_leaf = 0;
  strcpy(_leafid,"");
  strcpy(_destleaf,"");
  
  _service = 0;
  _reject_calls = false;
    
  _incall = 0;

  strcpy(_wait,"");
  OpenFile(cmdfile);

  RegisterPeriodicCallback(1,STEP);
}

Kontroller::~Kontroller() { }

void Kontroller::PeriodicCallback(int code) {
  if (code==2) {
    end_wait();
  }
  else {
    if (!_commandfile->eof())
      ReadNextLine();
    else {
      cout << _id << ": " << "*** EOF\n";
      exit(0);
    }
  }
}

void Kontroller::Service_Active(ATM_Service& service) {
  cout << "*** Kontroller " << _id << ": Service Active\n";
}

void Kontroller::Service_Inactive(ATM_Service& service) {
  cout << "*** Kontroller " << _id << ": Service Inactive\n";
}

void Kontroller::Incoming_Call(ATM_Service& service) {
  cout << "*** Kontroller " << _id << ": Incoming Call\n";
  Incoming_ATM_Call* c = service.Get_Incoming_Call();
  if (c) {
    _incall = c;
    c->Associate_Controller(*this);
    if (_reject_calls) {
      cout << "*** " << _id << ": " << " Rejecting call id=" << c->Get_Call_ID() << endl;
      c->TearDown();
    }
    else {
      cout << "*** " << _id << ": " << " Accepting call id=" << c->Get_Call_ID() << endl;
      c->Accept();
    }
  }
}

void Kontroller::Boot(ATM_Interface& interface) {
  cout << "*** " << _id << ": " << " Interface has booted" << endl;
}

void Kontroller::Call_Active(ATM_Call& call) {
  cout << "*** Kontroller: " << _id << ": " << " Call is active\n";
  if (&call==_outcall) {
    _leaf[0] = _outcall->Get_Initial_Leaf();
    _next_leaf=1;
  }
}

void Kontroller::Incoming_LIJ(Outgoing_ATM_Call& call) {
  cout << "*** Kontroller: " << _id << ": " << " Incoming LIJ\n";
  ATM_Leaf* leaf = call.Get_Leaf_Join_Request();
  assert(leaf);
  ATM_Attributes::result err;
  if (_teardown_lijs) {
    cout << "*** " << _id << ": " << " Rejecting LIJ request\n";
    assert((err = leaf->Drop()) == ATM_Attributes::success);
  }
  else {
    cout << "*** " << _id << ": " << " Accepting LIJ request\n";
    assert((err = leaf->Add()) == ATM_Attributes::success);
  }
}

void Kontroller::Call_Inactive(ATM_Call& call) {
  cout << "*** Kontroller: " << _id << ": " << " Call is Inactive\n";
}

void Kontroller::Incoming_Data(ATM_Call& call) {
  cout << "*** Kontroller: " << _id << ": " << " Incoming Data\n";
  call.Drain();
}

void Kontroller::Leaf_Active(class ATM_Leaf & leaf, class ATM_Call & c) {
  cout << "*** Kontroller: " << _id << ": " << " Leaf Active\n";
}

void Kontroller::Leaf_Inactive(class ATM_Leaf & leaf, class ATM_Call & c) {
  cout << "*** Kontroller: " << _id << ": " << " Leaf Inactive\n";
}

void Kontroller::Incoming_Call_From_Root(Leaf_Initiated_ATM_Call& lic)   {
  cout << "*** Kontroller: " << _id << ": " << " Incoming_Call_From_Root\n";
  ATM_Attributes::result err;
  lic.Associate_Controller(*this);
  if (_accept_root_call) {
    cout << "*** " << _id << ": " << " Accepting root call\n";
    assert((err = lic.Accept()) == ATM_Attributes::success);
  }
  else {
    cout << "*** " << _id << ": " << " Rejecting root call\n";
    assert((err = lic.TearDown()) == ATM_Attributes::success);
  }
}

//-----------------------------------------------------------------

int main(int argc, char** argv) {
  
  DiagLevel("ipc",DIAG_ENV);
  DiagLevel("api",DIAG_ENV);

  if (argc!=3) {
    cout << "Usage: " << argv[0] << " host-file commandfile" << endl;
    exit(1);
  }
  
  Kontroller *manager = new Kontroller(argv[1],argv[2]);

  SEAN_Run();
  exit(0);
}

