// -*- Mode: C++; -*-
//                            Package   : omniORB2
// giopBiDir.cc               Created on: 17/7/2001
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
  Revision 1.1.2.5  2001/08/23 10:11:53  sll
  Initialise BiDirPolicy constants properly for compilers with no namespace
  support.

  Revision 1.1.2.4  2001/08/21 11:02:14  sll
  orbOptions handlers are now told where an option comes from. This
  is necessary to process DefaultInitRef and InitRef correctly.

  Revision 1.1.2.3  2001/08/17 17:12:36  sll
  Modularise ORB configuration parameters.

  Revision 1.1.2.2  2001/07/31 17:31:40  sll
  strchr returns const char*.

  Revision 1.1.2.1  2001/07/31 16:10:38  sll
  Added GIOP BiDir support.

  */

#include <omniORB4/CORBA.h>
#include <omniORB4/minorCode.h>
#include <omniORB4/omniInterceptors.h>
#include <exceptiondefs.h>
#include <giopStrand.h>
#include <giopRope.h>
#include <giopBiDir.h>
#include <giopStreamImpl.h>
#include <giopStream.h>
#include <GIOP_C.h>
#include <GIOP_S.h>
#include <giopServer.h>
#include <initialiser.h>
#include <objectAdapter.h>
#include <tcp/tcpTransportImpl.h>
#include <orbOptions.h>
#include <orbParameters.h>

OMNI_USING_NAMESPACE(omni)

// XXX To-do
// a) BiDirServerRope garbage collection. At the moment, the server
//    side of a bidirectional connection is garbage collected in
//    the same way as an ordinary connection. That is, it is closed after
//    a period of inactivity. In fact, the connection should only be closed
//    if all the object references unmarshal from the connection have
//    been released. In other words, the reference count on the
//    BiDirServerRope instance attached to the connection == 0.

////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////
CORBA::Boolean orbParameters::acceptBiDirectionalGIOP = 0;
//   Applies to the server side. Set to 1 to indicates that the
//   ORB may choose to accept a clients offer to use bidirectional
//   GIOP calls on a connection. Set to 0 means the ORB should
//   never accept any bidirectional offer and should stick to normal
//   GIOP.
//
//   Valid values = 0 or 1

CORBA::Boolean orbParameters::offerBiDirectionalGIOP = 0;
//   Applies to the client side. Set to 1 to indicates that the
//   ORB may choose to use a connection to do bidirectional GIOP
//   calls. Set to 0 means the ORB should never do bidirectional.
//
//   Valid values = 0 or 1

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

_init_in_def_( const CORBA::PolicyType
	       BiDirPolicy::BIDIRECTIONAL_POLICY_TYPE = 37; )

_init_in_def_( const BiDirPolicy::BidirectionalPolicyValue 
                BiDirPolicy::NORMAL = 0; )

_init_in_def_( const BiDirPolicy::BidirectionalPolicyValue 
                BiDirPolicy::BOTH = 1; )

BiDirPolicy::BidirectionalPolicy::~BidirectionalPolicy() {}

CORBA::Policy_ptr
BiDirPolicy::BidirectionalPolicy::copy()
{
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();
  return new BidirectionalPolicy(pd_value);
}

void*
BiDirPolicy::BidirectionalPolicy::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId );

  if( omni::ptrStrMatch(repoId, BiDirPolicy::BidirectionalPolicy::_PD_repoId) )
    return (BiDirPolicy::BidirectionalPolicy_ptr) this;
  if( omni::ptrStrMatch(repoId, CORBA::Policy::_PD_repoId) )
    return (CORBA::Policy_ptr) this;
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}

BiDirPolicy::BidirectionalPolicy_ptr
BiDirPolicy::BidirectionalPolicy::_duplicate(BiDirPolicy::BidirectionalPolicy_ptr obj)
{
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();

  return obj;
}

BiDirPolicy::BidirectionalPolicy_ptr
BiDirPolicy::BidirectionalPolicy::_narrow(CORBA::Object_ptr obj)
{
  if( CORBA::is_nil(obj) )  return _nil();

  BidirectionalPolicy_ptr p = (BidirectionalPolicy_ptr) obj->_ptrToObjRef(BidirectionalPolicy::_PD_repoId);

  if( p )  p->_NP_incrRefCount();

  return p ? p : _nil();
}

