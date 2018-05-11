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
#ifndef _Q93BLEAF_H_
#define _Q93BLEAF_H_
#ifndef LINT
static char const _Q93bLeaf_h_rcsid_[] =
"$Id: Q93bLeaf.h,v 1.1 1998/07/20 11:12:34 bilal Exp $";
#endif


#include <iostream.h>
#include <sys/types.h>
#include <sys/time.h>

#include <FW/kernel/Kernel.h>
#include <FW/actors/State.h>
#include <FW/kernel/Handlers.h>
#include <codec/uni_ie/ie.h>

class LeafTimer;

class Leaf {
  friend class Call;
public:
  enum LeafStates {
    Null = 0,
    LeafSetupInitiated = 1
  };
  Leaf(Call *ca, int seqnum);
  ~Leaf();
  void LeafSetupReq();
  void LeafSetupFailure();

  void set_called_number(ie_called_party_num *cn_ie);
  void set_called_subaddr(ie_called_party_subaddr *cs_ie);
  InfoElem* get_called_number(void);
  InfoElem* get_called_subaddr(void);
  void ExpT331();
  void SetT331();
  void StopT331();
  void ChangeStateToNull();
private:
  Call *_ca;
  LeafTimer *_t331;
  int _t331_retries;
  int _t331_max_retries;
  int _seqnum;
  LeafStates _cs;
  ie_called_party_num *_dest;
  ie_called_party_subaddr *_dest_subaddr;
};

#endif








