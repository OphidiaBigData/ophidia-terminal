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

#ifndef OPH_TERM_ENV_H_
#define OPH_TERM_ENV_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <curl/curl.h>
#include <dirent.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

//Environment management
#include "hashtbl.h"
//Status codes
#include "oph_term_status.h"
//Other defs
#include "../oph_term.h"

#define OPH_TERM_ENV_OPH_PREFIX			"oph_"
#define OPH_TERM_ENV_OPH_PREFIX_LEN		4
#define OPH_TERM_ENV_OPH_MASSIVE		"oph_massive"
#define OPH_TERM_ENV_OPH_MASSIVE_LEN	11

#define OPH_TERM_CMD_MAX_LEN			72
#define OPH_TERM_WF_MAX_LEN				100*1024

//Variables keys
#define OPH_TERM_ENV_OPH_TERM_PS1       	"OPH_TERM_PS1"
#define OPH_TERM_ENV_OPH_USER           	"OPH_USER"
#define OPH_TERM_ENV_OPH_PASSWD         	"OPH_PASSWD"
#define OPH_TERM_ENV_OPH_SERVER_HOST    	"OPH_SERVER_HOST"
#define OPH_TERM_ENV_OPH_SERVER_PORT    	"OPH_SERVER_PORT"
#define OPH_TERM_ENV_OPH_SESSION_ID     	"OPH_SESSION_ID"
#define OPH_TERM_ENV_OPH_EXEC_MODE      	"OPH_EXEC_MODE"
#define OPH_TERM_ENV_OPH_NCORES         	"OPH_NCORES"
#define OPH_TERM_ENV_OPH_CWD	        	"OPH_CWD"
#define OPH_TERM_ENV_OPH_CDD	        	"OPH_CDD"
#define OPH_TERM_ENV_OPH_DATACUBE			"OPH_DATACUBE"
#define OPH_TERM_ENV_OPH_TERM_VIEWER		"OPH_TERM_VIEWER"
#define OPH_TERM_ENV_OPH_TERM_IMGS			"OPH_TERM_IMGS"
#define OPH_TERM_ENV_OPH_TERM_FORMAT		"OPH_TERM_FORMAT"
#define OPH_TERM_ENV_OPH_GRAPH_LAYOUT		"OPH_GRAPH_LAYOUT"
#define OPH_TERM_ENV_OPH_RESPONSE_BUFFER	"OPH_RESPONSE_BUFFER"
#define OPH_TERM_ENV_OPH_WORKFLOW_AUTOVIEW	"OPH_WORKFLOW_AUTOVIEW"
#define OPH_TERM_ENV_OPH_TOKEN				"OPH_TOKEN"
#ifdef WITH_IM_SUPPORT
#define OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL	"OPH_INFRASTRUCTURE_URL"
#define OPH_TERM_ENV_OPH_AUTH_HEADER		"OPH_AUTH_HEADER"
#define env_vars_num 20
#else
#define env_vars_num 18
#endif

static char *env_vars[env_vars_num] = {
	OPH_TERM_ENV_OPH_TERM_PS1,
	OPH_TERM_ENV_OPH_USER,
	OPH_TERM_ENV_OPH_PASSWD,
	OPH_TERM_ENV_OPH_SERVER_HOST,
	OPH_TERM_ENV_OPH_SERVER_PORT,
	OPH_TERM_ENV_OPH_SESSION_ID,
	OPH_TERM_ENV_OPH_EXEC_MODE,
	OPH_TERM_ENV_OPH_NCORES,
	OPH_TERM_ENV_OPH_CWD,
	OPH_TERM_ENV_OPH_CDD,
	OPH_TERM_ENV_OPH_DATACUBE,
	OPH_TERM_ENV_OPH_TERM_VIEWER,
	OPH_TERM_ENV_OPH_TERM_IMGS,
	OPH_TERM_ENV_OPH_TERM_FORMAT,
	OPH_TERM_ENV_OPH_GRAPH_LAYOUT,
	OPH_TERM_ENV_OPH_RESPONSE_BUFFER,
	OPH_TERM_ENV_OPH_WORKFLOW_AUTOVIEW,
	OPH_TERM_ENV_OPH_TOKEN
#ifdef WITH_IM_SUPPORT
	    ,
	OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL,
	OPH_TERM_ENV_OPH_AUTH_HEADER
#endif
};

