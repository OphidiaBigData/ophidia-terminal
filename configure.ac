#
#    Ophidia Terminal
#    Copyright (C) 2012-2017 CMCC Foundation
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

AC_PREREQ([2.60])
AC_INIT([ophidia-terminal], [1.2.0], [ophidia-info@cmcc.it])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PROG_CC_STDC
AM_PROG_CC_C_O

LT_INIT([disable-static])

# Checks for libraries.
AC_CHECK_LIB(ncurses, initscr, [have_ncurses=yes], [have_ncurses=no])
AX_LIB_CRYPTO([yes])
AX_LIB_READLINE
AC_CHECK_LIB([m],[cos])
AX_PTHREAD(AC_MSG_NOTICE("pthread found"),AC_MSG_ERROR("pthread not found"))

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h inttypes.h limits.h locale.h memory.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/file.h sys/ioctl.h sys/socket.h sys/time.h sys/timeb.h unistd.h])
AC_CHECK_HEADERS(readline/history.h readline/readline.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_FUNC_STRTOD
AC_CHECK_FUNCS([ftime gethostbyname gettimeofday localtime_r memmove memset mkdir select socket strcasecmp strchr strdup strerror strstr strtol])

PKG_CHECK_MODULES(ZLIB, zlib >= 1.2.3)
AC_ARG_WITH(libz_header_path,
	   [  --with-libz-header-path=PATH 		Location of zlib includes directory],
	   [
		libz_header_path="$withval"
	   ],
     [
    libz_header_path="$ZLIB_CFLAGS"
     ])
AC_ARG_WITH(libz_lib_path,
	   [  --with-libz-lib-path=PATH 		Location of libz.a],
	   [
		libz_lib_path="$withval"
	   ],
     [
    libz_lib_path="$ZLIB_LIBS"
     ])

LIBZ_INCLUDE="$libz_header_path"
AC_SUBST(LIBZ_INCLUDE)
LIBZ_LIB="$libz_lib_path"
AC_SUBST(LIBZ_LIB)

PKG_CHECK_MODULES(XML, libxml-2.0 >= 2.4)
AC_ARG_WITH(libxml_header_path,
	   [  --with-libxml-header-path=PATH 		Location of libxml2 includes directory],
	   [
		libxml_header_path="$withval"
	   ],
     [
    libxml_header_path="$XML_CFLAGS"
     ])
AC_ARG_WITH(libxml_lib_path,
	   [  --with-libxml-lib-path=PATH 		Location of libxml2.a],
	   [
		libxml_lib_path="$withval"
	   ],
     [
    libxml_lib_path="$XML_LIBS"
     ])

LIBXML_INCLUDE="$libxml_header_path"
AC_SUBST(LIBXML_INCLUDE)
LIBXML_LIB="$libxml_lib_path"
AC_SUBST(LIBXML_LIB)

PKG_CHECK_MODULES(CURL, libcurl >= 0.23)
AC_ARG_WITH(libcurl_header_path,
	   [  --with-libcurl-header-path=PATH 		Location of libcurl includes directory],
	   [
		libcurl_header_path="$withval"
	   ],
     [
    libcurl_header_path="$CURL_CFLAGS"
     ])
AC_ARG_WITH(libcurl_lib_path,
	   [  --with-libcurl-lib-path=PATH 		Location of libcurl.a],
	   [
		libcurl_lib_path="$withval"
	   ],
     [
    libcurl_lib_path="$CURL_LIBS"
     ])

LIBCURL_INCLUDE="$libcurl_header_path"
AC_SUBST(LIBCURL_INCLUDE)
LIBCURL_LIB="$libcurl_lib_path"
AC_SUBST(LIBCURL_LIB)

PKG_CHECK_MODULES(OPENSSL, openssl >= 1.0.0)
AC_ARG_WITH(libopenssl_header_path,
	   [  --with-libopenssl-header-path=PATH 		Location of openssl includes directory],
	   [
		libopenssl_header_path="$withval"
	   ],
     [
    libopenssl_header_path="$OPENSSL_CFLAGS"
     ])
AC_ARG_WITH(libopenssl_lib_path,
	   [  --with-libopenssl-lib-path=PATH 		Location of openssl],
	   [
		libopenssl_lib_path="$withval"
	   ],
     [
    libopenssl_lib_path="$OPENSSL_LIBS"
     ])
LIBSSL_INCLUDE="$libopenssl_header_path"
AC_SUBST(LIBSSL_INCLUDE)
LIBSSL_LIB="$libopenssl_lib_path"
AC_SUBST(LIBSSL_LIB)

PKG_CHECK_MODULES(LIBGVC, libgvc >= 2.26.0)
AC_ARG_WITH(libgvc_header_path,
	   [  --with-libgvc-header-path=PATH 		Location of libgvc (graphviz) includes directory],
	   [
		libgvc_header_path="$withval"
	   ],
     [
    libgvc_header_path="$LIBGVC_CFLAGS"
     ])
AC_ARG_WITH(libgvc_lib_path,
	   [  --with-libgvc-lib-path=PATH 		Location of libgvc (graphviz)],
	   [
		libgvc_lib_path="$withval"
	   ],
     [
    libgvc_lib_path="$LIBGVC_LIBS"
     ])
LIBGVC_INCLUDE="$libgvc_header_path"
AC_SUBST(LIBGVC_INCLUDE)
LIBGVC_LIB="$libgvc_lib_path"
AC_SUBST(LIBGVC_LIB)

PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.0)
AC_ARG_WITH(gtk2_header_path,
       [  --with-gtk2-header-path=PATH        Location of GTK+-2.0 includes directory],
       [
        gtk2_header_path="$withval"
       ],
     [
    gtk2_header_path="$GTK_CFLAGS"
     ])
