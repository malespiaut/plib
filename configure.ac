dnl Process this file with autoconf to produce a configure script.

# **WARNING**WARNING**WARNING**WARNING**WARNING**WARNING**
#
# When you make a new release, there are TWO places where you
# have to change the version number:
#  1)  Right below this comment.
#  2)  In 'ul.h' where PLIB_VERSION is created.
#
# **WARNING**WARNING**WARNING**WARNING**WARNING**WARNING**

AC_INIT([plib], [1.8.5])
AC_CONFIG_SRCDIR([src/js/js.h])
AM_INIT_AUTOMAKE([1.16])

# This AC_PREFIX_DEFAULT will place the library files
# into /usr/lib - but will put the include files into
# /usr/include - when /usr/include/plib would be less
# polluting to the filename space.
#
# There is probably a better way to do this.

AC_PREFIX_DEFAULT(/usr)

if echo $includedir | grep -E "plib$" > /dev/null; then
    echo "includedir is" $includedir "libdir is" $libdir
else
    includedir="${includedir}/plib"
    echo "includedir changed to" $includedir "libdir is" $libdir
fi


dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_PROG_RANLIB

dnl Missing checks detected by autoscan
AC_CHECK_FUNCS([atexit])
AC_CHECK_FUNCS([floor])
AC_CHECK_FUNCS([getcwd])
AC_CHECK_FUNCS([gethostbyname])
AC_CHECK_FUNCS([gethostname])
AC_CHECK_FUNCS([gettimeofday])
AC_CHECK_FUNCS([inet_ntoa])
AC_CHECK_FUNCS([memmove])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([munmap])
AC_CHECK_FUNCS([pow])
AC_CHECK_FUNCS([select])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([sqrt])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strcspn])
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNCS([strncasecmp])
AC_CHECK_FUNCS([strrchr])
AC_CHECK_FUNCS([strstr])
AC_CHECK_FUNCS([strtol])
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([mach/mach.h])
AC_CHECK_HEADERS([netdb.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([sys/ioctl.h])
AC_CHECK_HEADERS([sys/param.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_PREREQ([2.71])
AC_PROG_MAKE_SET
AC_TYPE_SIZE_T

dnl Command line arguments

dnl Don't use AS_HELP_STRING here as old versions of autoconf do not
dnl understand it.
AC_ARG_WITH(GL,
[  --with-GL=DIR           set the prefix directory where GL resides],
GL_PREFIX=$withval, GL_PREFIX=auto)

AC_ARG_ENABLE(fnt,
[  --enable-fnt            build FNT library (OpenGL needed)       default: yes],,
enable_fnt=yes)

AC_ARG_ENABLE(js,
[  --enable-js             build JS library                        default: yes],,
enable_js=yes)

AC_ARG_ENABLE(pw,
[  --enable-pw             build PW library                        default: yes],,
enable_pw=yes)

AC_ARG_ENABLE(net,
[  --enable-net            build NET library                       default: yes],,
enable_net=yes)

AC_ARG_ENABLE(pui,
[  --enable-pui            build PUI library (OpenGL needed)       default: yes],,
enable_pui=yes)

AC_ARG_ENABLE(sg,
[  --enable-sg             build SG library                        default: yes],,
enable_sg=yes)

AC_ARG_ENABLE(psl,
[  --enable-psl            build PSL library                       default: yes],,
enable_psl=yes)

AC_ARG_ENABLE(sl,
[  --enable-sl             build SL library                        default: yes],,
enable_sl=yes)

AC_ARG_ENABLE(ssg,
[  --enable-ssg            build SSG library (OpenGL needed)       default: yes],,
enable_ssg=yes)

AC_ARG_ENABLE(puaux,
[  --enable-puaux         build puAux library (OpenGL needed)    default: yes],,
enable_puaux=yes)

AC_ARG_ENABLE(ssgaux,
[  --enable-ssgaux         build ssgAux library (OpenGL needed)    default: yes],,
enable_ssgaux=yes)

AC_ARG_ENABLE(ul,
[  --enable-ul             build UTIL library                      default: yes],,
enable_ul=yes)


dnl Dependancy checking

if test "x$enable_fnt" = "xyes"; then
    need_opengl=yes

    if test "x$enable_sg" = "xno"; then
        AC_MSG_WARN([cannot build FNT library without SG library enabled])
        dependancy_problem=yes
    fi
    if test "x$enable_ul" = "xno"; then
        AC_MSG_WARN([cannot build FNT library without UL library enabled])
        dependancy_problem=yes
    fi
fi

if test "x$enable_pw" = "xyes"; then
    if test "x$enable_ul" = "xno"; then
        AC_MSG_WARN([cannot build PW library without UL library enabled])
        dependancy_problem=yes
    fi
fi

if test "x$enable_js" = "xyes"; then
    if test "x$enable_ul" = "xno"; then
        AC_MSG_WARN([cannot build JS library without UL library enabled])
        dependancy_problem=yes
    fi
fi

if test "x$enable_net" = "xyes"; then
    if test "x$enable_ul" = "xno"; then
        AC_MSG_WARN([cannot build NET library without UL library enabled])
        dependancy_problem=yes
    fi
fi

if test "x$enable_pui" = "xyes"; then
    need_opengl=yes

    if test "x$enable_fnt" = "xno"; then
        AC_MSG_WARN([cannot build PUI library without FNT library enabled])
        dependancy_problem=yes
    fi
    if test "x$enable_sg" = "xno"; then
        AC_MSG_WARN([cannot build PUI library without SG library enabled])
        dependancy_problem=yes
    fi
    if test "x$enable_ul" = "xno"; then
        AC_MSG_WARN([cannot build PUI library without UL library enabled])
        dependancy_problem=yes
    fi
fi

if test "x$enable_sg" = "xyes"; then
    if test "x$enable_ul" = "xno"; then
        AC_MSG_WARN([cannot build SG library without UL library enabled])
        dependancy_problem=yes
    fi
fi

if test "x$enable_psl" = "xyes"; then
    if test "x$enable_ul" = "xno"; then
        AC_MSG_WARN([cannot build PSL library without UL library enabled])
        dependancy_problem=yes
    fi
fi

if test "x$enable_sl" = "xyes"; then
    if test "x$enable_ul" = "xno"; then
        AC_MSG_WARN([cannot build SL library without UL library enabled])
        dependancy_problem=yes
    fi
fi

if test "x$enable_ssg" = "xyes"; then
    need_opengl=yes

    if test "x$enable_sg" = "xno"; then
        AC_MSG_WARN([cannot build SSG library without SG library enabled])
        dependancy_problem=yes
    fi
    if test "x$enable_ul" = "xno"; then
        AC_MSG_WARN([cannot build SSG library without UL library enabled])
        dependancy_problem=yes
    fi
fi

if test "x$enable_puaux" = "xyes"; then
    need_opengl=yes

    if test "x$enable_pui" = "xno"; then
        AC_MSG_WARN([cannot build puAux library without PUI library enabled])
        dependancy_problem=yes
    fi
    if test "x$enable_sg" = "xno"; then
        AC_MSG_WARN([cannot build puAux library without SG library enabled])
        dependancy_problem=yes
    fi
    if test "x$enable_ul" = "xno"; then
        AC_MSG_WARN([cannot build puAux library without UL library enabled])
        dependancy_problem=yes
    fi
fi

if test "x$enable_ssgaux" = "xyes"; then
    need_opengl=yes

    if test "x$enable_ssg" = "xno"; then
        AC_MSG_WARN([cannot build ssgAux library without SSG library enabled])
        dependancy_problem=yes
    fi
    if test "x$enable_sg" = "xno"; then
        AC_MSG_WARN([cannot build ssgAux library without SG library enabled])
        dependancy_problem=yes
    fi
    if test "x$enable_ul" = "xno"; then
        AC_MSG_WARN([cannot build ssgAux library without UL library enabled])
        dependancy_problem=yes
    fi
fi

if test "x$dependancy_problem" = "xyes"; then
    AC_MSG_ERROR([dependancy problems detected])
fi


AM_CONDITIONAL(BUILD_FNT, test "x$enable_fnt" = "xyes")
AM_CONDITIONAL(BUILD_JS, test "x$enable_js" = "xyes")
AM_CONDITIONAL(BUILD_PW, test "x$enable_pw" = "xyes")
AM_CONDITIONAL(BUILD_NET, test "x$enable_net" = "xyes")
AM_CONDITIONAL(BUILD_PUI, test "x$enable_pui" = "xyes")
AM_CONDITIONAL(BUILD_PUAUX, test "x$enable_puaux" = "xyes")
AM_CONDITIONAL(BUILD_SG, test "x$enable_sg" = "xyes")
AM_CONDITIONAL(BUILD_PSL, test "x$enable_psl" = "xyes")
AM_CONDITIONAL(BUILD_SL, test "x$enable_sl" = "xyes")
AM_CONDITIONAL(BUILD_SSG, test "x$enable_ssg" = "xyes")
AM_CONDITIONAL(BUILD_SSGAUX, test "x$enable_ssgaux" = "xyes")
AM_CONDITIONAL(BUILD_UL, test "x$enable_ul" = "xyes")

if test "x$need_opengl" = "xyes" && test "x$GL_PREFIX" != "xauto"; then
    LDFLAGS="$LDFLAGS -L$GL_PREFIX/lib"
    CPPFLAGS="$CPPFLAGS -I$GL_PREFIX/include"
fi


dnl Checks for library functions.
dnl check for OpenGL related libraries

AC_CANONICAL_HOST

case "${host}" in
*-*-cygwin* | *-*-mingw32*)
    dnl CygWin under Windoze.

    dnl Multimedia library
    LIBS="$LIBS -lwinmm"

    if test "x$need_opengl" = "xyes"; then
        dnl OpenGL libraries
        LIBS="$LIBS -lopengl32"
    fi
	;;
*-apple-darwin*)
    dnl Mac OS X

    if test "x$need_opengl" = "xyes"; then
        dnl OpenGL libraries
        LIBS="$LIBS -framework OpenGL"
    fi

    dnl Sound libraries
    LIBS="$LIBS -framework Carbon"
	;;
*)
    if test "x$need_opengl" = "xyes"; then
        dnl Check for X11

        AC_PATH_XTRA

        LDFLAGS="$LDFLAGS $X_LIBS"

        LIBS="$LIBS $X_PRE_LIBS $X_LIBS $X_EXTRA_LIBS -lm"

dnl =========================================================
dnl    if test "x$x_includes" != "x"; then
dnl       CPPFLAGS="$CPPFLAGS -I$x_includes"
dnl    fi
dnl =========================================================

        dnl Reasonable stuff non-windoze variants ... :-)

        AC_CHECK_LIB(pthread, pthread_create)
        AC_CHECK_LIB(GL, glNewList)
        if test "x$ac_cv_lib_GL_glNewList" = "xno" ; then
                dnl if no GL, check for MesaGL
                AC_CHECK_LIB(MesaGL, glNewList,,
                AC_MSG_ERROR([could not find working GL library]))
        fi

    fi

    AC_CHECK_LIB(dl, dlclose)

    dnl Check SGI audio library
    AC_CHECK_LIB(audio, ALopenport)
	;;
