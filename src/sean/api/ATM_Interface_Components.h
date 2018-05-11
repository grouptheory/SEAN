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

#ifndef __ATM_INTERFACE_COMPONENTS_H__
#define __ATM_INTERFACE_COMPONENTS_H__
#ifndef LINT
static char const _ATM_Interface_Components_h_rcsid_[] =
"$Id: ATM_Interface_Components.h,v 1.3 1998/08/21 14:40:14 mountcas Exp $";
#endif

#include <FW/actors/Expander.h>
#include <FW/actors/Accessor.h>
#include <FW/actors/Creator.h>
#include <FW/actors/Terminal.h>
#include <FW/actors/State.h>

#include <DS/containers/dictionary.h>

class Conduit;
class ATM_Call;
class ATM_Service;
class ATM_Leaf;
class ATM_Interface;
class abstract_local_id;
class IPC_DataVisitor;
class appid_and_int;

//------------------------------------------------
#define API_SERVS_OUTER_MUX "Services_OuterMux"
#define API_SERVS_FACTORY   "Services_Factory"
#define API_SERVS_INNER_MUX "Services_InnerMux"

class API_Services_Block : public Expander {
public:
  API_Services_Block(ATM_Interface* i);

  virtual Conduit *GetAHalf(void) const;
  virtual Conduit *GetBHalf(void )const;

protected:
  virtual ~API_Services_Block();

private:
  Conduit*       _sid_outside_mux;
  Conduit*       _service_factory;
  Conduit*       _sid_inside_mux;
};

//------------------------------------------------

class API_ServiceFactory : public Creator {
public:  
  API_ServiceFactory(ATM_Interface* i);
  virtual Conduit * Create(Visitor * v);

  void Interrupt(SimEvent* e);
  
protected:

  virtual ~API_ServiceFactory();
  
private:
  ATM_Interface* _master_interface;
  int            _next_allocated_sid;
  static const VisitorType* _sean_visitor_type;
};

//------------------------------------------------

#define API_CALLS_OUTER_MUX "Calls_OuterMux"
#define API_CALLS_FACTORY   "Calls_Factory"
#define API_CALLS_INNER_MUX "Calls_InnerMux"

class API_Calls_Block : public Expander {
public:
  API_Calls_Block(ATM_Interface* i);

  virtual Conduit *GetAHalf(void) const;
  virtual Conduit *GetBHalf(void )const;

protected:
  virtual ~API_Calls_Block();

private:
  Conduit*       _cid_outside_mux;
  Conduit*       _call_factory;
  Conduit*       _cid_inside_mux;
};

//------------------------------------------------

class API_CallFactory : public Creator {
public:  
  API_CallFactory(ATM_Interface* i);
  virtual Conduit * Create(Visitor * v);

  void Interrupt(SimEvent* e);
  
protected:

  virtual ~API_CallFactory();
  
private:
  ATM_Interface* _master_interface;
  int            _next_allocated_cid;
  static const VisitorType* _sean_visitor_type;
};

//------------------------------------------------

#define API_PARTY_OUTER_MUX "Party_OuterMux"
#define API_PARTY_INNER_MUX "Party_InnerMux"
#define API_PARTY_FACTORY   "Party_Factory"
#define API_CALL_STATE      "Call_State"

class API_Single_Call : public Expander {
public:
  State* Handle(Visitor* v);
  void Interrupt(class SimEvent *e);

  API_Single_Call(ATM_Interface* i, int id);

  virtual Conduit *GetAHalf(void) const;
  virtual Conduit *GetBHalf(void )const;

protected:
  virtual ~API_Single_Call();

private:
  Conduit*       _lid_outside_mux;
  Conduit*       _leaf_factory;
  Conduit*       _lid_inside_mux;
  Conduit*       _call;
};

//------------------------------------------------

class API_PartyFactory : public Creator {
public:  
  API_PartyFactory(ATM_Interface* i);
  virtual Conduit * Create(Visitor * v);

  void Interrupt(SimEvent* e);
  
protected:

  virtual ~API_PartyFactory();
  
private:
  ATM_Interface* _master_interface;
  static const VisitorType* _sean_visitor_type;
};

//------------------------------------------------
//------------------------------------------------

class API_Translator : public State {
public:
  API_Translator(int port, ATM_Interface* owner = 0, const char* machine = 0);

  State* Handle(Visitor* v);
  void Interrupt(class SimEvent *e);

protected:
  virtual ~API_Translator();

  enum { INT_SIZE = 4 };
  enum { IPC_HEADER_OFFSET = 4 * INT_SIZE };

private:
  bool _alive;
  int  _port;
  char _machine[255];
  abstract_local_id * _local_id;

  list<IPC_DataVisitor*> _waiting_to_be_sent;

  ATM_Interface*  _owner;

  static const VisitorType* _sean_visitor_type;
  static const VisitorType* _ipc_signalling_type;
  static const VisitorType* _ipc_data_type;
};


//------------------------------------------------

class API_Op_Accessor : public Accessor {  
public:
  API_Op_Accessor(void);
  virtual Conduit * GetNextConduit(Visitor * v);
  
protected:

  virtual ~API_Op_Accessor();

  virtual bool Broadcast(Visitor * v);
  virtual bool Add(Conduit * c, Visitor * v);
  virtual bool Del(Conduit * c);
  virtual bool Del(Visitor * v);
  
private:
  Conduit* _calls_side;
  Conduit* _services_side;
  static const VisitorType* _sean_visitor_type;
};

//------------------------------------------------

class API_Injector_Terminal : public Terminal {
public:
  API_Injector_Terminal(void);

  void Absorb(Visitor* v);
  void Interrupt(class SimEvent *e);

protected:
  virtual ~API_Injector_Terminal(void);

private:
  static const VisitorType* _sean_visitor_type;
};


//-------------------------------------------------------
//-------------------------------------------------------
class call_accessor: public Accessor{
public:

  call_accessor(void);
  virtual Conduit * GetNextConduit(Visitor * v);
  
protected:

  virtual ~call_accessor();

  virtual bool Broadcast(Visitor * v);
  virtual bool Add(Conduit * c, Visitor * v);
  virtual bool Del(Conduit * c);
  virtual bool Del(Visitor * v);
  
  dictionary<appid_and_int*, Conduit *>  _access_map;
  static const VisitorType* _sean_visitor_type;
};

//-------------------------------------------------------
//-------------------------------------------------------
class leaf_accessor: public Accessor{
public:

  leaf_accessor(void);
  virtual Conduit * GetNextConduit(Visitor * v);
  
protected:

  virtual ~leaf_accessor();

  virtual bool Broadcast(Visitor * v);
  virtual bool Add(Conduit * c, Visitor * v);
  virtual bool Del(Conduit * c);
  virtual bool Del(Visitor * v);
  
  dictionary<appid_and_int*, Conduit *>  _access_map;
  static const VisitorType* _sean_visitor_type;
};


#endif



