# 
# configure.ac
# 
# Contact: JeCortex@yahoo.com
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

AC_PREREQ(2.69)
################################################################################
dnl -- Process this file with autoconf to produce a configure script.

AC_INIT
CONFIGURE_LINE="$0 $@"
AC_CONFIG_SRCDIR([src/tools/lbd/lbd.c])
AC_CONFIG_HEADERS([src/include/configure.h])

################################################################################
dnl -- Setup the directory where autoconf has auxilary files

AC_CONFIG_AUX_DIR(autoconf)

################################################################################
dnl -- Get system type

AC_CANONICAL_TARGET([])

AS_IF([test -z "$CFLAGS"], [COPTIMISE_FLAG="-O2 -g"])

CLDFLAGS="$CLDFLAGS -Wl,--version-script,.export.sym"
# equivalent to -rdynamic
ELDFLAGS="-Wl,--export-dynamic"
# FIXME Generate list and use --dynamic-list=.dlopen.sym
CLDWHOLEARCHIVE="-Wl,-whole-archive"
CLDNOWHOLEARCHIVE="-Wl,-no-whole-archive"
LDDEPS="$LDDEPS .export.sym"

LIB_SUFFIX=so

################################################################################
dnl -- Checks for programs.

AC_PROG_SED
AC_PROG_AWK
AC_PROG_MAKE_SET
save_CFLAGS=$CFLAGS
save_CXXFLAGS=$CXXFLAGS
AC_PROG_CC
AC_PROG_CXX
CFLAGS=$save_CFLAGS
CXXFLAGS=$save_CXXFLAGS

AC_PROG_MKDIR_P
AC_CHECK_TOOL(AR, ar)
AC_PATH_TOOL(CHMOD, chmod)
AC_PATH_TOOL(WC, wc)
AC_PATH_TOOL(SORT, sort)

################################################################################
dnl -- various defaults
dnl -- FIXME: need to switch to regular option here  --sysconfdir
AC_ARG_WITH(default-system-dir,
	    AC_HELP_STRING([--with-default-system-dir=DIR],
			   [default LBD system directory [/etc/lbd]]),
	    DEFAULT_SYS_DIR=$withval, DEFAULT_SYS_DIR="/etc/lbd")
AC_DEFINE_UNQUOTED(DEFAULT_SYS_DIR, ["$DEFAULT_SYS_DIR"],
 		   [Path to LBD system directory.])

################################################################################
dnl -- Check for header files.

AC_HEADER_DIRENT

################################################################################
dnl -- Check for typedefs, structures.

AC_C_INLINE

AC_STRUCT_TM

AC_TYPE_UINT32_T

################################################################################
dnl -- Check for functions.

AC_FUNC_FORK

################################################################################
dnl -- Prefix is /usr by default.

AC_PREFIX_DEFAULT(/usr)

################################################################################
dnl Version define
read VER < "$srcdir"/VERSION 2>/dev/null || VER=Unknown
LBD_VERSION=\"$VER\"
LBD_RELEASE_DATE="\"`echo $VER | $SED 's/.* (//;s/).*//'`\""
LBD_MAJOR=`echo "$VER" | $AWK -F '.' '{print $1}'`
LBD_MINOR=`echo "$VER" | $AWK -F '.' '{print $2}'`
LBD_PATCHLEVEL=`echo "$VER" | $AWK -F '[[(.]]' '{print $3}'`

################################################################################
dnl -- Vaules export.

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(COPTIMISE_FLAG)
AC_SUBST(CLDFLAGS)
AC_SUBST(ELDFLAGS)
AC_SUBST(LDDEPS)
AC_SUBST(LIB_SUFFIX)
AC_SUBST(LBD_VERSION)
AC_SUBST(LBD_MAJOR)
AC_SUBST(LBD_MINOR)
AC_SUBST(LBD_PATCHLEVEL)
AC_SUBST(DEFAULT_SYS_DIR)

################################################################################
dnl -- Transform 
AC_CONFIG_FILES([
Makefile
src/lib/Makefile
src/libjraid/Makefile
src/tools/lbd/Makefile
src/include/lbd-version.h
make.build
])
AC_OUTPUT
