dnl Copyright (C) 2014 William Breathitt Gray
dnl
dnl This file is part of Interfic.
dnl
dnl Interfic is free software: you can redistribute it and/or modify it
dnl under the terms of the GNU Affero General Public License as
dnl published by the Free Software Foundation, either version 3 of the
dnl License, or (at your option) any later version.
dnl
dnl Interfic is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl Affero General Public License for more details.
dnl
dnl You should have received a copy of the GNU Affero General Public
dnl License along with Interfic.  If not, see
dnl <http://www.gnu.org/licenses/>.

AC_PREREQ([2.69])
AC_INIT([Interfic], [1.0], [vilhelm.gray@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/interfic.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC_C99

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h stddef.h stdint.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memmove strtoul])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
