# -*- python -*-
#                           Package   : omniidl
# template.py               Created on: 2000/01/20
#			    Author    : David Scott (djs)
#
#    Copyright (C) 1999 AT&T Laboratories Cambridge
#
#  This file is part of omniidl.
#
#  omniidl is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
#  02111-1307, USA.
#
# Description:
#   
#   C++ templates for the DynSK.cc file

# $Id$
# $Log$
# Revision 1.3.2.14  2005/08/15 11:10:53  dgrisby
# Changes on 22 July broke the stubs for MSVC and maybe other compilers.
#
# Revision 1.3.2.13  2005/07/22 09:52:56  dgrisby
# Remove more gcc warnings. Thanks Matej Kenda.
#
# Revision 1.3.2.12  2001/10/29 17:42:39  dpg1
# Support forward-declared structs/unions, ORB::create_recursive_tc().
#
# Revision 1.3.2.11  2001/08/29 13:41:04  dpg1
# jnw's fix for compilers with variable sizeof(enum)
#
# Revision 1.3.2.10  2001/08/22 13:29:48  dpg1
# Re-entrant Any marshalling.
#
# Revision 1.3.2.9  2001/08/15 10:29:53  dpg1
# Update DSI to use Current, inProcessIdentity.
#
# Revision 1.3.2.8  2001/06/13 17:26:41  sll
# Remove '+' from 'namespace' in the generated code.
#
# Revision 1.3.2.7  2001/06/08 17:12:15  dpg1
# Merge all the bug fixes from omni3_develop.
#
# Revision 1.3.2.6  2001/04/19 09:30:12  sll
#  Big checkin with the brand new internal APIs.
# Scoped where appropriate with the omni namespace.
#
# Revision 1.3.2.5  2001/03/13 10:32:08  dpg1
# Fixed point support.
#
# Revision 1.3.2.4  2000/11/20 14:43:24  sll
# Added support for wchar and wstring.
#
# Revision 1.3.2.3  2000/11/03 19:21:55  sll
# Use include omniORB4 in the generated code.
#
# Revision 1.3.2.2  2000/10/12 15:37:50  sll
# Updated from omni3_1_develop.
#
# Revision 1.4  2000/07/13 15:26:00  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.1.2.10  2000/07/04 12:57:52  djs
# Fixed Any insertion/extraction operators for unions and exceptions
#
# Revision 1.1.2.9  2000/06/30 09:33:05  djs
# Removed more possible nameclashes with user supplied names.
#
# Revision 1.1.2.8  2000/06/27 16:15:10  sll
# New classes: _CORBA_String_element, _CORBA_ObjRef_Element,
# _CORBA_ObjRef_tcDesc_arg to support assignment to an element of a
# sequence of string and a sequence of object reference.
#
# Revision 1.1.2.7  2000/06/26 16:23:27  djs
# Refactoring of configuration state mechanism.
#
# Revision 1.1.2.6  2000/06/05 13:03:05  djs
# Removed union member name clash (x & pd_x, pd__default, pd__d)
# Removed name clash when a sequence is called "pd_seq"
#
# Revision 1.1.2.5  2000/05/31 18:02:51  djs
# Better output indenting (and preprocessor directives now correctly output at
# the beginning of lines)
#
# Revision 1.1.2.4  2000/03/28 18:28:23  djs
# Sequence deletion function used unescaped name "data" which could clash with a
# user identifier
#   eg typedef sequence<char> data
# produces output that could not be parsed.
#
# Revision 1.1.2.3  2000/03/24 16:18:25  djs
# Added missing prefix to CORBA::Any extraction operators used for
#   typedef sequence<X> Y
# (typedef sequence<long> a; would not produce C++ which gcc could parse)
#
# Revision 1.1.2.2  2000/02/16 18:34:49  djs
# Fixed problem generating fragments in DynSK.cc file
#
# Revision 1.1.2.1  2000/02/15 15:36:25  djs
# djr's and jnw's "Super-Hacky Optimisation" patched and added
#
# Revision 1.1  2000/01/20 18:26:45  djs
# Moved large C++ output strings into an external template file
#

