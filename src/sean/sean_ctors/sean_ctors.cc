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
static char const _ctors_fsm_cc_rcsid_[] =
"$Id: sean_ctors.cc,v 1.5 1998/08/13 05:55:01 bilal Exp $";
#endif
#include <common/cprototypes.h>


#include <FW/basics/Visitor.h>
#include <FW/basics/VisitorType.h>
#include <sean/ipc/IPC_Visitors.h>


vistype IPC_Visitor::_my_type(IPC_VISITOR_NAME);


vistype IPC_SignallingVisitor::_my_type(IPC_SIGNALLING_VISITOR_NAME);
vistype IPC_DataVisitor::_my_type(IPC_DATA_VISITOR_NAME);

extern "C" {
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>       
#include <arpa/inet.h>

int close(int fd);
};


#include <sean/ipc/tcp_ipc.h>
struct protoent * tcp_listener::_ppe = getprotobyname("tcp");

#include <sean/daemon/SwitchFabric.h>
vistype ConnectorVisitor::_my_type(CONNECTOR_VISITOR_NAME);

#include <sean/daemon/ATM_PDU_Visitor.h>
vistype ATM_PDU_Visitor::_my_type(ATM_PDU_VISITOR_NAME);

//--------------- sean_Visitor ---------------
#include <sean/cc/sean_Visitor.h>
vistype sean_Visitor::_my_type(SEAN_VISITOR_NAME);

//--------------- SSCFVisitor ---------------
#include <sean/uni_fsm/SSCFVisitor.h>
vistype sscfVisitor::_my_type(SSCF_VISITOR_NAME);

// -------------------------------------------------------------
#include <codec/q93b_msg/Buffer.h>
#include <sean/xti/sean_io.h>
Buffer* sean_io::_tmp = new Buffer(DEFAULT_MAX_PDU_SIZE);

//--------------- SSCFVisitor ---------------
#include <sean/uni_fsm/Rebind_Visitor.h>
vistype Rebind_Visitor::_my_type(REBIND_VISITOR_NAME);

#include <sean/qsaal/sscop_visitors.h>
vistype   StatVisitor::_my_type(SSCOP_STATVISITOR_NAME);   
vistype   PollVisitor::_my_type(SSCOP_POLLVISITOR_NAME);   
vistype   MAAErrorVisitor::_my_type(SSCOP_MAAERRORVISITOR_NAME);   
vistype   RetrieveVisitor::_my_type(SSCOP_RETRIEVEVISITOR_NAME);   
vistype   RecovAckVisitor::_my_type(SSCOP_RECOVACKVISITOR_NAME);   
vistype   BeginRejVisitor::_my_type(SSCOP_BEGINREJVISITOR_NAME);   
vistype   BeginAckVisitor::_my_type(SSCOP_BEGINACKVISITOR_NAME);   
vistype   MDVisitor::_my_type(SSCOP_MDVISITOR_NAME);  
vistype   UDVisitor::_my_type(SSCOP_UDVISITOR_NAME);  
vistype   SDVisitor::_my_type(SSCOP_SDVISITOR_NAME);  
vistype   UStatVisitor::_my_type(SSCOP_USTATVISITOR_NAME);   
vistype   RecovVisitor::_my_type(SSCOP_RECOVVISITOR_NAME);   
vistype   BeginVisitor::_my_type(SSCOP_BEGINVISITOR_NAME);   
vistype   SSCOPVisitor::_my_type(SSCOP_VISITOR_NAME);   
vistype   ResyncAckVisitor::_my_type(SSCOP_RESYNCACKNVISITOR_NAME);   
vistype   ResyncVisitor::_my_type(SSCOP_RESYNCVISITOR_NAME);   
vistype   EndAckVisitor::_my_type(SSCOP_ENDACKVISITOR_NAME);   
vistype   EndVisitor::_my_type(SSCOP_ENDVISITOR_NAME);   

#include <sean/ipc/async.h>
asynchronous_manager* asynchronous_manager::_singleton = AsynchronousManager();

// -------------------------------------------------------------
extern bool FW_ctors_inited;
extern bool init_ctors_FW(void);

extern bool init_ctors_sean(void);

bool sean_ctors_inited = init_ctors_sean();

bool init_ctors_sean(void) 
{
  if (!FW_ctors_inited) 
    FW_ctors_inited = init_ctors_FW();

  if (sean_ctors_inited)
    return sean_ctors_inited;

  return true;
}


