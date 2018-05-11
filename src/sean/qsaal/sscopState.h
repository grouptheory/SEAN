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

//-*-C++-*-

#ifndef SSCOPSTATE_H
#define SSCOPSTATE_H
// -*- C++ -*-
#ifndef LINT
static char const _sscopState_h_rcsid_[] =
"$Id: sscopState.h,v 1.1 1998/07/20 11:11:52 bilal Exp $";
#endif

class SSCOPVisitor;
class SSCOPconn;

class sscopState {
public:
  virtual SSCOPconn::sscop_states StateID() = 0;
  virtual void Name(char *name) = 0;
  int  GetVTCC(SSCOPconn *c);
  void SetVTCC(SSCOPconn *c, int vtcc);
  void Inc8VTSQ(SSCOPconn *c);

  // * in SDL charts page 47
  virtual int AA_UNITDATAReq(SSCOPconn *c, SSCOPVisitor *v);
  virtual int MAA_UNITDATAReq(SSCOPconn *c, SSCOPVisitor *v);
  virtual int RecvUD(SSCOPconn *c, SSCOPVisitor *v);
  virtual int RecvMD(SSCOPconn *c, SSCOPVisitor *v);
  // invalid PDU event to FIX
  // UD queued up event to FIX
  // MD queued up event to FIX

  virtual int AA_EstablishReq(SSCOPconn *c, SSCOPVisitor *v); 
  virtual int AA_EstablishResp(SSCOPconn *c, SSCOPVisitor *v); 
  virtual int AA_RetrieveReq(SSCOPconn *c, SSCOPVisitor *v);
  virtual int AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v);
  virtual int AA_ResyncReq(SSCOPconn *c, SSCOPVisitor *v);
  virtual int AA_ResyncResp(SSCOPconn *c, SSCOPVisitor *v);
  virtual int AA_DATAReq(SSCOPconn *c, SSCOPVisitor *v);
  virtual int AA_RecoverResp(SSCOPconn *c, SSCOPVisitor *v);
  
  // PDU Received
  virtual int RecvBGN(SSCOPconn *c, SSCOPVisitor *v);
  virtual int RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v);
  virtual int RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v);
  virtual int RecvEND(SSCOPconn *c, SSCOPVisitor *v);
  virtual int RecvENDAK(SSCOPconn *c, SSCOPVisitor *v);
  virtual int RecvRS(SSCOPconn *c, SSCOPVisitor *v);
  virtual int RecvRSAK(SSCOPconn *c, SSCOPVisitor *v);
  virtual int RecvER(SSCOPconn *c, SSCOPVisitor *v);
  virtual int RecvERAK(SSCOPconn *c, SSCOPVisitor *v);
  virtual int RecvSD(SSCOPconn *c, SSCOPVisitor *v);
  virtual int RecvPOLL(SSCOPconn *c, SSCOPVisitor *v);
  virtual int RecvSTAT(SSCOPconn *c, SSCOPVisitor *v);
  virtual int RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v);
  virtual int RecvMAAError(SSCOPconn *c, SSCOPVisitor *v);

  // Timers expiry

  virtual void ExpTimerCC(SSCOPconn *c);
  virtual void ExpTimerPOLL(SSCOPconn *c);
  virtual void ExpTimerNORESP(SSCOPconn *c);
  virtual void ExpTimerKEEPALIVE(SSCOPconn *c);
  virtual void ExpTimerIDLE(SSCOPconn *c);
protected:
  void ChangeState(SSCOPconn *c, sscopState *s, char *StateName);
  void GetStateName(SSCOPconn *c, char *name);
};



class sscop_idle : public sscopState {
public:
  virtual ~sscop_idle();
  static sscopState* Instance();
  SSCOPconn::sscop_states StateID();
  void Name(char *name);

