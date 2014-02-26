AC_INIT([nss-etcd],[0.1],[ryan@doylenet.net])
# AC_CONFIG_SRCDIR([src/query.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall foreign])

ac_default_prefix="/"

AC_GNU_SOURCE

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# libtool stuff
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h nss.h sys/ioctl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([gethostbyaddr gethostbyname gettimeofday inet_ntoa memset select socket strcspn strdup strerror strncasecmp strcasecmp strspn])



AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
