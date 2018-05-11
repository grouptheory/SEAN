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
static char const _ccd_cc_rcsid_[] =
"$Id: SigStack.cc,v 1.6 1998/08/21 15:04:10 mountcas Exp $";
#endif
#include <common/cprototypes.h>

#include "SigStack.h"

#include <FW/behaviors/Adapter.h>
#include <FW/behaviors/Cluster.h>
#include <FW/basics/diag.h>

#include <sean/cc/cc_layer.h>
#include <sean/cc/call_control.h>
#include <sean/uni_fsm/Coordinator.h>
#include <sean/uni_fsm/QueueState.h>
#include <sean/uni_fsm/q93b_expander.h>
#include <sean/qsaal/qsaal_expander.h>

#include <sean/daemon/DataStack.h>
#include <sean/daemon/Streams.h>


extern "C" {
#include <stdio.h>
#include <stdlib.h>
};

SigStack::SigStack(int s,
		   SigStack::type t,
		   int portnumber) : _side(s), _t(t)
{
  State    * qs = new QueueState(0.01);
  Protocol * qp = new Protocol(qs);
  Conduit  * qc = new Conduit (QUEUE_CONV_NAME,qp);

  q93b_expander * uni_exp   = new q93b_expander(s);
  Cluster       * uni_proto = new Cluster(uni_exp);
  Conduit       * uni_con   = new Conduit(UNI_LAYER_NAME,uni_proto);
  
  Conduit       * coord_con   = 0;
  Conduit       * qs_con      = 0;
  Conduit       * cc_con      = 0;
  Conduit       * lower_mux   = 0;
  Conduit       * upper_mux   = 0;
  Conduit       * datapath    = 0;
  

  if (s==NETWORK_SIDE) {
    
    if (t == SigStack::full_stack) {
      cout << "\n SEAN does not presently support full Network-Side Signalling.\n";
      cout << "\n      You can only instantiate Network-Side SigStack objects\n";
      cout << "\n      if you specify 'sans_aal' in the constructor.\n";
      cout << endl;
      abort();
    }
    _call_control = new call_control(NETWORK_SIDE, portnumber);
    State    * cc_state = _call_control;
    Protocol * cc_proto = new Protocol(cc_state);
               cc_con = new Conduit (CC_LAYER_NAME,cc_proto);
  }
  else {
    cc_layer_expander * cc_exp   = new cc_layer_expander(USER_SIDE, portnumber);
    Cluster           * cc_proto = new Cluster(cc_exp);
                        cc_con   = new Conduit(CC_LAYER_NAME, cc_proto);

    _call_control = cc_exp->Get_CC();
  }

  if (t == SigStack::full_stack) { 
    Coordinator   * coord_exp   = new Coordinator(s);
    Protocol      * coord_proto = new Protocol(coord_exp);
                    coord_con   = new Conduit(COORDINATOR_LAYER_NAME,coord_proto);

    QSAALExpander * qs_exp   = new QSAALExpander();
    Cluster       * qs_proto = new Cluster(qs_exp);
                    qs_con   = new Conduit(QSAAL_LAYER_NAME,qs_proto);
  }
  else {
    Accessor * down_acc  = new StackSelector( );
    Mux      * mux       = new Mux(down_acc);
               lower_mux = new Conduit(DOWN_MUX_NAME, mux);

    Creator * crea     = new DataStack( );
    Factory * fac      = new Factory(crea);
              datapath = new Conduit(DATAPATH_NAME, fac);

    Accessor * up_acc    = new StackSelector( );
               mux       = new Mux(up_acc);
               upper_mux = new Conduit(UP_MUX_NAME, mux);
  }

  Join(B_half(cc_con),  A_half(qc));
  Join(B_half(qc),      A_half(uni_con));

  if (t == SigStack::full_stack) { 
    Join(B_half(uni_con),   A_half(coord_con));
    Join(B_half(coord_con), A_half(qs_con));
  }
  else {
    Join(B_half(upper_mux), A_half(datapath));
    Join(B_half(datapath),  B_half(lower_mux));
  
    sean_Visitor * sv = new sean_Visitor(sean_Visitor::service_register_req);
    Join(B_half(upper_mux), A_half(cc_con), sv, 0);
    Join(B_half(lower_mux), B_half(uni_con), sv, 0);
    sv->Suicide();
  }

  _cc          = cc_con;
  _conv        = qc;
  _uni         = uni_con;
  _coordinator = coord_con;
  _qsaal       = qs_con;
  _datapath    = datapath;
  _lower_mux   = lower_mux;
  _upper_mux   = upper_mux;

  DefinitionComplete();
}

Conduit * SigStack::GetAHalf(void) const 
{
  if (_upper_mux)
    return A_half(_upper_mux); 
  else
    return A_half(_cc);
}

Conduit * SigStack::GetBHalf(void )const 
{
  if (_lower_mux)
    return A_half(_lower_mux);
  else 
    return B_half(_qsaal);
}

call_control* SigStack::Get_CC(void) {
  return _call_control;
}

SigStack::~SigStack(void) 
{
  delete _cc;
  delete _conv;
  delete _uni;
  delete _coordinator;
  delete _qsaal;
  delete _datapath;
  delete _lower_mux;
  delete _upper_mux;
}
