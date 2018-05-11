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
#ifndef __CC_LAYER_H__
#define __CC_LAYER_H__
#ifndef LINT
static char const _services_h_rcsid_[] =
"$Id: cc_layer.h,v 1.9 1998/08/21 15:26:57 mountcas Exp $";
#endif

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
#include <codec/uni_ie/cause.h>

#include "sean_Visitor.h"
#include "info.h"
#include "call_control.h"

#include <sean/templates/constants.h>


class setup_attributes;
class appid_and_int;
class sean_Visitor;
class abstract_local_id;

//-------------------------------------------------------
//-------------------------------------------------------
class networkside_serv_accessor: public Accessor{
public:

  networkside_serv_accessor(void);
  virtual Conduit * GetNextConduit(Visitor * v);
  
protected:

  virtual ~networkside_serv_accessor();

  virtual bool Broadcast(Visitor * v);
  virtual bool Add(Conduit * c, Visitor * v);
  virtual bool Del(Conduit * c);
  virtual bool Del(Visitor * v);
  
  dictionary<setup_attributes*, Conduit *>  _access_map;
  static const VisitorType* _sean_visitor_type;

  static dictionary<setup_attributes*, Conduit *>* _ptr;
  friend void mon(void);
};

//-------------------------------------------------------------
// NOTE: userside_serv_accessor is also instantiated in the API
//-------------------------------------------------------------
class userside_serv_accessor: public Accessor{
public:

  userside_serv_accessor(void);
  virtual Conduit * GetNextConduit(Visitor * v);
  
protected:

  virtual ~userside_serv_accessor();

  virtual bool Broadcast(Visitor * v);
  virtual bool Add(Conduit * c, Visitor * v);
  virtual bool Del(Conduit * c);
  virtual bool Del(Visitor * v);
  
  dictionary<appid_and_int* , Conduit *>  _access_map;
  static const VisitorType* _sean_visitor_type;
};

//-------------------------------------------------------
//-------------------------------------------------------
class serv_creator: public Creator{
public:
  
  serv_creator(void);
  virtual Conduit * Create(Visitor * v);

  void Interrupt(SimEvent* e);
  
protected:

  virtual ~serv_creator(void);
  
private:
  static const VisitorType* _sean_visitor_type;
};

//-------------------------------------------------------
//-------------------------------------------------------

#define ATTRIBUTESMUX_NAME     "Attr_Mux"
#define SIDAPPIDMUX_NAME       "SID_Mux"
#define SERVFACTORY_NAME       "ServFact"
#define CALLCONTROL_NAME       "CallCont"
#define CID2CREF_NAME          "CID-CREF"

class cc_layer_expander : public Expander {
public:  
  cc_layer_expander(int side, int portnumber = NO_PORT );

  virtual Conduit *GetAHalf(void) const;
  virtual Conduit *GetBHalf(void )const;

  call_control * Get_CC(void);

protected:

  virtual ~cc_layer_expander(void);

  call_control* _cc_state;
  Conduit * _call_control;

  Conduit * _attributes_mux;
  Conduit * _sid_appid_mux;
  Conduit * _serv_factory;
  Conduit * _cid2cref_map;

  int _side;
};

//-------------------------------------------------------
//-------------------------------------------------------

class sean_Service : public State {
public:

  sean_Service(sean_Visitor* birther);

  virtual bool Will_You_Accept(Visitor* examine);

  State* Handle(Visitor* v);
  void Interrupt(class SimEvent *e);

protected:

  virtual void on_destruction(void);

  virtual ~sean_Service();

  const abstract_local_id * _appid;
  int                       _sid;
  bool                      _active;

  bool _initial_joining_phase;

  static const VisitorType * _sean_type;
};


//----------------

class LIJ_Root_Sensing_Service : public sean_Service {
public:

  LIJ_Root_Sensing_Service(sean_Visitor* birther);

  virtual bool Will_You_Accept(Visitor* examine);

  State* Handle(Visitor* v);
  void Interrupt(class SimEvent *e);

protected:

  virtual void on_destruction(void);

  virtual ~LIJ_Root_Sensing_Service();

  generic_q93b_msg * _delay_setup_msg;
  sean_Visitor     * _delay_setup_visitor;

  int                _cid;

  static const VisitorType * _sean_type;
};



//----------------

class LIJ_Leaf_Sensing_Service : public sean_Service {
public:

  LIJ_Leaf_Sensing_Service(sean_Visitor* birther);

  virtual bool Will_You_Accept(Visitor* examine);

  State* Handle(Visitor* v);
  void Interrupt(class SimEvent *e);

protected:

  virtual void on_destruction(void);

  virtual ~LIJ_Leaf_Sensing_Service();

  int                _cid;

  static const VisitorType * _sean_type;
};


#endif
