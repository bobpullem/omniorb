#
# Usage:
#   nmake /f dir.mk [<build option>]
#
#  <build option>:
#      all       - build all executables
#      clean     - delete all executables and obj files
#      veryclean - clean plus delete all stub files generated by omniidl2
#        
#
# Pre-requisite:
#
# Make sure that you have environment variable LIB and INCLUDE setup for
# using Developer studio from the command line. Usually, this is accomplished
# by source the vcvars32.bat file.
#

# Where is the top of this distribution. All executable, library and include
# directories are relative to this variable.
#
TOP = ..\..\..


##########################################################################
# Essential flags to use omniORB2
#
DIR_CPPFLAGS   = -I. -I$(TOP)\include
#
#
CORBA_CPPFLAGS = -D__WIN32__ -D__x86__ -D__NT__ -D__OSVERSION__=4
CORBA_LIB      = omniDynamic271_rt.lib omniORB271_rt.lib omnithread2_rt.lib \
                 wsock32.lib advapi32.lib \
                 -libpath:$(TOP)\lib\x86_win32
CORBA_LC_LIB   = omniLC22_rt.lib
CXXFLAGS       = -O2 -MD -GX $(CORBA_CPPFLAGS) $(DIR_CPPFLAGS)
CXXLINKOPTIONS =

.SUFFIXES: .cc
.cc.obj:
  cl /nologo /c $(CXXFLAGS) /Tp$<

########################################################################
# To build debug executables
# Replace the above with the following:
#
#CORBA_CPPFLAGS = -D__WIN32__ -D__x86__ -D__NT__ -D__OSVERSION__=4
#CORBA_LIB      = omniDynamic271_rtd.lib omniORB271_rtd.lib \
#                 omnithread2_rtd.lib wsock32.lib \
#                 advapi32.lib -libpath:$(TOP)\lib\x86_win32
#CORBA_LC_LIB   = omniLC22_rtd.lib
#CXXFLAGS       = -MDd -GX -Z7 -Od  $(CORBA_CPPFLAGS) $(DIR_CPPFLAGS)
#CXXLINKOPTIONS = -debug -PDB:NONE	

all:: anyExample_impl.exe anyExample_clt.exe

anyExample_impl.exe: anyExampleSK.obj anyExampleDynSK.obj anyExample_impl.obj
  link -nologo $(CXXLINKOPTIONS) -out:$@ $** $(CORBA_LIB) $(CORBA_LC_LIB)

anyExample_clt.exe: anyExampleSK.obj anyExampleDynSK.obj anyExample_clt.obj
  link -nologo $(CXXLINKOPTIONS) -out:$@ $** $(CORBA_LIB) $(CORBA_LC_LIB)

clean::
  -del *.obj
  -del *.exe


veryclean::
  -del *.obj
  -del anyExampleSK.cc anyExample.hh
  -del *.exe


# Notice the extra -l which instructs the idl compiler to generate
# stubs to support the life cycle service.
#
anyExample.hh anyExampleSK.cc: anyExample.idl
	$(TOP)\bin\x86_win32\omniidl2 -a -h .hh -s SK.cc anyExample.idl
