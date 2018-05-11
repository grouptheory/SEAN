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
static char const _Application_cc_rcsid_[] =
"$Id: Application.cc,v 1.9 1998/09/28 17:54:56 mountcas Exp $";
#endif
#include <common/cprototypes.h>

#include "Application.h"
#include <sean/cc/sean_Visitor.h>
#include <FW/basics/diag.h>

#include <sean/ipc/IPC_Visitors.h>
#include <sean/ipc/tcp_ipc.h>
#include <sean/templates/constants.h>

#include <iostream.h>
#include <FW/basics/Conduit.h>
#include <FW/behaviors/Adapter.h>
#include <FW/behaviors/Protocol.h>
#include <FW/basics/Visitor.h>
#include <FW/kernel/Kernel.h>
#include <FW/kernel/SimEvent.h>
#include <FW/basics/diag.h>
#include <FW/basics/FW.h>
#include <codec/uni_ie/ie.h>
#include <codec/q93b_msg/q93b_msg.h>
#include <sean/ipc/tcp_ipc.h>

#include <sean/cc/call_control.h>
#include <sean/cc/sean_Visitor.h>

#include <codec/q93b_msg/Buffer.h>
#include <codec/uni_ie/ie_base.h>
#include <codec/uni_ie/cause.h>
#include <codec/uni_ie/conn_id.h>
#include <codec/uni_ie/UNI40_leaf.h>
#include <codec/uni_ie/broadband_send_comp_ind.h>
#include <codec/uni_ie/end_pt_ref.h>
#include <codec/q93b_msg/generic_q93b_msg.h>
#include <codec/q93b_msg/release.h>
#include <codec/q93b_msg/release_comp.h>
#include <codec/q93b_msg/setup.h>
#include <codec/q93b_msg/add_party_ack.h>
#include <FW/kernel/SimEvent.h>

#include <sean/daemon/ccd_config.h>

#define SETUP_TIME      2.0
#define ADD_PARTY_TIME  8.0
#define DROP_PARTY_TIME 12.0
#define RELEASE_TIME    20.0


VisitorType * Application::_sean_type = 0;


//-----------------
Application::Application(char * file, int identity) {
  char buf[2048], action[255], arg[5][255];
  double t;
  int to_identity;

  if (!_sean_type)
    _sean_type = (VisitorType *)QueryRegistry(SEAN_VISITOR_NAME);
  

  FILE* fp = fopen(file,"r");
  if (!fp) {
    diag("daemon.app",DIAG_FATAL,"unable to open file %s\n",file);
  } 
  
  _identity = identity;
  diag("daemon.app",DIAG_INFO,"application identity = %d\n",identity);

  _appid = new tcp_local_id( identity );
  _vpi = NO_VPI;
  _vci = NO_VCI;
  _cid = 2;
  bzero(_ieo, sizeof(InfoElem *) * num_ie);
  bzero(_iei, sizeof(InfoElem *) * num_ie);

  IPC_SignallingVisitor* iv = new 
    IPC_SignallingVisitor(IPC_SignallingVisitor::establish_by_peer, 
			  (const abstract_local_id*) _appid,
			  0);
  _boot = new VisitorLaunch(this, iv, 0.0);
  Register(_boot);

  while (fscanf(fp,"time %lf action %s",&t, action) != EOF) {

      switch (action[0]) {

      case 's': // setup
	fscanf(fp,"%d\n",&to_identity);
	diag("daemon.app",DIAG_INFO,"app#%d : setup scheduled to %d for time %lf\n",identity, to_identity,t);
	setup_call(to_identity,t);
	break;

      case 'a': // add_party
	fscanf(fp,"%d\n",&to_identity);
	add_party(to_identity,t);
	diag("daemon.app",DIAG_INFO,"app#%d : add_party scheduled to %d for time %lf\n",identity, to_identity,t);
	break;

      case 'r': // register_service
	fscanf(fp,"\n");
	register_service(t);
	diag("daemon.app",DIAG_INFO,"app#%d : register_service scheduled for time %lf\n",identity, t);
	break;

      case 'd': // drop_party
	diag("daemon.app",DIAG_INFO,"drop_party not yet implemented\n");
	break;
      case 't': // teardown
	fscanf(fp,"\n");
	teardown(t);
	diag("daemon.app",DIAG_INFO,"app#%d : teardown scheduled for time %lf\n",identity, t);
	break;
      case 'l': // lij
	diag("daemon.app",DIAG_INFO,"lij not yet implemented\n");
	break;
      case '#': // comment
	break;
      default:
	break;
      }
  };
}

