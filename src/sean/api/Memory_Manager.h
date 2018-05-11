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
#ifndef __MEMORY_MANAGER_H__
#define __MEMORY_MANAGER_H__
#ifndef LINT
static char const _MEMORY_MANAGER_H_rcsid_[] =
"$Id: Memory_Manager.h,v 1.1 1998/07/20 11:10:34 bilal Exp $";
#endif


class ATM_Call;
class ATM_Service;
class ATM_Leaf;
class ATM_Interface;


class Memory_Manager;

class Recyclable {
  virtual void Suicide(void) = 0;
  bool User_Accessible(void) const;

protected:
  Memory_Manager& The_Recycle_Bin(void);

  Recyclable(void);
  ~Recyclable();

private:
  friend class Memory_Manager;
  void Mark_Inaccessible(void);
  virtual bool Ready_To_Die(void) = 0;

  bool _user_accessible;
};



class Memory_Manager {
public:
  void Digest_Call(ATM_Call* x);
  void Digest_Service(ATM_Service* x);
  void Digest_Leaf(ATM_Leaf* x);
  void Digest_Interface(ATM_Interface* x);

  friend Memory_Manager& Recyclable::The_Recycle_Bin(void);

private:
  list<ATM_Call*>      _call_trash;
  list<ATM_Service*>   _service_trash;
  list<ATM_Leaf*>      _leaf_trash;
  list<ATM_Interface*> _interface_trash;

  void Dump(void);

  list<ATM_Call*>      _call_ready_to_die;
  list<ATM_Service*>   _service_ready_to_die;
  list<ATM_Leaf*>      _leaf_ready_to_die;
  list<ATM_Interface*> _interface_ready_to_die;

  Memory_Manager(void);
  ~Memory_Manager();

  static Memory_Manager* _singleton;
};


#endif
