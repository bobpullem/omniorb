#
# Usage:
#   nmake /f dir.mk [<build option>]
#
#  <build option>:
#      all       - build all executables
#      clean     - delete all executables and obj files
#      veryclean - clean plus delete all stub files generated by omniidl
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
TOP = ..\..\..\..

##########################################################################
# Essential flags to use omniORB.
#
DIR_CPPFLAGS   = -I. -I$(TOP)\include
#
# omniDynamic410_rt.lib is the runtime DLL to support the CORBA dynamic
# interfaces, such as Anys, typecodes, DSI and DII. In these examples, the
# runtime library is not required as none of these features are used.
# However, a bug in MSVC++ causes it to generate a bunch of references
# to functions in omniDynamic410_rt.lib when compiling the stubs.
# So now we link the dynamic library as well.
# An alternative is to replace the dynamic library with the much smaller 
# library msvcstub.lib. The smaller library contains nothing but stubs
# for the required functions. This is enough when non of the dynamic
# interfaces are used. We use the small library here. If you prefer
# to link with the dynamic library, swap the comment on the next 2
# lines.
#OMNI_DYNAMIC_LIB = omniDynamic410_rt.lib
OMNI_DYNAMIC_LIB = msvcstub.lib -NODEFAULTLIB:libcmt.lib -NODEFAULTLIB:libcmtd.lib 

CORBA_CPPFLAGS = -D__WIN32__ -D_WIN32_WINNT=0x0400 -D__x86__ -D__NT__ \
                 -D__OSVERSION__=4
CORBA_LIB      = omniORB410_rt.lib omnithread32_rt.lib \
                 $(OMNI_DYNAMIC_LIB) \
                 ws2_32.lib mswsock.lib advapi32.lib \
                 -libpath:$(TOP)\lib\x86_win32

CXXFLAGS       = -O2 -MD -GX $(CORBA_CPPFLAGS) $(DIR_CPPFLAGS)
CXXLINKOPTIONS =

.SUFFIXES: .cc
.cc.obj:
  cl /nologo /c $(CXXFLAGS) /Tp$<

########################################################################
# To build debug executables
# Replace the above with the following:
#
#OMNI_DYNAMIC_LIB = omniDynamic410_rtd.lib
#OMNI_DYNAMIC_LIB = msvcstubd.lib -NODEFAULTLIB:libcmt.lib -NODEFAULTLIB:libcmtd.lib 
#CORBA_CPPFLAGS = -D__WIN32__ -D_WIN32_WINNT=0x0400 -D__x86__ -D__NT__ -D__OSVERSION__=4
#CORBA_LIB      = omniORB410_rtd.lib omnithread32_rtd.lib \
#                 $(OMNI_DYNAMIC_LIB) \
#                 ws2_32.lib mswsock.lib advapi32.lib -libpath:$(TOP)\lib\x86_win32
#CXXFLAGS       = -MDd -GX -Z7 -Od  $(CORBA_CPPFLAGS) $(DIR_CPPFLAGS)
#CXXLINKOPTIONS = -debug -PDB:NONE	

all:: eg1.exe

eg1.exe: echoSK.obj eg1.obj
  link -nologo $(CXXLINKOPTIONS) -out:$@ $** $(CORBA_LIB)

clean::
  -del *.obj
  -del *.exe


veryclean::
  -del *.obj
  -del echoSK.* echo.hh
  -del *.exe


echo.hh echoSK.cc: echo.idl
	$(TOP)\bin\x86_win32\omniidl -T -bcxx -Wbh=.hh -Wbs=SK.cc echo.idl

echo.idl: $(TOP)\idl\echo.idl
	copy $(TOP)\idl\echo.idl .
