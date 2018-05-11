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
#ifndef _SSCOPVISITOR_H__
#define _SSCOPVISITOR_H__

#ifndef LINT
static char const _sscop_visitors_h_rcsid_[] =
"$Id: sscop_visitors.h,v 1.2 1998/08/21 15:16:59 mountcas Exp $";
#endif

#include <FW/basics/Visitor.h>
#include <FW/kernel/Kernel.h>
#include <FW/actors/State.h>
#include "sscop.h"

#define SSCOP_VISITOR_NAME "SSCOPVisitor"
#define SSCOP_BEGINVISITOR_NAME "BeginVisitor"
#define SSCOP_BEGINACKVISITOR_NAME "BeginAckVisitor"
#define SSCOP_BEGINREJVISITOR_NAME "BeginRejVisitor"
#define SSCOP_ENDVISITOR_NAME "EndVisitor"
#define SSCOP_ENDACKVISITOR_NAME "EndAckVisitor"
#define SSCOP_RESYNCVISITOR_NAME "ResyncVisitor"
#define SSCOP_RESYNCACKNVISITOR_NAME "ResyncAckVisitor"
#define SSCOP_RECOVVISITOR_NAME "RecovVisitor"
#define SSCOP_RECOVACKVISITOR_NAME "RecovAckVisitor"
#define SSCOP_SDVISITOR_NAME "SDVisitor"
#define SSCOP_POLLVISITOR_NAME "PollVisitor"
#define SSCOP_STATVISITOR_NAME "StatVisitor"
#define SSCOP_USTATVISITOR_NAME "UStatVisitor"
#define SSCOP_UDVISITOR_NAME "UDVisitor"
#define SSCOP_MDVISITOR_NAME "MDVisitor"
#define SSCOP_RETRIEVEVISITOR_NAME "RetrieveVisitor"
#define SSCOP_MAAERRORVISITOR_NAME "MAAErrorVisitor"

/*
 *
 *
 *
 */
class SSCOPVisitor : public Visitor {
  friend class SSCOPconn;
  friend int compare(SSCOPVisitor * const &, SSCOPVisitor * const &);
public:

  enum SSCOPVisitorType {
    Unknown      = 0,
    BeginReq     = 1, // same as AA-EstablishReq
    BeginAckReq  = 2, // same as AA-EstablishConf
    EndReq       = 3, // same as AA-ReleaseReq
    EndAckReq    = 4,
    ResyncReq    = 5, // same as AA-Resynch
    ResyncAckReq = 6,
    BeginRejReq  = 7,
    SDReq        = 8, // same as AA-DATA-REQ
    RecovReq     = 9,
    PollReq      = 10,
    StatReq      = 11,
    UstatReq     = 12,
    UDReq        = 13, // AA-UNITDATA-Req
    MDReq        = 14, // MAA-UNITDATA-Req
    RecovAckReq  = 15,
    RetrieveReq  = 16, // AA-RETRIEVE-Req
    BeginInd     = -1,
    BeginAckInd  = -2,
    EndInd       = -3,
    EndAckInd    = -4,
    ResyncInd    = -5,
    ResyncAckInd = -6,
    BeginRejInd  = -7,
    SDInd        = -8,
    RecovInd     = -9,
    PollInd      = -10,
    StatInd      = -11,
    UstatInd     = -12,
    UDInd        = -13,
    MDInd        = -14,
    RecovAckInd  = -15,
    MAAErrorInd  = -17
  };

  SSCOPVisitor();
  SSCOPVisitor(const SSCOPVisitor & v, int deep = 1);
  SSCOPVisitor(SSCOPVisitorType vt, u_char *sdu, int len);
  virtual ~SSCOPVisitor();
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  SSCOPVisitorType GetVT();
  void SetVT(SSCOPVisitorType t);
  void ClearFields();

  virtual bool Protocol(SSCOPconn *c) = 0;
  virtual int UnPackPDU(u_char *pdu, int len) = 0;
  virtual int PackPDU() = 0;
  virtual int MapFields(SSCOPconn *c) = 0;
  virtual SSCOPVisitor * Copy(void) = 0;
  virtual void Name(char *name) = 0;
  
