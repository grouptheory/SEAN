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

#ifndef LINT
static char const _sscop_visitors_cc_rcsid_[] =
"$Id: sscop_visitors.cc,v 1.5 1999/03/26 00:41:17 battou Exp $";
#endif
#include <common/cprototypes.h>

#include <sys/types.h> 
#include "sscop.h"
#include "sscopState.h"
#include "sscop_visitors.h"

extern "C" {
#include <netinet/in.h>
};


SSCOPVisitor::SSCOPVisitor(void) : Visitor(_my_type),
  _vt(Unknown), _br(0), _sscop_uu(0), _uulen(0), _PDUtype(SSCOPconn::SSCOP_BAD),
  _pdu(0), _len(0), _mr(-1), _ps(-1), _saved_ps(-1), _s(-1), _sbit(false), 
  _r(-1), _sq(-1), _list(0), _llen(0)
{
  //  SetLoggingOn();  
}

SSCOPVisitor::SSCOPVisitor(SSCOPVisitorType vt, u_char *sscop_uu, int uulen): Visitor(_my_type),
  _vt(vt), _br(0), _sscop_uu(sscop_uu), _uulen(uulen), _PDUtype(SSCOPconn::SSCOP_BAD), _pdu(0),
  _len(0), _mr(-1), _ps(-1), _saved_ps(-1), _s(-1), _sbit(false), _r(-1), _sq(-1), 
  _list(0), _llen(0)
{
  switch(vt)
    {
    case SSCOPVisitor::BeginReq :
    case SSCOPVisitor::BeginInd :
      _PDUtype = SSCOPconn::SSCOP_BGN;
      break;

    case SSCOPVisitor::BeginAckReq :
    case SSCOPVisitor::BeginAckInd :
      _PDUtype = SSCOPconn::SSCOP_BGAK;
      break;

    case SSCOPVisitor::BeginRejReq :
    case SSCOPVisitor::BeginRejInd :
      _PDUtype = SSCOPconn::SSCOP_BGREJ;
      break;

    case SSCOPVisitor::EndReq:
    case SSCOPVisitor::EndInd:
      _PDUtype = SSCOPconn::SSCOP_END;
      break;

    case SSCOPVisitor::EndAckReq:
    case SSCOPVisitor::EndAckInd:
      _PDUtype = SSCOPconn::SSCOP_ENDAK;
      break;

    case SSCOPVisitor::ResyncReq:
    case SSCOPVisitor::ResyncInd:
      _PDUtype = SSCOPconn::SSCOP_RS;
      break;

    case SSCOPVisitor::ResyncAckReq:
    case SSCOPVisitor::ResyncAckInd:
      _PDUtype = SSCOPconn::SSCOP_RSAK;
      break;

    case SSCOPVisitor::RecovReq:
    case SSCOPVisitor::RecovInd:
      _PDUtype = SSCOPconn::SSCOP_ER;
      break;

    case SSCOPVisitor::RecovAckReq:
    case SSCOPVisitor::RecovAckInd:
      _PDUtype = SSCOPconn::SSCOP_ERAK;
      break;

    case SSCOPVisitor::SDReq:
    case SSCOPVisitor::SDInd:
      _PDUtype = SSCOPconn::SSCOP_SD;
      break;

    case SSCOPVisitor::PollReq:
    case SSCOPVisitor::PollInd:
      _PDUtype = SSCOPconn::SSCOP_POLL;
      break;

    case SSCOPVisitor::StatReq:
    case SSCOPVisitor::StatInd:
      _PDUtype = SSCOPconn::SSCOP_STAT;
      break;

    case SSCOPVisitor::UstatReq:
    case SSCOPVisitor::UstatInd:
      _PDUtype = SSCOPconn::SSCOP_USTAT;
      break;

    case SSCOPVisitor::UDReq:
    case SSCOPVisitor::UDInd:
      _PDUtype = SSCOPconn::SSCOP_UD;
      break;

    case SSCOPVisitor::MDReq:
    case SSCOPVisitor::MDInd:
      _PDUtype = SSCOPconn::SSCOP_MD;
      break;

    default:
      _PDUtype = SSCOPconn::SSCOP_BAD;
    }
  //  SetLoggingOn();  
}

