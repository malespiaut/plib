dnl Process this file with autoconf to produce a configure script.

AC_INIT([plib_examples], [1.8.5])
AC_CONFIG_SRCDIR([src/js/js_demo.cxx])
AM_INIT_AUTOMAKE([1.16])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL

AC_LANG([C++])

AC_CHECK_FUNCS([atexit])
AC_CHECK_FUNCS([floor])
AC_CHECK_FUNCS([gettimeofday])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([pow])
AC_CHECK_FUNCS([sqrt])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_PREREQ([2.71])
AC_TYPE_OFF_T

dnl Command line arguments.

dnl Don't use AS_HELP_STRING here as old versions of autoconf do not
dnl understand it.
AC_ARG_WITH(GL,
[  --with-GL=DIR           set the prefix directory where GL resides],
GL_PREFIX=$withval, GL_PREFIX=auto)


AC_ARG_ENABLE(fnt,
[  --enable-fnt            build FNT examples                      default: yes],,
enable_fnt=yes)

AC_ARG_ENABLE(js,
[  --enable-js             build JS examples                       default: yes],,
enable_js=yes)

AC_ARG_ENABLE(pw,
[  --enable-pw             build PW examples                       default: yes],,
enable_pw=yes)

AC_ARG_ENABLE(net,
[  --enable-net            build NET examples                      default: yes],,
enable_net=yes)

AC_ARG_ENABLE(pui,
[  --enable-pui            build PUI examples                      default: yes],,
enable_pui=yes)

AC_ARG_ENABLE(sg,
[  --enable-sg             build SG examples                       default: yes],,
enable_sg=yes)

AC_ARG_ENABLE(psl,
[  --enable-psl            build PSL examples                       default: yes],,
enable_psl=yes)

AC_ARG_ENABLE(sl,
[  --enable-sl             build SL examples                       default: yes],,
enable_sl=yes)

AC_ARG_ENABLE(ssg,
[  --enable-ssg            build SSG/ssgAux examples               default: yes],,
enable_ssg=yes)

AC_ARG_ENABLE(ul,
[  --enable-ul             build UTIL examples                     default: yes],,
enable_ul=yes)


if test "x$enable_fnt" = "xyes"; then need_opengl=yes; fi
if test "x$enable_pui" = "xyes"; then need_opengl=yes; fi
if test "x$enable_sg" = "xyes"; then need_opengl=yes; fi
if test "x$enable_ssg" = "xyes"; then need_opengl=yes; fi
if test "x$enable_ssgaux" = "xyes"; then need_opengl=yes; fi

AM_CONDITIONAL(BUILD_FNT, test "x$enable_fnt" = "xyes")
AM_CONDITIONAL(BUILD_JS, test "x$enable_js" = "xyes")
AM_CONDITIONAL(BUILD_PW, test "x$enable_pw" = "xyes")
AM_CONDITIONAL(BUILD_NET, test "x$enable_net" = "xyes")
AM_CONDITIONAL(BUILD_PUI, test "x$enable_pui" = "xyes")
AM_CONDITIONAL(BUILD_SG, test "x$enable_sg" = "xyes")
AM_CONDITIONAL(BUILD_PSL, test "x$enable_psl" = "xyes")
AM_CONDITIONAL(BUILD_SL, test "x$enable_sl" = "xyes")
AM_CONDITIONAL(BUILD_SSG, test "x$enable_ssg" = "xyes")
AM_CONDITIONAL(BUILD_UL, test "x$enable_ul" = "xyes")

if test "x$need_opengl" = "xyes" && test "x$GL_PREFIX" != "xauto"; then
    LDFLAGS="$LDFLAGS -L$GL_PREFIX/lib"
    CPPFLAGS="$CPPFLAGS -I$GL_PREFIX/include"
fi


dnl Checks for library functions.
dnl check for OpenGL related libraries

AC_CANONICAL_HOST

case "${host}" in
*-*-cygwin*)

    dnl CygWin under Windoze.

    dnl Sound / joystick stuff, mmtimer (ulClock !)
    LIBS="$LIBS -lwinmm"

    dnl OpenGL libraries
    OGL_LIBS="-lglu32 -lopengl32 -lgdi32"
    GLUT_LIBS="-lglut32"
	;;
*-*-mingw32*)
    dnl Mingw under Windoze.

    dnl Sound / joystick stuff, mmtimer (ulClock !)
    LIBS="$LIBS -lwinmm -lwsock32"


    dnl OpenGL libraries
    OGL_LIBS="-lglu32 -lopengl32 -lgdi32"
    GLUT_LIBS="-lglut32"
	;;
*-apple-darwin*)
    dnl Mac OS X

    dnl OpenGL libraries
    OGL_LIBS="-framework OpenGL"
    GLUT_LIBS="-framework GLUT -lobjc"

    dnl Sound libraries
    SND_LIBS="-framework Carbon"
	;;