//-----------------
State * Application::sean_visitor_from_B(sean_Visitor * sv) 
{
  const abstract_local_id * id = sv->get_shared_app_id();
  sean_Visitor::sean_Visitor_Type t = sv->get_op();
  generic_q93b_msg * msg = sv->share_message();
  InfoElem ** ies = msg->get_ie_array();

  switch (t) {
    case sean_Visitor::setup_ind: 
      diag("daemon.app",DIAG_INFO,"cc: setup_ind, sending connect_req\n");
      SendMessageToUNI(sv, sean_Visitor::connect_req);
      break;

    case sean_Visitor::add_party_ind:
      diag("daemon.app",DIAG_INFO,"cc: add_party_ind, sending add_party_ack_req\n");
      SendMessageToUNI(sv, sean_Visitor::add_party_ack_req);
      break;

    case sean_Visitor::drop_party_ind:
      diag("daemon.app",DIAG_INFO,"cc: setup_ind, sending drop_party_ack_req\n");
      SendMessageToUNI(sv, sean_Visitor::drop_party_ack_req);
      break;

    case sean_Visitor::service_register_accept: 
    case sean_Visitor::service_dereg_accept: 
    case sean_Visitor::service_register_deny: 
    case sean_Visitor::service_dereg_deny: 
    case sean_Visitor::call_proc_ind:
    case sean_Visitor::connect_ind: 
    case sean_Visitor::release_ind: 
    case sean_Visitor::release_comp_ind: 
    case sean_Visitor::connect_ack_ind:
    case sean_Visitor::setup_comp_ind:
    case sean_Visitor::status_enq_ind:
    case sean_Visitor::status_ind:
    case sean_Visitor::add_party_comp_ind:
    case sean_Visitor::add_party_ack_ind:
    case sean_Visitor::add_party_rej_ind:
    case sean_Visitor::drop_party_comp_ind:
    case sean_Visitor::drop_party_ack_ind:
    case sean_Visitor::leaf_setup_ind: 
    case sean_Visitor::restart_ind:
    case sean_Visitor::restart_comp_ind:
    case sean_Visitor::restart_ack_ind:
      break;
    default:
      break;
  }  return this;
}


//-----------------
State * Application::Handle(Visitor * v) 
{
  State * override = 0;
  VisitorType vt = v->GetType();
  const VisitorType * const sean_type = QueryRegistry(SEAN_VISITOR_NAME);
  sean_Visitor * sv;

  if (vt.Is_A(sean_type)) {
    sv = (sean_Visitor*)v;

    switch (VisitorFrom(v)) {
    case Visitor::A_SIDE: 
      diag("daemon.app",DIAG_INFO,"%s:Application - from the A-side we get a <%s>\n", OwnerName(), sv->op_name());
      PassThru(sv);
      break; 

    case Visitor::B_SIDE: 
      diag("daemon.app",DIAG_INFO,"%s:Application - from the Signalling daemon, we get a <%s>\n", OwnerName(), sv->op_name());
      sean_visitor_from_B( sv );
      
      CUSTOM_HANDLE(v);

      break;
    default:
      break;
    }
  }
  else v->Suicide();

  return this;
}


void Application::Failure_SendRelease(sean_Visitor* sv, ie_cause::CauseValues c) 
{
  InfoElem** ie_array;
  ie_array=new InfoElem* [num_ie];
  for (int i=0;i<num_ie;i++) ie_array[i]=0;
  ie_cause::Location loc;
  loc= ie_cause::user;
  ie_array[InfoElem::ie_cause_ix] = new ie_cause(c,loc);
  generic_q93b_msg* msg = new q93b_release_message(ie_array, sv->get_crv(), 0);
  sean_Visitor* killv = new sean_Visitor(sean_Visitor::release_req, msg);
  killv->set_crv( sv->get_crv() );
  killv->set_cid( sv->get_cid() );
  killv->set_shared_app_id( sv->get_shared_app_id() );
  PassVisitorToB(killv);
}