SSCOPVisitor::SSCOPVisitor(vistype& child_type, SSCOPVisitorType vt, u_char *sscop_uu,
			   int uulen) :   Visitor(child_type.derived_from(_my_type)),
  _vt(vt), _br(0), _sscop_uu(sscop_uu), _uulen(uulen), _PDUtype(SSCOPconn::SSCOP_BAD), _pdu(0),
  _len(0), _mr(-1), _ps(-1), _saved_ps(-1), _s(-1), _sbit(false), _r(-1), _sq(-1), 
  _list(0), _llen(0)
{
  switch(vt)
    {
    case SSCOPVisitor::BeginReq :
    case SSCOPVisitor::BeginInd :
      _PDUtype = SSCOPconn::SSCOP_BGN;
      break;

    case SSCOPVisitor::BeginAckReq :
    case SSCOPVisitor::BeginAckInd :
      _PDUtype = SSCOPconn::SSCOP_BGAK;
      break;

    case SSCOPVisitor::BeginRejReq :
    case SSCOPVisitor::BeginRejInd :
      _PDUtype = SSCOPconn::SSCOP_BGREJ;
      break;

    case SSCOPVisitor::EndReq:
    case SSCOPVisitor::EndInd:
      _PDUtype = SSCOPconn::SSCOP_END;
      break;

    case SSCOPVisitor::EndAckReq:
    case SSCOPVisitor::EndAckInd:
      _PDUtype = SSCOPconn::SSCOP_ENDAK;
      break;

    case SSCOPVisitor::ResyncReq:
    case SSCOPVisitor::ResyncInd:
      _PDUtype = SSCOPconn::SSCOP_RS;
      break;

    case SSCOPVisitor::ResyncAckReq:
    case SSCOPVisitor::ResyncAckInd:
      _PDUtype = SSCOPconn::SSCOP_RSAK;
      break;

    case SSCOPVisitor::RecovReq:
    case SSCOPVisitor::RecovInd:
      _PDUtype = SSCOPconn::SSCOP_ER;
      break;

    case SSCOPVisitor::RecovAckReq:
    case SSCOPVisitor::RecovAckInd:
      _PDUtype = SSCOPconn::SSCOP_ERAK;
      break;

    case SSCOPVisitor::SDReq:
    case SSCOPVisitor::SDInd:
      _PDUtype = SSCOPconn::SSCOP_SD;
      break;

    case SSCOPVisitor::PollReq:
    case SSCOPVisitor::PollInd:
      _PDUtype = SSCOPconn::SSCOP_POLL;
      break;

    case SSCOPVisitor::StatReq:
    case SSCOPVisitor::StatInd:
      _PDUtype = SSCOPconn::SSCOP_STAT;
      break;

    case SSCOPVisitor::UstatReq:
    case SSCOPVisitor::UstatInd:
      _PDUtype = SSCOPconn::SSCOP_USTAT;
      break;

    case SSCOPVisitor::UDReq:
    case SSCOPVisitor::UDInd:
      _PDUtype = SSCOPconn::SSCOP_UD;
      break;

    case SSCOPVisitor::MDReq:
    case SSCOPVisitor::MDInd:
      _PDUtype = SSCOPconn::SSCOP_MD;
      break;

    default:
      _PDUtype = SSCOPconn::SSCOP_BAD;
    }
  //  SetLoggingOn();
}

SSCOPVisitor::SSCOPVisitor(const SSCOPVisitor & v, int deep)
  : Visitor(v), _vt(v._vt), _br(v._br), _mr(v._mr), _ps(v._ps),
    _saved_ps(v._saved_ps), _s(v._s), _sbit(v._sbit), _r(v._r),
    _sq(v._sq), _uulen(0), _sscop_uu(0), _len(0), _pdu(0), _llen(0),
    _list(0)
{
  // So GT looks nice
  SetLast(0);

  if (deep == 1) {
    // make a deep Copy 
    if (v._sscop_uu && v._uulen > 0) {
      _uulen    = v._uulen; 
      _sscop_uu = new u_char[_uulen];
      assert(_sscop_uu);
      memcpy(_sscop_uu, v._sscop_uu, _uulen);
    }
    if (v._pdu && v._len > 0) {
      _PDUtype  = v._PDUtype;
      _len      = v._len;
      _pdu      = new u_char[_len];
      assert(_pdu);
      memcpy(_pdu, v._pdu, _len);
    }
    if (v._list && v._llen > 0) {
      _llen     = v._llen;
      _list     = new int[_llen];
      assert(_list);
      memcpy(_list, v._list, _llen);
    }
  }
  //  SetLoggingOn();  
}

SSCOPVisitor::~SSCOPVisitor() { }

const vistype & SSCOPVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType SSCOPVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }



inline u_char *SSCOPVisitor::TakePDU(void)
{
  u_char *pdu = new u_char[_len];
  memcpy(pdu,_pdu,_len);
  return pdu;
}


inline int SSCOPVisitor::TakeLen()
{
  return _len;
}

inline int SSCOPVisitor::GetElem1()
{
  assert(_list);
  return _list[0];
}

inline int SSCOPVisitor::GetElem2()
{
  assert(_list);
  return _list[1];
}

inline int *SSCOPVisitor::GetList()
{
  return _list;
}

inline int SSCOPVisitor::GetBR(){ return _br ;}
inline void SSCOPVisitor::SetBR(int br){_br = br;}
inline void SSCOPVisitor::SetS(int s){_s = s;}

inline  void SSCOPVisitor::SetPDU(u_char* pdu) {_pdu = pdu;}

inline int SSCOPVisitor::GetLen(){ return _len ;}
inline int SSCOPVisitor::GetR(){ return _r ;}
inline int SSCOPVisitor::GetMR(){ return _mr;}
inline int SSCOPVisitor::GetS(){ return _s;}
inline int SSCOPVisitor::GetPS(){ return _ps ;}
inline int SSCOPVisitor::GetSavedPS(){ return _saved_ps ;}
inline void SSCOPVisitor::StorePS(int ps){ _saved_ps = ps;}
inline int SSCOPVisitor::GetSQ(){ return _sq ;}

inline SSCOPconn::PDUtypes SSCOPVisitor::GetPDUType(){ return _PDUtype;}

inline void SSCOPVisitor::SetPDUType(SSCOPconn::PDUtypes pdu_type){_PDUtype = pdu_type;}


inline  SSCOPVisitor::SSCOPVisitorType SSCOPVisitor::GetVT() {return _vt;}

