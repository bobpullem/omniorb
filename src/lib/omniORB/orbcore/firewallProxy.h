// -*- Mode: C++; -*-
//                            Package   : omniORB2
// firewallProxy.h            Created on: 28/9/99
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
 Revision 1.1.4.1  2000/09/27 17:30:28  sll
 *** empty log message ***

 Revision 1.1.2.1  1999/10/02 18:21:27  sll
 Added support to decode optional tagged components in the IIOP profile.
 Added support to negogiate with a firewall proxy- GIOPProxy to invoke
 remote objects inside a firewall.
 Added tagged component TAG_ORB_TYPE to identify omniORB IORs.

*/

#ifndef __FIREWALLPROXY_H__
#define __FIREWALLPROXY_H__

class firewallProxyRopeFactory : public outgoingRopeFactory {
public:

  const ropeFactoryType* getType() const { return 0; }
  CORBA::Boolean isOutgoing(Endpoint* addr) const { return 0; }

  Rope* findOrCreateOutgoing(Endpoint* addr, omniIOR* ior=0);

  friend class nobody;

  static firewallProxyRopeFactory* singleton();

private:

  firewallProxyRopeFactory() {}
  ~firewallProxyRopeFactory() {} // Cannot delete a factory instance
};


#endif // __TCPSOCKET_H__