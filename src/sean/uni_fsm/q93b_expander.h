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

/** -*- C++ -*-
 * File: q93b_expander.h
 * @author 
 * @version $Id: q93b_expander.h,v 1.4 1998/08/21 15:35:47 mountcas Exp $
 * @memo
 * BUGS:
 */
#ifndef __Q93B_EXPANDER_H__
#define __Q93B_EXPANDER_H__

#ifndef LINT
static char const _q93b_expander_h_rcsid_[] =
"$Id: q93b_expander.h,v 1.4 1998/08/21 15:35:47 mountcas Exp $";
#endif

#include <sean/daemon/ccd_config.h>

#include <FW/actors/Accessor.h>
#include <FW/actors/Creator.h>
#include <FW/actors/State.h>
#include <FW/actors/Expander.h>

#include <FW/behaviors/Factory.h>
#include <FW/behaviors/Mux.h>
#include <FW/behaviors/Cluster.h>
#include <FW/behaviors/Protocol.h>

#include <DS/containers/dictionary.h>
#include <FW/basics/Conduit.h>
#include <FW/basics/Visitor.h>

class appid_and_int;
class sean_Visitor;

class Mux;
class Protocol;
class Addr;

#define Q93B_UPMUX_NAME       "CREF-UpperMux"
#define Q93B_UPMUX_NAME_USER  "CID_APPID-UpperMux"
#define Q93B_DOWNMUX_NAME     "CREF-LowerMux"
#define Q93B_FACTORY_NAME     "VC-alloc-Factory"
#define Q93B_LOWER_COORD_NAME "Lower-Coord"


class cid_accessor: public Accessor{
public:

  cid_accessor(void);
  virtual Conduit * GetNextConduit(Visitor * v);
  
protected:

  virtual ~cid_accessor();

  virtual bool Broadcast(Visitor * v);
  virtual bool Add(Conduit * c, Visitor * v);
  virtual bool Del(Conduit * c);
  virtual bool Del(Visitor * v);
  
  dictionary<appid_and_int*, Conduit *>  _access_map;
  static const VisitorType* _sean_visitor_type;
};



class lij_key {
public:
  enum dir {
    incoming_message =0,
    outgoing_message =1
  };

  lij_key(sean_Visitor* sv, dir d);
  ~lij_key();

  friend int compare(lij_key *const & c1, lij_key *const & c2);

private:
  Addr* _dest;
  Addr* _src;
  int   _seqnum;
  int   _cid;

  bool  _setup;
};



class cref_accessor: public Accessor {
public:

  cref_accessor(void);
  virtual Conduit * GetNextConduit(Visitor * v);
  void rebind( lij_key* key, int crv );

protected:

  virtual ~cref_accessor();
  virtual bool Broadcast(Visitor * v);
  virtual bool Add(Conduit * c, Visitor * v);
  virtual bool Del(Conduit * c);
  virtual bool Del(Visitor * v);
  dictionary<int, Conduit *>  _access_map;
  dictionary<lij_key*, Conduit *>  _temporary_map;

  static const VisitorType *  _sean_type;
};



class CallCreator: public Creator {
public:

  CallCreator(int s = USER_SIDE);
  virtual Conduit * Create(Visitor * v);
  void Interrupt(SimEvent* e);

protected:

  virtual ~CallCreator(void);  

private:

  int _side;
  static const VisitorType *  _sean_type;
};



class q93b_expander : public Expander {
public:

  q93b_expander(int s);
  virtual Conduit *GetAHalf(void) const;
  virtual Conduit *GetBHalf(void )const;

protected:

  virtual ~q93b_expander(void);

  Conduit * _upper_crefmux;
  Conduit * _q93b_factory;
  Conduit * _lower_crefmux;
  int _user;
};


#endif // 