static HASHTBL *env_vars_ptr = NULL;
static HASHTBL *alias_ptr = NULL;
static HASHTBL *xml_defs = NULL;
static char **operators_list = NULL;
static int operators_list_size = 0;

typedef struct operator_argument {
	char name[30];
	char type[15];
	char mandatory;
	char default_val[40];
	char min[8];
	char max[8];
	char values[200];
	short int is_last;
} operator_argument;

#define OPH_TERM_ENV_LAST_JOBID "_last_jobid"

//Alloc hashtable
int oph_term_env_init(HASHTBL ** hashtbl);
//Free hashtable
int oph_term_env_clear(HASHTBL * hashtbl);
//Print all environment variables
int oph_term_env(HASHTBL * hashtbl);
//Set value for variable key (insert new variable if not present)
int oph_term_setenv(HASHTBL * hashtbl, const char *key, const char *value);
//Clear variable key if present or do nothing
int oph_term_unsetenv(HASHTBL * hashtbl, const char *key);
//Print value for variable key
int oph_term_getenv(HASHTBL * hashtbl, const char *key);
// Check if key is an environment variable
int is_env_var(const char *key);
// Get the list of user-defined variables
int oph_term_get_user_vars(HASHTBL * hashtbl, char ***user_vars, int *user_vars_num);
// Check if key does not start with "oph_" or "/" or "." and is not a number
int is_env_var_ok(const char *key);

// Update XML definitions for Ophidia operators (userpwd="user:pwd" or NULL)
int oph_term_env_update_xml(const char *repoURL, const char *userpwd, const char *local_dir);
// Remove a directory recursively
int oph_term_env_rmdir(const char *local_dir);

// XML autocompletion management
//Alloc hashtable
int oph_term_env_xmlauto_init(HASHTBL ** hashtbl, const char *local_dir);
//Free hashtable
int oph_term_env_xmlauto_clear(HASHTBL * hashtbl, char **oplist, int oplist_size);
//Get all operators names
int oph_term_env_xmlauto_get_operators(HASHTBL * hashtbl, char ***list, int *list_size);
//Get an operator parameter list
int oph_term_env_xmlauto_get_parameters(HASHTBL * hashtbl, const char *key, operator_argument ** args, int *args_size);

// Start XML parser
int oph_term_env_start_xml_parser();
// Close XML parser
int oph_term_env_end_xml_parser();
// Load operator arguments from XML (operator_name[OPH_TERM_MAX_LEN])
int oph_term_env_load_xml(const char *xmlfilename, char **operator_name, operator_argument ** operator_args, size_t * operator_args_size);

// Get a property from server
int oph_term_env_oph_get_config(const char *key, const char *host, const char *port, int *return_value, char **property, const char *user, const char *passwd, int workflow_wrap, HASHTBL * hashtbl);

/* Other commands */

// Read all the contents of a file into memory (buffer has to be de-allocated after use)
int oph_term_read_file(char *filename, char **buffer);

// Read all the contents of a file into memory (buffer has to be de-allocated after use) with allocated size as output
int oph_term_read_file_with_len(char *filename, char **buffer, long *alloc_size);

// Parse view filters and return the number of jobs (jobs_num) and an array of size jobs_num with unique MarkerIDs
int oph_term_view_filters_parsing(char *filters, int end, int **jobs, int *jobs_num);

// Get the number of jobs of a remote session
int oph_term_get_session_size(char *session, char *user, char *passwd, char *host, char *port, int *return_value, int *size, int workflow_wrap, HASHTBL * hashtbl, char ***exit_status);

// Tell server to switch session
int oph_term_switch_remote_session(char *session, char *user, char *passwd, char *host, char *port, int *return_value, int workflow_wrap, HASHTBL * hashtbl);

// Substitute all occurrences of $variable or ${variable} in a string with a specified value string (expanded_string has to be de-allocated after use)
int oph_term_var_expansion(char *param_string, char *variable, char *value, char **expanded_string);

