// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpEndpoint.cc             Created on: 19 Mar 2001
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
  Revision 1.1.2.5  2001/07/13 15:34:24  sll
  Added the ability to monitor connections and callback to the giopServer when
  data has arrived at a connection.

  Revision 1.1.2.4  2001/06/20 18:35:15  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.2.3  2001/06/13 20:13:49  sll
  Minor updates to make the ORB compiles with MSVC++.

  Revision 1.1.2.2  2001/05/01 16:04:42  sll
  Silly use of sizeof() on const char*. Should use strlen().

  Revision 1.1.2.1  2001/04/18 18:10:43  sll
  Big checkin with the brand new internal APIs.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <tcp/tcpConnection.h>
#include <tcp/tcpAddress.h>
#include <tcp/tcpEndpoint.h>

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
unsigned long tcpEndpoint::scan_interval_sec  = 0;
unsigned long tcpEndpoint::scan_interval_nsec = 50*1000*1000;
CORBA::ULong  tcpEndpoint::hashsize           = 103;


/////////////////////////////////////////////////////////////////////////
tcpEndpoint::tcpEndpoint(const IIOP::Address& address) :
  pd_socket(RC_INVALID_SOCKET), pd_address(address), 
  pd_n_fdset_1(0), pd_n_fdset_2(0), pd_n_fdset_dib(0),
  pd_abs_sec(0), pd_abs_nsec(0) {

  pd_address_string = (const char*) "giop:tcp:255.255.255.255:65535";
  // address string is not valid until bind is called.

  FD_ZERO(&pd_fdset_1);
  FD_ZERO(&pd_fdset_2);
  FD_ZERO(&pd_fdset_dib);

  pd_hash_table = new tcpConnection* [hashsize];
  for (CORBA::ULong i=0; i < hashsize; i++)
    pd_hash_table[i] = 0;
}

/////////////////////////////////////////////////////////////////////////
tcpEndpoint::tcpEndpoint(const char* address) :
  pd_socket(RC_INVALID_SOCKET),
  pd_n_fdset_1(0), pd_n_fdset_2(0), pd_n_fdset_dib(0),
  pd_abs_sec(0), pd_abs_nsec(0) {

  pd_address_string = address;
  // OMNIORB_ASSERT(strncmp(address,"giop:tcp:",9) == 0);
  const char* host = strchr(address,':');
  host = strchr(host+1,':') + 1;
  const char* port = strchr(host,':') + 1;
  CORBA::ULong hostlen = port - host - 1;
  // OMNIORB_ASSERT(hostlen);
  pd_address.host = CORBA::string_alloc(hostlen);
  strncpy(pd_address.host,host,hostlen);
  pd_address.host[hostlen] = '\0';
  int rc;
  unsigned int v;
  rc = sscanf(port,"%u",&v);
  // OMNIORB_ASSERT(rc == 1);
  // OMNIORB_ASSERT(v > 0 && v < 65536);
  pd_address.port = v;

  FD_ZERO(&pd_fdset_1);
  FD_ZERO(&pd_fdset_2);
  FD_ZERO(&pd_fdset_dib);

  pd_hash_table = new tcpConnection* [hashsize];
  for (CORBA::ULong i=0; i < hashsize; i++)
    pd_hash_table[i] = 0;
}

/////////////////////////////////////////////////////////////////////////
tcpEndpoint::~tcpEndpoint() {
  if (pd_socket != RC_INVALID_SOCKET) {
    CLOSESOCKET(pd_socket);
    pd_socket = RC_INVALID_SOCKET;
  }
}

/////////////////////////////////////////////////////////////////////////
const char*
tcpEndpoint::type() const {
  return "giop:tcp";
}

