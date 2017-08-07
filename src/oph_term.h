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

#ifndef OPH_TERM_H_
#define OPH_TERM_H_

/* OPH_TERM Headers */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <pthread.h>

/* PKGS Headers */
#include "client/oph_term_client.h"
#include "env/oph_term_env.h"
#include "env/oph_term_status.h"
#include "help/oph_term_help.h"
#include "viewer/oph_term_viewer.h"
#include "workflow/oph_workflow_library.h"
//...

#define OPH_TERM_CMD_MASSIVE	"oph_massive"

/* Local Commands */
#define OPH_TERM_CMD_VERSION    "version"
#define OPH_TERM_CMD_WARRANTY   "warranty"
#define OPH_TERM_CMD_WATCH      "watch"
#define OPH_TERM_CMD_CONDITIONS "conditions"
#define OPH_TERM_CMD_HELP       "help"
#define OPH_TERM_CMD_HISTORY    "history"
#define OPH_TERM_CMD_ENV        "env"
#define OPH_TERM_CMD_SETENV     "setenv"
#define OPH_TERM_CMD_UNSETENV   "unsetenv"
#define OPH_TERM_CMD_GETENV     "getenv"
#define OPH_TERM_CMD_QUIT       "quit"
#define OPH_TERM_CMD_EXIT       "exit"
#define OPH_TERM_CMD_CLEAR      "clear"
#define OPH_TERM_CMD_WATCH      "watch"
#define OPH_TERM_CMD_UPDATE     "update"
#define OPH_TERM_CMD_RESUME     "resume"
#define OPH_TERM_CMD_VIEW       "view"
#ifndef NO_WORKFLOW
#define OPH_TERM_CMD_CHECK      "check"
#endif
#define OPH_TERM_CMD_ALIAS      "alias"
#define OPH_TERM_CMD_SETALIAS   "setalias"
#define OPH_TERM_CMD_UNSETALIAS "unsetalias"
#define OPH_TERM_CMD_GETALIAS   "getalias"
#ifdef WITH_IM_SUPPORT
#define OPH_TERM_CMD_DEPLOY      		"deploy"
#define OPH_TERM_CMD_DEPLOY_STATUS  	"deploy_status"
#define OPH_TERM_CMD_GET_SERVER  		"get_server"
#define OPH_TERM_CMD_UNDEPLOY  			"undeploy"
#define OPH_TERM_CMD_DEPLOYS_LIST  		"deploys_list"
#define OPH_TERM_CMD_DEPLOY_VMS_LIST  	"deploy_vms_list"
#define im_cmds_num 6
#else
#define im_cmds_num 0
#endif
/* Remote Commands */
#define OPH_TERM_REMOTE_CMD_PREFIX	OPH_TERM_ENV_OPH_PREFIX
#define OPH_TERM_REMOTE_CMD_PREFIX_LEN	OPH_TERM_ENV_OPH_PREFIX_LEN

#define OPH_TERM_TOKEN_USER "__token__"

#ifndef NO_WORKFLOW
#define cmds_num 21+im_cmds_num
#else
#define cmds_num 20+im_cmds_num
#endif
static char *cmds[cmds_num] = {
	OPH_TERM_CMD_VERSION,
	OPH_TERM_CMD_WARRANTY,
	OPH_TERM_CMD_WATCH,
	OPH_TERM_CMD_CONDITIONS,
	OPH_TERM_CMD_HELP,
	OPH_TERM_CMD_HISTORY,
	OPH_TERM_CMD_ENV,
	OPH_TERM_CMD_SETENV,
	OPH_TERM_CMD_UNSETENV,
	OPH_TERM_CMD_GETENV,
	OPH_TERM_CMD_QUIT,
	OPH_TERM_CMD_EXIT,
	OPH_TERM_CMD_CLEAR,
	OPH_TERM_CMD_UPDATE,
	OPH_TERM_CMD_RESUME,
	OPH_TERM_CMD_VIEW,
#ifndef NO_WORKFLOW
	OPH_TERM_CMD_CHECK,
#endif
	OPH_TERM_CMD_ALIAS,
	OPH_TERM_CMD_SETALIAS,
	OPH_TERM_CMD_UNSETALIAS,
	OPH_TERM_CMD_GETALIAS
#ifdef WITH_IM_SUPPORT
	    ,
	OPH_TERM_CMD_DEPLOY,
	OPH_TERM_CMD_DEPLOY_STATUS,
	OPH_TERM_CMD_GET_SERVER,
	OPH_TERM_CMD_UNDEPLOY,
	OPH_TERM_CMD_DEPLOYS_LIST,
	OPH_TERM_CMD_DEPLOY_VMS_LIST
#endif
};

