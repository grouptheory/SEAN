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
static char const _CallControl_cc_rcsid_[] =
"$Id: SwitchFabric.cc,v 1.23 1999/04/09 20:45:02 marsh Exp $";
#endif
#include <common/cprototypes.h>


#include <DS/containers/dictionary.h>

#include <FW/basics/diag.h>
#include <codec/uni_ie/ie.h>
#include <codec/q93b_msg/q93b_msg.h>
#include <sean/cc/sean_Visitor.h>
#include <sean/cc/VPVCSimEvent.h>
#include "SwitchFabric.h"
#include "ATM_PDU_Visitor.h"

#define Q93B_DEBUGGING "fsm.uni.Q93B_debugging"

VisitorType * PortAccessor::_conn_type = 0;
VisitorType * PortAccessor::_sean_type = 0;
VisitorType * PortAccessor::_data_type = 0;


// -----------------------------------------------
Binding::Binding(int ivpi, int ivci, int iport, int iepr, int icrv,
		 int ovpi, int ovci, int oport, int oepr, int ocrv) 
  : _ivpi(ivpi), _ivci(ivci), _iport(iport), _iepr(iepr), _icrv(icrv),
    _ovpi(ovpi), _ovci(ovci), _oport(oport), _oepr(oepr), _ocrv(ocrv),
    _strength(STRONG_BINDING) 
{ }

Binding::~Binding( ) { }
  
int Binding::GetiVPI(void) const  { return _ivpi;  }
int Binding::GetiVCI(void) const  { return _ivci;  }
int Binding::GetiPort(void) const { return _iport; }
int Binding::GetiEpr(void) const  { return _iepr;  }
int Binding::GetiCrv(void) const  { return _icrv;  }

int Binding::GetoVPI(void) const  { return _ovpi;  }
int Binding::GetoVCI(void) const  { return _ovci;  }
int Binding::GetoPort(void) const { return _oport; }
int Binding::GetoEpr(void) const  { return _oepr;  }
int Binding::GetoCrv(void) const  { return _ocrv;  }

void Binding::SetoVPI(int ovpi)   { _ovpi  = ovpi;  }
void Binding::SetoVCI(int ovci)   { _ovci  = ovci;  }
void Binding::SetoPort(int oport) { _oport = oport; }
void Binding::SetoEpr(int oepr)   { _oepr  = oepr;  }
void Binding::SetoCrv(int ocrv)   { _ocrv  = ocrv;  }

void Binding::SetiCrv(int icrv)   { _icrv = icrv;   }


void Binding::Set_strength(int value)
{ _strength = value; }

int Binding::Get_strength(void)
{ return _strength; }



// -----------------------------------------------
VCAlloc::VCAlloc(void) : _lowest(MinVCI + 1) { }

VCAlloc::~VCAlloc( ) 
{ 
  _svc_free.clear();
  _svc_in_use.clear();
}

// -----------------------------------------------
unsigned int VCAlloc::GetNewVCI(void)
{
 unsigned int vci = 0;

  // If the free list is smaller than we'd like allocate more vc's
  if (_svc_free.size() < MinSize) {
    for (unsigned int i = _lowest; i < _lowest + 64 && i > 0; i++) 
      _svc_free.append(i);
    _lowest += 64;
  }

  if (!_svc_free.empty()) {
    vci = _svc_free.pop();
    list_item li;
    if (li = _svc_free.search(vci))
      _svc_free.del_item(li);
    _svc_in_use.append(vci); 
  }
  return vci;
}

// -----------------------------------------------
unsigned int VCAlloc::RequestVCI(unsigned int vci)
{
  list_item it;
  if (it = _svc_free.search(vci)) {
    _svc_free.del_item(it);
    _svc_in_use.append(vci);
    return vci;
  }
  return 0;
}

// -----------------------------------------------
void VCAlloc::ReturnVCI(unsigned int vci)
{
  list_item it;
  if (it = _svc_in_use.search(vci)) {
    _svc_in_use.del_item(it);
    _svc_free.append(vci);
  }
}

// -----------------------------------------------
// This is the routing fabric at the top of this switch
// -----------------------------------------------
VisitorType * Router::_sean_vistype = 0;
VisitorType * Router::_data_vistype = 0;