"""C++ templates for the DynSK.cc file"""

header_comment = """\
// This file is generated by @program@- @library@. Do not edit.
"""

header = """\
#include "@basename@@hh@"
#include <omniORB4/tcDescriptor.h>

OMNI_USING_NAMESPACE(omni)

static const char* @prefix@_dyn_library_version = @library@;
"""

# Required symbols:
#   @private_prefix@_buildDesc_cstring
# Generated symbols:
#   @private_prefix@_buildDesc_c@n@string
bdesc_string = """\
#ifndef @private_prefix@_buildDesc_c@n@string
#define @private_prefix@_buildDesc_c@n@string @private_prefix@_buildDesc_cstring
#endif
"""

# Required symbols:
#   @private_prefix@_buildDesc_cwstring
# Generated symbols:
#   @private_prefix@_buildDesc_c@n@wstring
bdesc_wstring = """\
#ifndef @private_prefix@_buildDesc_c@n@wstring
#define @private_prefix@_buildDesc_c@n@wstring @private_prefix@_buildDesc_cwstring
#endif
"""

# Required symbols:
#   @private_prefix@_buildDesc_cfixed
# Generated symbols:
#   @private_prefix@_buildDesc_c@digits@_@scale@fixed
bdesc_fixed = """\
#ifndef @private_prefix@_buildDesc_c@digits@_@scale@fixed
#define @private_prefix@_buildDesc_c@digits@_@scale@fixed @private_prefix@_buildDesc_cfixed
#endif
"""

# Required symbols:
#   NONE
# Generated symbols:
#   @private_prefix@_tcParser_getElementDesc@this_cname@
getdesc_array = """\
#ifndef _@private_prefix@_tcParser_getElementDesc@this_cname@__
#define _@private_prefix@_tcParser_getElementDesc@this_cname@__
static CORBA::Boolean
@private_prefix@_tcParser_getElementDesc@this_cname@(const tcArrayDesc* _adesc, CORBA::ULong _index, tcDescriptor &_desc, _CORBA_ULong& _contiguous)
{
  @type@ (&@private_prefix@_tmp)@tail_dims@ = (*((@type@(*)@index_string@)_adesc->opq_array))[_index];
  @builddesc@
  return 1;
}
#endif
"""

# Requried symbols:
#   @private_prefix@_tcParser_getElementDesc@decl_cname@
# Generated symbols:
#   @private_prefix@_buildDesc@decl_cname@
builddesc_array = """\
#ifndef _@private_prefix@_tcParser_buildDesc@decl_cname@__
#define _@private_prefix@_tcParser_buildDesc@decl_cname@__
static void
@private_prefix@_buildDesc@decl_cname@(tcDescriptor& _desc, const @dtype@(*_data)@tail_dims@)
{
  _desc.p_array.getElementDesc = @private_prefix@_tcParser_getElementDesc@decl_cname@;
  _desc.p_array.opq_array = OMNI_CONST_VOID_CAST(_data);
}
#endif
"""

# Required symbols:
#   NONE
# Generated symbols:
#   @private_prefix@_buildDesc@cname@
builddesc_extern = """\
@where@void @private_prefix@_buildDesc@cname@(tcDescriptor &, const @name@&);
"""

# Required symbols:
#   NONE
# Generated symbols:
#   @private_prefix@_buildDesc@cname@
builddesc_forward = """\
void @private_prefix@_buildDesc@cname@(tcDescriptor &, const @name@&);
"""

# Required symbols:
#   NONE
sequence_elementDesc_contiguous = """\
_newdesc.p_streamdata = @sequence@->NP_data();
_contiguous = @sequence@->length() - _index;
"""

# Required symbols:
#   @private_prefix@_buildDesc@thing_cname@
sequence_elementDesc_noncontiguous = """\
@private_prefix@_buildDesc@thing_cname@(_newdesc, @thing@);
"""

