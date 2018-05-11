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
#ifndef __CALL_CONTROL_H__
#define __CALL_CONTROL_H__

#ifndef LINT
static char const _call_control_h_rcsid_[] =
"$Id: call_control.h,v 1.8 1999/02/09 18:46:15 mountcas Exp $";
#endif

#include <FW/actors/State.h>
#include <sean/cc/sean_Visitor.h>
#include <codec/uni_ie/cause.h>
#include <DS/containers/dictionary.h>

class IPC_DataVisitor;
class IPC_SignallingVisitor;
class sean_Visitor;
class call_info;
class serv_info;

#include <sean/templates/constants.h>


#define PHASE_EPR_ADDITION 1
#define PHASE_EPR_DELETION 2


class call_control : public State {
public:

  call_control(int s, int port = NO_PORT);

  State * Handle(Visitor * v);
  void    Interrupt(class SimEvent * e);

  bool streamfilter_data_query(int vpi, int vci, int& cid, abstract_local_id*& locid);

protected:

  virtual ~call_control();
  void ForwardMessageToAPI(sean_Visitor *& sv);
  void ForwardMessageToUNI(sean_Visitor *& sv);
  void SendMessageToUNI(sean_Visitor * sv, 
			sean_Visitor::sean_Visitor_Type t,
			bool self_inject=false);
  void SendMessageToAPI(sean_Visitor * sv, 
			sean_Visitor::sean_Visitor_Type t);

  void Failure_SendRelease(sean_Visitor * sv, 
			   ie_cause::CauseValues c,
			   bool self_inject=false);
  void Failure_SendReleaseComplete(sean_Visitor * sv, 
				   ie_cause::CauseValues c,
				   bool self_inject=false);
  void Failure_SendLeafSetupFailure(sean_Visitor * sv, 
				    ie_cause::CauseValues c,
				    bool self_inject=false);

  // Visitors from above
  State * ipc_data_visitor_from_A(IPC_DataVisitor * dv);
  State * ipc_signalling_visitor_from_A(IPC_SignallingVisitor * iv);
  State * visitor_arrived_from_A_side(Visitor * v);
  State * sean_visitor_from_A(sean_Visitor * sv);
  // Visitors from below
  State * sean_visitor_from_B(sean_Visitor * sv);
  State * visitor_arrived_from_B_side(Visitor * v);

private:

  void CheckP2MP(sean_Visitor * sv);
  void Maintain_CRV_To_EPR_Table( Visitor* v , int phase );

  bool is_related(const sean_Visitor * sv);
  bool relate(const sean_Visitor * sv);
  bool unrelate(const sean_Visitor * sv);

  call_info & get_related_call_info(const sean_Visitor * sv) const;
  serv_info & get_related_serv_info(const sean_Visitor * sv) const;
  
  bool attempt_final_app_death_cleanup(const abstract_local_id * id);

  // --------------------------------------------------------------------  
  static const    int  _buffer_size = 4096;
  unsigned char _buffer[ call_control::_buffer_size ];
  
  dictionary< const abstract_local_id * , list<call_info *> * > _callids;
  dictionary< const abstract_local_id * , list<serv_info *> * > _servids;
  dictionary< const abstract_local_id * , int > _app_alive;

  dictionary< int, call_info * >   _crv2callinf;
  dictionary< int, sean_Visitor *> _cref2setup;

  int _next_available_cref; // for outgoing calls
  int _location;
  int _port;
  // allows us to send a call proceeding message

  bool           _no_bind;
  bool           _has_ipc_above;
  int            _next_sequence_number;

  static const VisitorType * _sean_vistype;
  static const VisitorType * _ipc_data_vistype;
  static const VisitorType * _ipc_sig_vistype;
};

#endif // __CALL_CONTROL_H__
