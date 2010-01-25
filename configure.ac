AC_INIT([fap-balls], [0.1], [Yuri.Borisov@teleca.com])

AM_INIT_AUTOMAKE([-Wall])

AC_CONFIG_FILES([
		 Makefile
		 src/Makefile
		 ])


AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(DBUSGLIB, dbus-glib-1, , [ AC_MSG_ERROR([
You must have 'dbus-glib-1' development headers installed to build.
]) ])

AC_SUBST(DBUSGLIB_CFLAGS)
AC_SUBST(DBUSGLIB_LIBS)


PKG_CHECK_MODULES(LIBOSSO, libosso, , [ AC_MSG_ERROR([
You must have 'libosso' development headers installed to build.
]) ])

AC_SUBST(LIBOSSO_CFLAGS)
AC_SUBST(LIBOSSO_LIBS)


PKG_CHECK_MODULES(HILDON, hildon-1, , [ AC_MSG_ERROR([
You must have 'hildon-1' development headers (hildon-1-dev) installed to build.
]) ])

AC_SUBST(HILDON_CFLAGS)
AC_SUBST(HILDON_LIBS)


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



 
CXXFLAGS="$CFLAGS $DBUSGLIB_CFLAGS $LIBOSSO_CFLAGS $HILDON_CFLAGS $GLIB_CFLAGS $GTKPLUS_CFLAGS $FAPWS_CFLAGS" 
LDFLAGS="$LDFLAGS $DBUSGLIB_LIBS $LIBOSSO_LIBS $HILDON_LIBS $GLIB_LIBS $GTKPLUS_LIBS $FAPWS_LIBS"

AC_OUTPUT

echo Environment variables
echo CXXFLAGS $CXXFLAGS