#define OUTPUT_MAX_LEN 500*1024	// 500 KB
#define OUTPUT_JSON_STRING "{\n  \"request\":\"%s\",\n  \"jobid\":\"%s\",\n  \"response\":%s,\n  \"stdout\":\"%s\",\n  \"stderr\":\"%s\"\n}"
extern int print_json;
extern char oph_term_request[OUTPUT_MAX_LEN];
extern char oph_term_jobid[OUTPUT_MAX_LEN];
extern char oph_term_output[OUTPUT_MAX_LEN];
extern int oph_term_output_cur;
extern char oph_term_error[OUTPUT_MAX_LEN];
extern int oph_term_error_cur;
#define my_printf(...) (oph_term_output_cur+=snprintf(oph_term_output+oph_term_output_cur,OUTPUT_MAX_LEN-oph_term_output_cur,__VA_ARGS__))
#define my_fprintf(stream,...) ((stream==stderr)?(oph_term_error_cur+=snprintf(oph_term_error+oph_term_error_cur,OUTPUT_MAX_LEN-oph_term_error_cur,__VA_ARGS__)):(oph_term_output_cur+=snprintf(oph_term_output+oph_term_output_cur,OUTPUT_MAX_LEN-oph_term_output_cur,__VA_ARGS__)))


/* Miscellanea */
#define OPH_TERM_HISTORY_FILE "%s/.oph_term_history"
#define OPH_TERM_XML_FOLDER "%s/.oph_term_xml"
#define OPH_TERM_XML_URL_KEY "OPH_XML_URL"
#define OPH_TERM_SESSION_ID_KEY "OPH_SESSION_ID"
#define OPH_TERM_OPH_DATACUBE_KEY "OPH_DATACUBE"
#define OPH_TERM_OPH_CWD_KEY "OPH_CWD"
#define OPH_TERM_OPH_CDD_KEY "OPH_CDD"
#define OPH_TERM_OPH_BASE_SRC_PATH_KEY "OPH_BASE_SRC_PATH"
#define OPH_TERM_GET_CONFIG_PROP "operator=oph_get_config;"
#define OPH_TERM_OPT_W_FILENAME "opt_w_filename"
#define OPH_TERM_OPT_W_ARGS "opt_w_args"
#define OPH_TERM_OPT_W_ARGS_DELIMITER ","
#define OPH_TERM_OPH_RESUME_STRING_NO_SAVE "operator=oph_resume;"
#define OPH_TERM_OPH_RESUME_STRING_SAVE "operator=oph_resume;save=yes;"
#define OPH_TERM_USERVAR_PATTERN "OPH_TERM_USERVAR_"
#define OPH_TERM_ALIAS_PATTERN "OPH_TERM_ALIAS_"
#define OPH_TERM_SUBSTITUTION_MAX_CYCLES 10000

#define OPH_TERM_PROMPT "[OPH_TERM] >>"
#define OPH_TERM_BLACK_PROMPT "\001\033[1;30m\002%s\001\033[0m\002 "
#define OPH_TERM_RED_PROMPT "\001\033[1;31m\002%s\001\033[0m\002 "
#define OPH_TERM_GREEN_PROMPT "\001\033[1;32m\002%s\001\033[0m\002 "
#define OPH_TERM_YELLOW_PROMPT "\001\033[1;33m\002%s\001\033[0m\002 "
#define OPH_TERM_BLUE_PROMPT "\001\033[1;34m\002%s\001\033[0m\002 "
#define OPH_TERM_PURPLE_PROMPT "\001\033[1;35m\002%s\001\033[0m\002 "
#define OPH_TERM_CYAN_PROMPT "\001\033[1;36m\002%s\001\033[0m\002 "
#define OPH_TERM_WHITE_PROMPT "\001\033[1;37m\002%s\001\033[0m\002 "