void Application::Failure_SendReleaseComplete(sean_Visitor* sv, ie_cause::CauseValues c) 
{
  InfoElem** ie_array;
  ie_array=new InfoElem* [num_ie];
  for (int i=0;i<num_ie;i++) ie_array[i]=0;
  ie_cause::Location loc;
  loc= ie_cause::user;

  ie_array[InfoElem::ie_cause_ix] = new ie_cause(c,loc);
  generic_q93b_msg* msg = new q93b_release_comp_message(ie_array, sv->get_crv(), 0);
  sean_Visitor* killv = new sean_Visitor(sean_Visitor::release_comp_req, msg);
  killv->set_crv( sv->get_crv() );
  killv->set_cid( sv->get_cid() );
  killv->set_shared_app_id( sv->get_shared_app_id() );
  PassVisitorToB(killv);
}


//-----------------
void Application::SendMessageToUNI(sean_Visitor * sv, 
				    sean_Visitor::sean_Visitor_Type t) 
{
  sean_Visitor * clone = sv->Clone(t);
  if (clone) {
    PassVisitorToB(clone);
  } else 
    diag("daemon.app",DIAG_ERROR,"Couldn't build clone sean_Visitor\n");
}


//-----------------
VisitorLaunch* Application::register_service(double timefromnow) {
  InfoElem * ie1[num_ie];
  bzero(ie1, sizeof(InfoElem *) * num_ie);
  // ATM traffic descriptor
  IE_TRAFFIC_DESC * td = new IE_TRAFFIC_DESC();
  td->set_TP1(IE_TRAFFIC_DESC::fw,100,100);
  _ieo[InfoElem::ie_traffic_desc_ix]= td;
  // Bbc
  _ieo[InfoElem::ie_broadband_bearer_cap_ix]=
    new ie_bbc(ie_bbc::BCOB_X,
	       ie_bbc::not_clipped,
	       ie_bbc::p2mp,
	       ie_bbc::NonRT_VBR);
  // end_pt_ref
  _ieo[InfoElem::ie_end_pt_ref_ix] = new ie_end_pt_ref(0);

  // called party number
  char addrstring[255];
  sprintf(addrstring,"0x47.0005.80.ffde00.0000.0000.0104.00000000000%1d.00", _identity );
  _addr = new NSAP_DCC_ICD_addr(addrstring);

  _ieo[InfoElem::ie_called_party_num_ix] = new ie_called_party_num(_addr);
  // QOS class
  _ieo[InfoElem::ie_qos_param_ix] = new ie_qos_param(ie_qos_param::qos0,ie_qos_param::qos0);
  // broadband sending complete 
  _ieo[InfoElem::ie_broadband_send_comp_ind_ix] = new ie_broadband_send_comp_ind(); 
  q93b_freeform_message *msg = new q93b_freeform_message(_ieo, _crv, 0);
  assert(msg);

  // build a setup visitor sv
  sean_Visitor * sv = new sean_Visitor(sean_Visitor::service_register_req, msg);
  sv->set_crv(_crv);
  sv->set_shared_app_id(_appid);
  sv->set_cid(_cid);
  sv->set_lid(NO_LID);
  sv->set_vpi(_vpi);
  sv->set_vci(_vci);
  sv->set_oport(_identity);
  VisitorLaunch* launch = new VisitorLaunch(this, sv, timefromnow);
  Register(launch);
  return launch;
}

