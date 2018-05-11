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
static char const _security_service_cc_rcsid_[] =
"$Id: security_service.cc,v 1.99 1999/04/15 16:06:31 mountcas Exp $";
#endif

#include "security_service.h"
#include <arpa/inet.h>
#include <iostream.h>

ie_security_service::ie_security_service( void ) 
  : InfoElem(ie_security_service_id) { }

ie_security_service::ie_security_service( const ie_security_service & rhs ) 
  : InfoElem(ie_security_service_id) { }

ie_security_service::ie_security_service( int /* ADD PARAMS HERE */ ) 
  : InfoElem(ie_security_service_id) { }

ie_security_service::~ie_security_service() { }

int ie_security_service::equals(const InfoElem * him) const 
{ 
  return (him->equals(this));
}

int ie_security_service::equals(const ie_security_service * himptr) const 
{ 
  // memberwise compare
  return 1;
}

int       ie_security_service::encode(u_char * buffer) 
{ 
  int length    = 0;
  u_char * bptr = buffer + ie_header_len;

  list_item li;
  forall_items( li, _security_assocs ) {
    security_association * sa = _security_assocs.inf( li );
    int len = sa->encode( bptr );
    bptr   += len;
    length += len;
  }
  // finally encode the header
  encode_ie_header( buffer, ie_security_service_id, length);
  // I'm not quite sure if I should incl header in length
  return length + ie_header_len;
}

InfoElem::ie_status ie_security_service::decode(u_char * buffer, int & id) 
{ 
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );
  int ident;

  id           = buffer[0];
  if ( id != ie_security_service_id ) r = InfoElem::bad_id;
  // buffer[1] contains the coding std, etc.
  short length = ( buffer[2] << 8 | buffer[3] ) & 0xFFFF;
  int   index  = 4;
  int   len    = 0;

  if ( length == 0 ) // empty contents
    r = InfoElem::empty;

  while ( r == ok && len < length ) {
    security_association * sa = new security_association( );
    r = sa->decode( buffer + index, ident );
    if ( r != ok )
      break;

    len += sa->getLength() + 3; // length plus id and length bytes

    addSecurityAssociation( sa );
  }
  if ( r == ok && len != length )
    r = invalid_contents;

  return r;
}

InfoElem * ie_security_service::copy(void) const 
{ 
  return new ie_security_service( *this );
}

int ie_security_service::operator == ( const ie_security_service & rhs ) const 
{ 
  return equals(&rhs);
}

int ie_security_service::Length( void ) const
{
  int buflen = ie_header_len;
  list_item li;
  forall_items( li, _security_assocs ) {
    security_association * sa = _security_assocs.inf( li );
    buflen += sa->getLength();
  }

  return buflen;
}

const u_char                  ie_security_service::getMaxSAID( void ) const
{
  u_char rval = 0;
  list_item li;
  forall_items( li, _security_assocs ) {
    // find the SAS with the highest first byte of the relative ID and return it;
    security_association * sa = _security_assocs.inf( li );
    if ( ((sa->getRelativeId() >> 8) & 0xFF) > rval )
      rval = ((sa->getRelativeId() >> 8) & 0xFF);
  }
  return rval;
}

void ie_security_service::addSecurityAssociation( security_association * & sa )
{
  int length = Length() + sa->getLength();

  // Maximum length of SSIE is 512 octets, page 5 of the UNI Signaling
  // 4.0 Security Addendum
  if ( length < 512 ) {
    _security_assocs.insert( sa );
    sa = 0;
  }
  // otherwise there has been a problem, adding this SA will exceed
  // the length of the maximum size of the SSIE.
}

const list< security_association * > * ie_security_service::getSecurityAssociations( void ) const
{
  return &_security_assocs;
}

const security_association * ie_security_service::getSecurityAssociation( short relativeID ) const
{
  const security_association * rval = 0;

  security_association tmp;
  tmp.setRelativeId( relativeID );
  list_item li;
  if ( li = _security_assocs.lookup( &tmp ) )
    rval = _security_assocs.inf( li );

  return rval;
}

security_association * ie_security_service::takeSecurityAssociation( short relativeID )
{
  security_association * rval = 0;

  security_association tmp;
  tmp.setRelativeId( relativeID );
  list_item li;
  if ( li = _security_assocs.lookup( &tmp ) ) {
    rval = _security_assocs.inf( li );
    _security_assocs.del_item( li );
  }

  return rval;
}

// If any of the security_associations requires in-band SME, then
// requiresInBandSME() returns true.  If none of them require in-band
// SME, then it returns false.
//
// BUG- the algorithm works only at the called party, the calling
// party, and wherever all security associations terminate at the
// caller.
bool ie_security_service::requiresInBandSME(void)
{
  bool answer = false;

  if (_security_assocs.size() > 0) {
    list_item li = _security_assocs.first();

    do {
      const security_association *sec_assoc = _security_assocs.inf(li);
      if (sec_assoc->getTransportIndicator() == security_association::InBand) {
	li = 0;
	answer = true;
      } else li = _security_assocs.next(li);

    } while (li != 0);
  }

  return answer;
}

// If any of the security_associations support signaled SME, then
// supportsSignaledSME() returns true.  If none of them support
// in-band SME then it returns false.
//
// BUG- the algorithm works only at the called party, the calling
// party, and wherever all security associations terminate at the
// caller.
bool ie_security_service::supportsSignaledSME(void)
{
  bool answer = false;

  if (_security_assocs.size() > 0) {
    list_item li = _security_assocs.first();

    do {
      const security_association * sec_assoc = _security_assocs.inf(li);
      if (sec_assoc->getTransportIndicator() == 
	  security_association::Signaling) {
	li = 0;
	answer = true;
      } else li = _security_assocs.next(li);

    } while (li != 0);
  }

  return answer;
}


char ie_security_service::PrintSpecific(ostream & os) const
{
  os << endl << "\tAssociations:" << endl;
  list_item li;
  forall_items( li, _security_assocs ) {
    security_association * sa = _security_assocs.inf( li );
    os << *sa << endl;
  }
  return 0;
}

ostream & operator << ( ostream & os, const ie_security_service & ssie )
{
  ssie.PrintGeneral( os );
  ssie.PrintSpecific( os );
  return os;
}

// ----------------------------------------------------------------------------
security_association::security_association( void ) 
  : _identifier( 0x01 ), _length( 0 ), _version( _VersionOne ),
    _trans_ind( Signaling ), _flow_ind( FirstFlow ), 
    _discard( DoNotDiscardAfterProcessing ), _scope( 0 ), _relative_id( 0 ),
    _target_se_id( 0 ), _ssd_section( 0 ) { }

security_association::security_association( int identifier )
  : _identifier( identifier ), _length( 0 ), _version( _VersionOne ),
    _trans_ind( Signaling ), _flow_ind( FirstFlow ), 
    _discard( DoNotDiscardAfterProcessing ), _scope( 0 ), _relative_id( 0 ),
    _target_se_id( 0 ), _ssd_section( 0 ) { }

security_association::~security_association( ) { }

void security_association::encode_sa_header(u_char * buffer, int length)
{
  buffer[0] = _identifier;
  short len = htons( length );
  buffer[1] = (len >> 8) & 0xFF;
  buffer[2] = (len & 0xFF);
}

int       security_association::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer + 3;

  *bptr++ = (( _version << 5 ) & 0xE0) | ((_trans_ind << 3) & 0x18) | 
            ((_flow_ind << 1) & 0x06) | (_discard & 0x01);
  *bptr++ = ( htons( _scope ) >> 8 ) & 0xFF;
  *bptr++ = ( htons( _scope ) & 0xFF );
  *bptr++ = ( htons( _relative_id ) >> 8 ) & 0xFF;
  *bptr++ = ( htons( _relative_id ) & 0xFF );
  length += 5;
  // The target se id is only included when the Explicit bit is set 
  // and all other bits are zeroed in the scope field.
  if ( _scope & 0x8000 && !(_scope & 0x7FFF) ) {
    assert( _target_se_id != 0 );
    int len = _target_se_id->encode( bptr );
    bptr   += len;
    length += len;
  }
  if ( _ssd_section != 0 )
    length += _ssd_section->encode( bptr );
  // finally encode the header
  encode_sa_header( buffer, length );
  return length + 3; // sa_header_len
}

InfoElem::ie_status security_association::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  _identifier  = buffer[0];
  _length      = ntohs( ( buffer[1] << 8 | buffer[2] ) & 0xFFFF );

  if ( _length == 0 ) // empty contents
    r = InfoElem::empty;

  _version     = ( buffer[3] >> 5 ) & 0x07;
  _trans_ind   = ( buffer[3] >> 3 ) & 0x03;
  _flow_ind    = ( buffer[3] >> 1 ) & 0x03;
  _discard     = ( buffer[3]      ) & 0x01;
  _scope       = ntohs( ( buffer[4] << 8 | buffer[5] ) & 0xFFFF );
  _relative_id = ntohs( ( buffer[6] << 8 | buffer[7] ) & 0xFFFF );
  // The target se id is only included when the Explicit bit is set 
  // and all other bits are zeroed in the scope field.
  int ident, index = 8;
  if ( _scope & 0x8000 && !(_scope & 0x7FFF) ) {
    if ( buffer[ index ] == 0x84 )
      _target_se_id = new security_agent_id( );
    else if ( buffer[ index ] == 0x82 )
      _target_se_id = new initiator_distinguished_name( );
    else if ( buffer[ index ] == 0x83 )
      _target_se_id = new responder_distinguished_name( );
    else
      abort();

    r = _target_se_id->decode( buffer + index, ident );
    index += _target_se_id->getLength( ) + 3; // Identifier, length, type
  } else
    _target_se_id = 0;

  if ( buffer[ index ] == 0x28 )
    _ssd_section = new label_based_access_control( );
  else
    _ssd_section = new security_message_exchange( );

  r = _ssd_section->decode( buffer + index, ident );

  return r;
}

const u_char                  security_association::getIdentifier( void ) const
{ return _identifier; }

const short                   security_association::getLength( void ) const
{ return _length; }

const u_char                  security_association::getVersion( void ) const
{ return _version; }

const u_char                  security_association::getTransportIndicator( void ) const
{ return _trans_ind; }

const u_char                  security_association::getFlowIndicator( void ) const
{ return _flow_ind; }

const u_char                  security_association::getDiscard( void ) const
{ return _discard; }

const short                   security_association::getScope( void ) const
{ return _scope; }

const short                   security_association::getRelativeId( void ) const
{ return _relative_id; }

const security_agent_id     * security_association::getTargetSecEntId( void ) const
{ return _target_se_id; }

const security_service_data * security_association::getSecurityServiceData( void ) const
{ return _ssd_section; }

void  security_association::setIdentifier( const u_char id )
{ _identifier = id; }

void  security_association::setVersion( const u_char ver )
{ _version = ver; }

void  security_association::setTransportIndicator( const u_char ti )
{ _trans_ind = ti; }

void  security_association::setFlowIndicator( const u_char fi )
{ _flow_ind = fi; }

void  security_association::setDiscard( const u_char discard )
{ _discard = discard; }

void  security_association::setScope( const short scope )
{ _scope = scope; }

void  security_association::setRelativeId( const short rid )
{ 
  // The least significant 4 bits are reserved
  _relative_id = rid & 0xFFF0; 
}

void  security_association::setTargetSecEntId( security_agent_id * & tse )
{ 
  if ( _target_se_id ) delete _target_se_id;
  _target_se_id = tse; 
  tse = 0;
}

void  security_association::setSecurityServiceData( security_service_data * & ssd )
{
  delete _ssd_section;
  _ssd_section = ssd;
  ssd = 0;
}

security_agent_id     * security_association::takeTargetSecEntId( void )
{
  security_agent_id     * rval = _target_se_id;
  _target_se_id = 0;
  return rval;
}

security_service_data * security_association::takeSecurityServiceData( void )
{
  security_service_data * rval = _ssd_section;
  _ssd_section = 0;
  return rval;
}

ostream & operator << ( ostream & os, const security_association & sa )
{
  os << hex;
  os << "\tIdentifier::" << (int)sa._identifier << endl
     << "\tLength::" << (int)sa._length << endl
     << "\tVersion::" << (int)sa._version << endl
     << "\tTransport Indicator::" << (int)sa._trans_ind << endl
     << "\tFlow Indicator::" << (int)sa._flow_ind << endl
     << "\tDiscard::" << (int)sa._discard << endl
     << "\tScope::" << (int)sa._scope << endl
     << "\tRelative Identifier::" << (int)sa._relative_id << endl;
  if ( sa._target_se_id != 0 )
    os << "\tTarget Security Entity Id::" << *sa._target_se_id << endl;
  if ( sa._ssd_section != 0 )
    os << "\tSecurity Service Data::" << *sa._ssd_section << endl;
  os << dec;
  return os;
}

// ----------------------------------------------------------------------------
security_service_data::security_service_data( void ) { }
security_service_data::~security_service_data( ) { }

ostream & operator << ( ostream & os, const security_service_data & ssd )
{
  ssd.Print( os );
  return os;
}
// ----------------------------------------------------------------------------
security_message_exchange::security_message_exchange( u_char format ) :
  _format( format ), _security_service_spec( 0 ),
  _confidentiality_sec( 0 ), _authentication_sec( 0 ) { }

security_message_exchange::~security_message_exchange( ) 
{ 
  delete _security_service_spec;
  delete _confidentiality_sec;
  delete _authentication_sec;
}

int                 security_message_exchange::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;
  
  *bptr++ = 0x24 | ( _format & 0x03 );
  length++;

  if ( _identifier.length() != 0 ) {
    list_item li;
    forall_items( li, _identifier ) {
      security_agent_id * tmp = _identifier.inf( li );
      int len = tmp->encode( bptr );
      length += len;
      bptr   += len;
    }
  }
  if ( _security_service_spec != 0 ) {
    int len = _security_service_spec->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _confidentiality_sec != 0 ) {
    int len = _confidentiality_sec->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _authentication_sec != 0 ) {
    int len = _authentication_sec->encode( bptr );
    length += len;
    bptr   += len;
  }
  return length;
}

InfoElem::ie_status security_message_exchange::decode(u_char * buffer, int & id)
{
  assert( buffer != 0 );
  InfoElem::ie_status r = InfoElem::ok;

  _format = buffer[0] & 0x03;
  int index = 1;

  while ( ( buffer[ index ] == 0x82 ||
	    buffer[ index ] == 0x83 ||
	    buffer[ index ] == 0x84 ) &&
	  r == InfoElem::ok ) {
    security_agent_id     * tmp = 0;
    if ( buffer[ index ] == 0x82 )
      tmp = new initiator_distinguished_name( );
    else if ( buffer[ index ] == 0x83 )
      tmp = new responder_distinguished_name( );
    else if ( buffer[ index ] == 0x84 )
      tmp = new security_agent_id( );

    r = tmp->decode( buffer + index, id );
    index += tmp->getLength() + 3; // length + ILT
    _identifier.append( tmp );
  }

  if ( r == InfoElem::ok && buffer[ index ] == 0x88 ) {
    _security_service_spec = new security_service_spec( );
    r = _security_service_spec->decode( buffer + index, id );
    index += _security_service_spec->getLength();
  }

  if ( r == InfoElem::ok && buffer[ index ] == 0x40 ) {
    if ( _confidentiality_sec == 0 )
      _confidentiality_sec = new confidential_sec( );
    r = _confidentiality_sec->decode( buffer + index, id );
    index += _confidentiality_sec->getLength();
  }

  if ( r == InfoElem::ok && buffer[ index ] == 0x50 ) {
    if ( _authentication_sec == 0 )
      _authentication_sec = new authentication_sec( );
    r = _authentication_sec->decode( buffer + index, id );
  }

  return r;
}

const security_message_exchange::formatType security_message_exchange::getFormat( void ) const
{ return (formatType)_format; }

const list<security_agent_id * > * security_message_exchange::getSecurityAgentId( void ) const
{ return &_identifier; }

const security_service_spec * security_message_exchange::getSecurityServiceSpec( void ) const
{ return _security_service_spec; }

const confidential_sec      * security_message_exchange::getConfidentiality( void ) const
{ return _confidentiality_sec; }

const authentication_sec    * security_message_exchange::getAuthentication( void ) const
{ return _authentication_sec; }

void  security_message_exchange::setFormat( security_message_exchange::formatType ft )
{ _format = ft; }

void  security_message_exchange::setSecurityAgentId( security_agent_id * & sa )
{
  list_item li;
  forall_items( li, _identifier ) {
    security_agent_id * tmp = _identifier.inf( li );
    if ( tmp->getIdentifier() == sa->getIdentifier() ) {
      _identifier.del_item( li );
      delete tmp;
      break;
    }
  }
  _identifier.append( sa );
  sa = 0;
}

void  security_message_exchange::setSecurityServiceSpec( security_service_spec * & ssp, 
							 security_message_exchange::formatType ft )
{
  delete _security_service_spec;

  _format = ft;
  _security_service_spec = ssp;
  ssp = 0;
}

void  security_message_exchange::setConfidentiality( confidential_sec * & cs )
{
  delete _confidentiality_sec;
  _confidentiality_sec = cs;
  cs = 0;
}

void  security_message_exchange::setAuthentication( authentication_sec * & as )
{
  delete _authentication_sec;
  _authentication_sec = as;
  as = 0;
}

list<security_agent_id *> * security_message_exchange::takeSecurityAgentId( void )
{
  list<security_agent_id *> * rval = new list< security_agent_id * >( _identifier );
  _identifier.clear();
  return rval;
}

security_service_spec * security_message_exchange::takeSecurityServiceSpec( void )
{
  security_service_spec * rval = _security_service_spec;
  _security_service_spec = 0;
  return rval;
}

