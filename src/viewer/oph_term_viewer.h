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

#ifndef OPH_TERM_VIEWER_H_
#define OPH_TERM_VIEWER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define OUTPUT_MAX_LEN 500*1024	// 500 KB
#define OUTPUT_JSON_STRING "{\n  \"request\":\"%s\",\n  \"jobid\":\"%s\",\n  \"response\":%s,\n  \"stdout\":\"%s\",\n  \"stderr\":\"%s\"\n}"
extern int print_json;
extern int print_debug_data;
extern char oph_term_request[OUTPUT_MAX_LEN];
extern char oph_term_jobid[OUTPUT_MAX_LEN];
extern char *oph_term_response;
extern char oph_term_output[OUTPUT_MAX_LEN];
extern int oph_term_output_cur;
extern char oph_term_error[OUTPUT_MAX_LEN];
extern int oph_term_error_cur;
#define my_printf(...) (oph_term_output_cur+=snprintf(oph_term_output+oph_term_output_cur,OUTPUT_MAX_LEN-oph_term_output_cur,__VA_ARGS__))
#define my_fprintf(stream,...) ((stream==stderr)?(oph_term_error_cur+=snprintf(oph_term_error+oph_term_error_cur,OUTPUT_MAX_LEN-oph_term_error_cur,__VA_ARGS__)):(oph_term_output_cur+=snprintf(oph_term_output+oph_term_output_cur,OUTPUT_MAX_LEN-oph_term_output_cur,__VA_ARGS__)))


//Status codes
#include "../env/oph_term_status.h"

#define OPH_TERM_VIEWER_TYPE_DUMP		"dump"
#define OPH_TERM_VIEWER_TYPE_BASIC		"basic"
#define OPH_TERM_VIEWER_TYPE_COLOURED		"coloured"
#define OPH_TERM_VIEWER_TYPE_EXTENDED		"extended"
#define OPH_TERM_VIEWER_TYPE_EXTENDED_COLOURED	"extended_coloured"

#define OPH_TERM_VIEWER_NO_COLOR_STRING		"%s"

#define OPH_TERM_VIEWER_BLACK_STRING		"\033[1;30m%s\033[0m"
#define OPH_TERM_VIEWER_RED_STRING		"\033[1;31m%s\033[0m"
#define OPH_TERM_VIEWER_GREEN_STRING		"\033[1;32m%s\033[0m"
#define OPH_TERM_VIEWER_YELLOW_STRING		"\033[1;33m%s\033[0m"
#define OPH_TERM_VIEWER_BLUE_STRING		"\033[1;34m%s\033[0m"
#define OPH_TERM_VIEWER_PURPLE_STRING		"\033[1;35m%s\033[0m"
#define OPH_TERM_VIEWER_CYAN_STRING		"\033[1;36m%s\033[0m"
#define OPH_TERM_VIEWER_WHITE_STRING		"\033[1;37m%s\033[0m"

#define OPH_TERM_VIEWER_COLOR_STRING_SURPLUS	11
#define OPH_TERM_VIEWER_EXIT_STATUS_INDEX	5

#define OPH_TERM_VIEWER_WORKFLOW_TASK_LIST	"Workflow Task List"
#define OPH_TERM_VIEWER_WORKFLOW_TASK_LIST2	"Workflow Basic Information"

//Render OPH_JSON string into console
// viewer_type : dump (for plain json), basic (for a pretty output), coloured (basic+colors), extended (basic+source+consumers), extended_coloured (extended+colors)
// json_string : json string to be rendered as double pointer (upon return the string is freed and NULLed)
// color : color to be used in coloured viewers (color equal to term prompt)
// save_img : 1 to save graph images to files, 0 otherwise
// open_img : 1 to auto-open with xdg-open graph images, 0 otherwise. Considered only when save_img=1
// newdatacube : retrieved new output cube if any or NULL
// newcwd : retrieved new cwd if any or NULL
// newcdd : retrieved new cdd if any or NULL
// layout : layout of graph or NULL
int oph_term_viewer(const char *viewer_type, char **json_string, const char *color, int save_img, int open_img, int show_list, char **newdatacube, char **newcwd, char **newccd, char **newtoken,
		    char **exectime, char *layout);

//Retrieve command and/or jobid in request after a call to oph_resume
int oph_term_viewer_retrieve_command_jobid_creation(char *json_string, char **command, char **jobid, char **creation_time);

//Retrieve command and/or jobid in request after a call to oph_resume
int oph_term_viewer_retrieve_command_jobid(char *json_string, char **command, char **jobid);

//Retrieve number of jobs in session after a call to oph_resume
int oph_term_viewer_retrieve_session_size(char *json_string, int *session_size, char ***exit_status);

//Check if session has been saved by server after a call to oph_resume with save=yes
int oph_term_viewer_is_session_switched(char *json_string);

//Retrieve config value corresponding to a key after a call to oph_get_config
int oph_term_viewer_retrieve_config(char *json_string, const char *key, char ***keys, char ***props, unsigned int *nprops, char **newtoken);

//Check if wid#mkid is a valid pair in json
int oph_term_viewer_check_wid_mkid(char *json_string, char *wid, char *mkid);

//Check if objkey "workflow_status" is present in json
int oph_term_viewer_check_workflow_status(char *json_string);

//Check if workflow has ended
int oph_term_viewer_check_workflow_ended(char *json_string);

void get_term_size(int *rows, int *cols);

#endif