inline  void  SSCOPVisitor::SetVT(SSCOPVisitor::SSCOPVisitorType vt)
{_vt = vt;}



inline void SSCOPVisitor::TurnIntoReq(void)
{
  if (_vt < 0)
    _vt = (SSCOPVisitorType) (- _vt);
}

inline void SSCOPVisitor::TurnIntoInd(void)
{
  if (_vt > 0)
    _vt = (SSCOPVisitorType) (- _vt);
}

inline bool SSCOPVisitor::IsReq(void)
{
  if (_vt > 0)
    return true;
  return false;
}

inline bool SSCOPVisitor::IsInd(void)
{
  if (_vt < 0)
    return true;
  return false;
}

void SSCOPVisitor::ClearFields()
{
  // do not reset _ps (set from PollVisitor and used by StatVisitor)
  /*
  _mr = -1;
  _s  = -1;
  _r  = -1;
  _sq = -1;
  */
}

// ----------------------------------------------------------
BeginVisitor::BeginVisitor(SSCOPVisitorType vt, u_char *data, int len) : 
  SSCOPVisitor(_my_type,vt,data,len) { }

BeginVisitor::BeginVisitor(const BeginVisitor & rhs) :  SSCOPVisitor(rhs,(int )1) { }

BeginVisitor::~BeginVisitor() { }

void BeginVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"BeginVisitor Request");
  else
    strcpy(name,"BeginVisitor Indication");
}

Visitor * BeginVisitor::dup(void) const
{
  return new BeginVisitor(*this);
}

const vistype & BeginVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType BeginVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

bool BeginVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;

  if (GetVT() == BeginReq)
    rval = c->GetCS()->AA_EstablishReq(c, this);
  else
    rval = c->GetCS()->RecvBGN(c, this);
  return rval;
}

int  BeginVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  _len = len;
  u_int *data = (u_int *)(_pdu+_len-4);
  _mr  = ntohl(data[0]) & 0xffffff;
  _sq  = ntohl(data[-1]) & 0xff;
  return 0;
}


int  BeginVisitor::PackPDU()
{
  // possible sscop_uu
  int trailer_len = 8;
  int pad = (4-(_uulen & 0x3)) & 0x3;
  _len = _uulen + pad + trailer_len;
  _pdu = new u_char[_len];
  if(_sscop_uu)
    memcpy(_pdu,_sscop_uu,_uulen);
  u_int *ptr  = (u_int *)(_pdu+_uulen+pad);
  *ptr++ = _sq;
  *ptr++ = (SSCOPconn::SSCOP_BGN << 24) | (pad << 30) | htonl(_mr);
  return 0;
}

int  BeginVisitor::MapFields(SSCOPconn *c)
{
  // ascertain that only relevant (pdu fields) are conveyed in the visitor
  ClearFields();
  _sq = c->_vt_sq;
  _mr = c->_vr_mr;
  return 0;
}

SSCOPVisitor *BeginVisitor::Copy(void) { return new BeginVisitor(*this); }

// ----------------------------------------------------------
BeginAckVisitor::BeginAckVisitor(SSCOPVisitorType vt, u_char *sscop_uu, int len) : 
  SSCOPVisitor(_my_type,vt,sscop_uu,len) { }

BeginAckVisitor::BeginAckVisitor(const BeginAckVisitor & rhs) :  SSCOPVisitor(rhs,(int )1) { }

BeginAckVisitor::~BeginAckVisitor() { }

Visitor * BeginAckVisitor::dup(void) const
{
  return new BeginAckVisitor(*this);
}

const vistype & BeginAckVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType BeginAckVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

void BeginAckVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"BeginAckVisitor Request");
  else
    strcpy(name,"BeginAckVisitor Indication");
}

bool BeginAckVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;

  if (GetVT() == BeginAckReq)
    rval = c->GetCS()->AA_EstablishResp(c, this);
  else
    rval = c->GetCS()->RecvBGNAK(c, this);
  return rval;
}

int  BeginAckVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  _len = len;
  u_int *data = (u_int *)(_pdu+_len-4);
  _mr  = ntohl(data[0]) & 0xffffff;
  return 0;
}

int  BeginAckVisitor::PackPDU()
{
  // possible sscop_uu
  int trailer_len = 8;
  int pad = (4-(_uulen & 0x3)) & 0x3;
  _len = _uulen + pad + trailer_len;
  _pdu = new u_char[_len];
  if(_sscop_uu)
    memcpy(_pdu,_sscop_uu,_uulen);
  u_int *ptr  = (u_int *)(_pdu+_uulen+pad);
  *ptr++ = 0;
  *ptr++ = (SSCOPconn::SSCOP_BGAK << 24) | (pad << 30) | htonl(_mr);
  return 0;
}

int  BeginAckVisitor::MapFields(SSCOPconn *c)
{
  // ascertain that only relevant (pdu fields) are conveyed in the visitor
  ClearFields();
  _mr = c->_vr_mr;
  return 0;
}

SSCOPVisitor *BeginAckVisitor::Copy(void) { return new BeginAckVisitor(*this); }


// ----------------------------------------------------------
BeginRejVisitor::BeginRejVisitor(SSCOPVisitorType vt, u_char *data, int len) : 
  SSCOPVisitor(_my_type,vt,data,len) { }

BeginRejVisitor::BeginRejVisitor(const BeginRejVisitor & rhs) : SSCOPVisitor(rhs,(int )1) { }

