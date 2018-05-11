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
#ifndef __SORTSEQ_H__
#define __SORTSEQ_H__

#ifndef LINT
static char const _sortseq_h_rcsid_[] =
"$Id: sortseq.h,v 1.7 1999/01/26 18:04:13 mountcas Exp $";
#endif

#include <DS/containers/dictionary.h>

typedef association * seq_item;

template < class K, class I > 
class sortseq : public dictionary < K, I > {
public:

  sortseq(void);
  sortseq(const sortseq & rhs);
  virtual ~sortseq();

  seq_item min(void) const;
  seq_item max(void) const;

  sortseq & operator = (const sortseq & rhs);
};

#endif
