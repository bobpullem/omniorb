// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslContext.cc              Created on: 29 May 2001
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
  Revision 1.1.2.2  2001/06/20 18:53:34  sll
  Rearrange the declaration of for-loop index variable to work with old and
  standard C++.

  Revision 1.1.2.1  2001/06/11 18:11:06  sll
  *** empty log message ***

*/

#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <omniORB4/CORBA.h>
#include <omniORB4/minorCode.h>
#include <omniORB4/sslContext.h>
#include <exceptiondefs.h>
#include <ssl/sslTransportImpl.h>
#include <openssl/rand.h>
#include <openssl/err.h>

static void report_error();

const char* sslContext::certificate_authority_file = 0;
const char* sslContext::key_file = 0;
const char* sslContext::key_file_password = 0;
sslContext* sslContext::singleton = 0;


/////////////////////////////////////////////////////////////////////////
sslContext::sslContext(const char* cafile,
		       const char* keyfile,
		       const char* password) :
  pd_cafile(cafile), pd_keyfile(keyfile), pd_password(password), pd_ctx(0) {
}

/////////////////////////////////////////////////////////////////////////
sslContext::sslContext() :
  pd_cafile(0), pd_keyfile(0), pd_password(0), pd_ctx(0) {
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::internal_initialise() {

  if (pd_ctx) return;

  set_cipher();
  SSL_load_error_strings();

  pd_ctx = SSL_CTX_new(set_method());
  if (!pd_ctx) {
    report_error();
    OMNIORB_THROW(INITIALIZE,_OMNI_NS(TRANSPORT_ERROR),CORBA::COMPLETED_NO);
  }

  seed_PRNG();
  set_certificate();
  set_privatekey();
  set_CA();
  set_DH();
  set_ephemeralRSA();
}

/////////////////////////////////////////////////////////////////////////
SSL_METHOD*
sslContext::set_method() {
  return SSLv3_method();
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_CA() {

  {
    struct stat buf;
    if (!pd_cafile || stat(pd_cafile,&buf) < 0) {
      omniORB::logger log;
      log << "Error: sslContext CA file is not set "
	  << "or cannot be found\n";
      OMNIORB_THROW(INITIALIZE,_OMNI_NS(TRANSPORT_ERROR),CORBA::COMPLETED_NO);
    }
  }

  if (!(SSL_CTX_load_verify_locations(pd_ctx,pd_cafile,0))) {
    report_error();
    OMNIORB_THROW(INITIALIZE,_OMNI_NS(TRANSPORT_ERROR),CORBA::COMPLETED_NO);
  }

  SSL_CTX_set_verify_depth(pd_ctx,1);

}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_certificate() {
  {
    struct stat buf;
    if (!pd_keyfile || stat(pd_keyfile,&buf) < 0) {
      omniORB::logger log;
      log << "Error: sslContext certificate file is not set "
	  << "or cannot be found\n";
      OMNIORB_THROW(INITIALIZE,_OMNI_NS(TRANSPORT_ERROR),CORBA::COMPLETED_NO);
    }
  }

  if(!(SSL_CTX_use_certificate_file(pd_ctx,pd_keyfile,SSL_FILETYPE_PEM))) {
    report_error();
    OMNIORB_THROW(INITIALIZE,_OMNI_NS(TRANSPORT_ERROR),CORBA::COMPLETED_NO);
  }
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_cipher() {
  OpenSSL_add_all_algorithms();
}

/////////////////////////////////////////////////////////////////////////
static const char* ssl_password = 0;

extern "C" 
int sslContext_password_cb (char *buf,int num,int,void *) {
  int size = strlen(ssl_password);
  if (num < size+1) return 0;
  strcpy(buf,ssl_password);
  return size;
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_privatekey() {

  if (!pd_password) {
    {
      omniORB::logger log;
      log << "Error: sslContext private key file is not set\n";
    }
    OMNIORB_THROW(INITIALIZE,_OMNI_NS(TRANSPORT_ERROR),CORBA::COMPLETED_NO);
  }

  ssl_password = pd_password;
  SSL_CTX_set_default_passwd_cb(pd_ctx,sslContext_password_cb);
  if(!(SSL_CTX_use_PrivateKey_file(pd_ctx,pd_keyfile,SSL_FILETYPE_PEM))) {
    report_error();
    OMNIORB_THROW(INITIALIZE,_OMNI_NS(TRANSPORT_ERROR),CORBA::COMPLETED_NO);
  }
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::seed_PRNG() {
  // Seed the PRNG if it has not been done
  if (!RAND_status()) {

    // This is not necessary on systems with /dev/urandom. Otherwise, the
    // application is strongly adviced to seed the PRNG using one of the
    // seeding functions: RAND_seed(), RAND_add(), RAND_event() or 
    // RAND_screen().
    // What we do here is a last resort and does not necessarily give a very
    // good seed!

    int* data = new int[256];

#if ! defined(__win32__)    
    srand(getuid() + getpid());
#else
    srand(_getpid());
#endif
    int i;
    for(i = 0 ; i < 128 ; ++i)
      data[i] = rand();

    unsigned long abs_sec, abs_nsec;
    omni_thread::get_time(&abs_sec,&abs_nsec);
    srand(abs_sec + abs_nsec);
    for(i = 128 ; i < 256 ; ++i)
      data[i] = rand();

    RAND_seed((unsigned char *)data, (256 * (sizeof(int))));

    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "SSL: the pseudo random number generator has not been seeded.\n"
	  << "A seed is generated but it is not consided to be of crypto strength.\n"
	  << "The application should call one of the OpenSSL seed functions,\n"
	  << "e.g. RAND_event() to initialise the PRNG before calling sslTransportImpl::initiailse().\n";
    }
  }
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_DH() {

  DH* dh = DH_new();
  if(dh == 0) {
    OMNIORB_THROW(INITIALIZE,_OMNI_NS(TRANSPORT_ERROR),CORBA::COMPLETED_NO);
  }

  unsigned char dh512_p[] = {
    0xDA,0x58,0x3C,0x16,0xD9,0x85,0x22,0x89,0xD0,0xE4,0xAF,0x75,
    0x6F,0x4C,0xCA,0x92,0xDD,0x4B,0xE5,0x33,0xB8,0x04,0xFB,0x0F,
    0xED,0x94,0xEF,0x9C,0x8A,0x44,0x03,0xED,0x57,0x46,0x50,0xD3,
    0x69,0x99,0xDB,0x29,0xD7,0x76,0x27,0x6B,0xA2,0xD3,0xD4,0x12,
    0xE2,0x18,0xF4,0xDD,0x1E,0x08,0x4C,0xF6,0xD8,0x00,0x3E,0x7C,
    0x47,0x74,0xE8,0x33
  };

  unsigned char dh512_g[] = {
    0x02
  };

  dh->p = BN_bin2bn(dh512_p, sizeof(dh512_p), 0);
  dh->g = BN_bin2bn(dh512_g, sizeof(dh512_g), 0);
  if( !dh->p || !dh->g) {
    OMNIORB_THROW(INITIALIZE,_OMNI_NS(TRANSPORT_ERROR),CORBA::COMPLETED_NO);
  }

  SSL_CTX_set_tmp_dh(pd_ctx, dh);
  DH_free(dh);
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_ephemeralRSA() {

  RSA *rsa;

  rsa = RSA_generate_key(512,RSA_F4,NULL,NULL);
    
  if (!SSL_CTX_set_tmp_rsa(pd_ctx,rsa)) {
    OMNIORB_THROW(INITIALIZE,_OMNI_NS(TRANSPORT_ERROR),CORBA::COMPLETED_NO);
  }
  RSA_free(rsa);
}


/////////////////////////////////////////////////////////////////////////
static void report_error() {

  if (omniORB::trace(1)) {
    char buf[128];
    ERR_error_string_n(ERR_get_error(),buf,128);
    omniORB::logger log;
    log << "sslContext.cc : " << (const char*) buf << "\n";
  }
}