Router::Router(void) : Terminal()
{ 
  if (!_sean_vistype)
    _sean_vistype = (VisitorType *)QueryRegistry(SEAN_VISITOR_NAME);
  if (!_data_vistype)
    _data_vistype = (VisitorType *)QueryRegistry(ATM_PDU_VISITOR_NAME);
}

// -----------------------------------------------
Router::~Router(void) 
{ 
  if (_sean_vistype) {
    delete _sean_vistype;
    _sean_vistype = 0;
  }
}

// -----------------------------------------------
void Router::RegisterAddr(NSAP_DCC_ICD_addr * addr, int port) 
{
  dic_item di;
  assert (!(di = _addr2port.lookup(addr)));

  // Add the address and setup the VCAllocator and list of bindings
  _addr2port.insert(addr, port);
  _p2vca.insert(port, new VCAlloc());
  _p2bind.insert(port, new list<Binding *>);
}

// -----------------------------------------------
int Router::AddrToPort(const NSAP_DCC_ICD_addr * a) const
{
  int rval = NO_PORT;
  dic_item di;
  if (di = _addr2port.lookup((NSAP_DCC_ICD_addr *)a))
    rval = _addr2port.inf(di);
  return rval;
}

// -----------------------------------------------
void Router::Full_RT(void)
{
  dic_item li;
  forall_items(li, _p2vca) {
    int port = _p2vca.key(li);
    RT(port);
  }
}

// -----------------------------------------------
void Router::RT(int port)
{
  Binding *e  = 0;
  dic_item di;
  cout << " :::: (port  vp  vc  crv  epr) <===>  (port  vp  vc  crv  epr) :::: port = " << port << endl;
  if (di = _p2bind.lookup(port)) {
    list<Binding *> * blist = _p2bind.inf(di);
    list_item li;
    forall_items(li, *blist) {
      if (e = blist->inf(li)) {
	cout << "( " 
	     << e->GetiPort() << " " << e->GetiVPI() << " " << e->GetiVCI() << " " 
	     << hex << "0x" << e->GetiCrv() << dec << " " << e->GetiEpr() << " ) <= ";

	switch(e->Get_strength()) {
	case STRONG_BINDING:              cout << "$";
	  break;
	case WEAK_BINDING__SETUP_SEEN:    cout << "s";
	  break;
	case WEAK_BINDING__DROP_SEEN:     cout << "d";
	  break;
	case WEAK_BINDING__RELEASE_SEEN:  cout << "r";
	  break;
	case UNSPECIFIED_BINDING_TYPE:
	  break;
	default:                          cout << "?";
	  break;
	};

	cout << " => ( " <<e->GetoPort() << " " << e->GetoVPI() << " " << e->GetoVCI() << " " 
	     << hex << "0x" << e->GetoCrv() << dec << " " << e->GetoEpr() << " )" << endl;
      }
    }
  }
}