/////////////////////////////////////////////////////////////////////////
const char*
tcpEndpoint::address() const {
  return pd_address_string;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpEndpoint::Bind() {

  OMNIORB_ASSERT(pd_socket == RC_INVALID_SOCKET);

  struct sockaddr_in addr;
  if ((pd_socket = socket(INETSOCKET,SOCK_STREAM,0)) == RC_INVALID_SOCKET) {
    return 0;
  }

  addr.sin_family = INETSOCKET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(pd_address.port);

  if (addr.sin_port) {
    int valtrue = 1;
    if (setsockopt(pd_socket,SOL_SOCKET,SO_REUSEADDR,
		   (char*)&valtrue,sizeof(int)) == RC_SOCKET_ERROR) {
      CLOSESOCKET(pd_socket);
      pd_socket = RC_INVALID_SOCKET;
      return 0;
    }
  }

  if (::bind(pd_socket,(struct sockaddr *)&addr,
	     sizeof(struct sockaddr_in)) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_socket);
    return 0;
  }

  if (listen(pd_socket,5) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_socket);
    return 0;
  }

  SOCKNAME_SIZE_T l;
  l = sizeof(struct sockaddr_in);
  if (getsockname(pd_socket,
		  (struct sockaddr *)&addr,&l) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_socket);
    return 0;
  }
  pd_address.port = ntohs(addr.sin_port);

  {
    char self[64];
    if (gethostname(&self[0],64) == RC_SOCKET_ERROR) {
      if (omniORB::trace(0)) {
	omniORB::logger log;
	log << "Cannot get the name of this host\n";
      }
      CLOSESOCKET(pd_socket);
      return 0;
    }

    LibcWrapper::hostent_var h;
    int rc;

    if (LibcWrapper::gethostbyname(self,h,rc) < 0) {
      if (omniORB::trace(0)) {
	omniORB::logger log;
	log << "Cannot get the address of this host\n";
      }
      CLOSESOCKET(pd_socket);
      return 0;
    }
    memcpy((void *)&addr.sin_addr,
	   (void *)h.hostent()->h_addr_list[0],
	   sizeof(addr.sin_addr));
  }
  if (!(char*)pd_address.host || strlen(pd_address.host) == 0) {
    pd_address.host = tcpConnection::ip4ToString(addr.sin_addr.s_addr);
  }
  if (strcmp(pd_address.host,"127.0.0.1") == 0 && omniORB::trace(1)) {
    omniORB::logger log;
    log << "Warning: the local loop back interface (127.0.0.1) is used as this server's address.\n";
    log << "Warning: only clients on this machine can talk to this server.\n";
  }

  const char* format = "giop:tcp:%s:%d";
  pd_address_string = CORBA::string_alloc(strlen(pd_address.host)+strlen(format)+6);
  sprintf((char*)pd_address_string,format,
	  (const char*)pd_address.host,(int)pd_address.port);

  FD_SET(pd_socket,&pd_fdset_1);
  FD_SET(pd_socket,&pd_fdset_2);
  pd_n_fdset_1++;
  pd_n_fdset_2++;
  return 1;
}

/////////////////////////////////////////////////////////////////////////
void
tcpEndpoint::Poke() {

  tcpAddress* target = new tcpAddress(pd_address);
  tcpConnection* conn;
  if ((conn = (tcpConnection*)target->Connect()) == 0) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Warning: Fail to connect to myself ("
	  << (const char*) pd_address_string << ") via tcp!\n";
      log << "Warning: ATM this is ignored but this may cause the ORB shutdown to hang.\n";
    }
  }
  else {
    delete conn;
  }
  delete target;
}

/////////////////////////////////////////////////////////////////////////
void
tcpEndpoint::Shutdown() {
  SHUTDOWNSOCKET(pd_socket);
}

