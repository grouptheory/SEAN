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
#ifndef __SECURITY_SERVICE_H__
#define __SECURITY_SERVICE_H__

#ifndef LINT
static char const _security_service_h_rcsid_[] =
"$Id: security_service.h,v 1.54 1999/04/15 13:57:43 mountcas Exp $";
#endif

#include <codec/uni_ie/ie_base.h>
#include <DS/containers/list.h>

class ie_errmsg;
class security_association;

class ie_security_service : public InfoElem {
  friend ostream & operator << ( ostream & os, const ie_security_service & ssie );
public:

  int equals(const InfoElem * him) const;
  int equals(const ie_security_service * himptr) const;

  ie_security_service( void );
  ie_security_service( const ie_security_service & rhs );
  ie_security_service( int /* ADD PARAMS HERE */ );

  virtual ~ie_security_service();

  int       encode(u_char * buffer);
  ie_status decode(u_char * buffer, int & id);

  InfoElem * copy(void) const;
  int operator == ( const ie_security_service & rhs ) const;

  const u_char                  getMaxSAID( void ) const;
  int                           Length( void ) const;

  // You lose the ptr when you call this, unless something went wrong ...
  void addSecurityAssociation( security_association * & sa );
  char PrintSpecific(ostream & os) const;

  const list< security_association * > * getSecurityAssociations( void ) const;
  const security_association           * getSecurityAssociation( short relativeID ) const;

  security_association                 * takeSecurityAssociation( short relativeID );

  // If any of the security_associations requires in-band SME,
  // then requiresInBandSME() returns true.  If none of them
  // require in-band SME, then it returns false.
  //
  // BUG- the algorithm works only at the called party, the
  // calling party, and wherever all security associations
  // terminate at the caller.
  bool requiresInBandSME(void);

  // If any of the security_associations support signaled SME,
  // then supportsSignaledSME() returns true.  If none of them
  // support in-band SME then it returns false.
  //
  // BUG- the algorithm works only at the called party, the
  // calling party, and wherever all security associations
  // terminate at the caller.
  bool supportsSignaledSME(void);

private:

  list< security_association * > _security_assocs;
};

class security_service_data;
class security_agent_id;

// Sec. 5.1.3.2.
// The Security Association Section provides the information needed to
// establish a single security service association between two ATM
// security agents.
class security_association {
  friend ostream & operator << ( ostream & os, const security_association & sa );
public:

  enum flowIndicators {
    FirstFlow  = 0x00,
    SecondFlow = 0x01,
    ThirdFlow  = 0x02,
    FourthFlow = 0x03
  };

  enum transportIndicators {
    Signaling = 0x00,
    InBand    = 0x03
  };

  enum discardTypes {
    DoNotDiscardAfterProcessing = 0x00,
    DiscardAfterProcessing      = 0x01
  };

  security_association( void );
  security_association( int identifier );
  ~security_association( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const u_char                  getIdentifier( void ) const;
  const short                   getLength( void ) const;
  const u_char                  getVersion( void ) const;
  const u_char                  getTransportIndicator( void ) const;
  const u_char                  getFlowIndicator( void ) const;
  const u_char                  getDiscard( void ) const;
  const short                   getScope( void ) const;
  const short                   getRelativeId( void ) const;
  const security_agent_id     * getTargetSecEntId( void ) const;
  const security_service_data * getSecurityServiceData( void ) const;

  void  setIdentifier( const u_char id );
  void  setVersion( const u_char ver );
  void  setTransportIndicator( const u_char ti );
  void  setFlowIndicator( const u_char fi );
  void  setDiscard( const u_char discard );
  void  setScope( const short scope );
  void  setRelativeId( const short rid );
  void  setTargetSecEntId( security_agent_id     * & tse );
  // You no longer own the argument passed in, it will be zeroed.
  void  setSecurityServiceData( security_service_data * & ssd );

  // You own the return value from this method
  security_agent_id     * takeTargetSecEntId( void );
  security_service_data * takeSecurityServiceData( void );

private:

  void encode_sa_header( u_char * buffer, int length );

  // This octet identifies the Security Association Service type for
  // this SAS.  User Specified Security Service Identifier ( bit 8 ) 0
  // - ATM Forum Specified Security Service 1 - User Specified
  // Security Service Type ( bits 7 - 1 are encoded in a user-specifed
  // way ) If the bit is cleared, then the following Security
  // Association Service Types apply: 0000001 - Security Message
  // Exchange 0000010 - Label Based Access Control
  u_char _identifier;   
  // This 16-bit octet group is the length of the SAS, excluding the
  // identifier and length bytes
  short  _length;       

  // These 3 bits denote the specification to which this specific SAS
  // type is complaint
  u_char _version:3;
  // This field provides the security agents a means of indicating
  // what transport method will be used to exchange the indicated
  // Security Information Exchange Protocol for this security service.
  u_char _trans_ind:2;
  // This field is used to identify the flow number of the Security
  // Information Exchange for this Security Association Section.
  u_char _flow_ind:2;
  // This indicator specified whether a security agent will discard
  // the SAS after processing.
  u_char _discard:1;

  // These octets describe the intended scope of the security services
  // association, providing an implicit identification of the
  // responding security agent for which this SAS is relevent.
  short  _scope;        
  // The relative identifier provides Security association
  // Identification and explicit Security Association Section
  // ordering.
  short  _relative_id;  
  // This optional octet group is the explicit security agent
  // identifier of the target security agent for this specific SAS.
  // This group is included only when the Explicit bit is set and all
  // other bits are zeroed in the Scope field.  This value can be one
  // of the Security Agent Identificationn Primitives: Initiator
  // Distinguished Name, Responder Distinguished Name, or Security
  // Agent Identifier, which are defined in Sec 6.2.1.
  security_agent_id     * _target_se_id;
  // This section defines the format of the Security Service Data
  // Section of the SAS.  See the below class for further information.
  security_service_data * _ssd_section;

  // Version 1.0 Security Service
  static const u_char _VersionOne = 0x00;
};

// 5.1.3.2.10 SSD 
// There are two types of data section: security message exchange and
// label-based access control.
class security_service_data {
  friend ostream & operator << ( ostream & os, const security_service_data & ssd );
public:

  security_service_data( void );
  virtual ~security_service_data( );

  virtual int                 encode(u_char * buffer) = 0;
  virtual InfoElem::ie_status decode(u_char * buffer, int & id) = 0;

protected:

  virtual void Print( ostream & os ) const = 0;
};

class security_agent_id;
class security_service_spec;
class confidential_sec;
class authentication_sec;

// Sec. 5.1.3.2.10.1
// The SME Protocol is the principal protocol used by two peer
// security agents to communicate security parameters needed to
// establish a security service.
class security_message_exchange : public security_service_data {
public:

  enum formatType {
    Undefined          = 0x00,
    TwoWaySME_Flow2Opt = 0x01, 
    TwoWaySME_Flow2Req = 0x02,
    ThreeWaySME        = 0x03
  };