// -----------------------------------------------
void Router::Absorb(Visitor * v)
{
  sean_Visitor * sv = 0;
  int binding_type  = UNSPECIFIED_BINDING_TYPE;

  if (v->GetType().Is_A(_data_vistype)) {
    Handle_Data_Visitor(v);
  } 
  else if (v->GetType().Is_A(_sean_vistype)) {

    sv = (sean_Visitor *)v;
    int original_icrv = sv->get_crv();

    diag("daemon.router", DIAG_DEBUG, "Router::Absorb: received %s from port %d.\n", 
	 sv->op_name(), sv->get_iport());

    int iport, ivpi, ivci, icrv, epr, oport, ovpi, ovci;

    generic_q93b_msg * msg = sv->share_message();
    assert(msg);

    InfoElem ** ies = msg->get_ie_array();
    ie_called_party_num * number = 0;
    
    epr = sv->get_epr();
    icrv = sv->get_crv();
    iport = sv->get_iport();
    number = (ie_called_party_num *)(msg->ie(InfoElem::ie_called_party_num_ix));
    if (number) {
      oport = AddrToPort( (NSAP_DCC_ICD_addr *)number->get_addr() );
    }
    else {
      oport = NO_PORT;
    }

    diag("daemon.router", DIAG_DEBUG, "%s received a %s ...\n", OwnerName(), v->GetType().Name());
    DIAG("daemon.router", DIAG_DEBUG, cout << "*** Before: \n"; Full_RT(); );

    switch (sv->get_op()) {

    //********************************************************************
    case sean_Visitor::add_party_ind:
    case sean_Visitor::setup_ind:
      {
	if (oport == NO_PORT) {
	  No_Route_To_Destination(sv);
	  sv = 0;
	  // life ends here for the original visitor
	  break;
	}
	
	// Allocate an input VPI/VCI if necessary
	
	if ( sv->get_op() == sean_Visitor::setup_ind ) {
	  ivci = GetNextVCI(iport);
	  ivpi = 0;
	  diag("daemon.router", DIAG_DEBUG, "Incoming leg: allocating new VP=%d,VC=%d\n",ivpi,ivci);
	  
	  // Obtain the SimEvent to carry the VP/VC back to the Network CC
	  SimEvent * se = sv->GetReturnEvent();
	  if (!se)
	    diag("daemon.router", DIAG_ERROR, "setup/addparty indication w/o SimEvent.\n");
	  else {
	    // assume it's the VPVCSimEvent
	    assert(se->GetCode() == VPVC_CODE);
	    VPVCSimEvent * vse = (VPVCSimEvent *)se;
	    vse->SetVCI(ivci);
	    vse->SetVPI(ivpi);
	    // The CREF should've been set already, but set it again
	    vse->SetCREF(original_icrv);
	    // Notify the previous Network side CC
	    ReturnToSenderSynchronous(se);
	  }
	}
	else {
	  ivci = sv->get_vci();
	  ivpi = sv->get_vpi();
	  diag("daemon.router", DIAG_DEBUG, "Incoming leg: using existing VP=%d,VC=%d\n",ivpi,ivci);
	}

	int hint_crv = NO_CREF;
	
	// Obtain a new VCI if necessary
	Binding* old_branch = OnExistingOutgoingBranch(ivpi,iport,ivci,oport);
	if ( !old_branch ) {
	  ovci = GetNextVCI(oport);
	  ovpi = 0;
	  diag("daemon.router", DIAG_DEBUG, "Outgoing leg: allocating new VP=%d,VC=%d\n",ovpi,ovci);
	}
	else {
	  ovci = old_branch->GetoVCI();
	  ovpi = old_branch->GetoVPI();
	  diag("daemon.router", DIAG_DEBUG, "Outgoing leg: using existing VP=%d,VC=%d\n",ovpi,ovci);

	  Binding* hint = LocateBinding(iport, ivpi, ivci, NO_EPR, original_icrv);
	  assert(hint);
	  hint_crv = hint->GetoCrv();
	}
	
	// build the connection identifier IE.
	
	if (!ies[InfoElem::ie_conn_identifier_ix])
	  ies[InfoElem::ie_conn_identifier_ix] = new ie_conn_id(ovpi, ovci, 1, 0);
	else {
	  ie_conn_id * cid_ie = (ie_conn_id *)(msg->ie(InfoElem::ie_conn_identifier_ix));
	  cid_ie->set_vpi(ovpi);
	  cid_ie->set_vci(ovci);
	}
	msg->re_encode();
	sv->give_message(msg);  // We cannot use msg herafter!  It is zeroed.

	int outward_epr = epr;

	// allocate the forward binding
	Binding* fb = AddBinding(ivpi, ivci, iport, epr, original_icrv,
				 ovpi, ovci, oport, outward_epr, hint_crv);
	// allocate the backward binding
	Binding* bb = AddBinding(ovpi, ovci, oport, outward_epr, hint_crv, 
				 ivpi, ivci, iport, epr, original_icrv);
	
	fb->Set_strength(WEAK_BINDING__SETUP_SEEN);
	bb->Set_strength(WEAK_BINDING__SETUP_SEEN);

	// Allocate a new SimEvent to carry the cref back to us
	SimEvent* se = new VPVCSimEvent(this, this, ovpi, ovci, NO_CREF);
	((VPVCSimEvent *)se)->SetPort(oport);
	sv->SetReturnEvent(se);
	// Set the newly alloc'd VPI/VCI in the Visitor
	sv->set_vci(ovci);
	sv->set_vpi(ovpi);

	sv->set_crv(hint_crv);

	// Set the out port to the proper destination
	sv->set_oport(oport);  	 
	// pass it on ...
      }
    break;
    
    //********************************************************************
    case sean_Visitor::add_party_ack_ind: 
    case sean_Visitor::connect_ind: 
      {
	Binding * b = FindEntry(sv, WEAK_BINDING__SETUP_SEEN);
	if (b) {
	  Binding * reverse = LocateBinding(b->GetoPort(), b->GetoVPI(), 
					    b->GetoVCI(), b->GetoEpr(), b->GetoCrv());
	  assert (b->Get_strength() == WEAK_BINDING__SETUP_SEEN);
	  assert (reverse->Get_strength() == WEAK_BINDING__SETUP_SEEN);

	  b->Set_strength(STRONG_BINDING);
	  reverse->Set_strength(STRONG_BINDING);

	  Route_Sean_Visitor(sv, STRONG_BINDING);
	  sv=0;
	}
      }
    break;
    
    //********************************************************************
    case sean_Visitor::leaf_setup_ind:
    case sean_Visitor::leaf_setup_failure_ind:
      {
	number = (ie_called_party_num *)(msg->ie(InfoElem::ie_called_party_num_ix));
	// find out oport
	oport = AddrToPort( (NSAP_DCC_ICD_addr *)number->get_addr() );
	if (oport == NO_PORT) {
	  No_Route_To_Destination(sv);
	  sv = 0;
	  break;
	}

	// Set the out port to the proper destination
	sv->set_oport(oport);
	// pass it on ...
      }
    break;
    
    //********************************************************************
    case sean_Visitor::release_ind:     
      if (binding_type == UNSPECIFIED_BINDING_TYPE) {
	binding_type = WEAK_BINDING__RELEASE_SEEN;
      }
      // fall through..
    case sean_Visitor::drop_party_ind:   
      if (binding_type == UNSPECIFIED_BINDING_TYPE) {
	binding_type = WEAK_BINDING__DROP_SEEN;
      }

      {
	Binding * b = FindEntry(sv, STRONG_BINDING);
	if (b) {
	  Binding * reverse = LocateBinding(b->GetoPort(), b->GetoVPI(), 
					    b->GetoVCI(), b->GetoEpr(), b->GetoCrv());

	  Route_Sean_Visitor(sv, STRONG_BINDING);
	  
	  b->Set_strength( binding_type );
	  reverse->Set_strength( binding_type );
	  sv = 0;
	}
      }
    break;

    //********************************************************************
    case sean_Visitor::release_comp_ind: 
      {
	Binding * b = FindEntry(sv, WEAK_BINDING__RELEASE_SEEN);
	if ( b ) {
	  Binding * reverse = LocateBinding(b->GetoPort(), b->GetoVPI(), 
					    b->GetoVCI(), b->GetoEpr(), b->GetoCrv());
	  int strength = b->Get_strength();
	  if ((strength == WEAK_BINDING__RELEASE_SEEN)) {
	    sv->Suicide();
	  }

	  if ( strength == WEAK_BINDING__DROP_SEEN) 
	    Route_Sean_Visitor(sv, WEAK_BINDING__DROP_SEEN);
	  else
	    if (strength == WEAK_BINDING__SETUP_SEEN)
	      Route_Sean_Visitor(sv, WEAK_BINDING__SETUP_SEEN);
	  sv = 0;

	  if (reverse)
	    RemoveBinding(reverse);
	  else 
	    diag("daemon.router", DIAG_WARNING, "release_comp_ind reverse removal failed\n");

	  RemoveBinding(b);
	}
	else
	  diag("daemon.router", DIAG_WARNING, "release_comp_ind forward removal failed\n");
      }
    break;

    //********************************************************************
    case sean_Visitor::drop_party_ack_ind:
      {
	Binding * b = FindEntry(sv, WEAK_BINDING__RELEASE_SEEN);
	assert( b );
	Binding * reverse = LocateBinding(b->GetoPort(), b->GetoVPI(), 
					  b->GetoVCI(), b->GetoEpr(), b->GetoCrv());
	assert(reverse);

	int strength = b->Get_strength();
	if ( strength >= WEAK_BINDING__DROP_SEEN) {
	  Route_Sean_Visitor(sv, WEAK_BINDING__DROP_SEEN);
	  sv = 0;
	}

	RemoveBinding(b);
	RemoveBinding(reverse);
      }
    break;
    
    default:
      Route_Sean_Visitor(sv, STRONG_BINDING);
      sv = 0;
      break;
    }
    
    if (sv) {
      sv->Transform_into_req();

      DIAG("daemon.router", DIAG_DEBUG, \
       { cout << "sean_Visitor leaves Router::***** Final act in Absorb() "  \
	      << sv->op_name() ; printf ("  crv= %lx, epr= %lx, vpi=%d, vci=%d", 
		 sv->get_crv(), sv->get_epr(), sv->get_vpi(), sv->get_vci()); \
	     cout << " out to stack on port = " << sv->get_oport() << endl;} );

      Inject(sv);
    }
  } else {
    diag("daemon.router", DIAG_DEBUG, "Absorbed unknown Visitor (%s).\n", v->GetType().Name());
    v->Suicide();
  }
  
  DIAG("daemon.router", DIAG_DEBUG, cout << "*** After: \n"; Full_RT(); );
}

