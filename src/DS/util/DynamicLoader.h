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
#ifndef __DYNAMICLOADER_H__
#define __DYNAMICLOADER_H__

#ifndef LINT
static char const _DynamicLoader_h_rcsid_[] =
"$Id: DynamicLoader.h,v 1.2 1999/04/14 14:23:17 mountcas Exp $";
#endif

typedef void (* DLPROC) ( void );

class DynamicLoader {
public:

  DynamicLoader( void );
  DynamicLoader( const char * library );
  
  virtual ~DynamicLoader( );

  bool   Load(   const char * library );
  bool   Unload( void );
  DLPROC Lookup( const char * procedure ) const;
#ifdef _WIN32
  DLPROC Lookup( const int ordinal ) const;
#endif

private:

#ifndef _WIN32
  void * _libHandle;
#else
  HANDLE _libHandle;
#endif
};

#endif