BiDirPolicy::BidirectionalPolicy_ptr
BiDirPolicy::BidirectionalPolicy::_nil()
{
  static BidirectionalPolicy* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new BidirectionalPolicy;
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

const char*
BiDirPolicy::BidirectionalPolicy::_PD_repoId = "IDL:omg.org/BiDirPolicy/BidirectionalPolicy:1.0";

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
typedef _CORBA_Unbounded_Sequence<IIOP::Address> ListenPointList;

////////////////////////////////////////////////////////////////////////
RopeLink BiDirServerRope::ropes;

////////////////////////////////////////////////////////////////////////
BiDirServerRope::BiDirServerRope(giopStrand* strand, giopAddress* addr) : 
  giopRope(addr,0), 
  pd_sendfrom((const char*)strand->connection->peeraddress()) 
{
  pd_maxStrands = 1;
  pd_oneCallPerConnection = 0;
  strand->RopeLink::insert(pd_strands);
}

////////////////////////////////////////////////////////////////////////
BiDirServerRope*
BiDirServerRope::addRope(giopStrand* strand, const giopAddressList& addrlist) {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  OMNIORB_ASSERT(!strand->isClient() && strand->biDir == 1);

  const char* sendfrom = strand->connection->peeraddress();

  BiDirServerRope* br = 0;

  // Check if there already exists a rope for the strand.
  RopeLink* p = BiDirServerRope::ropes.next;
  while ( p != &BiDirServerRope::ropes ) {
    br = (BiDirServerRope*)p;
    if (strcmp(sendfrom,br->pd_sendfrom) == 0) {
      break;
    }
    else if (br->pd_refcount == 0 &&
	     RopeLink::is_empty(br->pd_strands) &&
	     !br->pd_nwaiting) {
      // garbage rope, remove it
      p = p->next;
      br->RopeLink::remove();
      delete br;
    }
    else {
      p = p->next;
    }
    br = 0;
  }

  if (!br) {
    giopAddress* addr = giopAddress::str2Address(strand->connection->peeraddress());
    br = new BiDirServerRope(strand,addr);
    br->RopeLink::insert(BiDirServerRope::ropes);
  }

  giopAddressList::const_iterator i, last;
  i    = addrlist.begin();
  last = addrlist.end();
  for (; i != last; i++) {
    CORBA::Boolean matched = 0;
    giopAddressList::const_iterator j, k;
    j = br->pd_redirect_addresses.begin();
    k = br->pd_redirect_addresses.end();
    for (; j != k; j++) {
      if (omni::ptrStrMatch((*i)->address(),(*j)->address())) {
	matched = 1;
	break;
      }
    }
    if (!matched) {
      giopAddress* a = (*i)->duplicate();
      br->pd_redirect_addresses.push_back(a);
    }
  }

  return br;
}

////////////////////////////////////////////////////////////////////////
int
BiDirServerRope::selectRope(const giopAddressList& addrlist,
			    omniIOR::IORInfo* info,
			    Rope*& r) {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  const char* sendfrom = 0;

  omniIOR::IORExtraInfoList& cinfo = info->extraInfo();
  for (CORBA::ULong index = 0; index < cinfo.length(); index++) {
    if (cinfo[index]->compid == IOP::TAG_OMNIORB_BIDIR) {
      sendfrom = ((BiDirInfo*)cinfo[index])->sendfrom;
      break;
    }
  }

  if (!sendfrom) return 0;

  GIOP::Version v = info->version();
  // Only GIOP 1.2 IORs can do bidirectional calls.
  if (v.major != 1 || v.minor < 2) return 0;


  BiDirServerRope* br;

  RopeLink* p = BiDirServerRope::ropes.next;
  while ( p != &BiDirServerRope::ropes ) {
    br = (BiDirServerRope*)p;
    if (br->match(sendfrom,addrlist)) {
      br->realIncrRefCount();
      r = (Rope*)br;
      return 1;
    }
    else if (br->pd_refcount == 0 &&
	     RopeLink::is_empty(br->pd_strands) &&
	     !br->pd_nwaiting) {
      // garbage rope, remove it
      p = p->next;
      br->RopeLink::remove();
      delete br;
    }
    else {
      p = p->next;
    }
  }

  // Reach here because we cannot find a match.
  return 0;
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
BiDirServerRope::match(const char* sendfrom,
		       const giopAddressList& addrlist) const {

  if (strcmp(pd_sendfrom,sendfrom) != 0) return 0;

  giopAddressList::const_iterator i, last;
  i    = addrlist.begin();
  last = addrlist.end();
  for (; i != last; i++) {
    giopAddressList::const_iterator j, k;
    j = pd_redirect_addresses.begin();
    k = pd_redirect_addresses.end();
    for (; j != k; j++) {
      if (omni::ptrStrMatch((*i)->address(),(*j)->address())) return 1;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
IOP_C*
BiDirServerRope::acquireClient(const omniIOR* ior,
			       const CORBA::Octet* key,
			       CORBA::ULong keysize,
			       omniCallDescriptor* calldesc) {

  GIOP::Version v = ior->getIORInfo()->version();
  giopStreamImpl* impl = giopStreamImpl::matchVersion(v);
  if (!impl) {
    impl = giopStreamImpl::maxVersion();
    v = impl->version();
  }

  // Only use this connection to do calls with GIOP 1.2 or above.
  OMNIORB_ASSERT(v.major > 1 || v.minor >= 2);

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);

  omni_tracedmutex_lock sync(*omniTransportLock);

  OMNIORB_ASSERT(!pd_oneCallPerConnection && pd_maxStrands == 1);

  giopStrand* s = 0;
  if (pd_strands.next != &pd_strands) {
    s = (giopStrand*)(pd_strands.next);
  }

  if (!s ||  s->state() == giopStrand::DYING) {
    // We no longer have a working bidirectional connection.
    OMNIORB_THROW(TRANSIENT,TRANSIENT_BiDirConnIsGone,CORBA::COMPLETED_NO);
  }

  OMNIORB_ASSERT(s->state() == giopStrand::ACTIVE);

  // We do not check what GIOP version(s) the strand has been used for previously.
  // If ever we have 2 calls using 2 different versions (and both are 1.2 or 
  // above), we allow this to happen. Contrast this with the algorithm in
  // giopRope::acquireClient.

  GIOP_C* g;
  if (!giopStreamList::is_empty(s->clients)) {
    giopStreamList* gp = s->clients.next;
    for (; gp != &s->clients; gp = gp->next) {
      g = (GIOP_C*)gp;
      if (g->state() == IOP_C::UnUsed) {
	g->impl(impl);
	g->initialise(ior,key,keysize,calldesc);
	return g;
      }
    }
  }
  g = new GIOP_C(this,s);
  g->impl(impl);
  g->initialise(ior,key,keysize,calldesc);
  g->giopStreamList::insert(s->clients);
  return g;
}

////////////////////////////////////////////////////////////////////////
void
BiDirServerRope::decrRefCount() {
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);

  omni_tracedmutex_lock sync(*omniTransportLock);
  pd_refcount--;
  OMNIORB_ASSERT(pd_refcount >=0);

  if (pd_refcount) return;

  // Normally pd_strands always contain 1 strand unless the bidirectional
  // strand has been shutdown.
  if (RopeLink::is_empty(pd_strands) && !pd_nwaiting) {
    RopeLink::remove();
    delete this;
  }
}


////////////////////////////////////////////////////////////////////////
void
BiDirServerRope::realIncrRefCount() {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  OMNIORB_ASSERT(pd_refcount >= 0);

  pd_refcount++;
}

////////////////////////////////////////////////////////////////////////
BiDirClientRope::BiDirClientRope(const giopAddressList& addrlist,
				 const omnivector<CORBA::ULong>& preferred) :
  giopRope(addrlist,preferred)
{
  pd_maxStrands = 1;
  pd_oneCallPerConnection = 0;
}

////////////////////////////////////////////////////////////////////////
IOP_C*
BiDirClientRope::acquireClient(const omniIOR* ior,
			       const CORBA::Octet* key,
			       CORBA::ULong keysize,
			       omniCallDescriptor* calldesc) {

  GIOP_C* giop_c = (GIOP_C*) giopRope::acquireClient(ior,key,keysize,calldesc);

  omni_tracedmutex_lock sync(pd_lock);
  giopStrand& s = (giopStrand&)((giopStream&)(*giop_c));
  if (s.connection == 0 && s.state() != giopStrand::DYING) {
    giopActiveConnection* c = s.address->Connect(0,0); // XXX no deadline set
    if (c) s.connection = &(c->getConnection());
    if (!s.connection) {
      s.state(giopStrand::DYING);
    }
    else {
      // now make the connection manage by the giopServer.
      s.biDir = 1;
      giopActiveCollection* watcher = c->registerMonitor();
      if (omniORB::trace(20)) {
	omniORB::logger log;
	log << "Client opened bidirectional connection to " 
	    << s.connection->peeraddress() << "\n";
      }
      if (!giopServer::singleton()->addBiDirStrand(&s,watcher)) {
	{
	  omni_tracedmutex_lock sync(*omniTransportLock);
	  s.connection->decrRefCount();
	}
	s.connection = 0;
	s.biDir = 0;
	giopRope::releaseClient(giop_c);
	OMNIORB_THROW(TRANSIENT,
		      TRANSIENT_BiDirConnUsedWithNoPOA,
		      CORBA::COMPLETED_NO);
      }
    }
  }
  return giop_c;
}

/////////////////////////////////////////////////////////////////////////////
//            Server side interceptor for code set service context         //
/////////////////////////////////////////////////////////////////////////////
static
CORBA::Boolean
getBiDirServiceContext(omniInterceptors::serverReceiveRequest_T::info_T& info) {

  if (!orbParameters::acceptBiDirectionalGIOP) {
    // XXX If the ORB policy is "don't support bidirectional", don't bother 
    // doing any of the stuff below.
    return 1;
  }

  GIOP::Version ver = info.giop_s.version();

  giopStrand& strand = (giopStrand&)((giopStream&)info.giop_s);

  if (ver.minor != 2 || strand.isClient()) {
    // Only parse service context if the GIOP version is 1.2, on the server side
    return 1;
  }

  IOP::ServiceContextList& svclist = info.giop_s.receive_service_contexts();
  CORBA::ULong total = svclist.length();
  for (CORBA::ULong index = 0; index < total; index++) {
    if (svclist[index].context_id == IOP::BI_DIR_IIOP) {
      cdrEncapsulationStream e(svclist[index].context_data.get_buffer(),
			       svclist[index].context_data.length(),1);

      ListenPointList l;
      l <<= e;

      if (l.length() == 0) continue;

      if (omniORB::trace(25)) {
	omniORB::logger log;
	log << " receive bidir IIOP service context: ( ";

	for (CORBA::ULong i = 0; i < l.length(); i++) {
	  log << (const char*) l[i].host << ":" << l[i].port << " ";
	}

	log << ")\n";
      }

      giopAddressList addrList;

      for (CORBA::ULong i=0; i<l.length(); i++) {
	giopAddress* p = giopAddress::fromTcpAddress(l[i]);
	if (p) addrList.push_back(p);

	// XXX If this connection is SSL, we also add to the redirection
	// list a ssl version of the address. This will ensure that any 
	// callback objects from the other end will score a match.
	// This is necessary because the BI_DIR_IIOP service context does
	// not allow the other side to say an endpoint is SSL.
	// If omniORB 4 is the client, it sends both the TCP and the SSL
	// endpoint in the BI_DIR_IIOP listen point list. It may not 
	// have a TCP endpoint at all. In that case, the IORs from the
	// client will only contain a ssl endpoint. Our hack here will ensure
	// that the unmarshal object reference code will score a match.
	if (strncmp(strand.connection->myaddress(),"giop:ssl",8) == 0) {
	  p = giopAddress::fromSslAddress(l[i]);
	  if (p) addrList.push_back(p);
	}
      }

      if (addrList.empty()) continue;

      {
	ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);

	omni_tracedmutex_lock sync(*omniTransportLock);

	if (!strand.biDir) {
	  strand.biDir = 1;
	  if (!strand.server->notifySwitchToBiDirectional(strand.connection))
	    return 1;
	}

	BiDirServerRope* r = BiDirServerRope::addRope(&strand,addrList);

      }
    }
  }
  return 1;
}

/////////////////////////////////////////////////////////////////////////////
//            Client side interceptor for code set service context         //
/////////////////////////////////////////////////////////////////////////////
static
CORBA::Boolean
setBiDirServiceContext(omniInterceptors::clientSendRequest_T::info_T& info) {

  if (!orbParameters::offerBiDirectionalGIOP) {
    // XXX If the ORB policy is "don't support bidirectional", don't bother 
    // doing any of the stuff below.
    return 1;
  }

  giopStrand& g = (giopStrand&)(info.giopstream);
  GIOP::Version ver = info.giopstream.version();

  if (ver.minor != 2 || !g.biDir || !g.isClient() || g.biDir_initiated) {
    // Only send service context if the GIOP version is 1.2, this is
    // a bidirectional connection, on the client side and it has not
    // been used yet.
    return 1;
  }

  const omnivector<const char*>& epts = omniObjAdapter::listMyEndpoints();
  omnivector<const char*>::const_iterator i = epts.begin();
  omnivector<const char*>::const_iterator last = epts.end();

  ListenPointList l(epts.size());
  CORBA::ULong j = 0;
  for ( ; i != last; i++) {
    if (strncmp((*i),"giop:tcp",8) == 0 || strncmp((*i),"giop:ssl",8) == 0) {
      const char* p = strchr((*i),':');
      OMNIORB_ASSERT(p);
      p = strchr(p+1,':');
      OMNIORB_ASSERT(p);
      l.length(j+1);
      if (!tcpTransportImpl::parseAddress(p+1,l[j])) OMNIORB_ASSERT(0);
    }
  }
  if (l.length()) {
    cdrEncapsulationStream e(CORBA::ULong(0),1);
    l >>= e;

    CORBA::Octet* data;
    CORBA::ULong max,datalen;
    e.getOctetStream(data,max,datalen);

    CORBA::ULong len = info.service_contexts.length() + 1;
    info.service_contexts.length(len);
    info.service_contexts[len-1].context_id = IOP::BI_DIR_IIOP;
    info.service_contexts[len-1].context_data.replace(max,datalen,data,1);

    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << " send bidir IIOP service context: ( ";

      for (CORBA::ULong i = 0; i < l.length(); i++) {
	log << (const char*) l[i].host << ":" << l[i].port << " ";
      }

      log << ")\n";
    }
  }
  g.biDir_initiated = 1;
  return 1;
}

/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class acceptBiDirectionalGIOPHandler : public orbOptions::Handler {
public:

  acceptBiDirectionalGIOPHandler() : 
    orbOptions::Handler("acceptBiDirectionalGIOP",
			"acceptBiDirectionalGIOP = 0 or 1",
			1,
			"-ORBacceptBiDirectionalGIOP < 0 | 1 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::acceptBiDirectionalGIOP = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::acceptBiDirectionalGIOP,
			     result);
  }

};