#define OPH_TERM_DEFAULT_ALIAS_KEY_1 	"ls"
#define OPH_TERM_DEFAULT_ALIAS_KEY_2 	"cd"
#define OPH_TERM_DEFAULT_ALIAS_KEY_3 	"mkdir"
#define OPH_TERM_DEFAULT_ALIAS_KEY_4 	"mvdir"
#define OPH_TERM_DEFAULT_ALIAS_KEY_5 	"rmdir"
#define OPH_TERM_DEFAULT_ALIAS_KEY_6 	"mv"
#define OPH_TERM_DEFAULT_ALIAS_KEY_7 	"rm"
#define OPH_TERM_DEFAULT_ALIAS_KEY_8 	"del"
#define OPH_TERM_DEFAULT_ALIAS_KEY_9 	"pwd"
#define OPH_TERM_DEFAULT_ALIAS_KEY_10 	"man"
#define OPH_TERM_DEFAULT_ALIAS_KEY_11 	"ll"
#define OPH_TERM_DEFAULT_ALIAS_KEY_12 	"lr"
#define OPH_TERM_DEFAULT_ALIAS_KEY_13 	"cc"
#define OPH_TERM_DEFAULT_ALIAS_KEY_14 	"rc"
#define OPH_TERM_DEFAULT_ALIAS_KEY_15 	"kill"
#define OPH_TERM_DEFAULT_ALIAS_KEY_16 	"lsd"
#define OPH_TERM_DEFAULT_ALIAS_KEY_17 	"cdd"
#define OPH_TERM_DEFAULT_ALIAS_KEY_18 	"pdd"
#define OPH_TERM_DEFAULT_ALIAS_KEY_19 	"getprogress"
#define OPH_TERM_DEFAULT_ALIAS_KEY_20 	"show"
#define OPH_TERM_DEFAULT_ALIAS_KEY_21 	"new"
#define OPH_TERM_DEFAULT_ALIAS_KEY_22 	"drop"
#define OPH_TERM_DEFAULT_ALIAS_KEY_23 	"jobs"
#define OPH_TERM_DEFAULT_ALIAS_KEY_24 	"ncdump"
#define OPH_TERM_DEFAULT_ALIAS_KEY_25 	"mkdird"
#define OPH_TERM_DEFAULT_ALIAS_KEY_26 	"mvd"
#define OPH_TERM_DEFAULT_ALIAS_KEY_27 	"rmd"

#define OPH_TERM_DEFAULT_ALIAS_VAL_1 	"oph_list path=$1;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_2 	"oph_folder command=cd;path=$1;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_3 	"oph_folder command=mkdir;path=$1;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_4 	"oph_folder command=mv;path=$1|$2;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_5 	"oph_folder command=rm;path=$1;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_6 	"oph_movecontainer container=$1|$2;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_7 	"oph_deletecontainer container=$1;hidden=no;delete_type=physical;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_8 	"oph_delete cube=$1;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_9 	"getenv OPH_CWD"
#define OPH_TERM_DEFAULT_ALIAS_VAL_10 	"oph_man function=$1;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_11 	"oph_list path=$1;level=2;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_12 	"oph_list path=$1;level=2;recursive=yes;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_13 	"oph_createcontainer container=$1;dim=lat|lon|time;dim_type=double|double|double;hierarchy=oph_base|oph_base|oph_time;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_14 	"oph_randcube container=$1;dim=lat|lon|time;dim_size=$2|$3|$4;exp_ndim=2;host_partition=test;measure=example_measure;measure_type=double;nfrag=$2;ntuple=$3;compressed=no;concept_level=c|c|d;filesystem=local;ndb=1;ndbms=1;nhost=1;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_15 	"oph_cancel id=$1"
#define OPH_TERM_DEFAULT_ALIAS_VAL_16 	"oph_fs command=ls;dpath=$1;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_17 	"oph_fs command=cd;dpath=$1;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_18 	"getenv OPH_CDD"
#define OPH_TERM_DEFAULT_ALIAS_VAL_19 	"oph_resume level=0;id=$1;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_20 	"oph_explorecube show_time=yes;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_21 	"oph_manage_session action=new;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_22 	"del [*]"
#define OPH_TERM_DEFAULT_ALIAS_VAL_23 	"oph_loggingbk job_level=1;mask=010;session_filter=@OPH_SESSION_ID;parent_job_filter=%;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_24 	"oph_explorenc src_path=$1;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_25 	"oph_fs command=mkdir;dpath=$1;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_26 	"oph_fs command=mv;dpath=$1|$2;"
#define OPH_TERM_DEFAULT_ALIAS_VAL_27 	"oph_fs command=rm;dpath=$1;"

#define OPH_TERM_DEFAULT_ALIAS_NUM 27

static char *pre_defined_aliases_keys[OPH_TERM_DEFAULT_ALIAS_NUM] = {
	OPH_TERM_DEFAULT_ALIAS_KEY_1,
	OPH_TERM_DEFAULT_ALIAS_KEY_2,
	OPH_TERM_DEFAULT_ALIAS_KEY_3,
	OPH_TERM_DEFAULT_ALIAS_KEY_4,
	OPH_TERM_DEFAULT_ALIAS_KEY_5,
	OPH_TERM_DEFAULT_ALIAS_KEY_6,
	OPH_TERM_DEFAULT_ALIAS_KEY_7,
	OPH_TERM_DEFAULT_ALIAS_KEY_8,
	OPH_TERM_DEFAULT_ALIAS_KEY_9,
	OPH_TERM_DEFAULT_ALIAS_KEY_10,
	OPH_TERM_DEFAULT_ALIAS_KEY_11,
	OPH_TERM_DEFAULT_ALIAS_KEY_12,
	OPH_TERM_DEFAULT_ALIAS_KEY_13,
	OPH_TERM_DEFAULT_ALIAS_KEY_14,
	OPH_TERM_DEFAULT_ALIAS_KEY_15,
	OPH_TERM_DEFAULT_ALIAS_KEY_16,
	OPH_TERM_DEFAULT_ALIAS_KEY_17,
	OPH_TERM_DEFAULT_ALIAS_KEY_18,
	OPH_TERM_DEFAULT_ALIAS_KEY_19,
	OPH_TERM_DEFAULT_ALIAS_KEY_20,
	OPH_TERM_DEFAULT_ALIAS_KEY_21,
	OPH_TERM_DEFAULT_ALIAS_KEY_22,
	OPH_TERM_DEFAULT_ALIAS_KEY_23,
	OPH_TERM_DEFAULT_ALIAS_KEY_24,
	OPH_TERM_DEFAULT_ALIAS_KEY_25,
	OPH_TERM_DEFAULT_ALIAS_KEY_26,
	OPH_TERM_DEFAULT_ALIAS_KEY_27
};

