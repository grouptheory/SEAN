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
#ifndef __SEAN_VISITOR_H__
#define __SEAN_VISITOR_H__

#ifndef LINT
static char const _sean_Visitor_h_rcsid_[] =
"$Id: sean_Visitor.h,v 1.5 1998/08/21 14:43:27 mountcas Exp $";
#endif

#include <FW/basics/Visitor.h>

class abstract_local_id;
class generic_q93b_msg;
class appid_and_int;
class setup_attributes;
class Buffer;
class Q93bVisitor;

#define SEAN_VISITOR_NAME "SEAN_Visitor"

#define DECLARE_SEAN_VISITOR_TYPES       \
enum sean_Visitor_Type {                 \
    unknown = 0,                         \
                                         \
    setup_req =  1,                      \
    setup_ind = -1,                      \
                                         \
    call_proc_req = 2,                   \
    call_proc_ind = -2,                  \
                                         \
    connect_req = 3,                     \
    connect_ind = -3,                    \
                                         \
    release_req =  4,                    \
    release_ind = -4,                    \
                                         \
    connect_ack_req = 5,                 \
    connect_ack_ind = -5,                \
                                         \
    setup_comp_req = 6,                  \
    setup_comp_ind = -6,                 \
                                         \
    release_comp_req = 7,                \
    release_comp_ind = -7,               \
                                         \
    status_enq_req = 8,                  \
    status_enq_ind = -8,                 \
                                         \
    status_req = 9,                      \
    status_ind = -9,                     \
                                         \
    add_party_req = 10,                  \
    add_party_ind = -10,                 \
                                         \
    add_party_comp_req = 11,             \
    add_party_comp_ind = -11,            \
                                         \
    add_party_ack_req = 12,              \
    add_party_ack_ind = -12,             \
                                         \
    add_party_rej_req = 13,              \
    add_party_rej_ind = -13,             \
                                         \
    drop_party_req = 14,                 \
    drop_party_ind = -14,                \
                                         \
    drop_party_comp_req = 15,            \
    drop_party_comp_ind = -15,           \
                                         \
    drop_party_ack_req = 16,             \
    drop_party_ack_ind = -16,            \
                                         \
    leaf_setup_req = 17,                 \
    leaf_setup_ind = -17,                \
                                         \
    leaf_setup_failure_req = 18,         \
    leaf_setup_failure_ind = -18,        \
                                         \
    restart_req = 19,                    \
    restart_ind = -19,                   \
                                         \
    restart_resp = 20,                   \
    restart_comp_ind = -20,              \
                                         \
    restart_ack_req = 21,                \
    restart_ack_ind = -21,               \
                                         \
    service_register_req = 100,          \
    service_register_accept = 101,       \
    service_register_deny = 102,         \
    service_dereg_req = 103,             \
    service_dereg_accept = 104,          \
    service_dereg_deny = 105,            \
				         \
    api_call_construction = 200,         \
    api_call_destruction = 201,          \
    api_service_construction = 202,      \
    api_service_destruction = 203,       \
    api_leaf_construction = 204,         \
    api_leaf_destruction = 205,          \
                                         \
    api_setup_ind_response_to_lsr = 206, \
                                         \
    raw_simulation_pdu = 999             \
  };


class sean_Visitor : public Visitor {  
  friend class Toggler;
  friend class call_control;

public:
  DECLARE_SEAN_VISITOR_TYPES;

  sean_Visitor(sean_Visitor_Type t, generic_q93b_msg* msg=0, Buffer* buf=0);
  sean_Visitor(Buffer * buf);

  appid_and_int * get_cid_key(void) const;
  appid_and_int * get_sid_key(void) const;
  appid_and_int * get_lid_key(void) const;
  
  const abstract_local_id * get_shared_app_id(void) const;

  int get_cid(void) const;
  int get_sid(void) const;
  int get_lid(void) const;
  int get_crv(void) const;
  int get_flg(void) const;
  // added 7-8-98 - mountcas
  int get_iport(void) const;
  int get_oport(void) const;
  int get_vpi(void) const;
  int get_vci(void) const;
  int get_pid(void) const;
  int get_epr(void) const;

  setup_attributes * get_setup_attributes(void) const;

  void set_shared_app_id(const abstract_local_id * x);
  void set_cid(const int x);
  void set_sid(const int x);
  void set_lid(const int x);
  void set_crv(const int x);
  void set_flg(const int x);
  // added 7-8-98 - mountcas
  void set_iport(const int x);
  void set_oport(const int x);
  void set_vpi(const int x);
  void set_vci(const int x);
  void set_pid(const int x);
  void set_epr(const int x);

  Buffer * take_buffer(void);
  bool Valid(void);

  sean_Visitor_Type get_op(void) const;
  const char *      op_name(void) const;
  bool is_service_related(void) const;
  bool is_call_related(void) const;

  const VisitorType GetType(void) const;

  friend sean_Visitor * Transform_into_sean(Q93bVisitor * qv);
  friend Q93bVisitor  * Transform_into_Q93b(sean_Visitor * sv);
  sean_Visitor * Clone(sean_Visitor_Type t);

  void Transform_into_req(void);
  void Transform_into_ind(void);
  bool is_req(void);
  bool is_ind(void);

  void mark_setup_ind_as_response_to_lsr(void);

  bool IsP2MP(void) const;

  generic_q93b_msg * take_message(void);
  generic_q93b_msg * share_message(void);
  void               give_message(generic_q93b_msg *& new_msg);

  void Name(char* buf) const;

protected:

  virtual ~sean_Visitor();

  void SetP2MP(bool mp);

  virtual Visitor * dup(void) const;
  sean_Visitor(const sean_Visitor & rhs);

private:

  int                       _crv;
  int                       _flg;
  int                       _cid;
  int                       _sid;
  int                       _lid;
  // added 7-8-98 - mountcas
  int                       _iport;
  int                       _oport;
  int                       _vpi;
  int                       _vci;
  int                       _epr;

  bool                      _p2mp;

  const abstract_local_id * _appid;
  sean_Visitor_Type         _op;
  generic_q93b_msg        * _msg;
  Buffer                  * _buf;

  virtual const vistype & GetClassType(void) const;
  static vistype         _my_type;

  void Fill_Subtype_Name(char* buf) const;
};

void op_to_str(char* buf, sean_Visitor::sean_Visitor_Type t);

#endif  // __SEAN_VISITOR_H__