  security_message_exchange( u_char format = Undefined );
  virtual ~security_message_exchange( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const formatType                  getFormat( void ) const;
  const list<security_agent_id *> * getSecurityAgentId( void ) const;
  const security_service_spec     * getSecurityServiceSpec( void ) const;
  const confidential_sec          * getConfidentiality( void ) const;
  const authentication_sec        * getAuthentication( void ) const;

  void  setFormat( formatType ft );
  // You no longer own the argument passed in, it will be zeroed.
  void  setSecurityAgentId( security_agent_id * & sa );
  void  setSecurityServiceSpec( security_service_spec * & ssp, formatType ft );
  void  setConfidentiality( confidential_sec * & cs );
  void  setAuthentication( authentication_sec * & as );

  // You own the return value from these methods
  list<security_agent_id *> * takeSecurityAgentId( void );
  security_service_spec     * takeSecurityServiceSpec( void );
  confidential_sec          * takeConfidentiality( void );
  authentication_sec        * takeAuthentication( void );

protected:

  void Print( ostream & os ) const;

private:

  // This octet holds the SME type in the first two bits:
  u_char                  _format;
  // This octet group provides the identity of the agents involved in
  // the security exchange.  The primatives that can appear in this
  // section are described in detail in Sec 6.2.1.
  list<security_agent_id * > _identifier;
  // This octet group contains primitives that provide the security
  // negotiation parameters that are used in the SME protocol, and is
  // described in detail in Sec 6.2.2 and 6.2.3.
  security_service_spec * _security_service_spec;
  // The confidentiality section primitive is described in Sec 6.2.4.
  confidential_sec      * _confidentiality_sec;
  // This authentication section primitive is described in Sec 6.2.5.
  authentication_sec    * _authentication_sec;
};

class FIPS188;

// Sec. 5.1.3.2.10.2
// Label Based Access Control is a uni-directional security service
// where an initiating ATM security agent provides an access control
// label from which another security agent can perform an ATM level
// access control decision.  Access control labels will generally be
// limited to a region within which the label has significance.  When
// labels are provided in signaling, any security agent that is
// involved in the connection SETUP or CONNECT message can make an
// access control decision based on the contents of the label.
// Because off this, the label is generally not discarded by the
// security agent that inspects the label.  When labels are passed
// in-band the security agents that receive the in-band exchange can
// enforce the access control policy.
class label_based_access_control : public security_service_data {
public:

  label_based_access_control( void );
  virtual ~label_based_access_control( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

protected:

  void Print( ostream & os ) const;

private:

  static const u_char _identifier = 0x28;
  // This octet indicates the length of the Label-Specific Data, it
  // can be any value in the range 0 to 255.
  u_char              _length;
  // This octet group indicates the security label for the connection
  // in the FIPS 188 format.
  static const u_char _type = 0x01;
  // FIPS 188 Coding Details new class, as yet unnamed!  
  // -- THIS NEEDS TO BE IMPLEMENTED!!
  FIPS188           * _label_specific_data;
};

class FIPSSecurityTag {
public:

  enum SecTagType {
    RestrictiveBitMapType = 0x01,
    PermissiveBitMapType  = 0x06,
    EnumeratedType        = 0x02,
    RangeType             = 0x05,
    FreeFormType          = 0x07
  };

  FIPSSecurityTag( SecTagType type );
  virtual ~FIPSSecurityTag( );

  virtual int                 encode(u_char * buffer) = 0;
  virtual InfoElem::ie_status decode(u_char * buffer, int & id) = 0;

  SecTagType getType( void ) const;

protected:

  SecTagType _type;
};

class FIPSRestrictiveBitMapTag : public FIPSSecurityTag {
public:

  FIPSRestrictiveBitMapTag( u_char se, u_char *& attr, u_char length );
  virtual ~FIPSRestrictiveBitMapTag( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

private:

  u_char   _security_level;
  u_char   _length;
  u_char * _attributes;
};

class FIPSPermissiveBitMapTag : public FIPSSecurityTag {
public:

  FIPSPermissiveBitMapTag( u_char se, u_char *& attr, u_char length );
  virtual ~FIPSPermissiveBitMapTag( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

private:

  u_char   _security_level;
  u_char   _length;
  u_char * _attributes;
};

class FIPSEnumeratedTag : public  FIPSSecurityTag {
public:

  FIPSEnumeratedTag( );
  virtual ~FIPSEnumeratedTag( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

private:

  u_char        _security_level;
  list< short > _enumerated_attribs;
};

class FIPSRangeTag : public  FIPSSecurityTag {
public:

  FIPSRangeTag( );
  virtual ~FIPSRangeTag( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

private:

  u_char        _security_level;
  list< short > _attribute_ranges;
};

class FIPSFreeFormTag : public  FIPSSecurityTag {
public:

  FIPSFreeFormTag( u_char *& data, u_char length );
  virtual ~FIPSFreeFormTag( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

private:

  u_char   _length;
  // Free-form may carry any user-defined type of data
  u_char * _data;
};

class FIPSNamedTagSet {
public:

  FIPSNamedTagSet( );
  ~FIPSNamedTagSet( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

private:

  list< FIPSSecurityTag * > _securityTags;
};

class FIPS188 {
  friend ostream & operator << ( ostream & os, const FIPS188 & fip );
public:

  FIPS188( );
  ~FIPS188( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);
  
private:

  list< FIPSNamedTagSet * > _namedTagSets;
};

// Sec. 6.2.1.3
class security_agent_id {
  friend ostream & operator << ( ostream & os, const security_agent_id & sai );
public:

  enum secTypes {
    X509_DistinguishedName = 0x01,
    ATMEndSystemAddress    = 0x02
  };

  security_agent_id( void );
  virtual ~security_agent_id( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const u_char   getLength( void ) const;
  const secTypes getType( void ) const;
  const u_char * getValue( void ) const;

  void setType( secTypes st );
  // You no longer own the argument passed in, it will be zeroed.
  void setValueLen( u_char * & v, int len );

  // You own the return value from these methods
  u_char * takeValue( void );

  virtual const u_char getIdentifier( void ) const { return _identifier; }

protected:

  // a binary number indicating the length in octets of the Responder
  // distinguished name type and value fields.
  u_char              _length;
  // Security Agent type, see enumerations above
  u_char              _type;
  // This octet group contains the value of the distinguished name of
  // the Reponder of the SME Protocol
  u_char            * _value;

private:

  static const u_char _identifier = 0x84;
};

// Sec. 6.2.1.1.
class initiator_distinguished_name : public security_agent_id {
public:

  initiator_distinguished_name( void );
  virtual ~initiator_distinguished_name( );

  virtual const u_char getIdentifier( void ) const { return _identifier; }

private:

  static const u_char _identifier = 0x82;
};

// Sec. 6.2.1.2.
class responder_distinguished_name : public security_agent_id {
public:

  responder_distinguished_name( void );
  virtual ~responder_distinguished_name( );

  virtual const u_char getIdentifier( void ) const { return _identifier; }

private:

  static const u_char _identifier = 0x83;
};

class security_service_decl;
class security_service_opt;
class security_service_alg;

class security_service_spec {
  friend ostream & operator << ( ostream & os, const security_service_spec & sss );
public:

  security_service_spec( void );
  security_service_spec( const security_service_spec & rhs );
  ~security_service_spec( );

  security_service_spec * copy( void ) const;

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const u_char                         getLength( void ) const;
  const security_service_decl        * getSecurityServiceDecl( void ) const;
  const list<security_service_opt *> * getSecurityServiceOpts( void ) const;
  const list<security_service_alg *> * getSecurityServiceAlg( void ) const;