/////////////////////////////////////////////////////////////////////////
giopConnection*
tcpEndpoint::AcceptAndMonitor(giopEndpoint::notifyReadable_t func,
			      void* cookie) {

  OMNIORB_ASSERT(pd_socket != RC_INVALID_SOCKET);

  struct timeval timeout;
  fd_set         rfds;
  tcpSocketHandle_t sock;
  int            total;
  
  while (1) {

    // (pd_abs_sec,pd_abs_nsec) define the absolute time when we switch fdset
    tcpConnection::setTimeOut(pd_abs_sec,pd_abs_nsec,timeout);

    if (timeout.tv_sec == 0 && timeout.tv_usec == 0) {

      omni_thread::get_time(&pd_abs_sec,&pd_abs_nsec,
			    scan_interval_sec,scan_interval_nsec);
      timeout.tv_sec = scan_interval_sec;
      timeout.tv_usec = scan_interval_nsec / 1000;

      omni_tracedmutex_lock sync(pd_fdset_lock);
      rfds  = pd_fdset_2;
      total = pd_n_fdset_2;
      pd_fdset_2 = pd_fdset_1;
      pd_n_fdset_2 = pd_n_fdset_1;
    }
    else {

      omni_tracedmutex_lock sync(pd_fdset_lock);
      rfds  = pd_fdset_2;
      total = pd_n_fdset_2;
    }

    int maxfd = 0;
    int fd = 0;
    while (total) {
      if (FD_ISSET(fd,&rfds)) {
	maxfd = fd;
	total--;
      }
      fd++;
    }

    int nready = select(maxfd+1,&rfds,0,0,&timeout);

    if (nready == RC_SOCKET_ERROR) {
      if (ERRNO != RC_EINTR) {
	sock = RC_SOCKET_ERROR;
	break;
      }
      else {
	continue;
      }
    }

    // Reach here if nready >= 0

    if (FD_ISSET(pd_socket,&rfds)) {
      // Got a new connection
      sock = ::accept(pd_socket,0,0);
      break;
    }

    {
      omni_tracedmutex_lock sync(pd_fdset_lock);

      // Process the result from the select.
      tcpSocketHandle_t fd = 0;
      while (nready) {
	if (FD_ISSET(fd,&rfds)) {
	  notifyReadable(fd,func,cookie);
	  if (FD_ISSET(fd,&pd_fdset_1)) {
	    pd_n_fdset_1--;
	    FD_CLR(fd,&pd_fdset_1);
	  }
	  if (FD_ISSET(fd,&pd_fdset_dib)) {
	    pd_n_fdset_dib--;
	    FD_CLR(fd,&pd_fdset_dib);
	  }
	  nready--;
	}
	fd++;
      }

      // Process pd_fdset_dib. Those sockets with their bit set have
      // already got data in buffer. We do a call back for these sockets if
      // their entries in pd_fdset_1 is also set.
      fd = 0;
      nready = pd_n_fdset_dib;
      while (nready) {
	if (FD_ISSET(fd,&pd_fdset_dib)) {
	  if (FD_ISSET(fd,&pd_fdset_2)) {
	    notifyReadable(fd,func,cookie);
	    if (FD_ISSET(fd,&pd_fdset_1)) {
	      pd_n_fdset_1--;
	      FD_CLR(fd,&pd_fdset_1);
	    }
	    pd_n_fdset_dib--;
	    FD_CLR(fd,&pd_fdset_dib);
	  }
	  nready--;
	}
	fd++;
      }
    }
  }

  if (sock == RC_SOCKET_ERROR) {
    return 0;
  }
  else {
    return new tcpConnection(sock,this);
  }

}

/////////////////////////////////////////////////////////////////////////
void
tcpEndpoint::notifyReadable(tcpSocketHandle_t fd,
			    giopEndpoint::notifyReadable_t func,
			    void* cookie) {

  // Caller hold pd_fdset_lock

  if (FD_ISSET(fd,&pd_fdset_2)) {
    pd_n_fdset_2--;
    FD_CLR(fd,&pd_fdset_2);
    giopConnection* conn = 0;
    tcpConnection** head = &(pd_hash_table[fd%hashsize]);
    while (*head) {
      if ((*head)->handle() == fd) {
	conn = (giopConnection*)(*head);
	break;
      }
      head = &((*head)->pd_next);
    }
    if (conn) {
      // Note: do the callback while holding pd_fdset_lock
      func(cookie,conn);
    }
  }
}