// Substitute all variables in a string with their values or "" until all $s are substituted (expanded_string has to be de-allocated after use)
int oph_term_full_var_expansion(char *param_string, HASHTBL * hashtbl, char **expanded_string);

// Get session code from sessionid (code is char[OPH_TERM_MAX_LEN])
int oph_term_get_session_code(char *sessionid, char *code);

// Get the command and jobid of a request identified by a wid
int oph_term_get_request(char *session, char *wid, char *user, char *passwd, char *host, char *port, int *return_value, char **command, char **jobid, char **request_time, HASHTBL * hashtbl);

// Get the command and jobid of a request identified by a mkid
int oph_term_get_request_with_marker(char *session, char *mkid, char *user, char *passwd, char *host, char *port, int *return_value, char **command, char **jobid, HASHTBL * hashtbl);

// Get the entire json request and jobid of a request identified by a wid
int oph_term_get_full_request(char *session, char *wid, char *user, char *passwd, char *host, char *port, int *return_value, char **command, char **jobid, HASHTBL * hashtbl);

// Check if wid and mkid are valid
int oph_term_check_wid_mkid(char *session, char *wid, char *mkid, char *user, char *passwd, char *host, char *port, int *return_value, HASHTBL * hashtbl);

/* ALIAS */
//Alloc hashtable
int oph_term_alias_init(HASHTBL ** hashtbl);
//Free hashtable
int oph_term_alias_clear(HASHTBL * hashtbl);
//Print all aliases
int oph_term_alias(HASHTBL * hashtbl);
//Set value for alias key (insert new variable if not present)
int oph_term_setalias(HASHTBL * hashtbl, const char *key, const char *value);
//Clear alias key if present or do nothing
int oph_term_unsetalias(HASHTBL * hashtbl, const char *key);
//Print value for alias key
int oph_term_getalias(HASHTBL * hashtbl, const char *key);
// Get the list of aliases
int oph_term_get_aliases(HASHTBL * hashtbl, char ***aliases, int *aliases_num);
// Check if key does not start with "oph_" or "/" or "." and is not a number and is different from local commands
int is_alias_ok(const char *key);
// Check if key is an alias
int is_alias(HASHTBL * hashtbl, const char *key);

#ifdef WITH_IM_SUPPORT
/*IM related functions*/
#define OPH_IM_SERVER_SUFFIX "/vms/0"
#define OPH_IM_SERVER_DEFAULT_PORT "11732"
#define OPH_IM_STATUS_FINISHED "INFO - Process finished\\n\\n\"}"
#define OPH_IM_STATUS_EMPTY "\"vm_list\": [], \"cont_out\": \"\""
#define OPH_IM_STATUS_ERROR "error"
#define OPH_IM_SERVER_INTERFACE "net_interface.0.ip = '"
#define OPH_IM_INFRASTRUCTURES_NAME "/infrastructures"
#define OPH_IM_VMS_TOKEN "/vms/"
#define OPH_IM_MAX_LEN 102400
// Deploy an Ophidia cluster with IM and set OPH_INFRASTRUCTURE_URL
int oph_term_env_deploy(const char *auth_header, const char *infrastructure_url, const char *radl_filename, HASHTBL * hashtbl);
// Un-Deploy an Ophidia cluster with IM and reset OPH_INFRASTRUCTURE_URL, OPH_SERVER_HOST and OPH_SERVER_PORT
int oph_term_env_undeploy(const char *auth_header, const char *infrastructure_url, HASHTBL * hashtbl);
// Check status of deploy
int oph_term_env_deploy_status(const char *auth_header, const char *infrastructure_url);
// After deploy set OPH_SERVER_HOST and OPH_SERVER_PORT
int oph_term_env_get_server(const char *auth_header, const char *infrastructure_url, HASHTBL * hashtbl);
// View the list of all deployed infrastructures
int oph_term_env_deploys_list(const char *auth_header, const char *infrastructure_url);
// View the list of all deployed VMs for an infrastructure
int oph_term_env_deploy_vms_list(const char *auth_header, const char *infrastructure_url);
#endif

#endif
