// -*- Mode: C++; -*-
//                            Package   : omniORB
// SocketCollection.cc        Created on: 23 Jul 2001
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
  Revision 1.1.2.4  2001/08/24 15:56:44  sll
  Fixed code which made the wrong assumption about the semantics of
  do { ...; continue; } while(0)

  Revision 1.1.2.3  2001/08/02 13:00:53  sll
  Do not use select(0,0,0,0,&timeout), it doesn't work on win32.

  Revision 1.1.2.2  2001/08/01 15:56:07  sll
  Workaround MSVC++ bug. It generates wrong code with FD_ISSET and FD_SET
  under certain conditions.

  Revision 1.1.2.1  2001/07/31 16:16:26  sll
  New transport interface to support the monitoring of active connections.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <SocketCollection.h>

OMNI_NAMESPACE_BEGIN(omni)

#define GDB_DEBUG

/////////////////////////////////////////////////////////////////////////
void
SocketSetTimeOut(unsigned long abs_sec,
		 unsigned long abs_nsec,struct timeval& t)
{
  unsigned long now_sec, now_nsec;
  omni_thread::get_time(&now_sec,&now_nsec);

  if ((abs_sec <= now_sec) && ((abs_sec < now_sec) || (abs_nsec < now_nsec))) {
    t.tv_sec = t.tv_usec = 0;
  }
  else {
    t.tv_sec = abs_sec - now_sec;
    if (abs_nsec >= now_nsec) {
      t.tv_usec = (abs_nsec - now_nsec) / 1000;
    }
    else {
      t.tv_usec = (1000000000 + abs_nsec - now_nsec) / 1000;
      t.tv_sec -= 1;
    }
  }
}

