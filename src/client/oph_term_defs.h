/*
    Ophidia Terminal
    Copyright (C) 2012-2017 CMCC Foundation

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __OPH_TERM_DEFS_H
#define __OPH_TERM_DEFS_H

#include "../env/oph_term_status.h"
#include <stddef.h>

#ifndef INTERFACE_TYPE_IS_WPS
#include "stdsoap2.h"
// Types
typedef char *xsd__anyURI;
typedef char xsd__boolean;
typedef char *xsd__date;
typedef time_t xsd__dateTime;
typedef double xsd__double;
typedef char *xsd__duration;
typedef float xsd__float;
typedef char *xsd__time;
typedef char *xsd__decimal;
typedef char *xsd__integer;
typedef LONG64 xsd__long;
typedef long xsd__int;
typedef short xsd__short;
typedef char xsd__byte;
typedef char *xsd__nonPositiveInteger;
typedef char *xsd__negativeInteger;
typedef char *xsd__nonNegativeInteger;
typedef char *xsd__positiveInteger;
typedef ULONG64 xsd__unsignedLong;
typedef unsigned long xsd__unsignedInt;
typedef unsigned short xsd__unsignedShort;
typedef unsigned char xsd__unsignedByte;
typedef char *xsd__string;
typedef char *xsd__normalizedString;
typedef char *xsd__token;
#endif

// Server errors
#define OPH_SERVER_OK 0
#define OPH_SERVER_UNKNOWN 1
#define OPH_SERVER_NULL_POINTER 2
#define OPH_SERVER_ERROR 3
#define OPH_SERVER_IO_ERROR 4
#define OPH_SERVER_AUTH_ERROR 5
#define OPH_SERVER_SYSTEM_ERROR 6
#define OPH_SERVER_WRONG_PARAMETER_ERROR 7
#define OPH_SERVER_NO_RESPONSE 8

// Other consts
#define OPH_MAX_STRING_SIZE 1024
#define OPH_SHORT_STRING_SIZE 128

#endif