  // You no longer own the argument passed in, it will be zeroed.
  void setSecurityServiceDecl( security_service_decl * & ssd );
  void setSecurityServiceOpts( security_service_opt  * & sso );
  void setSecurityServiceAlg(  security_service_alg  * & ssa );

  // You own the return value from these methods
  security_service_decl        * takeSecurityServiceDecl( void );
  list<security_service_opt *> * takeSecurityServiceOpts( void );
  list<security_service_alg *> * takeSecurityServiceAlg( void );

  security_service_spec & operator &= ( const security_service_spec & rhs );
  security_service_spec & operator |= ( const security_service_spec & rhs );
  security_service_spec & operator &  ( const security_service_spec & rhs );
  security_service_spec & operator |  ( const security_service_spec & rhs );

  bool empty( void ) const;

private:

  static const u_char          _identifier = 0x88;
  // This optional octet group declares the specific security
  // services that are requested for a VC.
  security_service_decl      * _security_service_dec_sec;
  // This optional octet group indicates specific options for the
  // indicated security service.
  list<security_service_opt *> _security_service_opt_sec;
  // This octet group describes the details of security service
  // algorithms for the indicated security service.
  list<security_service_alg *> _security_service_alg_sec;
};

// Sec. 6.2.2.1.
// The Security Service Declaration Section provides a minimal
// description of the security services that are requested/supported
// by a security agent.  The Security Service Declaration can be
// employed with in-band security message exchange.  In this
// situation, the security service will be negotiated after the VC is
// established.  This method of declaring the security service can be
// employed when communicating a security requirement to a proxy
// security agent.
class security_service_decl { 
  friend ostream & operator << ( ostream & os, const security_service_decl & ssd );
public:

  enum declTypes {
    Unspecified                = 0x00,
    ConfidentialityService     = 0x01,
    IntegrityService           = 0x02,
    AuthenticationService      = 0x04,
    KeyExchangeService         = 0x08,
    CertificateExchangeService = 0x10,
    SessionKeyUpdateService    = 0x20,
    AccessControlService       = 0x40
  };

  security_service_decl( void );
  security_service_decl( const security_service_decl & rhs );
  security_service_decl( u_char decl );
  ~security_service_decl( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  const declTypes getDeclaration( void ) const;
  void            setDeclaration( declTypes dt );

  int             getLength( void ) const;

private:

  static const u_char _identifier = 0x8A;
  u_char              _declaration;
};

// Sec. 6.2.2.2.
// In the 3-way exchange, the initiator indicates what security
// services are required, what security services can be supported
// should the responder require them, and what security services are
// not supported for the connection.  In the 2-way exchange, the
// initiator indicates what security services are required and what
// security services are not supported for the connection.  In the
// 3-way exchange, the responder indicates what security services are
// required and what security services are not supported for the
// connection.
class security_service_opt {
  friend ostream & operator << ( ostream & os, const security_service_opt & sso );
public:

  security_service_opt( void );
  security_service_opt( const security_service_opt & rhs );
  security_service_opt( u_char opt );
  virtual ~security_service_opt( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  virtual security_service_opt * copy( void ) const = 0;
  virtual const u_char getIdentifier( void ) const = 0;

protected:

  // enums from derived classes are stored here
  u_char              _option;
};

// Sec. 6.2.2.2.1.
class data_confidentiality_service_opt : public security_service_opt {
public:

  enum optTypes {
    NotSupported           = 0x00,
    SupportAtATMCellLevel  = 0x01,
    RequiredAtATMCellLevel = 0x81
  };

  data_confidentiality_service_opt( void );
  data_confidentiality_service_opt( const data_confidentiality_service_opt & rhs );
  data_confidentiality_service_opt( optTypes opt );
  virtual ~data_confidentiality_service_opt( );

  security_service_opt * copy( void ) const;
  const u_char   getIdentifier( void ) const { return _identifier; }
  const optTypes getOption( void ) const;
  void           setOption( optTypes ot );

private:

  static const u_char _identifier = 0x90;
};

// Sec. 6.2.2.2.2.
class data_integrity_service_opt : public security_service_opt {
public:

  enum optTypes {
    NotSupported           = 0x00,
    SupportedWithRRProt    = 0x01,
    SupportedWithoutRRProt = 0x02,
    RequiredWithRRProt     = 0x81,
    RequiredWithoutRRProt  = 0x82
  };

  data_integrity_service_opt( void );
  data_integrity_service_opt( const data_integrity_service_opt & rhs );
  data_integrity_service_opt( optTypes opt );
  virtual ~data_integrity_service_opt( );

  security_service_opt * copy( void ) const;
  const u_char   getIdentifier( void ) const { return _identifier; }
  const optTypes getOption( void ) const;
  void           setOption( optTypes ot );

private:

  static const u_char _identifier = 0x92;
};

// Sec. 6.2.2.2.3.
class authentication_service_opt : public security_service_opt {
public:
     
  enum optTypes {
    NotSupported           = 0x00,
    SupportsAuthentication = 0x01,
    RequiresAuthentication = 0x81
  };

  authentication_service_opt( void );
  authentication_service_opt( const authentication_service_opt & rhs );
  authentication_service_opt( optTypes opt );
  virtual ~authentication_service_opt( );

  security_service_opt * copy( void ) const;
  const u_char   getIdentifier( void ) const { return _identifier; }
  const optTypes getOption( void ) const;
  void           setOption( optTypes ot );

private:

  static const u_char _identifier = 0x93;
};

// Sec. 6.2.2.2.4.
class key_exchange_service_opt : public security_service_opt {
public:
     
  enum optTypes {
    NotSupported           = 0x00,
    SupportsAuthentication = 0x01,
    RequiresAuthentication = 0x81
  };

  key_exchange_service_opt( void );
  key_exchange_service_opt( const key_exchange_service_opt & rhs );
  key_exchange_service_opt( optTypes opt );
  virtual ~key_exchange_service_opt( );

  security_service_opt * copy( void ) const;
  const u_char   getIdentifier( void ) const { return _identifier; }
  const optTypes getOption( void ) const;
  void           setOption( optTypes ot );

private:

  static const u_char _identifier = 0x94;
};

// Sec. 6.2.2.2.5.
class session_key_update_service_opt : public security_service_opt {
public:
     
  enum optTypes {
    NotSupported             = 0x00,
    SupportsSessionKeyUpdate = 0x01,
    RequiresSessionKeyUpdate = 0x81
  };

  session_key_update_service_opt( void );
  session_key_update_service_opt( const session_key_update_service_opt & rhs );
  session_key_update_service_opt( optTypes opt );
  virtual ~session_key_update_service_opt( );

  security_service_opt * copy( void ) const;
  const u_char   getIdentifier( void ) const { return _identifier; }
  const optTypes getOption( void ) const;
  void           setOption( optTypes ot );

private:

  static const u_char _identifier = 0x95;
};

// Sec. 6.2.2.2.6.
class access_control_service_opt : public security_service_opt {
public:
     
  enum optTypes {
    NotSupported             = 0x00,
    SupportsAccessControl    = 0x01,
    RequiresAccessControl    = 0x81
  };

