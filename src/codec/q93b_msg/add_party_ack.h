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
#ifndef _ADD_PARTY_ACK_H_
#define _ADD_PARTY_ACK_H_

#ifndef LINT
static char const _add_party_ack_h_rcsid_[] =
"$Id: add_party_ack.h,v 1.12 1998/12/15 15:20:09 mountcas Exp $";
#endif

#include <codec/q93b_msg/generic_q93b_msg.h>

class Buffer;

class q93b_add_party_ack_message : public generic_q93b_msg {
protected:

  enum {    // H E
    min_size = 9+4,
            // H E 2 GI GI GI N BL BL BL AL
    max_size = 9+7+7+33+33+33+4+17+17+17+21
  };

  virtual int min_len(void) const;
  virtual int max_len(void) const;

public:

  q93b_add_party_ack_message(q93b_add_party_ack_message & him);
  q93b_add_party_ack_message(InfoElem ** ie_array, u_long call_ref_value, u_int flag);
  q93b_add_party_ack_message(InfoElem ** ie_array, u_long call_ref_value,
			     u_int flag, Buffer* buf);

  q93b_add_party_ack_message(const u_char * buf, int len);

  virtual void SetupLegal(void);
  virtual void SetupMandatory(void);
  generic_q93b_msg * copy(void);
};

#endif  // _ADD_PARTY_ACK_H_
