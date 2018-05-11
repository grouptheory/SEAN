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
static char const _Fiber_cc_rcsid_[] =
"$Id: Fiber.cc,v 1.10 1999/04/09 20:34:39 marsh Exp $";
#endif
#include <common/cprototypes.h>

#include "Fiber.h"
#include <sean/cc/sean_Visitor.h>
#include <sean/daemon/ATM_PDU_Visitor.h>

#include <sean/templates/constants.h>
#include <codec/q93b_msg/generic_q93b_msg.h>
#include <codec/uni_ie/ie_base.h>
#include <codec/uni_ie/UNI40_bbc.h>
#include <codec/uni_ie/conn_id.h>
#include <codec/uni_ie/end_pt_ref.h>
#include <codec/q93b_msg/generic_q93b_msg.h>

const VisitorType * Fiber::_uni_visitor_type = 0;
const VisitorType * Fiber::_data_vistype = 0;


//------------------------------------------
Fiber::Fiber(int portstampa, int portstampb, double t) : State( true ),  // queueing is on
  _portstampa(portstampa), _portstampb(portstampb)
{
  if (!_uni_visitor_type)
    _uni_visitor_type = QueryRegistry(SEAN_VISITOR_NAME);  
   
  if (!_data_vistype)
    _data_vistype = QueryRegistry(ATM_PDU_VISITOR_NAME);
}

//------------------------------------------
State * Fiber::Handle(Visitor * v) 
{
  Visitor::which_side side = VisitorFrom(v);

  VisitorType vt = v->GetType();
  
  assert (vt.Is_A(_uni_visitor_type) || vt.Is_A(_data_vistype) );

  if (vt.Is_A(_data_vistype)) {
    ATM_PDU_Visitor* atv = (ATM_PDU_Visitor*)v;
    switch(side) {
    case Visitor::A_SIDE:
      atv->SetPort(_portstampb);
      break;
	
    case Visitor::B_SIDE:
      atv->SetPort(_portstampa);
      break;

    default:
      abort();
    }
    PassThru(atv);
  }
  else // sean_Visitor
    {
      sean_Visitor * qv = (sean_Visitor *)v;
      sean_Visitor::sean_Visitor_Type t;

      rebuild( qv, packetize(qv,t) , t);   // Abracadabra!!!

      switch(side) {
      case Visitor::A_SIDE:
	qv->set_iport(_portstampb);
	qv->set_oport(_portstampb);
	PassVisitorToB(qv);
	break;

      case Visitor::B_SIDE:
	qv->set_oport(_portstampa);
	qv->set_iport(_portstampa);
	PassVisitorToA(qv);
	break;

      default:
	abort();
      }
    }

  return this;
}

//------------------------------------------
void Fiber::Interrupt(class SimEvent * e) { }


//------------------------------------------
Fiber::~Fiber() { }


//------------------------------------------
generic_q93b_msg* Fiber::packetize(sean_Visitor* sv, 
				   sean_Visitor::sean_Visitor_Type& t) {

  // Leave the CREF, FLAG untouched.

  sv->set_vpi( NO_VPI );
  sv->set_vci( NO_VCI );

  sv->set_epr( NO_EPR );
  sv->set_cid( NO_CID );
  sv->set_sid( NO_SID );
  sv->set_lid( NO_LID );
  sv->set_iport( NO_PORT );
  sv->set_oport( NO_PORT );
  sv->set_shared_app_id( NO_APPID );

  generic_q93b_msg* msg = sv->share_message();

  return msg;
}


//------------------------------------------
sean_Visitor* Fiber::rebuild(sean_Visitor* sv, 
			     generic_q93b_msg* m, 
			     sean_Visitor::sean_Visitor_Type t) {

  sv->Transform_into_ind();

  ie_conn_id* vpvc_ie = 0;
  InfoElem ** ies = m->get_ie_array();
  if (ies[InfoElem::ie_conn_identifier_ix]) {
    vpvc_ie = (ie_conn_id*) ( ies[InfoElem::ie_conn_identifier_ix] );
    
    sv-> set_vpi( vpvc_ie->get_vpi() );   // if we can, get the VPVC
    sv-> set_vci( vpvc_ie->get_vci() );
  }

  //  if(VisitorFrom(sv) == Visitor::A_SIDE)
  //    m->UniDump("A<: ");
  //  else
  //   m->UniDump("B<: ");

  char name[80];
  sv->Name(name);
  int icrv = m->get_crv();
  sv-> set_crv( m->get_crv() );
  sv-> set_flg( ! m->get_crf() );  // toggle the flag
  int ocrv = m->get_crv();

#ifdef __CRAP__
  ie_end_pt_ref *epr_ie = 0;
  if(epr_ie = (ie_end_pt_ref *)(m->ie(InfoElem::ie_end_pt_ref_ix)))
    {
      int iepr = epr_ie->get_epr_value();
      int oepr = iepr ^ 0x8000;
      sv->set_pid(oepr) ;
      epr_ie->set_epr_value(oepr);

      // do it in the encoded message as well --
      u_char *pdu = m->encoded_buffer();
      // search for EPR and toggle
      int msg_len = pdu[7] << 8 | pdu[8];
      u_char *bptr = &pdu[9];
      int epr_id = 0x54;
      while(msg_len > 0)
	{
	  if(bptr[0] != epr_id)
	    {
	      int ie_len = bptr[2] << 8 | bptr[3];
	      bptr += (ie_len + 4);
	      msg_len -= (ie_len + 4);
	      continue;
	    }
	  // we got it
	  bptr[5] ^= 0x80;
	  break;
	}
      if(VisitorFrom(sv) == Visitor::A_SIDE)
	m->UniDump("out to B>: ");
      else
	m->UniDump("out to A>: ");
      cout << endl;
    }
#endif

  return sv;
}

