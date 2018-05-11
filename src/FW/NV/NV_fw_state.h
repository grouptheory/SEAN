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
#ifndef __NV_FW_STATE_H__
#define __NV_FW_STATE_H__

#ifndef LINT
static char const _NV_fw_state_h_rcsid_[] =
"$Id: NV_fw_state.h,v 1.2 1999/02/24 21:49:47 mountcas Exp $";
#endif

#include <DS/containers/stack.h>
#include <DS/containers/dictionary.h>

class NV_visitor_registry;
class NV_visitor_colors_table;
class NV_conduit_registry;
class NV_conduit_pixmaps_table;
class NV_conduit;

char * ExtractName(const char * n);
char * ExtractType(const char * n);

class NV_fw_state {
  friend NV_fw_state * FWState(void);
  friend int eventNumber(const char * event);
public:

  int parse_and_update(char * cmd);
  bool push_time(double t);

  bool readXXX(void) const;
  bool searchingForEvent(void) const;
  void searchForEvent(int event);
  void addEvent(const char * event);

private:

  bool hasEvent(int event);

  NV_fw_state(void);
  ~NV_fw_state();

  NV_visitor_registry      * _visitor_registry;
  NV_visitor_colors_table  * _visitor_colors_table;
  NV_conduit_registry      * _conduits_registry;
  NV_conduit_pixmaps_table * _conduits_pixmap_table;
  double                     _simtime;
  stack<NV_conduit *>        _current_expanders;
  dictionary<char *, bool>   _event_log;
  int                        _next_event;
  bool                       _searching_for_event;

  static const int _STR_SIZE = 255;
  static const int _MAX_WORDS = 8;
  char      _chunk[_MAX_WORDS][_STR_SIZE];
  char      _scan_str[_STR_SIZE];

  bool                 _read_xxx;
  static NV_fw_state * _singleton;
};

#define PARSE_NOERR       0
#define PARSE_XXX_LINE    1
#define PARSE_TOKEN_FOUND 2

#endif // __NV_FW_STATE_H__