// -----------------------------------------------
void Router::Interrupt(SimEvent * event) 
{ 
  // The Network side CC will call us back with a SimEvent of type VPVCSimEvent
  // containing the newly allocated CREF for the outbound side.
  assert(event->GetCode() == VPVC_CODE);

  VPVCSimEvent * vse = (VPVCSimEvent *)event;
  int oport = vse->GetPort();
  int ovpi = vse->GetVPI();
  int ovci = vse->GetVCI();
  int ocrv = vse->GetCREF();

  // Need port, vp, vc
    
  diag("daemon.router",DIAG_INFO,"SimEvent specifies cref=%d.\n", ocrv);
  Binding * b = LocateBinding(oport, ovpi, ovci);
  assert(b);

  b->SetiCrv(ocrv);
  // Now repair the reverse direction ...
  Binding * b2 = LocateBinding(b->GetoPort(), b->GetoVPI(), b->GetoVCI(), b->GetoEpr(), b->GetoCrv());
  assert(b2);
  
  b2->SetoCrv(ocrv);
}

// -----------------------------------------------
Binding * Router::LocateBinding(int port, int vpi, int vci, int epr, int crv)
{
  Binding * rval = 0;
  dic_item di;
  if (di = _p2bind.lookup( port )) {
    list<Binding *> * lst = _p2bind.inf(di);
    list_item li;
    int hits=0;
    forall_items(li, *lst) {
      Binding * tmp = lst->inf(li);
      
      // if (crv==NO_CREF)
      if (tmp->GetiPort() == port &&
	  tmp->GetiVCI() == vci   &&
	  tmp->GetiVPI() == vpi   &&
	  ((tmp->GetiCrv() == crv) || (crv == NO_CREF))   &&
	  ((tmp->GetiEpr() == epr) || (epr == NO_EPR))) {
	// This should work since [port, vpi, vci] should be unique
	rval = tmp;
	hits++;
	break;
      }
    }
    assert(hits<=1);
  }
  return rval;
}