confidential_sec      * security_message_exchange::takeConfidentiality( void )
{
  confidential_sec      * rval = _confidentiality_sec;
  _confidentiality_sec = 0;
  return rval;
}

authentication_sec    * security_message_exchange::takeAuthentication( void )
{
  authentication_sec    * rval = _authentication_sec;
  _authentication_sec = 0;
  return rval;
}

void security_message_exchange::Print( ostream & os ) const
{
  os << (int)_format;
  if ( _format == TwoWaySME_Flow2Opt )
    os << " (TwoWaySME Flow2 Optional)" << endl;
  else if ( _format == TwoWaySME_Flow2Req )
    os << " (TwoWaySME Flow2 Required)" << endl;
  else if ( _format == ThreeWaySME )
    os << " (ThreeWaySME)" << endl;
  else
    os << " (!ERROR!)" << endl;
  list_item li;
  forall_items( li, _identifier ) {
    security_agent_id * tmp = _identifier.inf( li );
    os << "\t\tSecurity Agent Identifier::" << *tmp << endl;
  }
  if ( _security_service_spec != 0 )
    os << "\t\tSecurity Service Specification::" << *_security_service_spec << endl;
  if ( _confidentiality_sec != 0 )
    os << "\t\tConfidential Section::" << *_confidentiality_sec << endl;
  if ( _authentication_sec != 0 )
    os << "\t\tAuthentication Section::" << *_authentication_sec << endl;
}

// ----------------------------------------------------------------------------
label_based_access_control::label_based_access_control( void ) :
  _length( 0 ) { }

label_based_access_control::~label_based_access_control( ) { }

int                 label_based_access_control::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = _identifier;
  *bptr++ = _length;
  *bptr++ = _type;
  length += 3;
  // FIPS 188 Coding details
  int len = _label_specific_data->encode( bptr );
  length += len;

  return length;
}

InfoElem::ie_status label_based_access_control::decode(u_char * buffer, int & id)
{
  assert( buffer != 0 );
  InfoElem::ie_status r = InfoElem::ok;
  
  id = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length = buffer[1];
  assert( buffer[2] == 0x01 ); // FIPS 188
  
  // decode the FIPS 188

  return r;
}

void label_based_access_control::Print( ostream & os ) const
{
  os << hex;
  os << (int)_identifier << " (Label Based Access Control)" << endl
     << "Length::" << (int)_length << endl
     << "Type::" << (int)_type << endl;
  if ( _label_specific_data != 0 )
    os << "Label Specific Data::" << *_label_specific_data << endl;
  os << dec;
}

// ----------------------------------------------------------------------------
FIPSSecurityTag::FIPSSecurityTag( SecTagType type )
  : _type( type ) { }

FIPSSecurityTag::~FIPSSecurityTag( ) { }

FIPSSecurityTag::SecTagType FIPSSecurityTag::getType( void ) const
{ return _type; }

// ----------------------------------------------------------------------------
FIPSRestrictiveBitMapTag::FIPSRestrictiveBitMapTag( u_char se, u_char *& attr, u_char length )
  : FIPSSecurityTag( RestrictiveBitMapType ), _security_level( se ), 
    _attributes( attr ), _length( length )
{
  attr = 0;
}

FIPSRestrictiveBitMapTag::~FIPSRestrictiveBitMapTag( )
{
  delete [] _attributes;
}

int                 FIPSRestrictiveBitMapTag::encode(u_char * buffer)
{
  int len       = 0;
  u_char * bptr = buffer;

  *bptr++ = (u_char)_type;
  *bptr++ = _length;
  *bptr++ = 0x00;
  *bptr++ = _security_level;
  len += 4;
  for ( int i = 0; i < _length; i++ ) {
    *bptr++ = _attributes[ i ];
    len++;
  }
  return len;
}

InfoElem::ie_status FIPSRestrictiveBitMapTag::decode(u_char * buffer, int & id)
{
  return InfoElem::empty;
}

FIPSPermissiveBitMapTag::FIPSPermissiveBitMapTag( u_char se, u_char *& attr, u_char length ) 
  : FIPSSecurityTag( PermissiveBitMapType ), 
    _security_level( se ), _attributes( attr ), _length( length )
{ 
  attr = 0;
}

FIPSPermissiveBitMapTag::~FIPSPermissiveBitMapTag( ) 
{ 
  delete [] _attributes;
}

int                 FIPSPermissiveBitMapTag::encode(u_char * buffer)
{
  int len       = 0;
  u_char * bptr = buffer;

  *bptr++ = (u_char)_type;
  *bptr++ = _length;
  *bptr++ = 0x00;
  *bptr++ = _security_level;
  len += 4;
  for ( int i = 0; i < _length; i++ ) {
    *bptr++ = _attributes[ i ];
    len++;
  }
  return len;
}

InfoElem::ie_status FIPSPermissiveBitMapTag::decode(u_char * buffer, int & id)
{
  return InfoElem::empty;
}

FIPSEnumeratedTag::FIPSEnumeratedTag( ) : FIPSSecurityTag( EnumeratedType ) { }
FIPSEnumeratedTag::~FIPSEnumeratedTag( ) { }

int                 FIPSEnumeratedTag::encode(u_char * buffer)
{
  int len       = 0;
  u_char * bptr = buffer;

  *bptr++ = (u_char)_type;
  *bptr++ = _enumerated_attribs.length() * 2;
  *bptr++ = 0x00;
  *bptr++ = _security_level;
  len += 4;
  list_item li;
  forall_items( li, _enumerated_attribs ) {
    short ea = _enumerated_attribs.inf( li );
    *bptr++ = ( htons( ea ) >> 8 ) & 0xFF;
    *bptr++ = ( htons( ea ) & 0xFF );
    len += 2;
  }
  return len;
}

InfoElem::ie_status FIPSEnumeratedTag::decode(u_char * buffer, int & id)
{
  return InfoElem::empty;
}

FIPSRangeTag::FIPSRangeTag( void ) : FIPSSecurityTag( RangeType ) { }
FIPSRangeTag::~FIPSRangeTag( ) { }

int                 FIPSRangeTag::encode(u_char * buffer)
{
  int       len = 0;
  u_char * bptr = buffer;

  *bptr++ = (u_char)_type;
  *bptr++ = _attribute_ranges.length() * 2;
  *bptr++ = 0x00;
  *bptr++ = _security_level;
  len += 4;
  list_item li;
  forall_items( li, _attribute_ranges ) {
    short ea = _attribute_ranges.inf( li );
    *bptr++ = ( htons( ea ) >> 8 ) & 0xFF;
    *bptr++ = ( htons( ea ) & 0xFF );
    len += 2;
  }
  return len;
}

InfoElem::ie_status FIPSRangeTag::decode(u_char * buffer, int & id)
{
  return InfoElem::empty;
}

FIPSFreeFormTag::FIPSFreeFormTag( u_char *& data, u_char length ) 
  : FIPSSecurityTag( FreeFormType ), _data( data ), _length( length ) 
{ 
  data = 0;
}

FIPSFreeFormTag::~FIPSFreeFormTag( ) 
{ 
  delete [] _data;
}

int                 FIPSFreeFormTag::encode(u_char * buffer)
{
  int       len = 0;
  u_char * bptr = buffer;

  *bptr++ = (u_char)_type;
  *bptr++ = _length;
  for ( int i = 0; i < _length; i++ ) {
    *bptr++ = _data[ i ];
    len++;
  }
  return len;
}

InfoElem::ie_status FIPSFreeFormTag::decode(u_char * buffer, int & id)
{
  return InfoElem::empty;
}

FIPSNamedTagSet::FIPSNamedTagSet( void ) { }
FIPSNamedTagSet::~FIPSNamedTagSet( ) { }

int                 FIPSNamedTagSet::encode(u_char * buffer)
{
  int       len = 0;
  u_char * bptr = buffer;

  list_item li;
  forall_items( li, _securityTags ) {
    FIPSSecurityTag * st = _securityTags.inf( li );
    int l = st->encode( bptr );
    bptr += l;
    len  += l;
  }
  return len;
}

InfoElem::ie_status FIPSNamedTagSet::decode(u_char * buffer, int & id)
{
  return InfoElem::empty;
}

FIPS188::FIPS188( ) { }
FIPS188::~FIPS188( ) { }

int                 FIPS188::encode(u_char * buffer)
{
  int       len = 0;
  u_char * bptr = buffer;

  list_item li;
  forall_items( li, _namedTagSets ) {
    FIPSNamedTagSet * ts = _namedTagSets.inf( li );
    int l = ts->encode( bptr );
    bptr += l;
    len  += l;
  }
  return len;
}

InfoElem::ie_status FIPS188::decode(u_char * buffer, int & id)
{
  return InfoElem::empty;
}

ostream & operator << ( ostream & os, const FIPS188 & fips )
{
  list_item li;
  forall_items( li, fips._namedTagSets ) {
    FIPSNamedTagSet * ts = fips._namedTagSets.inf( li );
    // os << *ts << endl;
  }
  return os;
}
// ----------------------------------------------------------------------------
security_agent_id::security_agent_id( void ) 
: _length( 0 ), _type( 0 ), _value( 0 ) { }

security_agent_id::~security_agent_id( ) 
{ 
  delete [] _value;
}

const u_char   security_agent_id::getLength( void ) const
{ return _length; }

const security_agent_id::secTypes security_agent_id::getType( void ) const
{ return (secTypes)_type; }

const u_char * security_agent_id::getValue( void ) const
{ return _value; }

void security_agent_id::setType( security_agent_id::secTypes st )
{ _type = st; }

void security_agent_id::setValueLen( u_char * & v, int len )
{
  // delete [] _value; 
  _value = v;
  _length = len;
  v = 0;

  if ( _length > 0 ) {
    assert( _value != 0 );
  }
}

u_char * security_agent_id::takeValue( void )
{ 
  u_char * rval = _value;
  _value = 0; _length = 0;
  return rval;
}

int                 security_agent_id::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = getIdentifier( );
  *bptr++ = _length;
  *bptr++ = _type;
  length += 3;
  
  for ( int i = 0; i < _length; i++ ) {
    *bptr++ = _value[ i ];
    length++;
  }

  buffer[1] = length - 3;  // ILT doesn't count

  return length;
}

InfoElem::ie_status security_agent_id::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id = buffer[0];
  if ( id != getIdentifier() ) return InfoElem::bad_id;
  _length = buffer[1];
  _type   = buffer[2];
  _value  = new u_char [ _length ];
  for ( int i = 0; i < _length; i++ )
    _value[i] = buffer[ i + 3 ];

  return r;
}

ostream & operator << ( ostream & os, const security_agent_id & sai )
{
  os << hex;
  os << (int)sai.getIdentifier();
  if ( sai.getIdentifier() == 0x84 )
    os << " (Security Agent)" << endl;
  else if ( sai.getIdentifier() == 0x82 )
    os << " (Initiator Distinguished Name)" << endl;
  else if ( sai.getIdentifier() == 0x83 )
    os << " (Responder Distinguished Name)" << endl;
  os << "\t\t\tLength::" << (int)sai._length << endl
     << "\t\t\tType::" << (int)sai._type << endl
     << "\t\t\tValue::";
  for ( int i = 0; i < sai._length; i++ )
    os << (int)sai._value[ i ];
  os << dec;
  return os;
}

// ----------------------------------------------------------------------------
initiator_distinguished_name::initiator_distinguished_name( void ) { }
initiator_distinguished_name::~initiator_distinguished_name( ) { }

// ----------------------------------------------------------------------------
responder_distinguished_name::responder_distinguished_name( void ) { }
responder_distinguished_name::~responder_distinguished_name( ) { }

// ----------------------------------------------------------------------------
security_service_spec::security_service_spec( void ) :
  _security_service_dec_sec( 0 ) { }

security_service_spec::security_service_spec( const security_service_spec & rhs )
  : _security_service_dec_sec( 0 )
{
  if ( rhs._security_service_dec_sec != 0 )
    _security_service_dec_sec = new security_service_decl( *(rhs._security_service_dec_sec) );

  if ( rhs._security_service_opt_sec.empty() == false ) {
    list_item li;
    forall_items( li, rhs._security_service_opt_sec ) {
      security_service_opt * tmp = rhs._security_service_opt_sec.inf( li );
      _security_service_opt_sec.append( tmp->copy() );
    }
  }
  if ( rhs._security_service_alg_sec.empty() == false ) {
    list_item li;
    forall_items( li, rhs._security_service_alg_sec ) {
      security_service_alg * tmp = rhs._security_service_alg_sec.inf( li );
      _security_service_alg_sec.append( tmp->copy() );
    }
  }
}

security_service_spec::~security_service_spec( ) 
{ 
  delete _security_service_dec_sec;
}

security_service_spec & security_service_spec::operator &= ( const security_service_spec & rhs )
{
  *this = *this & rhs;
  return *this;
}

security_service_spec & security_service_spec::operator |= ( const security_service_spec & rhs )
{
  *this = *this | rhs;
  return *this;
}

security_service_spec & security_service_spec::operator &  ( const security_service_spec & rhs )
{
  // compute the intersection of this and rhs

  return *this;
}

security_service_spec & security_service_spec::operator |  ( const security_service_spec & rhs )
{
  // compute the weighted union of this and rhs

  return *this;
}

bool security_service_spec::empty( void ) const
{
  if ( _security_service_dec_sec == 0 &&
       _security_service_opt_sec.empty() &&
       _security_service_alg_sec.empty() )
    return true;
  return false;
}

security_service_spec * security_service_spec::copy( void ) const
{
  return new security_service_spec(*this);
}

int                 security_service_spec::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = _identifier;
  length += 1;
  if ( _security_service_dec_sec != 0 ) {
    int len = _security_service_dec_sec->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _security_service_opt_sec.empty() != true ) {
    list_item li;
    forall_items( li, _security_service_opt_sec ) {
      security_service_opt * sso = _security_service_opt_sec.inf( li );
      int len = sso->encode( bptr );
      length += len;
      bptr   += len;
    }
  }
  if ( _security_service_alg_sec.empty() != true ) {
    list_item li;
    forall_items( li, _security_service_alg_sec ) {
      security_service_alg * ssa = _security_service_alg_sec.inf( li );
      int len = ssa->encode( bptr );
      length += len;
      bptr   += len;
    }
  }

  return length;
}

InfoElem::ie_status security_service_spec::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );
  int ident, index = 1;

  id = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;

  if ( buffer[ index ] == 0x8A ) {
    _security_service_dec_sec = new security_service_decl( );
    r = _security_service_dec_sec->decode( buffer + index, ident );
    index += 2;
  }
  
  while (( buffer [ index ] == 0x90 ||
	   buffer [ index ] == 0x92 ||
	   buffer [ index ] == 0x93 ||
	   buffer [ index ] == 0x94 ||
	   buffer [ index ] == 0x95 ||
	   buffer [ index ] == 0x96 ) &&
	 r == InfoElem::ok ) {
	
    security_service_opt * tmp = 0;

    if ( buffer[ index ] == 0x90 )
      tmp = new data_confidentiality_service_opt( );
    else if ( buffer[ index ] == 0x92 ) 
      tmp = new data_integrity_service_opt( );
    else if ( buffer[ index ] == 0x93 )
      tmp = new authentication_service_opt( );
    else if ( buffer[ index ] == 0x94 )
      tmp = new key_exchange_service_opt( );
    else if ( buffer[ index ] == 0x95 )
      tmp = new session_key_update_service_opt( );
    else if ( buffer[ index ] == 0x96 )
      tmp = new access_control_service_opt( );

    r = tmp->decode( buffer + index, ident );
    _security_service_opt_sec.append( tmp );    
    index += 2;
  }

  while (( buffer[ index ] == 0xA0 ||
	   buffer[ index ] == 0xA2 ||
	   buffer[ index ] == 0xA4 ||
	   buffer[ index ] == 0xA6 ||
	   buffer[ index ] == 0xA8 ||
	   buffer[ index ] == 0xAA ) &&
	 r == InfoElem::ok ) {

    security_service_alg * tmp = 0;
    
    if ( buffer[ index ] == 0xA0 )
      tmp = new data_confidentiality_alg( );
    else if ( buffer[ index ] == 0xA2 )
      tmp = new data_integrity_alg( );
    else if ( buffer[ index ] == 0xA4 )
      tmp = new hash_alg( );
    else if ( buffer[ index ] == 0xA6 )
      tmp = new signature_alg( );
    else if ( buffer[ index ] == 0xA8 )
      tmp = new key_exchange_alg( );
    else if ( buffer[ index ] == 0xAA )
      tmp = new session_key_update_alg( );

    r = tmp->decode( buffer + index, ident );
    _security_service_alg_sec.append( tmp );
    index += tmp->getLength() + 2; // identifier + length byte + length
  }

  return r;
}

const u_char                  security_service_spec::getLength( void ) const
{
  u_char length = 1;  // identifier

  if ( _security_service_dec_sec != 0 )
    length += _security_service_dec_sec->getLength();
  if ( _security_service_opt_sec.empty() != true )
    length += 2 * _security_service_opt_sec.length();
  if ( _security_service_alg_sec.empty() != true ) {
    list_item li;
    forall_items( li, _security_service_alg_sec ) {
      security_service_alg * ssa = _security_service_alg_sec.inf( li );
      length += ssa->getLength() + 2;
    }
  }
  
  return length;
}

const security_service_decl * security_service_spec::getSecurityServiceDecl( void ) const
{ return _security_service_dec_sec; }

const list<security_service_opt * > * security_service_spec::getSecurityServiceOpts( void ) const
{ return &_security_service_opt_sec; }

const list<security_service_alg * > * security_service_spec::getSecurityServiceAlg( void ) const
{ return &_security_service_alg_sec; }

