// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopEndpoint.h               Created on: 20 Dec 2000
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
  Revision 1.1.4.1  2001/04/18 17:26:29  sll
  Big checkin with the brand new internal APIs.

*/

#ifndef __GIOPENDPOINT_H__
#define __GIOPENDPOINT_H__

#include <omniORB4/omniutilities.h>

OMNI_NAMESPACE_BEGIN(omni)

class giopConnection;
class giopEndpoint;



class giopAddress {
public:
  // Each giopAddress must register via decodeIOR interceptor if it
  // wants to decode its own IOR component.

  // None of the members raise an exception.

  static giopAddress* str2Address(const char* address);
  // Given a string, returns an instance that can be used to connect to
  // the address.
  // The format of the address string is as follows:
  //     giop:<transport identifier>:[<transport specific fields]+
  //
  // The format of the following transports are defined (but may not be
  // implemented yet):
  //
  //   giop:tcp:<hostname>:<port no.>
  //   giop:ssl:<hostname>:<port no.>
  //   giop:unix:<filename>
  //   giop:fd:<file no.>
  //
  // Returns 0 if no suitable endpoint can be created.

  virtual const char* type() const = 0;
  // return the transport identifier

  virtual const char* address() const = 0;
  // return the string that describe this remote address.
  // The string format is described in str2Address().

  virtual giopConnection* connect(unsigned long deadline_secs = 0,
				  unsigned long deadline_nanosecs = 0)const =0;
  // Connect to the remote address.
  // Return 0 if no connection can be established.

  virtual giopAddress* duplicate() const = 0;
  // Return an identical instance.

  giopAddress() {}
  virtual ~giopAddress() {}

private:
  giopAddress(const giopAddress&);
  giopAddress& operator= (const giopAddress&);

};

typedef omnivector<giopAddress*>  giopAddressList;

class giopEndpoint {
public:
  // None of the members raise an exception.

  static giopEndpoint* str2Endpoint(const char* endpoint);
  // Given a string, returns an instance that represent the endpoint
  // The format of an endpoint string is as follows:
  //     <transport identifier>/[<transport specific fields]+
  //
  // The format of the following transports are defined (but may not be
  // implemented yet):
  //
  //   giop:tcp:<hostname>:<port no.>      note 1
  //   giop:ssl:<hostname>:<port no.>      note 1
  //   giop:unix:<filename>
  //   giop:fd:<file no.>
  //
  // Note 1: if <hostname> is empty, the IP address of one of the host
  //         network interface will be used.
  //         if <port no.> is not present, a port number is chosen by
  //         the operation system.
  //
  // Returns 0 if no suitable endpoint can be created.

  static _CORBA_Boolean strIsValidEndpoint(const char* endpoint);
  // Return true if endpoint is syntactically correct as described
  // in str2Endpoint(). None of the fields are optional.

  static _CORBA_Boolean addToIOR(const char* endpoint);
  // Return true if the endpoint has been sucessfully registered so that
  // all IORs generated by the ORB will include this endpoint.

  virtual const char* type() const = 0;
  // return the transport identifier

  virtual const char* address() const = 0;
  // return the string that describe this endpoint.
  // The string format is described in str2Endpoint().

  virtual _CORBA_Boolean bind() = 0;
  // Establish a binding to the this address.
  // Return TRUE(1) if the binding has been established successfully,
  // otherwise returns FALSE(0).

  virtual giopConnection* accept() = 0;
  // Accept a new connection. Returns 0 if no connection can be
  // accepted.

  virtual void poke() = 0;
  // Call to unblock any thread blocking in accept().

  virtual void shutdown() = 0;
  // Remove the binding.

  giopEndpoint() {}
  virtual ~giopEndpoint() {}

private:
  giopEndpoint(const giopEndpoint&);
  giopEndpoint& operator=(const giopEndpoint&);
};

typedef omnivector<giopEndpoint*>  giopEndpointList;

class giopConnection {
public:
  // None of the members raise an exception.

  virtual int send(void* buf, size_t sz,
		   unsigned long deadline_secs = 0,
		   unsigned long deadline_nanosecs = 0) = 0;
  virtual int recv(void* buf, size_t sz,
		   unsigned long deadline_secs = 0,
		   unsigned long deadline_nanosecs = 0) = 0;
  virtual void   shutdown() = 0;

  virtual const char* myaddress() = 0;
  virtual const char* peeraddress() = 0;

  giopConnection() {}
  virtual ~giopConnection() {}

private:
  giopConnection(const giopConnection&);
  giopConnection& operator=(const giopConnection&);
};

class giopTransportImpl {
public:

  virtual giopEndpoint* toEndpoint(const char* param) = 0;
  virtual giopAddress*   toAddress(const char* param) = 0;
  virtual _CORBA_Boolean isValid(const char* param) = 0;
  virtual _CORBA_Boolean addToIOR(const char* param) = 0;

  const char*     type;
  giopTransportImpl* next;

  giopTransportImpl(const char* t);
  virtual ~giopTransportImpl();

private:
  giopTransportImpl();
  giopTransportImpl(const giopTransportImpl&);
  giopTransportImpl& operator=(const giopTransportImpl&);
};


OMNI_NAMESPACE_END(omni)

#endif // __GIOPENDPOINT_H__