# Required symbols:
#   NONE
# Generated symbolc:
#   @private_prefix@_tcParser_setElementCount@cname@
#   @private_prefix@_tcParser_getElementCount@cname@
#   @private_prefix@_tcParser_getElementDesc@cname@
anon_sequence = """\
#ifndef _@private_prefix@_tcParser_buildDesc@cname@__
#define _@private_prefix@_tcParser_buildDesc@cname@__
static void
@private_prefix@_tcParser_setElementCount@cname@(const tcSequenceDesc* _desc, CORBA::ULong _len)
{
  ((@sequence_template@*)_desc->opq_seq)->length(_len);
}

static CORBA::ULong
@private_prefix@_tcParser_getElementCount@cname@(const tcSequenceDesc* _desc)
{
  return ((@sequence_template@*)_desc->opq_seq)->length();
}

static CORBA::Boolean
@private_prefix@_tcParser_getElementDesc@cname@(const tcSequenceDesc* _desc, CORBA::ULong _index, tcDescriptor& _newdesc, _CORBA_ULong& _contiguous)
{
  @elementDesc@
  return 1;
}

static void
@private_prefix@_buildDesc@cname@(tcDescriptor &_desc, const @sequence_template@& _data)
{
  _desc.p_sequence.opq_seq = OMNI_CONST_VOID_CAST(&_data);
  _desc.p_sequence.setElementCount =
    @private_prefix@_tcParser_setElementCount@cname@;
  _desc.p_sequence.getElementCount =
    @private_prefix@_tcParser_getElementCount@cname@;
  _desc.p_sequence.getElementDesc =
    @private_prefix@_tcParser_getElementDesc@cname@;
  }
#endif
"""

# Required symbols:
#   @private_prefix@_tcParser_getMemberDesc_@guard_name@ 
# Generated symbols:
#   @private_prefix@_tcParser_getMemberCount_@guard_name@
#   @private_prefix@_buildDesc_c@guard_name@
builddesc_member = """\
@private_prefix@_tcParser_getMemberCount_@guard_name@(const tcStructDesc *_desc)
{
  return @num_members@;
}

void @private_prefix@_buildDesc_c@guard_name@(tcDescriptor &_desc, const @fqname@& _data)
{
  _desc.p_struct.getMemberDesc = @private_prefix@_tcParser_getMemberDesc_@guard_name@;
  _desc.p_struct.getMemberCount = @private_prefix@_tcParser_getMemberCount_@guard_name@;
  _desc.p_struct.opq_struct = OMNI_CONST_VOID_CAST(&_data);
}
"""

