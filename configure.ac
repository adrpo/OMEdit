dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([OMOptim],[dev],[https://trac.openmodelica.org/OpenModelica],[openmodelica],[https://openmodelica.org])

AC_SUBST(APP)
AC_SUBST(EXE)
AC_SUBST(IDLCMD)
AC_SUBST(RPATH_QMAKE)
AC_SUBST(host_short)
AC_SUBST(SOURCE_REVISION)

m4_include([common/m4/pre-commit.m4])
cp common/install-sh common/config.guess common/config.sub ./
m4_include([common/m4/ombuilddir.m4])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

dnl Checks for programs.
AC_LANG([C++])
AC_PROG_CC
AC_PROG_CXX

host_short=$host_cpu-$host_os

m4_include([common/m4/qmake.m4])

m4_include([common/m4/omhome.m4])
FIND_OPENMODELICAHOME()
FIND_LIBOPENMODELICACOMPILER()
FIND_LIBOMQWT()

m4_include([common/m4/semver.m4])
SOURCE_REVISION="$SOURCE_REVISION$NON_FREE_VERSION"

AC_OUTPUT(Makefile OMEdit/OMEditGUI/Makefile.unix OMEdit/OMEditGUI/OMEdit.config OMEdit/OMEditGUI/Debugger/Parser/Makefile.unix OMEdit/OMEditGUI/Debugger/Parser/Makefile.lib.unix OMEdit/OMEditGUI/omc_config.h)