BeginRejVisitor::~BeginRejVisitor() { }

Visitor * BeginRejVisitor::dup(void) const
{
  return new BeginRejVisitor(*this);
}

const vistype & BeginRejVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType BeginRejVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

void BeginRejVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"BeginRejVisitor Request");
  else
    strcpy(name,"BeginRejVisitor Indication");
}

bool BeginRejVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;
  rval = c->GetCS()->RecvBGNREJ(c, this);
  return rval;
}

int  BeginRejVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  _len = len;
  return 0;
}

int  BeginRejVisitor::PackPDU()
{
  // possible sscop_uu
  int trailer_len = 8;
  int pad = (4-(_uulen & 0x3)) & 0x3;
  _len = _uulen + pad + trailer_len;
  _pdu = new u_char[_len];
  u_int *ptr  = (u_int *)(_pdu+_uulen+pad);
  if(_sscop_uu)
    memcpy(_pdu,_sscop_uu,_uulen);
  *ptr++ = 0;
  *ptr++ = SSCOPconn::SSCOP_BGREJ << 24 | pad << 30;
  return 0;
}

int  BeginRejVisitor::MapFields(SSCOPconn *c)
{
  // ascertain that only relevant (pdu fields) are conveyed in the visitor
  ClearFields();
  return 0;
}

SSCOPVisitor *BeginRejVisitor::Copy(void) { return new BeginRejVisitor(*this); }

// ----------------------------------------------------------
EndVisitor::EndVisitor(SSCOPVisitorType vt, u_char *data, int len) : 
  SSCOPVisitor(_my_type,vt,data,len) { }

EndVisitor::EndVisitor(const EndVisitor & rhs) : SSCOPVisitor(rhs,(int )1) { }

EndVisitor::~EndVisitor() { }

Visitor * EndVisitor::dup(void) const
{
  return new EndVisitor(*this);
}

const vistype & EndVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType EndVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

void EndVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"EndVisitor Request");
  else
    strcpy(name,"EndVisitor Indication");
}

bool EndVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;

  if (GetVT() == EndReq)
    rval = c->GetCS()->AA_ReleaseReq(c, this);
  else
    rval = c->GetCS()->RecvEND(c, this);
  return rval;
}

int  EndVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  _len = len;
  return 0;
}

// to FIX to include the S bit
int  EndVisitor::PackPDU()
{
  // possible sscop_uu
  int trailer_len = 8;
  int pad = (4-(_uulen & 0x3)) & 0x3;
  _len = _uulen + pad + trailer_len;
  _pdu = new u_char[_len];
  u_int *ptr  = (u_int *)(_pdu+_uulen+pad);
  if(_sscop_uu)
    memcpy(_pdu,_sscop_uu,_uulen);
  *ptr++ = 0;
  // PL, R, S bit and Reserved
  if(_sbit)
    *ptr++ = (pad << 30) | 0x10000000  | (SSCOPconn::SSCOP_END << 24);
  else
    *ptr++ = (pad << 30) | (SSCOPconn::SSCOP_END << 24) ;
  return 0;
}

int  EndVisitor::MapFields(SSCOPconn *c)
{
  return 0;
}

SSCOPVisitor *EndVisitor::Copy(void) { return new EndVisitor(*this); }

// ----------------------------------------------------------
EndAckVisitor::EndAckVisitor(SSCOPVisitorType vt, u_char *data, int len) : 
  SSCOPVisitor(_my_type,vt,data,len) { }

EndAckVisitor::EndAckVisitor(const EndAckVisitor & rhs) : SSCOPVisitor(rhs,(int )1) { }

EndAckVisitor::~EndAckVisitor() { }

Visitor * EndAckVisitor::dup(void) const
{
  return new EndAckVisitor(*this);
}

const vistype & EndAckVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType EndAckVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

void EndAckVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"EndAckVisitor Request");
  else
    strcpy(name,"EndAckVisitor Indication");
}

bool EndAckVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;
  rval = c->GetCS()->RecvENDAK(c, this);
  return rval;
}

int EndAckVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  _len = len;
  return 0;
}

int EndAckVisitor::PackPDU()
{
  // no sscop_uu
  _len = 8;
  _pdu = new u_char[_len];
  u_int *ptr  = (u_int *)_pdu;
  int pad = (4-(_len & 0x3)) & 0x3;
  *ptr++ = 0;
  *ptr++ = SSCOPconn::SSCOP_ENDAK << 24 | pad << 30;
  return 0;
}

int  EndAckVisitor::MapFields(SSCOPconn *c)
{
  return 0;
}

SSCOPVisitor *EndAckVisitor::Copy(void) { return new EndAckVisitor(*this); }

// ----------------------------------------------------------
ResyncVisitor::ResyncVisitor(SSCOPVisitorType vt, u_char *data, int len) : 
  SSCOPVisitor(_my_type,vt,data,len) { }

ResyncVisitor::ResyncVisitor(const ResyncVisitor & rhs) : SSCOPVisitor(rhs,(int )1) { }

ResyncVisitor::~ResyncVisitor() { }

Visitor * ResyncVisitor::dup(void) const
{
  return new ResyncVisitor(*this);
}

const vistype & ResyncVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType ResyncVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

void ResyncVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"ResyncVisitor Request");
  else
    strcpy(name,"ResyncVisitor Indication");
}

