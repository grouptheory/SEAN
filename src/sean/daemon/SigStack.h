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
#ifndef __SIGSTACK_H__
#define __SIGSTACK_H__

#ifndef LINT
static char const _sigstack_h_rcsid_[] =
"$$";
#endif

#include <sean/daemon/ccd_config.h>

#include <FW/actors/Expander.h>
#include <FW/basics/Conduit.h>
#include <FW/basics/Visitor.h>

#include <sean/templates/constants.h>


//-------------------------------------------------------
#define CC_LAYER_NAME            "Call-Control"
#define QUEUE_CONV_NAME          "Queue"
#define UNI_LAYER_NAME           "Uni"
#define COORDINATOR_LAYER_NAME   "Coord"
#define QSAAL_LAYER_NAME         "Qsaal"
#define UP_MUX_NAME              "StackSel-Upper"
#define DOWN_MUX_NAME            "StackSel-Lower"
#define DATAPATH_NAME            "DataPath"

class call_control;

//-------------------------------------------------------
class SigStack : public Expander {
public:

  enum type {
    full_stack,
    sans_aal
  };

  SigStack(int s = USER_SIDE,
	   type t = full_stack,
	   int portnumber = NO_PORT);

  virtual Conduit * GetAHalf(void) const;
  virtual Conduit * GetBHalf(void) const;

protected:

  call_control* Get_CC(void);

  virtual ~SigStack(void);
  
  call_control * _call_control;

  Conduit * _cc;  // if this is USER_SIDE, this will be a cc_layer
                  // if this is NETWORK_SIDE, this will be just a call_control

  Conduit * _conv;   // a queue-state
  Conduit * _uni;    // the q93b_expander

                            // only made if this is a full_stack
  Conduit * _coordinator;   // the link coordinator
  Conduit * _qsaal;         // the qsaal_expander

                            // only made if this is NOT a full_stack
  Conduit * _upper_mux;     // the upper stackselector mux
  Conduit * _lower_mux;     // the lower stackselector mux
  Conduit * _datapath;      // the data path

  int  _side;
  type _t;

  friend class Host;
};

#endif
