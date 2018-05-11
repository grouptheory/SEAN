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
static char const _StreamFilter_cc_rcsid_[] =
"$Id: Streams.cc,v 1.3 1998/08/06 04:04:09 bilal Exp $";
#endif
#include <common/cprototypes.h>


#include "Streams.h"
#include "ATM_PDU_Visitor.h"
#include <sean/cc/sean_Visitor.h>
#include <sean/cc/call_control.h>
#include <sean/ipc/IPC_Visitors.h>
#include <codec/q93b_msg/Buffer.h>

const VisitorType * StackSelector::_sean_vistype = 0;
const VisitorType * StackSelector::_data_vistype = 0;
const VisitorType * StackSelector::_ipc_sig_vistype = 0;
const VisitorType * StackSelector::_ipc_dat_vistype = 0;

const VisitorType * StreamFilter::_ipc_dat_vistype = 0;
const VisitorType * StreamFilter::_ipc_sig_vistype = 0;
const VisitorType * StreamFilter::_data_vistype = 0;

//----------------------------------------------------------
StreamFilter::StreamFilter(void) : _cc(0)
{ 
  if (!_ipc_dat_vistype)
    _ipc_dat_vistype = QueryRegistry(IPC_DATA_VISITOR_NAME);
  if (!_ipc_sig_vistype)
    _ipc_sig_vistype = QueryRegistry(IPC_SIGNALLING_VISITOR_NAME);
  if (!_data_vistype)
    _data_vistype = QueryRegistry(ATM_PDU_VISITOR_NAME);
}

//----------------------------------------------------------
void StreamFilter::Inform_Of_Call_Control(call_control* cc) {
  _cc=cc;
}

//----------------------------------------------------------
StreamFilter::~StreamFilter() { }

//----------------------------------------------------------
State * StreamFilter::Handle(Visitor * v)
{
  if (v->GetType().Is_A(_ipc_sig_vistype))  // signalling
    PassThru(v);
  else if (v->GetType().Is_A(_ipc_dat_vistype)) {  // ipc data
    IPC_DataVisitor * dv = (IPC_DataVisitor *)v;

    if (VisitorFrom(v) == Visitor::A_SIDE) {  // ipc going down

      Visitor * new_vis = 0;
      Buffer * buf = dv->take_packet_and_zero_length();
      // Obtain the op code to see if it's signalling or data
      sean_Visitor::sean_Visitor_Type op = sean_Visitor::unknown;
      unsigned char * buffer = buf->data();
      bcopy((const void *)(buffer), (void *)(&op), sizeof(int)); 

      if (op != sean_Visitor::raw_simulation_pdu) 
	{  // -----make a sean_Visitor-----
	  new_vis = new sean_Visitor(buf);
	  buf->ShrinkHeader( 4 * sizeof(int) );
	  ((sean_Visitor *)new_vis)->
	    set_shared_app_id( dv->share_local_id() );
	} 
      else 
	{  // -----make an ATM_PDU_Visitor-----
	  int vpi, vci;
	  // opcode is bytes 0-3, cid is bytes 4-7, vpi is bytes 8-11, vci is bytes 12-15

	  bcopy((const void *)(buffer + 2 * sizeof(int)), 
		(void *)(&vpi), sizeof(int)); 
	  bcopy((const void *)(buffer + 3 * sizeof(int)), 
		(void *)(&vci), sizeof(int)); 
	  buf->ShrinkHeader( 4 * sizeof(int) );
	  new_vis = new ATM_PDU_Visitor(vpi, vci, buf);
	}

      // Kill the original IPC Data Visitor
      dv->Suicide();

      // Pass the sean_Visitor or ATM_PDU_Visitor to B
      PassVisitorToB(new_vis);
    }
    else  // ipc going up
      {
	PassVisitorToA(v);
      }
  }
  else if (v->GetType().Is_A(_data_vistype)) { // simulated data path

    assert (VisitorFrom(v) == Visitor::B_SIDE);

    ATM_PDU_Visitor* atv = (ATM_PDU_Visitor*)v;
    int vp = atv->GetVPI();
    int vc = atv->GetVCI();
    int cid = -1;
    abstract_local_id* locid = 0;
    assert(_cc);

    // ask the call control to identify the application
    // that is bound to this vpi,vci

    if (_cc->streamfilter_data_query(vp,vc,cid,locid)) {

      assert ((cid != -1) && (locid != 0));
      // Then, we must turn the ATM_PDU into an IPC_DataVisitor

      Buffer* buf = atv->TakeBuffer();
      int len = buf->length();
      buf->GrowHeader(16);

      sean_Visitor::sean_Visitor_Type op = sean_Visitor::raw_simulation_pdu;

      unsigned char* data = buf->data();

      int index=0;
      // first four bytes of data are the op code
      bcopy((const void *)(&op), (void *)(data + index * sizeof(int)), sizeof(int)); 
      index++; 
      // next four bytes of data (header) are the cid
      bcopy((const void *)(&cid), (void *)(data + index * sizeof(int)), sizeof(int));
      index++; 
      // next four bytes of data are the vpi
      bcopy((const void *)(&vp), (void *)(data + index * sizeof(int)), sizeof(int)); 
      index++; 
      // following four bytes of data are the vci
      bcopy((const void *)(&vc), (void *)(data + index * sizeof(int)), sizeof(int)); 

      buf->set_length(len+16);

      IPC_DataVisitor* idv = new IPC_DataVisitor(locid,buf);
      atv->Suicide();
      PassVisitorToA(idv);
    }
    else {
      // No way to deliver this PDU
      v->Suicide();
    }
  }
  else
    PassThru(v);

  return this;
}


//----------------------------------------------------------
void StreamFilter::Interrupt(SimEvent * se) { }



//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
StackSelector::StackSelector(void) : _c(0)
{
  if (!_sean_vistype)
    _sean_vistype = QueryRegistry(SEAN_VISITOR_NAME);
  if (!_data_vistype)
    _data_vistype = QueryRegistry(ATM_PDU_VISITOR_NAME);
  if (!_ipc_sig_vistype)
    _ipc_sig_vistype = QueryRegistry(IPC_SIGNALLING_VISITOR_NAME);
  if (!_ipc_dat_vistype)
    _ipc_dat_vistype = QueryRegistry(IPC_DATA_VISITOR_NAME);
}

//----------------------------------------------------------
StackSelector::~StackSelector() { }

//----------------------------------------------------------
// Find destination for this visitor.
Conduit * StackSelector::GetNextConduit(Visitor * v)
{
  // data is the default

  Conduit * rval = 0;
  if (v->GetType().Is_A(_ipc_dat_vistype))
    rval = 0;
  else
    if (v->GetType().Is_A(_sean_vistype) ||
	v->GetType().Is_A(_ipc_sig_vistype))
      rval = _c;

  return rval;
}

//----------------------------------------------------------
bool StackSelector::Broadcast(Visitor * v)
{  return false;  }

// Allows the factory to update the accessor's map.
bool StackSelector::Add(Conduit * c, Visitor * v)
{
  bool rval = false;
  if (!_c) {
    assert(v->GetType().Is_A(_sean_vistype));
    _c = c;
    rval = true;
  }
  return rval;
}

//----------------------------------------------------------
bool StackSelector::Del(Conduit * c)
{ return false; }

//----------------------------------------------------------
bool StackSelector::Del(Visitor * v)
{ return false; }