# Required symbols:
#   @private_prefix@_tcParser_setObjectPtr_@guard_name@
#   @private_prefix@_tcParser_getObjectPtr_@guard_name@
#   @private_prefix@_buildDesc_c@guard_name@
# Generated symbols:
#   @private_prefix@_tcParser_setObjectPtr_@guard_name@
#   @private_prefix@_tcParser_getObjectPtr_@guard_name@
#   @private_prefix@_buildDesc_c@guard_name@
#   @private_prefix@_delete_@guard_name@
interface = """\
static void
@private_prefix@_tcParser_setObjectPtr_@guard_name@(const tcObjrefDesc *_desc, CORBA::Object_ptr _ptr)
{
  @fqname@_ptr _p = @fqname@::_narrow(_ptr);
  @fqname@_ptr* pp = (@fqname@_ptr*)_desc->opq_objref;
  if (_desc->opq_release && !CORBA::is_nil(*pp)) CORBA::release(*pp);
  *pp = _p;
  CORBA::release(_ptr);
}

static CORBA::Object_ptr
@private_prefix@_tcParser_getObjectPtr_@guard_name@(const tcObjrefDesc *_desc)
{
  return (CORBA::Object_ptr) *((@fqname@_ptr*)_desc->opq_objref);
}

void @private_prefix@_buildDesc_c@guard_name@(tcDescriptor& _desc, const @objref_member@& _d)
{
  _desc.p_objref.opq_objref = OMNI_CONST_VOID_CAST(&_d._data);
  _desc.p_objref.opq_release = _d._rel;
  _desc.p_objref.setObjectPtr = @private_prefix@_tcParser_setObjectPtr_@guard_name@;
  _desc.p_objref.getObjectPtr = @private_prefix@_tcParser_getObjectPtr_@guard_name@;
}

void @private_prefix@_delete_@guard_name@(void* _data) {
  CORBA::release((@fqname@_ptr) _data);
}

void operator<<=(CORBA::Any& _a, @fqname@_ptr _s) {
  tcDescriptor tcd;
  @objref_member@ tmp(_s,0);
  @private_prefix@_buildDesc_c@guard_name@(tcd, tmp);
  _a.PR_packFrom(@tc_name@, &tcd);
}

void operator<<=(CORBA::Any& _a, @fqname@_ptr* _sp) {
  _a <<= *_sp;
  CORBA::release(*_sp);
  *_sp = @fqname@::_nil();
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, @fqname@_ptr& _s) {
  @fqname@_ptr sp = (@fqname@_ptr) _a.PR_getCachedData();
  if (sp == 0) {
    tcDescriptor tcd;
    @fqname@_var tmp;
    @private_prefix@_buildDesc_c@guard_name@(tcd, tmp);
    if( _a.PR_unpackTo(@tc_name@, &tcd) ) {
      if (!omniORB::omniORB_27_CompatibleAnyExtraction) {
        (OMNI_CONST_CAST(CORBA::Any*, &_a))->PR_setCachedData((void*)(@fqname@_ptr)tmp,@private_prefix@_delete_@guard_name@);
      }
      _s = tmp._retn();
      return 1;
    } else {
      _s = @fqname@::_nil(); return 0;
    }
  }
  else {
    CORBA::TypeCode_var tc = _a.type();
    if (tc->equivalent(@tc_name@)) {
    _s = sp; return 1;
    }
    else {
    _s = @fqname@::_nil(); return 0;
    }
  }
}
"""

# Required symbols:
#   NONE
# Generated symbols:
#   @private_prefix@_delete_@guard_name@
typedef_array_decl_delete = """\
void @private_prefix@_delete_@guard_name@(void* _data) {
  @fqname@_slice* _0RL_t = (@fqname@_slice*) _data;
  @fqname@_free(_0RL_t);
}
"""

# Required symbols:
#   @private_prefix@_buildDesc@decl_cname@
# Generated symbols:
#   NONE
typedef_array_decl_oper = """\
void operator<<=(CORBA::Any& _a, const @fqname@_forany& _s) {
  @fqname@_slice* @private_prefix@_s = _s.NP_getSlice();
  tcDescriptor @private_prefix@_tcdesc;
  @private_prefix@_buildDesc@decl_cname@(@private_prefix@_tcdesc, (const @dtype@(*)@tail_dims@)(@dtype@(*)@tail_dims@)(@private_prefix@_s));
  _a.PR_packFrom(@tcname@, &@private_prefix@_tcdesc);
  if( _s.NP_nocopy() ) {
    delete[] @private_prefix@_s;
  }
}
CORBA::Boolean operator>>=(const CORBA::Any& _a, @fqname@_forany& _s) {
  @fqname@_slice* @private_prefix@_s = (@fqname@_slice*) _a.PR_getCachedData();
  if( !@private_prefix@_s ) {
    @private_prefix@_s = @fqname@_alloc();
    tcDescriptor @private_prefix@_tcdesc;
    @private_prefix@_buildDesc@decl_cname@(@private_prefix@_tcdesc, (const @dtype@(*)@tail_dims@)(@dtype@(*)@tail_dims@)(@private_prefix@_s));
    if( !_a.PR_unpackTo(@tcname@, &@private_prefix@_tcdesc) ) {
      delete[] @private_prefix@_s;
      _s = 0;
      return 0;
    }
    (OMNI_CONST_CAST(CORBA::Any*, &_a))->PR_setCachedData(@private_prefix@_s, @private_prefix@_delete_@guard_name@);
  } else {
    CORBA::TypeCode_var @private_prefix@_tc = _a.type();
    if( !@private_prefix@_tc->equivalent(@tcname@) ) {
      _s = 0;
      return 0;
    }
  }
  _s = @private_prefix@_s;
  return 1;
}
"""