bool ResyncVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;

  if (GetVT() == ResyncReq)
    rval = c->GetCS()->AA_ResyncReq(c, this);
  else
    rval = c->GetCS()->RecvRS(c, this);
  return rval;
}

int  ResyncVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  _len = len;
  u_int *data = (u_int *)(_pdu+_len-4);
  _mr  = ntohl(data[0]) & 0xffffff;
  _sq  = ntohl(data[-1]) & 0xff;
  return 0;
}

int  ResyncVisitor::PackPDU()
{
    // possible sscop_uu
  int trailer_len = 8;
  int pad = (4-(_uulen & 0x3)) & 0x3;
  _len = _uulen + pad + trailer_len;
  _pdu = new u_char[_len];
  if(_sscop_uu)
    memcpy(_pdu,_sscop_uu,_uulen);
  u_int *ptr  = (u_int *)(_pdu+_uulen+pad);
  *ptr++ = _sq;
  *ptr++ = (SSCOPconn::SSCOP_RS << 24) | (pad << 30) | htonl(_mr);
  return 0;
}

int  ResyncVisitor::MapFields(SSCOPconn *c)
{
  // ascertain that only relevant (pdu fields) are conveyed in the visitor
  ClearFields();
  _mr = c->_vr_mr;
  _sq = c->_vt_sq;
  return 0;
}

SSCOPVisitor *ResyncVisitor::Copy(void) { return new ResyncVisitor(*this); }

// ----------------------------------------------------------
ResyncAckVisitor::ResyncAckVisitor(SSCOPVisitorType vt, u_char *data, int len) : 
  SSCOPVisitor(_my_type,vt,data,len) { }

ResyncAckVisitor::ResyncAckVisitor(const ResyncAckVisitor & rhs) : SSCOPVisitor(rhs,(int )1) { }

ResyncAckVisitor::~ResyncAckVisitor() { }

Visitor * ResyncAckVisitor::dup(void) const
{
  return new ResyncAckVisitor(*this);
}

const vistype & ResyncAckVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType ResyncAckVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

void ResyncAckVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"ResyncAckVisitor Request");
  else
    strcpy(name,"ResyncAckVisitor Indication");
}

bool ResyncAckVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;
  rval = c->GetCS()->RecvRSAK(c, this);
  return rval;
}


int  ResyncAckVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  _len = len;
  u_int *data = (u_int *)(_pdu+_len-4);
  _mr  = ntohl(data[0]) & 0xffffff;
  return 0;
}

int  ResyncAckVisitor::PackPDU()
{
  // no sscop_uu
  _len = 8;
  _pdu = new u_char[_len];
  u_int *ptr  = (u_int *)_pdu;
  int pad = (4-(_len & 0x3)) & 0x3;
  *ptr++ = 0;
  *ptr++ = (SSCOPconn::SSCOP_RSAK << 24) | (pad << 30) | htonl(_mr);
  return 0;
}

int  ResyncAckVisitor::MapFields(SSCOPconn *c)
{
  // ascertain that only relevant (pdu fields) are conveyed in the visitor
  ClearFields();
  _mr = c->_vr_mr;
  return 0;
}

SSCOPVisitor *ResyncAckVisitor::Copy(void) { return new ResyncAckVisitor(*this); }

// ----------------------------------------------------------
SDVisitor::SDVisitor(SSCOPVisitorType vt, u_char *data, int len) : 
  SSCOPVisitor(_my_type,vt,data,len) { }

SDVisitor::SDVisitor(const SDVisitor & rhs) : SSCOPVisitor(rhs,(int )1) { }

SDVisitor::~SDVisitor() { }

Visitor * SDVisitor::dup(void) const
{
  return new SDVisitor(*this);
}

const vistype & SDVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType SDVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

void SDVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"SDVisitor Request");
  else
    strcpy(name,"SDVisitor Indication");
}

bool SDVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;

  if (GetVT() == SDReq)
    rval = c->GetCS()->AA_DATAReq(c, this); // SDReq
  else
    rval = c->GetCS()->RecvSD(c, this);
  return rval;
}


int  SDVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  u_int *data = (u_int *)(_pdu+len-4);
  int pad = ntohl(data[0]) >> 30;
  _len = len - pad - 4;
  _s  = ntohl(data[0]) & 0xffffff;
  return 0;
}

// pdu must be  32-bit aligned and we need 4 extra bytes at the end for seqnum
int  SDVisitor::PackPDU()
{
  // data is in sscop_uu
  int trailer_len = 4;
  int pad = (4-(_uulen & 0x3)) & 0x3;
  _len = _uulen + pad + trailer_len;
  _pdu = new u_char[_len];
  if(_sscop_uu)
    memcpy(_pdu,_sscop_uu,_uulen);
  u_int *ptr  = (u_int *)(_pdu+_uulen+pad);
  *ptr++ = (SSCOPconn::SSCOP_SD << 24) | (pad << 30) | htonl(_s);
  return 0;
}

int  SDVisitor::MapFields(SSCOPconn *c)
{
  // ascertain that only relevant (pdu fields) are conveyed in the visitor
  ClearFields();
  _s = c->_vt_s;
  return 0;
}

SSCOPVisitor * SDVisitor::Copy(void) { return new SDVisitor(*this); }