AC_ARG_WITH(gtk2_lib_path,
       [  --with-gtk2-lib-path=PATH       Location of GTK+-2.0],
       [
        gtk2_lib_path="$withval"
       ],
     [
    gtk2_lib_path="$GTK_LIBS"
     ])
GTK2_INCLUDE="$gtk2_header_path"
AC_SUBST(GTK2_INCLUDE)
GTK2_LIB="$gtk2_lib_path"
AC_SUBST(GTK2_LIB)


PKG_CHECK_MODULES(JANSSON, jansson >= 2.3)
AC_ARG_WITH(jansson_header_path,
       [  --with-jansson-header-path=PATH        Location of Jansson includes directory],
       [
        jansson_header_path="$withval"
       ],
     [
    jansson_header_path="$JANSSON_CFLAGS"
     ])
AC_ARG_WITH(jansson_lib_path,
       [  --with-jansson-lib-path=PATH       Location of jansson lib],
       [
        jansson_lib_path="$withval"
       ],
     [
    jansson_lib_path="$JANSSON_LIBS"
     ])
JANSON_CFLAGS="$jansson_header_path"
AC_SUBST(JANSON_CFLAGS)
JANSON_LIBS="$jansson_lib_path"
AC_SUBST(JANSON_LIBS)
JANSSON_LIBDIR=`pkg-config --variable=libdir jansson`
AC_SUBST(JANSSON_LIBDIR)

AC_ARG_WITH(interface_type,
	   [  --with-interface-type=TYPE 		Interface type: SSL],
	   [
		interface_type="$withval"
	   ],
	   [
		interface_type="ssl"
	   ])
if test "x${interface_type}" = "xssl"; then
INTERFACE_TYPE=$interface_type
elif test "x${interface_type}" = "xgsi"; then
INTERFACE_TYPE=$interface_type
elif test "x${interface_type}" = "xwps"; then
INTERFACE_TYPE=$interface_type
else
AC_MSG_ERROR(Unsupported interface type)
fi
AC_SUBST(INTERFACE_TYPE)
AM_CONDITIONAL([INTERFACE_TYPE_IS_SSL], [test "x$interface_type" = "xssl"])
AM_CONDITIONAL([INTERFACE_TYPE_IS_GSI], [test "x$interface_type" = "xgsi"])
AM_CONDITIONAL([INTERFACE_TYPE_IS_WPS], [test "x$interface_type" = "xwps"])

AC_ARG_WITH(globus-libs,
                                [  --with-globus-libs=GLOBUS_LIB_LOCATION   Set the Globus library location],
                                [GLOBUS_LIB="$withval"]
                                )
AC_SUBST(GLOBUS_LIB)
if test -n $GLOBUS_LIB; then
        AC_MSG_NOTICE([Globus library in: $GLOBUS_LIB])
fi

AC_ARG_WITH(globus-include,
                                [  --with-globus-include=GLOBUS_INCLUDE_LOCATION   Set the Globus header location],
                                [GLOBUS_INCLUDE="$withval"]
                                )
AC_SUBST(GLOBUS_INCLUDE)
if test -n $GLOBUS_INCLUDE; then
       AC_MSG_NOTICE([Globus includes: $GLOBUS_INCLUDE])
fi

#Enable VOMS
voms_enable=0
AC_ARG_ENABLE(voms,
            [  --enable-voms  Enables support for VOMS on GLITE],
                     [voms_enable=1],
                     [voms_enable=0])
VOMS=$voms_enable
AC_SUBST(VOMS)

#Set VOMS libs path
AC_ARG_WITH(voms-libs,
            [  --with-voms-libs=VOMS_LIBS       Sets the VOMS libraries path],
                    [
                             VOMS_LIBS="$withval"
                    ]
            )
if test -z "$VOMS_LIBS" && test $voms_enable = 1; then
        AC_MSG_ERROR([no VOMS libraries directory found])
else
	AC_MSG_NOTICE([VOMS libraries directory: $VOMS_LIBS])
	VOMS_DEFINE="GLITE_VOMS"
fi
AC_SUBST(VOMS_LIBS)
AC_SUBST(VOMS_DEFINE)

#Set VOMS include path
AC_ARG_WITH(voms-include,
            [  --with-voms-include=VOMS_INCLUDE       Sets the VOMS include path],
                    [
                             VOMS_INCLUDE="$withval"
                    ]
            )
