// -*- Mode: C++; -*-
//                            Package   : omniORB
// objectStub.cc              Created on: 04/09/2000
//                            Author    : Sai-Lai Lo (sll)
//
//    Copyright (C) 2000 AT&T Research Cambridge
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
//

/*
  $Log$
  Revision 1.1.2.2  2000/11/03 19:12:07  sll
  Use new marshalling functions for byte, octet and char. Use get_octet_array
  instead of get_char_array and put_octet_array instead of put_char_array.

  Revision 1.1.2.1  2000/09/27 17:30:30  sll
  *** empty log message ***

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/callDescriptor.h>
#include <objectStub.h>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void
omni_is_a_CallDesc::marshalArguments(cdrStream& s)
{
  s.marshalString(a_1);
}

void
omni_is_a_CallDesc::unmarshalReturnedValues(cdrStream& s)
{
  result = s.unmarshalBoolean();
}

void
omni_is_a_CallDesc::unmarshalArguments(cdrStream& s)
{
  a_1 = s.unmarshalString();
}

void
omni_is_a_CallDesc::marshalReturnedValues(cdrStream& s)
{
  s.marshalBoolean(result);
}

void
omni_is_a_CallDesc::lcfn(omniCallDescriptor* cd, omniServant* servant)
{
  omni_is_a_CallDesc* tcd = (omni_is_a_CallDesc*) cd;

  tcd->result = servant->_is_a(tcd->a_1);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void
omni_non_existent_CallDesc::unmarshalReturnedValues(cdrStream& s)
{
  result = s.unmarshalBoolean();
}

void
omni_non_existent_CallDesc::marshalReturnedValues(cdrStream& s)
{
  s.marshalBoolean(result);
}


void
omni_non_existent_CallDesc::lcfn(omniCallDescriptor* cd, omniServant* servant)
{
  ((omni_non_existent_CallDesc*) cd)->result = servant->_non_existent();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void
omni_interface_CallDesc::unmarshalReturnedValues(cdrStream& s)
{
  pd_result = CORBA::Object::_unmarshalObjRef(s);
}

void
omni_interface_CallDesc::marshalReturnedValues(cdrStream& s)
{
  CORBA::Object::_marshalObjRef(pd_result,s);
}

void
omni_interface_CallDesc::lcfn(omniCallDescriptor* cd, omniServant* servant)
{
  omniObjRef* intf = servant->_do_get_interface();
  ((omni_interface_CallDesc*) cd)->pd_result = (CORBA::Object_ptr)intf->_ptrToObjRef(CORBA::Object::_PD_repoId);
}

