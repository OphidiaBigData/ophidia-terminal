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

#include "oph_term_help.h"

#define UNUSED(x) {(void)(x);}
void foo()
{
	UNUSED(pre_defined_aliases_keys)
	    UNUSED(pre_defined_aliases_values)
	    UNUSED(env_vars_ptr) UNUSED(alias_ptr)
	    UNUSED(xml_defs) UNUSED(operators_list)
	    UNUSED(operators_list_size)
}

#ifdef WITH_IM_SUPPORT
// Print usage for "deploy"
void print_deploy_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_DEPLOY_LONG2) : printf("%s", OPH_TERM_HELP_DEPLOY_LONG);
}

// Print usage for "deploy_status"
void print_deploy_status_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_DEPLOY_STATUS_LONG2) : printf("%s", OPH_TERM_HELP_DEPLOY_STATUS_LONG);
}

// Print usage for "get_server"
void print_get_server_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_GET_SERVER_LONG2) : printf("%s", OPH_TERM_HELP_GET_SERVER_LONG);
}

// Print usage for "undeploy"
void print_undeploy_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_UNDEPLOY_LONG2) : printf("%s", OPH_TERM_HELP_UNDEPLOY_LONG);
}

// Print usage for "deploys_list"
void print_deploys_list_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_DEPLOYS_LIST_LONG2) : printf("%s", OPH_TERM_HELP_DEPLOYS_LIST_LONG);
}

// Print usage for "deploy_vms_list"
void print_deploy_vms_list_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_DEPLOY_VMS_LIST_LONG2) : printf("%s", OPH_TERM_HELP_DEPLOY_VMS_LIST_LONG);
}
#endif

#ifndef NO_WORKFLOW
// Print usage for "check"
void print_check_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_CHECK_LONG2) : printf("%s", OPH_TERM_HELP_CHECK_LONG);
}

// Print usage for "remote workflow"
void print_remote_workflow_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_REMOTE_WORKFLOW2) : printf("%s", OPH_TERM_HELP_REMOTE_WORKFLOW);
}
#endif

// Print usage for "version"
void print_version_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_VERSION_LONG2) : printf("%s", OPH_TERM_HELP_VERSION_LONG);
}

// Print usage for "warranty"
void print_warranty_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_WARRANTY_LONG2) : printf("%s", OPH_TERM_HELP_WARRANTY_LONG);
}

// Print usage for "watch"
void print_watch_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_WATCH_LONG2) : printf("%s", OPH_TERM_HELP_WATCH_LONG);
}

// Print usage for "version"
void print_conditions_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_CONDITIONS_LONG2) : printf("%s", OPH_TERM_HELP_CONDITIONS_LONG);
}

// Print usage for "view"
void print_view_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_VIEW_LONG2) : printf("%s", OPH_TERM_HELP_VIEW_LONG);
}

// Print usage for "resume"
void print_resume_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_RESUME_LONG2) : printf("%s", OPH_TERM_HELP_RESUME_LONG);
}

// Print usage for "help"
void print_help_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_HELP_LONG2) : printf("%s", OPH_TERM_HELP_HELP_LONG);
}

// Print usage for "history"
void print_history_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_HISTORY_LONG2) : printf("%s", OPH_TERM_HELP_HISTORY_LONG);
}

// Print usage for "remote command"
void print_remote_command_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_REMOTE_COMMAND2) : printf("%s", OPH_TERM_HELP_REMOTE_COMMAND);
}

// Print usage for "autocompletion"
void print_autocompletion_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_AUTOCOMPLETION2) : printf("%s", OPH_TERM_HELP_AUTOCOMPLETION);
}

// Print usage for "var substitution"
void print_var_substitution_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_VARIABLE_SUBSTITUTION2) : printf("%s", OPH_TERM_HELP_VARIABLE_SUBSTITUTION);
}

// Print usage for "alias substitution"
void print_alias_substitution_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_ALIAS_SUBSTITUTION2) : printf("%s", OPH_TERM_HELP_ALIAS_SUBSTITUTION);
}

// Print usage for "env"
void print_env_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_ENV_LONG2) : printf("%s", OPH_TERM_HELP_ENV_LONG);
}

// Print usage for "setenv"
void print_setenv_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_SETENV_LONG2) : printf("%s", OPH_TERM_HELP_SETENV_LONG);
}

// Print usage for "unsetenv"
void print_unsetenv_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_UNSETENV_LONG2) : printf("%s", OPH_TERM_HELP_UNSETENV_LONG);
}

