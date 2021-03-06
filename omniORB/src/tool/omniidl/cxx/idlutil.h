// -*- c++ -*-
//                          Package   : omniidl
// idlutil.h                Created on: 1999/10/11
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 1999 AT&T Laboratories Cambridge
//
//  This file is part of omniidl.
//
//  omniidl is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
//
// Description:
//   
//   Utility functions

// $Id$
// $Log$
// Revision 1.5.2.4  2002/01/15 16:38:15  dpg1
// On the road to autoconf. Dependencies refactored, configure.ac
// written. No makefiles yet.
//
// Revision 1.5.2.3  2000/10/27 16:31:11  dpg1
// Clean up of omniidl dependencies and types, from omni3_develop.
//
// Revision 1.5.2.2  2000/10/10 10:18:51  dpg1
// Update omniidl front-end from omni3_develop.
//
// Revision 1.3.2.1  2000/08/07 15:34:37  dpg1
// Partial back-port of long long from omni3_1_develop.
//
// Revision 1.3  1999/11/09 12:41:40  dpg1
// strcasecmp changes for NT, AIX.
//
// Revision 1.2  1999/11/04 17:16:54  dpg1
// Changes for NT.
//
// Revision 1.1  1999/10/27 14:05:53  dpg1
// *** empty log message ***
//

#ifndef _idlutil_h_
#define _idlutil_h_

#include <assert.h>

#include <idlsysdep.h>

#ifdef HAS_LongLong
typedef IDL_ULongLong IdlIntLiteral;
#else
typedef IDL_ULong IdlIntLiteral;
#endif

#ifdef HAS_LongDouble
typedef IDL_LongDouble IdlFloatLiteral;
#else
typedef IDL_Double IdlFloatLiteral;
#endif

// Version of strdup which uses new
char*      idl_strdup(const char* s);
IDL_WChar* idl_wstrdup(const IDL_WChar* s);

// strlen, strcpy and strcat for wstring
int        idl_wstrlen(const IDL_WChar* s);
IDL_WChar* idl_wstrcpy(IDL_WChar* a, const IDL_WChar* b);
IDL_WChar* idl_wstrcat(IDL_WChar* a, const IDL_WChar* b);

#ifndef HAVE_STRCASECMP
int strcasecmp(const char* s1, const char* s2);
#endif


// Versions of strtoul and strtod which work with the type sizes in use

IdlIntLiteral   idl_strtoul(const char* text, int base);
IdlFloatLiteral idl_strtod (const char* text);


#endif // _idlutil_h_