// -----------------------------------------------
Binding * Router::OnExistingOutgoingBranch(int port, int vpi, int vci, int oport)
{
  Binding * rval = 0;
  dic_item di;
  if (di = _p2bind.lookup( port )) {
    list<Binding *> * lst = _p2bind.inf(di);
    list_item li;
    int hits=0;
    forall_items(li, *lst) {
      Binding * tmp = lst->inf(li);
      
      if (tmp->GetiPort() == port &&
	  tmp->GetiVCI() == vci   &&
	  tmp->GetiVPI() == vpi   &&
	  (tmp->GetoPort() == oport)) {
	rval = tmp;
	hits++;
	break;
      }
    }
    assert(hits<=1);
  }
  return rval;
}

// -----------------------------------------------
Binding * Router::AddBinding(int ivpi, int ivci, int iport, int iepr, int icrv,
			     int ovpi, int ovci, int oport, int oepr, int ocrv)
{
  Binding * b = new Binding(ivpi, ivci, iport, iepr, icrv, ovpi, ovci, oport, oepr, ocrv);
  dic_item di = _p2bind.lookup(iport);
  assert(di);
  _p2bind.inf(di)->append(b);
  return b;
}

// -----------------------------------------------
void Router::RemoveBinding(Binding *& b)
{
  dic_item di = _p2bind.lookup(b->GetiPort());
  assert(di);
  // You may NOT delete the lists, only the items in the list
  list<Binding *> * lst = _p2bind.inf(di);
  list_item li = lst->search(b);
  assert(li);
  lst->del_item(li);
  delete b;
  b = 0;
}

