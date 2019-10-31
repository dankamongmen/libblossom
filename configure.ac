AC_PREREQ([2.61])
AC_INIT([libblossom], [1.3.0], [dankamongmen@gmail.com], [libblossom], [https://nick-black.com/dankwiki/index.php/Libblossom])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([-Wall foreign dist-xz std-options subdir-objects nostdinc color-tests])
AC_CONFIG_SRCDIR([src/libblossom/blossom.h])

## compiler check
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CC_C99
AC_C_INLINE
AC_C_RESTRICT

AC_GNU_SOURCE

AM_PROG_AR
AM_PROG_CC_C_O

AX_PTHREAD

AX_PROG_XSLTPROC
if test "x$XSLTPROC" = "x" ; then
	AC_MSG_ERROR([Cannot find xsltproc.])
fi
AX_CHECK_DOCBOOK_XSLT

AC_PROG_LIBTOOL

SRCDIR=${srcdir}

## output
AC_SUBST([CFLAGS])
AC_SUBST([SRCDIR])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