//-----------------
VisitorLaunch* Application::setup_call(int hostnumber, double timefromnow) {
  InfoElem * ie1[num_ie];
  bzero(ie1, sizeof(InfoElem *) * num_ie);
  // ATM traffic descriptor
  IE_TRAFFIC_DESC * td = new IE_TRAFFIC_DESC();
  td->set_TP1(IE_TRAFFIC_DESC::fw,100,100);
  _ieo[InfoElem::ie_traffic_desc_ix]= td;
  // Bbc
  _ieo[InfoElem::ie_broadband_bearer_cap_ix]=
    new ie_bbc(ie_bbc::BCOB_X,
	       ie_bbc::not_clipped,
	       ie_bbc::p2mp,
	       ie_bbc::NonRT_VBR);
  // end_pt_ref
  _ieo[InfoElem::ie_end_pt_ref_ix] = new ie_end_pt_ref(0);

  // called party number
  char addrstring[255];
  sprintf(addrstring,"0x47.0005.80.ffde00.0000.0000.0104.00000000000%1d.00",hostnumber);
  _addr = new NSAP_DCC_ICD_addr(addrstring);

  _ieo[InfoElem::ie_called_party_num_ix] = new ie_called_party_num(_addr);
  // QOS class
  _ieo[InfoElem::ie_qos_param_ix] = new ie_qos_param(ie_qos_param::qos0,ie_qos_param::qos0);
  // broadband sending complete 
  _ieo[InfoElem::ie_broadband_send_comp_ind_ix] = new ie_broadband_send_comp_ind(); 
  q93b_setup_message *msg = new q93b_setup_message(_ieo, _crv, 0);
  assert(msg);
  // build a setup visitor sv
  sean_Visitor * sv = new sean_Visitor(sean_Visitor::setup_req, msg);
  sv->set_crv(_crv);
  sv->set_shared_app_id(_appid);
  sv->set_cid(_cid);
  sv->set_lid(NO_LID);
  sv->set_vpi(_vpi);
  sv->set_vci(_vci);
  sv->set_oport(_identity);
  VisitorLaunch* launch = new VisitorLaunch(this, sv, timefromnow);
  Register(launch);
  return launch;
}

//-----------------
VisitorLaunch* Application::add_party(int hostnumber, double timefromnow) {
  // AddParty -- we are cheating by putting the Bbc, td, and QOS in the add party message
  // due to an incomplete CC 
  InfoElem * ie[num_ie];
  for (int i = 0; i < num_ie; i++)
    ie[i] = 0;
  ie[InfoElem::ie_broadband_bearer_cap_ix] = _ieo[InfoElem::ie_broadband_bearer_cap_ix]->copy();
  ie[InfoElem::ie_traffic_desc_ix] = _ieo[InfoElem::ie_traffic_desc_ix]->copy();
  ie[InfoElem::ie_qos_param_ix] = _ieo[InfoElem::ie_qos_param_ix]->copy();
  
  char addrstring[255];
  sprintf(addrstring,"0x47.0005.80.ffde00.0000.0000.0104.00000000000%1d.00",hostnumber);
  _addr = new NSAP_DCC_ICD_addr(addrstring);

  ie[InfoElem::ie_called_party_num_ix] = new ie_called_party_num(_addr);
  ie[InfoElem::ie_end_pt_ref_ix] = new ie_end_pt_ref(1);
  q93b_add_party_message *m = new q93b_add_party_message(ie, _crv, 1);
  sean_Visitor * apv = new sean_Visitor(sean_Visitor::add_party_req, m);
  apv->set_crv(_crv);
  apv->set_shared_app_id(_appid);
  apv->set_cid(_cid);
  apv->set_lid(1);
  apv->set_pid(1);
  apv->set_vpi(_vpi);
  apv->set_vci(_vci);
  apv->set_oport(_identity);
  VisitorLaunch* launch = new VisitorLaunch(this, apv, timefromnow);
  Register(launch);
  return launch;
}