void security_service_spec::setSecurityServiceDecl( security_service_decl * & ssd )
{ 
  delete _security_service_dec_sec;
  _security_service_dec_sec = ssd;
  ssd = 0;
}

void security_service_spec::setSecurityServiceOpts( security_service_opt  * & sso )
{
  // delete _security_service_opt_sec;
  _security_service_opt_sec.append( sso );
  sso = 0;
}

void security_service_spec::setSecurityServiceAlg(  security_service_alg  * & ssa )
{
  // delete _security_service_alg_sec;
  _security_service_alg_sec.append( ssa );
  ssa = 0;
}

security_service_decl * security_service_spec::takeSecurityServiceDecl( void )
{
  security_service_decl * rval = _security_service_dec_sec;
  _security_service_dec_sec = 0;
  return rval;
}

list<security_service_opt *> * security_service_spec::takeSecurityServiceOpts( void )
{
  list<security_service_opt *> * rval = new list<security_service_opt *>( _security_service_opt_sec );
  _security_service_opt_sec.clear();
  return rval;
}

list<security_service_alg *> * security_service_spec::takeSecurityServiceAlg( void )
{
  list<security_service_alg *> * rval = new list<security_service_alg *>( _security_service_alg_sec );
  _security_service_alg_sec.clear();
  return rval;
}

ostream & operator << ( ostream & os, const security_service_spec & sss )
{
  os << hex;
  os << (int)sss._identifier << endl;
  if ( sss._security_service_dec_sec != 0 )
    os << "\t\t\tSecurity Service Decl::" << *sss._security_service_dec_sec << endl;
  list_item li;
  forall_items( li, sss._security_service_opt_sec ) {
    security_service_opt * sso = sss._security_service_opt_sec.inf( li );
    os << "\t\t\tSecurity Service Opt::" << *sso << endl;
  }
  forall_items( li, sss._security_service_alg_sec ) {
    security_service_alg * ssa = sss._security_service_alg_sec.inf( li );
    os << "\t\t\tSecurity Service Alg::" << *ssa << endl;
  }
  os << dec;
  return os;
}

// ----------------------------------------------------------------------------
security_service_decl::security_service_decl( void ) : _declaration( 0 ) { }
security_service_decl::security_service_decl( const security_service_decl & rhs )
  : _declaration( rhs._declaration ) { }

security_service_decl::security_service_decl( u_char decl ) :
  _declaration( decl ) { }
security_service_decl::~security_service_decl( ) { }

int                 security_service_decl::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = _identifier;
  *bptr++ = _declaration;
  length += 2;

  return length;
}

InfoElem::ie_status security_service_decl::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id = buffer[0];
  if ( id != _identifier ) r = InfoElem::bad_id;
  _declaration = buffer[1];

  return r;
}

int security_service_decl::getLength( void ) const
{
  // identifier + declaration
  return 2;
}

const security_service_decl::declTypes security_service_decl::getDeclaration( void ) const
{ return (declTypes)_declaration; }

void            security_service_decl::setDeclaration( security_service_decl::declTypes dt )
{ _declaration = (u_char)dt; }

ostream & operator << ( ostream & os, const security_service_decl & ssd )
{
  os << hex;
  os << (int)ssd._identifier << endl
     << "\t\t\t\tDeclaration::" << (int)ssd._declaration;
  os << dec;
  return os;
}
// ----------------------------------------------------------------------------
security_service_opt::security_service_opt( void ) : _option( 0 ) { }
security_service_opt::security_service_opt( const security_service_opt & rhs )
  : _option( rhs._option ) { }
security_service_opt::security_service_opt( u_char opt ) : _option( opt ) { }
security_service_opt::~security_service_opt( ) { }

int                 security_service_opt::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = getIdentifier( );
  *bptr++ = _option;
  length += 2;
  
  return length;
}

InfoElem::ie_status security_service_opt::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id = buffer[0];
  assert( id == getIdentifier() );
  _option = buffer[1];

  return r;
}

ostream & operator << ( ostream & os, const security_service_opt & sso ) 
{ 
  os << hex; 
  os << (int)sso.getIdentifier(); 
  switch ( sso.getIdentifier() ) { 
  case 0x90: os << " (Data Confidentiality Opt)"; break; 
  case 0x92: os << " (Data Integrity Opt)"; break; 
  case 0x93: os << " (Authentication Service Opt)"; break; 
  case 0x94: os << " (Key Exchange Service Opt)"; break; 
  case 0x95: os << " (Session Key Update Service Opt)"; break; 
  case 0x96: os << " (Access Control Service Opt)"; break; 
  default: os << " !ERROR!"; break; 
  } 
  os << endl << "Option::" << (int)sso._option; os << dec;

  return os; 
}

// ----------------------------------------------------------------------------
data_confidentiality_service_opt::data_confidentiality_service_opt( void ) { }

data_confidentiality_service_opt::data_confidentiality_service_opt(
const data_confidentiality_service_opt & rhs ) : security_service_opt( rhs ) { }

data_confidentiality_service_opt::
data_confidentiality_service_opt( data_confidentiality_service_opt::optTypes opt ) :
  security_service_opt( opt ) { }

data_confidentiality_service_opt::~data_confidentiality_service_opt( ) { }

security_service_opt * data_confidentiality_service_opt::copy( void ) const 
{ 
  return new data_confidentiality_service_opt(*this); 
}

const data_confidentiality_service_opt::optTypes data_confidentiality_service_opt::getOption( void ) const
{ return (optTypes)_option; }

void           data_confidentiality_service_opt::setOption( data_confidentiality_service_opt::optTypes ot )
{ _option = (u_char)ot; }

// ----------------------------------------------------------------------------
data_integrity_service_opt::data_integrity_service_opt( void ) { }
data_integrity_service_opt::data_integrity_service_opt( data_integrity_service_opt::optTypes opt ) :
  security_service_opt( opt ) { }
data_integrity_service_opt::data_integrity_service_opt( const data_integrity_service_opt & rhs )
  : security_service_opt( rhs ) { }

data_integrity_service_opt::~data_integrity_service_opt( ) { }

security_service_opt * data_integrity_service_opt::copy( void ) const
{
  return new data_integrity_service_opt(*this);
}

const data_integrity_service_opt::optTypes data_integrity_service_opt::getOption( void ) const
{ return (optTypes)_option; }

void           data_integrity_service_opt::setOption( data_integrity_service_opt::optTypes ot )
{ _option = (u_char)ot; }

// ----------------------------------------------------------------------------
authentication_service_opt::authentication_service_opt( void ) { }
authentication_service_opt::authentication_service_opt( authentication_service_opt::optTypes opt ) :
  security_service_opt( opt ) { }

authentication_service_opt::~authentication_service_opt( ) { }

authentication_service_opt::authentication_service_opt( const authentication_service_opt & rhs )
  : security_service_opt( rhs ) { }

security_service_opt * authentication_service_opt::copy( void ) const
{
  return new authentication_service_opt(*this);
}

const authentication_service_opt::optTypes authentication_service_opt::getOption( void ) const
{ return (optTypes)_option; }

void           authentication_service_opt::setOption( authentication_service_opt::optTypes ot )
{ _option = (u_char)ot; }

// ----------------------------------------------------------------------------
key_exchange_service_opt::key_exchange_service_opt( void ) { }
key_exchange_service_opt::key_exchange_service_opt( key_exchange_service_opt::optTypes opt ) :
  security_service_opt( opt ) { }
key_exchange_service_opt::key_exchange_service_opt( const key_exchange_service_opt & rhs )
  : security_service_opt( rhs ) { }

key_exchange_service_opt::~key_exchange_service_opt( ) { }

security_service_opt * key_exchange_service_opt::copy( void ) const
{
  return new key_exchange_service_opt(*this);
}

const key_exchange_service_opt::optTypes key_exchange_service_opt::getOption( void ) const
{ return (optTypes)_option; }

void           key_exchange_service_opt::setOption( key_exchange_service_opt::optTypes ot )
{ _option = (u_char)ot; }

// ----------------------------------------------------------------------------
session_key_update_service_opt::session_key_update_service_opt( void ) { }
session_key_update_service_opt::
session_key_update_service_opt( session_key_update_service_opt::optTypes opt ) :
  security_service_opt( opt ) { }
session_key_update_service_opt::session_key_update_service_opt( const session_key_update_service_opt & rhs )
  : security_service_opt(rhs) { }

session_key_update_service_opt::~session_key_update_service_opt( ) { }

security_service_opt * session_key_update_service_opt::copy( void ) const
{
  return new session_key_update_service_opt(*this);
}

const session_key_update_service_opt::optTypes session_key_update_service_opt::getOption( void ) const
{ return (optTypes)_option; }

void           session_key_update_service_opt::setOption( session_key_update_service_opt::optTypes ot )
{ _option = (u_char)ot; }

// ----------------------------------------------------------------------------
access_control_service_opt::access_control_service_opt( void ) { }
access_control_service_opt::access_control_service_opt( const access_control_service_opt & rhs )
  : security_service_opt(rhs) { }

access_control_service_opt::access_control_service_opt( access_control_service_opt::optTypes opt ) :
  security_service_opt( opt ) { }

access_control_service_opt::~access_control_service_opt( ) { }

security_service_opt * access_control_service_opt::copy( void ) const
{
  return new access_control_service_opt(*this);
}

const access_control_service_opt::optTypes access_control_service_opt::getOption( void ) const
{ return (optTypes)_option; }

void           access_control_service_opt::setOption( access_control_service_opt::optTypes ot )
{ _option = (u_char)ot; }

// ----------------------------------------------------------------------------
security_service_alg::security_service_alg( void ) : _length( 0 ) { }
security_service_alg::security_service_alg( const security_service_alg & rhs )
  : _length( rhs._length ) { }

security_service_alg::~security_service_alg( ) { }

const u_char security_service_alg::getLength( void ) const { return _length; }

ostream & operator << ( ostream & os, const security_service_alg & ssa )
{
  ssa.Print( os );
  return os;
}

// ----------------------------------------------------------------------------
data_confidentiality_alg::data_confidentiality_alg( void ) :
  _algorithm( 0 ), _algorithm_oui( 0 ), _mode_of_operation( 0 ), _mode_oui( 0 ), 
  _algorithm_details( 0 ), _mode_details( 0 ) { }

data_confidentiality_alg::data_confidentiality_alg( data_confidentiality_alg::dcAlgorithms a,
						    data_confidentiality_alg::dcModes m )
  : _algorithm( a ), _algorithm_oui( 0 ), _mode_of_operation( m ), _mode_oui( 0 ),
    _algorithm_details( 0 ), _mode_details( 0 ) 
{ 
  switch ( _algorithm ) {
    case DES56:
    case TripleDES:
    case FEAL:
      break;
    case DES40:
      _algorithm_details = new u_char [ 16 ];
      bzero( _algorithm_details, sizeof(u_char) * 16 );
      break;
  }
  switch ( _mode_of_operation ) {
    case CBC:
      _mode_details = new u_char [ 8 ];
      bzero( _mode_details, sizeof(u_char) * 8 );
      break;
    case CounterMode:
    case ECB:
      break;
  }
}

data_confidentiality_alg::~data_confidentiality_alg( ) { }

security_service_alg * data_confidentiality_alg::copy( void ) const
{
  return new data_confidentiality_alg(*this);
}

int                 data_confidentiality_alg::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;
  
  *bptr++ = _identifier;
  *bptr++ = _length;
  *bptr++ = _algorithm;
  length += 3;
  if ( _algorithm & 0x80 ) {
    *bptr++ = ( htonl( _algorithm_oui ) >> 16 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui ) >>  8 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui )       ) & 0xFF;
    length += 3;
  }
  *bptr++ = _mode_of_operation;
  length++;
  if ( _mode_of_operation & 0x80 ) {
    *bptr++ = ( htonl( _mode_oui ) >> 16 ) & 0xFF;
    *bptr++ = ( htonl( _mode_oui ) >>  8 ) & 0xFF;
    *bptr++ = ( htonl( _mode_oui )       ) & 0xFF;
    length += 3;
  }
  switch ( _algorithm ) {
    case DES56:
    case TripleDES:
    case FEAL:
      assert( _algorithm_details == 0 );
      break;
    case DES40:
      // Two eight-octet fields are added to the data confidentiality
      // algorithm information element details.  The first eight
      // octets contain a binary encoded 64-bit random number.  The
      // second eight octets contain the binary encoding of that
      // random number encrypted under the DES40 key in the ECB mode.
      assert( _algorithm_details != 0 );
      for ( int i = 0; i < 16; i++ ) {
	*bptr++ = _algorithm_details[i];
	length++;
      }
      break;
  }
  switch ( _mode_of_operation ) {
    case CBC:
      // A 64-bit (8 byte) binary encoded value.  This is the
      // initialization vector for the CBC mode.
      assert( _mode_details != 0 );
      for ( int i = 0; i < 8; i++ ) {
	*bptr++ = _mode_details[i];
	length++;
      }
      break;
    case CounterMode:
    case ECB:
      assert( _mode_details == 0 );
      break;
  }
  
  buffer[1] = _length = length - 2;

  return length;
}

InfoElem::ie_status data_confidentiality_alg::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length    = buffer[1];
  _algorithm = buffer[2];
  int index = 3;
  if ( _algorithm & 0x80 ) {
    _algorithm_oui = ntohl( ( buffer[ index ] << 16 | buffer[ index + 1 ] << 8 
			      | buffer[ index + 2 ] ) & 0xFFFFFF );
    index += 3;
  }
  _mode_of_operation = buffer[ index++ ];
  if ( _mode_of_operation & 0x80 ) {
    _mode_oui = ntohl( ( buffer[ index ] << 16 | buffer[ index + 1 ] << 8 
			 | buffer[ index + 2 ] ) & 0xFFFFFF );
    index += 3;
  }
  
  switch ( _algorithm ) {
    case DES56:
    case TripleDES:
    case FEAL:
      _algorithm_details = 0;
      break;
    case DES40:
      // Two eight-octet fields are added to the data confidentiality
      // algorithm information element details.  The first eight
      // octets contain a binary encoded 64-bit random number.  The
      // second eight octets contain the binary encoding of that
      // random number encrypted under the DES40 key in the ECB mode.
      _algorithm_details = new u_char [ 16 ];
      for ( int i = 0; i < 16; i++ ) {
	_algorithm_details[i] = buffer[ index++ ];
      }
      break;
  }
  switch ( _mode_of_operation ) {
    case CounterMode:
    case ECB:
      _mode_details = 0;
      break;
    case CBC:
      // A 64-bit (8 byte) binary encoded value.  This is the
      // initialization vector for the CBC mode.
      _mode_details = new u_char [ 8 ];
      for ( int i = 0; i < 8; i++ ) {
	_mode_details[i] = buffer[ index++ ];
      }
      break;
  }

  return r;
}

const data_confidentiality_alg::dcAlgorithms data_confidentiality_alg::getAlgorithm( void ) const
{ return (dcAlgorithms)_algorithm; }

const int          data_confidentiality_alg::getAlgorithmOUI( void ) const
{ return _algorithm_oui; }

const data_confidentiality_alg::dcModes      data_confidentiality_alg::getModeOfOperation( void ) const
{ return (dcModes)_mode_of_operation; }

const int          data_confidentiality_alg::getModeOfOperationOUI( void ) const
{ return _mode_oui; }

const u_char     * data_confidentiality_alg::getAlgorithmDetails( void ) const
{ return _algorithm_details; }

const u_char     * data_confidentiality_alg::getModeDetails( void ) const
{ return _mode_details; }

void  data_confidentiality_alg::setAlgorithm( data_confidentiality_alg::dcAlgorithms a )
{ _algorithm = (u_char)a; }

void  data_confidentiality_alg::setAlgorithmOUI( int oui )
{ _algorithm_oui = oui; }

void  data_confidentiality_alg::setModeOfOperation( data_confidentiality_alg::dcModes m )
{ _mode_of_operation = (u_char)m; }

void  data_confidentiality_alg::setModeOfOperationOUI( int oui )
{ _mode_oui = oui; }

void  data_confidentiality_alg::setAlgorithmDetails( u_char * & ad )
{ 
  // delete [] _algorithm_details;
  _algorithm_details = ad;
  ad = 0;
}

void  data_confidentiality_alg::setModeDetails( u_char * & md )
{
  // delete [] _mode_details;
  _mode_details = md;
  md = 0;
}

u_char * data_confidentiality_alg::takeAlgorithmDetails( void )
{
  u_char * rval = _algorithm_details;
  _algorithm_details = 0;
  return rval;
}

u_char * data_confidentiality_alg::takeModeDetails( void )
{
  u_char * rval = _mode_details;
  _mode_details = 0;
  return rval;
}

void data_confidentiality_alg::Print( ostream & os ) const
{
  os << hex;
  os << (int)_identifier << " (Data Confidentiality)" << endl
     << "\t\t\t\tAlgorithm::" << (int)_algorithm;
  switch ( _algorithm ) {
    case DES56:
      os << " (DES 56)";
      break;
    case DES40:
      os << " (DES 40)";
      break;
    case TripleDES:
      os << " (Triple DES)";
      break;
    case FEAL:
      os << " (FEAL)";
      break;
    default:
      os << " (User Defined)";
      break;
  }
  os << endl << "\t\t\t\tAlgorithm OUI::" << (int)_algorithm_oui << endl
  // << endl << _algorithm_details
     << "\t\t\t\tMode of Operation::" << (int)_mode_of_operation << endl
     << "\t\t\t\tMode OUI::" << (int)_mode_oui
  // << endl << _mode_details
    ;
  os << dec;
}

// ----------------------------------------------------------------------------
data_integrity_alg::data_integrity_alg( void ) :
  _user( 0 ), _replay( 0 ), _algorithm( 0 ), _algorithm_oui( 0 ), _algorithm_details( 0 ) { }