if test -z "$VOMS_INCLUDE" && test $voms_enable = 1; then
        AC_MSG_ERROR([no VOMS include directory found])
else
	AC_MSG_NOTICE([VOMS includes: $VOMS_INCLUDE])
fi
AC_SUBST(VOMS_INCLUDE)

OPT=
AC_ARG_ENABLE(debug,
	   [  --enable-debug	Enable debug. (Disabled by default)],
	   [ debug="yes"],
           [ debug="no"]
	   )
if test "x$debug" = "xyes"; then
	AC_MSG_WARN("Debug activated")
	OPT="-g3 -Wall -Wextra -O0 -DDEBUG_LEVEL"
else
	OPT="-O2"
fi
AC_ARG_ENABLE(workflows,
       [  --disable-workflows    Show workflow references. (Enabled by default)],
       [
            if test "x$enableval" = "xno"; then
                workflows="no"
            else
                workflows="yes"
            fi
       ],
       [ workflows="yes"]
       )
if test "x${workflows}" = "xno"; then
    AC_MSG_WARN("Workflows hidden")
    OPT+=" -DNO_WORKFLOW"
fi

im_support=
AC_ARG_ENABLE(im-support,
       [  --enable-im-support    Enable support for IM-related commands. (Disabled by default)],
       [
            if test "x$enableval" = "xno"; then
                im_support="no"
            else
                im_support="yes"
            fi
       ],
       [ im_support="no"]
       )
if test "x${im_support}" = "xyes"; then
    AC_MSG_NOTICE("Enabled IM support")
    OPT+=" -DWITH_IM_SUPPORT"
fi

base_src_path=
AC_ARG_ENABLE(base_src_path,
       [  --enable-base-src-path    Enable support for OPH_BASE_SRC_PATH. (Disabled by default)],
       [
            if test "x$enableval" = "xno"; then
                base_src_path="no"
            else
                base_src_path="yes"
            fi
       ],
       [ base_src_path="no"]
       )
if test "x${base_src_path}" = "xyes"; then
    AC_MSG_NOTICE("Enabled OPH_BASE_SRC_PATH support")
    OPT+=" -DCHDDIR"
fi

xml_ssl_check=
AC_ARG_ENABLE(xml_ssl_check,
       [  --disable-xml-ssl-check    [Disable verification of SSL certificates when 
				downloading operators XML. (Enabled by default)]],
       [
            if test "x$enableval" = "xno"; then
                xml_ssl_check="no"
            else
                xml_ssl_check="yes"
            fi
       ],
       [ xml_ssl_check="yes"]
       )
if test "x${xml_ssl_check}" = "xno"; then
    AC_MSG_NOTICE("Disabled XML SSL certificate check")
    OPT+=" -DNO_XML_SSL_CHECK"
fi

case "${host}" in
        *-*-solaris*)   PLATFORM=SUN_OS
                        SAMPLE_EXTRA_LIBS="-lxnet -lsocket -lnsl"
                        ;;
        *-*-sysv5*)     PLATFORM=UNIXWARE ;;
        *-*-UnixWare*)  PLATFORM=UNIXWARE ;;
        *-*-unixware*)  PLATFORM=UNIXWARE ;;
        *-*-sco3.2v5*)  PLATFORM=OPENSERVER
                        SAMPLE_EXTRA_LIBS="-lsocket"
                        ;;
        *-*-linux*)     PLATFORM=LINUX
                        
			            ;;
                *-*-cygwin*)    PLATFORM=CYGWIN
                        ;;
        *-*-freebsd*)   PLATFORM=FREEBSD ;;
        *-*-irix*)      PLATFORM=IRIX
                        ;;
        *-*-aix*)       PLATFORM=AIX
                      case "${host}" in
                         *-*-aix4.1*)   osver=AIX41;;
                         *-*-aix4.2*)   osver=AIX42;;
                         *-*-aix4.3*)   osver=AIX43;;
                         *-*-aix4.*)    osver=AIX43;;
                      esac;;
        *-sequent-*)    PLATFORM=PTX ;;
        *-*-hp*)        PLATFORM=HP_UX ;
                        case "${host}" in
                            *-*-hpux11*)  osver=HPUX11;;
                            *-*-hpux10*)  osver=HPUX10;;
                            *)            osver=HPUX11;;
                        esac;;
        *-*-mvs*)       PLATFORM=OS390 ;;
        *-*-os400*)     PLATFORM=OS400 ;;
        *-*-OS400*)     PLATFORM=OS400 ;;
        *-*-osf*)       PLATFORM=TRU64
				        OPT+=" -g3"
				        REENTRANT="1"
                               ;;
        *-apple-*)      PLATFORM=__APPLE__
				        OPT+=" -g3"
				        ;;
        *)              PLATFORM=UNKNOWN ;;
esac

AC_SUBST(SAMPLE_EXTRA_LIBS)
AC_SUBST(PLATFORM)
AC_SUBST(OPT)
AC_SUBST(REENTRANT)

AM_CONDITIONAL(VOMS, test $voms_enable = 1)

AC_OUTPUT([Makefile src/Makefile])
