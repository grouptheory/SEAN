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
#ifndef __CID2CREF_H__
#define __CID2CREF_H__

#ifndef LINT
static char const _cid2cref_h_rcsid_[] =
"$Id: cid2cref.h,v 1.3 1998/08/12 20:15:45 bilal Exp $";
#endif

#include <FW/actors/State.h>
#include <FW/kernel/SimEvent.h>

#include <DS/containers/dictionary.h>

class VisitorType;
class appid_and_int;
class abstract_local_id;

#define CID2CREF_CODE 0xC1D2C8EF

class cid2crefSimEvent : public SimEvent {
public:

  cid2crefSimEvent(SimEntity * to, SimEntity * from, 
		   int cref, const abstract_local_id * ali, int cid);
  virtual ~cid2crefSimEvent(void);

  int                       get_cid(void) const; 
  int                       get_crv(void) const; 
  const abstract_local_id * get_appid(void) const;

  void set_crv(int cref);
  void set_cid(int cid);
  void set_appid(const abstract_local_id * cid_appid);

private:

  int                 _cref;
  abstract_local_id * _appid;
  int                 _cid;
};

//----------------------------------------------------------

class cid2cref : public State {
public:

  cid2cref(void);
  virtual ~cid2cref();

  State * Handle(Visitor * v);
  void    Interrupt(SimEvent * se);
  
  int             lookup(appid_and_int * cid_appid) const;
  appid_and_int * lookup(int cref) const;

  void add_binding(int cref, appid_and_int * cid_appid);
  void change_binding(int cref, appid_and_int * cid_appid);
  void remove_binding(int cref);
  void remove_binding(appid_and_int * cid_appid);
  
private:
  
  // cref to cid, appid
  dictionary< int, appid_and_int * > _cref2cid;
  dictionary< int, appid_and_int * > _seqnum2cid;
  int                                _next_cid;

  static const VisitorType         * _sean_vistype;
};

#endif // __CID2CREF_H__