data_integrity_alg::data_integrity_alg( data_integrity_alg::diAlgorithms a )
  : _user( ATMForumDefinedAlgorithm ), _replay( NoRRProtectionProvided ),
    _algorithm( a ), _algorithm_oui( 0 ), _algorithm_details( 0 ) { }

data_integrity_alg::~data_integrity_alg( ) { }

security_service_alg * data_integrity_alg::copy( void ) const
{
  return new data_integrity_alg(*this);
}

int                 data_integrity_alg::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;
  
  *bptr++ = _identifier;
  *bptr++ = _length;
  *bptr++ = (( _user << 7 ) & 0x80) | (( _replay << 6 ) & 0x40) | ( _algorithm & 0x3F );
  length += 3;
  if ( _user & 0x01 ) {
    *bptr++ = ( htonl( _algorithm_oui ) >> 16 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui ) >>  8 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui )       ) & 0xFF;
    length += 3;
  }
  switch ( _algorithm ) {
    case HMAC_MD5:
    case HMAC_SHA_1:
    case HMAC_RIPEMD_160:
    case MAC_DES_CBC:
    case MAC_DES40_CBC:
    case MAC_TripleDES_CBC:
    case MAC_FEAL_CBC:
      assert( _algorithm_details == 0 );
      break;
  }
  
  buffer[1] = _length = length - 2;

  return length;
}

InfoElem::ie_status data_integrity_alg::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length    = buffer[1];
  _user      = (buffer[2] >> 7 ) & 0x01;
  _replay    = (buffer[2] >> 6 ) & 0x01;
  _algorithm = (buffer[2]      ) & 0x3F;
  int index  = 3;
  if ( _user & 0x01 ) {
    _algorithm_oui = ntohl( ( buffer[ index ] << 16 | buffer[ index + 1 ] << 8 
			      | buffer[ index + 2 ] ) & 0xFFFFFF );
    index += 3;
  }
  switch ( _algorithm ) {
    case HMAC_MD5:
    case HMAC_SHA_1:
    case HMAC_RIPEMD_160:
    case MAC_DES_CBC:
    case MAC_DES40_CBC:
    case MAC_TripleDES_CBC:
    case MAC_FEAL_CBC:
      _algorithm_details = 0;
      break;
  }
  
  return r;
}

const data_integrity_alg::UserDefInd           data_integrity_alg::getUserDefinedInd( void ) const
{ return (UserDefInd)_user; }

const data_integrity_alg::ReplayReorderProtInd data_integrity_alg::getReplayProtInd( void ) const
{ return (ReplayReorderProtInd)_replay; }

const data_integrity_alg::diAlgorithms         data_integrity_alg::getAlgorithm( void ) const
{ return (diAlgorithms)_algorithm; }

const int                  data_integrity_alg::getAlgorithmOUI( void ) const
{ return _algorithm_oui; }

const u_char             * data_integrity_alg::getAlgorithmDetails( void ) const
{ return _algorithm_details; }

void  data_integrity_alg::setUserDefinedInd( data_integrity_alg::UserDefInd udi )
{ _user = (u_char)udi; }

void  data_integrity_alg::setReplayProtInd( data_integrity_alg::ReplayReorderProtInd rrpi )
{ _replay = (u_char)rrpi; }

void  data_integrity_alg::setAlgorithm( data_integrity_alg::diAlgorithms alg )
{ _algorithm = (diAlgorithms)alg; }

void  data_integrity_alg::setAlgorithmOUI( int oui )
{ _algorithm_oui = oui; }

void  data_integrity_alg::setAlgorithmDetails( u_char * & ad )
{ 
  // delete [] _algorithm_details;
  _algorithm_details = ad;
  ad = 0;
}

u_char * data_integrity_alg::takeAlgorithmDetails( void )
{
  u_char * rval = _algorithm_details;
  _algorithm_details = 0;
  return rval;
}

void data_integrity_alg::Print( ostream & os ) const
{
  os << hex;
  os << (int)_identifier << " (Data Integrity)" << endl
     << "\t\t\t\tUser::" << (int)_user << endl
     << "\t\t\t\tReplay::" << (int)_replay << endl
     << "\t\t\t\tAlgorithm::" << (int)_algorithm;
  switch ( _algorithm ) {
    case HMAC_MD5:
      os << " (HMAC_MD5)";
      break;
    case HMAC_SHA_1:
      os << " (HMAC_SHA-1)";
      break;
    case HMAC_RIPEMD_160:
      os << " (HMAC_RIPEMD 160)";
      break;
    case MAC_DES_CBC:
      os << " (DES/CBC)";
      break;
    case MAC_DES40_CBC:
      os << " (DES40/CBC)";
      break;
    case MAC_TripleDES_CBC:
      os << " (TripleDES/CBC)";
      break;
    case MAC_FEAL_CBC:
      os << " (FEAL/CBC)";
      break;
    default:
      os << " (UserDefined)";
      break;
  }
  os << endl << "\t\t\t\tAlgorithm OUI::" << (int)_algorithm_oui
    // << endl << "\t\t\t\tAlgorithm Details::" << _algorithm_details
    ;
  os << dec;
}

// ----------------------------------------------------------------------------
hash_alg::hash_alg( void ) :
  _algorithm( 0 ), _algorithm_oui( 0 ), _algorithm_details( 0 ) { }

hash_alg::hash_alg( hash_alg::hAlgorithms a )
  : _algorithm( a ), _algorithm_oui( 0 ), _algorithm_details( 0 ) { }

hash_alg::~hash_alg( ) { }

security_service_alg * hash_alg::copy( void ) const
{
  return new hash_alg(*this);
}

int                 hash_alg::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;
  
  *bptr++ = _identifier;
  *bptr++ = _length;
  *bptr++ = _algorithm;
  length += 3;
  if ( _algorithm & 0x80 ) {
    *bptr++ = ( htonl( _algorithm_oui ) >> 16 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui ) >>  8 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui )       ) & 0xFF;
    length += 3;
  }
  switch ( _algorithm ) {
    case MD5:
    case SHA_1:
    case RIPEMD_160:
      assert( _algorithm_details == 0 );
      break;
  }
  
  buffer[1] = _length = length - 2;
  return length;
}

InfoElem::ie_status hash_alg::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );
  
  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length    = buffer[1];
  _algorithm = buffer[2];
  int index = 3;
  if ( _algorithm & 0x80 ) {
    _algorithm_oui = ntohl( ( buffer[ index ] << 16 | buffer[ index + 1 ] << 8 
			      | buffer[ index + 2 ] ) & 0xFFFFFF );
    index += 3;
  }
  switch ( _algorithm ) {
    case MD5:
    case SHA_1:
    case RIPEMD_160:
      _algorithm_details = 0;
      break;
  }
  
  return r;
}

const hash_alg::hAlgorithms hash_alg::getAlgorithm( void ) const
{ return (hAlgorithms)_algorithm; }

const int         hash_alg::getAlgorithmOUI( void ) const
{ return _algorithm_oui; }

const u_char    * hash_alg::getAlgorithmDetails( void ) const
{ return _algorithm_details; }

void  hash_alg::setAlgorithm( hash_alg::hAlgorithms alg )
{ _algorithm = (u_char)alg; }

void  hash_alg::setAlgorithmOUI( int oui )
{ _algorithm_oui = oui; }

void  hash_alg::setAlgorithmDetails( u_char * & ad )
{
  // delete [] _algorithm_details;
  _algorithm_details = ad;
  ad = 0;
}

u_char * hash_alg::takeAlgorithmDetails( void )
{
  u_char * rval = _algorithm_details;
  _algorithm_details = 0;
  return rval;
}

void hash_alg::Print( ostream & os ) const
{
  os << hex;
  os << (int)_identifier << " (Hash)" << endl
     << "\t\t\t\tAlgorithm::" << (int)_algorithm;
  switch ( _algorithm ) {
    case MD5:
      os << " (MD5)";
      break;
    case SHA_1:
      os << " (SHA-1)";
      break;
    case RIPEMD_160:
      os << " (RIPEMD 160)";
      break;
    default:
      os << " (UserDefined)";
      break;
  }
  os << endl << "\t\t\t\tAlgorithm OUI::" << (int)_algorithm_oui
    // << endl << "\t\t\t\tAlgorithm Details::" << _algorithm_details
    ;
  os << dec;
}
// ----------------------------------------------------------------------------
signature_alg::signature_alg( void ) :
  _algorithm( 0 ), _algorithm_oui( 0 ), _algorithm_details( 0 ) { }

signature_alg::signature_alg( signature_alg::sAlgorithms a )
  : _algorithm( a ), _algorithm_oui( 0 ), _algorithm_details( 0 ) { }

signature_alg::~signature_alg( ) { }

security_service_alg * signature_alg::copy( void ) const
{
  return new signature_alg(*this);
}

int                 signature_alg::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;
  
  *bptr++ = _identifier;
  *bptr++ = _length;
  *bptr++ = _algorithm;
  length += 3;
  if ( _algorithm & 0x80 ) {
    *bptr++ = ( htonl( _algorithm_oui ) >> 16 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui ) >>  8 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui )       ) & 0xFF;
    length += 3;
  }
  switch ( _algorithm ) {
    case RSA:
    case DSA:
    case EllipticCurve:
    case DES_CBC:
    case DES40_CBC:
    case TripleDES_CBC:
    case FEAL_CBC:
      assert( _algorithm_details == 0 );
      break;
    case ESIGN:
      // The order of k:
      // This octet contains the binary encoding of the order of k
      // (i.e. X, in the relation k = 2^x)
      *bptr++ = _algorithm_details[0];
      length++;
      break;
  }
  
  buffer[1] = _length = length - 2;
  return length;
}

InfoElem::ie_status signature_alg::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length    = buffer[1];
  _algorithm = buffer[2];
  int index = 3;
  if ( _algorithm & 0x80 ) {
    _algorithm_oui = ntohl( ( buffer[ index ] << 16 | buffer[ index + 1 ] << 8
			      | buffer[ index + 2 ] ) & 0xFFFFFF );
    index += 3;
  }
  switch ( _algorithm ) {
    case RSA:
    case DSA:
    case EllipticCurve:
    case DES_CBC:
    case DES40_CBC:
    case TripleDES_CBC:
    case FEAL_CBC:
      _algorithm_details = 0;
      break;
    case ESIGN:
      // The order of k:
      // This octet contains the binary encoding of the order of k
      // (i.e. X, in the relation k = 2^x)
      _algorithm_details = new u_char [ 1 ];
      _algorithm_details[0] = buffer[ index++ ];
      break;
  }
  
  return r;
}

const signature_alg::sAlgorithms signature_alg::getAlgorithm( void ) const
{ return (sAlgorithms)_algorithm; }

const int         signature_alg::getAlgorithmOUI( void ) const
{ return _algorithm_oui; }

const u_char    * signature_alg::getAlgorithmDetails( void ) const
{ return _algorithm_details; }

void  signature_alg::setAlgorithm( signature_alg::sAlgorithms alg )
{ _algorithm = (u_char)alg; }

void  signature_alg::setAlgorithmOUI( int oui )
{ _algorithm_oui = oui; }

void  signature_alg::setAlgorithmDetails( u_char * & ad )
{ 
  // delete [] _algorithm_details;
  _algorithm_details = ad; 
  ad = 0;
}

u_char * signature_alg::takeAlgorithmDetails( void )
{
  u_char * rval = _algorithm_details;
  _algorithm_details = 0;
  return rval;
}

void signature_alg::Print( ostream & os ) const
{
  os << hex;
  os << (int)_identifier << " (Signature)" << endl
     << "\t\t\t\tAlgorithm::" << (int)_algorithm;
  switch ( _algorithm ) {
    case RSA:
      os << " (RSA)";
      break;
    case DSA:
      os << " (DSA)";
      break;
    case EllipticCurve:
      os << " (EllipticCurve)";
      break;
    case ESIGN:
      os << " (ESIGN)";
      break;
    case DES_CBC:
      os << " (DES/CBC)";
      break;
    case DES40_CBC:
      os << " (DES40/CBC)";
      break;
    case TripleDES_CBC:
      os << " (TripleDES/CBC)";
      break;
    case FEAL_CBC:
      os << " (FEAL/CBC)";
      break;
    default:
      os << " (UserDefined)";
      break;
  }
  os << endl<< "\t\t\t\tAlgorithm OUI::" << (int)_algorithm_oui
    // << endl << "\t\t\t\tAlgorithm Details::" << (int)_algorithm_details;
    ;
  os << dec;
}

// ----------------------------------------------------------------------------
key_exchange_alg::key_exchange_alg( void ) :
  _algorithm( 0 ), _algorithm_oui( 0 ), _algorithm_details( 0 ) { }

key_exchange_alg::key_exchange_alg( key_exchange_alg::keAlgorithms k )
  : _algorithm( k ), _algorithm_oui( 0 ), _algorithm_details( 0 ) { }

key_exchange_alg::~key_exchange_alg( ) { }

security_service_alg * key_exchange_alg::copy( void ) const
{
  return new key_exchange_alg(*this);
}

int                 key_exchange_alg::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;
  
  *bptr++ = _identifier;
  *bptr++ = _length;
  *bptr++ = _algorithm;
  length += 3;
  if ( _algorithm & 0x80 ) {
    *bptr++ = ( htonl( _algorithm_oui ) >> 16 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui ) >>  8 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui )       ) & 0xFF;
    length += 3;
  }
  switch ( _algorithm ) {
    case RSA:
    case DES_CBC:
    case DES40_CBC:
    case TripleDES_CBC:
    case FEAL_CBC:
      assert( _algorithm_details == 0 );
      break;
    case Diffie_Hellman:
      {
	// This octet is the identifier for the Diffie-Hellman public
	// key:
	assert( _algorithm_details[0] == 0x01 );
	*bptr++ = _algorithm_details[0];
	// This octet specified the lenth of the Diffie-Hellman public
	// in octets.
	*bptr++ = _algorithm_details[1];
	length += 2;
	// This field contains the octet string representation of the
	// Diffie-Hellman public key value.
	for ( int i = 0; i < _algorithm_details[1]; i++ ) {
	  *bptr++ = _algorithm_details[ i + 2 ];
	  length++;
	}
	// This octet is the identifier for the Diffie-Hellman public
	// modulus.
	int index = _algorithm_details[1] + 2;
	assert( _algorithm_details[ index ] == 0x02 );
	*bptr++ = _algorithm_details[ index++ ];
	// This octet specifies the length of the Diffie-Hellman public
	// modulus in octets.
	*bptr++ = _algorithm_details[ index ];
	length += 2;
	// This field contains octet string representation of the
	// Diffie-Hellman public modulus value.
	for ( int i = 0; i < _algorithm_details[ index ]; i++ ) {
	  *bptr++ = _algorithm_details[ index + i ];
	  length++;
	}
	// This octet is the identifier for the Diffie-Hellman public
	// base.
	index = index + _algorithm_details[ index ];
	assert( _algorithm_details[ index ] == 0x03 );
	*bptr++ = _algorithm_details[ index++ ];
	// This octet specifies the length of the Diffie-Hellman public
	// base in octets.
	*bptr++ = _algorithm_details[ index ];
	length += 2;
	// This field contains octet string representation of the
	// Diffie-Hellman public base value.
	for ( int i = 0; i < _algorithm_details[ index ]; i++ ) {
	  *bptr++ = _algorithm_details[ index + i ];
	  length++;
	}
      }
      break;
    case ECKAS_DH_Prime:
      // NOT IMPLEMENTED YET,  see pages 114-116
      break;
    case ECKAS_DH_Char:
      // NOT IMPLEMENTED YET,  see pages 116-117
      break;
  }
  
  buffer[1] = _length = length - 2;
  return length;
}

InfoElem::ie_status key_exchange_alg::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length    = buffer[1];
  _algorithm = buffer[2];
  int index = 3;
  if ( _algorithm & 0x80 ) {
    _algorithm_oui = ntohl( ( buffer[ index ] << 16 | buffer[ index + 1 ] << 8
			      | buffer[ index + 2 ] ) & 0xFFFFFF );
    index += 3;
  }
  switch ( _algorithm ) {
    case RSA:
    case DES_CBC:
    case DES40_CBC:
    case TripleDES_CBC:
    case FEAL_CBC:
      _algorithm_details = 0;
      break;
    case Diffie_Hellman:
      {
	int length = buffer[ index + 1  ] + 1;
	length    += buffer[ length + 2 ] + 2;
	length    += buffer[ length + 2 ] + 2;
	_algorithm_details = new u_char [ length ];
	// This octet is the identifier for the Diffie-Hellman public
	// key:
	_algorithm_details[ 0 ] = buffer[ index++ ];
	assert( _algorithm_details[0] == 0x01 );
	// This octet specified the lenth of the Diffie-Hellman public
	// in octets.
	_algorithm_details[ 1 ] = buffer[ index++ ];
	// This field contains the octet string representation of the
	// Diffie-Hellman public key value.
	for ( int i = 0; i < _algorithm_details[ 1 ]; i++ ) {
	  _algorithm_details[ i + 2 ] = buffer[ index++ ];
	}
	// This octet is the identifier for the Diffie-Hellman public
	// modulus.
	int tindex = _algorithm_details[ 1 ] + 2;
	_algorithm_details[ tindex++ ] = buffer[ index++ ];
	assert( _algorithm_details[ tindex ] == 0x02 );
	// This octet specifies the length of the Diffie-Hellman public
	// modulus in octets.
	_algorithm_details[ tindex++ ] = buffer[ index++ ];
	// This field contains octet string representation of the
	// Diffie-Hellman public modulus value.
	int size = _algorithm_details[ tindex - 1 ];
	for ( int i = 0; i < size; i++ ) {
	  _algorithm_details[ tindex++ ] = buffer[ index++ ];
	}
	// This octet is the identifier for the Diffie-Hellman public
	// base.
	_algorithm_details[ tindex++ ] = buffer[ index++ ];
	assert( _algorithm_details[ tindex ] == 0x03 );
	// This octet specifies the length of the Diffie-Hellman public
	// base in octets.
	_algorithm_details[ tindex++ ] = buffer[ index++ ];
	// This field contains octet string representation of the
	// Diffie-Hellman public base value.
	size = _algorithm_details[ tindex - 1 ];
	for ( int i = 0; i < size; i++ ) {
	  _algorithm_details[ tindex++ ] = buffer[ index++ ];
	}
      }
      break;
    case ECKAS_DH_Prime:
      // NOT IMPLEMENTED YET,  see pages 114-116
      break;
    case ECKAS_DH_Char:
      // NOT IMPLEMENTED YET,  see pages 116-117
      break;
  }

  return r;
}