static acceptBiDirectionalGIOPHandler acceptBiDirectionalGIOPHandler_;

/////////////////////////////////////////////////////////////////////////////
class offerBiDirectionalGIOPHandler : public orbOptions::Handler {
public:

  offerBiDirectionalGIOPHandler() : 
    orbOptions::Handler("offerBiDirectionalGIOP",
			"offerBiDirectionalGIOP = 0 or 1",
			1,
			"-ORBofferBiDirectionalGIOP < 0 | 1 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::offerBiDirectionalGIOP = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::offerBiDirectionalGIOP,
			     result);
  }

};

static offerBiDirectionalGIOPHandler offerBiDirectionalGIOPHandler_;

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_giopbidir_initialiser : public omniInitialiser {
public:

  omni_giopbidir_initialiser() {
    orbOptions::singleton().registerHandler(offerBiDirectionalGIOPHandler_);
    orbOptions::singleton().registerHandler(acceptBiDirectionalGIOPHandler_);
  }

  void attach() {

    // install interceptors
    omniInterceptors* interceptors = omniORB::getInterceptors();
    interceptors->clientSendRequest.add(setBiDirServiceContext);
    interceptors->serverReceiveRequest.add(getBiDirServiceContext);
  }
  void detach() {
  }
};


static omni_giopbidir_initialiser initialiser;

omniInitialiser& omni_giopbidir_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)

