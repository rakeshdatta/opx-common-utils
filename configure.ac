#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([opx-common-utils], [1.5.0+opx1], [ops-dev@lists.openswitch.net])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_SRCDIR([.])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
LT_INIT([shared])

# Checks for libraries.
AC_CHECK_LIB([dl], [dlopen])
AC_CHECK_LIB([event_log], [event_log_msg])
AC_CHECK_LIB([pthread], [pthread_self])
AC_CHECK_LIB([xml2], [xmlInitParser])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_MALLOC
AC_CHECK_FUNCS([clock_gettime dup2 memmove memset select socket strcasecmp strchr strtoul])

AC_CONFIG_FILES([Makefile inc/Makefile])
AC_OUTPUT