// ----------------------------------------------------------
RecovVisitor::RecovVisitor(SSCOPVisitorType vt, u_char *data, int len) : 
  SSCOPVisitor(_my_type,vt,data,len) { }


RecovVisitor::RecovVisitor(const RecovVisitor & rhs) : SSCOPVisitor(rhs,(int )1) { }

RecovVisitor::~RecovVisitor() { }

Visitor * RecovVisitor::dup(void) const
{
  return new RecovVisitor(*this);
}
const vistype & RecovVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType RecovVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

void RecovVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"RecovVisitor Request");
  else
    strcpy(name,"RecovVisitor Indication");
}

bool RecovVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;
  rval = c->GetCS()->RecvER(c, this);
  return rval;
}

int  RecovVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  _len = len;
  u_int *data = (u_int *)(_pdu+_len-4);
  _mr  = ntohl(data[0]) & 0xffffff;
  _sq  = ntohl(data[-1]) & 0xff;
  return 0;
}

int  RecovVisitor::PackPDU()
{
  // no sscop_uu
  _len = 8;
  _pdu = new u_char[_len];
  u_int *ptr  = (u_int *)_pdu;
  int pad = (4-(_len & 0x3)) & 0x3;
  *ptr++ = _sq;
  *ptr++ = (SSCOPconn::SSCOP_ER << 24) | (pad << 30) | htonl(_mr);
  return 0;
}

int  RecovVisitor::MapFields(SSCOPconn *c)
{
  // ascertain that only relevant (pdu fields) are conveyed in the visitor
  ClearFields();
  _mr = c->_vr_mr;
  _sq = c->_vt_sq;
  return 0;
}

SSCOPVisitor * RecovVisitor::Copy(void) { return new RecovVisitor(*this); }

// ----------------------------------------------------------
RecovAckVisitor::RecovAckVisitor(SSCOPVisitorType vt, u_char *data, int len) : 
  SSCOPVisitor(_my_type,vt,data,len) { }

RecovAckVisitor::RecovAckVisitor(const RecovAckVisitor & rhs) : SSCOPVisitor(rhs,(int )1) { }

RecovAckVisitor::~RecovAckVisitor() { }

Visitor * RecovAckVisitor::dup(void) const
{
  return new RecovAckVisitor(*this);
}

const vistype & RecovAckVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType RecovAckVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

void RecovAckVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"RecovAckVisitor Request");
  else
    strcpy(name,"RecovAckVisitor Indication");
}

bool RecovAckVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;
  if (GetVT() == RecovAckReq)
    rval = c->GetCS()->AA_RecoverResp(c, this);
  else
    rval = c->GetCS()->RecvERAK(c, this);
  return rval;
}

int  RecovAckVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  _len = len;
  u_int *data = (u_int *)(_pdu+_len-4);
  _mr  = ntohl(data[0]) & 0xffffff;
  return 0;
}

int  RecovAckVisitor::PackPDU()
{
  // no sscop_uu
  _len = 8;
  _pdu = new u_char[_len];
  u_int *ptr  = (u_int *)_pdu;
  int pad = (4-(_len & 0x3)) & 0x3;
  *ptr++ = 0;
  *ptr++ = (SSCOPconn::SSCOP_ERAK << 24) | (pad << 30) | htonl(_mr);
  return 0;
}

int  RecovAckVisitor::MapFields(SSCOPconn *c)
{
  // ascertain that only relevant (pdu fields) are conveyed in the visitor
  ClearFields();
  _mr = c->_vr_mr;
  return 0;
}

SSCOPVisitor * RecovAckVisitor::Copy(void) { return new RecovAckVisitor(*this); }

// ----------------------------------------------------------
PollVisitor::PollVisitor(SSCOPVisitorType vt, u_char *data, int len) : 
  SSCOPVisitor(_my_type,vt,data,len) { }

PollVisitor::PollVisitor(const PollVisitor & rhs) : SSCOPVisitor(rhs,(int )1) { }

PollVisitor::~PollVisitor() { }

Visitor * PollVisitor::dup(void) const
{
  return new PollVisitor(*this);
}

const vistype & PollVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType PollVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

void PollVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"PollVisitor Request");
  else
    strcpy(name,"PollVisitor Indication");
}

bool PollVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;
  rval = c->GetCS()->RecvPOLL(c, this);
  return rval;
}

int  PollVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  _len = len;
  u_int *data = (u_int *)(_pdu+_len-4);
  _s  = ntohl(data[0]) & 0xffffff;
  _ps  = ntohl(data[-1]) & 0xffffff;
  return 0;
}

int  PollVisitor::PackPDU()
{
  // no sscop_uu
  _len = 8;
  _pdu = new u_char[_len];
  u_int *ptr  = (u_int *)_pdu;
  int pad = (4-(_len & 0x3)) & 0x3;
  *ptr++ = htonl(_ps);
  *ptr++ = (SSCOPconn::SSCOP_POLL << 24) | (pad << 30) | htonl(_s);
  return 0;
}

int  PollVisitor::MapFields(SSCOPconn *c)
{
  // ascertain that only relevant (pdu fields) are conveyed in the visitor
  ClearFields();
  _s = c->_vt_s;
  _ps = c->_vt_ps;
  return 0;
}

SSCOPVisitor * PollVisitor::Copy(void) { return new PollVisitor(*this); }