/////////////////////////////////////////////////////////////////////////
void
tcpConnection::setSelectable(CORBA::Boolean now,
			     CORBA::Boolean data_in_buffer) {

  if (!pd_endpoint) return;

  omni_tracedmutex_lock sync(pd_endpoint->pd_fdset_lock);

  if (data_in_buffer && !FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_dib))) {
    pd_endpoint->pd_n_fdset_dib++;
    FD_SET(pd_socket,&(pd_endpoint->pd_fdset_dib));
  }

  if (!FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_1)))
    pd_endpoint->pd_n_fdset_1++;
  FD_SET(pd_socket,&(pd_endpoint->pd_fdset_1));
  if (now) {
    if (!FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_2)))
      pd_endpoint->pd_n_fdset_2++;
    FD_SET(pd_socket,&(pd_endpoint->pd_fdset_2));
    // XXX poke the thread doing accept to look at the fdset immediately.
  }
}


/////////////////////////////////////////////////////////////////////////
void
tcpConnection::clearSelectable() {

  if (!pd_endpoint) return;

  omni_tracedmutex_lock sync(pd_endpoint->pd_fdset_lock);

  if (FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_1)))
    pd_endpoint->pd_n_fdset_1--;
  FD_CLR(pd_socket,&(pd_endpoint->pd_fdset_1));
  if (FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_2)))
    pd_endpoint->pd_n_fdset_2--;
  FD_CLR(pd_socket,&(pd_endpoint->pd_fdset_2));
  if (FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_dib)))
    pd_endpoint->pd_n_fdset_dib--;
  FD_CLR(pd_socket,&(pd_endpoint->pd_fdset_dib));
}

/////////////////////////////////////////////////////////////////////////
void
tcpConnection::Peek(giopEndpoint::notifyReadable_t func, void* cookie) {

  if (!pd_endpoint) return;

  {
    omni_tracedmutex_lock sync(pd_endpoint->pd_fdset_lock);
   
    // Do nothing if this connection is not set to be monitored.
    if (!FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_1)))
      return;

    // If data in buffer is set, do callback straight away.
    if (FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_dib))) {
      func(cookie,this);
      if (FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_1))) {
	pd_endpoint->pd_n_fdset_1--;
	FD_CLR(pd_socket,&(pd_endpoint->pd_fdset_1));
      }
      pd_endpoint->pd_n_fdset_2--;
      FD_CLR(pd_socket,&(pd_endpoint->pd_fdset_2));
      pd_endpoint->pd_n_fdset_dib--;
      FD_CLR(pd_socket,&(pd_endpoint->pd_fdset_dib));
    }
  }

  struct timeval timeout;
  // select on the socket for half the time of scan_interval, if no request
  // arrives in this interval, we just let AcceptAndMonitor take care
  // of it.
  timeout.tv_sec = tcpEndpoint::scan_interval_sec / 2;
  timeout.tv_usec = tcpEndpoint::scan_interval_nsec / 1000 / 2;
  fd_set         rfds;

  do {
    FD_SET(pd_socket,&rfds);
    int nready = select(pd_socket+1,&rfds,0,0,&timeout);

    if (nready == RC_SOCKET_ERROR) {
      if (ERRNO != RC_EINTR) {
	break;
      }
      else {
	continue;
      }
    }

    // Reach here if nready >= 0

    if (FD_ISSET(pd_socket,&rfds)) {
      omni_tracedmutex_lock sync(pd_endpoint->pd_fdset_lock);

      // Are we still interested?
      if (FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_1))) {
	func(cookie,this);
	if (FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_2))) {
	  pd_endpoint->pd_n_fdset_2--;
	  FD_CLR(pd_socket,&(pd_endpoint->pd_fdset_2));
	}
	pd_endpoint->pd_n_fdset_1--;
	FD_CLR(pd_socket,&pd_endpoint->pd_fdset_1);
	if (FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_dib))) {
	  pd_endpoint->pd_n_fdset_dib--;
	  FD_CLR(pd_socket,&(pd_endpoint->pd_fdset_dib));
	}
      }
    }
  } while(0);
}


OMNI_NAMESPACE_END(omni)
