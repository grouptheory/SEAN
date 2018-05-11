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
#ifndef LINT
static char const _DynamicLoader_cc_rcsid_[] =
"$Id: DynamicLoader.cc,v 1.2 1999/04/14 14:23:33 mountcas Exp $";
#endif

#include "DynamicLoader.h"

#ifndef _WIN32
#include <dlfcn.h>
#else
#define NOGDI
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

DynamicLoader::DynamicLoader( void ) 
  : _libHandle( 0 ) { }

DynamicLoader::DynamicLoader( const char * library ) 
  : _libHandle( 0 ) 
{ 
  Load( library );
}

DynamicLoader::~DynamicLoader( ) 
{
  Unload( );
}

bool DynamicLoader::Load( const char * library )
{
  if ( library != 0 && *library ) {
    if ( _libHandle != 0 )
      Unload( );

#ifndef _WIN32
    _libHandle = ::dlopen( library, RTLD_LAZY );
#else
    _libHandle = ::LoadLibrary( library );
#endif
  }

  return ( _libHandle != 0 );
}

bool DynamicLoader::Unload( void )
{
  if ( _libHandle != 0 ) {
#ifndef _WIN32
    ::dlclose( _libHandle );
#else
    ::FreeLibrary( _libHandle );
#endif
    _libHandle = 0;
  }

  return ( _libHandle == 0 );
}

DLPROC DynamicLoader::Lookup( const char * procedure ) const
{
  DLPROC rval = 0;

  if ( _libHandle != 0 ) {
#ifndef _WIN32
    rval = (DLPROC)::dlsym( _libHandle, procedure );
#else
    rval = (DLPROC)::GetProcAddress( _libHandle, procedure );
#endif
  }

  return rval;
}

#ifdef _WIN32
DLPROC DynamicLoader::Lookup( const int ordinal ) const
{
  DLPROC rval = 0;

  if ( _libHandle != 0 ) {
    rval = (DLPROC)::GetProcAddress( _libHandle, 
				     MAKEINTRESOURCE( ordinal ) );
  }

  return rval;
}
#endif
