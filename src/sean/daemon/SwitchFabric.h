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
#ifndef __SWITCHFABRIC_H__
#define __SWITCHFABRIC_H__

#ifndef LINT
static char const _switchfabric_h_rcsid_[] =
"$Id: SwitchFabric.h,v 1.11 1998/08/25 21:21:56 mountcas Exp $";
#endif

#include <FW/actors/Terminal.h>
#include <DS/containers/list.h>
#include <DS/containers/dictionary.h>
#include <sean/templates/constants.h>
#include <FW/actors/Accessor.h>
#include <FW/behaviors/Mux.h>
#include <FW/basics/Conduit.h>
#include <FW/basics/Visitor.h>
#include <DS/containers/dictionary.h>

#define CONNECTOR_VISITOR_NAME "ConnectorVisitor"

class sean_Visitor;
class NSAP_DCC_ICD_addr;

#define STRONG_BINDING               4
#define WEAK_BINDING__SETUP_SEEN     2
#define WEAK_BINDING__DROP_SEEN      3
#define WEAK_BINDING__RELEASE_SEEN   1
#define UNSPECIFIED_BINDING_TYPE    -1

//----------------------------------------------------------
class Binding {
public:

  Binding(int ivpi, int ivci, int iport, int iepr, int icrv,
	  int ovpi = NO_VPI, int ovci = NO_VCI, int oport = NO_PORT, 
	  int oepr = NO_EPR, int ocrv = NO_CREF);

  ~Binding( );

  int GetiVPI(void) const;
  int GetiVCI(void) const;
  int GetiPort(void) const;
  int GetiEpr(void) const;
  int GetiCrv(void) const;
  int GetoVPI(void) const;
  int GetoVCI(void) const;
  int GetoPort(void) const;
  int GetoEpr(void) const;
  int GetoCrv(void) const;

  void SetoVPI(int ovpi);
  void SetoVCI(int ovci);
  void SetoPort(int oport);
  void SetoEpr(int epr);
  void SetoCrv(int crv);

  void SetiCrv(int crv);

  void Set_strength(int value);
   int Get_strength(void);

private:

  int _ivpi;
  int _ivci;
  int _iport;
  int _iepr;
  int _icrv;
  int _ovpi;
  int _ovci;
  int _oport;
  int _oepr;
  int _ocrv;

  int _strength;
};

//----------------------------------------------------------
class VCAlloc {
public:

  VCAlloc(void);
  ~VCAlloc( );

  // Returns the next available VC, 0 is a failure.
  unsigned int GetNewVCI(void);
  // Returns VC if the requested VC is available otherwise it returns 0.
  unsigned int RequestVCI(unsigned int vci);
  // Places the passed VC back on the free VC list.
  void ReturnVCI(unsigned int vci);         

private:

  // List of free VC's.
  list<unsigned int> _svc_free;
  // List of used VC's.
  list<unsigned int> _svc_in_use;
  // Used when the VCAllocator must allocate more VC's for its free list.
  unsigned int       _lowest;

  enum { 
    MinVCI = 32, 
    MinSize = 15 
  };
};

//----------------------------------------------------------
class Router : public Terminal {
public:

  Router(void);
  virtual ~Router( );

  virtual void Absorb(Visitor * v);
  virtual void Interrupt(SimEvent * event);
  void    Inject(Visitor * v);

  void RegisterAddr(NSAP_DCC_ICD_addr * addr, int port);
  int  AddrToPort(const NSAP_DCC_ICD_addr * a) const;

  Binding * AddBinding(int ivpi, int ivci, int iport, int iepr, int icrv,
		       int ovpi, int ovci, int oport, int oepr, int ocrv);
  Binding * FindEntry(sean_Visitor * sv, int min_strength) const;
  list<Binding *> * FindEntries(int port, int vp, int vc) const;

  void RT(int port);
  void Full_RT(void);

private:
  void Route_Sean_Visitor(sean_Visitor * sv, int min_strength);
  void Handle_Data_Visitor(Visitor* v);
  void No_Route_To_Destination(sean_Visitor* sv);

  int GetNextVCI(int port);
  Binding * LocateBinding(int p, int vp, int vc, 
			  int epr = NO_EPR, int crv = NO_CREF);
  Binding * OnExistingOutgoingBranch(int port, int vpi, int vci, int oport);
  
  void      RemoveBinding(Binding *& b);

  int LivingBranches(int port, int vp, int vc);

  // port to list of bindings for that port
  dictionary< int, list<Binding *> *>   _p2bind;
  // port to address of host connected on that port
  dictionary< NSAP_DCC_ICD_addr *, int> _addr2port;
  // port to VCAllocator for that port
  dictionary< int, VCAlloc *>           _p2vca;
  // keeps track of QoS
  // handles visitors of type sean_Visitor
  static VisitorType                  * _sean_vistype;
  static VisitorType                  * _data_vistype;
};

//----------------------------------------------------------
class ConnectorVisitor : public Visitor {
public:

  ConnectorVisitor(Conduit * c, int port);
  virtual ~ConnectorVisitor();

  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;

  int  GetPort(void) const;
  void SetPort(int port);

  Conduit * GetConduit(void) const;
  void      SetConduit(Conduit * );

  void at(Mux * m, Accessor * a);

protected:

  ConnectorVisitor(const ConnectorVisitor & rhs);
  virtual Visitor * dup(void) const;

private:

  int            _port; 
  Conduit      * _c;
  static vistype _my_type;
};

//----------------------------------------------------------
class PortAccessor : public Accessor {
public:

  PortAccessor(bool passive=false);
  virtual Conduit * GetNextConduit(Visitor * v);
  void AddBinding(int port, Conduit * c);

protected:

  virtual ~PortAccessor();
  virtual bool Broadcast(Visitor * v);
  virtual bool Add(Conduit * c, Visitor * v);
  virtual bool Del(Conduit * c);
  virtual bool Del(Visitor * v);

  bool _passive;

  dictionary< int, Conduit *> _route;
  static VisitorType * _conn_type;
  static VisitorType * _sean_type;
  static VisitorType * _data_type;
};

#endif
