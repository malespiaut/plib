dnl Process this file with autoconf to produce a configure script.

AC_INIT([ttt3d], [0.2.0])
AC_CONFIG_SRCDIR([src/ttt3d.cxx])
AM_INIT_AUTOMAKE([1.16])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL

plib_suffix="-lplibjs -lplibsl -lplibssgaux -lplibssg -lplibpu -lplibfnt -lplibsg -lplibul"

AC_LANG([C++])

AC_CHECK_HEADERS([unistd.h])
AC_PREREQ([2.71])

dnl List files of a given extension in a given directory
file_list() {
  for f in $top_srcdir/$1/*.$2; do
    echo $(basename $f)
  done
}

dnl # Create the list of files
AC_SUBST([DAT_FILES], [$(file_list data dat)])
AC_SUBST([HTML_FILES], [$(file_list doc html)])
AC_SUBST([TXF_FILES], [$(file_list fonts txf)])
AC_SUBST([RGB_FILES], [$(file_list images rgb)])
AC_SUBST([MOD_FILES], [$(file_list mods mod)])
AC_SUBST([WAV_FILES], [$(file_list wavs wav)])

dnl Command line arguments.

dnl Don't use AS_HELP_STRING here as old versions of autoconf do not
dnl understand it.
AC_ARG_WITH(GL,
[  --with-GL=DIR           set the prefix directory where GL resides],
GL_PREFIX=$withval, GL_PREFIX=auto)

if test "x$GL_PREFIX" != "xauto"; then
    LDFLAGS="$LDFLAGS -L$GL_PREFIX/lib"
    CPPFLAGS="$CPPFLAGS -I$GL_PREFIX/include"
fi


dnl Checks for library functions.
dnl check for OpenGL related libraries

AC_CANONICAL_HOST

case "${host}" in
*-*-cygwin* | *-*-mingw32*)
    dnl CygWin under Windoze.

    LIBS="$LIBS -lwinmm -lglut32 -lglu32 -lopengl32 -luser32 -lgdi32"
	;;
*-apple-darwin*)
    dnl Mac OS X

    LIBS="$LIBS -framework GLUT -lobjc -framework OpenGL -framework Carbon"
	;;
*)
    dnl Check for X11

    AC_PATH_XTRA

    LDFLAGS="$LDFLAGS $X_LIBS"

    LIBS="$LIBS $X_PRE_LIBS -lXi -lXmu -lXext -lX11 $X_EXTRA_LIBS -lm"

dnl =========================================================
dnl    if test "x$x_includes" != "x"; then
dnl       CPPFLAGS="$CPPFLAGS -I$x_includes"
dnl    fi
dnl =========================================================

    dnl Reasonable stuff non-windoze variants ... :-)

    AC_CHECK_LIB(GL, glNewList)
    if test "x$ac_cv_lib_GL_glNewList" = "xno" ; then
        dnl if no GL, check for MesaGL
        AC_CHECK_LIB(MesaGL, glNewList,,
        AC_MSG_ERROR([could not find working GL library]))
    fi

    AC_CHECK_LIB(GLU, gluLookAt)
    if test "x$ac_cv_lib_GLU_gluLookAt" = "xno" ; then
        dnl if no GLU, check for MesaGLU
        AC_CHECK_LIB(MesaGLU, gluLookAt,,
        AC_MSG_ERROR([could not find working GLU library]))
    fi

    dnl check for glut...or freeglut
    AC_CHECK_LIB(freeglut, glutGetModifiers)
    if test "x$ac_cv_lib_freeglut_glutGetModifiers" = "xno" ; then
        dnl if no freeglut, check for Real GLUT
        AC_CHECK_LIB(glut, glutGetModifiers,,
        AC_MSG_ERROR([could not find working GLUT library]))
    fi

    AC_CHECK_LIB(dl, dlclose)

    dnl Check SGI audio library
    AC_CHECK_LIB(audio, ALopenport, SND_LIBS="-laudio")
	;;
esac

dnl Check PLIB library
AC_CHECK_LIB(plibul, ulInit, true,
AC_MSG_ERROR([there seems to be a problem with the PLIB libraries]))

LIBS="$plib_suffix $LIBS"


dnl Checks for header files.

AC_CHECK_HEADER(windows.h, AC_DEFINE(WIN32))

case "${host}" in
*-*-cygwin* | *-*-mingw32*)
    dnl CygWin under Windoze.

    AC_CHECK_HEADERS(GL/gl.h GL/glu.h,,
    AC_MSG_ERROR([OpenGL header files not found]))

    AC_CHECK_HEADER(GL/glut.h, AC_DEFINE(GLUT_IS_PRESENT),
    AC_MSG_ERROR([GLUT header file not found]))
	;;
*-apple-darwin*)
    dnl Mac OS X

    AC_CHECK_HEADERS(OpenGL/gl.h OpenGL/glu.h,,
    AC_MSG_ERROR([OpenGL header files not found]))

    AC_CHECK_HEADER(GLUT/glut.h, AC_DEFINE(GLUT_IS_PRESENT),
    AC_MSG_ERROR([GLUT header file not found]))
	;;
*)
    AC_CHECK_HEADERS(GL/gl.h GL/glu.h,,
    AC_MSG_ERROR([OpenGL header files not found]))

    if test "x$ac_cv_lib_freeglut_glutGetModifiers" = "xyes"; then
        AC_CHECK_HEADER(GL/freeglut.h, AC_DEFINE(FREEGLUT_IS_PRESENT),
        AC_MSG_ERROR([freeglut header file not found]))
    else
        AC_CHECK_HEADER(GL/glut.h, AC_DEFINE(GLUT_IS_PRESENT),
        AC_MSG_ERROR([GLUT header file not found]))
    fi
	;;
esac

AC_CHECK_HEADER(plib/ul.h,, AC_MSG_ERROR([PLIB header files not found]))


dnl Warning flags
WFLAG="-Wall"
case "$host" in
mips-sgi-irix*)
    if test "x$CXX" = "xCC"; then
        WFLAG="-fullwarn";
    fi
        ;;
*)
    if test "x$CXX" = "xicpc" || test "x$CXX" = "xicc"; then
        WFLAG="-w1"
    fi
        ;;
esac

CFLAGS="$CFLAGS $WFLAG"
CXXFLAGS="$CXXFLAGS $WFLAG"


MY_EXPAND_DIR(ttt3d, "$datadir/games/$PACKAGE")
AC_DEFINE_UNQUOTED(TTT3D_DATADIR, "$ttt3d_datadir")

AC_CONFIG_FILES([Makefile         \
           data/Makefile    \
           doc/Makefile     \
           fonts/Makefile   \
           images/Makefile  \
           mods/Makefile    \
           src/Makefile     \
           wavs/Makefile    \
          ])
AC_OUTPUT

