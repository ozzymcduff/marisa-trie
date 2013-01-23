#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([marisa], [0.2.2], [syata@acm.org])
AC_CONFIG_SRCDIR([lib/marisa.h])
AM_INIT_AUTOMAKE

# Checks for programs.
LT_INIT
AC_PROG_CXX
AC_PROG_INSTALL

AC_CONFIG_MACRO_DIR([m4])

# Checks for SSE availability.
AC_MSG_CHECKING([whether to use SSE2])
AC_ARG_ENABLE([sse2],
              [AS_HELP_STRING([--enable-sse2],
                              [use SSE2 [default=no]])],
              [],
              [enable_sse2="no"])
AS_IF([test "x${enable_sse2}" != "xno"], [
  enable_sse2="yes"
])
AC_MSG_RESULT([${enable_sse2}])

AC_MSG_CHECKING([whether to use SSSE3])
AC_ARG_ENABLE([ssse3],
              [AS_HELP_STRING([--enable-ssse3],
                              [use SSSE3 [default=no]])],
              [],
              [enable_ssse3="no"])
AS_IF([test "x${enable_ssse3}" != "xno"], [
  enable_sse2="yes"
  enable_ssse3="yes"
])
AC_MSG_RESULT([${enable_ssse3}])

AC_MSG_CHECKING([whether to use SSE4.2])
AC_ARG_ENABLE([sse4_2],
              [AS_HELP_STRING([--enable-sse4_2],
                              [use SSE4.2 [default=no]])],
              [],
              [enable_sse4_2="no"])
AC_ARG_ENABLE([popcnt],
              [AS_HELP_STRING([--enable-popcnt],
                              [use popcnt (SSE4.2) [default=no]])],
              [],
              [enable_popcnt="no"])
AS_IF([test "x${enable_sse4_2}" != "xno" -o "x${enable_popcnt}" != "xno"], [
  enable_sse2="yes"
  enable_ssse3="yes"
  enable_sse4_2="yes"
  enable_popcnt="yes"
])
AC_MSG_RESULT([${enable_sse4_2}])

AS_IF([test "x${enable_sse2}" != "xno"], [
  CXXFLAGS="$CXXFLAGS -DMARISA_USE_SSE2 -msse2"
])
AS_IF([test "x${enable_ssse3}" != "xno"], [
  CXXFLAGS="$CXXFLAGS -DMARISA_USE_SSSE3 -mssse3"
])
AS_IF([test "x${enable_sse4_2}" != "xno"], [
  CXXFLAGS="$CXXFLAGS -DMARISA_USE_SSE4_2 -msse4.2"
])

AC_CONFIG_FILES([Makefile
                 marisa.pc
                 lib/Makefile
                 tests/Makefile
                 tools/Makefile])
AC_OUTPUT

AS_ECHO([])
AS_ECHO(["${PACKAGE_NAME} ${PACKAGE_VERSION} configuration:"])
AS_ECHO(["-------------------------------"])
AS_ECHO(["  HOST:      ${host}"])
AS_ECHO(["  CXX:       ${CXX}"])
AS_ECHO(["  CXXFLAGS:  ${AM_CXXFLAGS} ${CXXFLAGS}"])
AS_ECHO(["  LDFLAGS:   ${AM_LDFLAGS} ${LDFLAGS}"])
AS_ECHO(["  PREFIX:    ${prefix}"])
AS_ECHO([])
AS_ECHO(["  SSE2:      ${enable_sse2}"])
AS_ECHO(["  SSSE3:     ${enable_ssse3}"])
AS_ECHO(["  SSE4.2:    ${enable_sse4_2}"])
AS_ECHO([])