  access_control_service_opt( void );
  access_control_service_opt( const access_control_service_opt & rhs );
  access_control_service_opt( optTypes opt );
  virtual ~access_control_service_opt( );

  security_service_opt * copy( void ) const;
  const u_char   getIdentifier( void ) const { return _identifier; }
  const optTypes getOption( void ) const;
  void           setOption( optTypes ot );

private:

  static const u_char _identifier = 0x96;
};

// Sec. 6.2.3.
// The security algorithm section specifies the algorithms to be used
// for the indicated security services.  This includes the algorithm's
// mode of operation that is to be used.
class security_service_alg {
  friend ostream & operator << ( ostream & os, const security_service_alg & ssa );
public:

  security_service_alg( void );
  security_service_alg( const security_service_alg & rhs );
  virtual ~security_service_alg( );

  virtual int                 encode(u_char * buffer) = 0;
  virtual InfoElem::ie_status decode(u_char * buffer, int & id) = 0;

  const u_char getLength( void ) const;

  virtual security_service_alg * copy( void ) const = 0;

protected:

  virtual void Print( ostream & os ) const = 0;

  // A binary number indicating the length in octets of the
  // security_service_alg
  u_char              _length;
};

class data_confidentiality_alg : public security_service_alg {
public:

  enum dcAlgorithms {
    DES56                = 0x01,
    DES40                = 0x02,
    TripleDES            = 0x03,
    FEAL                 = 0x04,
    // Note that the value of bits 1-7 is user-defined
    UserDefinedAlgorithm = 0x80
  };

  enum dcModes {
    Unspecified     = 0x00,
    CBC             = 0x01,
    CounterMode     = 0x02,
    ECB             = 0x03,
    // Note that the value of bits 1-7 is user defined
    UserDefinedMode = 0x80
  };

  data_confidentiality_alg( void );
  data_confidentiality_alg( dcAlgorithms a, dcModes m );
  virtual ~data_confidentiality_alg( );

  security_service_alg * copy( void ) const;

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const dcAlgorithms getAlgorithm( void ) const;
  const int          getAlgorithmOUI( void ) const;
  const dcModes      getModeOfOperation( void ) const;
  const int          getModeOfOperationOUI( void ) const;
  const u_char     * getAlgorithmDetails( void ) const;
  const u_char     * getModeDetails( void ) const;

  void  setAlgorithm( dcAlgorithms a );
  void  setAlgorithmOUI( int oui );
  void  setModeOfOperation( dcModes m );
  void  setModeOfOperationOUI( int oui );
  // You no longer own the argument passed in, it will be zeroed.
  void  setAlgorithmDetails( u_char * & ad );
  void  setModeDetails( u_char * & md );

  // You own the return value from this method
  u_char * takeAlgorithmDetails( void );
  u_char * takeModeDetails( void );

protected:

  void Print( ostream & os ) const;
  
private:

  static const u_char _identifier = 0xA0;
  // the data confidentiality algorithm, see enumerations
  u_char              _algorithm;
  // Organizationally Unique Identifer (OUI), these octects are only
  // included if the algorithm is user defined.
  int                 _algorithm_oui;
  // the mode of operation, see enumerations
  u_char              _mode_of_operation;
  // These octects are only included if the mode of operation is
  // user-defined
  int                 _mode_oui;
  // These octets idicate coding details for each data confidentiality
  // algorithm.
  u_char            * _algorithm_details;
  // These octets indicate coding details for each data
  // confidentiality mode of operation.
  u_char            * _mode_details;
};

// Sec. 6.2.3.2
// This octet group indicates an algorithm for the data integrity
// service.
class data_integrity_alg : public security_service_alg {
public:

  enum diAlgorithms {
    HMAC_MD5          = 0x01,
    HMAC_SHA_1        = 0x02,
    HMAC_RIPEMD_160   = 0x03,
    MAC_DES_CBC       = 0x04,
    MAC_DES40_CBC     = 0x05,
    MAC_TripleDES_CBC = 0x06,
    MAC_FEAL_CBC      = 0x07,
  };

  enum UserDefInd {
    // User defined Indicator
    ATMForumDefinedAlgorithm = 0x00,
    UserDefinedAlgorithm     = 0x01,
  };

  enum ReplayReorderProtInd {
    // Replay/Reordering protection
    NoRRProtectionProvided   = 0x00,
    RRProtectionProvided     = 0x01
  };

  data_integrity_alg( void );
  data_integrity_alg( diAlgorithms a );
  virtual ~data_integrity_alg( );

  security_service_alg * copy( void ) const;

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const UserDefInd           getUserDefinedInd( void ) const;
  const ReplayReorderProtInd getReplayProtInd( void ) const;
  const diAlgorithms         getAlgorithm( void ) const;
  const int                  getAlgorithmOUI( void ) const;
  const u_char             * getAlgorithmDetails( void ) const;

  void  setUserDefinedInd( UserDefInd udi );
  void  setReplayProtInd( ReplayReorderProtInd rrpi );
  void  setAlgorithm( diAlgorithms alg );
  void  setAlgorithmOUI( int oui );
  // You no longer own the argument passed in, it will be zeroed.
  void  setAlgorithmDetails( u_char * & ad );

  // You own the return value from this method
  u_char * takeAlgorithmDetails( void );

protected:

  void Print( ostream & os ) const;
  
private:

  static const u_char _identifier = 0xA2;
  // user defined indicator
  u_char              _user:1;
  // replay/reordering protection indicator
  u_char              _replay:1;
  // data integrity algorithm, see enumerations
  u_char              _algorithm:6;
  // The following octets are including only if the
  // algorithm is user defined.
  int                 _algorithm_oui;
  // data integrity details
  u_char            * _algorithm_details;
};

// Sec. 6.2.3.3.
// This octet group indicates a hashing algorithm for the
// authentication service.
class hash_alg : public security_service_alg {
public:

  enum hAlgorithms {
    MD5                  = 0x01,
    SHA_1                = 0x02,
    RIPEMD_160           = 0x03,
    // Note that the value of bits 1-7 is user defined
    UserDefinedAlgorithm = 0x80
  };

  hash_alg( void );
  hash_alg( hAlgorithms a );
  virtual ~hash_alg( );

  security_service_alg * copy( void ) const;

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const hAlgorithms getAlgorithm( void ) const;
  const int         getAlgorithmOUI( void ) const;
  const u_char    * getAlgorithmDetails( void ) const;

  void  setAlgorithm( hAlgorithms alg );
  void  setAlgorithmOUI( int oui );
  // You no longer own the argument passed in, it will be zeroed.
  void  setAlgorithmDetails( u_char * & ad );

  // You own the return value from this method
  u_char * takeAlgorithmDetails( void );

protected:

  void Print( ostream & os ) const;
  
private:

  static const u_char _identifier = 0xA4;
  // Hash algorithm, see enumerations
  u_char              _algorithm;
  // The following octets are only included if 
  // the algorithm is user-defined.
  int                 _algorithm_oui;
  // Hash algorithm details 
  u_char            * _algorithm_details;
};

// Sec. 6.2.3.4.
// This octet group indicates a signature algorithm for the
// Authentication Service.
class signature_alg : public security_service_alg {
public:

  enum sAlgorithms {
    RSA                  = 0x01,
    DSA                  = 0x02,
    EllipticCurve        = 0x03,
    ESIGN                = 0x04,
    DES_CBC              = 0x05,
    DES40_CBC            = 0x06,
    TripleDES_CBC        = 0x07,
    FEAL_CBC             = 0x08,
    // Note that the value of bits 1-7 is user defined
    UserDefinedAlgorithm = 0x80
  };

  signature_alg( void );
  signature_alg( sAlgorithms a );
  virtual ~signature_alg( );

  security_service_alg * copy( void ) const;

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const sAlgorithms getAlgorithm( void ) const;
  const int         getAlgorithmOUI( void ) const;
  const u_char    * getAlgorithmDetails( void ) const;

  void  setAlgorithm( sAlgorithms alg );
  void  setAlgorithmOUI( int oui );
  // You no longer own the argument passed in, it will be zeroed.
  void  setAlgorithmDetails( u_char * & ad );

  // You own the return value from this method
  u_char * takeAlgorithmDetails( void );

protected:

  void Print( ostream & os ) const;
  
private:

  static const u_char _identifier = 0xA6;
  // Signature Algorithm, see enumerations
  u_char              _algorithm;
  // The following octets are only included if 
  // the algorithm is user-defined.
  int                 _algorithm_oui;
  // Signature algorithm details 
  u_char            * _algorithm_details;
};

// Sec. 6.2.3.5.
// This octet group indicates an algorithm for key exchange.  This
// algorithm is used to encrypt the contents part of the "Security
// Confidential Parameters" information element.
class key_exchange_alg : public security_service_alg {
public:

  enum keAlgorithms {
    RSA                  = 0x01,
    Diffie_Hellman       = 0x02,
    ECKAS_DH_Prime       = 0x03, 
    ECKAS_DH_Char        = 0x04, 
    DES_CBC              = 0x05,
    DES40_CBC            = 0x06,
    TripleDES_CBC        = 0x07,
    FEAL_CBC             = 0x08,
    // Note that the value of bits 1-7 is user defined
    UserDefinedAlgorithm = 0x80
  };

  key_exchange_alg( void );
  key_exchange_alg( keAlgorithms k );
  virtual ~key_exchange_alg( );

  security_service_alg * copy( void ) const;

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const keAlgorithms getAlgorithm( void ) const;
  const int         getAlgorithmOUI( void ) const;
  const u_char    * getAlgorithmDetails( void ) const;

  void  setAlgorithm( keAlgorithms alg );
  void  setAlgorithmOUI( int oui );
  // You no longer own the argument passed in, it will be zeroed.
  void  setAlgorithmDetails( u_char * & ad );

  // You own the return value from this method
  u_char * takeAlgorithmDetails( void );

protected:

  void Print( ostream & os ) const;
  
private:

  static const u_char _identifier = 0xA8;
  // Key Exchange Algorithm, see enumerations
  u_char              _algorithm;
  // The following octets are only included if 
  // the algorithm is user-defined.
  int                 _algorithm_oui;
  // Key Exchange algorithm details 
  u_char            * _algorithm_details;
};

// Sec. 6.2.3.6.
// This octet group indicates a session key update algorithm for
// updating the data confidentiality and data integrity session keys.
class session_key_update_alg : public security_service_alg {
public:

  enum skAlgorithms {
    SKE_MD5              = 0x01,
    SKE_SHA_1            = 0x02,
    SKE_RIPEMD_160       = 0x03,
    // Note that the value of bits 1-7 is user defined
    UserDefinedAlgorithm = 0x80
  };

  session_key_update_alg( void );
  session_key_update_alg( skAlgorithms a );
  virtual ~session_key_update_alg( );

  security_service_alg * copy( void ) const;

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const skAlgorithms getAlgorithm( void ) const;
  const int         getAlgorithmOUI( void ) const;
  const u_char    * getAlgorithmDetails( void ) const;

  void  setAlgorithm( skAlgorithms alg );
  void  setAlgorithmOUI( int oui );
  // You no longer own the argument passed in, it will be zeroed.
  void  setAlgorithmDetails( u_char * & ad );

  // You own the return value from this method
  u_char * takeAlgorithmDetails( void );

protected:

  void Print( ostream & os ) const;
  
private:

  static const u_char _identifier = 0xAA;
  // Session Key Update algorithm
  u_char              _algorithm;
  // The following octets are only included if 
  // the algorithm is user-defined.
  int                 _algorithm_oui;
  // Session Key Update algorithm details 
  u_char            * _algorithm_details;
};

// Sec. 6.2.3.7.
// This octet group indicates a set of algorithms support
// authentication service.
class authentication_algorithm_profile_group : public security_service_alg {
public:

  enum aAlgorithms {
    AUTH_1               = 0x01, // Sig=DES/CBC, Hash=N/A
    AUTH_2               = 0x02, // Sig=DSA, Hash=SHA_1
    AUTH_3               = 0x03, // Sig=EC/DSA, Hash=SHA_1
    AUTH_4               = 0x04, // Sig=ESIGN, Hash=MD5
    AUTH_5               = 0x05, // Sig=FEAL/CBC, Hash=N/A
    AUTH_6               = 0x06, // Sig=RSA, Hash=MD5
    // Note that the value of bits 1-7 is user defined
    UserDefinedAlgorithm = 0x80
  };

  authentication_algorithm_profile_group( void );
  authentication_algorithm_profile_group( const char * alg );
  virtual ~authentication_algorithm_profile_group( );

  security_service_alg * copy( void ) const;

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const aAlgorithms     getAlgorithm( void ) const;
  const int             getAlgorithmOUI( void ) const;
  const u_char          getSignatureLength( void ) const;
  const signature_alg * getSignatureAlgorithm( void ) const;
  const u_char          getHashLength( void ) const;
  const hash_alg      * getHashAlgorithm( void ) const;

  void  setAlgorithm( aAlgorithms alg );
  void  setAlgorithmOUI( int oui );
  // You no longer own the argument passed in, it will be zeroed.
  void  setSignatureAlgorithm( signature_alg * & sa );
  void  setHashAlgorithm( hash_alg * & ha );

  // You own the return value from this method
  signature_alg * takeSignatureAlgorithm( void );
  hash_alg      * takeHashAlgorithm( void );

protected:

  void Print( ostream & os ) const;
  
private:

  static const u_char _identifier = 0xAC;
  // Authentication Algorithm Group
  u_char              _algorithm;
  // The following octets are only included if the algorithm is
  // user-defined.
  int                 _algorithm_oui;
  // These optional octets include a binary number indicating the
  // length of the signature algorithm details and the coding details
  // for each signature algorithm.
  static const u_char _sig_alg_id = 0xB2;
  u_char              _sig_alg_length;
  signature_alg     * _sig_alg_details;
  // These optional octets include a binary number indicating the
  // length of the hash algorithm details and the coding details for
  // each hash algorithm.
  static const u_char _hash_alg_id = 0xB4;
  u_char              _hash_alg_length;
  hash_alg          * _hash_alg_details;
};

// Sec. 6.2.3.8.
// This octet group indicates a set of algorithms support integrity
// service.
class integrity_algorithm_profile_group : public security_service_alg {
public:

  enum iAlgorithms {
    INTEG_1              = 0x01,
    INTEG_2              = 0x02,
    INTEG_3              = 0x03,
    INTEG_4              = 0x04,
    INTEG_5              = 0x05,
    INTEG_6              = 0x06,
    INTEG_7              = 0x07,
    INTEG_8              = 0x08,
    INTEG_9              = 0x09,
    INTEG_10             = 0x0A,
    INTEG_11             = 0x0B,
    // Note that the value of bits 1_7 is user defined
    UserDefinedAlgorithm = 0x80
  };

  integrity_algorithm_profile_group( void );
  integrity_algorithm_profile_group( const char * alg );
  virtual ~integrity_algorithm_profile_group( );

  security_service_alg * copy( void ) const;

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const iAlgorithms              getAlgorithm( void ) const;
  const int                      getAlgorithmOUI( void ) const;
  const u_char                   getMACLength( void ) const;
  const data_integrity_alg     * getMACAlgorithm( void ) const;
  const u_char                   getSignatureLength( void ) const;
  const signature_alg          * getSignatureAlgorithm( void ) const;
  const u_char                   getKeyExchangeLength( void ) const;
  const key_exchange_alg       * getKeyExchangeAlgorithm( void ) const;
  const u_char                   getKeyUpdateLength( void ) const;
  const session_key_update_alg * getKeyUpdateAlgorithm( void ) const;
  const u_char                   getHashLength( void ) const;
  const hash_alg               * getHashAlgorithm( void ) const;
  
  void  setAlgorithm( iAlgorithms alg );
  void  setAlgorithmOUI( int oui );
  // You no longer own the argument passed in, it will be zeroed.
  void  setMACAlgorithm( data_integrity_alg * & dia );
  void  setSignatureAlgorithm( signature_alg * & sa );
  void  setKeyExchangeAlgorithm( key_exchange_alg * & kea );
  void  setKeyUpdateAlgorithm( session_key_update_alg * & sku );
  void  setHashAlgorithm( hash_alg * & ha );

  // You own the return value from this method
  data_integrity_alg     * takeMACAlgorithm( void );
  signature_alg          * takeSignatureAlgorithm( void );
  key_exchange_alg       * takeKeyExchangeAlgorithm( void );
  session_key_update_alg * takeKeyUpdateAlgorithm( void );
  hash_alg               * takeHashAlgorithm( void );
  
protected:

  void Print( ostream & os ) const;
  
private:

  static const u_char      _identifier = 0xAE;
  // Integrity Algorithm Group
  u_char                   _algorithm;
  // The following octets are only included if the algorithm is
  // user-defined.
  int                      _algorithm_oui;
  // These optional octets include a binary number indicating the
  // length of the MAC algorithm details and the coding details for
  // each MAC algorithm.
  static const u_char      _mac_alg_id = 0xB6;
  u_char                   _mac_alg_length;
  data_integrity_alg     * _mac_alg_details;
  // These optional octets include a binary number indicating the
  // length of the signature algorithm details and the coding details
  // for each signature algorithm.
  static const u_char      _sig_alg_id = 0xB2;
  u_char                   _sig_alg_length;
  signature_alg          * _sig_alg_details;
  // These optional octets include a binary number indicating the
  // length of the key exchange algorithm details and the coding
  // details for each key algorithm.
  static const u_char      _key_ex_alg_id = 0xB8;
  u_char                   _key_ex_alg_length;
  key_exchange_alg       * _key_ex_alg_details;
  // These optional octets include a binary number indicating the
  // length of the key update algorithm details and the coding details
  // for each key update algorithm.
  static const u_char      _key_up_alg_id = 0xBA;
  u_char                   _key_up_alg_length;
  session_key_update_alg * _key_up_alg_details;
  // These optional octets include a binary number indicating the
  // length of the hash algorithm details and the coding details for
  // each hash algorithm.
  static const u_char      _hash_alg_id = 0xB4;
  u_char                   _hash_alg_length;
  hash_alg               * _hash_alg_details;
};

// Sec. 6.2.3.9.
// This octet group indicates a set of algorithms supporting
// confidentiality service.
class confidentiality_algorithm_profile_group : public security_service_alg {
public:

  enum cAlgorithms {
    CONF_1               = 0x01,
    CONF_2               = 0x02,
    CONF_3               = 0x03,
    CONF_4               = 0x04,
    CONF_5               = 0x05,
    CONF_6               = 0x06,
    CONF_7               = 0x07,
    CONF_8               = 0x08,
    CONF_9               = 0x09,
    CONF_10              = 0x0A,
    CONF_11              = 0x0B,
    CONF_12              = 0x0C,
    CONF_13              = 0x0D,
    CONF_14              = 0x0E,
    CONF_15              = 0x0F,
    CONF_16              = 0x10,
    CONF_17              = 0x11,
    CONF_18              = 0x12,
    CONF_19              = 0x13,
    CONF_20              = 0x14,
    // Note that the value of bits 1-7 is user defined
    UserDefinedAlgorithm = 0x80
  };

  confidentiality_algorithm_profile_group( void );
  confidentiality_algorithm_profile_group( const char * alg );
  virtual ~confidentiality_algorithm_profile_group( );

  security_service_alg * copy( void ) const;

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const cAlgorithms                getAlgorithm( void ) const;
  const int                        getAlgorithmOUI( void ) const;
  const u_char                     getDataConfidentialityLength( void ) const;
  const data_confidentiality_alg * getDataConfidentialityAlgorithm( void ) const;
  const u_char                     getSignatureLength( void ) const;
  const signature_alg            * getSignatureAlgorithm( void ) const;
  const u_char                     getKeyExchangeLength( void ) const;
  const key_exchange_alg         * getKeyExchangeAlgorithm( void ) const;
  const u_char                     getKeyUpdateLength( void ) const;
  const session_key_update_alg   * getKeyUpdateAlgorithm( void ) const;
  const u_char                     getHashLength( void ) const;
  const hash_alg                 * getHashAlgorithm( void ) const;
  
  void  setAlgorithm( cAlgorithms alg );
  void  setAlgorithmOUI( int oui );
  // You no longer own the argument passed in, it will be zeroed.
  void  setDataConfidentialityAlgorithm( data_confidentiality_alg * & dca );
  void  setSignatureAlgorithm( signature_alg * & sa );
  void  setKeyExchangeAlgorithm( key_exchange_alg * & kea );
  void  setKeyUpdateAlgorithm( session_key_update_alg * & sku );
  void  setHashAlgorithm( hash_alg * & ha );

  // You own the return value from this method
  data_confidentiality_alg * takeDataConfidentialityAlgorithm( void );
  signature_alg            * takeSignatureAlgorithm( void );
  key_exchange_alg         * takeKeyExchangeAlgorithm( void );
  session_key_update_alg   * takeKeyUpdateAlgorithm( void );
  hash_alg                 * takeHashAlgorithm( void );

protected:

  void Print( ostream & os ) const;
  
private:

  static const u_char  _identifier = 0xB0;
  // Confidentiality Algorithm Group
  u_char               _algorithm;
  // The following octets are only included if the algorithm is
  // user-defined.
  int                  _algorithm_oui;
  // These optional octets include a binary number indicating the
  // length of the encryption algorithm details and the coding details
  // for each encryption algorithm.
  static const u_char        _enc_alg_id = 0xBE;
  u_char                     _enc_alg_length;
  data_confidentiality_alg * _enc_alg_details;
  // These optional octets include a binary number indicating the
  // length of the signature algorithm details and the coding details
  // for each signature algorithm.
  static const u_char  _sig_alg_id = 0xB2;
  u_char               _sig_alg_length;
  signature_alg      * _sig_alg_details;
  // These optional octets include a binary number indicating the
  // length of the key exchange algorithm details and the coding
  // details for each key algorithm.
  static const u_char  _key_ex_alg_id = 0xB8;
  u_char               _key_ex_alg_length;
  key_exchange_alg   * _key_ex_alg_details;
  // These optional octets include a binary number indicating the
  // length of the key update algorithm details and the coding details
  // for each key update algorithm.
  static const u_char      _key_up_alg_id = 0xBA;
  u_char                   _key_up_alg_length;
  session_key_update_alg * _key_up_alg_details;
  // These optional octets include a binary number indicating the
  // length of the hash algorithm details and the coding details for
  // each hash algorithm.
  static const u_char  _hash_alg_id = 0xB4;
  u_char               _hash_alg_length;
  hash_alg           * _hash_alg_details;
};

class confidential_params;

// Sec. 6.2.4.
// The security confidentiality section provides for the exchange of
// confidential information in the SME.  This optional section
// provides an opportunity to include encrypted data in an SAS.
class confidential_sec {
  friend ostream & operator << ( ostream & os, const confidential_sec & cs );
public:

  confidential_sec( void );
  ~confidential_sec( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const short                 getLength( void ) const;
  const confidential_params * getConfidentialParams( void ) const;

  // You no longer own the argument passed in, it will be zeroed.
  void  setConfidentialParams( confidential_params * & cp );
  
  // You own the return value from this method
  confidential_params * takeConfidentialParams( void );

private:

  static const u_char   _identifier = 0x40;
  // This 16-octet group provides the length of the Confidential
  // Section.  This is the length of the ciphertext, which may not be
  // the same as the plaintext.
  short                 _length;
  // This octet group contains encrypted data that requires decruption
  // in order to be meaningful to the security agent.  In the SME,
  // this is the encryption of the Confidential Parameters primitive,
  // using the key exchange algorithm referenced by the SAS
  // identifier.
  confidential_params * _confidential_data;
};

class master_key;
class first_confidentiality_key;
class first_integrity_key;

// Sec. 6.2.4.1.
// The SME protocol supports the exchange of confidentiality
// parameters.  These SAS primitives can be consolidated in a single
// Confidential Parameters section, or can be included individually.
class confidential_params {
  friend ostream & operator << ( ostream & os, const confidential_params & cp );
public:

  confidential_params( void );
  ~confidential_params( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  const master_key                * getMasterKey( void ) const;
  const first_confidentiality_key * getFirstConfidentialityKey( void ) const;
  const first_integrity_key       * getFirstIntegrityKey( void ) const;

  void  setMasterKey( master_key * & mk );
  void  setFirstConfidentialityKey( first_confidentiality_key * & fck );
  void  setFirstIntegrityKey( first_integrity_key * & fik );

  master_key                * takeMasterKey( void );
  first_confidentiality_key * takeFirstConfidentialityKey( void );
  first_integrity_key       * takeFirstIntegrityKey( void );

private:

  static const u_char         _identifier = 0xC4;
  // This octet group provides the Master Key for the cryptographic
  // services.  Sec 5.2.3.
  master_key                * _master_key;
  // This field contains the initial Data Confidentiality Session Key.
  // Sec. 5.2.4.
  first_confidentiality_key * _first_confid_key;
  // This field contains the initial Data Integrity Session Key.
  // Sec. 5.2.4.
  first_integrity_key       * _first_integ_key;
};

// Sec. 6.2.4.2.
// This octet group contains the key used to encrypt the subsequent
// session key.
class master_key {
  friend ostream & operator << ( ostream & os, const master_key & mk );
public:

  master_key( void );
  ~master_key( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const u_char   getLength( void ) const;
  const u_char * getValue( void ) const;

  // You no longer own the argument passed in, it will be zeroed.
  void  setValue( u_char * & val );

  // You own the return value from this method
  u_char * takeValue( void );

private:

  static const u_char _identifier = 0xC8;
  // A binary number indicating the length in octets of the Master key
  // value.
  u_char              _length;
  // This field contains the binary encoding of the Master Key value,
  // with bit 8 of the first octet being the most significant bit, and
  // bit 1 of the last octet being the least significant bit.  The
  // Master Key length must be an integer multiple of 8 bits.
  u_char            * _value;
};

// Sec. 6.2.4.3.
// This octet group contains the first session key to be used to
// provide the data confidentiality service.
class first_confidentiality_key {
  friend ostream & operator << ( ostream & os, const first_confidentiality_key & fck );
public:

  first_confidentiality_key( void );
  ~first_confidentiality_key( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const u_char   getLength( void ) const;
  const u_char * getValue( void ) const;

  // You no longer own the argument passed in, it will be zeroed.
  void  setValue( u_char * & val );

  // You own the return value from this method
  u_char * takeValue( void );

private:

  static const u_char _identifier = 0xCA;
  // A binary number indicating the length in octets of the First Data
  // Confidentiality Session Key value.
  u_char              _length;
  // This fiield contains the binary encoding of the First Data
  // Confidentiality Session Key value, with bit 8 of the first octet
  // being the most significant bit, and bit 1 of the last octet being
  // the least significant bit.  The First Data Confidentiality
  // Session Key length must be an integer multiple of 8 bits.
  u_char            * _value;
};

// Sec. 6.2.4.4.
// This octet group contains the first session key to be used to
// provide the data integrity service.
class first_integrity_key {
  friend ostream & operator << ( ostream & os, const first_integrity_key & fik );
public:

  first_integrity_key( void );
  ~first_integrity_key( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const u_char   getLength( void ) const;
  const u_char * getValue( void ) const;

  // You no longer own the argument passed in, it will be zeroed.
  void  setValue( u_char * & val );

  // You own the return value from this method
  u_char * takeValue( void );

private:

  static const u_char   _identifier = 0xCC;
  // A binary number indicating the length in octets of the First Data
  // Integrity Session Key value.
  u_char                _length;
  // This field contains the binary encoding of the First Data
  // Integrity Session Key value, with bit 8 of the first octet being
  // the most significant bit, and bit 1 of the last octet being the
  // least significant bit.  The First Data Integrity Session Key
  // length must be an integer multiple of 8 bits.
  u_char              * _value;
};

class initiator_nonce;
class responder_nonce;
class time_variant_time_stamp;
class credentials;
class digital_signature;

// Sec. 6.2.5.
// This octet group specifies the SAS Authentication Section.  This
// section is intented to support authentication as specified in the
// SME Protocol.  However, the design provides a generic
// authentication service that can be applied to any part of the SSIE.
// All fields of this section are optional.
class authentication_sec {
  friend ostream & operator << ( ostream & os, const authentication_sec & as );
public:

  authentication_sec( void );
  ~authentication_sec( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const initiator_nonce         * getInitiatorRandomNumber( void ) const;
  const responder_nonce         * getResponderRandomNumber( void ) const;
  const time_variant_time_stamp * getTimeStamp( void ) const;
  const credentials             * getCredentials( void ) const;
  const digital_signature       * getDigitalSignature( void ) const;

  // You no longer own the argument passed in, it will be zeroed.
  void  setInitiatorRandomNumber( initiator_nonce * & in );
  void  setResponderRandomNumber( responder_nonce * & rn );
  void  setTimeStamp( time_variant_time_stamp * & ts );
  void  setCredentials( credentials * & creds );
  void  setDigitalSignature( digital_signature * & ds );

  // You own the return value from this method
  initiator_nonce         * takeInitiatorRandomNumber( void );
  responder_nonce         * takeResponderRandomNumber( void );
  time_variant_time_stamp * takeTimeStamp( void );
  credentials             * takeCredentials( void );
  digital_signature      * takeDigitalSignature( void );
 
private:

  static const u_char       _identifier = 0x50;
  // This section provides the initiator supplied nonce, if provided
  // in the exchange.
  initiator_nonce         * _initiator_random_number;
  // This section provides the responder supplied nonce, if provided
  // in the exchange.
  responder_nonce         * _responder_random_number;
  // This section specifies a unique time stamp/sequence number that
  // will provide replay protection.
  time_variant_time_stamp * _time_stamp;
  // This section is employed when credential presentation or exchange
  // is required in the SME.
  credentials             * _credentials;
  // This section is the Digitial Signature.
  digital_signature       * _digital_signature;
};

// Sec. 6.2.5.1
class initiator_nonce {
  friend ostream & operator << ( ostream & os, const initiator_nonce & in );
public:

  initiator_nonce( void );
  initiator_nonce( int rn );
  ~initiator_nonce( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  const int getRandomNumber( void ) const;

private:

  static const u_char _identifier = 0xD4;
  // 32 bit binary random number;
  int                 _random_number;
};

// Sec. 6.2.5.2
class responder_nonce {
  friend ostream & operator << ( ostream & os, const responder_nonce & rn );
public:

  responder_nonce( void );
  responder_nonce( int rn );
  ~responder_nonce( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  const int getRandomNumber( void ) const;

private:

  static const u_char _identifier = 0xD5;
  // 32 bit binary random number;
  int                 _random_number;
};

// Sec. 6.2.5.3
// This field provides a label that can be used as a unique identifier
// for a given security IE, such as an IE used in an authentication
// exchange.
class time_variant_time_stamp {
  friend ostream & operator << ( ostream & os, const time_variant_time_stamp & tvts );
public:

  time_variant_time_stamp( void );
  time_variant_time_stamp( int time_stamp, int seq_num );
  ~time_variant_time_stamp( );
  
  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  const int getTimeStamp( void ) const;
  const int getSequenceNumber( void ) const;

private:

  static const u_char _identifier = 0xD6;
  // The time stamp value is a 4 byte coordinated universal time, the
  // GMT at which the signature was generated.  This value is the
  // binary encoding of the number of seconds since 00:00:00 GMT on
  // Jan. 1, 1970.
  int                 _time_stamp_value;
  // The sequence number is a 4 octet binary number that is
  // incremented with each authentication flow that is sent to the
  // same destination with the same time stamp value.  When the time
  // stamp value changes, the sequence number is reset to 0.
  int                 _sequence_number;
};

// Sec. 6.2.5.4.
// Certificate exchange is an optional feature of the SME Protocol,
// and is in general a credential presentation feature.  User defined
// credential types may contain a complete set of security services,
// and as such, additional authentication sections, such as time
// stamps, random numbers, and signatures, may not be required in the
// authentication process.
class credentials {
  friend ostream & operator << ( ostream & os, const credentials & c );
public:

  enum credTypes {
    X509_Cert_V1          = 0x01,
    X509_Cert_V2          = 0x02,
    X509_Cert_V3          = 0x03,
    // Note that the value of bits 1-7 is user-defined
    UserDefinedCredential = 0x80
  };

  credentials( void );
  credentials( credTypes t, u_char * & value, int length );
  ~credentials( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const short     getLength( void );
  const credTypes getType( void ) const;
  const u_char  * getValue( void ) const;

  // You no longer own the argument passed in, it will be zeroed.
  void  setTypeValueLen( credTypes t, u_char * & val, int length );

  // You own the return value from this method
  u_char  * takeValue( void );

private:

  static const u_char _identifier = 0xD8;
  // The overall length of the SSIE limits maximum length of the
  // credentials, when signaling channel is used for certificate
  // exchange.
  short               _length;
  // Credential Type, see enumerations
  u_char              _type;
  // This field contains the binary encoding of the credentials value,
  // the format of which conforms to the type specified.
  u_char            * _value;
};

// Sec. 6.2.5.5.
// This octet group contains the signature value computed over the
// objects required by the SME Protocol.  The signature algorithm is
// specified in other sections of the SSIE SAAS.
class digital_signature {
  friend ostream & operator << ( ostream & os, const digital_signature & ds );
public:

  digital_signature( void );
  digital_signature( u_char * & value, int length );
  ~digital_signature( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const u_char   getLength( void ) const;
  const u_char * getValue( void ) const;

  // You no longer own the argument passed in, it will be zeroed.
  void  setValueLen( u_char * & val, int length );

  // You own the return value from this method
  u_char * takeValue( void );

private:

  static const u_char _identifier = 0xDA;
  // a binary number indicating the length in octets of the digital
  // signature value.
  u_char              _length;
  // This field contains the binary encoding of the signature value.
  // This is signature algorithm specific and both security agents in
  // the SME must know the signature algorithm used.  Sec. 6.2.5.5.
  u_char            * _value;
};

// Sec. 6.2.5.6.
// This octet group contains a digital signature computed over an SAS.
// The digital signature value is computed over the entire contents of
// the SAS, up to, but not including, this SAS primitive.  An SAS can
// contain any number of digital signatures.  The context of all
// signatures in a particular SAS, i.e. the signature format,
// algorithm, and keys needed to calculate the signature, are
// identical, as the scope of the context of each signature is defined
// by the SAS.
class sas_digital_signature {
  friend ostream & operator << ( ostream & os, const sas_digital_signature & sds );
public:

  sas_digital_signature( void );
  sas_digital_signature( u_char * & value, int length );
  ~sas_digital_signature( );

  int                 encode(u_char * buffer);
  InfoElem::ie_status decode(u_char * buffer, int & id);

  // You DO NOT own the return value, just glance at it then forget it
  const u_char   getLength( void ) const;
  const u_char * getValue( void ) const;

  // You no longer own the argument passed in, it will be zeroed.
  void  setValueLen( u_char * & val, int length );

  // You own the return value from this method
  u_char * takeValue( void );

private:

  static const u_char _identifier = 0xDC;
  // A binary number indicating the length in octets of the digital
  // signature value.
  short               _length;
  // This field contains the binary encoding of the signature value.
  // The signature is computed over the encoded contents of this
  // Security Association Section (excluding this digital signature
  // octet group).  The contents of this SAS must remain intact as
  // this SSIE (and SAS) travels through the network.  That is, the
  // network must not change or reorder the contents of the SAS.
  // Sec. 6.2.5.5.
  u_char            * _value;
};

#endif // __SECURITY_SERVICE_H__