//-----------------
VisitorLaunch* Application::teardown(double timefromnow) {
  // AddParty -- we are cheating by putting the Bbc, td, and QOS in the add party message
  // due to an incomplete CC 
  // Send a release to close party with epr = 1
  InfoElem * release_ie[num_ie];
  for (int i = 0; i < num_ie; i++)
    release_ie[i] = 0;
  release_ie[InfoElem::ie_cause_ix] = new ie_cause(ie_cause::normal_unspecified,ie_cause::user);
  q93b_release_message *rm = new q93b_release_message(release_ie, _crv, 1);
  sean_Visitor * rv = new sean_Visitor(sean_Visitor::release_req, rm);
  rv->set_crv(_crv);
  rv->set_shared_app_id(_appid);
  rv->set_cid(_cid);
  rv->set_lid(0);
  rv->set_pid(1);
  rv->set_vpi(_vpi);
  rv->set_vci(_vci);
  rv->set_oport(_identity);
  VisitorLaunch* launch = new VisitorLaunch(this, rv, timefromnow);
  Register(launch);
  return launch;
}

//-----------------
void Application::Interrupt(class SimEvent * e) 
{
}

//-----------------
Application::~Application() { } 



void Application:: CUSTOM_HANDLE (Visitor * v) {

  // cout << OwnerName() << " received " << v->GetType() << " (" << v << ") \n";
  // cout << "which took the path " << endl;
  // v->DumpLog(cout);
  // cout << endl;

    if (v->GetType().Is_A(_sean_type)) {
      sean_Visitor * sv = (sean_Visitor *)v;

      generic_q93b_msg * msg = sv->share_message();
      if (!msg) return;

      InfoElem ** ies = msg->get_ie_array();

      switch (sv->get_op()) {
      case sean_Visitor::setup_ind: 
	{
	  // cout << OwnerName() << " received setup_ind visitor" << endl;
	  for (int i = 0; i < num_ie; i++)
	    _iei[i] = ies[i];
	}
      break;

      case sean_Visitor::call_proc_ind :
	// cout << OwnerName() << " received call_proc_ind  visitor" << endl;
	_vpi = sv->get_vpi();
	_vci = sv->get_vci();
	break;
	    
      case sean_Visitor::connect_ind :
	// cout << OwnerName() << " received connect_ind visitor" << endl;
	break;

      case sean_Visitor::connect_ack_ind :
	// cout << OwnerName() << " received connect_ack_ind visitor" << endl;
	break;

      case sean_Visitor::release_ind :
	// cout << OwnerName() << " received release_ind visitor" << endl;
	break;

      case sean_Visitor::release_comp_ind :
	// cout << OwnerName() << " received release_comp_ind visitor" << endl;
	break;

      case sean_Visitor::setup_comp_ind :
	// cout << OwnerName() << " received setup_comp_ind visitor" << endl;
	break;

      case sean_Visitor::status_enq_ind :
	// cout << OwnerName() << " received status_enq_ind visitor" << endl;
	break;

      case sean_Visitor::add_party_ind :
	// cout << OwnerName() << " received add_party_ind visitor" << endl;
	break;

      case sean_Visitor::add_party_comp_ind :
	// cout << OwnerName() << " received add_party_comp_ind visitor" << endl;
	break;

      case sean_Visitor::add_party_ack_ind :
	// cout << OwnerName() << " received add_party_ack_ind visitor" << endl;
	break;

      case sean_Visitor::add_party_rej_ind :
	// cout << OwnerName() << " received add_party_rej_ind visitor" << endl;
	break;

      case sean_Visitor::drop_party_ind :
	// cout << OwnerName() << " received drop_party_ind visitor" << endl;
	break;

      case sean_Visitor::drop_party_comp_ind :
	// cout << OwnerName() << " received drop_party_comp_ind visitor" << endl;
	break;

      case sean_Visitor::drop_party_ack_ind :
	// cout << OwnerName() << " received drop_party_ack_ind visitor" << endl;
	break;

      case sean_Visitor::leaf_setup_ind :
	// cout << OwnerName() << " received leaf_setup_ind visitor" << endl;
	break;

      case sean_Visitor::restart_ind :
	// cout << OwnerName() << " received restart_ind visitor" << endl;
	break;

      case sean_Visitor::restart_comp_ind :
	// cout << OwnerName() << " received restart_comp_ind visitor" << endl;
	break;

      case sean_Visitor::restart_ack_ind :
	// cout << OwnerName() << " received restart_comp_ind visitor" << endl;
	break;
      }
    }
}