// Print usage for "getenv"
void print_getenv_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_GETENV_LONG2) : printf("%s", OPH_TERM_HELP_GETENV_LONG);
}

// Print usage for "quit"
void print_quit_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_QUIT_LONG2) : printf("%s", OPH_TERM_HELP_QUIT_LONG);
}

// Print usage for "exit"
void print_exit_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_EXIT_LONG2) : printf("%s", OPH_TERM_HELP_EXIT_LONG);
}

// Print usage for "clear"
void print_clear_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_CLEAR_LONG2) : printf("%s", OPH_TERM_HELP_CLEAR_LONG);
}

// Print usage for "update"
void print_update_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_UPDATE_LONG2) : printf("%s", OPH_TERM_HELP_UPDATE_LONG);
}

// Print usage for "alias"
void print_alias_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_ALIAS_LONG2) : printf("%s", OPH_TERM_HELP_ALIAS_LONG);
}

// Print usage for "setalias"
void print_setalias_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_SETALIAS_LONG2) : printf("%s", OPH_TERM_HELP_SETALIAS_LONG);
}

// Print usage for "unsetalias"
void print_unsetalias_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_UNSETALIAS_LONG2) : printf("%s", OPH_TERM_HELP_UNSETALIAS_LONG);
}

// Print usage for "getalias"
void print_getalias_usage()
{
	(print_json) ? my_printf("%s", OPH_TERM_HELP_GETALIAS_LONG2) : printf("%s", OPH_TERM_HELP_GETALIAS_LONG);
}

// Print list of commands and env vars
void print_cmd_list()
{
	(print_json) ? my_printf("OPH_TERM COMMANDS\\n\\n") : printf("\e[1mOPH_TERM COMMANDS\e[0m\n\n");
	int i;
	for (i = 0; i < cmds_num; i++) {
		if (!strcmp(cmds[i], OPH_TERM_CMD_CLEAR)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_CLEAR_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_CLEAR_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_VERSION)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_VERSION_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_VERSION_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_WARRANTY)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_WARRANTY_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_WARRANTY_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_WATCH)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_WATCH_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_WATCH_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_CONDITIONS)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_CONDITIONS_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_CONDITIONS_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_ENV)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_ENV_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_ENV_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_GETENV)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_GETENV_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_GETENV_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_HELP)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_HELP_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_HELP_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_HISTORY)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_HISTORY_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_HISTORY_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_QUIT)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_QUIT_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_QUIT_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_EXIT)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_EXIT_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_EXIT_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_SETENV)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_SETENV_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_SETENV_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_UNSETENV)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_UNSETENV_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_UNSETENV_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_UPDATE)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_UPDATE_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_UPDATE_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_RESUME)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_RESUME_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_RESUME_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_VIEW)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_VIEW_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_VIEW_SHORT);
		}
#ifndef NO_WORKFLOW
		else if (!strcmp(cmds[i], OPH_TERM_CMD_CHECK)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_CHECK_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_CHECK_SHORT);
		}
#endif
		else if (!strcmp(cmds[i], OPH_TERM_CMD_ALIAS)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_ALIAS_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_ALIAS_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_GETALIAS)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_GETALIAS_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_GETALIAS_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_SETALIAS)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_SETALIAS_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_SETALIAS_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_UNSETALIAS)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_UNSETALIAS_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_UNSETALIAS_SHORT);
		}
#ifdef WITH_IM_SUPPORT
		else if (!strcmp(cmds[i], OPH_TERM_CMD_DEPLOY)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_DEPLOY_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_DEPLOY_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_DEPLOY_STATUS)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_DEPLOY_STATUS_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_DEPLOY_STATUS_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_GET_SERVER)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_GET_SERVER_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_GET_SERVER_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_UNDEPLOY)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_UNDEPLOY_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_UNDEPLOY_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_DEPLOYS_LIST)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_DEPLOYS_LIST_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_DEPLOYS_LIST_SHORT);
		} else if (!strcmp(cmds[i], OPH_TERM_CMD_DEPLOY_VMS_LIST)) {
			(print_json) ? my_printf("\\t%25s = %s", cmds[i], OPH_TERM_HELP_DEPLOY_VMS_LIST_SHORT) : printf("\t%25s = %s", cmds[i], OPH_TERM_HELP_DEPLOY_VMS_LIST_SHORT);
		}