const key_exchange_alg::keAlgorithms key_exchange_alg::getAlgorithm( void ) const
{ return (keAlgorithms)_algorithm; }

const int         key_exchange_alg::getAlgorithmOUI( void ) const
{ return _algorithm_oui; }

const u_char    * key_exchange_alg::getAlgorithmDetails( void ) const
{ return _algorithm_details; }

void  key_exchange_alg::setAlgorithm( key_exchange_alg::keAlgorithms alg )
{ _algorithm = (u_char)alg; }

void  key_exchange_alg::setAlgorithmOUI( int oui )
{ _algorithm_oui = oui; }

void  key_exchange_alg::setAlgorithmDetails( u_char * & ad )
{ 
  // delete [] _algorithm_details;
  _algorithm_details = ad;
  ad = 0;
}

u_char * key_exchange_alg::takeAlgorithmDetails( void )
{
  u_char * rval = _algorithm_details;
  _algorithm_details = 0;
  return rval;
}

void key_exchange_alg::Print( ostream & os ) const
{
  os << hex;
  os << (int)_identifier << " (Key Exchange)" << endl
     << "\t\t\t\tAlgorithm::" << (int)_algorithm;
  switch ( _algorithm ) {
    case RSA:
      os << " (RSA)";
      break;
    case Diffie_Hellman:
      os << " (DiffieHellman)";
      break;
    case ECKAS_DH_Prime:
      os << " (ECKAS DH Prime)";
      break;
    case ECKAS_DH_Char:
      os << " (ECKAS DH Char)";
      break;
    case DES_CBC:
      os << " (DES/CBC)";
      break;
    case DES40_CBC:
      os << " (DES40/CBC)";
      break;
    case TripleDES_CBC:
      os << " (TripleDES/CBC)";
      break;
    case FEAL_CBC:
      os << " (FEAL/CBC)";
      break;
    default:
      os << " (UserDefined)";
      break;
  }
  os << endl << "\t\t\t\tAlgorithm OUI::" << (int)_algorithm_oui
    // << endl << "\t\t\t\tAlgorithm Details::" << (int)_algorithm_details
    ;
  os << dec;
}

// ----------------------------------------------------------------------------
session_key_update_alg::session_key_update_alg( void ) :
  _algorithm( 0 ), _algorithm_oui( 0 ), _algorithm_details( 0 ) { }

session_key_update_alg::session_key_update_alg( session_key_update_alg::skAlgorithms a )
  : _algorithm( a ), _algorithm_oui( 0 ), _algorithm_details( 0 ) { }

session_key_update_alg::~session_key_update_alg( ) { }

security_service_alg * session_key_update_alg::copy( void ) const
{
  return new session_key_update_alg(*this);
}

int                 session_key_update_alg::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;
  
  *bptr++ = _identifier;
  *bptr++ = _length;
  *bptr++ = _algorithm;
  length += 3;
  if ( _algorithm & 0x80 ) {
    *bptr++ = ( htonl( _algorithm_oui ) >> 16 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui ) >>  8 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui )       ) & 0xFF;
    length += 3;
  }
  switch ( _algorithm ) {
    case SKE_MD5:
    case SKE_SHA_1:
    case SKE_RIPEMD_160:
      assert( _algorithm_details == 0 );
      break;
  }
  
  buffer[1] = _length = length - 2;
  return length;
}

InfoElem::ie_status session_key_update_alg::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length    = buffer[1];
  _algorithm = buffer[2];
  int index = 3;
  if ( _algorithm & 0x80 ) {
    _algorithm_oui = ntohl( ( buffer[ index ] << 16 | buffer[ index + 1 ] << 8 
			      | buffer[ index + 2 ] ) & 0xFFFFFF );
    index += 3;
  }
  switch ( _algorithm ) {
    case SKE_MD5:
    case SKE_SHA_1:
    case SKE_RIPEMD_160:
      _algorithm_details = 0;
      break;
  }

  return r;
}

const session_key_update_alg::skAlgorithms session_key_update_alg::getAlgorithm( void ) const
{ return (skAlgorithms)_algorithm; }

const int         session_key_update_alg::getAlgorithmOUI( void ) const
{ return _algorithm_oui; }

const u_char    * session_key_update_alg::getAlgorithmDetails( void ) const
{ return _algorithm_details; }

void  session_key_update_alg::setAlgorithm( session_key_update_alg::skAlgorithms alg )
{ _algorithm = (u_char)alg; }

void  session_key_update_alg::setAlgorithmOUI( int oui )
{ _algorithm_oui = oui; }

void  session_key_update_alg::setAlgorithmDetails( u_char * & ad )
{
  // delete [] _algorithm_details;
  _algorithm_details = ad;
  ad = 0;
}

u_char * session_key_update_alg::takeAlgorithmDetails( void )
{
  u_char * rval = _algorithm_details;
  _algorithm_details = 0;
  return rval;
}

void session_key_update_alg::Print( ostream & os ) const
{
  os << hex;
  os << (int)_identifier << " (Session Key Update)" << endl
     << "\t\t\t\tAlgorithm::" << (int)_algorithm;
  switch ( _algorithm ) {
    case SKE_MD5:
      os << " (MD5)";
      break;
    case SKE_SHA_1:
      os << " (SHA-1)";
      break;
    case SKE_RIPEMD_160:
      os << " (RIPEMD 160)";
      break;
    default:
      os << " (UserDefined)";
      break;
  }
  os << endl << "\t\t\t\tAlgorithm OUI::" << (int)_algorithm_oui
    // << endl << "\t\t\t\tAlgorithm Details::" << (int)_algorithm_details
    ;
  os << dec;
}

// ----------------------------------------------------------------------------
authentication_algorithm_profile_group::authentication_algorithm_profile_group( void ) :
  _algorithm( 0 ), _algorithm_oui( 0 ), _sig_alg_length( 0 ), _sig_alg_details( 0 ), 
  _hash_alg_length( 0 ), _hash_alg_details( 0 ) { }

authentication_algorithm_profile_group::authentication_algorithm_profile_group( const char * alg ) 
  : _algorithm( 0 ), _algorithm_oui( 0 ), _sig_alg_length( 0 ), 
    _sig_alg_details( 0 ), _hash_alg_length( 0 ), _hash_alg_details( 0 ) 
{
  if ( !strncmp( alg, "AUTH_1", 6 ) ) {
    _algorithm = AUTH_1;
    _sig_alg_details = new signature_alg( signature_alg::DES_CBC );
    _sig_alg_length  = _sig_alg_details->getLength();
  } else if ( !strncmp( alg, "AUTH_2", 6 ) ) {
    _algorithm = AUTH_2;
    _sig_alg_details  = new signature_alg( signature_alg::DSA );
    _sig_alg_length   = _sig_alg_details->getLength();
    _hash_alg_details = new hash_alg( hash_alg::SHA_1 );
    _hash_alg_length  = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "AUTH_3", 6 ) ) {
    _algorithm = AUTH_3;
    _sig_alg_details  = new signature_alg( signature_alg::EllipticCurve );
    _sig_alg_length   = _sig_alg_details->getLength();
    _hash_alg_details = new hash_alg( hash_alg::SHA_1 );
    _hash_alg_length  = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "AUTH_4", 6 ) ) {
    _algorithm = AUTH_4;
    _sig_alg_details  = new signature_alg( signature_alg::ESIGN );
    _sig_alg_length   = _sig_alg_details->getLength();
    _hash_alg_details = new hash_alg( hash_alg::MD5 );
    _hash_alg_length  = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "AUTH_5", 6 ) ) {
    _algorithm = AUTH_5;
    _sig_alg_details  = new signature_alg( signature_alg::FEAL_CBC );
    _sig_alg_length   = _sig_alg_details->getLength();
  } else if ( !strncmp( alg, "AUTH_6", 6 ) ) {
    _algorithm = AUTH_6;
    _sig_alg_details  = new signature_alg( signature_alg::RSA );
    _sig_alg_length   = _sig_alg_details->getLength();
    _hash_alg_details = new hash_alg( hash_alg::MD5 );
    _hash_alg_length  = _hash_alg_details->getLength();
  } else { // UserDefined
    _algorithm = UserDefinedAlgorithm;
  }
}

authentication_algorithm_profile_group::~authentication_algorithm_profile_group( ) { }

security_service_alg * authentication_algorithm_profile_group::copy( void ) const
{
  return new authentication_algorithm_profile_group(*this);
}

int                 authentication_algorithm_profile_group::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;
  
  *bptr++ = _identifier;
  *bptr++ = _length;
  *bptr++ = _algorithm;
  length += 3;
  if ( _algorithm & 0x80 ) {
    *bptr++ = ( htonl( _algorithm_oui ) >> 16 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui ) >>  8 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui )       ) & 0xFF;
    length += 3;
  }
  if ( _sig_alg_details != 0 ) {
    *bptr++ = _sig_alg_id;
    *bptr++ = _sig_alg_length;
    length += 2;

    int len = _sig_alg_details->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _hash_alg_details != 0 ) {
    *bptr++ = _hash_alg_id;
    *bptr++ = _hash_alg_length;
    length += 2;

    int len = _hash_alg_details->encode( bptr );
    length += len;
    bptr   += len;
  }

  buffer[1] = _length = length - 2;
  return length;
}

InfoElem::ie_status authentication_algorithm_profile_group::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length    = buffer[1];
  _algorithm = buffer[2];
  int index  = 3;
  if ( _algorithm & 0x80 ) {
    _algorithm_oui = ntohl( ( buffer[ index ] << 16 | buffer[ index + 1 ] << 8
			      | buffer[ index + 2 ] ) & 0xFFFFFF );
    index += 3;
  }
  if ( buffer[ index ] == _sig_alg_id ) {
    int ident; index++; // we just checked above that it matched
    _sig_alg_length = buffer[ index++ ];
    _sig_alg_details = new signature_alg( );
    r = _sig_alg_details->decode( buffer + index, ident );
    index += _sig_alg_length;
  }
  if ( buffer[ index ] == _hash_alg_id ) {
    int ident; index++; // we just checked above that it matched
    _hash_alg_length  = buffer[ index++ ];
    _hash_alg_details = new hash_alg( );
    r = _hash_alg_details->decode( buffer + index, ident );
    index += _hash_alg_length;
  }
  return r;
}

const authentication_algorithm_profile_group::aAlgorithms     
authentication_algorithm_profile_group::getAlgorithm( void ) const
{ return (aAlgorithms)_algorithm; }

const int             authentication_algorithm_profile_group::getAlgorithmOUI( void ) const
{ return _algorithm_oui; }

const u_char          authentication_algorithm_profile_group::getSignatureLength( void ) const
{ return _sig_alg_length; }

const signature_alg * authentication_algorithm_profile_group::getSignatureAlgorithm( void ) const
{ return _sig_alg_details; }

const u_char          authentication_algorithm_profile_group::getHashLength( void ) const
{ return _hash_alg_length; }

const hash_alg      * authentication_algorithm_profile_group::getHashAlgorithm( void ) const
{ return _hash_alg_details; }

void  authentication_algorithm_profile_group::setAlgorithm( authentication_algorithm_profile_group::aAlgorithms alg )
{ _algorithm = (u_char)alg; }

void  authentication_algorithm_profile_group::setAlgorithmOUI( int oui )
{ _algorithm_oui = oui; }

void  authentication_algorithm_profile_group::setSignatureAlgorithm( signature_alg * & sa )
{ 
  delete _sig_alg_details;
  _sig_alg_details = sa;
  sa = 0;
}

void  authentication_algorithm_profile_group::setHashAlgorithm( hash_alg * & ha )
{
  delete _hash_alg_details;
  _hash_alg_details = ha;
  ha = 0;
}

signature_alg * authentication_algorithm_profile_group::takeSignatureAlgorithm( void )
{
  signature_alg * rval = _sig_alg_details;
  _sig_alg_details = 0;
  return rval;
}

hash_alg      * authentication_algorithm_profile_group::takeHashAlgorithm( void )
{
  hash_alg      * rval = _hash_alg_details;
  _hash_alg_details = 0;
  return rval;
}

void authentication_algorithm_profile_group::Print( ostream & os ) const
{
  os << hex;
  os << (int)_identifier << " (Authentication Profile Group)" << endl
     << "\t\t\t\tAlgorithm::" << (int)_algorithm;
  switch ( _algorithm ) {
    case AUTH_1:
      os << " (AUTH_1)";
      break;
    case AUTH_2:
      os << " (AUTH_2)";
      break;
    case AUTH_3:
      os << " (AUTH_3)";
      break;
    case AUTH_4:
      os << " (AUTH_4)";
      break;
    case AUTH_5:
      os << " (AUTH_5)";
      break;
    case AUTH_6:
      os << " (AUTH_6)";
      break;
    default:
      os << " (UserDefined)";
      break;
  }
  os << endl << "\t\t\t\tAlgorithm OUI::" << (int)_algorithm_oui << endl;
  if ( _sig_alg_details != 0 )
    os << "\t\t\t\tSignature Algorithm Id::" << (int)_sig_alg_id << endl
       << "\t\t\t\tSignature Algorithm Length::" << (int)_sig_alg_length << endl
       << "\t\t\t\tSignature Algorithm Details::" << *_sig_alg_details << endl;
  if ( _hash_alg_details != 0 )
    os << "\t\t\t\tHash Algorithm Id::" << (int)_hash_alg_id << endl
       << "\t\t\t\tHash Algorithm Length::" << (int)_hash_alg_length << endl
       << "\t\t\t\tHash Algorithm Details::" << *_hash_alg_details << endl;
  os << dec;
}

// ----------------------------------------------------------------------------
integrity_algorithm_profile_group::integrity_algorithm_profile_group( void ) :
  _algorithm( 0 ), _algorithm_oui( 0 ), _mac_alg_length( 0 ), _mac_alg_details( 0 ),
  _sig_alg_length( 0 ), _sig_alg_details( 0 ), _key_ex_alg_length( 0 ), 
  _key_ex_alg_details( 0 ), _key_up_alg_length( 0 ), _key_up_alg_details( 0 ), 
  _hash_alg_length( 0 ), _hash_alg_details( 0 ) { }

integrity_algorithm_profile_group::integrity_algorithm_profile_group( const char * alg )
  : _algorithm( 0 ), _algorithm_oui( 0 ), _mac_alg_length( 0 ), _mac_alg_details( 0 ),
    _sig_alg_length( 0 ), _sig_alg_details( 0 ), _key_ex_alg_length( 0 ), 
    _key_ex_alg_details( 0 ), _key_up_alg_length( 0 ), _key_up_alg_details( 0 ), 
    _hash_alg_length( 0 ), _hash_alg_details( 0 ) 
{
  if ( !strncmp( alg, "INTEG_1", 7 ) ) {
    _algorithm          = INTEG_1;
    _mac_alg_details    = new data_integrity_alg( data_integrity_alg::MAC_DES_CBC );
    _mac_alg_length     = _mac_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::RSA );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::RSA );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::MD5 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "INTEG_2", 7 ) ) {
    _algorithm          = INTEG_2;
    _mac_alg_details    = new data_integrity_alg( data_integrity_alg::MAC_DES_CBC );
    _mac_alg_length     = _mac_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::DSA );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::Diffie_Hellman );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_SHA_1 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::SHA_1 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "INTEG_3", 7 ) ) {
    _algorithm          = INTEG_3;
    _mac_alg_details    = new data_integrity_alg( data_integrity_alg::MAC_DES_CBC );
    _mac_alg_length     = _mac_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::EllipticCurve );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::Diffie_Hellman );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_SHA_1 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::SHA_1 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "INTEG_4", 7 ) ) {
    _algorithm          = INTEG_4;
    _mac_alg_details    = new data_integrity_alg( data_integrity_alg::MAC_DES_CBC );
    _mac_alg_length     = _mac_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::DES_CBC );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::DES_CBC );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
  } else if ( !strncmp( alg, "INTEG_5", 7 ) ) {
    _algorithm          = INTEG_5;
    _mac_alg_details    = new data_integrity_alg( data_integrity_alg::HMAC_MD5 );
    _mac_alg_length     = _mac_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::RSA );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::RSA );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::MD5 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "INTEG_6", 7 ) ) {
    _algorithm          = INTEG_6;
    _mac_alg_details    = new data_integrity_alg( data_integrity_alg::HMAC_MD5 );
    _mac_alg_length     = _mac_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::DES_CBC );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::DES_CBC );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
  } else if ( !strncmp( alg, "INTEG_7", 7 ) ) {
    _algorithm          = INTEG_7;
    _mac_alg_details    = new data_integrity_alg( data_integrity_alg::HMAC_SHA_1 );
    _mac_alg_length     = _mac_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::DSA );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::Diffie_Hellman );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_SHA_1 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::SHA_1 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "INTEG_8", 7 ) ) {
    _algorithm          = INTEG_8;
    _mac_alg_details    = new data_integrity_alg( data_integrity_alg::HMAC_SHA_1 );
    _mac_alg_length     = _mac_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::EllipticCurve );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::Diffie_Hellman );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_SHA_1 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::SHA_1 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "INTEG_9", 7 ) ) {
    _algorithm          = INTEG_9;
    _mac_alg_details    = new data_integrity_alg( data_integrity_alg::HMAC_SHA_1 );
    _mac_alg_length     = _mac_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::DES_CBC );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::DES_CBC );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_SHA_1 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
  } else if ( !strncmp( alg, "INTEG_10", 8 ) ) {
    _algorithm          = INTEG_10;
    _mac_alg_details    = new data_integrity_alg( data_integrity_alg::MAC_FEAL_CBC );
    _mac_alg_length     = _mac_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::ESIGN );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::Diffie_Hellman );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::MD5 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "INTEG_11", 8 ) ) {
    _algorithm          = INTEG_11;
    _mac_alg_details    = new data_integrity_alg( data_integrity_alg::MAC_FEAL_CBC );
    _mac_alg_length     = _mac_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::FEAL_CBC );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::FEAL_CBC );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
  } else { // UserDefined
    _algorithm = UserDefinedAlgorithm;
  }
}

