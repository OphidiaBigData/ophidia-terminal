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
#include <stddef.h>
#define OPH_SERVER_OK 0
#define OPH_SERVER_UNKNOWN 1
#define OPH_SERVER_NULL_POINTER 2
#define OPH_SERVER_ERROR 3
#define OPH_SERVER_IO_ERROR 4
#define OPH_SERVER_AUTH_ERROR 5
#define OPH_SERVER_SYSTEM_ERROR 6
#define OPH_SERVER_WRONG_PARAMETER_ERROR 7
#define OPH_SERVER_NO_RESPONSE 8
#define OPH_MAX_STRING_SIZE 1024
#define OPH_SHORT_STRING_SIZE 128
#endif

#ifndef OPH_TERM_STATUS_H_
#define OPH_TERM_STATUS_H_

/* Status codes */
#define OPH_TERM_SUCCESS                        0
#define OPH_TERM_GENERIC_ERROR                  1
#define OPH_TERM_MEMORY_ERROR                   2
#define OPH_TERM_COMMAND_NOT_RECOGNIZED         3
#define OPH_TERM_OPTION_NOT_RECOGNIZED          4
#define OPH_TERM_SERVER_NOT_REACHABLE           5
#define OPH_TERM_PARAM_NOT_RECOGNIZED_BY_SERVER 6
#define OPH_TERM_SERVER_ACCESS_DENIED           7
#define OPH_TERM_INVALID_PARAM_VALUE            8
#define OPH_TERM_ERROR_WITHIN_JSON              9

#endif
