// -*- Mode: C++; -*-
//                            Package   : omniORB
// GIOP_C.h                   Created on: 05/01/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
//    02111-1307, USA
//
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//	

/*
  $Log$
  Revision 1.1.4.2  2001/05/01 16:07:33  sll
  All GIOP implementations should now work with fragmentation and abitrary
  sizes non-copy transfer.

  Revision 1.1.4.1  2001/04/18 17:19:01  sll
  Big checkin with the brand new internal APIs.

  */


#ifndef __GIOP_C_H__
#define __GIOP_C_H__

#include <omniORB4/IOP_C.h>

OMNI_NAMESPACE_BEGIN(omni)

class GIOP_C : public IOP_C, public giopStream, public giopStreamList {
 public:

  GIOP_C(giopRope*,giopStrand*);
  ~GIOP_C();

  void InitialiseRequest();

  GIOP::ReplyStatusType ReceiveReply();

  void RequestCompleted(CORBA::Boolean skip=0);

  GIOP::LocateStatusType IssueLocateRequest();

  void notifyCommFailure(CORBA::ULong& minor,
			 CORBA::Boolean& retry);
  // override giopStream member

  _CORBA_ULong completion();
  // override cdrStream member.

  cdrStream& getStream() { return *this; }

  void initialise(const omniIOR*,
		  const CORBA::Octet* key,
		  int keysize,
		  omniCallDescriptor*);

  void cleanup();

  IOP_C::State state() const { return pd_state; }
  void state(IOP_C::State s) { pd_state = s; }

  omniCallDescriptor* calldescriptor() { return pd_calldescriptor; }
  void calldescriptor(omniCallDescriptor* c) { pd_calldescriptor = c; }

  inline const CORBA::Octet* key() const  { return pd_key;    }
  inline void key(const CORBA::Octet* k)  { pd_key = k; }

  inline CORBA::ULong keysize() const  { return pd_keysize; }
  inline void keysize(CORBA::ULong sz) { pd_keysize = sz; }

  const omniIOR* ior() const { return pd_ior; }
  void ior(const omniIOR* c) { pd_ior = c; }

  GIOP::ReplyStatusType replyStatus() const { return pd_replyStatus; }
  void replyStatus(GIOP::ReplyStatusType rc) { pd_replyStatus = rc; }

  GIOP::LocateStatusType locateStatus() const { return pd_locateStatus; }
  void locateStatus(GIOP::LocateStatusType rc) { pd_locateStatus = rc; }

  inline CORBA::ULong  replyId() const { return pd_reply_id; }
  inline void replyId(CORBA::ULong v) { pd_reply_id = v; }

private:
  IOP_C::State            pd_state;
  omniCallDescriptor*     pd_calldescriptor;
  const omniIOR*          pd_ior;
  const CORBA::Octet*     pd_key;
  CORBA::ULong            pd_keysize;
  giopRope*               pd_rope;
  GIOP::ReplyStatusType   pd_replyStatus;
  GIOP::LocateStatusType  pd_locateStatus;
  CORBA::ULong            pd_reply_id;

  void UnMarshallSystemException();

  GIOP_C();
  GIOP_C(const GIOP_C&);
  GIOP_C& operator=(const GIOP_C&);
};

OMNI_NAMESPACE_END(omni)

#endif // __GIOP_C_H__