  // same order as in the SDL charts
  int AA_EstablishReq(SSCOPconn *c, SSCOPVisitor *v); 
  int RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGN(SSCOPconn *c, SSCOPVisitor *v);
  int RecvENDAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvEND(SSCOPconn *c, SSCOPVisitor *v);
  int RecvER(SSCOPconn *c, SSCOPVisitor *v);
  int RecvPOLL(SSCOPconn *c, SSCOPVisitor *v);
  int RecvSD(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvERAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvSTAT(SSCOPconn *c, SSCOPVisitor *v);
  int RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v);
  int RecvRS(SSCOPconn *c, SSCOPVisitor *v);
  int RecvRSAK(SSCOPconn *c, SSCOPVisitor *v);
  // use default AA_RetrieveReq
private:
  static sscopState* _instance;
  sscop_idle();
};


class sscop_outconn : public sscopState {
public:
  virtual ~sscop_outconn();
  static sscopState* Instance();
  SSCOPconn::sscop_states StateID();
  void Name(char *name);
  // same order as in the SDL charts
  int RecvENDAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvSD(SSCOPconn *c, SSCOPVisitor *v);
  int RecvERAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvEND(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v);
  void ExpTimerCC(SSCOPconn *c);
  int RecvSTAT(SSCOPconn *c, SSCOPVisitor *v);
  int RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v);
  int RecvPOLL(SSCOPconn *c, SSCOPVisitor *v);
  int AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v);
  int RecvER(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGN(SSCOPconn *c, SSCOPVisitor *v);
  int RecvRSAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvRS(SSCOPconn *c, SSCOPVisitor *v);
  // do not want to use default retrieve
  int AA_RetrieveReq(SSCOPconn *c, SSCOPVisitor *v);
private:
  static sscopState* _instance;
  sscop_outconn();
};

class sscop_inconn : public sscopState {
public:
  virtual ~sscop_inconn();
  static sscopState* Instance();
  SSCOPconn::sscop_states StateID();
  void Name(char *name);
  int AA_EstablishResp(SSCOPconn *c, SSCOPVisitor *v); 
  int AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGN(SSCOPconn *c, SSCOPVisitor *v);
  int RecvER(SSCOPconn *c, SSCOPVisitor *v);
  int RecvENDAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v);
  int RecvSD(SSCOPconn *c, SSCOPVisitor *v);
  int RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v);
  int RecvSTAT(SSCOPconn *c, SSCOPVisitor *v);
  int RecvPOLL(SSCOPconn *c, SSCOPVisitor *v);
  int RecvERAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvRSAK(SSCOPconn *c, SSCOPVisitor *v);
  // use default AA_RetrieveReq
  int RecvRS(SSCOPconn *c, SSCOPVisitor *v);
  int RecvEND(SSCOPconn *c, SSCOPVisitor *v);
private:
  static sscopState* _instance;
  sscop_inconn();
};

class sscop_outdisc : public sscopState {
public:
  virtual ~sscop_outdisc();
  static sscopState* Instance();
  SSCOPconn::sscop_states StateID();
  void Name(char *name);

  int AA_EstablishReq(SSCOPconn *c, SSCOPVisitor *v); 
  int RecvSD(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvPOLL(SSCOPconn *c, SSCOPVisitor *v);
  int RecvSTAT(SSCOPconn *c, SSCOPVisitor *v);
  int RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v);
  int RecvEND(SSCOPconn *c, SSCOPVisitor *v);
  int RecvENDAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v);
  void ExpTimerCC(SSCOPconn *c);
  int RecvERAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvRS(SSCOPconn *c, SSCOPVisitor *v);
  int RecvRSAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGN(SSCOPconn *c, SSCOPVisitor *v);
  int RecvER(SSCOPconn *c, SSCOPVisitor *v);
  // use default AA_RetrieveReq
private:
  static sscopState* _instance;
  sscop_outdisc();
};

class sscop_outres : public sscopState {
public:
  virtual ~sscop_outres();
  static sscopState* Instance();
  SSCOPconn::sscop_states StateID();
  void Name(char *name);