  int GetBR(); 
  void SetBR(int br); 
  u_char *TakePDU();
  int TakeLen();
  int GetElem1(); // USTAT
  int GetElem2(); // USTAT
  int *GetList(); // STAT
  void SetPDU(u_char *pdu);
  void SetS(int s);
  int GetLen();
  int GetR(); // STAT & USTAT
  int GetMR();
  int GetS();
  int GetPS(); // POLL & STAT
  int GetSavedPS();
  void StorePS(int ps);
  int GetSQ();
  SSCOPconn::PDUtypes GetPDUType();
  void SetPDUType(SSCOPconn::PDUtypes pdu_type);
  void TurnIntoReq(void);
  void TurnIntoInd(void);
  bool IsReq(void);
  bool IsInd(void);

protected:

  // family requirements
  SSCOPVisitor(vistype & child_type, SSCOPVisitorType vt, u_char *sscop_uu, int len);
  static vistype _my_type;
  // this visitor requirements
  SSCOPVisitorType _vt;
  int _br;
  // user info
  u_char *_sscop_uu;
  int _uulen;
  // sscop pdu info needed to encode a real SSCOP PDU
  SSCOPconn::PDUtypes _PDUtype;
  // we keep aroud the SSCOP PDU received from peer here
  // even after being decoded
  u_char *_pdu;
  // You might want to clearly label what these variables are since the variable names don't make sense
  int _len;
  int _mr;
  int _ps;
  int _saved_ps;
  int _s;
  bool _sbit;
  int _r;
  int _sq;
  // list info: 2 elements for USTATs and 0 or more for STATs
  int *_list;
  int _llen;
};


// need the type in the constructor to distinguish between Req and Ind
class BeginVisitor : public SSCOPVisitor {
  public:
  BeginVisitor(SSCOPVisitorType vt, u_char *data, int len);
  BeginVisitor(const BeginVisitor & rhs);
  void Name(char *name);
  virtual ~BeginVisitor();

  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual SSCOPVisitor * Copy(void);
protected:

  virtual Visitor * dup(void) const;

private:
  static vistype _my_type;
};

class BeginAckVisitor : public SSCOPVisitor {
  public:
  BeginAckVisitor(SSCOPVisitorType vt, u_char *data, int len);
  BeginAckVisitor(const BeginAckVisitor & rhs);
  virtual ~BeginAckVisitor();
  void Name(char *name);
  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual SSCOPVisitor * Copy(void);
protected:

  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};

class BeginRejVisitor : public SSCOPVisitor {
  public:
  BeginRejVisitor(SSCOPVisitorType vt, u_char *data, int len);
  BeginRejVisitor(const BeginRejVisitor & rhs);
  virtual ~BeginRejVisitor();
  void Name(char *name);
  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual SSCOPVisitor * Copy(void);
protected:

  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};


class EndVisitor : public SSCOPVisitor {
  public:
  EndVisitor(SSCOPVisitorType vt, u_char *data, int len);
  EndVisitor(const EndVisitor & rhs);
  virtual ~EndVisitor();
  void Name(char *name);
  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual SSCOPVisitor * Copy(void);
protected:

  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};

class EndAckVisitor : public SSCOPVisitor {
  public:
  EndAckVisitor(SSCOPVisitorType vt, u_char *data, int len);
  EndAckVisitor(const EndAckVisitor & rhs);
  virtual ~EndAckVisitor();
  void Name(char *name);
  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual SSCOPVisitor * Copy(void);
protected:

  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};

class ResyncVisitor : public SSCOPVisitor {
  public:
  ResyncVisitor(SSCOPVisitorType vt, u_char *data, int len);
  ResyncVisitor(const ResyncVisitor & rhs);
  virtual ~ResyncVisitor();
  void Name(char *name);
  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual SSCOPVisitor * Copy(void);
protected:

  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};