// ----------------------------------------------------------
StatVisitor::StatVisitor(SSCOPVisitorType vt, u_char *data, int len) : 
  SSCOPVisitor(_my_type,vt,data,len) { }

StatVisitor::StatVisitor(const StatVisitor & rhs) : SSCOPVisitor(rhs,(int )1) { }

StatVisitor::~StatVisitor() { }

Visitor * StatVisitor::dup(void) const
{
  return new StatVisitor(*this);
}

const vistype & StatVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType StatVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

void StatVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"StatVisitor Request");
  else
    strcpy(name,"StatVisitor Indication");
}

bool StatVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;
  rval = c->GetCS()->RecvSTAT(c, this);
  return rval;
}

int  StatVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  _len = len;
  u_int *data = (u_int *)(_pdu+_len-4);
  _r  = ntohl(data[0]) & 0xffffff;
  _mr  = ntohl(data[-1]) & 0xffffff;
  _ps  = ntohl(data[-2]) & 0xffffff;
  // do the list now, we can have 0 or more list elements
  int pad = ntohl(data[0]) >> 30;
  // set _llen
  _llen = (_len - 12 - pad)/4;
  // allocate the list
  if(_llen)
    {
      _list = new int[_llen];
      data = (u_int *)_pdu;
      for(int i = 0; i < _llen; i++)
	_list[i] = _pdu[i];
    }
  else
    _list = 0;
  return 0;
}

int  StatVisitor::PackPDU()
{
  // no sscop_uu
  _len = sizeof(int)*(_llen+3);
  _pdu = new u_char[_len];
  u_int *ptr = (u_int *)_pdu;
  for(int i = 0; i < _llen; i++)
    *ptr++ = htonl(_list[i] & 0xffffff);
  *ptr++ = htonl(_ps);
  *ptr++ = htonl(_mr);
  *ptr++ = (SSCOPconn::SSCOP_STAT << 24) | htonl(_r);
  return 0;
}

int  StatVisitor::MapFields(SSCOPconn *c)
{
  // ascertain that only relevant (pdu fields) are conveyed in the visitor
  ClearFields();
  _mr = c->_vr_mr;
  _r = c->_vr_r;
  return 0;
}

SSCOPVisitor * StatVisitor::Copy(void) { return new StatVisitor(*this); }

// ----------------------------------------------------------
UStatVisitor::UStatVisitor(SSCOPVisitorType vt, u_char *data, int len) : 
  SSCOPVisitor(_my_type,vt,data,len) { }

UStatVisitor::UStatVisitor(const UStatVisitor & rhs) : SSCOPVisitor(rhs,(int )1) { }

UStatVisitor::~UStatVisitor() { }

Visitor * UStatVisitor::dup(void) const
{
  return new UStatVisitor(*this);
}

const vistype & UStatVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType UStatVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

void UStatVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"UStatVisitor Request");
  else
    strcpy(name,"UStatVisitor Indication");
}

bool UStatVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;
  rval = c->GetCS()->RecvUSTAT(c, this);
  return rval;
}

int  UStatVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  _len = len;
  u_int *data = (u_int *)(_pdu+_len-4);
  _r  = ntohl(data[0]) & 0xffffff;
  _mr  = ntohl(data[-1]) & 0xffffff;
  // do the list now, we only have 2 elements
  int pad = ntohl(data[0]) >> 30;
  // set _llen
  _llen = (_len - 12 - pad)/4;
  assert(_llen == 2);
  _list = new int[_llen];
  data = (u_int *)_pdu;
  _list[0] = _pdu[0];
  _list[1] = _pdu[1];
  return 0;
}

int  UStatVisitor::PackPDU()
{
  // no sscop_uu
  _len = 16;
  _pdu = new u_char[_len];
  u_int *ptr = (u_int *)_pdu;
  *ptr++ = htonl(_list[0] & 0xffffff);
  *ptr++ = htonl(_list[1] & 0xffffff);
  *ptr++ = htonl(_mr);
  *ptr++ = (SSCOPconn::SSCOP_USTAT << 24) | htonl(_r);
  return 0;
}

int  UStatVisitor::MapFields(SSCOPconn *c)
{
  // ascertain that only relevant (pdu fields) are conveyed in the visitor
  ClearFields();
  _mr = c->_vr_mr;
  _r = c->_vr_r;
  return 0;
}

SSCOPVisitor * UStatVisitor::Copy(void) { return new UStatVisitor(*this); }

// ----------------------------------------------------------
UDVisitor::UDVisitor(SSCOPVisitorType vt, u_char *data, int len) : 
  SSCOPVisitor(_my_type,vt,data,len) { }

UDVisitor::UDVisitor(const UDVisitor & rhs) : SSCOPVisitor(rhs,(int )1) { }

UDVisitor::~UDVisitor() { }

Visitor * UDVisitor::dup(void) const
{
  return new UDVisitor(*this);
}

const vistype & UDVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType UDVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

void UDVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"UDVisitor Request");
  else
    strcpy(name,"UDVisitor Indication");
}

bool UDVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;

  if (GetVT() == UDReq)
    rval = c->GetCS()->AA_UNITDATAReq(c, this);
  else
    rval = c->GetCS()->RecvUD(c, this);
  return rval;
}

int  UDVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  _len = len;
  u_int *data = (u_int *)(_pdu+_len-4);
  return 0;
}