*)
    if test "x$need_opengl" = "xyes"; then
        dnl Check for X11

        AC_PATH_XTRA

        LDFLAGS="$LDFLAGS $X_LIBS"

        OGL_LIBS="$X_PRE_LIBS -lXi -lXmu -lXext -lX11 $X_EXTRA_LIBS -lm"

dnl =========================================================
dnl    if test "x$x_includes" != "x"; then
dnl       CPPFLAGS="$CPPFLAGS -I$x_includes"
dnl    fi
dnl =========================================================

        dnl Reasonable stuff non-windoze variants ... :-)

        AC_CHECK_LIB(pthread, pthread_create)

        AC_CHECK_LIB(GL, glNewList, OGL_LIBS="-lGL $OGL_LIBS",,$OGL_LIBS)
        if test "x$ac_cv_lib_GL_glNewList" = "xno" ; then
                dnl if no GL, check for MesaGL
                AC_CHECK_LIB(MesaGL, glNewList, OGL_LIBS="-lMesaGL $OGL_LIBS",
                AC_MSG_ERROR([could not find working GL library]),
                $OGL_LIBS)
        fi

        AC_CHECK_LIB(GLU, gluLookAt, OGL_LIBS="-lGLU $OGL_LIBS",,$OGL_LIBS)
        if test "x$ac_cv_lib_GLU_gluLookAt" = "xno" ; then
                dnl if no GLU, check for MesaGLU
                AC_CHECK_LIB(MesaGLU, gluLookAt,
                OGL_LIBS="-lMesaGLU $OGL_LIBS",
                AC_MSG_ERROR([could not find working GLU library]),
                $OGL_LIBS)
	fi

        dnl check for glut...or freeglut

        AC_CHECK_LIB(freeglut, glutGetModifiers, GLUT_LIBS="-lfreeglut",,
        $OGL_LIBS)
        if test "x$ac_cv_lib_freeglut_glutGetModifiers" = "xno" ; then
                dnl if no freeglut, check for real GLUT
                AC_CHECK_LIB(glut, glutGetModifiers, GLUT_LIBS="-lglut",
                AC_MSG_ERROR([could not find working GLUT library]),
                $OGL_LIBS)
        fi
    fi

    AC_CHECK_LIB(dl, dlclose)

    dnl Check SGI audio library
    AC_CHECK_LIB(audio, ALopenport, SND_LIBS="-laudio")
	;;
esac

dnl Check PLIB library
AC_CHECK_LIB(plibul, ulInit, true,
AC_MSG_ERROR([there seems to be a problem with the PLIB libraries]))


AC_SUBST(OGL_LIBS)
AC_SUBST(GLUT_LIBS)
AC_SUBST(SND_LIBS)


dnl Checks for header files.

AC_CHECK_HEADER(windows.h, AC_DEFINE(WIN32))

if test "x$need_opengl" = "xyes"; then
    dnl Check for OpenGl header files

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
fi

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


AC_CONFIG_FILES([\
	Makefile \
	src/Makefile \
	src/js/Makefile \
	src/pw/Makefile \
	src/pui/Makefile \
	src/sg/Makefile \
	src/sl/Makefile \
	src/ssg/Makefile \
	src/ssg/tux/Makefile \
	src/ssg/majik/Makefile \
	src/ssg/load_save/Makefile \
	src/ssg/state_test/Makefile \
	src/ssg/tween_test/Makefile \
	src/ssg/viewer/Makefile \
	src/ssg/water/Makefile \
	src/ssg/sky/Makefile \
	src/ssg/dynamics/Makefile \
	src/ssg/shrubs/Makefile \
	src/ssg/shapes/Makefile \
	src/fnt/Makefile \
	src/net/Makefile \
	src/net/client_server/Makefile \
	src/net/http_get/Makefile \
	src/net/net_echo/Makefile \
	src/psl/Makefile \
	src/util/Makefile ])
AC_OUTPUT

if test "x$need_opengl" = "xyes"; then
    if test "x$ac_cv_lib_freeglut_glutGetModifiers" = "xyes"; then
        glut_msg="yes (found freeglut)"
    else
        glut_msg="yes (found GLUT)"
    fi
else
    glut_msg="no"
fi

AC_MSG_RESULT(
[PLIB configuration information:

  Building FNT examples:         $enable_fnt
  Building JS examples:          $enable_js
  Building PW examples:          $enable_pw
  Building NET examples:         $enable_net
  Building PUI examples:         $enable_pui
  Building SG examples:          $enable_sg
  Building PSL examples:         $enable_psl
  Building SL examples:          $enable_sl
  Building SSG/ssgAux examples:  $enable_ssg
  Building UTIL examples:        $enable_ul

  Need GLUT:                     $glut_msg
])