class ResyncAckVisitor : public SSCOPVisitor {
  public:
  ResyncAckVisitor(SSCOPVisitorType vt, u_char *data, int len);
  ResyncAckVisitor(const ResyncAckVisitor & rhs);
  virtual ~ResyncAckVisitor();
  void Name(char *name);
  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual SSCOPVisitor * Copy(void);
protected:

  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};


class RecovVisitor : public SSCOPVisitor {
  public:
  RecovVisitor(SSCOPVisitorType vt, u_char *data, int len);
  RecovVisitor(const RecovVisitor & rhs);
  virtual ~RecovVisitor();
  void Name(char *name);
  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual SSCOPVisitor * Copy(void);
protected:

  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};

class RecovAckVisitor : public SSCOPVisitor {
  public:
  RecovAckVisitor(SSCOPVisitorType vt, u_char *data, int len);
  RecovAckVisitor(const RecovAckVisitor & rhs);
  virtual ~RecovAckVisitor();
  void Name(char *name);
  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual SSCOPVisitor * Copy(void);
protected:

  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};

class SDVisitor : public SSCOPVisitor {
  public:
  SDVisitor(SSCOPVisitorType vt, u_char *data, int len);
  SDVisitor(const SDVisitor & rhs);
  virtual ~SDVisitor();
  void Name(char *name);
  SSCOPVisitor *Copy(void);
  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
protected:

  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};

class PollVisitor : public SSCOPVisitor {
  public:
  PollVisitor(SSCOPVisitorType vt, u_char *data, int len);
  PollVisitor(const PollVisitor & rhs);
  virtual ~PollVisitor();
  void Name(char *name);
  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual SSCOPVisitor * Copy(void);
protected:

  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};

class StatVisitor : public SSCOPVisitor {
  public:
  StatVisitor(SSCOPVisitorType vt, u_char *data, int len);
  StatVisitor(const StatVisitor & rhs);
  virtual ~StatVisitor();
  void Name(char *name);
  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual SSCOPVisitor * Copy(void);
protected:

  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};

class UStatVisitor : public SSCOPVisitor {
  public:
  UStatVisitor(SSCOPVisitorType vt, u_char *data, int len);
  UStatVisitor(const UStatVisitor & rhs);
  virtual ~UStatVisitor();
  void Name(char *name);
  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual SSCOPVisitor * Copy(void);
protected:

  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};

class UDVisitor : public SSCOPVisitor {
  public:
  UDVisitor(SSCOPVisitorType vt, u_char *data, int len);
  UDVisitor(const UDVisitor & rhs);
  virtual ~UDVisitor();
  void Name(char *name);
  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual SSCOPVisitor * Copy(void);
protected:

  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};

class MDVisitor : public SSCOPVisitor {
  public:
  MDVisitor(SSCOPVisitorType vt, u_char *data, int len);
  MDVisitor(const MDVisitor & rhs);
  virtual ~MDVisitor();
  void Name(char *name);
  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual SSCOPVisitor * Copy(void);
protected:

  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};


class RetrieveVisitor : public SSCOPVisitor {
  public:
  RetrieveVisitor(SSCOPVisitorType vt, u_char *data, int len);
  RetrieveVisitor(const RetrieveVisitor & rhs);
  virtual ~RetrieveVisitor();
  void Name(char *name);
  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual SSCOPVisitor * Copy(void);
protected:

  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};


class MAAErrorVisitor : public SSCOPVisitor {
  public:
  MAAErrorVisitor(SSCOPVisitorType vt, u_char *data, int len);
  MAAErrorVisitor(const MAAErrorVisitor & rhs);
  virtual ~MAAErrorVisitor();
  void Name(char *name);
  bool Protocol(SSCOPconn *c);
  int UnPackPDU(u_char *pdu, int len);
  int PackPDU();
  int MapFields(SSCOPconn *c);
  virtual const VisitorType GetType(void) const;
  virtual const vistype & GetClassType(void) const;
  virtual SSCOPVisitor * Copy(void);
protected:

  virtual Visitor * dup(void) const;
private:
  static vistype _my_type;
};

#endif