  int RecvER(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGN(SSCOPconn *c, SSCOPVisitor *v);
  int RecvPOLL(SSCOPconn *c, SSCOPVisitor *v);
  int RecvENDAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v);
  int RecvEND(SSCOPconn *c, SSCOPVisitor *v);
  void ExpTimerCC(SSCOPconn *c);
  int RecvSTAT(SSCOPconn *c, SSCOPVisitor *v);
  int RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v);
  int AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v);
  int RecvRS(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvERAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvSD(SSCOPconn *c, SSCOPVisitor *v);
  int RecvRSAK(SSCOPconn *c, SSCOPVisitor *v);
  // do not want to use default retrieve
  int AA_RetrieveReq(SSCOPconn *c, SSCOPVisitor *v);
private:
  static sscopState* _instance;
  sscop_outres();
};

class sscop_inres : public sscopState {
public:
  virtual ~sscop_inres();
  static sscopState* Instance();
  SSCOPconn::sscop_states StateID();
  void Name(char *name);
  
  int AA_ResyncResp(SSCOPconn *c, SSCOPVisitor *v);
  int AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v);
  int RecvENDAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v);
  int RecvEND(SSCOPconn *c, SSCOPVisitor *v);
  int RecvER(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGN(SSCOPconn *c, SSCOPVisitor *v);
  int RecvSD(SSCOPconn *c, SSCOPVisitor *v);
  int RecvPOLL(SSCOPconn *c, SSCOPVisitor *v);
  int RecvSTAT(SSCOPconn *c, SSCOPVisitor *v);
  int RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v);
  int RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvERAK(SSCOPconn *c, SSCOPVisitor *v);
  int RecvRS(SSCOPconn *c, SSCOPVisitor *v);
  int RecvRSAK(SSCOPconn *c, SSCOPVisitor *v);
  // use deafault AA_RetrieveReq
private:
  static sscopState* _instance;
  sscop_inres();
};

class sscop_outrec : public sscopState {
public:
  virtual ~sscop_outrec();
  static sscopState* Instance();
  SSCOPconn::sscop_states StateID();
  void Name(char *name);
  
   int AA_DATAReq(SSCOPconn *c, SSCOPVisitor *v);
   int RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v);
   int RecvERAK(SSCOPconn *c, SSCOPVisitor *v);
   int RecvEND(SSCOPconn *c, SSCOPVisitor *v);
   int RecvENDAK(SSCOPconn *c, SSCOPVisitor *v);
   int RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v);
   void ExpTimerCC(SSCOPconn *c);
   int RecvSTAT(SSCOPconn *c, SSCOPVisitor *v);
   int RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v);
   int RecvPOLL(SSCOPconn *c, SSCOPVisitor *v);
   int AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v);
   int AA_ResyncReq(SSCOPconn *c, SSCOPVisitor *v);
   int RecvSD(SSCOPconn *c, SSCOPVisitor *v);
   int RecvBGN(SSCOPconn *c, SSCOPVisitor *v);
   int RecvER(SSCOPconn *c, SSCOPVisitor *v);
   int RecvRSAK(SSCOPconn *c, SSCOPVisitor *v);
   int RecvRS(SSCOPconn *c, SSCOPVisitor *v);
  // do not want to use default retrieve
  int AA_RetrieveReq(SSCOPconn *c, SSCOPVisitor *v);
private:
  static sscopState* _instance;
  sscop_outrec();
};

class sscop_recresp : public sscopState {
public:
  virtual ~sscop_recresp();
  static sscopState* Instance();
  SSCOPconn::sscop_states StateID();
  void Name(char *name);
  