static char *pre_defined_aliases_values[OPH_TERM_DEFAULT_ALIAS_NUM] = {
	OPH_TERM_DEFAULT_ALIAS_VAL_1,
	OPH_TERM_DEFAULT_ALIAS_VAL_2,
	OPH_TERM_DEFAULT_ALIAS_VAL_3,
	OPH_TERM_DEFAULT_ALIAS_VAL_4,
	OPH_TERM_DEFAULT_ALIAS_VAL_5,
	OPH_TERM_DEFAULT_ALIAS_VAL_6,
	OPH_TERM_DEFAULT_ALIAS_VAL_7,
	OPH_TERM_DEFAULT_ALIAS_VAL_8,
	OPH_TERM_DEFAULT_ALIAS_VAL_9,
	OPH_TERM_DEFAULT_ALIAS_VAL_10,
	OPH_TERM_DEFAULT_ALIAS_VAL_11,
	OPH_TERM_DEFAULT_ALIAS_VAL_12,
	OPH_TERM_DEFAULT_ALIAS_VAL_13,
	OPH_TERM_DEFAULT_ALIAS_VAL_14,
	OPH_TERM_DEFAULT_ALIAS_VAL_15,
	OPH_TERM_DEFAULT_ALIAS_VAL_16,
	OPH_TERM_DEFAULT_ALIAS_VAL_17,
	OPH_TERM_DEFAULT_ALIAS_VAL_18,
	OPH_TERM_DEFAULT_ALIAS_VAL_19,
	OPH_TERM_DEFAULT_ALIAS_VAL_20,
	OPH_TERM_DEFAULT_ALIAS_VAL_21,
	OPH_TERM_DEFAULT_ALIAS_VAL_22,
	OPH_TERM_DEFAULT_ALIAS_VAL_23,
	OPH_TERM_DEFAULT_ALIAS_VAL_24,
	OPH_TERM_DEFAULT_ALIAS_VAL_25,
	OPH_TERM_DEFAULT_ALIAS_VAL_26,
	OPH_TERM_DEFAULT_ALIAS_VAL_27
};

static const int pre_defined_aliases_num = OPH_TERM_DEFAULT_ALIAS_NUM;

#define OPH_TERM_MAX_LEN 1024

extern pthread_t tid;

#define OPH_TERM_VERSION "Oph_Term - the Ophidia shell, version " PACKAGE_VERSION "\nCopyright (C) 2012-2017 CMCC Foundation - www.cmcc.it\n"
#define OPH_TERM_VERSION2 "Oph_Term - the Ophidia shell, version " PACKAGE_VERSION "\\nCopyright (C) 2012-2017 CMCC Foundation - www.cmcc.it\\n"

#define OPH_TERM_DISCLAIMER "This program comes with ABSOLUTELY NO WARRANTY; for details type `oph_term -x'.\nThis is free software, and you are welcome to redistribute it\nunder certain conditions; type `oph_term -z' for details.\n"
#define OPH_TERM_DISCLAIMER2 "This program comes with ABSOLUTELY NO WARRANTY; for details type `oph_term -x'.\\nThis is free software, and you are welcome to redistribute it\nunder certain conditions; type `oph_term -z' for details.\\n"
#define OPH_TERM_DISCLAIMER3 "This program comes with ABSOLUTELY NO WARRANTY; for details type `warranty'.\nThis is free software, and you are welcome to redistribute it\nunder certain conditions; type `conditions' for details.\n"
#define OPH_TERM_DISCLAIMER4 "This program comes with ABSOLUTELY NO WARRANTY; for details type `warranty'.\\nThis is free software, and you are welcome to redistribute it\nunder certain conditions; type `conditions' for details.\\n"

#define OPH_TERM_WARRANTY "\nTHERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n"
#define OPH_TERM_WARRANTY2 "\\nTHERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\\n"

#include "oph_license.h"

#endif