# Required symbols:
#   @private_prefix@_buildDesc@decl_cname@
# Generated symbols:
#   NONE
# (Note the non-copying operator is inlined in the .hh file)
typedef_sequence_oper = """\
void operator <<= (CORBA::Any& _a, const @fqname@& _s)
{
  tcDescriptor tcdesc;
  @private_prefix@_buildDesc@decl_cname@(tcdesc, _s);
  _a.PR_packFrom(@tcname@, &tcdesc);
}

void @private_prefix@_seq_delete_@guard_name@(void* _data)
{
  delete (@fqname@*)_data;
}

CORBA::Boolean operator >>= (const CORBA::Any& _a, @fqname@*& _s_out)
{
  return _a >>= (const @fqname@*&) _s_out;
}

CORBA::Boolean operator >>= (const CORBA::Any& _a, const @fqname@*& _s_out)
{
  _s_out = 0;
  @fqname@* stmp = (@fqname@*) _a.PR_getCachedData();
  if( stmp == 0 ) {
    tcDescriptor tcdesc;
    stmp = new @fqname@;
    @private_prefix@_buildDesc@decl_cname@(tcdesc, *stmp);
    if( _a.PR_unpackTo(@tcname@, &tcdesc)) {
      (OMNI_CONST_CAST(CORBA::Any*, &_a))->PR_setCachedData(OMNI_CONST_VOID_CAST(stmp), @private_prefix@_seq_delete_@guard_name@);
      _s_out = stmp;
      return 1;
    } else {
      delete OMNI_CONST_CAST(@fqname@*,stmp);
      return 0;
    }
  } else {
    CORBA::TypeCode_var tctmp = _a.type();
    if( tctmp->equivalent(@tcname@) ) {
      _s_out = stmp;
      return 1;
    } else {
      return 0;
    }
  }
}
"""

# Required symbols:
#   NONE
# Generated symbols:
#   @private_prefix@_buildDesc_c@guard_name@
enum = """\
void @private_prefix@_buildDesc_c@guard_name@(tcDescriptor& _desc, const @fqname@& _data)
{
  _desc.p_enum.data = OMNI_CONST_VOID_CAST(&_data);
  _desc.p_enum.size = sizeof(_data);
}

void operator<<=(CORBA::Any& _a, @fqname@ _s)
{
  tcDescriptor @private_prefix@_tcd;
  @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcd, _s);
  _a.PR_packFrom(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcd);
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, @fqname@& _s)
{
  tcDescriptor @private_prefix@_tcd;
  @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcd, _s);
  return _a.PR_unpackTo(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcd);
}
"""

# Required symbols:
#   @private_prefix@_buildDesc_c@guard_name@
# Generated symbols:
#   @private_prefix@_delete_@guard_name@
struct = """\
void @private_prefix@_delete_@guard_name@(void* _data) {
  @fqname@* @private_prefix@_t = (@fqname@*) _data;
  delete @private_prefix@_t;
}

@member_desc@

void operator<<=(CORBA::Any& _a, const @fqname@& _s) {
  tcDescriptor @private_prefix@_tcdesc;
  @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, _s);
  _a.PR_packFrom(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc);
}
 
void operator<<=(CORBA::Any& _a, @fqname@* _sp) {
  tcDescriptor @private_prefix@_tcdesc;
  @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, *_sp);
  _a.PR_packFrom(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc);
  delete _sp;
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, @fqname@*& _sp) {
  return _a >>= (const @fqname@*&) _sp;
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, const @fqname@*& _sp) {
  _sp = (@fqname@ *) _a.PR_getCachedData();
  if (_sp == 0) {
    tcDescriptor @private_prefix@_tcdesc;
    _sp = new @fqname@;
    @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, *_sp);
    if (_a.PR_unpackTo(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc)) {
      (OMNI_CONST_CAST(CORBA::Any *, &_a))->PR_setCachedData(OMNI_CONST_VOID_CAST(_sp), @private_prefix@_delete_@guard_name@);
      return 1;
    } else {
      delete OMNI_CONST_CAST(@fqname@*,_sp); _sp = 0;
      return 0;
    }
  } else {
    CORBA::TypeCode_var @private_prefix@_tctmp = _a.type();
    if (@private_prefix@_tctmp->equivalent(@private_prefix@_tc_@guard_name@)) return 1;
    _sp = 0;
    return 0;
  }
}"""