  // use default AA_RetrieveReq
   int RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v);
   int RecvERAK(SSCOPconn *c, SSCOPVisitor *v);
   int RecvEND(SSCOPconn *c, SSCOPVisitor *v);
   int RecvENDAK(SSCOPconn *c, SSCOPVisitor *v);
   int RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v);
   int AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v);
   int RecvRSAK(SSCOPconn *c, SSCOPVisitor *v);
   int RecvRS(SSCOPconn *c, SSCOPVisitor *v);
   int RecvSD(SSCOPconn *c, SSCOPVisitor *v);
   int RecvPOLL(SSCOPconn *c, SSCOPVisitor *v);
   int AA_RecoverResp(SSCOPconn *c, SSCOPVisitor *v);
   int AA_ResyncReq(SSCOPconn *c, SSCOPVisitor *v);
   int RecvER(SSCOPconn *c, SSCOPVisitor *v);
   int RecvBGN(SSCOPconn *c, SSCOPVisitor *v);
   int RecvSTAT(SSCOPconn *c, SSCOPVisitor *v);
   int RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v);
private:
  static sscopState* _instance;
  sscop_recresp();
};

class sscop_inrec : public sscopState {
public:
  virtual ~sscop_inrec();
  static sscopState* Instance();
  SSCOPconn::sscop_states StateID();
  void Name(char *name);

   int AA_RecoverResp(SSCOPconn *c, SSCOPVisitor *v);
   int AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v);
   int RecvEND(SSCOPconn *c, SSCOPVisitor *v);
   int AA_ResyncReq(SSCOPconn *c, SSCOPVisitor *v);
   int RecvENDAK(SSCOPconn *c, SSCOPVisitor *v);
   int RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v);
   int RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v);
   int RecvSTAT(SSCOPconn *c, SSCOPVisitor *v);
   int RecvPOLL(SSCOPconn *c, SSCOPVisitor *v);
   int RecvSD(SSCOPconn *c, SSCOPVisitor *v);
   int RecvRSAK(SSCOPconn *c, SSCOPVisitor *v);
  // use default AA_RetrieveReq
   int RecvRS(SSCOPconn *c, SSCOPVisitor *v);
   int RecvER(SSCOPconn *c, SSCOPVisitor *v);
   int RecvBGN(SSCOPconn *c, SSCOPVisitor *v);
   int RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v);
   int RecvERAK(SSCOPconn *c, SSCOPVisitor *v);
private:
  static sscopState* _instance;
  sscop_inrec();
};


class sscop_ready : public sscopState {
public:
  virtual ~sscop_ready();
  static sscopState* Instance();
  SSCOPconn::sscop_states StateID();
  void Name(char *name);

   int AA_ResyncReq(SSCOPconn *c, SSCOPVisitor *v);
   int AA_ReleaseReq(SSCOPconn *c, SSCOPVisitor *v);
   int RecvBGNAK(SSCOPconn *c, SSCOPVisitor *v);
   int RecvERAK(SSCOPconn *c, SSCOPVisitor *v);
   int RecvER(SSCOPconn *c, SSCOPVisitor *v);
   int RecvBGN(SSCOPconn *c, SSCOPVisitor *v);
   int RecvENDAK(SSCOPconn *c, SSCOPVisitor *v);
   int RecvBGNREJ(SSCOPconn *c, SSCOPVisitor *v);
   int RecvRSAK(SSCOPconn *c, SSCOPVisitor *v);
   int RecvRS(SSCOPconn *c, SSCOPVisitor *v);
   int RecvEND(SSCOPconn *c, SSCOPVisitor *v);
   void ExpTimerPOLL(SSCOPconn *c);
   void ExpTimerKEEPALIVE(SSCOPconn *c);
   void ExpTimerIDLE(SSCOPconn *c);
   void ExpTimerNORESP(SSCOPconn *c);
   int AA_DATAReq(SSCOPconn *c, SSCOPVisitor *v);
   int RecvSD(SSCOPconn *c, SSCOPVisitor *v);
   int RecvPOLL(SSCOPconn *c, SSCOPVisitor *v);
   int RecvUSTAT(SSCOPconn *c, SSCOPVisitor *v);
   int RecvSTAT(SSCOPconn *c, SSCOPVisitor *v);
  // do not want to use default retrieve
  int AA_RetrieveReq(SSCOPconn *c, SSCOPVisitor *v);
private:
  static sscopState* _instance;
  sscop_ready();
};

#endif