integrity_algorithm_profile_group::~integrity_algorithm_profile_group( ) { }

security_service_alg * integrity_algorithm_profile_group::copy( void ) const
{
  return new integrity_algorithm_profile_group(*this);
}

int                 integrity_algorithm_profile_group::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;
  
  *bptr++ = _identifier;
  *bptr++ = _length;
  *bptr++ = _algorithm;
  length += 3;
  if ( _algorithm & 0x80 ) {
    *bptr++ = ( htonl( _algorithm_oui ) >> 16 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui ) >>  8 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui )       ) & 0xFF;
    length += 3;
  }
  if ( _mac_alg_details != 0 ) {
    *bptr++ = _mac_alg_id;
    *bptr++ = _mac_alg_length;
    length += 2;

    int len = _mac_alg_details->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _sig_alg_details != 0 ) {
    *bptr++ = _sig_alg_id;
    *bptr++ = _sig_alg_length;
    length += 2;

    int len = _sig_alg_details->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _key_ex_alg_details != 0 ) {
    *bptr++ = _key_ex_alg_id;
    *bptr++ = _key_ex_alg_length;
    length += 2;

    int len = _key_ex_alg_details->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _key_up_alg_details != 0 ) {
    *bptr++ = _key_up_alg_id;
    *bptr++ = _key_up_alg_length;
    length += 2;

    int len = _key_up_alg_details->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _hash_alg_details != 0 ) {
    *bptr++ = _hash_alg_id;
    *bptr++ = _hash_alg_length;
    length += 2;

    int len = _hash_alg_details->encode( bptr );
    length += len;
    bptr   += len;
  }

  buffer[1] = _length = length - 2;

  return length;
}

InfoElem::ie_status integrity_algorithm_profile_group::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length    = buffer[1];
  _algorithm = buffer[2];
  int index  = 3;
  if ( _algorithm & 0x80 ) {
    _algorithm_oui = ntohl( ( buffer[ index ] << 16 | buffer[ index + 1 ] << 8
			      | buffer[ index + 2 ] ) & 0xFFFFFF );
    index += 3;
  }
  if ( buffer[ index ] == _mac_alg_id ) {
    int ident; index++; // we just checked above that it matched
    _mac_alg_length = buffer[ index++ ];
    _mac_alg_details = new data_integrity_alg( );
    r = _mac_alg_details->decode( buffer + index, ident );
    index += _mac_alg_length;
  }
  if ( buffer[ index ] == _sig_alg_id ) {
    int ident; index++; // we just checked above that it matched
    _sig_alg_length = buffer[ index++ ];
    _sig_alg_details = new signature_alg( );
    r = _sig_alg_details->decode( buffer + index, ident );
    index += _sig_alg_length;
  }
  if ( buffer[ index ] == _key_ex_alg_id ) {
    int ident; index++; // we just checked above that it matched
    _key_ex_alg_length = buffer[ index++ ];
    _key_ex_alg_details = new key_exchange_alg( );
    r = _key_ex_alg_details->decode( buffer + index, ident );
    index += _key_ex_alg_length;
  }
  if ( buffer[ index ] == _key_up_alg_id ) {
    int ident; index++; // we just checked above that it matched
    _key_up_alg_length = buffer[ index++ ];
    _key_up_alg_details = new session_key_update_alg( );
    r = _key_up_alg_details->decode( buffer + index, ident );
    index += _key_up_alg_length;
  }
  if ( buffer[ index ] == _hash_alg_id ) {
    int ident; index++; // we just checked above that it matched
    _hash_alg_length  = buffer[ index++ ];
    _hash_alg_details = new hash_alg( );
    r = _hash_alg_details->decode( buffer + index, ident );
    index += _hash_alg_length;
  }
  return r;
}

const integrity_algorithm_profile_group::iAlgorithms              
integrity_algorithm_profile_group::getAlgorithm( void ) const
{ return (iAlgorithms)_algorithm; }

const int                      integrity_algorithm_profile_group::getAlgorithmOUI( void ) const
{ return _algorithm_oui; }

const u_char                   integrity_algorithm_profile_group::getMACLength( void ) const
{ return _mac_alg_length; }

const data_integrity_alg     * integrity_algorithm_profile_group::getMACAlgorithm( void ) const
{ return _mac_alg_details; }

const u_char                   integrity_algorithm_profile_group::getSignatureLength( void ) const
{ return _sig_alg_length; }

const signature_alg          * integrity_algorithm_profile_group::getSignatureAlgorithm( void ) const
{ return _sig_alg_details; }

const u_char                   integrity_algorithm_profile_group::getKeyExchangeLength( void ) const
{ return _key_ex_alg_length; }

const key_exchange_alg       * integrity_algorithm_profile_group::getKeyExchangeAlgorithm( void ) const
{ return _key_ex_alg_details; }

const u_char                   integrity_algorithm_profile_group::getKeyUpdateLength( void ) const
{ return _key_up_alg_length; }

const session_key_update_alg * integrity_algorithm_profile_group::getKeyUpdateAlgorithm( void ) const
{ return _key_up_alg_details; }

const u_char                   integrity_algorithm_profile_group::getHashLength( void ) const
{ return _hash_alg_length; }

const hash_alg               * integrity_algorithm_profile_group::getHashAlgorithm( void ) const
{ return _hash_alg_details; }

void  integrity_algorithm_profile_group::setAlgorithm( integrity_algorithm_profile_group::iAlgorithms alg )
{ _algorithm = (u_char)alg; }

void  integrity_algorithm_profile_group::setAlgorithmOUI( int oui )
{ _algorithm_oui = oui; }

void  integrity_algorithm_profile_group::setMACAlgorithm( data_integrity_alg * & dia )
{ 
  delete _mac_alg_details;
  _mac_alg_details = dia;
  dia = 0;
}

void  integrity_algorithm_profile_group::setSignatureAlgorithm( signature_alg * & sa )
{
  delete _sig_alg_details;
  _sig_alg_details = sa;
  sa = 0;
}

void  integrity_algorithm_profile_group::setKeyExchangeAlgorithm( key_exchange_alg * & kea )
{
  delete _key_ex_alg_details;
  _key_ex_alg_details = kea;
  kea = 0;
}

void  integrity_algorithm_profile_group::setKeyUpdateAlgorithm( session_key_update_alg * & sku )
{
  delete _key_up_alg_details;
  _key_up_alg_details = sku;
  sku = 0;
}

void  integrity_algorithm_profile_group::setHashAlgorithm( hash_alg * & ha )
{
  delete _hash_alg_details;
  _hash_alg_details = ha;
  ha = 0;
}

data_integrity_alg     * integrity_algorithm_profile_group::takeMACAlgorithm( void )
{
  data_integrity_alg     * rval = _mac_alg_details;
  _mac_alg_details = 0;
  return rval;
}

signature_alg          * integrity_algorithm_profile_group::takeSignatureAlgorithm( void )
{
  signature_alg          * rval = _sig_alg_details;
  _sig_alg_details = 0;
  return rval;
}

key_exchange_alg       * integrity_algorithm_profile_group::takeKeyExchangeAlgorithm( void )
{
  key_exchange_alg       * rval = _key_ex_alg_details;
  _key_ex_alg_details = 0;
  return rval;
}

session_key_update_alg * integrity_algorithm_profile_group::takeKeyUpdateAlgorithm( void )
{
  session_key_update_alg * rval = _key_up_alg_details;
  _key_up_alg_details = 0;
  return rval;
}

hash_alg               * integrity_algorithm_profile_group::takeHashAlgorithm( void )
{
  hash_alg               * rval = _hash_alg_details;
  _hash_alg_details = 0;
  return rval;
}

void   integrity_algorithm_profile_group::Print( ostream & os ) const
{
  os << hex;
  os << (int)_identifier << " (Integrity Profile Group)" << endl
     << "\t\t\t\tAlgorithm::" << (int)_algorithm;
  switch ( _algorithm ) {
    case INTEG_1:
      os << " (INTEG_1)";
      break;
    case INTEG_2:
      os << " (INTEG_2)";
      break;
    case INTEG_3:
      os << " (INTEG_3)";
      break;
    case INTEG_4:
      os << " (INTEG_4)";
      break;
    case INTEG_5:
      os << " (INTEG_5)";
      break;
    case INTEG_6:
      os << " (INTEG_6)";
      break;
    case INTEG_7:
      os << " (INTEG_7)";
      break;
    case INTEG_8:
      os << " (INTEG_8)";
      break;
    case INTEG_9:
      os << " (INTEG_9)";
      break;
    case INTEG_10:
      os << " (INTEG_10)";
      break;
    case INTEG_11:
      os << " (INTEG_11)";
      break;
    default:
      os << " (UserDefined)";
      break;
  }
  os << endl << "\t\t\t\tAlgorithm OUI::" << (int)_algorithm_oui << endl;
  if ( _mac_alg_details !=0 )
    os << "\t\t\t\tMAC Algorithm Id::" << (int)_mac_alg_id << endl
       << "\t\t\t\tMAC Algorithm Length::" << (int)_mac_alg_length << endl
       << "\t\t\t\tMAC Algorithm Details::" << *_mac_alg_details << endl;
  if ( _sig_alg_details != 0 )
    os << "\t\t\t\tSignature Algorithm Id::" << (int)_sig_alg_id << endl
       << "\t\t\t\tSignature Algorithm Length::" << (int)_sig_alg_length << endl
       << "\t\t\t\tSignature Algorithm Details::" << *_sig_alg_details << endl;
  if ( _key_ex_alg_details != 0 )
    os << "\t\t\t\tKey Exchange Algorithm Id::" << (int)_key_ex_alg_id << endl
       << "\t\t\t\tKey Exchange Algorithm Length::" << (int)_key_ex_alg_length << endl
       << "\t\t\t\tKey Exchange Algorithm Details::" << *_key_ex_alg_details << endl;
  if ( _key_up_alg_details != 0 )
    os << "\t\t\t\tKey Update Algorithm Id::" << (int)_key_up_alg_id << endl
       << "\t\t\t\tKey Update Algorithm Length::" << (int)_key_up_alg_length << endl
       << "\t\t\t\tKey Update Algorithm Details::" << *_key_up_alg_details << endl;
  if ( _hash_alg_details != 0 )
    os << "\t\t\t\tHash Algorithm Id::" << (int)_hash_alg_id << endl
       << "\t\t\t\tHash Algorithm Length::" << (int)_hash_alg_length << endl
       << "\t\t\t\tHash Algorithm Details::" << *_hash_alg_details << endl;
  os << dec;
}

// ----------------------------------------------------------------------------
confidentiality_algorithm_profile_group::confidentiality_algorithm_profile_group( void ) :
  _algorithm( 0 ), _algorithm_oui( 0 ), _enc_alg_length( 0 ), _enc_alg_details( 0 ),
  _sig_alg_length( 0 ), _sig_alg_details( 0 ), _key_ex_alg_length( 0 ), 
  _key_ex_alg_details( 0 ), _key_up_alg_length( 0 ), _key_up_alg_details( 0 ), 
  _hash_alg_length( 0 ), _hash_alg_details( 0 ) { }

confidentiality_algorithm_profile_group::confidentiality_algorithm_profile_group( const char * alg )
  : _algorithm( 0 ), _algorithm_oui( 0 ), _enc_alg_length( 0 ), _enc_alg_details( 0 ),
  _sig_alg_length( 0 ), _sig_alg_details( 0 ), _key_ex_alg_length( 0 ), 
  _key_ex_alg_details( 0 ), _key_up_alg_length( 0 ), _key_up_alg_details( 0 ), 
  _hash_alg_length( 0 ), _hash_alg_details( 0 ) 
{
  if ( !strncmp( alg, "CONF_1", 6 ) ) {
    _algorithm          = CONF_1;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::DES56,
							data_confidentiality_alg::CBC );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::RSA );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::RSA );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::MD5 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_2", 6 ) ) {
    _algorithm          = CONF_2;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::DES56,
							data_confidentiality_alg::CBC );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::DSA );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::Diffie_Hellman );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_SHA_1 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::SHA_1 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_3", 6 ) ) {
    _algorithm          = CONF_3;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::DES56,
							data_confidentiality_alg::CBC );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::EllipticCurve );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::Diffie_Hellman );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_SHA_1 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::SHA_1 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_4", 6 ) ) {
    _algorithm          = CONF_4;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::DES56,
							data_confidentiality_alg::CBC );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::DES_CBC );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::DES_CBC );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_5", 6 ) ) {
    _algorithm          = CONF_5;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::DES56,
							data_confidentiality_alg::CounterMode );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::RSA );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::RSA );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::MD5 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_6", 6 ) ) {
    _algorithm          = CONF_6;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::DES56,
							data_confidentiality_alg::CounterMode );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::DSA );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::Diffie_Hellman );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_SHA_1 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::SHA_1 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_7", 6 ) ) {
    _algorithm          = CONF_7;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::DES56,
							data_confidentiality_alg::CounterMode );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::EllipticCurve );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::Diffie_Hellman );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_SHA_1 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::SHA_1 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_8", 6 ) ) {
    _algorithm          = CONF_8;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::DES56,
							data_confidentiality_alg::CounterMode );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::DES_CBC );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::DES_CBC );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_9", 6 ) ) {
    _algorithm          = CONF_9;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::TripleDES,
							data_confidentiality_alg::CBC );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::RSA );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::RSA );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::MD5 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_10", 7 ) ) {
    _algorithm          = CONF_10;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::TripleDES,
							data_confidentiality_alg::CBC );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::DSA );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::Diffie_Hellman );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_SHA_1 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::SHA_1 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_11", 7 ) ) {
    _algorithm          = CONF_11;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::TripleDES,
							data_confidentiality_alg::CBC );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::EllipticCurve );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::Diffie_Hellman );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_SHA_1 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::SHA_1 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_12", 7 ) ) {
    _algorithm          = CONF_12;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::TripleDES,
							data_confidentiality_alg::CBC );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::TripleDES_CBC );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::TripleDES_CBC );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_13", 7 ) ) {
    _algorithm          = CONF_13;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::TripleDES,
							data_confidentiality_alg::CounterMode );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::RSA );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::RSA );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::MD5 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_14", 7 ) ) {
    _algorithm          = CONF_14;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::TripleDES,
							data_confidentiality_alg::CounterMode );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::DSA );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::Diffie_Hellman );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_SHA_1 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::SHA_1 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_15", 7 ) ) {
    _algorithm          = CONF_15;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::TripleDES,
							data_confidentiality_alg::CounterMode );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::EllipticCurve );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::Diffie_Hellman );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_SHA_1 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::SHA_1 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_16", 7 ) ) {
    _algorithm          = CONF_16;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::TripleDES,
							data_confidentiality_alg::CounterMode );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::TripleDES_CBC );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::TripleDES_CBC );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_17", 7 ) ) {
    _algorithm          = CONF_17;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::FEAL,
							data_confidentiality_alg::CBC );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::ESIGN );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::Diffie_Hellman );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::MD5 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_18", 7 ) ) {
    _algorithm          = CONF_18;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::FEAL,
							data_confidentiality_alg::CBC );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::FEAL_CBC );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::FEAL_CBC );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_19", 7 ) ) {
    _algorithm          = CONF_19;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::FEAL,
							data_confidentiality_alg::CounterMode );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::ESIGN );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::Diffie_Hellman );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
    _hash_alg_details   = new hash_alg( hash_alg::MD5 );
    _hash_alg_length    = _hash_alg_details->getLength();
  } else if ( !strncmp( alg, "CONF_20", 7 ) ) {
    _algorithm          = CONF_20;
    _enc_alg_details    = new data_confidentiality_alg( data_confidentiality_alg::FEAL,
							data_confidentiality_alg::CounterMode );
    _enc_alg_length     = _enc_alg_details->getLength();
    _sig_alg_details    = new signature_alg( signature_alg::FEAL_CBC );
    _sig_alg_length     = _sig_alg_details->getLength();
    _key_ex_alg_details = new key_exchange_alg( key_exchange_alg::FEAL_CBC );
    _key_ex_alg_length  = _key_ex_alg_details->getLength();
    _key_up_alg_details = new session_key_update_alg( session_key_update_alg::SKE_MD5 );
    _key_up_alg_length  = _key_up_alg_details->getLength();
  } else { // UserDefined
    _algorithm = UserDefinedAlgorithm;
  }
}