esac


dnl Checks for header files.

AC_CHECK_HEADER(windows.h, AC_DEFINE(WIN32))

if test "x$need_opengl" = "xyes"; then
    dnl Check for OpenGl header files

    case "${host}" in
    *-*-cygwin* | *-*-mingw32*)
        dnl CygWin under Windoze.

        AC_CHECK_HEADER(GL/gl.h,,
        AC_MSG_ERROR([OpenGL header file not found]))
		;;
    *-apple-darwin*)
        dnl Mac OS X

        AC_CHECK_HEADER(OpenGL/gl.h,,
        AC_MSG_ERROR([OpenGL header file not found]))
		;;
    *)
        AC_CHECK_HEADER(GL/gl.h,,
        AC_MSG_ERROR([OpenGL header file not found]))
		;;
    esac
fi


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
   if test "x$GCC" != xyes; then
      # if the compiler is not GCC we reset WFLAG to null for safety
      WFLAG=""
   fi
	;;
esac

CFLAGS="$CFLAGS $WFLAG"
CXXFLAGS="$CXXFLAGS $WFLAG"

dnl FreeBSD requires the -pthread switch to enable pthreads. Look for this
dnl weirdness.
save_CXXFLAGS="$CXXFLAGS"
save_CFLAGS="$CFLAGS"
save_LIBS=$LIBS
CXXFLAGS="-pthread $CXXFLAGS"
CFLAGS="-pthread $CFLAGS"
AC_CHECK_LIB(c_r, pthread_exit)
if test "x$ac_cv_lib_c_r_pthread_exit" != "xyes"; then
   CFLAGS=$save_CFLAGS
   CXXFLAGS=$save_CXXFLAGS