// -----------------------------------------------
int Router::LivingBranches(int port, int vp, int vc) {
  list<Binding *> * branches = FindEntries(port,vp,vc);
  list_item li;
  int ct=0;
  forall_items(li,*branches) {
    Binding * b = branches->inf(li);
    if ( b->Get_strength() == STRONG_BINDING )
      ct ++;
  }
  return ct;
}

// -----------------------------------------------
Binding * Router::FindEntry(sean_Visitor *sv, int min_strength) const
{
  if(sv == 0) return 0;

  Binding * e = 0;
  dic_item di;

  Binding* ret = 0;
  if (di = _p2bind.lookup(sv->get_iport())) {
    list<Binding *> * blist = _p2bind.inf(di);
    list_item li;
    int hits=0;

    forall_items(li, *blist) {
      e = blist->inf(li);
      if (e->GetiVPI() == sv->get_vpi() && 
	  e->GetiVCI() == sv->get_vci() && 
	  e->GetiPort() == sv->get_iport() &&
	  e->Get_strength() >= min_strength &&
	  ( (e->GetiEpr() == sv->get_epr()) || (sv->get_epr() == NO_EPR) )) {
	ret = e;
	hits++;
      }
    }
    assert(hits<=1);
  }
  return ret;
}

// -----------------------------------------------
list<Binding *> * Router::FindEntries(int port, int vp, int vc) const
{
  list<Binding *> * rval = 0;

  dic_item di;
  forall_items(di, _p2bind) {
    list<Binding *> * binds = _p2bind.inf(di);

    list_item li;
    forall_items(li, *binds) {
      Binding * b = binds->inf(li);

      if (b->GetiPort() == port &&
	  b->GetiVPI()  == vp   &&
	  b->GetiVCI()  == vc) {
	if (!rval)
	  rval = new list<Binding *> ;
	rval->append(b);
      }
    }
  }
  return rval;
}

// -----------------------------------------------
int  Router::GetNextVCI(int port)
{
  int rval = NO_VCI;
  dic_item di;
  if (di = _p2vca.lookup(port))
    rval = _p2vca.inf(di)->GetNewVCI();
  return rval;
}

// -----------------------------------------------
void Router::Inject(Visitor * v)
{
  sean_Visitor* sv  = (sean_Visitor*) v;

  // I'd be glad to inject this visitor for you ...
  Terminal::Inject(v);
}


// -----------------------------------------------
void Router::Route_Sean_Visitor(sean_Visitor * sv, int min_strength) {
  Binding * b = FindEntry(sv, min_strength);
  
  assert(b);

  // If we have bindings, set the proper data members
  sv->set_oport(b->GetoPort());
  sv->set_vpi(b->GetoVPI());
  sv->set_vci(b->GetoVCI());
  sv->set_epr(b->GetoEpr());
  sv->set_crv(b->GetoCrv());
  sv->Transform_into_req();

  DIAG("daemon.router", DIAG_DEBUG, \
       { cout << "sean_Visitor leaves Router::***** Route_Sean_Visitor() "  \
	      << sv->op_name() ; printf ("  crv= %lx, epr= %lx, vpi=%d, vci=%d", 
		 sv->get_crv(), sv->get_epr(), sv->get_vpi(), sv->get_vci()); \
	     cout << " out to stack on port = " << sv->get_oport() << endl;} );
      
  Inject(sv);
}