confidentiality_algorithm_profile_group::~confidentiality_algorithm_profile_group( ) { }

security_service_alg * confidentiality_algorithm_profile_group::copy( void ) const
{
  return new confidentiality_algorithm_profile_group(*this);
}

int                 confidentiality_algorithm_profile_group::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;
  
  *bptr++ = _identifier;
  *bptr++ = _length;
  *bptr++ = _algorithm;
  length += 3;
  if ( _algorithm & 0x80 ) {
    *bptr++ = ( htonl( _algorithm_oui ) >> 16 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui ) >>  8 ) & 0xFF;
    *bptr++ = ( htonl( _algorithm_oui )       ) & 0xFF;
    length += 3;
  }
  if ( _enc_alg_details != 0 ) {
    *bptr++ = _enc_alg_id;
    *bptr++ = _enc_alg_length;
    length += 2;

    int len = _enc_alg_details->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _sig_alg_details != 0 ) {
    *bptr++ = _sig_alg_id;
    *bptr++ = _sig_alg_length;
    length += 2;

    int len = _sig_alg_details->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _key_ex_alg_details != 0 ) {
    *bptr++ = _key_ex_alg_id;
    *bptr++ = _key_ex_alg_length;
    length += 2;

    int len = _key_ex_alg_details->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _key_up_alg_details != 0 ) {
    *bptr++ = _key_up_alg_id;
    *bptr++ = _key_up_alg_length;
    length += 2;

    int len = _key_up_alg_details->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _hash_alg_details != 0 ) {
    *bptr++ = _hash_alg_id;
    *bptr++ = _hash_alg_length;
    length += 2;

    int len = _hash_alg_details->encode( bptr );
    length += len;
    bptr   += len;
  }

  buffer[1] = _length = length - 2;

  return length;
}

InfoElem::ie_status confidentiality_algorithm_profile_group::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length    = buffer[1];
  _algorithm = buffer[2];
  int index  = 3;
  if ( _algorithm & 0x80 ) {
    _algorithm_oui = ntohl( ( buffer[ index ] << 16 | buffer[ index + 1 ] << 8
			      | buffer[ index + 2 ] ) & 0xFFFFFF );
    index += 3;
  }
  if ( buffer[ index ] == _enc_alg_id ) {
    int ident; index++; // we just checked above that it matched
    _enc_alg_length = buffer[ index++ ];
    _enc_alg_details = new data_confidentiality_alg( );
    r = _enc_alg_details->decode( buffer + index, ident );
    index += _enc_alg_length;
  }
  if ( buffer[ index ] == _sig_alg_id ) {
    int ident; index++; // we just checked above that it matched
    _sig_alg_length = buffer[ index++ ];
    _sig_alg_details = new signature_alg( );
    r = _sig_alg_details->decode( buffer + index, ident );
    index += _sig_alg_length;
  }
  if ( buffer[ index ] == _key_ex_alg_id ) {
    int ident; index++; // we just checked above that it matched
    _key_ex_alg_length = buffer[ index++ ];
    _key_ex_alg_details = new key_exchange_alg( );
    r = _key_ex_alg_details->decode( buffer + index, ident );
    index += _key_ex_alg_length;
  }
  if ( buffer[ index ] == _key_up_alg_id ) {
    int ident; index++; // we just checked above that it matched
    _key_up_alg_length = buffer[ index++ ];
    _key_up_alg_details = new session_key_update_alg( );
    r = _key_up_alg_details->decode( buffer + index, ident );
    index += _key_up_alg_length;
  }
  if ( buffer[ index ] == _hash_alg_id ) {
    int ident; index++; // we just checked above that it matched
    _hash_alg_length  = buffer[ index++ ];
    _hash_alg_details = new hash_alg( );
    r = _hash_alg_details->decode( buffer + index, ident );
    index += _hash_alg_length;
  }
  return r;
}

const confidentiality_algorithm_profile_group::cAlgorithms                
confidentiality_algorithm_profile_group::getAlgorithm( void ) const
{ return (cAlgorithms)_algorithm; }

const int                        
confidentiality_algorithm_profile_group::getAlgorithmOUI( void ) const
{ return _algorithm_oui; }

const u_char                     
confidentiality_algorithm_profile_group::getDataConfidentialityLength( void ) const
{ return _enc_alg_length; }

const data_confidentiality_alg * 
confidentiality_algorithm_profile_group::getDataConfidentialityAlgorithm( void ) const
{ return _enc_alg_details; }

const u_char                     
confidentiality_algorithm_profile_group::getSignatureLength( void ) const
{ return _sig_alg_length; }

const signature_alg            * 
confidentiality_algorithm_profile_group::getSignatureAlgorithm( void ) const
{ return _sig_alg_details; }

const u_char                     
confidentiality_algorithm_profile_group::getKeyExchangeLength( void ) const
{ return _key_ex_alg_length; }

const key_exchange_alg         * 
confidentiality_algorithm_profile_group::getKeyExchangeAlgorithm( void ) const
{ return _key_ex_alg_details; }

const u_char                     
confidentiality_algorithm_profile_group::getKeyUpdateLength( void ) const
{ return _key_up_alg_length; }

const session_key_update_alg   * 
confidentiality_algorithm_profile_group::getKeyUpdateAlgorithm( void ) const
{ return _key_up_alg_details; }

const u_char                     
confidentiality_algorithm_profile_group::getHashLength( void ) const
{ return _hash_alg_length; }

const hash_alg                 * 
confidentiality_algorithm_profile_group::getHashAlgorithm( void ) const
{ return _hash_alg_details; }

void  confidentiality_algorithm_profile_group::setAlgorithm( confidentiality_algorithm_profile_group::cAlgorithms alg )
{ _algorithm = (u_char)alg; }

void  confidentiality_algorithm_profile_group::setAlgorithmOUI( int oui )
{ _algorithm_oui = oui; }

void  confidentiality_algorithm_profile_group::
setDataConfidentialityAlgorithm( data_confidentiality_alg * & dca )
{
  delete _enc_alg_details;
  _enc_alg_details = dca;
  dca = 0;
}

void  confidentiality_algorithm_profile_group::setSignatureAlgorithm( signature_alg * & sa )
{
  delete _sig_alg_details;
  _sig_alg_details = sa;
  sa = 0;
}

void  confidentiality_algorithm_profile_group::setKeyExchangeAlgorithm( key_exchange_alg * & kea )
{
  delete _key_ex_alg_details;
  _key_ex_alg_details = kea;
  kea = 0;
}

void  confidentiality_algorithm_profile_group::setKeyUpdateAlgorithm( session_key_update_alg * & sku )
{
  delete _key_up_alg_details;
  _key_up_alg_details = sku;
  sku = 0;
}

void  confidentiality_algorithm_profile_group::setHashAlgorithm( hash_alg * & ha )
{
  delete _hash_alg_details;
  _hash_alg_details = ha;
  ha = 0;
}

data_confidentiality_alg * 
confidentiality_algorithm_profile_group::takeDataConfidentialityAlgorithm( void )
{
  data_confidentiality_alg * rval = _enc_alg_details;
  _enc_alg_details = 0;
  return rval;
}

signature_alg            * 
confidentiality_algorithm_profile_group::takeSignatureAlgorithm( void )
{
  signature_alg            * rval = _sig_alg_details;
  _sig_alg_details = 0;
  return rval;
}

key_exchange_alg         * 
confidentiality_algorithm_profile_group::takeKeyExchangeAlgorithm( void )
{
  key_exchange_alg         * rval = _key_ex_alg_details;
  _key_ex_alg_details = 0;
  return rval;
}

session_key_update_alg   * 
confidentiality_algorithm_profile_group::takeKeyUpdateAlgorithm( void )
{
  session_key_update_alg   * rval = _key_up_alg_details;
  _key_up_alg_details = 0;
  return rval;
}

hash_alg                 * 
confidentiality_algorithm_profile_group::takeHashAlgorithm( void )
{
  hash_alg                 * rval = _hash_alg_details;
  _hash_alg_details = 0;
  return rval;
}

void   confidentiality_algorithm_profile_group::Print( ostream & os ) const
{
  os << hex;
  os << (int)_identifier << " (Confidentiality Profile Group)" << endl
     << "\t\t\t\tAlgorithm::" << (int)_algorithm;
  switch ( _algorithm ) {
    case CONF_1:
      os << " (CONF_1)";
      break;
    case CONF_2:
      os << " (CONF_2)";
      break;
    case CONF_3:
      os << " (CONF_3)";
      break;
    case CONF_4:
      os << " (CONF_4)";
      break;
    case CONF_5:
      os << " (CONF_5)";
      break;
    case CONF_6:
      os << " (CONF_6)";
      break;
    case CONF_7:
      os << " (CONF_7)";
      break;
    case CONF_8:
      os << " (CONF_8)";
      break;
    case CONF_9:
      os << " (CONF_9)";
      break;
    case CONF_10:
      os << " (CONF_10)";
      break;
    case CONF_11:
      os << " (CONF_11)";
      break;
    case CONF_12:
      os << " (CONF_12)";
      break;
    case CONF_13:
      os << " (CONF_13)";
      break;
    case CONF_14:
      os << " (CONF_14)";
      break;
    case CONF_15:
      os << " (CONF_15)";
      break;
    case CONF_16:
      os << " (CONF_16)";
      break;
    case CONF_17:
      os << " (CONF_17)";
      break;
    case CONF_18:
      os << " (CONF_18)";
      break;
    case CONF_19:
      os << " (CONF_19)";
      break;
    case CONF_20:
      os << " (CONF_20)";
      break;
    default:
      os << " (UserDefined)";
      break;
  }
  os << endl << "\t\t\t\tAlgorithm OUI::" << (int)_algorithm_oui << endl
     << "\t\t\t\tEncryption Algorithm Id::" << (int)_enc_alg_id << endl
     << "\t\t\t\tEncryption Algorithm Length::" << (int)_enc_alg_length << endl
     << "\t\t\t\tEncryption Algorithm Details::" << *_enc_alg_details << endl
     << "\t\t\t\tSignature Algorithm Id::" << (int)_sig_alg_id << endl
     << "\t\t\t\tSignature Algorithm Length::" << (int)_sig_alg_length << endl
     << "\t\t\t\tSignature Algorithm Details::" << *_sig_alg_details << endl
     << "\t\t\t\tKey Exchange Algorithm Id::" << (int)_key_ex_alg_id << endl
     << "\t\t\t\tKey Exchange Algorithm Length::" << (int)_key_ex_alg_length << endl
     << "\t\t\t\tKey Exchange Algorithm Details::" << *_key_ex_alg_details << endl
     << "\t\t\t\tKey Update Algorithm Id::" << (int)_key_up_alg_id << endl
     << "\t\t\t\tKey Update Algorithm Length::" << (int)_key_up_alg_length << endl
     << "\t\t\t\tKey Update Algorithm Details::" << *_key_up_alg_details << endl
     << "\t\t\t\tHash Algorithm Id::" << (int)_hash_alg_id << endl
     << "\t\t\t\tHash Algorithm Length::" << (int)_hash_alg_length << endl
     << "\t\t\t\tHash Algorithm Details::" << *_hash_alg_details << endl;
  os << dec;
}
// ----------------------------------------------------------------------------
confidential_sec::confidential_sec( void ) :
  _length( 0 ), _confidential_data( 0 ) { }
confidential_sec::~confidential_sec( ) 
{ 
  delete _confidential_data;
}

int                 confidential_sec::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = _identifier;
  *bptr++ = ( htons( _length ) >> 8 ) & 0xFF;
  *bptr++ = ( htons( _length )      ) & 0xFF;
  length += 3;

  if ( _confidential_data != 0 ) {
    int len = _confidential_data->encode( bptr );
    length += len;
  }

  _length   = length - 3;
  buffer[1] = ( htons( _length ) >> 8 ) & 0xFF;
  buffer[2] = ( htons( _length )      ) & 0xFF;

  return length;
}

InfoElem::ie_status confidential_sec::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length    = ntohs( ( buffer[1] << 8 | buffer[2] ) & 0xFFFF );
  _confidential_data = new confidential_params( );
  int ident;
  r = _confidential_data->decode( buffer + 3, ident );

  return r;
}

const short                 confidential_sec::getLength( void ) const
{ return _length; }

const confidential_params * confidential_sec::getConfidentialParams( void ) const
{ return _confidential_data; }

void  confidential_sec::setConfidentialParams( confidential_params * & cp )
{
  delete _confidential_data;
  _confidential_data = cp;
  cp = 0;
}

confidential_params * confidential_sec::takeConfidentialParams( void )
{
  confidential_params * rval = _confidential_data;
  _confidential_data = 0;
  return rval;
}

ostream & operator << ( ostream & os, const confidential_sec & cs )
{
  os << "Identifier::" << (int)cs._identifier << endl
     << "Length::" << (int)cs._length << endl;
  if ( cs._confidential_data != 0 )
    os << "Confidential Params::" << *cs._confidential_data << endl;
  return os;
}

// ----------------------------------------------------------------------------
confidential_params::confidential_params( void ) :
  _master_key( 0 ), _first_confid_key( 0 ), _first_integ_key( 0 ) { }

confidential_params::~confidential_params( ) 
{ 
  delete _master_key;
  delete _first_confid_key;
  delete _first_integ_key;
}

int                 confidential_params::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = _identifier;
  if ( _master_key != 0 ) {
    int len = _master_key->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _first_confid_key != 0 ) {
    int len = _first_confid_key->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _first_integ_key != 0 ) {
    int len = _first_integ_key->encode( bptr );
    length += len;
    bptr   += len;
  }
  return length;
}

InfoElem::ie_status confidential_params::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _master_key = new master_key( );
  int ident;
  r = _master_key->decode( buffer + 1, ident );

  if ( r == InfoElem::ok ) {
    _first_confid_key = new first_confidentiality_key( );
    r = _first_confid_key->decode( buffer + 1 + _master_key->getLength() + 2, ident );
  }
  if ( r == InfoElem::ok ) {
    _first_integ_key = new first_integrity_key( );
    r = _first_integ_key->decode( buffer + 1 + _master_key->getLength() + 2 +
				  _first_confid_key->getLength() + 2, ident );
  }
  return r;
}

const master_key                * confidential_params::getMasterKey( void ) const
{ return _master_key; }

const first_confidentiality_key * confidential_params::getFirstConfidentialityKey( void ) const
{ return _first_confid_key; }

const first_integrity_key       * confidential_params::getFirstIntegrityKey( void ) const
{ return _first_integ_key; }

void  confidential_params::setMasterKey( master_key * & mk )
{
  delete _master_key;
  _master_key = mk;
  mk = 0;
}

void  confidential_params::setFirstConfidentialityKey( first_confidentiality_key * & fck )
{
  delete _first_confid_key;
  _first_confid_key = fck;
  fck = 0;
}

void  confidential_params::setFirstIntegrityKey( first_integrity_key * & fik )
{
  delete _first_integ_key;
  _first_integ_key = fik;
  fik = 0;
}

master_key                * confidential_params::takeMasterKey( void )
{
  master_key                * rval = _master_key;
  _master_key = 0;
  return rval;
}

first_confidentiality_key * confidential_params::takeFirstConfidentialityKey( void )
{
  first_confidentiality_key * rval = _first_confid_key;
  _first_confid_key = 0;
  return rval;
}

first_integrity_key       * confidential_params::takeFirstIntegrityKey( void )
{
  first_integrity_key       * rval = _first_integ_key;
  _first_integ_key = 0;
  return rval;
}

ostream & operator << ( ostream & os, const confidential_params & cp )
{
  os << "Identifier::" << (int)cp._identifier << endl;
  if ( cp._master_key != 0 )
    os << "Master Key::" << *cp._master_key << endl;
  if ( cp._first_confid_key != 0 )
    os << "First Confid Key::" << *cp._first_confid_key << endl;
  if ( cp._first_integ_key != 0 )
    os << "First Integ Key::" << *cp._first_integ_key << endl;
  return os;
}
// ----------------------------------------------------------------------------
master_key::master_key( void ) :
  _length( 0 ), _value( 0 ) { }

master_key::~master_key( ) 
{
  delete [] _value;
}

int                 master_key::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = _identifier;
  *bptr++ = _length;
  length += 2;
  for ( int i = 0; i < _length; i++ ) {
    *bptr++ = _value[i];
    length++;
  }
  return length;
}

InfoElem::ie_status master_key::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length    = buffer[1];
  _value     = new u_char [ _length ];
  for ( int i = 0; i < _length; i++ )
    _value[ i ] = buffer[ i + 2 ];

  return r;
}

const u_char   master_key::getLength( void ) const
{ return _length; }

const u_char * master_key::getValue( void ) const
{ return _value; }

void  master_key::setValue( u_char * & val )
{
  delete [] _value;
  _value = val;
  val = 0;
}

u_char * master_key::takeValue( void )
{ 
  u_char * rval = _value;
  _value = 0;
  return rval;
}

ostream & operator << ( ostream & os, const master_key & mk )
{
  os << hex;
  os << "Identifier::" << (int)mk._identifier << endl
     << "Length::" << (int)mk._length << endl
     << "Value::" << endl;
  for ( int i = 0; i < mk._length; i++ )
    os << (int)mk._value[ i ];
  os << dec;
  return os;
}

// ----------------------------------------------------------------------------
first_confidentiality_key::first_confidentiality_key( void ) :
  _length( 0 ), _value( 0 ) { }
first_confidentiality_key::~first_confidentiality_key( ) 
{ 
  delete [] _value;
}

int                 first_confidentiality_key::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = _identifier;
  *bptr++ = _length;
  length += 2;
  for ( int i = 0; i < _length; i++ ) {
    *bptr++ = _value[i];
    length++;
  }
  return length;
}

