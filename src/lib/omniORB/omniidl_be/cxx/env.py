# -*- python -*-
#                           Package   : omniidl
# env.py                    Created on: 2000/1/10
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
#   Prebuilds naming environments for each AST node and caches them
#   (they are needed for each pass over the tree so might as well be
#   computed just once and stored)

# $Id$
# $Log$
# Revision 1.1  2000/01/10 15:39:35  djs
# Better name and scope handling.
#
#

"""Prebuilds naming environments for each AST node by walking the tree
   and caching them. Subsequent passes over the tree can then lookup the
   cache rather than redo the calculation"""

from omniidl import idlast, idlvisitor
from omniidl.be.cxx import tyutil, name

import env

env._environments = None

# modules can't emulate sequence types?
def lookup(node):
    try:
        return env._environments[node]
    except KeyError:
        raise RuntimeError("Attempt to lookup node \"" + repr(node) + "\"" +\
                           "in environment (" + str(env._environments) + ")" +\
                           "failed")
        

class WalkTree(idlvisitor.AstVisitor):

    def _enterScope(self, name):
        self._env = self._env.enterScope(name)

    def _leaveScope(self):
        self._env = self._env.leaveScope()

    def _add(self, name, allow_already_exists = 0):
        #print "adding " + name + " to " + str(self._env)
        try:
            new = self._env.copy()
            new.add(name)
        except KeyError:
            if not(allow_already_exists):
                raise RuntimeError("Not permitted to redefine name (\"" + name +\
                                   "\") in environment " + str(self._env))
        self._env = new
        return

    def _cache(self, node):
        if env._environments.has_key(node):
            raise RuntimeError("Unexpected inconsistency: Duplicate key in cache.")
        env._environments[node] = self._env
            

    def __init__(self):
        if env._environments != None:
            raise RuntimeError("Class should only be instantiated once")
        env._environments = {}

        self._env = name.Environment()
        
    
    def visitAST(self, node):
        self._cache(node)
        
        for n in node.declarations():
            n.accept(self)
            
    def visitModule(self, node):
        self._cache(node)
        
        name = node.identifier()
        self._add(name)
        
        self._enterScope(name)
        for n in node.definitions():
            n.accept(self)
        self._leaveScope()
        
    def visitInterface(self, node):
        self._cache(node)

        name = node.identifier()
        self._add(name, allow_already_exists = 1)
        
        self._enterScope(name)
        for n in node.declarations():
            n.accept(self)
        self._leaveScope()
        
    def visitForward(self, node):
        self._cache(node)

        name = node.identifier()
        self._add(name, allow_already_exists = 1)
        
    def visitConst(self, node):
        self._cache(node)

        name = tyutil.name(node.scopedName())
        self._add(name)
        
    def visitDeclarator(self, node):
        self._cache(node)

        name = tyutil.name(node.scopedName())
        self._add(name)
        
    def visitTypedef(self, node):
        self._cache(node)

        for d in node.declarators():
            d.accept(self)
        
    def visitMember(self, node):
        self._cache(node)

        if node.constrType():
            node.memberType().decl().accept(self)
            
        for d in node.declarators():
            d.accept(self)
        
    def visitStruct(self, node):
        self._cache(node)

        name = node.identifier()
        self._add(name)

        self._enterScope(name)
        for n in node.members():
            n.accept(self)
        self._leaveScope()
        
    def visitException(self, node):
        self._cache(node)

        name = node.identifier()
        self._add(name)

        self._enterScope(name)
        for n in node.members():
            n.accept(self)
        self._leaveScope()
        
    def visitUnion(self, node):
        self._cache(node)

        name = node.identifier()
        self._add(name)

    def visitCaseLabel(self, node):
        pass
    def visitUnionCase(self, node):
        pass
    def visitEnumerator(self, node):
        pass
    def visitEnum(self, node):
        self._cache(node)

        name = node.identifier()
        self._add(name)
        