#endif
	}
	(print_json) ? my_printf("\\n") : printf("\n");

	(print_json) ? my_printf("OPH_TERM ENVIRONMENT VARIABLES\\n\\n") : printf("\e[1mOPH_TERM ENVIRONMENT VARIABLES\e[0m\n\n");
	for (i = 0; i < env_vars_num; i++) {
		if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_NCORES)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_NCORES_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_NCORES_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_EXEC_MODE)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_EXEC_MODE_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_EXEC_MODE_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_PASSWD)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_PASSWD_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_PASSWD_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_SERVER_HOST)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_SERVER_HOST_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_SERVER_HOST_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_SERVER_PORT)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_SERVER_PORT_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_SERVER_PORT_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_SESSION_ID)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_SESSION_ID_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_SESSION_ID_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_TERM_PS1)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_TERM_PS1_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_TERM_PS1_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_USER)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_USER_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_USER_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_CWD)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_CWD_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_CWD_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_DATACUBE)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_DATACUBE_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_DATACUBE_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_TERM_VIEWER)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_TERM_VIEWER_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_TERM_VIEWER_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_TERM_FORMAT)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_TERM_FORMAT_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_TERM_FORMAT_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_TERM_IMGS)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_TERM_IMGS_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_TERM_IMGS_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_GRAPH_LAYOUT)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_GRAPH_LAYOUT_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_GRAPH_LAYOUT_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_RESPONSE_BUFFER)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_RESPONSE_BUFFER_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_RESPONSE_BUFFER_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_WORKFLOW_AUTOVIEW)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_WORKFLOW_AUTOVIEW_SHORT) : printf("\t%25s = %s", env_vars[i],
																  OPH_TERM_HELP_OPH_WORKFLOW_AUTOVIEW_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_TOKEN)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_TOKEN_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_TOKEN_SHORT);
		}
#ifdef WITH_IM_SUPPORT
		else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_INFRASTRUCTURE_URL_SHORT) : printf("\t%25s = %s", env_vars[i],
																   OPH_TERM_HELP_OPH_INFRASTRUCTURE_URL_SHORT);
		} else if (!strcmp(env_vars[i], OPH_TERM_ENV_OPH_AUTH_HEADER)) {
			(print_json) ? my_printf("\\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_AUTH_HEADER_SHORT) : printf("\t%25s = %s", env_vars[i], OPH_TERM_HELP_OPH_AUTH_HEADER_SHORT);
		}
#endif
	}
	(print_json) ? my_printf("\\n") : printf("\n");

	print_remote_command_usage();
	(print_json) ? my_printf("\\n") : printf("\n");

#ifndef NO_WORKFLOW
	print_remote_workflow_usage();
	(print_json) ? my_printf("\\n") : printf("\n");
#endif

	print_autocompletion_usage();
	(print_json) ? my_printf("\\n") : printf("\n");

	print_var_substitution_usage();
	(print_json) ? my_printf("\\n") : printf("\n");

	print_alias_substitution_usage();
	(print_json) ? my_printf("\\n") : printf("\n");

	print_help_usage();
	(print_json) ? my_printf("\\nUSEFUL TIP:\\n\\tIn order to get a detailed description of the latest version of an operator use this command:\\n") :
	    printf("\n\e[1mUSEFUL TIP:\e[0m\n\tIn order to get a detailed description of the latest version of an operator use this command:\n");
	(print_json) ? my_printf("\\toph_man function=name;exec_mode=sync;\\n") : printf("\toph_man function=name;exec_mode=sync;\n");
}