InfoElem::ie_status first_confidentiality_key::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length    = buffer[1];
  _value     = new u_char [ _length ];
  for ( int i = 0; i < _length; i++ )
    _value[ i ] = buffer[ i + 2 ];

  return r;
}

const u_char   first_confidentiality_key::getLength( void ) const
{ return _length; }

const u_char * first_confidentiality_key::getValue( void ) const
{ return _value; }

void  first_confidentiality_key::setValue( u_char * & val )
{
  delete [] _value;
  _value = val;
  val = 0;
}

u_char * first_confidentiality_key::takeValue( void )
{ 
  u_char * rval = _value;
  _value = 0;
  return rval;
}

ostream & operator << ( ostream & os, const first_confidentiality_key & fck )
{
  os << hex;
  os << "Identifier::" << (int)fck._identifier << endl
     << "Length::" << (int)fck._length << endl
     << "Value::";
  for ( int i = 0; i < fck._length; i++ )
    os << (int)fck._value[ i ];
  os << endl;
  os << dec;
  return os;
}
// ----------------------------------------------------------------------------
first_integrity_key::first_integrity_key( void ) :
  _length( 0 ), _value( 0 ) { }
first_integrity_key::~first_integrity_key( ) 
{ 
  delete [] _value;
}

int                 first_integrity_key::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = _identifier;
  *bptr++ = _length;
  length += 2;
  for ( int i = 0; i < _length; i++ ) {
    *bptr++ = _value[i];
    length++;
  }
  return length;
}

InfoElem::ie_status first_integrity_key::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length    = buffer[1];
  _value     = new u_char [ _length ];
  for ( int i = 0; i < _length; i++ )
    _value[ i ] = buffer[ i + 2 ];

  return r;
}

const u_char   first_integrity_key::getLength( void ) const
{ return _length; }

const u_char * first_integrity_key::getValue( void ) const
{ return _value; }

void  first_integrity_key::setValue( u_char * & val )
{
  delete [] _value;
  _value = val;
  val = 0;
}

u_char * first_integrity_key::takeValue( void )
{ 
  u_char * rval = _value;
  _value = 0;
  return rval;
}

ostream & operator << ( ostream & os, const first_integrity_key & fik )
{
  os << hex;
  os << "Identifier::" << (int)fik._identifier << endl
     << "Length::" << (int)fik._length << endl
     << "Value::";
  for ( int i = 0; i < fik._length; i++ )
    os << (int)fik._value[ i ];
  os << endl;
  os << dec;
  return os;
}
// ----------------------------------------------------------------------------
authentication_sec::authentication_sec( void ) :
  _initiator_random_number( 0 ), _responder_random_number( 0 ), 
  _time_stamp( 0 ), _credentials( 0 ), _digital_signature( 0 ) { }
authentication_sec::~authentication_sec( ) 
{ 
  delete _initiator_random_number;
  delete _responder_random_number;
  delete _time_stamp;
  delete _credentials;
  delete _digital_signature;
}

int                 authentication_sec::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = _identifier;
  length++;
  if ( _initiator_random_number != 0 ) {
    int len = _initiator_random_number->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _responder_random_number != 0 ) {
    int len = _responder_random_number->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _time_stamp != 0 ) {
    int len = _time_stamp->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _credentials != 0 ) {
    int len = _credentials->encode( bptr );
    length += len;
    bptr   += len;
  }
  if ( _digital_signature != 0 ) {
    int len = _digital_signature->encode( bptr );
    length += len;
    bptr   += len;
  }

  return length;
}

InfoElem::ie_status authentication_sec::decode(u_char * buffer, int & id)
{
  int ident;
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  int index = 1;
  if ( buffer[ index ] == 0xD4 ) {
    _initiator_random_number = new initiator_nonce( );
    r = _initiator_random_number->decode( buffer + index , ident );
    index += 5;
  }
  if ( buffer[ index ] == 0xD5 ) {
    _responder_random_number = new responder_nonce( );
    r = _responder_random_number->decode( buffer + index , ident );
    index += 5;
  }
  if ( buffer[ index ] == 0xD6 ) {
    _time_stamp = new time_variant_time_stamp( );
    r = _time_stamp->decode( buffer + index , ident );
    index += 9;
  }
  if ( buffer[ index ] == 0xD8 ) {
    _credentials = new credentials( );
    r = _credentials->decode( buffer + index , ident );
    index += _credentials->getLength() + 3;
  }
  if ( buffer[ index ] == 0xDA ) {
    _digital_signature = new digital_signature( );
    r = _digital_signature->decode( buffer + index , ident );
    index += _digital_signature->getLength() + 2;
  }
  return r;
}

const initiator_nonce         * authentication_sec::getInitiatorRandomNumber( void ) const
{ return _initiator_random_number; }

const responder_nonce         * authentication_sec::getResponderRandomNumber( void ) const
{ return _responder_random_number; }

const time_variant_time_stamp * authentication_sec::getTimeStamp( void ) const
{ return _time_stamp; }

const credentials             * authentication_sec::getCredentials( void ) const
{ return _credentials; }

const digital_signature      * authentication_sec::getDigitalSignature( void ) const
{ return _digital_signature; }

void  authentication_sec::setInitiatorRandomNumber( initiator_nonce * & in )
{
  delete _initiator_random_number;
  _initiator_random_number = in;
  in = 0;
}

void  authentication_sec::setResponderRandomNumber( responder_nonce * & rn )
{
  delete _responder_random_number;
  _responder_random_number = rn;
  rn = 0;
}

void  authentication_sec::setTimeStamp( time_variant_time_stamp * & ts )
{
  delete _time_stamp;
  _time_stamp = ts;
  ts = 0;
}

void  authentication_sec::setCredentials( credentials * & creds )
{
  delete _credentials;
  _credentials = creds;
  creds = 0;
}

void  authentication_sec::setDigitalSignature( digital_signature * & ds )
{
  delete _digital_signature;
  _digital_signature = ds;
  ds = 0;
}

initiator_nonce         * authentication_sec::takeInitiatorRandomNumber( void )
{
  initiator_nonce    * rval = _initiator_random_number;
  _initiator_random_number = 0;
  return rval;
}

responder_nonce         * authentication_sec::takeResponderRandomNumber( void )
{
  responder_nonce    * rval = _responder_random_number;
  _responder_random_number = 0;
  return rval;
}

time_variant_time_stamp * authentication_sec::takeTimeStamp( void )
{
  time_variant_time_stamp * rval = _time_stamp;
  _time_stamp = 0;
  return rval;
}

credentials             * authentication_sec::takeCredentials( void )
{
  credentials        * rval = _credentials;
  _credentials = 0;
  return rval;
}

digital_signature      * authentication_sec::takeDigitalSignature( void )
{
  digital_signature * rval = _digital_signature;
  _digital_signature = 0;
  return rval;
}

ostream & operator << ( ostream & os, const authentication_sec & as )
{
  os << "Identifier::"       << (int)as._identifier << endl;
  if ( as._initiator_random_number != 0 )
    os << "Initiator Nonce::"   << *as._initiator_random_number << endl;
  if ( as._responder_random_number != 0 )
    os << "Responder Nonce::"   << *as._responder_random_number << endl;
  if ( as._time_stamp != 0 )
    os << "Time Stamp::"        << *as._time_stamp << endl;
  if ( as._credentials != 0 )
    os << "Credentials::"       << *as._credentials << endl;
  if ( as._digital_signature != 0 )
    os << "Digital Signature::" << *as._digital_signature << endl;
  return os;
}

// ----------------------------------------------------------------------------
initiator_nonce::initiator_nonce( void ) : _random_number( 0xFFFFFFFF ) { }
initiator_nonce::initiator_nonce( int rn ) : _random_number( rn ) { }
initiator_nonce::~initiator_nonce( ) { }

int                 initiator_nonce::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = _identifier;
  *bptr++ = ( htonl( _random_number ) >> 24 ) & 0xFF;
  *bptr++ = ( htonl( _random_number ) >> 16 ) & 0xFF;
  *bptr++ = ( htonl( _random_number ) >>  8 ) & 0xFF;
  *bptr++ = ( htonl( _random_number )       ) & 0xFF;
  length += 5;
  return length;
}

InfoElem::ie_status initiator_nonce::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _random_number = ntohl( ( buffer[1] << 24 | buffer[2] << 16 |
			    buffer[3] <<  8 | buffer[4] ) & 0xFFFFFFFF );
  return r;
}

const int initiator_nonce::getRandomNumber( void ) const
{ return _random_number; }

ostream & operator << ( ostream & os, const initiator_nonce & in )
{
  os << hex;
  os << "Identifier::" << (int)in._identifier << endl
     << "Random Number::" << (int)in._random_number << endl;
  os << dec;
  return os;
}
// ----------------------------------------------------------------------------
responder_nonce::responder_nonce( void ) : _random_number( 0xFFFFFFFF ) { }
responder_nonce::responder_nonce( int rn ) : _random_number( rn ) { }
responder_nonce::~responder_nonce( ) { }

int                 responder_nonce::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = _identifier;
  *bptr++ = ( htonl( _random_number ) >> 24 ) & 0xFF;
  *bptr++ = ( htonl( _random_number ) >> 16 ) & 0xFF;
  *bptr++ = ( htonl( _random_number ) >>  8 ) & 0xFF;
  *bptr++ = ( htonl( _random_number )      ) & 0xFF;
  length += 5;
  return length;
}

InfoElem::ie_status responder_nonce::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _random_number = ntohl( ( buffer[1] << 24 | buffer[2] << 16 |
			    buffer[3] <<  8 | buffer[4] ) & 0xFFFFFFFF );
  return r;
}

const int responder_nonce::getRandomNumber( void ) const 
{ return _random_number; }

ostream & operator << ( ostream & os, const responder_nonce & rn )
{
  os << hex;
  os << "Identifier::" << (int)rn._identifier << endl
     << "Random Number::" << (int)rn._random_number << endl;
  os << dec;
  return os;
}
// ----------------------------------------------------------------------------
time_variant_time_stamp::time_variant_time_stamp( void ) : _time_stamp_value( 0 ), 
  _sequence_number( 0 ) { }
time_variant_time_stamp::time_variant_time_stamp( int time_stamp, int seq_num )
  : _time_stamp_value( time_stamp ), _sequence_number( seq_num ) { }
time_variant_time_stamp::~time_variant_time_stamp( ) { }

int                 time_variant_time_stamp::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = _identifier;
  *bptr++ = ( htonl( _time_stamp_value ) >> 24 ) & 0xFF;
  *bptr++ = ( htonl( _time_stamp_value ) >> 16 ) & 0xFF;
  *bptr++ = ( htonl( _time_stamp_value ) >>  8 ) & 0xFF;
  *bptr++ = ( htonl( _time_stamp_value )       ) & 0xFF;
  *bptr++ = ( htonl( _sequence_number ) >> 24 ) & 0xFF;
  *bptr++ = ( htonl( _sequence_number ) >> 16 ) & 0xFF;
  *bptr++ = ( htonl( _sequence_number ) >>  8 ) & 0xFF;
  *bptr++ = ( htonl( _sequence_number )       ) & 0xFF;
  length += 9;
  return length;
}

InfoElem::ie_status time_variant_time_stamp::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _time_stamp_value = ntohl( ( buffer[1] << 24 | buffer[2] << 16 |
			       buffer[3] <<  8 | buffer[4] ) & 0xFFFFFFFF );
  _sequence_number = ntohl( ( buffer[5] << 24 | buffer[6] << 16 |
			      buffer[7] <<  8 | buffer[8] ) & 0xFFFFFFFF );

  return r;
}

const int time_variant_time_stamp::getTimeStamp( void ) const
{ return _time_stamp_value; }

const int time_variant_time_stamp::getSequenceNumber( void ) const
{ return _sequence_number; }

ostream & operator << ( ostream & os, const time_variant_time_stamp & tvts )
{
  os << hex;
  os << "Time Stamp Val::" << (int)tvts._time_stamp_value << endl
     << "Sequence Number::" << (int)tvts._sequence_number << endl;
  os << dec;
  return os;
}

// ----------------------------------------------------------------------------
credentials::credentials( void ) : _type( 0 ), _length( 0 ), _value( 0 ) { }
credentials::credentials( credTypes t, u_char * & value, int length )
  : _type( t ), _value( value ), _length( length ) { value = 0; }
credentials::~credentials( ) 
{ 
  delete [] _value;
}

int                 credentials::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = _identifier;
  *bptr++ = ( htons( _length ) >> 8 ) & 0xFF;
  *bptr++ = ( htons( _length )      ) & 0xFF;
  *bptr++ = _type;
  length += 4;
  for ( int i = 0; i < _length - 1; i++ ) {
    *bptr++ = _value[i];
    length++;
  }
  
  return length;
}

InfoElem::ie_status credentials::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length    = ntohs( ( buffer[1] << 8 | buffer[2] ) & 0xFFFF );
  _type      = buffer[3];
  _value     = new u_char [ _length - 1 ];
  for ( int i = 0; i < _length - 1; i++ ) {
    _value[ i ] = buffer[ i + 4 ];
  }

  return r;
}

const short     credentials::getLength( void )
{ return _length; }

const credentials::credTypes credentials::getType( void ) const
{ return (credTypes)_type; }

const u_char  * credentials::getValue( void ) const
{ return _value; }

void  credentials::setTypeValueLen( credentials::credTypes t, u_char * & val, int length )
{ 
  _type = (u_char)t;
  delete [] _value;
  _value = val;
  val = 0;
  _length = length;
}

u_char  * credentials::takeValue( void )
{
  u_char * rval = _value;
  _value = 0; _length = 0;
  return rval;
}

ostream & operator << ( ostream & os, const credentials & c )
{
  os << hex;
  os << "Identifier::" << (int)c._identifier << endl
     << "Length::" << (int)c._length << endl
     << "Type::" << (int)c._type << endl
     << "Value::";
  for ( int i = 0; i < c._length; i++ )
    os << (int)c._value[ i ];
  os << endl;
  os << dec;
  return os;
}

// ----------------------------------------------------------------------------
digital_signature::digital_signature( void ) : _length( 0 ), _value( 0 ) { }
digital_signature::digital_signature( u_char * & value, int length )
  : _value( value ), _length( length ) { value = 0; }
digital_signature::~digital_signature( ) 
{ 
  delete [] _value;
}

int                 digital_signature::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = _identifier;
  *bptr++ = _length;
  length += 2;
  // This section isn't quite finished, see page 135-136 for value info
  for ( int i = 0; i < _length; i++ ) {
    *bptr++ = _value[i];
    length++;
  }
  return length;
}

InfoElem::ie_status digital_signature::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length    = buffer[1];
  _value     = new u_char [ _length ];
  for ( int i = 0; i < _length; i++ )
    _value[i] = buffer[ i + 2 ];

  return r;
}

const u_char   digital_signature::getLength( void ) const
{ return _length; }

const u_char * digital_signature::getValue( void ) const
{ return _value; }

void  digital_signature::setValueLen( u_char * & val, int length )
{
  delete [] _value;
  _value = val;
  val = 0;
  _length = length;
}
u_char * digital_signature::takeValue( void )
{
  u_char * rval = _value;
  _value = 0; _length = 0;
  return rval;
}

ostream & operator << ( ostream & os, const digital_signature & ds )
{
  os << hex;
  os << "Identifier::" << (int)ds._identifier << endl
     << "Length::" << (int)ds._length << endl
     << "Value::";
  for ( int i = 0; i < ds._length; i++ )
    os << (int)ds._value[ i ];
  os << endl;
  os << dec;
  return os;
}
// ----------------------------------------------------------------------------
sas_digital_signature::sas_digital_signature( void ) : _length( 0 ), _value( 0 ) { }
sas_digital_signature::sas_digital_signature( u_char * & value, int length )
  : _value( value ), _length( length ) { value = 0; }
sas_digital_signature::~sas_digital_signature( ) 
{ 
  delete [] _value;
}

int                 sas_digital_signature::encode(u_char * buffer)
{
  int length    = 0;
  u_char * bptr = buffer;

  *bptr++ = _identifier;
  *bptr++ = ( htons( _length ) >> 8 ) & 0xFF;
  *bptr++ = ( htons( _length )      ) & 0xFF;
  length += 3;
  // See the algorithm-specific details in section 6.2.5.5.
  for ( int i = 0; i < _length; i++ ) {
    *bptr++ = _value[i];
    length++;
  }
  return length;
}

InfoElem::ie_status sas_digital_signature::decode(u_char * buffer, int & id)
{
  InfoElem::ie_status r = InfoElem::ok;
  assert( buffer != 0 );

  id         = buffer[0];
  if ( id != _identifier ) return InfoElem::bad_id;
  _length    = ntohs( ( buffer[1] << 8 | buffer[2] ) & 0xFFFF );
  _value     = new u_char [ _length ];
  for ( int i = 0; i < _length; i++ )
    _value[i] = buffer[ i + 3 ];

  return r;
}

const u_char   sas_digital_signature::getLength( void ) const
{ return _length; }

const u_char * sas_digital_signature::getValue( void ) const
{ return _value; }

void  sas_digital_signature::setValueLen( u_char * & val, int length )
{
  delete [] _value;
  _value = val;
  val = 0;
  _length = length;
}

u_char * sas_digital_signature::takeValue( void )
{
  u_char * rval = _value;
  _value = 0; _length = 0;
  return rval;
}

ostream & operator << ( ostream & os, const sas_digital_signature & sds )
{
  os << hex;
  os << "Identifier::" << (int)sds._identifier << endl
     << "Length::" << (int)sds._length << endl
     << "Value::";
  for ( int i = 0; i < sds._length; i++ )
    os << (int)sds._value[ i ];
  os << endl;
  os << dec;
  return os;
}