/////////////////////////////////////////////////////////////////////////
int
SocketSetnonblocking(SocketHandle_t sock) {
# if !defined(__WIN32__)
  int fl = O_NONBLOCK;
  if (fcntl(sock,F_SETFL,fl) < RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# else
  u_long v = 1;
  if (ioctlsocket(sock,FIONBIO,&v) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# endif
}

/////////////////////////////////////////////////////////////////////////
int
SocketSetblocking(SocketHandle_t sock) {
# if !defined(__WIN32__)
  int fl = 0;
  if (fcntl(sock,F_SETFL,fl) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# else
  u_long v = 0;
  if (ioctlsocket(sock,FIONBIO,&v) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# endif
}


/////////////////////////////////////////////////////////////////////////
unsigned long SocketCollection::scan_interval_sec  = 0;
unsigned long SocketCollection::scan_interval_nsec = 50*1000*1000;
CORBA::ULong  SocketCollection::hashsize           = 103;

/////////////////////////////////////////////////////////////////////////
SocketCollection::SocketCollection() :
  pd_n_fdset_1(0), pd_n_fdset_2(0), pd_n_fdset_dib(0),
  pd_abs_sec(0), pd_abs_nsec(0)
{
  FD_ZERO(&pd_fdset_1);
  FD_ZERO(&pd_fdset_2);
  FD_ZERO(&pd_fdset_dib);

  pd_hash_table = new SocketLink* [hashsize];
  for (CORBA::ULong i=0; i < hashsize; i++)
    pd_hash_table[i] = 0;
}


/////////////////////////////////////////////////////////////////////////
SocketCollection::~SocketCollection() {

  delete [] pd_hash_table;
}


/////////////////////////////////////////////////////////////////////////
void
SocketCollection::setSelectable(SocketHandle_t sock, 
				CORBA::Boolean now,
				CORBA::Boolean data_in_buffer,
				CORBA::Boolean hold_lock) {

  if (!hold_lock) pd_fdset_lock.lock();

  if (data_in_buffer && !FD_ISSET(sock,&pd_fdset_dib)) {
    pd_n_fdset_dib++;
    FD_SET(sock,&pd_fdset_dib);
  }

  if (!FD_ISSET(sock,&pd_fdset_1)) {
    pd_n_fdset_1++;
    FD_SET(sock,&pd_fdset_1);
  }
  if (now) {
    if (!FD_ISSET(sock,&pd_fdset_2)) {
      pd_n_fdset_2++;
      FD_SET(sock,&pd_fdset_2);
    }
    // XXX poke the thread doing accept to look at the fdset immediately.
  }

  if (!hold_lock) pd_fdset_lock.unlock();
}

/////////////////////////////////////////////////////////////////////////
void
SocketCollection::clearSelectable(SocketHandle_t sock) {

  omni_tracedmutex_lock sync(pd_fdset_lock);

  if (FD_ISSET(sock,&pd_fdset_1)) {
    pd_n_fdset_1--;
    FD_CLR(sock,&pd_fdset_1);
  }
  if (FD_ISSET(sock,&pd_fdset_2)) {
    pd_n_fdset_2--;
    FD_CLR(sock,&pd_fdset_2);
  }
  if (FD_ISSET(sock,&pd_fdset_dib)) {
    pd_n_fdset_dib--;
    FD_CLR(sock,&pd_fdset_dib);
  }
}

#ifdef GDB_DEBUG

static
int
do_select(int maxfd,fd_set* r, fd_set* w, fd_set* e, struct timeval* t) {
  return select(maxfd,r,w,e,t);
}

#endif

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
SocketCollection::Select() {

  struct timeval timeout;
  fd_set         rfds;
  int            total;
  
  // (pd_abs_sec,pd_abs_nsec) define the absolute time when we switch fdset
  SocketSetTimeOut(pd_abs_sec,pd_abs_nsec,timeout);

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

  int nready;
  if (fd != 0) {
#ifndef GDB_DEBUG
    nready = select(maxfd+1,&rfds,0,0,&timeout);
#else
    nready = do_select(maxfd+1,&rfds,0,0,&timeout);
#endif
  }
  else {
    omni_thread::sleep(pd_abs_sec,pd_abs_nsec);
    // Alternatively we could block on a conditional variable and be woken up
    // immediately when there is something to monitor.
    // Also, one cannot use select(0,0,0,0,&timeout) because win32 doesn't
    // like it.
    nready = 0; // simulate a timeout
  }

  if (nready == RC_SOCKET_ERROR) {
    if (ERRNO != RC_EINTR) {
      return 0;
    }
    else {
      return 1;
    }
  }

  // Reach here if nready >= 0
  {
    omni_tracedmutex_lock sync(pd_fdset_lock);

    // Process the result from the select.
    SocketHandle_t fd = 0;
    while (nready) {
      if (FD_ISSET(fd,&rfds)) {
	nready--;
	if (FD_ISSET(fd,&pd_fdset_2)) {
	  pd_n_fdset_2--;
	  FD_CLR(fd,&pd_fdset_2);
	  if (FD_ISSET(fd,&pd_fdset_1)) {
	    pd_n_fdset_1--;
	    FD_CLR(fd,&pd_fdset_1);
	  }
	  if (FD_ISSET(fd,&pd_fdset_dib)) {
	    pd_n_fdset_dib--;
	    FD_CLR(fd,&pd_fdset_dib);
	  }
	  if (!notifyReadable(fd)) return 0;
	}
      }
      fd++;
    }

    // Process pd_fdset_dib. Those sockets with their bit set have
    // already got data in buffer. We do a call back for these sockets if
    // their entries in pd_fdset_2 is also set.
    fd = 0;
    nready = pd_n_fdset_dib;
    while (nready) {
      if (FD_ISSET(fd,&pd_fdset_dib)) {
	if (FD_ISSET(fd,&pd_fdset_2)) {
	  pd_n_fdset_2--;
	  FD_CLR(fd,&pd_fdset_2);
	  pd_n_fdset_dib--;
	  FD_CLR(fd,&pd_fdset_dib);
	  if (FD_ISSET(fd,&pd_fdset_1)) {
	    pd_n_fdset_1--;
	    FD_CLR(fd,&pd_fdset_1);
	  }
	  if (!notifyReadable(fd)) return 0;
	}
	nready--;
      }
      fd++;
    }
  }
  return 1;

}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
SocketCollection::Peek(SocketHandle_t sock) {

  {
    omni_tracedmutex_lock sync(pd_fdset_lock);
   
    // Do nothing if this socket is not set to be monitored.
    if (!FD_ISSET(sock,&pd_fdset_1))
      return 0;

    // If data in buffer is set, do callback straight away.
    if (FD_ISSET(sock,&pd_fdset_dib)) {
      if (FD_ISSET(sock,&pd_fdset_1)) {
	pd_n_fdset_1--;
	FD_CLR(sock,&pd_fdset_1);
      }
      if (FD_ISSET(sock,&pd_fdset_2)) {
	pd_n_fdset_2--;
	FD_CLR(sock,&pd_fdset_2);
      }
      pd_n_fdset_dib--;
      FD_CLR(sock,&pd_fdset_dib);
      return 1;
    }
  }

  struct timeval timeout;
  // select on the socket for half the time of scan_interval, if no request
  // arrives in this interval, we just let AcceptAndMonitor take care
  // of it.
  timeout.tv_sec = scan_interval_sec / 2;
  timeout.tv_usec = scan_interval_nsec / 1000 / 2;
  fd_set         rfds;

  do {
    FD_ZERO(&rfds);
    FD_SET(sock,&rfds);
#ifndef GDB_DEBUG
    int nready = select(sock+1,&rfds,0,0,&timeout);
#else
    int nready = do_select(sock+1,&rfds,0,0,&timeout);
#endif

    if (nready == RC_SOCKET_ERROR) {
      if (ERRNO != RC_EINTR) {
	break;
      }
      else {
	continue;
      }
    }

    // Reach here if nready >= 0

    if (FD_ISSET(sock,&rfds)) {
      omni_tracedmutex_lock sync(pd_fdset_lock);

      // Are we still interested?
      if (FD_ISSET(sock,&pd_fdset_1)) {
	if (FD_ISSET(sock,&pd_fdset_2)) {
	  pd_n_fdset_2--;
	  FD_CLR(sock,&pd_fdset_2);
	}
	pd_n_fdset_1--;
	FD_CLR(sock,&pd_fdset_1);
	if (FD_ISSET(sock,&pd_fdset_dib)) {
	  pd_n_fdset_dib--;
	  FD_CLR(sock,&pd_fdset_dib);
	}
	return 1;
      }
    }
    break;

  } while(1);

  return 0;
}


/////////////////////////////////////////////////////////////////////////
void
SocketCollection::addSocket(SocketLink* conn) {

  omni_tracedmutex_lock sync(pd_fdset_lock);
  SocketLink** head = &(pd_hash_table[conn->pd_socket % hashsize]);
  conn->pd_next = *head;
  *head = conn;
}

/////////////////////////////////////////////////////////////////////////
SocketLink*
SocketCollection::removeSocket(SocketHandle_t sock) {

  omni_tracedmutex_lock sync(pd_fdset_lock);
  SocketLink* l = 0;
  SocketLink** head = &(pd_hash_table[sock % hashsize]);
  while (*head) {
    if ((*head)->pd_socket == sock) {
      l = *head;
      *head = (*head)->pd_next;
      break;
    }
    head = &((*head)->pd_next);
  }
  return l;
}

/////////////////////////////////////////////////////////////////////////
SocketLink*
SocketCollection::findSocket(SocketHandle_t sock,
				CORBA::Boolean hold_lock) {

  if (!hold_lock) pd_fdset_lock.lock();

  SocketLink* l = 0;
  SocketLink** head = &(pd_hash_table[sock % hashsize]);
  while (*head) {
    if ((*head)->pd_socket == sock) {
      l = *head;
      break;
    }
    head = &((*head)->pd_next);
  }

  if (!hold_lock) pd_fdset_lock.unlock();

  return l;
}

OMNI_NAMESPACE_END(omni)