fi
LIBS=$save_LIBS

dnl Checks for typedefs, structures, and compiler characteristics.

dnl socklen_t check
AC_MSG_CHECKING(for socklen_t)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/socket.h>
]], [[
  socklen_t foo;
]])],[
ac_cv_c_socklen_t=yes
	AC_DEFINE(HAVE_SOCKLEN_T, 1, [Have socklen_t])
	AC_MSG_RESULT(yes)
],[
	AC_MSG_RESULT(no)
])

AC_CONFIG_FILES([\
	Makefile \
	src/Makefile \
	src/js/Makefile \
	src/util/Makefile \
	src/sg/Makefile \
	src/sl/Makefile \
	src/psl/Makefile \
	src/ssg/Makefile \
	src/ssgAux/Makefile \
	src/fnt/Makefile \
	src/pui/Makefile \
	src/puAux/Makefile \
	src/pw/Makefile \
	src/net/Makefile ])
AC_OUTPUT

AC_MSG_RESULT(
[PLIB configuration information:

  Building FNT library:     $enable_fnt
  Building JS library:      $enable_js
  Building NET library:     $enable_net
  Building PUI library:     $enable_pui
  Building puAux library:   $enable_puaux
  Building SG library:      $enable_sg
  Building PSL library:     $enable_psl
  Building SL library:      $enable_sl
  Building SSG library:     $enable_ssg
  Building ssgAux library:  $enable_ssgaux
  Building PW library:      $enable_pw
  Building UTIL library:    $enable_ul
])