// -----------------------------------------------
void Router::Handle_Data_Visitor(Visitor* v) {
  ATM_PDU_Visitor * apv = (ATM_PDU_Visitor *)v;
  // lookup the proper bindings, and send it out ...
  int vpi = apv->GetVPI(), vci = apv->GetVCI(), iport = apv->GetPort();
  list<Binding *> * binds = FindEntries(iport, vpi, vci);
  
  if (binds) {
    while (binds->size()) {
      Binding * b = binds->pop();
      
      // Check if this is the last one ...
      ATM_PDU_Visitor * to_send = apv;
      if (! binds->empty())
	to_send = (ATM_PDU_Visitor *)apv->duplicate();
      
      to_send->SetVPI( b->GetoVPI() );
      to_send->SetVCI( b->GetoVCI() );
      to_send->SetPort( b->GetoPort() );
      Terminal::Inject(to_send);
	
      DIAG("daemon.router", DIAG_INFO, cout << " SENDING ATM PDU to vpi:"
	   << b->GetoVPI() << ", vci:" << b->GetoVCI()
	   << ", port:" << b->GetoPort()
	   << endl;);
    }
    delete binds;
  }
}


// -----------------------------------------------
void Router::No_Route_To_Destination(sean_Visitor* sv) {
  diag("daemon.router", DIAG_DEBUG,
       "No Route found!  Be sure to configure Router with AddBinding.\n");

  // Send a release complete back of type no_route_to_destination
  int i = 0;
  InfoElem ** ie_array = new InfoElem * [ num_ie ];
  for (i = 0; i < num_ie; i++) ie_array[i] = 0;

  generic_q93b_msg * msg = sv->share_message();
  // Send a leaf_setup_failure complete back of type no_route_to_destination
  for (i = 0; i < num_ie; i++) 
    if (msg->ie((InfoElem::ie_ix)i))
      ie_array[i] = msg->ie((InfoElem::ie_ix)i)->copy();

  ie_array[InfoElem::ie_cause_ix] = 
    new ie_cause(ie_cause::no_route_to_destination, 
		 ie_cause::local_private_network);

  generic_q93b_msg * new_msg =0;
  sean_Visitor     * killv   =0;

  if (sv->get_op() == sean_Visitor::leaf_setup_ind) {
    // swap called and calling
	  
    Addr* called = ((ie_called_party_num*)
		    ie_array[InfoElem::ie_called_party_num_ix])-> 
                    get_addr()->
                    copy();
    delete ie_array[InfoElem::ie_called_party_num_ix];
	  
    Addr* calling = ((ie_calling_party_num*)
		     ie_array[InfoElem::ie_calling_party_num_ix])->
                     get_addr()->
                     copy();
    delete ie_array[InfoElem::ie_calling_party_num_ix];
	  
    ie_array[InfoElem::ie_called_party_num_ix]  = new ie_called_party_num(calling);
    ie_array[InfoElem::ie_calling_party_num_ix] = new ie_calling_party_num(called,
						   ie_calling_party_num::presentation_allowed,
						   ie_calling_party_num::user_provided_not_screened);

    new_msg = new UNI40_leaf_setup_failure_message(ie_array, sv->get_crv(), 0);
    killv = new sean_Visitor(sean_Visitor::leaf_setup_failure_req, new_msg);
  }
  else {
    new_msg = new q93b_release_comp_message(ie_array, sv->get_crv(), 0);
    killv = new sean_Visitor(sean_Visitor::release_comp_req, new_msg);
  }
 
  killv->set_oport( sv->get_iport() );
  killv->set_crv( sv->get_crv() );
  killv->set_cid( sv->get_cid() );
  killv->set_shared_app_id( sv->get_shared_app_id() );
  killv->set_oport( sv->get_iport() );

  Terminal::Inject( killv );

  // Suicide the original 
  sv->Suicide();
}

// -----------------------------------------------
// -----------------------------------------------
// -----------------------------------------------