void print_env_var_usage(const char *var)
{
	if (!strcmp(var, OPH_TERM_ENV_OPH_PASSWD)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_PASSWD_LONG2) : printf("%s", OPH_TERM_HELP_OPH_PASSWD_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_SERVER_HOST)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_SERVER_HOST_LONG2) : printf("%s", OPH_TERM_HELP_OPH_SERVER_HOST_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_SERVER_PORT)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_SERVER_PORT_LONG2) : printf("%s", OPH_TERM_HELP_OPH_SERVER_PORT_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_SESSION_ID)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_SESSION_ID_LONG2) : printf("%s", OPH_TERM_HELP_OPH_SESSION_ID_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_USER)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_USER_LONG2) : printf("%s", OPH_TERM_HELP_OPH_USER_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_NCORES)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_NCORES_LONG2) : printf("%s", OPH_TERM_HELP_OPH_NCORES_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_EXEC_MODE)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_EXEC_MODE_LONG2) : printf("%s", OPH_TERM_HELP_OPH_EXEC_MODE_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_TERM_PS1)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_TERM_PS1_LONG2) : printf("%s", OPH_TERM_HELP_OPH_TERM_PS1_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_CWD)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_CWD_LONG2) : printf("%s", OPH_TERM_HELP_OPH_CWD_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_DATACUBE)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_DATACUBE_LONG2) : printf("%s", OPH_TERM_HELP_OPH_DATACUBE_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_TERM_VIEWER)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_TERM_VIEWER_LONG2) : printf("%s", OPH_TERM_HELP_OPH_TERM_VIEWER_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_TERM_FORMAT)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_TERM_FORMAT_LONG2) : printf("%s", OPH_TERM_HELP_OPH_TERM_FORMAT_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_TERM_IMGS)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_TERM_IMGS_LONG2) : printf("%s", OPH_TERM_HELP_OPH_TERM_IMGS_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_GRAPH_LAYOUT)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_GRAPH_LAYOUT_LONG2) : printf("%s", OPH_TERM_HELP_OPH_GRAPH_LAYOUT_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_RESPONSE_BUFFER)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_RESPONSE_BUFFER_LONG2) : printf("%s", OPH_TERM_HELP_OPH_RESPONSE_BUFFER_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_WORKFLOW_AUTOVIEW)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_WORKFLOW_AUTOVIEW_LONG2) : printf("%s", OPH_TERM_HELP_OPH_WORKFLOW_AUTOVIEW_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_TOKEN)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_TOKEN_LONG2) : printf("%s", OPH_TERM_HELP_OPH_TOKEN_LONG);
	}
#ifdef WITH_IM_SUPPORT
	else if (!strcmp(var, OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_INFRASTRUCTURE_URL_LONG2) : printf("%s", OPH_TERM_HELP_OPH_INFRASTRUCTURE_URL_LONG);
	} else if (!strcmp(var, OPH_TERM_ENV_OPH_AUTH_HEADER)) {
		(print_json) ? my_printf("%s", OPH_TERM_HELP_OPH_AUTH_HEADER_LONG2) : printf("%s", OPH_TERM_HELP_OPH_AUTH_HEADER_LONG);
	}
#endif
}

//Print usage for cmd or the list of commands if NULL
int oph_term_help(const char *cmd)
{

	if (!cmd) {
		print_cmd_list();
		return OPH_TERM_SUCCESS;
	}

	if (!strcmp(cmd, OPH_TERM_CMD_GETENV)) {
		print_getenv_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_VERSION)) {
		print_version_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_WARRANTY)) {
		print_warranty_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_WATCH)) {
		print_watch_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_CONDITIONS)) {
		print_conditions_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_ENV)) {
		print_env_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_HELP)) {
		print_help_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_HISTORY)) {
		print_history_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_QUIT)) {
		print_quit_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_EXIT)) {
		print_exit_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_SETENV)) {
		print_setenv_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_UNSETENV)) {
		print_unsetenv_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_CLEAR)) {
		print_clear_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_UPDATE)) {
		print_update_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_RESUME)) {
		print_resume_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_VIEW)) {
		print_view_usage();
	}
#ifndef NO_WORKFLOW
	else if (!strcmp(cmd, OPH_TERM_CMD_CHECK)) {
		print_check_usage();
	}
#endif
	else if (!strcmp(cmd, OPH_TERM_CMD_GETALIAS)) {
		print_getalias_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_ALIAS)) {
		print_alias_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_SETALIAS)) {
		print_setalias_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_UNSETALIAS)) {
		print_unsetalias_usage();
	}
#ifdef WITH_IM_SUPPORT
	else if (!strcmp(cmd, OPH_TERM_CMD_DEPLOY)) {
		print_deploy_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_DEPLOY_STATUS)) {
		print_deploy_status_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_GET_SERVER)) {
		print_get_server_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_UNDEPLOY)) {
		print_undeploy_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_DEPLOYS_LIST)) {
		print_deploys_list_usage();
	} else if (!strcmp(cmd, OPH_TERM_CMD_DEPLOY_VMS_LIST)) {
		print_deploy_vms_list_usage();
	}
#endif
	else if (is_env_var(cmd)) {
		print_env_var_usage(cmd);
	} else {
		(print_json) ? my_fprintf(stderr, "Error: command or variable not found\\n\\n") : fprintf(stderr, "\e[1;31mError: command or variable not found\e[0m\n\n");
		print_help_usage();
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	return OPH_TERM_SUCCESS;
}