# Required symbols:
#   @private_prefix@_buildDesc@discrim_cname@
# Generated symbols:
#   @private_prefix@_tcParser_unionhelper_@guard_name@
union_tcParser = """\
class @private_prefix@_tcParser_unionhelper_@guard_name@ {
public:
  static void getDiscriminator(const tcUnionDesc* _desc, tcDescriptor& _newdesc, CORBA::PR_unionDiscriminator& _discrim) {
    @fqname@* _u = (@fqname@*)_desc->opq_union;
    @private_prefix@_buildDesc@discrim_cname@(_newdesc, _u->_pd__d);
    _discrim = (CORBA::PR_unionDiscriminator)_u->_pd__d;
  }

  static void setDiscriminator(const tcUnionDesc* _desc, CORBA::PR_unionDiscriminator _discrim, int _is_default) {
    @fqname@* _u = (@fqname@*)_desc->opq_union;
    _u->_pd__d = (@discrim_type@)_discrim;
    _u->_pd__default = _is_default;
  }

  static CORBA::Boolean getValueDesc(const tcUnionDesc* _desc, tcDescriptor& _newdesc) {
    @fqname@* _u = (@fqname@*)_desc->opq_union;
    @switch@
    return 1;
  }
};
"""

# Required symbols:
#   @private_prefix@_tcParser_unionhelper_@guard_name@
#   @private_prefix@_tcParser_unionhelper_@guard_name@
#   @private_prefix@_tcParser_unionhelper_@guard_name@
# Generated symbols:
#   @private_prefix@_buildDesc_c@guard_name@
#   @private_prefix@_delete_@guard_name@
union = """\
void @private_prefix@_buildDesc_c@guard_name@(tcDescriptor& _desc, const @fqname@& _data)
{
  _desc.p_union.getDiscriminator = @private_prefix@_tcParser_unionhelper_@guard_name@::getDiscriminator;
  _desc.p_union.setDiscriminator = @private_prefix@_tcParser_unionhelper_@guard_name@::setDiscriminator;
  _desc.p_union.getValueDesc = @private_prefix@_tcParser_unionhelper_@guard_name@::getValueDesc;
  _desc.p_union.opq_union = OMNI_CONST_VOID_CAST(&_data);
}

void @private_prefix@_delete_@guard_name@(void* _data)
{
  @fqname@* @private_prefix@_t = (@fqname@*) _data;
  delete @private_prefix@_t;
}

void operator<<=(CORBA::Any& _a, const @fqname@& _s)
{
  tcDescriptor @private_prefix@_tcdesc;
  @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, _s);
  _a.PR_packFrom(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc);
}

void operator<<=(CORBA::Any& _a, @fqname@* _sp)
{
  tcDescriptor @private_prefix@_tcdesc;
  @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, *_sp);
  _a.PR_packFrom(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc);
  delete _sp;
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, @fqname@*& _sp) {
  return _a >>= (const @fqname@*&) _sp;
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, const @fqname@*& _sp) {
  _sp = (@fqname@ *) _a.PR_getCachedData();
  if (_sp == 0) {
    tcDescriptor @private_prefix@_tcdesc;
    _sp = new @fqname@;
    @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, *_sp);
    if( _a.PR_unpackTo(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc) ) {
      (OMNI_CONST_CAST(CORBA::Any*, &_a))->PR_setCachedData(OMNI_CONST_VOID_CAST(_sp), @private_prefix@_delete_@guard_name@);
      return 1;
    } else {
      delete OMNI_CONST_CAST(@fqname@*,_sp);
      _sp = 0;
      return 0;
    }
  } else {
    CORBA::TypeCode_var @private_prefix@_tctmp = _a.type();
    if (@private_prefix@_tctmp->equivalent(@private_prefix@_tc_@guard_name@)) return 1;
    _sp = 0;
    return 0;
  }
}
"""