PortAccessor::PortAccessor(bool passive) : _passive(passive)
{ 
  if (!_conn_type)
    _conn_type = (VisitorType *)QueryRegistry(CONNECTOR_VISITOR_NAME);
  if (!_sean_type)
    _sean_type = (VisitorType *)QueryRegistry(SEAN_VISITOR_NAME);
  if (!_data_type)
    _data_type = (VisitorType *)QueryRegistry(ATM_PDU_VISITOR_NAME);
}

// -----------------------------------------------
PortAccessor::~PortAccessor() { }

// -----------------------------------------------
bool PortAccessor::Broadcast(Visitor * v)
{
  return false;
}

// -----------------------------------------------
bool PortAccessor::Add(Conduit * c, Visitor * v)
{
  int port = -1;

  if (v->GetType().Is_A(_conn_type))
    port = ((ConnectorVisitor *)v)->GetPort();

  if (port != -1) {
    if (!_route.lookup( port ))
      _route.insert(port, c);
    else
      diag("daemon.portmux", DIAG_FATAL, "PortMux: Port %d is already in use!\n", port);
    return true;
  }
  return false;
}

// -----------------------------------------------
bool PortAccessor::Del(Conduit * c)
{
  DIAG("daemon.portmux", DIAG_DEBUG, cout <<  "PortAccessor::Del -- Not supported\n");
  return false;
}

// -----------------------------------------------
bool PortAccessor::Del(Visitor * v)
{
  DIAG("PortMux.cc", DIAG_DEBUG, cout <<  "PortAccessor::Del -- Not supported\n");
  return false;
}

// -----------------------------------------------
Conduit * PortAccessor::GetNextConduit(Visitor * v)
{ 
  if (v->GetType().Is_A(_sean_type)) {
    sean_Visitor * sv = (sean_Visitor *)v;
    dic_item di;
    if (di = _route.lookup(sv->get_oport()))
      return _route.inf(di);
  }
  else 
    {
      if (v->GetType().Is_A(_data_type)) {
	ATM_PDU_Visitor * atv = (ATM_PDU_Visitor*)v;
	dic_item di;
	if (di = _route.lookup( atv->GetPort() ))
	  return _route.inf(di);
      }
    }
  return 0;
}

// -----------------------------------------------
void PortAccessor::AddBinding(int port, Conduit * c)
{
  if (!_route.lookup(port))
    _route.insert(port, c);
  else
    diag("daemon.portmux", DIAG_FATAL, "PortMux: Port %d is already in use!\n", port);
}

// ------------------------------------------------------
// ------------------------------------------------------
// ------------------------------------------------------

ConnectorVisitor::ConnectorVisitor(Conduit * c, int port) 
  : Visitor(_my_type), _c(c), _port(port) { SetLoggingOn(); }

// ------------------------------------------------------
ConnectorVisitor::~ConnectorVisitor() { }

// ------------------------------------------------------
const vistype & ConnectorVisitor::GetClassType(void) const
{  return _my_type;  }

// ------------------------------------------------------
const VisitorType ConnectorVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

// ------------------------------------------------------
int ConnectorVisitor::GetPort(void) const
{  return  _port;  }

// ------------------------------------------------------
void ConnectorVisitor::SetPort(int port)
{  _port = port;  }

// ------------------------------------------------------
Conduit * ConnectorVisitor::GetConduit(void) const
{  return _c; }

// ------------------------------------------------------
void      ConnectorVisitor::SetConduit(Conduit * c)
{  _c = c; }

// ------------------------------------------------------
void ConnectorVisitor::at(Mux * m, Accessor * a)
{
  switch (EnteredFrom()) {
    case A_SIDE:
      SetLast(CurrentConduit());
      if (a)
        Join(B_half(CurrentConduit()), A_half(_c), this, this);
      break;
    case B_SIDE:
    case OTHER:
      break;
  }
  Suicide();
}

Visitor * ConnectorVisitor::dup(void) const
{
  return new ConnectorVisitor(*this);
}

ConnectorVisitor::ConnectorVisitor(const ConnectorVisitor & rhs)
  : Visitor(rhs), _port(rhs._port), _c(rhs._c) { }
