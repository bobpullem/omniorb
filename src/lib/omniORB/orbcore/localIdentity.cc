// -*- Mode: C++; -*-
//                            Package   : omniORB
// localIdentity.cc           Created on: 16/6/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
//

/*
  $Log$
  Revision 1.2.2.5  2001/06/07 16:24:10  dpg1
  PortableServer::Current support.

  Revision 1.2.2.4  2001/05/29 17:03:51  dpg1
  In process identity.

  Revision 1.2.2.3  2001/04/18 18:18:07  sll
  Big checkin with the brand new internal APIs.

  Revision 1.2.2.2  2000/09/27 17:57:05  sll
  Changed include/omniORB3 to include/omniORB4

  Revision 1.2.2.1  2000/07/17 10:35:54  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:57  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.5  2000/06/22 10:40:15  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.1.2.4  1999/10/27 17:32:11  djr
  omni::internalLock and objref_rc_lock are now pointers.

  Revision 1.1.2.3  1999/10/14 16:22:11  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.2  1999/09/30 12:25:59  djr
  Minor changes.

  Revision 1.1.2.1  1999/09/22 14:26:52  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <localIdentity.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/callHandle.h>
#include <objectAdapter.h>
#include <exceptiondefs.h>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
OMNI_NAMESPACE_BEGIN(omni)

class omniLocalIdentity_RefHolder {
public:
  inline omniLocalIdentity_RefHolder(omniLocalIdentity* id) : pd_id(id) {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    pd_id->pd_nInvocations++;
  }

  inline ~omniLocalIdentity_RefHolder() {
    omni::internalLock->lock();
    pd_id->pd_nInvocations--;
    pd_id->pd_adapter->leaveAdapter();
    int done = pd_id->pd_nInvocations > 0;
    omni::internalLock->unlock();
    if( done )  return;
    // Object has been deactivated, and the last invocation
    // has completed.  Pass the object back to the adapter
    // so it can be etherealised.
    pd_id->adapter()->lastInvocationHasCompleted(pd_id);
  }

private:
  omniLocalIdentity* pd_id;
};

OMNI_NAMESPACE_END(omni)


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
OMNI_USING_NAMESPACE(omni)

void
omniLocalIdentity::dispatch(omniCallDescriptor& call_desc)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(pd_adapter && pd_servant);

  call_desc.localId(this);

  omniLocalIdentity_RefHolder rh(this);

  omni::localInvocationCount++;

#ifndef HAS_Cplusplus_catch_exception_by_base
  // The compiler cannot catch exceptions by base class, hence
  // we cannot trap invalid exceptions going through here.
  pd_adapter->dispatch(call_desc, this);

#else
  try { pd_adapter->dispatch(call_desc, this); }

  catch(CORBA::Exception&) {
    throw;
  }
  catch(omniORB::LOCATION_FORWARD&) {
    throw;
  }

  catch(...) {
    if( omniORB::trace(2) ) {
      omniORB::logger l;
      l << "WARNING -- method \'" << call_desc.op() << "\' raised an unknown\n"
	" exception (not a legal CORBA exception).\n";
    }
    OMNIORB_THROW(UNKNOWN,0, CORBA::COMPLETED_MAYBE);
  }
#endif
}


void
omniLocalIdentity::dispatch(omniCallHandle& handle)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(pd_adapter && pd_servant);

  handle.localId(this);

  omniLocalIdentity_RefHolder rh(this);

  omni::remoteInvocationCount++;

  pd_adapter->dispatch(handle, this);
}


void
omniLocalIdentity::finishedWithDummyId()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(!pd_servant && !pd_adapter && !pd_localRefs);

  omniORB::logs(15, "Dummy omniLocalIdentity deleted (no more local refs).");

  delete this;
}


void
omniLocalIdentity::die()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);
  OMNIORB_ASSERT(pd_nInvocations == 0);
  OMNIORB_ASSERT(pd_localRefs == 0);

  // The servant is cleaned up by the caller.
  // We do not hold a ref to the adapter.

  omniORB::logs(15, "omniLocalIdentity deleted.");

  delete this;
}


void
omniLocalIdentity::gainObjRef(omniObjRef* objref)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(objref);

  *(objref->_addrOfLocalRefList()) = pd_localRefs;
  pd_localRefs = objref;
}


void
omniLocalIdentity::loseObjRef(omniObjRef* objref)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(objref);

  omniObjRef** p = &pd_localRefs;

  while( *p != objref ) {
    OMNIORB_ASSERT(*p);
    p = (*p)->_addrOfLocalRefList();
  }

  *p = *(*p)->_addrOfLocalRefList();
  *(objref->_addrOfLocalRefList()) = 0;
}

void
omniLocalIdentity::locateRequest() {
  // Its a local object, and we know its here.
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  omni::internalLock->unlock();
}

omniIdentity::equivalent_fn
omniLocalIdentity::get_real_is_equivalent() const {
  return real_is_equivalent;
}

CORBA::Boolean
omniLocalIdentity::real_is_equivalent(const omniIdentity* id1,
				      const omniIdentity* id2) {

  const CORBA::Octet* key1 = id1->key();
  int keysize1             = id1->keysize();

  const CORBA::Octet* key2 = id2->key();
  int keysize2             = id2->keysize();

  if (keysize1 != keysize2 || memcmp((void*)key1,(void*)key2,keysize1) != 0)
      // Object keys do not match
      return 0;

  return 1;
}