# Required symbols:
#   @private_prefix@_buildDesc_c@guard_name@
# Generated symbols:
#   @private_prefix@_delete_@guard_name   
exception = """\
void @private_prefix@_delete_@guard_name@(void* _data) {
  @fqname@* @private_prefix@_t = (@fqname@*) _data;
  delete @private_prefix@_t;
}

void operator<<=(CORBA::Any& _a, const @fqname@& _s) {
  tcDescriptor _0RL_tcdesc;
  @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, _s);
  _a.PR_packFrom(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc);
}

void operator<<=(CORBA::Any& _a, const @fqname@* _sp) {
  tcDescriptor @private_prefix@_tcdesc;
  @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, *_sp);
  _a.PR_packFrom(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc);
  delete OMNI_CONST_CAST(@fqname@*,_sp);
}

CORBA::Boolean operator>>=(const CORBA::Any& _a,const @fqname@*& _sp) {
  _sp = (@fqname@ *) _a.PR_getCachedData();
  if (_sp == 0) {
    tcDescriptor @private_prefix@_tcdesc;
    _sp = new @fqname@;
    @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, *_sp);
    if (_a.PR_unpackTo(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc)) {
      (OMNI_CONST_CAST(CORBA::Any *, &_a))->PR_setCachedData(OMNI_CONST_VOID_CAST(_sp), @private_prefix@_delete_@guard_name@);
      return 1;
    } else {
      delete OMNI_CONST_CAST(@fqname@*,_sp); _sp = 0;
      return 0;
    }
  } else {
    CORBA::TypeCode_var @private_prefix@_tctmp = _a.type();
    if (@private_prefix@_tctmp->equivalent(@private_prefix@_tc_@guard_name@)) return 1;
    delete OMNI_CONST_CAST(@fqname@*,_sp); _sp = 0;
    return 0;
  }
}

static void @private_prefix@_insertToAny__c@guard_name@(CORBA::Any& _a,const CORBA::Exception& _e) {
  const @fqname@ & _ex = (const @fqname@ &) _e;
  operator<<=(_a,_ex);
}

static void @private_prefix@_insertToAnyNCP__c@guard_name@ (CORBA::Any& _a,const CORBA::Exception* _e) {
  const @fqname@ * _ex = (const @fqname@ *) _e;
  operator<<=(_a,_ex);
}

class @private_prefix@_insertToAny_Singleton__c@guard_name@ {
public:
  @private_prefix@_insertToAny_Singleton__c@guard_name@() {
    @fqname@::insertToAnyFn = @private_prefix@_insertToAny__c@guard_name@;
    @fqname@::insertToAnyFnNCP = @private_prefix@_insertToAnyNCP__c@guard_name@;
  }
};
static @private_prefix@_insertToAny_Singleton__c@guard_name@ @private_prefix@_insertToAny_Singleton__c@guard_name@_;
"""

## TypeCode generation
##
tc_string = """\
#if !defined(___tc_string_@n@_value__) && !defined(DISABLE_Unnamed_Bounded_String_TC)
#define ___tc_string_@n@_value__
const CORBA::TypeCode_ptr _tc_string_@n@ = CORBA::TypeCode::PR_string_tc(@n@);
#endif
"""

tc_wstring = """\
#if !defined(___tc_wstring_@n@_value__) && !defined(DISABLE_Unnamed_Bounded_WString_TC)
#define ___tc_wstring_@n@_value__
const CORBA::TypeCode_ptr _tc_wstring_@n@ = CORBA::TypeCode::PR_wstring_tc(@n@);
#endif
"""

external_linkage = """\
#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the constant external linkage otherwise.
@open_namespace@
  const CORBA::TypeCode_ptr @tc_unscoped_name@ = @mangled_name@;
@close_namespace@
#else
const CORBA::TypeCode_ptr @tc_name@ = @mangled_name@;
#endif
"""