int  UDVisitor::PackPDU()
{
  
  // data is in sscop_uu
  int trailer_len = 4;
  int pad = (4-(_uulen & 0x3)) & 0x3;
  _len = _uulen + pad + trailer_len;
  _pdu = new u_char[_len];
  if(_sscop_uu)
    memcpy(_pdu,_sscop_uu,_uulen);
  u_int *ptr  = (u_int *)(_pdu+_uulen+pad);
  *ptr++ = (SSCOPconn::SSCOP_UD << 24) | (pad << 30);
  return 0;
}

int  UDVisitor::MapFields(SSCOPconn *c)
{
  return 0;
}

SSCOPVisitor * UDVisitor::Copy(void) { return new UDVisitor(*this); }

// ----------------------------------------------------------
MDVisitor::MDVisitor(SSCOPVisitorType vt, u_char *data, int len) : 
  SSCOPVisitor(_my_type,vt,data,len) { }

MDVisitor::MDVisitor(const MDVisitor & rhs) : SSCOPVisitor(rhs,(int )1) { }

MDVisitor::~MDVisitor() { }

Visitor * MDVisitor::dup(void) const
{
  return new MDVisitor(*this);
}

const vistype & MDVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType MDVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

void MDVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"MDVisitor Request");
  else
    strcpy(name,"MDVisitor Indication");
}

bool MDVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;

  if (GetVT() == MDReq)
    rval = c->GetCS()->MAA_UNITDATAReq(c, this);
  else
    rval = c->GetCS()->RecvMD(c, this);
  return rval;
}

int  MDVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  _len = len;
  u_int *data = (u_int *)(_pdu+_len-4);
  return 0;
}

int  MDVisitor::PackPDU()
{
    // data is in sscop_uu
  int trailer_len = 4;
  int pad = (4-(_uulen & 0x3)) & 0x3;
  _len = _uulen + pad + trailer_len;
  _pdu = new u_char[_len];
  if(_sscop_uu)
    memcpy(_pdu,_sscop_uu,_uulen);
  u_int *ptr  = (u_int *)(_pdu+_uulen+pad);
  *ptr++ = (SSCOPconn::SSCOP_MD << 24) | (pad << 30);
  return 0;
}

int  MDVisitor::MapFields(SSCOPconn *c)
{
  return 0;
}

SSCOPVisitor * MDVisitor::Copy(void) { return new MDVisitor(*this); }


// ----------------------------------------------------------
RetrieveVisitor::RetrieveVisitor(SSCOPVisitorType vt, u_char *data, int len) : 
  SSCOPVisitor(_my_type,vt,data,len) { }

RetrieveVisitor::RetrieveVisitor(const RetrieveVisitor & rhs) : SSCOPVisitor(rhs,(int )1) { }

RetrieveVisitor::~RetrieveVisitor() { }

Visitor * RetrieveVisitor::dup(void) const
{
  return new RetrieveVisitor(*this);
}

const vistype & RetrieveVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType RetrieveVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

void RetrieveVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"RetrieveVisitor Request");
  else
    strcpy(name,"RetrieveVisitor Indication");
}

bool RetrieveVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;
  rval = c->GetCS()->AA_RetrieveReq(c, this);
  return rval;
}

int  RetrieveVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  _len = len;
  u_int *data = (u_int *)(_pdu+_len-4);
  return 0;
}

int  RetrieveVisitor::PackPDU()
{
  return -1;
}

int  RetrieveVisitor::MapFields(SSCOPconn *c)
{
  return -1;
}

SSCOPVisitor * RetrieveVisitor::Copy(void) { return new RetrieveVisitor(*this); }

// ----------------------------------------------------------
MAAErrorVisitor::MAAErrorVisitor(SSCOPVisitorType vt, u_char *data, int len) : 
  SSCOPVisitor(_my_type,vt,data,len) { }

MAAErrorVisitor::MAAErrorVisitor(const MAAErrorVisitor & rhs) : SSCOPVisitor(rhs,(int )1) { }

MAAErrorVisitor::~MAAErrorVisitor() { }

Visitor * MAAErrorVisitor::dup(void) const
{
  return new MAAErrorVisitor(*this);
}

const vistype & MAAErrorVisitor::GetClassType(void) const
{  return _my_type;  }

const VisitorType MAAErrorVisitor::GetType(void) const 
{ return VisitorType(GetClassType()); }

void MAAErrorVisitor::Name(char *name)
{
  if(IsReq())
    strcpy(name,"MAAErrorVisitor Request");
  else
    strcpy(name,"MAAErrorVisitor Indication");
}

bool MAAErrorVisitor::Protocol(SSCOPconn *c)
{
  bool rval = false;
  rval = c->GetCS()->RecvMAAError(c, this);
  return rval;
}

int  MAAErrorVisitor::UnPackPDU(u_char *pdu, int len)
{
  _pdu = pdu;
  _len = len;
  return 0;
}

int  MAAErrorVisitor::PackPDU()
{
  return -1;
}

int  MAAErrorVisitor::MapFields(SSCOPconn *c)
{
  return -1;
}

SSCOPVisitor * MAAErrorVisitor::Copy(void) { return new MAAErrorVisitor(*this); }


int compare(SSCOPVisitor * const &lhs, SSCOPVisitor * const &rhs)
{
  if(lhs->GetS() < rhs->GetS())
    return -1;
  if(lhs->GetS() > rhs->GetS())
    return 1;
  return 0;
}






