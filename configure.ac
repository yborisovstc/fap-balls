AC_INIT([fap-balls], [0.1], [Yuri.Borisov@teleca.com])

AM_INIT_AUTOMAKE([-Wall])

AC_CONFIG_FILES([
		 Makefile
		 src/Makefile
		 ])


AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL

#CFLAGS="-g -O0"
CFLAGS="-g -O2"

dnl Check for Glib/GConf
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.0.0], , [AC_MSG_ERROR([
You must have the GLib 2.0 and development headers installed to build.
])])

AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)


PKG_CHECK_MODULES(GTKPLUS, gtk+-2.0, , [
    AC_MSG_ERROR([You must have 'gtk+-2.0' dev headers (gtk+-2.0-dev) installed
    ]) ])

AC_SUBST(GTKPLUS_CFLAGS)
AC_SUBST(GTKPLUS_LIBS)


PKG_CHECK_MODULES(FAPWS, libfapws, , [
    AC_MSG_ERROR([You must have 'libfapws' dev headers (libfapws-dev) installed
    ]) ])

AC_SUBST(FAPWS_CFLAGS)
AC_SUBST(FAPWS_LIBS)



 
CXXFLAGS="$CFLAGS $GLIB_CFLAGS $GTKPLUS_CFLAGS $FAPWS_CFLAGS" 
LDFLAGS="$LDFLAGS $GLIB_LIBS $GTKPLUS_LIBS $FAPWS_LIBS"

AC_OUTPUT

echo Environment variables
echo CFLAGS $CFLAGS

