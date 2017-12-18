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

#include "oph_term.h"
#include <unistd.h>
#include <ncurses.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <pthread.h>

extern int errno;
extern int abort_view;

char *_passwd = NULL;
pthread_mutex_t global_flag;
char get_config = 1;

int watching = 0;
char *stored_line = NULL;
int signal_raise = 0;

int print_json = 0;
#ifdef DEBUG_LEVEL
int print_debug_data = 1;
#else
int print_debug_data = 0;
#endif
char oph_term_request[OUTPUT_MAX_LEN] = "\0";
char oph_term_jobid[OUTPUT_MAX_LEN] = "\0";
char *oph_term_response = NULL;
char oph_term_output[OUTPUT_MAX_LEN] = "\0";
int oph_term_output_cur = 0;
char oph_term_error[OUTPUT_MAX_LEN] = "\0";
int oph_term_error_cur = 0;
char *oph_base_src_path = NULL;

/* Print Oph_Term version and copyright */
void print_version()
{
	(print_json) ? my_printf("%s", OPH_TERM_VERSION2) : printf("%s", OPH_TERM_VERSION);
}

/* Print Oph_Term disclamer */
void print_disclaimer()
{
	(print_json) ? my_printf("%s", OPH_TERM_DISCLAIMER2) : printf("%s", OPH_TERM_DISCLAIMER);
}

/* Print Oph_Term welcome message */
void print_welcome()
{
	print_version();
	(print_json) ? my_printf("%s", OPH_TERM_DISCLAIMER4) : printf("%s", OPH_TERM_DISCLAIMER3);
	(print_json) ? my_printf("\\nWelcome to Oph_Term !\\n\\n") : printf("\nWelcome to Oph_Term !\n\n");
	(print_json) ? my_printf("Use the power of the Ophidia framework right from your terminal.\\n") : printf("Use the power of the Ophidia framework right from your terminal.\n");
	(print_json) ? my_printf("If you are going to use Oph_Term for the first time and need something\\n") : printf("If you are going to use Oph_Term for the first time and need something\n");
	(print_json) ? my_printf("to get you started, just try entering \\\"help\\\"\\n\\n") : printf("to get you started, just try entering \"help\"\n\n");
}

/* Print Oph_Term usage at startup */
void print_startup_usage(char *arg0, FILE * stream)
{
	(print_json) ? my_fprintf(stream, "\\n") : fprintf(stream, "\n");

	(print_json) ? my_fprintf(stream, "Usage (1): %s [--json|-j]\\n", arg0) : fprintf(stream, "Usage (1): %s [--json|-j]\n", arg0);
	(print_json) ? my_fprintf(stream, "   Start Oph_Term with no particular options.\\n") : fprintf(stream, "   Start Oph_Term with no particular options.\n");
	(print_json) ? my_fprintf(stream, "   Use the content of your shell variables OPH_USER, OPH_PASSWD,\\n") : fprintf(stream,
															   "   Use the content of your shell variables OPH_USER, OPH_PASSWD,\n");
	(print_json) ? my_fprintf(stream, "   OPH_SERVER_HOST, OPH_SERVER_PORT, OPH_TERM_PS1 etc. if defined in your shell environment\\n") : fprintf(stream,
																		      "   OPH_SERVER_HOST, OPH_SERVER_PORT, OPH_TERM_PS1 etc. if defined in your shell environment\n");
	(print_json) ? my_fprintf(stream, "   at runtime or at shell startup through files like .bashrc to set corresponding Oph_Term variables.\\n") : fprintf(stream,
																				"   at runtime or at shell startup through files like .bashrc to set corresponding Oph_Term variables.\n");
	(print_json) ? my_fprintf(stream, "   If OPH_USER,OPH_PASSWD,OPH_SERVER_HOST and OPH_SERVER_PORT are set and correct, Oph_Term will try to\\n") : fprintf(stream,
																				  "   If OPH_USER,OPH_PASSWD,OPH_SERVER_HOST and OPH_SERVER_PORT are set and correct, Oph_Term will try to\n");
	(print_json) ? my_fprintf(stream, "   interact with the server in order to download operators XMLs for autocompletion and\\n") : fprintf(stream,
																		 "   interact with the server in order to download operators XMLs for autocompletion and\n");
	(print_json) ? my_fprintf(stream, "   resume the last remote session the user connected to.\\n") : fprintf(stream, "   resume the last remote session the user connected to.\n");
	(print_json) ? my_fprintf(stream,
				  "   When the GSI interface is used, the variables OPH_USER and OPH_PASSWD are ignored and the user certificate is used for authentication.\\n") : fprintf(stream,
																							    "   When the GSI interface is used, the variables OPH_USER and OPH_PASSWD are ignored and the user certificate is used for authentication.\n");
	(print_json) ? my_fprintf(stream, "   User-defined variables can be specified using OPH_TERM_USERVAR_X (with X any string)\\n") : fprintf(stream,
																		  "   User-defined variables can be specified using OPH_TERM_USERVAR_X (with X any string)\n");
	(print_json) ? my_fprintf(stream, "   and the pattern <OPH_TERM_USERVAR_X='uservarname=uservarvalue'> (note the single quotes).\\n") : fprintf(stream,
																		       "   and the pattern <OPH_TERM_USERVAR_X='uservarname=uservarvalue'> (note the single quotes).\n");
	(print_json) ? my_fprintf(stream, "   Oph_Term will search for all of these special variables in your environment.\\n") : fprintf(stream,
																	  "   Oph_Term will search for all of these special variables in your environment.\n");
	(print_json) ? my_fprintf(stream, "   The result will be equal to calling <setenv uservarname=uservarvalue> from within Oph_Term.\\n") : fprintf(stream,
																			 "   The result will be equal to calling <setenv uservarname=uservarvalue> from within Oph_Term.\n");
	(print_json) ? my_fprintf(stream, "   User-defined aliases can be specified using OPH_TERM_ALIAS_X (with X any string)\\n") : fprintf(stream,
																	      "   User-defined aliases can be specified using OPH_TERM_ALIAS_X (with X any string)\n");
	(print_json) ? my_fprintf(stream, "   and the pattern <OPH_TERM_ALIAS_X='aliasname=aliasvalue'> (note the single quotes).\\n") : fprintf(stream,
																		 "   and the pattern <OPH_TERM_ALIAS_X='aliasname=aliasvalue'> (note the single quotes).\n");
	(print_json) ? my_fprintf(stream, "   Oph_Term will search for all of these special variables in your environment.\\n") : fprintf(stream,
																	  "   Oph_Term will search for all of these special variables in your environment.\n");
	(print_json) ? my_fprintf(stream, "   The result will be equal to calling <setalias aliasname=aliasvalue> from within Oph_Term.\\n") : fprintf(stream,
																		       "   The result will be equal to calling <setalias aliasname=aliasvalue> from within Oph_Term.\n");
	(print_json) ? my_fprintf(stream, "   With the option -j, Oph_Term will always return a JSON string with 5 keys: request, response, jobid, stdout and stderr.\\n\\n") : fprintf(stream,
																							"   With the option -j, Oph_Term will always return a JSON string with 5 keys: request, response, jobid, stdout and stderr.\n\n");

	(print_json) ? my_fprintf(stream, "Usage (2): %s [--username=user|-u user] [--password=passwd|-p passwd] [--hostname=host|-H host] [--port=port|-P port] [--json|-j]\\n",
				  arg0) : fprintf(stream, "Usage (2): %s [--username=user|-u user] [--password=passwd|-p passwd] [--hostname=host|-H host] [--port=port|-P port] [--json|-j]\n", arg0);
	(print_json) ? my_fprintf(stream, "   Start Oph_Term with one or more options:\\n") : fprintf(stream, "   Start Oph_Term with one or more options:\n");
	(print_json) ? my_fprintf(stream, "      username: remote user;\\n") : fprintf(stream, "      username: remote user;\n");
	(print_json) ? my_fprintf(stream, "      password: remote user password;\\n") : fprintf(stream, "      password: remote user password;\n");
	(print_json) ? my_fprintf(stream, "      hostname: server address;\\n") : fprintf(stream, "      hostname: server address;\n");
	(print_json) ? my_fprintf(stream, "      port: server port number.\\n") : fprintf(stream, "      port: server port number.\n");
	(print_json) ? my_fprintf(stream, "   Oph_Term will also search for not-inserted parameters in your shell environment as described above.\\n") : fprintf(stream,
																				 "   Oph_Term will also search for not-inserted parameters in your shell environment as described above.\n");
	(print_json) ? my_fprintf(stream, "   All of these parameters can also be set or modified later during Oph_Term execution.\\n") : fprintf(stream,
																		  "   All of these parameters can also be set or modified later during Oph_Term execution.\n");
	(print_json) ? my_fprintf(stream, "   If OPH_USER,OPH_PASSWD,OPH_SERVER_HOST and OPH_SERVER_PORT are set and correct, Oph_Term will try to\\n") : fprintf(stream,
																				  "   If OPH_USER,OPH_PASSWD,OPH_SERVER_HOST and OPH_SERVER_PORT are set and correct, Oph_Term will try to\n");
	(print_json) ? my_fprintf(stream, "   interact with the server in order to download operators XMLs for autocompletion and\\n") : fprintf(stream,
																		 "   interact with the server in order to download operators XMLs for autocompletion and\n");
	(print_json) ? my_fprintf(stream, "   resume the last remote session the user connected to.\\n") : fprintf(stream, "   resume the last remote session the user connected to.\n");
	(print_json) ? my_fprintf(stream,
				  "   When the GSI interface is used, the variables OPH_USER and OPH_PASSWD are ignored and the user certificate is used for authentication.\\n") : fprintf(stream,
																							    "   When the GSI interface is used, the variables OPH_USER and OPH_PASSWD are ignored and the user certificate is used for authentication.\n");
	(print_json) ? my_fprintf(stream, "   With the option -j, Oph_Term will always return a JSON string with 5 keys: request, response, jobid, stdout and stderr.\\n\\n") : fprintf(stream,
																							"   With the option -j, Oph_Term will always return a JSON string with 5 keys: request, response, jobid, stdout and stderr.\n\n");

	(print_json) ? my_fprintf(stream, "Usage (3): %s [--help|-h]\\n", arg0) : fprintf(stream, "Usage (3): %s [--help|-h]\n", arg0);
	(print_json) ? my_fprintf(stream, "   Print this help.\\n\\n") : fprintf(stream, "   Print this help.\n\n");

	(print_json) ? my_fprintf(stream, "Usage (4): %s [--version|-v]\\n", arg0) : fprintf(stream, "Usage (4): %s [--version|-v]\n", arg0);
	(print_json) ? my_fprintf(stream, "   Show Oph_Term version.\\n\\n") : fprintf(stream, "   Show Oph_Term version.\n\n");

	(print_json) ? my_fprintf(stream, "Usage (5): %s [--execute=\\\"my command\\\"|-e \\\"my command\\\"] [--json|-j] ...\\n", arg0) : fprintf(stream,
																		   "Usage (5): %s [--execute=\"my command\"|-e \"my command\"] [--json|-j] ...\n",
																		   arg0);
	(print_json) ? my_fprintf(stream, "   Execute a command without entering Oph_Term.\\n") : fprintf(stream, "   Execute a command without entering Oph_Term.\n");
	(print_json) ? my_fprintf(stream, "   Oph_Term can always be started with one or more login options as described above.\\n") : fprintf(stream,
																	       "   Oph_Term can always be started with one or more login options as described above.\n");
	(print_json) ? my_fprintf(stream, "   Oph_Term will also search for not-inserted parameters in your shell environment as described above\\n") : fprintf(stream,
																				"   Oph_Term will also search for not-inserted parameters in your shell environment as described above\n");
	(print_json) ? my_fprintf(stream, "   and will use environment variables as in interactive mode.\\n") : fprintf(stream, "   and will use environment variables as in interactive mode.\n");
	(print_json) ? my_fprintf(stream, "   If OPH_USER,OPH_PASSWD,OPH_SERVER_HOST and OPH_SERVER_PORT are set and correct, Oph_Term will try to\\n") : fprintf(stream,
																				  "   If OPH_USER,OPH_PASSWD,OPH_SERVER_HOST and OPH_SERVER_PORT are set and correct, Oph_Term will try to\n");
	(print_json) ? my_fprintf(stream, "   interact with the server in order to resume the last remote session the user connected to.\\n") : fprintf(stream,
																			"   interact with the server in order to resume the last remote session the user connected to.\n");
	(print_json) ? my_fprintf(stream,
				  "   When the GSI interface is used, the variables OPH_USER and OPH_PASSWD are ignored and the user certificate is used for authentication.\\n") : fprintf(stream,
																							    "   When the GSI interface is used, the variables OPH_USER and OPH_PASSWD are ignored and the user certificate is used for authentication.\n");
	(print_json) ? my_fprintf(stream, "   Oph_Term will always try to perform variable and alias substitution on the submitted command.\\n") : fprintf(stream,
																			   "   Oph_Term will always try to perform variable and alias substitution on the submitted command.\n");
	(print_json) ? my_fprintf(stream, "   With the option -j, Oph_Term will always return a JSON string with 5 keys: request, response, jobid, stdout and stderr.\\n\\n") : fprintf(stream,
																							"   With the option -j, Oph_Term will always return a JSON string with 5 keys: request, response, jobid, stdout and stderr.\n\n");
#ifndef NO_WORKFLOW
	(print_json) ? my_fprintf(stream, "Usage (6): %s [--workflow=file|-w file] [--args=arg1,arg2,...,argN|-a arg1,arg2,...,argN] [--json|-j] ...\\n", arg0) : fprintf(stream,
																					  "Usage (6): %s [--workflow=file|-w file] [--args=arg1,arg2,...,argN|-a arg1,arg2,...,argN] [--json|-j] ...\n",
																					  arg0);
	(print_json) ? my_fprintf(stream, "   Submit an entire workflow without entering Oph_Term.\\n") : fprintf(stream, "   Submit an entire workflow without entering Oph_Term.\n");
	(print_json) ? my_fprintf(stream, "   Oph_Term can always be started with one or more login options as described above.\\n") : fprintf(stream,
																	       "   Oph_Term can always be started with one or more login options as described above.\n");
	(print_json) ? my_fprintf(stream, "   Oph_Term will also search for not-inserted parameters in your shell environment as described above\\n") : fprintf(stream,
																				"   Oph_Term will also search for not-inserted parameters in your shell environment as described above\n");
	(print_json) ? my_fprintf(stream, "   and will use environment variables as in interactive mode.\\n") : fprintf(stream, "   and will use environment variables as in interactive mode.\n");
	(print_json) ? my_fprintf(stream, "   If OPH_USER,OPH_PASSWD,OPH_SERVER_HOST and OPH_SERVER_PORT are set and correct, Oph_Term will try to\\n") : fprintf(stream,
																				  "   If OPH_USER,OPH_PASSWD,OPH_SERVER_HOST and OPH_SERVER_PORT are set and correct, Oph_Term will try to\n");
	(print_json) ? my_fprintf(stream, "   interact with the server in order to resume the last remote session the user connected to.\\n") : fprintf(stream,
																			"   interact with the server in order to resume the last remote session the user connected to.\n");
	(print_json) ? my_fprintf(stream,
				  "   When the GSI interface is used, the variables OPH_USER and OPH_PASSWD are ignored and the user certificate is used for authentication.\\n") : fprintf(stream,
																							    "   When the GSI interface is used, the variables OPH_USER and OPH_PASSWD are ignored and the user certificate is used for authentication.\n");
	(print_json) ? my_fprintf(stream, "   OPH_SESSION_ID,OPH_EXEC_MODE,OPH_NCORES,OPH_CWD,OPH_CDD and OPH_DATACUBE will be inserted in\\n") : fprintf(stream,
																			  "   OPH_SESSION_ID,OPH_EXEC_MODE,OPH_NCORES,OPH_CWD,OPH_CDD and OPH_DATACUBE will be inserted in\n");
	(print_json) ? my_fprintf(stream, "   the JSON request (original file will remain untouched) if not already present.\\n") : fprintf(stream,
																	    "   the JSON request (original file will remain untouched) if not already present.\n");
	(print_json) ? my_fprintf(stream, "   \\\"file\\\" must be the name of a valid JSON file according to the Ophidia Workflow JSON Schema.\\n") : fprintf(stream,
																			       "   \"file\" must be the name of a valid JSON file according to the Ophidia Workflow JSON Schema.\n");
	(print_json) ? my_fprintf(stream, "   With -a it is possible to substitute placeholders like $1,$2 etc in the JSON request\\n") : fprintf(stream,
																		  "   With -a it is possible to substitute placeholders like $1,$2 etc in the JSON request\n");
	(print_json) ? my_fprintf(stream, "   with the relative argument passed in a comma-separated list.\\n") : fprintf(stream, "   with the relative argument passed in a comma-separated list.\n");
	(print_json) ? my_fprintf(stream, "   Oph_Term will always try to perform variable substitution on the submitted workflow (no alias substitution).\\n") : fprintf(stream,
																					  "   Oph_Term will always try to perform variable substitution on the submitted workflow (no alias substitution).\n");
	(print_json) ? my_fprintf(stream, "   With the option -j, Oph_Term will always return a JSON string with 5 keys: request, response, jobid, stdout and stderr.\\n\\n") : fprintf(stream,
																							"   With the option -j, Oph_Term will always return a JSON string with 5 keys: request, response, jobid, stdout and stderr.\n\n");
#endif
}

/* Reading number of secords for watching */
int oph_get_winterval(char **cursor, char **saveptr, int *watching)
{
	if (!cursor || !saveptr)
		return 1;
	if (strcmp(*cursor, "-n"))
		return 0;
	*cursor = strtok_r(NULL, " \t\n\"", saveptr);
	if (*cursor) {
		if (watching) {
			*watching = (int) strtol(*cursor, NULL, 10);
			if (*watching <= 0)
				return 1;
		}
		*cursor = strtok_r(NULL, " \t\n\"", saveptr);
	}
	return (!*cursor);
}

/* Startup options parsing */
int startup_opt_setup(int argc, char *argv[], char *envp[], HASHTBL * hashtbl, char **exec_statement)
{
	int opt = 0;
	int long_index = 0;
	static struct option long_options[] = {
		{"username", required_argument, 0, 'u'},
		{"password", required_argument, 0, 'p'},
		{"hostname", required_argument, 0, 'H'},
		{"port", required_argument, 0, 'P'},
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'v'},
		{"execute", required_argument, 0, 'e'},
		{"workflow", required_argument, 0, 'w'},
		{"args", required_argument, 0, 'a'},
		{"json", no_argument, 0, 'j'},
		{0, 0, 0, 0}
	};

	//preset viewer-related variables OPH_TERM_VIEWER and OPH_TERM_IMGS
	if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER, "basic")) {
		(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_TERM_VIEWER, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			   "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																			   OPH_TERM_ENV_OPH_TERM_VIEWER,
																			   OPH_TERM_MEMORY_ERROR);
		return OPH_TERM_MEMORY_ERROR;
	}
	if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_TERM_FORMAT, "classic")) {
		(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_TERM_FORMAT, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			   "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																			   OPH_TERM_ENV_OPH_TERM_FORMAT,
																			   OPH_TERM_MEMORY_ERROR);
		return OPH_TERM_MEMORY_ERROR;
	}
	if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS, "no_op")) {
		(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_TERM_IMGS, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			 "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																			 OPH_TERM_ENV_OPH_TERM_IMGS,
																			 OPH_TERM_MEMORY_ERROR);
		return OPH_TERM_MEMORY_ERROR;
	}
	//preset OPH_EXEC_MODE
	if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_EXEC_MODE, "sync")) {
		(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_EXEC_MODE, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			 "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																			 OPH_TERM_ENV_OPH_EXEC_MODE,
																			 OPH_TERM_MEMORY_ERROR);
		return OPH_TERM_MEMORY_ERROR;
	}
	//preset OPH_GRAPH_LAYOUT
	if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_GRAPH_LAYOUT, "vertical_compact")) {
		(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_GRAPH_LAYOUT, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			    "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																			    OPH_TERM_ENV_OPH_GRAPH_LAYOUT,
																			    OPH_TERM_MEMORY_ERROR);
		return OPH_TERM_MEMORY_ERROR;
	}
	//preset OPH_RESPONSE_BUFFER
	if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_RESPONSE_BUFFER, "1024")) {
		(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_RESPONSE_BUFFER, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			       "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																			       OPH_TERM_ENV_OPH_RESPONSE_BUFFER,
																			       OPH_TERM_MEMORY_ERROR);
		return OPH_TERM_MEMORY_ERROR;
	}
	//preset OPH_WORKFLOW_AUTOVIEW
	if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_WORKFLOW_AUTOVIEW, "off")) {
		(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_WORKFLOW_AUTOVIEW, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																				 "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																				 OPH_TERM_ENV_OPH_WORKFLOW_AUTOVIEW,
																				 OPH_TERM_MEMORY_ERROR);
		return OPH_TERM_MEMORY_ERROR;
	}
	//preset OPH_TOKEN
	if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_TOKEN, "")) {
		(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_TOKEN, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																		     "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																		     OPH_TERM_ENV_OPH_TOKEN, OPH_TERM_MEMORY_ERROR);
		return OPH_TERM_MEMORY_ERROR;
	}
	//preset useful aliases
	int z;
	for (z = 0; z < pre_defined_aliases_num; z++) {
		if (oph_term_setalias(alias_ptr, pre_defined_aliases_keys[z], pre_defined_aliases_values[z])) {
			(print_json) ? my_fprintf(stderr, "Could not set alias %s [CODE %d]\\n", pre_defined_aliases_keys[z], OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			       "\e[1;31mCould not set alias %s [CODE %d]\e[0m\n",
																			       pre_defined_aliases_keys[z],
																			       OPH_TERM_MEMORY_ERROR);
			return OPH_TERM_MEMORY_ERROR;
		}
	}

	//get default env vars from shell env
	char *value;
	int i;
	for (i = 0; i < env_vars_num; i++) {
		value = getenv(env_vars[i]);
		if (value) {
			if (oph_term_setenv(hashtbl, env_vars[i], value)) {
				(print_json) ? my_fprintf(stderr, "Could not set variable [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr, "\e[1;31mCould not set variable [CODE %d]\e[0m\n",
																	  OPH_TERM_MEMORY_ERROR);
				return OPH_TERM_MEMORY_ERROR;
			}
		}
	}

	//get user-defined env vars from shell env
	char buf[OPH_TERM_MAX_LEN];
	for (i = 0; envp[i]; i++) {
		if (!strncmp(envp[i], OPH_TERM_USERVAR_PATTERN, 17)) {
			memset(buf, 0, OPH_TERM_MAX_LEN);
			snprintf(buf, OPH_TERM_MAX_LEN, "%s", envp[i]);
			size_t q, z;
			for (q = 0; q < strlen(buf); q++)
				if (buf[q] == '=')
					break;
			if (q == strlen(buf)) {
				(print_json) ? my_fprintf(stderr, "Invalid format for %s [CODE %d]\\n", buf, OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		     "\e[1;31mInvalid format for %s [CODE %d]\e[0m\n",
																		     buf, OPH_TERM_INVALID_PARAM_VALUE);
				return OPH_TERM_INVALID_PARAM_VALUE;
			}
			for (z = q + 1; z < strlen(buf); z++)
				if (buf[z] == '=')
					break;
			if (z == strlen(buf)) {
				(print_json) ? my_fprintf(stderr, "Invalid format for %s [CODE %d]\\n", buf, OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		     "\e[1;31mInvalid format for %s [CODE %d]\e[0m\n",
																		     buf, OPH_TERM_INVALID_PARAM_VALUE);
				return OPH_TERM_INVALID_PARAM_VALUE;
			}
			buf[z] = '\0';
			if (oph_term_setenv(hashtbl, buf + q + 1, buf + z + 1)) {
				(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", buf + q + 1, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			  "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																			  buf + q + 1, OPH_TERM_MEMORY_ERROR);
				return OPH_TERM_MEMORY_ERROR;
			}
		}
	}

	//get user-defined aliases from shell env
	for (i = 0; envp[i]; i++) {
		if (!strncmp(envp[i], OPH_TERM_ALIAS_PATTERN, 15)) {
			memset(buf, 0, OPH_TERM_MAX_LEN);
			snprintf(buf, OPH_TERM_MAX_LEN, "%s", envp[i]);
			size_t q, z;
			for (q = 0; q < strlen(buf); q++)
				if (buf[q] == '=')
					break;
			if (q == strlen(buf)) {
				(print_json) ? my_fprintf(stderr, "Invalid format for %s [CODE %d]\\n", buf, OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		     "\e[1;31mInvalid format for %s [CODE %d]\e[0m\n",
																		     buf, OPH_TERM_INVALID_PARAM_VALUE);
				return OPH_TERM_INVALID_PARAM_VALUE;
			}
			for (z = q + 1; z < strlen(buf); z++)
				if (buf[z] == '=')
					break;
			if (z == strlen(buf)) {
				(print_json) ? my_fprintf(stderr, "Invalid format for %s [CODE %d]\\n", buf, OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		     "\e[1;31mInvalid format for %s [CODE %d]\e[0m\n",
																		     buf, OPH_TERM_INVALID_PARAM_VALUE);
				return OPH_TERM_INVALID_PARAM_VALUE;
			}
			buf[z] = '\0';
			if (oph_term_setalias(alias_ptr, buf + q + 1, buf + z + 1)) {
				(print_json) ? my_fprintf(stderr, "Could not set alias %s [CODE %d]\\n", buf + q + 1, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																		       "\e[1;31mCould not set alias %s [CODE %d]\e[0m\n",
																		       buf + q + 1, OPH_TERM_MEMORY_ERROR);
				return OPH_TERM_MEMORY_ERROR;
			}
		}
	}

	while ((opt = getopt_long(argc, argv, "hvu:p:H:P:e:w:a:jt:xz", long_options, &long_index)) != -1) {
		switch (opt) {
			case 'h':
				print_startup_usage(argv[0], stdout);
				if (*exec_statement) {
					free(*exec_statement);
					*exec_statement = NULL;
				}
				return OPH_TERM_GENERIC_ERROR;
			case 'v':
				print_version();
				print_disclaimer();
				if (*exec_statement) {
					free(*exec_statement);
					*exec_statement = NULL;
				}
				return OPH_TERM_GENERIC_ERROR;
			case 'x':
				print_version();
				(print_json) ? my_printf("%s", OPH_TERM_WARRANTY2) : printf("%s", OPH_TERM_WARRANTY);
				if (*exec_statement) {
					free(*exec_statement);
					*exec_statement = NULL;
				}
				return OPH_TERM_GENERIC_ERROR;
			case 'z':
				print_version();
				(print_json) ? my_printf("%s", OPH_TERM_CONDITIONS2) : printf("%s", OPH_TERM_CONDITIONS);
				if (*exec_statement) {
					free(*exec_statement);
					*exec_statement = NULL;
				}
				return OPH_TERM_GENERIC_ERROR;
			case 'u':
				if (!optarg) {
					(print_json) ? my_fprintf(stderr, "Error: no username!\\n\\n") : fprintf(stderr, "\e[1;31mError: no username!\e[0m\n\n");
					print_startup_usage(argv[0], stderr);
					if (*exec_statement) {
						free(*exec_statement);
						*exec_statement = NULL;
					}
					return OPH_TERM_OPTION_NOT_RECOGNIZED;
				} else {
					if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_USER, optarg)) {
						if (*exec_statement) {
							free(*exec_statement);
							*exec_statement = NULL;
						}
						return OPH_TERM_MEMORY_ERROR;
					}
					break;
				}
			case 'p':
				if (!optarg) {
					(print_json) ? my_fprintf(stderr, "Error: no password!\\n\\n") : fprintf(stderr, "\e[1;31mError: no password!\e[0m\n\n");
					print_startup_usage(argv[0], stderr);
					if (*exec_statement) {
						free(*exec_statement);
						*exec_statement = NULL;
					}
					return OPH_TERM_OPTION_NOT_RECOGNIZED;
				} else {
					if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_PASSWD, optarg)) {
						if (*exec_statement) {
							free(*exec_statement);
							*exec_statement = NULL;
						}
						return OPH_TERM_MEMORY_ERROR;
					}
					break;
				}
			case 'H':
				if (!optarg) {
					(print_json) ? my_fprintf(stderr, "Error: no hostname!\\n\\n") : fprintf(stderr, "\e[1;31mError: no hostname!\e[0m\n\n");
					print_startup_usage(argv[0], stderr);
					if (*exec_statement) {
						free(*exec_statement);
						*exec_statement = NULL;
					}
					return OPH_TERM_OPTION_NOT_RECOGNIZED;
				} else {
					if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST, optarg)) {
						if (*exec_statement) {
							free(*exec_statement);
							*exec_statement = NULL;
						}
						return OPH_TERM_MEMORY_ERROR;
					}
					break;
				}
			case 'P':
				if (!optarg) {
					(print_json) ? my_fprintf(stderr, "Error: no port number!\\n\\n") : fprintf(stderr, "\e[1;31mError: no port number!\e[0m\n\n");
					print_startup_usage(argv[0], stderr);
					if (*exec_statement) {
						free(*exec_statement);
						*exec_statement = NULL;
					}
					return OPH_TERM_OPTION_NOT_RECOGNIZED;
				} else {
					if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT, optarg)) {
						if (*exec_statement) {
							free(*exec_statement);
							*exec_statement = NULL;
						}
						return OPH_TERM_MEMORY_ERROR;
					}
					break;
				}
			case 'e':
				if (!optarg) {
					(print_json) ? my_fprintf(stderr, "Error: no statement to execute!\\n\\n") : fprintf(stderr, "\e[1;31mError: no statement to execute!\e[0m\n\n");
					print_startup_usage(argv[0], stderr);
					if (*exec_statement) {
						free(*exec_statement);
						*exec_statement = NULL;
					}
					return OPH_TERM_OPTION_NOT_RECOGNIZED;
				} else {
					if (*exec_statement) {
						free(*exec_statement);
						*exec_statement = NULL;
					}
					if (optarg[0] == '\"' || optarg[0] == '\'')
						*exec_statement = strdup(optarg + 1);
					else
						*exec_statement = strdup(optarg);
					if (!*exec_statement) {
						(print_json) ? my_fprintf(stderr, "Error allocating memory for exec_statement!\\n\\n") : fprintf(stderr,
																		 "\e[1;31mError allocating memory for exec_statement!\e[0m\n\n");
						return OPH_TERM_MEMORY_ERROR;
					}
					if ((*exec_statement)[strlen(*exec_statement) - 1] == '\"' || (*exec_statement)[strlen(*exec_statement) - 1] == '\'')
						(*exec_statement)[strlen(*exec_statement) - 1] = '\0';
					break;
				}
			case 'w':
				if (!optarg) {
					(print_json) ? my_fprintf(stderr, "Error: no filename!\\n\\n") : fprintf(stderr, "\e[1;31mError: no filename!\e[0m\n\n");
					print_startup_usage(argv[0], stderr);
					if (*exec_statement) {
						free(*exec_statement);
						*exec_statement = NULL;
					}
					return OPH_TERM_OPTION_NOT_RECOGNIZED;
				} else {
					if (oph_term_setenv(hashtbl, OPH_TERM_OPT_W_FILENAME, optarg)) {
						if (*exec_statement) {
							free(*exec_statement);
							*exec_statement = NULL;
						}
						return OPH_TERM_MEMORY_ERROR;
					}
					break;
				}
			case 'a':
				if (!optarg) {
					(print_json) ? my_fprintf(stderr, "Error: no args!\\n\\n") : fprintf(stderr, "\e[1;31mError: no args!\e[0m\n\n");
					print_startup_usage(argv[0], stderr);
					if (*exec_statement) {
						free(*exec_statement);
						*exec_statement = NULL;
					}
					return OPH_TERM_OPTION_NOT_RECOGNIZED;
				} else {
					if (oph_term_setenv(hashtbl, OPH_TERM_OPT_W_ARGS, optarg)) {
						if (*exec_statement) {
							free(*exec_statement);
							*exec_statement = NULL;
						}
						return OPH_TERM_MEMORY_ERROR;
					}
					break;
				}
			case 'j':
				print_json = 1;
				break;
			case 't':
				if (!optarg) {
					(print_json) ? my_fprintf(stderr, "Error: no token!\\n\\n") : fprintf(stderr, "\e[1;31mError: no token!\e[0m\n\n");
					print_startup_usage(argv[0], stderr);
					if (*exec_statement) {
						free(*exec_statement);
						*exec_statement = NULL;
					}
					return OPH_TERM_OPTION_NOT_RECOGNIZED;
				} else {
					if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_TOKEN, optarg)) {
						if (*exec_statement) {
							free(*exec_statement);
							*exec_statement = NULL;
						}
						return OPH_TERM_MEMORY_ERROR;
					}
					break;
				}
		}
	}

	return OPH_TERM_SUCCESS;
}

/* Oph_Term base generator */
char *oph_term_base_generator(const char *text, int state)
{
	static int list_index, len;
	char *name;
	int i;

	if (!state) {
		list_index = 0;
		len = strlen(text);
	}
	// Get useful stuff
	char **cmds_list = cmds;
	int cmds_list_size = cmds_num;
	char **def_vars_list = env_vars;
	int def_vars_list_size = env_vars_num;
	char **user_vars_list = NULL;
	int user_vars_list_num = 0;
	char **aliases_list = NULL;
	int aliases_list_num = 0;

	/* Return the next name which partially matches from the lists. */
	while (list_index < cmds_list_size) {
		name = cmds_list[list_index];
		list_index++;

		if (strncmp(name, text, len) == 0)
			return strdup(name);
	}
	while (list_index >= cmds_list_size && list_index < (cmds_list_size + def_vars_list_size)) {
		name = def_vars_list[list_index - cmds_list_size];
		list_index++;

		if (strncmp(name, text, len) == 0)
			return strdup(name);
	}
	oph_term_get_user_vars(env_vars_ptr, &user_vars_list, &user_vars_list_num);
	if (user_vars_list_num) {
		while (list_index >= (cmds_list_size + def_vars_list_size) && list_index < (cmds_list_size + def_vars_list_size + user_vars_list_num)) {
			name = user_vars_list[list_index - (cmds_list_size + def_vars_list_size)];
			list_index++;

			if (strncmp(name, text, len) == 0) {
				char *match = strdup(name);
				for (i = 0; i < user_vars_list_num; i++)
					free(user_vars_list[i]);
				free(user_vars_list);
				return match;
			}
		}
		for (i = 0; i < user_vars_list_num; i++)
			free(user_vars_list[i]);
		free(user_vars_list);
	}
	oph_term_get_aliases(alias_ptr, &aliases_list, &aliases_list_num);
	if (aliases_list_num) {
		while (list_index >= (cmds_list_size + def_vars_list_size + user_vars_list_num) && list_index < (cmds_list_size + def_vars_list_size + user_vars_list_num + aliases_list_num)) {
			name = aliases_list[list_index - (cmds_list_size + def_vars_list_size + user_vars_list_num)];
			list_index++;

			if (strncmp(name, text, len) == 0) {
				char *match = strdup(name);
				for (i = 0; i < aliases_list_num; i++)
					free(aliases_list[i]);
				free(aliases_list);
				return match;
			}
		}
		for (i = 0; i < aliases_list_num; i++)
			free(aliases_list[i]);
		free(aliases_list);
	}

	/* If no names matched, then return NULL. */
	return NULL;
}

/* Oph_Term env vars generator */
char *oph_term_vars_generator(const char *text, int state)
{
	static int list_index, len;
	char *name;
	int i;

	if (!state) {
		list_index = 0;
		len = strlen(text);
	}
	// Get useful stuff
	char **def_vars_list = env_vars;
	int def_vars_list_size = env_vars_num;
	char **user_vars_list = NULL;
	int user_vars_list_num = 0;

	/* Return the next name which partially matches from the lists. */
	while (list_index < def_vars_list_size) {
		name = def_vars_list[list_index];
		list_index++;

		if (strncmp(name, text, len) == 0)
			return strdup(name);
	}
	oph_term_get_user_vars(env_vars_ptr, &user_vars_list, &user_vars_list_num);
	if (user_vars_list_num) {
		while (list_index >= def_vars_list_size && list_index < (def_vars_list_size + user_vars_list_num)) {
			name = user_vars_list[list_index - def_vars_list_size];
			list_index++;

			if (strncmp(name, text, len) == 0) {
				char *match = strdup(name);
				for (i = 0; i < user_vars_list_num; i++)
					free(user_vars_list[i]);
				free(user_vars_list);
				return match;
			}
		}
		for (i = 0; i < user_vars_list_num; i++)
			free(user_vars_list[i]);
		free(user_vars_list);
	}

	/* If no names matched, then return NULL. */
	return NULL;
}

/* Oph_Term operators generator */
char *oph_term_operators_generator(const char *text, int state)
{
	static int list_index, len;
	char *name;

	if (!state) {
		list_index = 0;
		len = strlen(text);
	}

	/* Return the next name which partially matches from the operators list. */
	if (operators_list_size == 0)
		return NULL;
	while (list_index < operators_list_size) {
		name = operators_list[list_index];
		list_index++;

		if (strncmp(name, text, len) == 0) {
			return strdup(name);
		}
	}

	/* If no names matched, then return NULL. */
	return NULL;
}

/* Oph_Term arguments generator */
char *oph_term_arguments_generator(const char *text, int state)
{
	static int list_index, len;
	char *name;
	char opname[100];
	operator_argument *args = NULL;
	int args_size = 0;
	int i, j, jj;
	int k = 0;

	if (!state) {
		list_index = 0;
		len = strlen(text);
	}

	int bracket = 0;
	for (jj = 0; jj < rl_end; jj++) {
		if (rl_line_buffer[jj] == '[')
			bracket++;
		else if ((rl_line_buffer[jj] == ']') && (bracket > 0))
			bracket--;
	}
	// Retrieve opname from buffer
	memset(opname, 0, 100);
	for (i = 0; i < rl_end; i++) {
		if (rl_line_buffer[i] != ' ')
			break;
	}
	if (i == rl_end)
		return NULL;
	if (bracket)
		sprintf(opname, OPH_TERM_CMD_MASSIVE);
	else {
		for (j = i; j < rl_end; j++) {
			if (rl_line_buffer[j] == ' ') {
				break;
			} else {
				opname[k] = rl_line_buffer[j];
				k++;
			}
		}
		if (!strcmp(opname, OPH_TERM_CMD_WATCH)) {
			k = 0;
			memset(opname, 0, 100);
			for (i = j; i < rl_end; i++) {
				if (rl_line_buffer[i] != ' ')
					break;
			}
			if (i == rl_end)
				return NULL;
			for (j = i; j < rl_end; j++) {
				if (rl_line_buffer[j] == ' ') {
					break;
				} else {
					opname[k] = rl_line_buffer[j];
					k++;
				}
			}
		}
	}

	/* Return the next name which partially matches from the arguments list. */
	if (oph_term_env_xmlauto_get_parameters(xml_defs, opname, &args, &args_size)) {
		rl_on_new_line();
		return NULL;
	}
	while (list_index < args_size) {
		name = args[list_index].name;
		list_index++;

		if (strncmp(name, text, len) == 0) {
			return strdup(name);
		}
	}

	/* If no names matched, then return NULL. */
	return NULL;
}

/* Oph_Term arguments values generator */
char *oph_term_arg_values_generator(const char *text, int state)
{
	static int list_index, len;
	//char *name;
	char opname[100];
	char argname[100];
	int argindex = -1;
	operator_argument *args = NULL;
	int args_size = 0;
	int i, j, jj;
	int k = 0;

	if (!state) {
		list_index = 0;
		len = strlen(text);
	}

	int bracket = 0;
	for (jj = 0; jj < rl_end; jj++) {
		if (rl_line_buffer[jj] == '[')
			bracket++;
		else if ((rl_line_buffer[jj] == ']') && (bracket > 0))
			bracket--;
	}
	// Retrieve opname from buffer
	memset(opname, 0, 100);
	for (i = 0; i < rl_end; i++) {
		if (rl_line_buffer[i] != ' ')
			break;
	}
	if (i == rl_end)
		return NULL;
	if (bracket)
		sprintf(opname, OPH_TERM_CMD_MASSIVE);
	else {
		for (j = i; j < rl_end; j++) {
			if (rl_line_buffer[j] == ' ') {
				break;
			} else {
				opname[k] = rl_line_buffer[j];
				k++;
			}
		}
		if (!strcmp(opname, OPH_TERM_CMD_WATCH)) {
			k = 0;
			memset(opname, 0, 100);
			for (i = j; i < rl_end; i++) {
				if (rl_line_buffer[i] != ' ')
					break;
			}
			if (i == rl_end)
				return NULL;
			for (j = i; j < rl_end; j++) {
				if (rl_line_buffer[j] == ' ') {
					break;
				} else {
					opname[k] = rl_line_buffer[j];
					k++;
				}
			}
		}
	}

	// Retrieve argname from buffer
	memset(argname, 0, 100);
	for (i = rl_end; i >= 0; i--) {
		if (rl_line_buffer[i] == ' ' || rl_line_buffer[i] == ';')
			break;
	}
	if (i == -1)
		return NULL;
	k = 0;
	for (j = i + 1; j < rl_end; j++) {
		if (rl_line_buffer[j] == '=') {
			break;
		} else {
			argname[k] = rl_line_buffer[j];
			k++;
		}
	}

	/* Return the next name which partially matches from the arguments list. */
	if (oph_term_env_xmlauto_get_parameters(xml_defs, opname, &args, &args_size)) {
		rl_on_new_line();
		return NULL;
	}
	// Retrieve argindex
	for (i = 0; i < args_size; i++) {
		if (!strcmp(args[i].name, argname)) {
			argindex = i;
			break;
		}
	}
	if (argindex == -1)
		return NULL;

	if (args[argindex].default_val[0] == '\0' && args[argindex].values[0] == '\0')
		return NULL;

	if (args[argindex].values[0] != '\0') {
		// parse values
		int values_num = 1;
		for (i = 0; i < (int) strlen(args[argindex].values); i++) {
			if (args[argindex].values[i] == '|')
				values_num++;
		}

		if (values_num == 1) {
			if (list_index == 0 && strncmp(args[argindex].values, text, len) == 0) {
				list_index++;
				return strdup(args[argindex].values);
			}
			/* If no names matched, then return NULL. */
			return NULL;
		}

		char buf[200];
		char *ptr = NULL, *saveptr = NULL;
		while (list_index < values_num) {
			memset(buf, 0, 200);
			snprintf(buf, 200, "%s", args[argindex].values);
			for (i = 0; i <= list_index; i++) {
				if (i == 0) {
					ptr = strtok_r(buf, "|", &saveptr);
				} else {
					if (!ptr)
						break;
					ptr = strtok_r(NULL, "|", &saveptr);
				}
			}

			if (!ptr)
				return NULL;

			list_index++;

			if (strncmp(ptr, text, len) == 0) {
				return strdup(ptr);
			}
		}

		/* If no names matched, then return NULL. */
		return NULL;
	} else {
		// compare default
		if (list_index == 0 && strncmp(args[argindex].default_val, text, len) == 0) {
			list_index++;
			return strdup(args[argindex].default_val);
		}
		/* If no names matched, then return NULL. */
		return NULL;
	}
}

/* Oph_Term cmds generator */
char *oph_term_oph_prefix_generator(const char *text, int state)
{
	static int list_index, len;

	if (!state) {
		list_index = 0;
		len = strlen(text);
	}

	/* Return the next name which partially matches from the command list. */
	while (list_index < 1) {
		list_index++;

		if (strncmp(OPH_TERM_REMOTE_CMD_PREFIX, text, len) == 0)
			return strdup(OPH_TERM_REMOTE_CMD_PREFIX);
	}

	/* If no names matched, then return NULL. */
	return NULL;
}

/* Oph_Term massive generator */
char *oph_term_massive_generator(const char *text, int state)
{
	static int list_index;

	if (!state) {
		list_index = 0;
	}

	/* Return the next name which partially matches from the command list. */
	while (list_index < 1) {
		list_index++;

		return strdup(text);
	}

	/* If no names matched, then return NULL. */
	return NULL;
}

/* Oph_Term completion function */
char **oph_term_completion(char *text, int start, int end)
{
	char **matches;
	matches = (char **) NULL;
	{
		(void) (end);
	}

	rl_attempted_completion_over = 1;

	if (text[0] == '.' || text[0] == '/') {
		int i = 0;
		char chddir[OPH_TERM_MAX_LEN];
		if (oph_base_src_path && (strlen(oph_base_src_path) > 1) && (*text == '/')) {
			snprintf(chddir, OPH_TERM_MAX_LEN, "%s%s", oph_base_src_path, text);
			text = chddir;
			i = 1;
		}
		// completion over local filesystem
		matches = rl_completion_matches(text, rl_filename_completion_function);
		if (oph_base_src_path && i && matches) {
			for (i = 0; matches[i]; i++) {
				snprintf(chddir, OPH_TERM_MAX_LEN, "%s", matches[i] + strlen(oph_base_src_path));
				free(matches[i]);
				matches[i] = strdup(chddir);
			}
		}
	} else if (rl_line_buffer[(start - 1 < 0) ? start : start - 1] == '$'
		   || (rl_line_buffer[(start - 2 < 0) ? start : start - 2] == '$' && rl_line_buffer[(start - 1 < 0) ? start : start - 1] == '{')) {
		// completion over env vars (def+user)
		if (rl_line_buffer[(start - 1 < 0) ? start : start - 1] == '$')
			rl_completion_suppress_append = 1;
		else
			rl_completion_append_character = '}';
		matches = rl_completion_matches(text, oph_term_vars_generator);
	} else {
		if (!strncmp(text, OPH_TERM_ENV_OPH_PREFIX, OPH_TERM_ENV_OPH_PREFIX_LEN)) {
			//completion over operators
			matches = rl_completion_matches(text, oph_term_operators_generator);
		} else {
			short int j, jj, bracket = 0;
			for (j = 0; j < rl_end; j++) {
				if (rl_line_buffer[j] != ' ')
					break;
			}

			size_t p = strlen(OPH_TERM_CMD_WATCH);
			if (!strncmp(rl_line_buffer + j, OPH_TERM_CMD_WATCH, p))
				for (p++; p && (j < rl_end) && rl_line_buffer[j]; p--)
					j++;

			if (j != rl_end && !strncmp(rl_line_buffer + j, OPH_TERM_ENV_OPH_PREFIX, OPH_TERM_ENV_OPH_PREFIX_LEN)) {
				for (jj = 0; jj < rl_end; jj++) {
					if (rl_line_buffer[jj] == '[')
						bracket++;
					else if ((rl_line_buffer[jj] == ']') && (bracket > 0))
						bracket--;
				}

				if (text && (text[0] == '[')) {
					rl_basic_word_break_characters = " \t\n\"\\'`@$><=;|&{([";
					rl_completer_word_break_characters = " \t\n\"\\'`@$><=;|&{([";
					rl_completion_suppress_append = 1;
					matches = rl_completion_matches(text, oph_term_massive_generator);
				} else if (text && strlen(text) && (text[strlen(text) - 1] == ']')) {
					rl_completion_append_character = ';';
					matches = rl_completion_matches(text, oph_term_massive_generator);
				} else {
					if (rl_line_buffer[(start - 1 < 0) ? start : start - 1] == '=') {
						//completion over operator parameters values
						rl_completion_append_character = ';';
						matches = rl_completion_matches(text, oph_term_arg_values_generator);
						if (matches && rl_completion_type != 37) {
							char opname[100];
							char argname[100];
							int argindex = -1;
							operator_argument *args = NULL;
							int args_size = 0;
							short int k;
							short int q = 0;
							short int i = 1;
							memset(opname, 0, 100);
							if (bracket)
								sprintf(opname, OPH_TERM_CMD_MASSIVE);
							else
								for (k = j; k < rl_end; k++) {
									if (rl_line_buffer[k] == ' ') {
										break;
									} else {
										opname[q] = rl_line_buffer[k];
										q++;
									}
								}
							memset(argname, 0, 100);
							for (k = rl_end; k >= 0; k--) {
								if (rl_line_buffer[k] == ' ' || rl_line_buffer[k] == ';')
									break;
							}
							if (k != -1) {
								q = 0;
								for (j = k + 1; j < rl_end; j++) {
									if (rl_line_buffer[j] == '=') {
										break;
									} else {
										argname[q] = rl_line_buffer[j];
										q++;
									}
								}
								if (!oph_term_env_xmlauto_get_parameters(xml_defs, opname, &args, &args_size)) {
									for (k = 0; k < args_size; k++) {
										if (!strcmp(args[k].name, argname)) {
											argindex = k;
											break;
										}
									}
									if (argindex != -1) {
										while (matches[i]) {
											if (!strcmp(matches[i], args[argindex].default_val)) {
												char *tmpmatch = (char *) malloc(40);
												if (tmpmatch) {
													memset(tmpmatch, 0, 40);
													snprintf(tmpmatch, 40, "(%s)", matches[i]);
													free(matches[i]);
													matches[i] = tmpmatch;
												}
												break;
											}
											i++;
										}
									}
								}
							}
						}
					} else {
						//completion over operator parameters
						rl_completion_append_character = '=';
						matches = rl_completion_matches(text, oph_term_arguments_generator);
						if (matches && rl_completion_type != 37) {
							char opname[100];
							operator_argument *args = NULL;
							int args_size = 0;
							short int k;
							short int q = 0;
							short int i = 1;
							memset(opname, 0, 100);
							if (bracket)
								sprintf(opname, OPH_TERM_CMD_MASSIVE);
							else
								for (k = j; k < rl_end; k++) {
									if (rl_line_buffer[k] == ' ') {
										break;
									} else {
										opname[q] = rl_line_buffer[k];
										q++;
									}
								}
							if (!oph_term_env_xmlauto_get_parameters(xml_defs, opname, &args, &args_size)) {
								while (matches[i]) {
									for (k = 0; k < args_size; k++) {
										if (!strcmp(matches[i], args[k].name)) {
											if (args[k].mandatory == 'M') {
												if (args[k].values[0] != '\0') {
													char *tmpmatch = (char *) malloc(200);
													if (tmpmatch) {
														memset(tmpmatch, 0, 200);
														snprintf(tmpmatch, 200, "** %s [%s]", args[k].name, args[k].values);
														free(matches[i]);
														matches[i] = tmpmatch;
													}
												} else {
													char *tmpmatch = (char *) malloc(200);
													if (tmpmatch) {
														memset(tmpmatch, 0, 200);
														snprintf(tmpmatch, 200, "** %s", args[k].name);
														free(matches[i]);
														matches[i] = tmpmatch;
													}
												}
											} else {
												if (args[k].values[0] != '\0' && args[k].default_val[0] != '\0') {
													char *tmpmatch = (char *) malloc(200);
													if (tmpmatch) {
														memset(tmpmatch, 0, 200);
														snprintf(tmpmatch, 200, "%s [%s (%s)]", args[k].name, args[k].values,
															 args[k].default_val);
														free(matches[i]);
														matches[i] = tmpmatch;
													}
												} else if (args[k].values[0] != '\0' && args[k].default_val[0] == '\0') {
													char *tmpmatch = (char *) malloc(200);
													if (tmpmatch) {
														memset(tmpmatch, 0, 200);
														snprintf(tmpmatch, 200, "%s [%s]", args[k].name, args[k].values);
														free(matches[i]);
														matches[i] = tmpmatch;
													}
												} else if (args[k].values[0] == '\0' && args[k].default_val[0] != '\0') {
													char *tmpmatch = (char *) malloc(200);
													if (tmpmatch) {
														memset(tmpmatch, 0, 200);
														snprintf(tmpmatch, 200, "%s (%s)", args[k].name, args[k].default_val);
														free(matches[i]);
														matches[i] = tmpmatch;
													}
												}
											}
											break;
										}
									}
									i++;
								}
							}
						}
					}
				}
			} else {
				if (text && (text[0] == 'o')) {
					//return "oph_"
					rl_completion_suppress_append = 1;
					matches = rl_completion_matches(text, oph_term_oph_prefix_generator);
				} else {
					// completion over env vars (def+user) and cmds and alias
					matches = rl_completion_matches(text, oph_term_base_generator);
				}
			}
		}
	}

	if (rl_completion_type == 37) {
		// in case of menu-completion suppress append char
		rl_completion_suppress_append = 1;
	}

	return (matches);
}

/* Expands escape sequences within a C-string
 *
 * src must be a C-string with a NUL terminator
 *
 * dest should be long enough to store the resulting expanded
 * string. A string of size 2 * strlen(src) + 1 will always be sufficient
 *
 * NUL characters are not expanded to \0 (otherwise how would we know when
 * the input string ends?)
 */
void expand_escapes(char *dest, char *src)
{
	char c;
	char *ptr = src;
	char *ptr2 = dest;

	while ((c = *(ptr++))) {
		switch (c) {
			case '\a':
				*(ptr2++) = '\\';
				*(ptr2++) = 'a';
				break;
			case '\b':
				*(ptr2++) = '\\';
				*(ptr2++) = 'b';
				break;
			case '\t':
				*(ptr2++) = '\\';
				*(ptr2++) = 't';
				break;
			case '\n':
				*(ptr2++) = '\\';
				*(ptr2++) = 'n';
				break;
			case '\v':
				*(ptr2++) = '\\';
				*(ptr2++) = 'v';
				break;
			case '\f':
				*(ptr2++) = '\\';
				*(ptr2++) = 'f';
				break;
			case '\r':
				*(ptr2++) = '\\';
				*(ptr2++) = 'r';
				break;
			case '\"':
				if (*(ptr - 2) != '\\') {
					*(ptr2++) = '\\';
					*(ptr2++) = '\"';
				} else
					*(ptr2++) = c;
				break;
			default:
				*(ptr2++) = c;
		}
	}

	*ptr2 = '\0';		/* Ensure nul terminator */
}

/* Print output as JSON */
void print_oph_term_output_json(HASHTBL * hashtbl)
{
	char oph_term_request_exp[2 * OUTPUT_MAX_LEN] = "\0";
	char oph_term_jobid_exp[2 * OUTPUT_MAX_LEN] = "\0";
	char oph_term_output_exp[2 * OUTPUT_MAX_LEN] = "\0";
	char oph_term_error_exp[2 * OUTPUT_MAX_LEN] = "\0";

	expand_escapes(oph_term_request_exp, oph_term_request);
	expand_escapes(oph_term_jobid_exp, oph_term_jobid);
	expand_escapes(oph_term_output_exp, oph_term_output);
	expand_escapes(oph_term_error_exp, oph_term_error);

	if (!print_debug_data && oph_term_response && hashtbl && ((int) strlen(oph_term_response) > 1024 * strtol((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_RESPONSE_BUFFER), NULL, 10)))
		fprintf(stderr, "\e[1;31mResponse exceeds the maximum size %s kB.\e[0m\n", (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_RESPONSE_BUFFER));
	else
		printf(OUTPUT_JSON_STRING, oph_term_request_exp, oph_term_jobid_exp, oph_term_response ? oph_term_response : "{}", oph_term_output_exp, oph_term_error_exp);

	memset(oph_term_request, 0, OUTPUT_MAX_LEN);
	memset(oph_term_jobid, 0, OUTPUT_MAX_LEN);
	memset(oph_term_output, 0, OUTPUT_MAX_LEN);
	memset(oph_term_error, 0, OUTPUT_MAX_LEN);
	if (oph_term_response) {
		free(oph_term_response);
		oph_term_response = NULL;
	}
	oph_term_output_cur = 0;
	oph_term_error_cur = 0;

	return;
}

pthread_t tid;

void siginthandler(int signum)
{
	(void) signum;
	abort_view = 1;
	if (watching) {
		(print_json) ? my_fprintf(stderr, "\\n") : fprintf(stderr, "\n");
		signal_raise = 1;
	} else
		(print_json) ? my_fprintf(stderr, "Use 'quit' or 'exit' to terminate Oph_Term! Use 'kill' to cancel jobs!\\n") : fprintf(stderr,
																	 "\e[1;31m Use 'quit' or 'exit' to terminate Oph_Term! Use 'kill' to cancel jobs!\e[0m\n");
	if (pthread_cancel(tid)) {
		rl_replace_line("", 0);
		rl_forced_update_display();
	}
	if (stored_line) {
		free(stored_line);
		stored_line = NULL;
	}
	watching = 0;
}

/* Oph_Term Main program */
int main(int argc, char **argv, char **envp)
{

	signal(SIGINT, siginthandler);
	signal(SIGTSTP, siginthandler);
	signal(SIGQUIT, siginthandler);

	pthread_mutex_init(&global_flag, NULL);

	char *line = NULL;
	char *linecopy = NULL;
	char *cursor = NULL;
	char *exec_statement = NULL;
	char *hist_path = NULL;
	char *xml_path = NULL;
	char xml_path_extended[OPH_TERM_MAX_LEN];
	char fixed_cursor[OPH_TERM_MAX_LEN];
	char *current_operator = NULL;
	char submission_string[OPH_TERM_MAX_LEN];
	char command_line[OPH_TERM_MAX_LEN];
	FILE *hist_file;
	HIST_ENTRY **hist_list = NULL;
	HASHTBL *hashtbl = NULL;
	HASHTBL *aliases = NULL;
	int i, res, n = 0;
	short int exec_one_statement = 0;
	int oph_term_return = OPH_TERM_SUCCESS;
	char *saveptr;
	short int exec_alias = 0, exec_alias2;
	int alias_substitutions = 0;

	/* INIT ENV */
	res = oph_term_env_init(&hashtbl);
	if (res) {
		(print_json) ? my_fprintf(stderr, "Unable to initialize environment [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr, "\e[1;31mUnable to initialize environment [CODE %d]\e[0m\n",
																    OPH_TERM_MEMORY_ERROR);
		if (print_json)
			print_oph_term_output_json(hashtbl);
		return OPH_TERM_MEMORY_ERROR;
	}
	env_vars_ptr = hashtbl;
	res = oph_term_alias_init(&aliases);
	if (res) {
		(print_json) ? my_fprintf(stderr, "Unable to initialize aliases [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr, "\e[1;31mUnable to initialize aliases [CODE %d]\e[0m\n",
																OPH_TERM_MEMORY_ERROR);
		oph_term_env_clear(hashtbl);
		if (print_json)
			print_oph_term_output_json(hashtbl);
		return OPH_TERM_MEMORY_ERROR;
	}
	alias_ptr = aliases;
	res = startup_opt_setup(argc, argv, envp, hashtbl, &exec_statement);
	if (res) {
		oph_term_env_clear(hashtbl);
		oph_term_alias_clear(aliases);
		if (print_json)
			print_oph_term_output_json(hashtbl);
		return OPH_TERM_GENERIC_ERROR;
	}
	// Set command line (excluding passwords)
	memset(command_line, 0, OPH_TERM_MAX_LEN);
	for (i = 0; i < argc; i++) {
		if (!strncmp(argv[i], "--password=", 11) || (!strncmp(argv[i], "-p", 2) && argv[i][2] != 0))
			continue;
		if (!strncmp(argv[i], "--password", 10) || (!strncmp(argv[i], "-p", 2) && argv[i][2] == 0)) {
			i++;
			continue;
		}
		n += snprintf(command_line + n, OPH_TERM_MAX_LEN - n, "%s ", argv[i]);
	}

	if (!print_json)
		printf("\e[2m");

	char *_user = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_USER);
	char token_user[10], *_token = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TOKEN);

	pthread_mutex_lock(&global_flag);
	_passwd = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_PASSWD);
	if (_token && strlen(_token)) {
		strcpy(token_user, OPH_TERM_TOKEN_USER);
		_user = token_user;
		_passwd = _token;
	}
	pthread_mutex_unlock(&global_flag);

#ifdef CHDDIR
	if (oph_term_env_oph_get_config
	    (OPH_TERM_OPH_BASE_SRC_PATH_KEY, hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST), hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), &oph_term_return, &oph_base_src_path, _user,
	     _passwd, 1, hashtbl)) {
		(print_json) ? my_fprintf(stderr, "\\nWarning: Unable to resume base src path\\n") : fprintf(stderr, "\e[2m\nWarning: Unable to resume base src path\e[0m\n");
		oph_term_return = OPH_TERM_SUCCESS;
		if (oph_base_src_path) {
			free(oph_base_src_path);
			oph_base_src_path = NULL;
		}
	}
#endif

	// Init OPH_SESSION_ID if empty
	if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) {
#ifndef INTERFACE_TYPE_IS_GSI
		if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST) && hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT) && _user && _passwd) {
#else
		if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST) && hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT)) {
#endif
			(print_json) ? my_printf("Resuming last session...") : printf("Resuming last session...");
			// Retrieve last sessionid from oph_server here
			char *last_sessionid = NULL;
			if (oph_term_env_oph_get_config
			    (OPH_TERM_SESSION_ID_KEY, hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST), hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), &oph_term_return, &last_sessionid, _user,
			     _passwd, 1, hashtbl)) {
				(print_json) ? my_fprintf(stderr, "\\nWarning: Unable to resume last session\\n") : fprintf(stderr, "\e[2m\nWarning: Unable to resume last session\e[0m\n");
				oph_term_return = OPH_TERM_SUCCESS;
				if (last_sessionid) {
					free(last_sessionid);
					last_sessionid = NULL;
				}
			}
			if (last_sessionid && strlen(last_sessionid)) {
				if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID, last_sessionid)) {
					oph_term_env_clear(hashtbl);
					oph_term_alias_clear(aliases);
					(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_SESSION_ID, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																						  "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																						  OPH_TERM_ENV_OPH_SESSION_ID,
																						  OPH_TERM_MEMORY_ERROR);
					if (!print_json)
						printf("\e[0m");
					if (exec_statement) {
						free(exec_statement);
						exec_statement = NULL;
					}
					free(last_sessionid);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					return OPH_TERM_MEMORY_ERROR;
				}
				(print_json) ? my_printf(" Done.\\nCurrent session is now \\\"%s\\\".\\n", last_sessionid) : printf(" Done.\nCurrent session is now \"%s\".\n", last_sessionid);
				free(last_sessionid);
				last_sessionid = NULL;

				// Init OPH_CWD if empty
				if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CWD)) {
					char *last_cwd = NULL;
					if (oph_term_env_oph_get_config
					    (OPH_TERM_OPH_CWD_KEY, hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST), hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), &oph_term_return, &last_cwd,
					     _user, _passwd, 1, hashtbl)) {
						(print_json) ? my_fprintf(stderr, "\\nWarning: Unable to resume last cwd\\n") : fprintf(stderr, "\e[2m\nWarning: Unable to resume last cwd\e[0m\n");
						oph_term_return = OPH_TERM_SUCCESS;
						if (last_cwd) {
							free(last_cwd);
							last_cwd = NULL;
						}
					}
					if (last_cwd && strlen(last_cwd)) {
						if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_CWD, last_cwd)) {
							oph_term_env_clear(hashtbl);
							oph_term_alias_clear(aliases);
							(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_CWD, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																							   "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																							   OPH_TERM_ENV_OPH_CWD,
																							   OPH_TERM_MEMORY_ERROR);
							if (!print_json)
								printf("\e[0m");
							if (exec_statement) {
								free(exec_statement);
								exec_statement = NULL;
							}
							free(last_cwd);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							return OPH_TERM_MEMORY_ERROR;
						}
						(print_json) ? my_printf("Last working directory was \\\"%s\\\".\\n", last_cwd) : printf("Last working directory was \"%s\".\n", last_cwd);
						free(last_cwd);
						last_cwd = NULL;

					} else {
						if (last_cwd) {
							free(last_cwd);
							last_cwd = NULL;
						}
						if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_CWD, "/")) {
							oph_term_env_clear(hashtbl);
							oph_term_alias_clear(aliases);
							(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_CWD, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																							   "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																							   OPH_TERM_ENV_OPH_CWD,
																							   OPH_TERM_MEMORY_ERROR);
							if (!print_json)
								printf("\e[0m");
							if (exec_statement) {
								free(exec_statement);
								exec_statement = NULL;
							}
							if (print_json)
								print_oph_term_output_json(hashtbl);
							return OPH_TERM_MEMORY_ERROR;
						}
						(print_json) ? my_printf("There is no cwd to resume. Current working directory is now \\\"/\\\" (session's root folder).\\n") :
						    printf("\e[2mThere is no cwd to resume. Current working directory is now \"/\" (session's root folder).\e[0m\n");
					}
				}
				// Init OPH_DATACUBE if empty
				if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_DATACUBE)) {
					char *last_cube = NULL;
					if (oph_term_env_oph_get_config
					    (OPH_TERM_OPH_DATACUBE_KEY, hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST), hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), &oph_term_return,
					     &last_cube, _user, _passwd, 1, hashtbl)) {
						(print_json) ? my_fprintf(stderr, "\\nWarning: Unable to resume last datacube\\n") : fprintf(stderr,
																	     "\e[2m\nWarning: Unable to resume last datacube\e[0m\n");
						oph_term_return = OPH_TERM_SUCCESS;
						if (last_cube) {
							free(last_cube);
							last_cube = NULL;
						}
					}
					if (last_cube && strlen(last_cube)) {
						if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_DATACUBE, last_cube)) {
							oph_term_env_clear(hashtbl);
							oph_term_alias_clear(aliases);
							(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_DATACUBE, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																								"\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																								OPH_TERM_ENV_OPH_DATACUBE,
																								OPH_TERM_MEMORY_ERROR);
							if (!print_json)
								printf("\e[0m");
							if (exec_statement) {
								free(exec_statement);
								exec_statement = NULL;
							}
							free(last_cube);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							return OPH_TERM_MEMORY_ERROR;
						}
						(print_json) ? my_printf("Last produced datacube was \\\"%s\\\".\\n", last_cube) : printf("Last produced datacube was \"%s\".\n", last_cube);
						free(last_cube);
						last_cube = NULL;

					} else {
						if (last_cube) {
							free(last_cube);
							last_cube = NULL;
						}
						(print_json) ? my_fprintf(stderr, "\\nWarning: There is no datacube to resume\\n") : fprintf(stderr,
																	     "\e[2m\nWarning: There is no datacube to resume\e[0m\n");
					}
				}
			} else {
				(print_json) ? my_fprintf(stderr, "\\nWarning: There is no session to resume\\n") : fprintf(stderr, "\e[2m\nWarning: There is no session to resume\e[0m\n");
				if (last_sessionid) {
					free(last_sessionid);
					last_sessionid = NULL;
				}
			}

			// Init OPH_CDD if empty
			char *is_cdd_already_set = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CDD);
			if (!is_cdd_already_set || oph_base_src_path) {

				char oph_cdd[OPH_TERM_MAX_LEN], *_oph_cdd = oph_cdd;
				if (oph_base_src_path) {
					_oph_cdd = getcwd(oph_cdd, OPH_TERM_MAX_LEN);
					char chddir[OPH_TERM_MAX_LEN], *_chddir = chddir, step = 0;
					snprintf(chddir, OPH_TERM_MAX_LEN, "%s", oph_base_src_path);
					while (*_oph_cdd && *_chddir && (*_oph_cdd == *_chddir)) {
						_oph_cdd++;
						_chddir++;
						step++;
					}
					if (*_chddir) {
						oph_term_env_clear(hashtbl);
						oph_term_alias_clear(aliases);
						(print_json) ? my_fprintf(stderr, "Directory \\\"%s\\\" cannot be used as data directory [CODE %d]\\n", oph_cdd,
									  OPH_TERM_GENERIC_ERROR) : fprintf(stderr, "\e[1;31mDirectory \"%s\" cannot be used as data directory [CODE %d]\e[0m\n",
													    oph_cdd, OPH_TERM_GENERIC_ERROR);
						if (!print_json)
							printf("\e[0m");
						if (exec_statement) {
							free(exec_statement);
							exec_statement = NULL;
						}
						if (print_json)
							print_oph_term_output_json(hashtbl);
						return OPH_TERM_MEMORY_ERROR;
					}
					if (*_oph_cdd)
						while (step && (*_oph_cdd != '/')) {
							_oph_cdd--;
							step--;
					} else
						_oph_cdd = strcpy(oph_cdd, "/");
					(print_json) ? my_printf("Current data directory is \\\"%s\\\".\\n", _oph_cdd) : printf("Current data directory is \"%s\".\n", _oph_cdd);
					if (is_cdd_already_set)
						oph_term_unsetenv(hashtbl, OPH_TERM_ENV_OPH_CDD);
				} else {
					char *last_cdd = NULL;
					if (oph_term_env_oph_get_config
					    (OPH_TERM_OPH_CDD_KEY, hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST), hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), &oph_term_return, &last_cdd,
					     _user, _passwd, 1, hashtbl)) {
						(print_json) ? my_fprintf(stderr, "\\nWarning: Unable to resume last data directory\\n") : fprintf(stderr,
																		   "\e[2m\nWarning: Unable to resume last data directory\e[0m\n");
						oph_term_return = OPH_TERM_SUCCESS;
						if (last_cdd) {
							free(last_cdd);
							last_cdd = NULL;
						}
					}
					if (last_cdd && strlen(last_cdd)) {
						snprintf(oph_cdd, OPH_TERM_MAX_LEN, "%s", last_cdd);
						(print_json) ? my_printf("Last data directory was \\\"%s\\\".\\n", last_cdd) : printf("Last data directory was \"%s\".\n", last_cdd);
					} else {
						strcpy(oph_cdd, "/");
						(print_json) ? my_printf("There is no cdd to resume. Current data directory is now \\\"/\\\".\\n") :
						    printf("\e[2mThere is no cdd to resume. Current data directory is now \"/\".\e[0m\n");
					}
					if (last_cdd) {
						free(last_cdd);
						last_cdd = NULL;
					}
				}

				if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_CDD, _oph_cdd)) {
					oph_term_env_clear(hashtbl);
					oph_term_alias_clear(aliases);
					(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_CDD, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																					   "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																					   OPH_TERM_ENV_OPH_CDD,
																					   OPH_TERM_MEMORY_ERROR);
					if (!print_json)
						printf("\e[0m");
					if (exec_statement) {
						free(exec_statement);
						exec_statement = NULL;
					}
					if (print_json)
						print_oph_term_output_json(hashtbl);
					return OPH_TERM_MEMORY_ERROR;
				}
			}
		}
	}

	(print_json) ? my_printf("\\n") : printf("\e[0m\n");

#ifndef NO_WORKFLOW
	/* Workflow submission if requested, then view result and exit */
	if (hashtbl_get(hashtbl, OPH_TERM_OPT_W_FILENAME)) {
		if (exec_statement) {
			free(exec_statement);
			exec_statement = NULL;
		}
#ifndef INTERFACE_TYPE_IS_GSI
		if (!_user) {
			(print_json) ? my_fprintf(stderr, "OPH_USER not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mOPH_USER not set [CODE %d]\e[0m\n",
																   OPH_TERM_INVALID_PARAM_VALUE);
			oph_term_env_clear(hashtbl);
			oph_term_alias_clear(aliases);
			if (print_json)
				print_oph_term_output_json(hashtbl);
			return OPH_TERM_INVALID_PARAM_VALUE;
		}
		if (!_passwd) {
			(print_json) ? my_fprintf(stderr, "OPH_PASSWD not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mOPH_PASSWD not set [CODE %d]\e[0m\n",
																     OPH_TERM_INVALID_PARAM_VALUE);
			oph_term_env_clear(hashtbl);
			oph_term_alias_clear(aliases);
			if (print_json)
				print_oph_term_output_json(hashtbl);
			return OPH_TERM_INVALID_PARAM_VALUE;
		}
#endif
		if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST)) {
			(print_json) ? my_fprintf(stderr, "OPH_SERVER_HOST not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mOPH_SERVER_HOST not set [CODE %d]\e[0m\n",
																	  OPH_TERM_INVALID_PARAM_VALUE);
			oph_term_env_clear(hashtbl);
			oph_term_alias_clear(aliases);
			if (print_json)
				print_oph_term_output_json(hashtbl);
			return OPH_TERM_INVALID_PARAM_VALUE;
		}
		if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT)) {
			(print_json) ? my_fprintf(stderr, "OPH_SERVER_PORT not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mOPH_SERVER_PORT not set [CODE %d]\e[0m\n",
																	  OPH_TERM_INVALID_PARAM_VALUE);
			oph_term_env_clear(hashtbl);
			oph_term_alias_clear(aliases);
			if (print_json)
				print_oph_term_output_json(hashtbl);
			return OPH_TERM_INVALID_PARAM_VALUE;
		}
		if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER) ||
		    (strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "dump") &&
		     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "basic") &&
		     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "coloured") &&
		     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "extended") && strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "extended_coloured"))) {
			(print_json) ? my_fprintf(stderr, "OPH_TERM_VIEWER not set or incorrect [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		       "\e[1;31mOPH_TERM_VIEWER not set or incorrect [CODE %d]\e[0m\n",
																		       OPH_TERM_INVALID_PARAM_VALUE);
			oph_term_env_clear(hashtbl);
			oph_term_alias_clear(aliases);
			if (print_json)
				print_oph_term_output_json(hashtbl);
			return OPH_TERM_INVALID_PARAM_VALUE;
		}
		// LOAD WORKFLOW FROM FILE
		char *submission_workflow = NULL;
		if (oph_term_read_file((char *) hashtbl_get(hashtbl, OPH_TERM_OPT_W_FILENAME), &submission_workflow)) {
			(print_json) ? my_fprintf(stderr, "Error loading workflow from file\\n") : fprintf(stderr, "\e[1;31mError loading workflow from file\e[0m\n");
			oph_term_env_clear(hashtbl);
			oph_term_alias_clear(aliases);
			if (submission_workflow) {
				free(submission_workflow);
				submission_workflow = NULL;
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			return OPH_TERM_GENERIC_ERROR;
		}
		// DO ARGUMENT SUBSTITUTION
		if (hashtbl_get(hashtbl, OPH_TERM_OPT_W_ARGS)) {
			char *tmpsaveptr = NULL, *tmpworkflow = NULL, *tmparg = NULL;
			int count = 1;
			char var[10];
			tmparg = strtok_r((char *) hashtbl_get(hashtbl, OPH_TERM_OPT_W_ARGS), OPH_TERM_OPT_W_ARGS_DELIMITER, &tmpsaveptr);
			while (tmparg) {
				memset(var, 0, 10);
				snprintf(var, 10, "%d", count);
				if (oph_term_var_expansion(submission_workflow, var, tmparg, &tmpworkflow)) {
					(print_json) ? my_fprintf(stderr, "Error performing argument substitution\\n") : fprintf(stderr, "\e[1;31mError performing argument substitution\e[0m\n");
					oph_term_env_clear(hashtbl);
					oph_term_alias_clear(aliases);
					if (submission_workflow) {
						free(submission_workflow);
						submission_workflow = NULL;
					}
					if (print_json)
						print_oph_term_output_json(hashtbl);
					return OPH_TERM_GENERIC_ERROR;
				}
				free(submission_workflow);
				submission_workflow = tmpworkflow;
				tmpworkflow = NULL;
				tmparg = strtok_r(NULL, OPH_TERM_OPT_W_ARGS_DELIMITER, &tmpsaveptr);
				count++;
			}
		}
		// DO VARIABLE SUBSTITUTION
		char *tmpworkflow = NULL;
		int error;
		if (oph_term_full_var_expansion(submission_workflow, hashtbl, &tmpworkflow)) {
			(print_json) ? my_fprintf(stderr, "Error performing variable substitution\\n") : fprintf(stderr, "\e[1;31mError performing variable substitution\e[0m\n");
			oph_term_env_clear(hashtbl);
			oph_term_alias_clear(aliases);
			if (submission_workflow) {
				free(submission_workflow);
				submission_workflow = NULL;
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			return OPH_TERM_GENERIC_ERROR;
		}
		free(submission_workflow);
		submission_workflow = tmpworkflow;
		tmpworkflow = NULL;

		// VALIDATION
		oph_workflow *tmp_workflow = NULL;
		if (oph_workflow_load(submission_workflow, "", &tmp_workflow)) {
			(print_json) ? my_fprintf(stderr, "Workflow is not a valid Ophidia Workflow JSON file.\\n") : fprintf(stderr,
															      "\e[1;31mWorkflow is not a valid Ophidia Workflow JSON file.\e[0m\n");
			oph_term_env_clear(hashtbl);
			oph_term_alias_clear(aliases);
			if (submission_workflow) {
				free(submission_workflow);
				submission_workflow = NULL;
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			return OPH_TERM_INVALID_PARAM_VALUE;
		}
		if (oph_workflow_indexing(tmp_workflow->tasks, tmp_workflow->tasks_num)) {
			(print_json) ? my_fprintf(stderr, "There are some problems with the tasks (duplicates, loops, etc.)\\n") : fprintf(stderr,
																	   "\e[1;31mThere are some problems with the tasks (duplicates, loops, etc.)\e[0m\n");
			(print_json) ? my_fprintf(stderr, "Workflow is not a valid Ophidia Workflow JSON file.\\n") : fprintf(stderr,
															      "\e[1;31mWorkflow is not a valid Ophidia Workflow JSON file.\e[0m\n");
			oph_term_env_clear(hashtbl);
			oph_term_alias_clear(aliases);
			if (submission_workflow) {
				free(submission_workflow);
				submission_workflow = NULL;
			}
			oph_workflow_free(tmp_workflow);
			if (print_json)
				print_oph_term_output_json(hashtbl);
			return OPH_TERM_INVALID_PARAM_VALUE;
		}
		if ((error = oph_workflow_validate(tmp_workflow))) {
			switch (error) {
				case OPH_WORKFLOW_EXIT_TASK_NAME_ERROR:
					(print_json) ? my_fprintf(stderr, "There are tasks with the same name\\n") : fprintf(stderr, "\e[1;31mThere are tasks with the same name\e[0m\n");
					break;
				case OPH_WORKFLOW_EXIT_FLOW_CONTROL_ERROR:
					(print_json) ? my_fprintf(stderr, "Flow control operators are not set correctly\\n") : fprintf(stderr,
																       "\e[1;31mFlow control operators are not set correctly\e[0m\n");
					break;
				default:
					(print_json) ? my_fprintf(stderr, "Probably the workflow is not a DAG\\n") : fprintf(stderr, "\e[1;31mProbably the workflow is not a DAG\e[0m\n");
			}
			(print_json) ? my_fprintf(stderr, "Workflow is not a valid Ophidia Workflow JSON file.\\n") : fprintf(stderr,
															      "\e[1;31mWorkflow is not a valid Ophidia Workflow JSON file.\e[0m\n");
			oph_term_env_clear(hashtbl);
			oph_term_alias_clear(aliases);
			if (submission_workflow) {
				free(submission_workflow);
				submission_workflow = NULL;
			}
			oph_workflow_free(tmp_workflow);
			if (print_json)
				print_oph_term_output_json(hashtbl);
			return OPH_TERM_INVALID_PARAM_VALUE;
		}
		oph_workflow_free(tmp_workflow);

		// SUBMISSION
		char *response_for_viewer = NULL;
		char *session = NULL;
		int last_jobid = 0;
		if (hashtbl_insert(hashtbl, OPH_TERM_ENV_LAST_JOBID, "get", strlen("get") + 1)) {
			(print_json) ? my_fprintf(stderr, "Error retrieving jobid\\n") : fprintf(stderr, "\e[1;31mError retrieving jobid\e[0m\n");
			oph_term_env_clear(hashtbl);
			oph_term_alias_clear(aliases);
			if (submission_workflow) {
				free(submission_workflow);
				submission_workflow = NULL;
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			return OPH_TERM_MEMORY_ERROR;
		}
		oph_term_client(command_line, submission_workflow, &session, _user, _passwd, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST),
				(char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), &oph_term_return, NULL, &response_for_viewer, 0, hashtbl);
		if (submission_workflow) {
			free(submission_workflow);
			submission_workflow = NULL;
		}
		if (session) {
			free(session);
			session = NULL;
		}
		if (hashtbl_get(hashtbl, OPH_TERM_ENV_LAST_JOBID)) {
			last_jobid = (int) strtol((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_LAST_JOBID), NULL, 10);
			hashtbl_remove(hashtbl, OPH_TERM_ENV_LAST_JOBID);
		}
		if (!print_debug_data && response_for_viewer && ((int) strlen(response_for_viewer) > 1024 * strtol((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_RESPONSE_BUFFER), NULL, 10))) {
			if (response_for_viewer)
				free(response_for_viewer);
			response_for_viewer = NULL;
			(print_json) ? my_fprintf(stderr, "Response exceeds the maximum size %s kB.\\n", (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_RESPONSE_BUFFER)) : fprintf(stderr,
																						    "\e[1;31mResponse exceeds the maximum size %s kB.\e[0m\n",
																						    (char *)
																						    hashtbl_get(hashtbl,
																								OPH_TERM_ENV_OPH_RESPONSE_BUFFER));
		}
		// VISUALIZATION
		if (response_for_viewer) {
			char *newtoken = NULL, *exectime = NULL;
			int viewer_res = oph_term_viewer((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), &response_for_viewer,
							 (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1)) ? ((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1)) : "red", 0, 0, 1, NULL,
							 NULL, NULL,
							 &newtoken, &exectime, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_GRAPH_LAYOUT));
			if (viewer_res != 0 && viewer_res != OPH_TERM_ERROR_WITHIN_JSON) {
				(print_json) ? my_fprintf(stderr, "Could not render result [CODE %d]\\n", OPH_TERM_GENERIC_ERROR) : fprintf(stderr, "\e[1;31mCould not render result [CODE %d]\e[0m\n",
																	    OPH_TERM_GENERIC_ERROR);
				oph_term_env_clear(hashtbl);
				oph_term_alias_clear(aliases);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (newtoken)
					free(newtoken);
				if (exectime)
					free(exectime);
				return OPH_TERM_GENERIC_ERROR;
			}
			if (newtoken) {
				pthread_mutex_lock(&global_flag);
				hashtbl_remove(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
				hashtbl_insert(hashtbl, OPH_TERM_ENV_OPH_TOKEN, newtoken, strlen(newtoken) + 1);
				_passwd = _token = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
				pthread_mutex_unlock(&global_flag);
				free(newtoken);
			}
			if (exectime) {
				(print_json) ? my_printf("Execution time: %s\\n", oph_print_exectime(&exectime)) : printf("Execution time: %s\n", oph_print_exectime(&exectime));
				free(exectime);
			}
		}
		if (print_json)
			print_oph_term_output_json(hashtbl);

		if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_WORKFLOW_AUTOVIEW) && last_jobid > 0) {
			char *autoview = strdup((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_WORKFLOW_AUTOVIEW));
			if (!autoview) {
				(print_json) ? my_fprintf(stderr, "Could not parse OPH_WORKFLOW_AUTOVIEW [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			 "\e[1;31mCould not parse OPH_WORKFLOW_AUTOVIEW [CODE %d]\e[0m\n",
																			 OPH_TERM_MEMORY_ERROR);
				oph_term_env_clear(hashtbl);
				oph_term_alias_clear(aliases);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				return OPH_TERM_GENERIC_ERROR;
			}
			char *token;
			char *svptr;
			token = strtok_r(autoview, "_", &svptr);
			if (!token) {
				free(autoview);
				(print_json) ? my_fprintf(stderr, "Invalid OPH_WORKFLOW_AUTOVIEW value [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			      "\e[1;31mInvalid OPH_WORKFLOW_AUTOVIEW value [CODE %d]\e[0m\n",
																			      OPH_TERM_INVALID_PARAM_VALUE);
				oph_term_env_clear(hashtbl);
				oph_term_alias_clear(aliases);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				return OPH_TERM_GENERIC_ERROR;
			}
			if (strcmp(token, "on") && strcmp(token, "off")) {
				free(autoview);
				(print_json) ? my_fprintf(stderr, "Invalid OPH_WORKFLOW_AUTOVIEW value [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			      "\e[1;31mInvalid OPH_WORKFLOW_AUTOVIEW value [CODE %d]\e[0m\n",
																			      OPH_TERM_INVALID_PARAM_VALUE);
				oph_term_env_clear(hashtbl);
				oph_term_alias_clear(aliases);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				return OPH_TERM_GENERIC_ERROR;
			}
			if (!strcmp(token, "on")) {
				int iterations = 0;
				int interval = 5;
				token = strtok_r(NULL, "_", &svptr);
				if (token) {
					iterations = (int) strtol((const char *) token, NULL, 10);
					if ((iterations == 0 && strcmp((const char *) token, "0")) || iterations < 0) {
						free(autoview);
						(print_json) ? my_fprintf(stderr, "Invalid OPH_WORKFLOW_AUTOVIEW value [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																					      "\e[1;31mInvalid OPH_WORKFLOW_AUTOVIEW value [CODE %d]\e[0m\n",
																					      OPH_TERM_INVALID_PARAM_VALUE);
						oph_term_env_clear(hashtbl);
						oph_term_alias_clear(aliases);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						return OPH_TERM_GENERIC_ERROR;
					}
					token = strtok_r(NULL, "_", &svptr);
					if (token) {
						interval = (int) strtol((const char *) token, NULL, 10);
						if (interval < 1) {
							free(autoview);
							(print_json) ? my_fprintf(stderr, "Invalid OPH_WORKFLOW_AUTOVIEW value [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																						      "\e[1;31mInvalid OPH_WORKFLOW_AUTOVIEW value [CODE %d]\e[0m\n",
																						      OPH_TERM_INVALID_PARAM_VALUE);
							oph_term_env_clear(hashtbl);
							oph_term_alias_clear(aliases);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							return OPH_TERM_GENERIC_ERROR;
						}
					}
				}
				// call "view last_jobid iterations interval"
				char view_string[OPH_TERM_MAX_LEN];
				memset(view_string, 0, OPH_TERM_MAX_LEN);
				snprintf(view_string, OPH_TERM_MAX_LEN, "view %d %d %d", last_jobid, iterations, interval);
				exec_statement = strdup((const char *) view_string);
				if (!exec_statement) {
					free(autoview);
					(print_json) ? my_fprintf(stderr, "Memory error calling view [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																		     "\e[1;31mMemory error calling view [CODE %d]\e[0m\n",
																		     OPH_TERM_MEMORY_ERROR);
					oph_term_env_clear(hashtbl);
					oph_term_alias_clear(aliases);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					return OPH_TERM_GENERIC_ERROR;
				}
			}
			free(autoview);
		}

		if (!exec_statement) {
			oph_term_env_clear(hashtbl);
			oph_term_alias_clear(aliases);
			return OPH_TERM_SUCCESS;
		}
	}			// workflow submission END
#endif

	// make sure opt_w tmp vars are cleared
	if (hashtbl_get(hashtbl, OPH_TERM_OPT_W_FILENAME))
		oph_term_unsetenv(hashtbl, OPH_TERM_OPT_W_FILENAME);
	if (hashtbl_get(hashtbl, OPH_TERM_OPT_W_ARGS))
		oph_term_unsetenv(hashtbl, OPH_TERM_OPT_W_ARGS);

	char *myhome = getenv("HOME");
	if (!myhome) {
		oph_term_env_clear(hashtbl);
		oph_term_alias_clear(aliases);
		if (exec_statement) {
			free(exec_statement);
			exec_statement = NULL;
		}
		(print_json) ? my_fprintf(stderr, "Unable to get HOME variable [CODE %d]\\n", OPH_TERM_GENERIC_ERROR) : fprintf(stderr, "\e[1;31mUnable to get HOME variable [CODE %d]\e[0m\n",
																OPH_TERM_GENERIC_ERROR);
		if (print_json)
			print_oph_term_output_json(hashtbl);
		return OPH_TERM_GENERIC_ERROR;
	}
	hist_path = malloc(strlen(myhome) + strlen(OPH_TERM_HISTORY_FILE) + 1);
	if (!hist_path) {
		oph_term_env_clear(hashtbl);
		oph_term_alias_clear(aliases);
		(print_json) ? my_fprintf(stderr, "Unable to set history path [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr, "\e[1;31mUnable to set history path [CODE %d]\e[0m\n",
															      OPH_TERM_MEMORY_ERROR);
		if (exec_statement) {
			free(exec_statement);
			exec_statement = NULL;
		}
		if (print_json)
			print_oph_term_output_json(hashtbl);
		return OPH_TERM_MEMORY_ERROR;
	}
	snprintf(hist_path, strlen(myhome) + strlen(OPH_TERM_HISTORY_FILE) + 1, OPH_TERM_HISTORY_FILE, myhome);

	if (!exec_statement) {
		xml_path = malloc(strlen(myhome) + strlen(OPH_TERM_XML_FOLDER) + 1);
		if (!xml_path) {
			oph_term_env_clear(hashtbl);
			oph_term_alias_clear(aliases);
			free(hist_path);
			(print_json) ? my_fprintf(stderr, "Unable to set XML folder path [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																	 "\e[1;31mUnable to set XML folder path [CODE %d]\e[0m\n",
																	 OPH_TERM_MEMORY_ERROR);
			if (exec_statement) {
				free(exec_statement);
				exec_statement = NULL;
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			return OPH_TERM_MEMORY_ERROR;
		}
		snprintf(xml_path, strlen(myhome) + strlen(OPH_TERM_XML_FOLDER) + 1, OPH_TERM_XML_FOLDER, myhome);
		if (mkdir(xml_path, 0777)) {
			if (errno != EEXIST) {
				oph_term_env_clear(hashtbl);
				oph_term_alias_clear(aliases);
				free(hist_path);
				free(xml_path);
				(print_json) ? my_fprintf(stderr, "Unable to create XML folder [CODE %d]\\n", OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																		"\e[1;31mUnable to create XML folder [CODE %d]\e[0m\n",
																		OPH_TERM_GENERIC_ERROR);
				if (exec_statement) {
					free(exec_statement);
					exec_statement = NULL;
				}
				if (print_json)
					print_oph_term_output_json(hashtbl);
				return OPH_TERM_GENERIC_ERROR;
			}
		}

		if (!print_json)
			printf("\e[2m");

		/* INIT/UPDATE XML FOLDER IF VAR SET */
#ifndef INTERFACE_TYPE_IS_GSI
		if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST) && hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT) && _user && _passwd) {
#else
		if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST) && hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT)) {
#endif
			// Retrieve correct URL from oph_server here
			char *tmpurl = NULL;
			if (oph_term_env_oph_get_config
			    (OPH_TERM_XML_URL_KEY, hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST), hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), &oph_term_return, &tmpurl, _user, _passwd, 1,
			     hashtbl)) {
				(print_json) ? my_fprintf(stderr, "Warning: Unable to get XML folder\\n") : fprintf(stderr, "\e[2mWarning: Unable to get XML folder\e[0m\n");
				oph_term_return = OPH_TERM_SUCCESS;
				if (tmpurl) {
					free(tmpurl);
					tmpurl = NULL;
				}
			}
			if (tmpurl) {
				memset(xml_path_extended, 0, OPH_TERM_MAX_LEN);
				snprintf(xml_path_extended, OPH_TERM_MAX_LEN, "%s/%s", xml_path, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST));
				if (mkdir(xml_path_extended, 0777)) {
					if (errno != EEXIST) {
						oph_term_env_clear(hashtbl);
						oph_term_alias_clear(aliases);
						free(hist_path);
						free(xml_path);
						free(tmpurl);
						(print_json) ? my_fprintf(stderr, "Unable to create XML folder %s [CODE %d]\\n", xml_path_extended, OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																						      "\e[1;31mUnable to create XML folder %s [CODE %d]\e[0m\n",
																						      xml_path_extended,
																						      OPH_TERM_GENERIC_ERROR);
						if (exec_statement) {
							free(exec_statement);
							exec_statement = NULL;
						}
						if (print_json)
							print_oph_term_output_json(hashtbl);
						return OPH_TERM_GENERIC_ERROR;
					}
				}
				oph_term_env_update_xml(tmpurl, NULL, xml_path_extended);
				free(tmpurl);
				(print_json) ? my_printf("\\n") : printf("\n");

				/* INIT XML DEFS */
				oph_term_env_start_xml_parser();
				if (oph_term_env_xmlauto_init(&xml_defs, xml_path_extended)) {
					oph_term_env_end_xml_parser();
					oph_term_env_clear(hashtbl);
					oph_term_alias_clear(aliases);
					free(hist_path);
					free(xml_path);
					if (exec_statement) {
						free(exec_statement);
						exec_statement = NULL;
					}
					if (print_json)
						print_oph_term_output_json(hashtbl);
					return OPH_TERM_GENERIC_ERROR;
				}
				if (oph_term_env_xmlauto_get_operators(xml_defs, &operators_list, &operators_list_size)) {
					oph_term_env_end_xml_parser();
					oph_term_env_clear(hashtbl);
					oph_term_alias_clear(aliases);
					oph_term_env_xmlauto_clear(xml_defs, operators_list, operators_list_size);
					free(hist_path);
					free(xml_path);
					if (exec_statement) {
						free(exec_statement);
						exec_statement = NULL;
					}
					if (print_json)
						print_oph_term_output_json(hashtbl);
					return OPH_TERM_GENERIC_ERROR;
				}
			}
		}
		(print_json) ? my_printf("\\n") : printf("\e[0m\n");
		print_welcome();
	}

	if (_token && strlen(_token) && (_passwd != _token))
		(print_json) ? my_printf("Access token is about to expire or expired.\\nNext time use the token just set in the environment variable.\\n\\n") :
		    printf("\e[1;33mAccess token is about to expire or expired.\nNext time use the token just set in the environment variable.\e[0m\n\n");

	/* INIT HISTORY */
	using_history();
	history_comment_char = '#';
	history_write_timestamps = 1;

	if (!exec_statement) {
		rl_bind_keyseq("\\M-[D", rl_backward_char);
		rl_bind_keyseq("\\M-[C", rl_forward_char);
		rl_bind_keyseq("\\M-[A", rl_get_previous_history);
		rl_bind_keyseq("\\M-[B", rl_get_next_history);

		rl_bind_keyseq("\\M-[Z", rl_menu_complete);
	}

	if ((hist_file = fopen(hist_path, "r"))) {
		read_history(hist_path);
		fclose(hist_file);
	}
	// history ready

	//Auto-completion
	if (!exec_statement) {
		rl_attempted_completion_function = (rl_completion_func_t *) oph_term_completion;
	}

	/* MAIN LOOP */
	while (1) {

		if (line && !exec_alias) {
			free(line);
			line = NULL;
		}
		if (linecopy) {
			free(linecopy);
			linecopy = NULL;
		}
		if (current_operator) {
			free(current_operator);
			current_operator = NULL;
		}

		oph_term_return = OPH_TERM_SUCCESS;	//reset result at each iteration

		/* READ LINE
		 * if first time then INIT READLINE*/
		if (!exec_alias) {
			if (!exec_statement) {
				if (watching && stored_line)
					sleep(watching);
				if (watching && stored_line)
					line = strdup(stored_line);
				else {
					char tmp_prompt[OPH_TERM_MAX_LEN];
					memset(tmp_prompt, 0, OPH_TERM_MAX_LEN);
					char tmp_session_code[OPH_TERM_MAX_LEN];
					memset(tmp_session_code, 0, OPH_TERM_MAX_LEN);
					char tmp_session_code2[OPH_TERM_MAX_LEN];
					memset(tmp_session_code2, 0, OPH_TERM_MAX_LEN);
					if (!signal_raise) {
						if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) {
							if (oph_term_get_session_code((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID), tmp_session_code)) {
								snprintf(tmp_session_code2, OPH_TERM_MAX_LEN, "%s", OPH_TERM_PROMPT);
							} else {
								int l = strlen(tmp_session_code);
								snprintf(tmp_session_code2, OPH_TERM_MAX_LEN, "[%.2s..%s] >>", tmp_session_code, tmp_session_code + l - 4);
							}
						} else {
							snprintf(tmp_session_code2, OPH_TERM_MAX_LEN, "%s", OPH_TERM_PROMPT);
						}
					}
					signal_raise = 0;
					if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1)) {
						if (!strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1), "black")) {
							snprintf(tmp_prompt, OPH_TERM_MAX_LEN, OPH_TERM_BLACK_PROMPT, tmp_session_code2);
						} else if (!strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1), "green")) {
							snprintf(tmp_prompt, OPH_TERM_MAX_LEN, OPH_TERM_GREEN_PROMPT, tmp_session_code2);
						} else if (!strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1), "yellow")) {
							snprintf(tmp_prompt, OPH_TERM_MAX_LEN, OPH_TERM_YELLOW_PROMPT, tmp_session_code2);
						} else if (!strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1), "blue")) {
							snprintf(tmp_prompt, OPH_TERM_MAX_LEN, OPH_TERM_BLUE_PROMPT, tmp_session_code2);
						} else if (!strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1), "purple")) {
							snprintf(tmp_prompt, OPH_TERM_MAX_LEN, OPH_TERM_PURPLE_PROMPT, tmp_session_code2);
						} else if (!strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1), "cyan")) {
							snprintf(tmp_prompt, OPH_TERM_MAX_LEN, OPH_TERM_CYAN_PROMPT, tmp_session_code2);
						} else if (!strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1), "white")) {
							snprintf(tmp_prompt, OPH_TERM_MAX_LEN, OPH_TERM_WHITE_PROMPT, tmp_session_code2);
						} else {
							snprintf(tmp_prompt, OPH_TERM_MAX_LEN, OPH_TERM_RED_PROMPT, tmp_session_code2);
						}
					} else {
						snprintf(tmp_prompt, OPH_TERM_MAX_LEN, OPH_TERM_RED_PROMPT, tmp_session_code2);
					}
					line = readline(tmp_prompt);
				}
			} else {
				line = exec_statement;
				exec_one_statement = 1;
			}
		}

		if (!line) {
			(print_json) ? my_fprintf(stderr, "^D\\n") : fprintf(stderr, "^D\n");
			line = strdup(OPH_TERM_CMD_QUIT);	// logout
		}
		// If the line has any text in it, save it on the history.
		if (line && *line) {
			char *expansion;
			int exp_res;

			exp_res = history_expand(line, &expansion);
			if (exp_res) {
				(print_json) ? my_fprintf(stderr, "\e[1;31m%s\e[0m\n", expansion) : fprintf(stderr, "\e[1;31m%s\e[0m\n", expansion);
			}

			if (exp_res < 0 || exp_res == 2) {
				free(expansion);
				exec_alias = 0;
				alias_substitutions = 0;
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_COMMAND_NOT_RECOGNIZED;
					break;
				}
				continue;
			}

			free(line);
			line = strdup(expansion);
			if (!line) {
				free(expansion);
				(print_json) ? my_fprintf(stderr, "\e[1;31mError allocating memory during history expansion\e[0m\n") : fprintf(stderr,
																	       "\e[1;31mError allocating memory during history expansion\e[0m\n");
				exec_alias = 0;
				alias_substitutions = 0;
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_MEMORY_ERROR;
					break;
				}
				continue;
			}

			free(expansion);

			if (!exec_alias) {
				add_history(line);
				write_history(hist_path);
			}
		}

		/* COMMAND PARSING */

		if (!(linecopy = (char *) strdup(line))) {
			(print_json) ? my_fprintf(stderr, "Error allocating memory [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr, "\e[1;31mError allocating memory [CODE %d]\e[0m\n",
																   OPH_TERM_MEMORY_ERROR);
			exec_alias = 0;
			alias_substitutions = 0;
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement) {
				oph_term_return = OPH_TERM_MEMORY_ERROR;
				break;
			}
			continue;
		}
		// Set command line at each iteration
		if (!exec_one_statement && !exec_alias) {
			memset(command_line, 0, OPH_TERM_MAX_LEN);
			snprintf(command_line, OPH_TERM_MAX_LEN, "%s", line);
		}

		cursor = strtok_r(linecopy, " \t\n", &saveptr);	// extract cmd name
		if (!cursor) {
			exec_alias = 0;
			alias_substitutions = 0;
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement) {
				oph_term_return = OPH_TERM_COMMAND_NOT_RECOGNIZED;
				break;
			}
			continue;
		}

		exec_alias2 = exec_alias;
		exec_alias = 0;

		if (!strcmp(cursor, OPH_TERM_CMD_WATCH)) {	// WATCH
			alias_substitutions = 0;
			cursor = strtok_r(NULL, " \t\n\"", &saveptr);
			if (!cursor) {
				(print_json) ? my_fprintf(stderr, "Command not present [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mCommand not present [CODE %d]\e[0m\n",
																	      OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			watching = 2;
			if (oph_get_winterval(&cursor, &saveptr, &watching)) {
				(print_json) ? my_fprintf(stderr, "Wrong use of option '-n' [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		   "\e[1;31mWrong use of option '-n' [CODE %d]\e[0m\n",
																		   OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (stored_line)
				free(stored_line);
			stored_line = strdup(line);
		}

		if (watching && !exec_alias2) {
			if (system("clear"))
				fprintf(stderr, "Error in executing command 'clear'\n");
			(print_json) ? my_fprintf(stdout, "Watching every %d seconds\\n\\n", watching) : fprintf(stdout, "Watching every %d seconds\n\n", watching);
		}
		// switch on cmd name
		if (!strcmp(cursor, OPH_TERM_CMD_QUIT) || !strcmp(cursor, OPH_TERM_CMD_EXIT)) {	// QUIT or EXIT
			alias_substitutions = 0;
			(print_json) ? my_fprintf(stdout, "Oph_Term closed. Bye!\\n") : fprintf(stdout, "Oph_Term closed. Bye!\n");
			if (print_json)
				print_oph_term_output_json(hashtbl);
			break;
		} else if (!strcmp(cursor, OPH_TERM_CMD_VERSION)) {	// VERSION
			alias_substitutions = 0;
			print_version();
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_WARRANTY)) {	// WARRANTY
			alias_substitutions = 0;
			print_version();
			(print_json) ? my_printf("%s", OPH_TERM_WARRANTY2) : printf("%s", OPH_TERM_WARRANTY);
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_CONDITIONS)) {	// CONDITIONS
			alias_substitutions = 0;
			print_version();
			(print_json) ? my_printf("%s", OPH_TERM_CONDITIONS2) : printf("%s", OPH_TERM_CONDITIONS);
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_HISTORY)) {	// HISTORY
			alias_substitutions = 0;
			cursor = strtok_r(NULL, " \t\n", &saveptr);
			if (!cursor) {	// "history"
				time_t tt;
				char timestr[128];
				hist_list = history_list();
				if (hist_list) {
					for (i = 0; hist_list[i]; i++) {
						tt = history_get_time(hist_list[i]);
						if (tt)
							strftime(timestr, sizeof(timestr), "%c", localtime(&tt));
						else
							strcpy(timestr, "??");
						(print_json) ? my_fprintf(stdout, "%d: %s: %s\n", i + history_base, timestr, hist_list[i]->line) : fprintf(stdout, "%d: %s: %s\n", i + history_base,
																			   timestr, hist_list[i]->line);
					}
				}
			} else if (!strcmp(cursor, "-c")) {	// "history -c"
				history_truncate_file(hist_path, 0);
				clear_history();
				(print_json) ? my_fprintf(stdout, "Oph_Term history successfully cleared\\n") : fprintf(stdout, "Oph_Term history successfully cleared\n");
			} else {
				(print_json) ? my_fprintf(stderr, "Option %s not recognized [CODE %d]\\n", cursor, OPH_TERM_OPTION_NOT_RECOGNIZED) : fprintf(stderr,
																			     "\e[1;31mOption %s not recognized [CODE %d]\e[0m\n",
																			     cursor, OPH_TERM_OPTION_NOT_RECOGNIZED);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_OPTION_NOT_RECOGNIZED;
					break;
				}
				continue;
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strncmp(cursor, OPH_TERM_REMOTE_CMD_PREFIX, OPH_TERM_REMOTE_CMD_PREFIX_LEN)) {	// REMOTE COMMAND OPH_*
			alias_substitutions = 0;

			// DO VARIABLE SUBSTITUTION
			char *new_line = NULL;
			if (oph_term_full_var_expansion(line, hashtbl, &new_line)) {
				(print_json) ? my_fprintf(stderr, "Error performing variable substitution\\n") : fprintf(stderr, "\e[1;31mError performing variable substitution\e[0m\n");
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_GENERIC_ERROR;
					break;
				}
				continue;
			}
			free(linecopy);
			linecopy = new_line;
			new_line = NULL;
			cursor = strtok_r(linecopy, " \t\n", &saveptr);

			if (watching && !exec_alias2 && cursor) {
				cursor = strtok_r(NULL, " \t\n", &saveptr);
				if (oph_get_winterval(&cursor, &saveptr, NULL)) {
					(print_json) ? my_fprintf(stderr, "Wrong use of option '-n' [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			   "\e[1;31mWrong use of option '-n' [CODE %d]\e[0m\n",
																			   OPH_TERM_INVALID_PARAM_VALUE);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
						break;
					}
					continue;
				}
			}
#ifndef INTERFACE_TYPE_IS_GSI
			if (!_user) {
				(print_json) ? my_fprintf(stderr, "OPH_USER not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mOPH_USER not set [CODE %d]\e[0m\n",
																	   OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!_passwd) {
				(print_json) ? my_fprintf(stderr, "OPH_PASSWD not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mOPH_PASSWD not set [CODE %d]\e[0m\n",
																	     OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
#endif
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST)) {
				(print_json) ? my_fprintf(stderr, "OPH_SERVER_HOST not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		  "\e[1;31mOPH_SERVER_HOST not set [CODE %d]\e[0m\n",
																		  OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT)) {
				(print_json) ? my_fprintf(stderr, "OPH_SERVER_PORT not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		  "\e[1;31mOPH_SERVER_PORT not set [CODE %d]\e[0m\n",
																		  OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER) ||
			    (strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "dump") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "basic") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "coloured") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "extended") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "extended_coloured"))) {
				(print_json) ? my_fprintf(stderr, "OPH_TERM_VIEWER not set or incorrect [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			       "\e[1;31mOPH_TERM_VIEWER not set or incorrect [CODE %d]\e[0m\n",
																			       OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_FORMAT) ||
			    (strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_FORMAT), "classic") && strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_FORMAT), "compact"))) {
				(print_json) ? my_fprintf(stderr, "OPH_TERM_FORMAT not set or incorrect [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			       "\e[1;31mOPH_TERM_FORMAT not set or incorrect [CODE %d]\e[0m\n",
																			       OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS) ||
			    (strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "no_op") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "save") && strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "open"))) {
				(print_json) ? my_fprintf(stderr, "OPH_TERM_IMGS not set or incorrect [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			     "\e[1;31mOPH_TERM_IMGS not set or incorrect [CODE %d]\e[0m\n",
																			     OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}

			short int massive_flag = 0;

			current_operator = strdup(cursor);
			if (!current_operator) {
				(print_json) ? my_fprintf(stderr, "Unable to duplicate operator name [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																		     "\e[1;31mUnable to duplicate operator name [CODE %d]\e[0m\n",
																		     OPH_TERM_MEMORY_ERROR);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_MEMORY_ERROR;
					break;
				}
				continue;
			}
			if (strchr(current_operator, ';')) {
				(print_json) ? my_fprintf(stderr, "Invalid operator name %s [CODE %d]\\n", current_operator, OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																				     "\e[1;31mInvalid operator name %s [CODE %d]\e[0m\n",
																				     current_operator,
																				     OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			cursor = strtok_r(NULL, "\t\n\"", &saveptr);
			if (cursor) {
				//trim heading spaces
				size_t p;
				for (p = 0; p < strlen(cursor); p++) {
					if (cursor[p] != ' ')
						break;
				}
				if (p == strlen(cursor))
					cursor = NULL;
				else
					cursor += p;
			}
			if (!cursor) {
				// submit with no other parameters
				char *newsession = NULL;
				char *newdatacube = NULL;
				char *newcwd = NULL;
				char *newcdd = NULL;
				int n = 0;

				//prefix operator name
				memset(submission_string, 0, OPH_TERM_MAX_LEN);
				n += snprintf(submission_string + n, OPH_TERM_MAX_LEN - n, "operator=%s;", current_operator);

				//sessionid management
				if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID))
					n += snprintf(submission_string + n, OPH_TERM_MAX_LEN - n, "sessionid=%s;", (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID));
				//exec_mode management
				if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_EXEC_MODE))
					n += snprintf(submission_string + n, OPH_TERM_MAX_LEN - n, "exec_mode=%s;", (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_EXEC_MODE));
				//ncores management
				if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_NCORES))
					n += snprintf(submission_string + n, OPH_TERM_MAX_LEN - n, "ncores=%s;", (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_NCORES));
				//cube management
				if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_DATACUBE))
					n += snprintf(submission_string + n, OPH_TERM_MAX_LEN - n, "cube=%s;", (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_DATACUBE));
				//cwd management
				if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CWD)) {
					if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_CWD, "/")) {
						(print_json) ? my_fprintf(stderr, "Could not set cwd [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																		     "\e[1;31mCould not set cwd [CODE %d]\e[0m\n",
																		     OPH_TERM_MEMORY_ERROR);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_MEMORY_ERROR;
							break;
						}
						continue;
					}
				}
				n += snprintf(submission_string + n, OPH_TERM_MAX_LEN - n, "cwd=%s;", (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CWD));
				//cdd management
				if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CDD)) {
					char oph_cdd[OPH_TERM_MAX_LEN], *_oph_cdd = oph_cdd;
					if (oph_base_src_path) {
						_oph_cdd = getcwd(oph_cdd, OPH_TERM_MAX_LEN);
						char chddir[OPH_TERM_MAX_LEN], *_chddir = chddir;
						snprintf(chddir, OPH_TERM_MAX_LEN, "%s", oph_base_src_path);
						while (*_oph_cdd && *_chddir && (*_oph_cdd == *_chddir)) {
							_oph_cdd++;
							_chddir++;
						}
					} else
						strcpy(oph_cdd, "/");
					if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_CDD, _oph_cdd)) {
						(print_json) ? my_fprintf(stderr, "Could not set cdd [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																		     "\e[1;31mCould not set cdd [CODE %d]\e[0m\n",
																		     OPH_TERM_MEMORY_ERROR);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_MEMORY_ERROR;
							break;
						}
						continue;
					}
				}
				n += snprintf(submission_string + n, OPH_TERM_MAX_LEN - n, "cdd=%s;", (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CDD));

				// SUBMISSION
				if (strstr(submission_string, ";cube=[") || strstr(submission_string, " cube=["))
					massive_flag = 1;
				(print_json) ? snprintf(oph_term_request, OUTPUT_MAX_LEN, "%s", submission_string) : printf("\e[1;34m[Request]:\e[0m\n%s\n\n", submission_string);
				char *response_for_viewer = NULL;
				oph_term_client(command_line, submission_string, &newsession, _user, _passwd, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST),
						(char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), &oph_term_return, NULL, &response_for_viewer, 1, hashtbl);
				if (!print_debug_data && response_for_viewer
				    && ((int) strlen(response_for_viewer) > 1024 * strtol((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_RESPONSE_BUFFER), NULL, 10))) {
					if (response_for_viewer)
						free(response_for_viewer);
					response_for_viewer = NULL;
					(print_json) ? my_fprintf(stderr, "Response exceeds the maximum size %s kB.\\n",
								  (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_RESPONSE_BUFFER)) : fprintf(stderr,
																	     "\e[1;31mResponse exceeds the maximum size %s kB.\e[0m\n",
																	     (char *) hashtbl_get(hashtbl,
																				  OPH_TERM_ENV_OPH_RESPONSE_BUFFER));
				}
				// VISUALIZATION
				if (response_for_viewer) {
					int open_img = !strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "open");
					int save_img = open_img || !strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "save");
					char *newtoken = NULL, *exectime = NULL;
					int viewer_res = oph_term_viewer((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), &response_for_viewer,
									 (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1)) ? ((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1)) : "red",
									 save_img,
									 open_img, 1, &newdatacube, &newcwd, &newcdd, &newtoken, &exectime, (char *) hashtbl_get(hashtbl,
																				 OPH_TERM_ENV_OPH_GRAPH_LAYOUT));
					if (viewer_res != 0 && viewer_res != OPH_TERM_ERROR_WITHIN_JSON) {
						(print_json) ? my_fprintf(stderr, "Could not render result [CODE %d]\\n", OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																			    "\e[1;31mCould not render result [CODE %d]\e[0m\n",
																			    OPH_TERM_GENERIC_ERROR);
						if (newsession) {
							free(newsession);
							newsession = NULL;
						}
						if (newdatacube) {
							free(newdatacube);
							newdatacube = NULL;
						}
						if (newcwd) {
							free(newcwd);
							newcwd = NULL;
						}
						if (newcdd) {
							free(newcdd);
							newcdd = NULL;
						}
						if (newtoken) {
							free(newtoken);
							newtoken = NULL;
						}
						if (exectime) {
							free(exectime);
							exectime = NULL;
						}
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = viewer_res;
							break;
						}
						continue;
					}
					if (newtoken) {
						pthread_mutex_lock(&global_flag);
						hashtbl_remove(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
						hashtbl_insert(hashtbl, OPH_TERM_ENV_OPH_TOKEN, newtoken, strlen(newtoken) + 1);
						_passwd = _token = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
						pthread_mutex_unlock(&global_flag);
						free(newtoken);
					}
					if (exectime) {
						(print_json) ? my_printf("Execution time: %s\\n", oph_print_exectime(&exectime)) : printf("Execution time: %s\n", oph_print_exectime(&exectime));
						free(exectime);
					}
				}
				//update OPH_SESSION_ID if necessary
				if (newsession) {
					if (strlen(newsession) == 0) {
						if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID))
							oph_term_unsetenv(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID);
					} else if ((hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID) && strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID), newsession))
						   || !hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) {
						if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID, newsession)) {
							(print_json) ? my_fprintf(stderr, "Could not set OPH_SESSION_ID [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																					"\e[1;31mCould not set OPH_SESSION_ID [CODE %d]\e[0m\n",
																					OPH_TERM_MEMORY_ERROR);
							free(newsession);
							newsession = NULL;
							if (newdatacube) {
								free(newdatacube);
								newdatacube = NULL;
							}
							if (newcwd) {
								free(newcwd);
								newcwd = NULL;
							}
							if (newcdd) {
								free(newcdd);
								newcdd = NULL;
							}
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_MEMORY_ERROR;
								break;
							}
							continue;
						}
						if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_CWD, "/")) {
							(print_json) ? my_fprintf(stderr, "Could not set OPH_CWD [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																				 "\e[1;31mCould not set OPH_CWD [CODE %d]\e[0m\n",
																				 OPH_TERM_MEMORY_ERROR);
							free(newsession);
							newsession = NULL;
							if (newdatacube) {
								free(newdatacube);
								newdatacube = NULL;
							}
							if (newcwd) {
								free(newcwd);
								newcwd = NULL;
							}
							if (newcdd) {
								free(newcdd);
								newcdd = NULL;
							}
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_MEMORY_ERROR;
								break;
							}
							continue;
						}
					}
					free(newsession);
					newsession = NULL;
				}
				//update OPH_DATACUBE if necessary
				if (newdatacube) {
					if (massive_flag) {
						if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_DATACUBE))
							oph_term_unsetenv(hashtbl, OPH_TERM_ENV_OPH_DATACUBE);
					} else {
						if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_DATACUBE, newdatacube)) {
							(print_json) ? my_fprintf(stderr, "Could not set OPH_DATACUBE [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																				      "\e[1;31mCould not set OPH_DATACUBE [CODE %d]\e[0m\n",
																				      OPH_TERM_MEMORY_ERROR);
							free(newdatacube);
							newdatacube = NULL;
							if (newcwd) {
								free(newcwd);
								newcwd = NULL;
							}
							if (newcdd) {
								free(newcdd);
								newcdd = NULL;
							}
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_MEMORY_ERROR;
								break;
							}
							continue;
						}
					}
					free(newdatacube);
					newdatacube = NULL;
				}
				//update OPH_CWD if necessary
				if (newcwd) {
					if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_CWD, newcwd)) {
						(print_json) ? my_fprintf(stderr, "Could not set OPH_CWD [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			 "\e[1;31mCould not set OPH_CWD [CODE %d]\e[0m\n",
																			 OPH_TERM_MEMORY_ERROR);
						free(newcwd);
						newcwd = NULL;
						if (newcdd) {
							free(newcdd);
							newcdd = NULL;
						}
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_MEMORY_ERROR;
							break;
						}
						continue;
					}
					free(newcwd);
					newcwd = NULL;
				}
				//update OPH_CDD if necessary
				if (newcdd) {
					if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_CDD, newcdd)) {
						(print_json) ? my_fprintf(stderr, "Could not set OPH_CDD [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			 "\e[1;31mCould not set OPH_CDD [CODE %d]\e[0m\n",
																			 OPH_TERM_MEMORY_ERROR);
						free(newcdd);
						newcdd = NULL;
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_MEMORY_ERROR;
							break;
						}
						continue;
					}
					if (oph_base_src_path) {
						char chddir[OPH_TERM_MAX_LEN];
						snprintf(chddir, OPH_TERM_MAX_LEN, "%s/%s", oph_base_src_path, newcdd);
						i = chdir(chddir);
					}
					free(newcdd);
					newcdd = NULL;
				}
			} else {	// submit with specific parameters
				char *newsession = NULL;
				char *newdatacube = NULL;
				char *newcwd = NULL;
				char *newcdd = NULL;
				char cursorcopy[OPH_TERM_MAX_LEN] = "\0";
				int flag = 0;
				for (i = strlen(cursor) - 1; i >= 0; i--) {
					if (cursor[i] != ' ') {
						cursor[i + 1] = '\0';
						break;
					}
				}
				if (cursor[strlen(cursor) - 1] != ';') {
					memset(fixed_cursor, 0, OPH_TERM_MAX_LEN);
					snprintf(fixed_cursor, OPH_TERM_MAX_LEN, "%s;", cursor);
					cursor = fixed_cursor;
				}
				snprintf(cursorcopy, OPH_TERM_MAX_LEN, "%s", cursor);

				//sessionid management
				if (!strstr(cursor, ";sessionid=") && strncmp(cursor, "sessionid=", 10)) {
					if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) {
						snprintf(cursorcopy, OPH_TERM_MAX_LEN, "%ssessionid=%s;", cursor, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID));
						flag = 1;
					}
				}
				//exec_mode management
				if (!strstr(cursor, ";exec_mode=") && strncmp(cursor, "exec_mode=", 10)) {
					if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_EXEC_MODE)) {
						if (!flag) {
							snprintf(cursorcopy, OPH_TERM_MAX_LEN, "%sexec_mode=%s;", cursor, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_EXEC_MODE));
							flag = 1;
						} else {
							char *cursorcopy2 = strdup(cursorcopy);
							if (!cursorcopy2) {
								(print_json) ? my_fprintf(stderr, "Could not use OPH_EXEC_MODE [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																					       "\e[1;31mCould not use OPH_EXEC_MODE [CODE %d]\e[0m\n",
																					       OPH_TERM_MEMORY_ERROR);
								if (print_json)
									print_oph_term_output_json(hashtbl);
								if (exec_one_statement) {
									oph_term_return = OPH_TERM_MEMORY_ERROR;
									break;
								}
								continue;
							}
							snprintf(cursorcopy, OPH_TERM_MAX_LEN, "%sexec_mode=%s;", cursorcopy2, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_EXEC_MODE));
							free(cursorcopy2);
						}
					}
				}
				//ncores management
				if (!strstr(cursor, ";ncores=") && strncmp(cursor, "ncores=", 7)) {
					if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_NCORES)) {
						if (!flag) {
							snprintf(cursorcopy, OPH_TERM_MAX_LEN, "%sncores=%s;", cursor, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_NCORES));
							flag = 1;
						} else {
							char *cursorcopy2 = strdup(cursorcopy);
							if (!cursorcopy2) {
								(print_json) ? my_fprintf(stderr, "Could not use OPH_NCORES [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																					    "\e[1;31mCould not use OPH_NCORES [CODE %d]\e[0m\n",
																					    OPH_TERM_MEMORY_ERROR);
								if (print_json)
									print_oph_term_output_json(hashtbl);
								if (exec_one_statement) {
									oph_term_return = OPH_TERM_MEMORY_ERROR;
									break;
								}
								continue;
							}
							snprintf(cursorcopy, OPH_TERM_MAX_LEN, "%sncores=%s;", cursorcopy2, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_NCORES));
							free(cursorcopy2);
						}
					}
				}
				//cube management
				if (!strstr(cursor, ";cube=") && strncmp(cursor, "cube=", 5)) {
					if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_DATACUBE)) {
						if (!flag) {
							snprintf(cursorcopy, OPH_TERM_MAX_LEN, "%scube=%s;", cursor, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_DATACUBE));
							flag = 1;
						} else {
							char *cursorcopy2 = strdup(cursorcopy);
							if (!cursorcopy2) {
								(print_json) ? my_fprintf(stderr, "Could not use OPH_DATACUBE [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																					      "\e[1;31mCould not use OPH_DATACUBE [CODE %d]\e[0m\n",
																					      OPH_TERM_MEMORY_ERROR);
								if (print_json)
									print_oph_term_output_json(hashtbl);
								if (exec_one_statement) {
									oph_term_return = OPH_TERM_MEMORY_ERROR;
									break;
								}
								continue;
							}
							snprintf(cursorcopy, OPH_TERM_MAX_LEN, "%scube=%s;", cursorcopy2, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_DATACUBE));
							free(cursorcopy2);
						}
					}
				}
				//cwd management
				if (!strstr(cursor, ";cwd=") && strncmp(cursor, "cwd=", 4)) {
					if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CWD)) {
						if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_CWD, "/")) {
							(print_json) ? my_fprintf(stderr, "Could not set cwd [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			     "\e[1;31mCould not set cwd [CODE %d]\e[0m\n",
																			     OPH_TERM_MEMORY_ERROR);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_MEMORY_ERROR;
								break;
							}
							continue;
						}
					}
					if (!flag) {
						snprintf(cursorcopy, OPH_TERM_MAX_LEN, "%scwd=%s;", cursor, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CWD));
						flag = 1;
					} else {
						char *cursorcopy2 = strdup(cursorcopy);
						if (!cursorcopy2) {
							(print_json) ? my_fprintf(stderr, "Could not use OPH_CWD [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																				 "\e[1;31mCould not use OPH_CWD [CODE %d]\e[0m\n",
																				 OPH_TERM_MEMORY_ERROR);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_MEMORY_ERROR;
								break;
							}
							continue;
						}
						snprintf(cursorcopy, OPH_TERM_MAX_LEN, "%scwd=%s;", cursorcopy2, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CWD));
						free(cursorcopy2);
					}
				}
				//cdd management
				if (!strstr(cursor, ";cdd=") && strncmp(cursor, "cdd=", 4)) {
					if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CDD)) {
						char oph_cdd[OPH_TERM_MAX_LEN], *_oph_cdd = oph_cdd;
						if (oph_base_src_path) {
							_oph_cdd = getcwd(oph_cdd, OPH_TERM_MAX_LEN);
							char chddir[OPH_TERM_MAX_LEN], *_chddir = chddir;
							snprintf(chddir, OPH_TERM_MAX_LEN, "%s", oph_base_src_path);
							while (*_oph_cdd && *_chddir && (*_oph_cdd == *_chddir)) {
								_oph_cdd++;
								_chddir++;
							}
						} else
							strcpy(oph_cdd, "/");
						if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_CDD, _oph_cdd)) {
							(print_json) ? my_fprintf(stderr, "Could not set cdd [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			     "\e[1;31mCould not set cdd [CODE %d]\e[0m\n",
																			     OPH_TERM_MEMORY_ERROR);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_MEMORY_ERROR;
								break;
							}
							continue;
						}
					}
					if (!flag) {
						snprintf(cursorcopy, OPH_TERM_MAX_LEN, "%scdd=%s;", cursor, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CDD));
						flag = 1;
					} else {
						char *cursorcopy2 = strdup(cursorcopy);
						if (!cursorcopy2) {
							(print_json) ? my_fprintf(stderr, "Could not use OPH_CDD [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																				 "\e[1;31mCould not use OPH_CDD [CODE %d]\e[0m\n",
																				 OPH_TERM_MEMORY_ERROR);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_MEMORY_ERROR;
								break;
							}
							continue;
						}
						snprintf(cursorcopy, OPH_TERM_MAX_LEN, "%scdd=%s;", cursorcopy2, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CDD));
						free(cursorcopy2);
					}
				}
				//prefix operator name
				memset(submission_string, 0, OPH_TERM_MAX_LEN);
				snprintf(submission_string, OPH_TERM_MAX_LEN, "operator=%s;%s", current_operator, cursorcopy);

				// SUBMISSION
				if (strstr(submission_string, ";cube=[") || strstr(submission_string, " cube=["))
					massive_flag = 1;
				(print_json) ? snprintf(oph_term_request, OUTPUT_MAX_LEN, "%s", submission_string) : printf("\e[1;34m[Request]:\e[0m\n%s\n\n", submission_string);
				char *response_for_viewer = NULL;
				oph_term_client(command_line, submission_string, &newsession, _user, _passwd, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST),
						(char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), &oph_term_return, NULL, &response_for_viewer, 1, hashtbl);
				if (!print_debug_data && response_for_viewer
				    && ((int) strlen(response_for_viewer) > 1024 * strtol((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_RESPONSE_BUFFER), NULL, 10))) {
					if (response_for_viewer)
						free(response_for_viewer);
					response_for_viewer = NULL;
					(print_json) ? my_fprintf(stderr, "Response exceeds the maximum size %s kB.\\n",
								  (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_RESPONSE_BUFFER)) : fprintf(stderr,
																	     "\e[1;31mResponse exceeds the maximum size %s kB.\e[0m\n",
																	     (char *) hashtbl_get(hashtbl,
																				  OPH_TERM_ENV_OPH_RESPONSE_BUFFER));
				}
				// VISUALIZATION
				if (response_for_viewer) {
					int open_img = !strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "open");
					int save_img = open_img || !strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "save");
					char *newtoken = NULL, *exectime = NULL;
					int viewer_res = oph_term_viewer((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), &response_for_viewer,
									 (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1)) ? ((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1)) : "red",
									 save_img,
									 open_img, 1, &newdatacube, &newcwd, &newcdd, &newtoken, &exectime, (char *) hashtbl_get(hashtbl,
																				 OPH_TERM_ENV_OPH_GRAPH_LAYOUT));
					if (viewer_res != 0 && viewer_res != OPH_TERM_ERROR_WITHIN_JSON) {
						(print_json) ? my_fprintf(stderr, "Could not render result [CODE %d]\\n", OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																			    "\e[1;31mCould not render result [CODE %d]\e[0m\n",
																			    OPH_TERM_GENERIC_ERROR);
						if (newsession) {
							free(newsession);
							newsession = NULL;
						}
						if (newdatacube) {
							free(newdatacube);
							newdatacube = NULL;
						}
						if (newcwd) {
							free(newcwd);
							newcwd = NULL;
						}
						if (newcdd) {
							free(newcdd);
							newcdd = NULL;
						}
						if (newtoken) {
							free(newtoken);
							newtoken = NULL;
						}
						if (exectime) {
							free(exectime);
							exectime = NULL;
						}
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = viewer_res;
							break;
						}
						continue;
					}
					if (newtoken) {
						pthread_mutex_lock(&global_flag);
						hashtbl_remove(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
						hashtbl_insert(hashtbl, OPH_TERM_ENV_OPH_TOKEN, newtoken, strlen(newtoken) + 1);
						_passwd = _token = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
						pthread_mutex_unlock(&global_flag);
						free(newtoken);
					}
					if (exectime) {
						(print_json) ? my_printf("Execution time: %s\\n", oph_print_exectime(&exectime)) : printf("Execution time: %s\n", oph_print_exectime(&exectime));
						free(exectime);
					}
				}
				//update OPH_SESSION_ID if necessary
				if (newsession) {
					if (strlen(newsession) == 0) {
						if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID))
							oph_term_unsetenv(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID);
					} else if ((hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID) && strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID), newsession))
						   || !hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) {
						if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID, newsession)) {
							(print_json) ? my_fprintf(stderr, "Could not set OPH_SESSION_ID [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																					"\e[1;31mCould not set OPH_SESSION_ID [CODE %d]\e[0m\n",
																					OPH_TERM_MEMORY_ERROR);
							free(newsession);
							newsession = NULL;
							if (newdatacube) {
								free(newdatacube);
								newdatacube = NULL;
							}
							if (newcwd) {
								free(newcwd);
								newcwd = NULL;
							}
							if (newcdd) {
								free(newcdd);
								newcdd = NULL;
							}
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_MEMORY_ERROR;
								break;
							}
							continue;
						}
						if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_CWD, "/")) {
							(print_json) ? my_fprintf(stderr, "Could not set OPH_CWD [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																				 "\e[1;31mCould not set OPH_CWD [CODE %d]\e[0m\n",
																				 OPH_TERM_MEMORY_ERROR);
							free(newsession);
							newsession = NULL;
							if (newdatacube) {
								free(newdatacube);
								newdatacube = NULL;
							}
							if (newcwd) {
								free(newcwd);
								newcwd = NULL;
							}
							if (newcdd) {
								free(newcdd);
								newcdd = NULL;
							}
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_MEMORY_ERROR;
								break;
							}
							continue;
						}
					}
					free(newsession);
					newsession = NULL;
				}
				//update OPH_DATACUBE if necessary
				if (newdatacube) {
					if (massive_flag) {
						if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_DATACUBE))
							oph_term_unsetenv(hashtbl, OPH_TERM_ENV_OPH_DATACUBE);
					} else {
						if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_DATACUBE, newdatacube)) {
							(print_json) ? my_fprintf(stderr, "Could not set OPH_DATACUBE [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																				      "\e[1;31mCould not set OPH_DATACUBE [CODE %d]\e[0m\n",
																				      OPH_TERM_MEMORY_ERROR);
							free(newdatacube);
							newdatacube = NULL;
							if (newcwd) {
								free(newcwd);
								newcwd = NULL;
							}
							if (newcdd) {
								free(newcdd);
								newcdd = NULL;
							}
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_MEMORY_ERROR;
								break;
							}
							continue;
						}
					}
					free(newdatacube);
					newdatacube = NULL;
				}
				//update OPH_CWD if necessary
				if (newcwd) {
					if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_CWD, newcwd)) {
						(print_json) ? my_fprintf(stderr, "Could not set OPH_CWD [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			 "\e[1;31mCould not set OPH_CWD [CODE %d]\e[0m\n",
																			 OPH_TERM_MEMORY_ERROR);
						free(newcwd);
						newcwd = NULL;
						if (newcdd) {
							free(newcdd);
							newcdd = NULL;
						}
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_MEMORY_ERROR;
							break;
						}
						continue;
					}
					free(newcwd);
					newcwd = NULL;
				}
				//update OPH_CDD if necessary
				if (newcdd) {
					if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_CDD, newcdd)) {
						(print_json) ? my_fprintf(stderr, "Could not set OPH_CDD [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			 "\e[1;31mCould not set OPH_CDD [CODE %d]\e[0m\n",
																			 OPH_TERM_MEMORY_ERROR);
						free(newcdd);
						newcdd = NULL;
						if (newcdd) {
							free(newcdd);
							newcdd = NULL;
						}
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_MEMORY_ERROR;
							break;
						}
						continue;
					}
					if (oph_base_src_path) {
						char chddir[OPH_TERM_MAX_LEN];
						snprintf(chddir, OPH_TERM_MAX_LEN, "%s/%s", oph_base_src_path, newcdd);
						i = chdir(chddir);
					}
					free(newcdd);
					newcdd = NULL;
				}
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_VIEW)) {	// VIEW
			alias_substitutions = 0;

			int iterations_num = 1;
			int time_interval = 5;	//seconds

			// DO VARIABLE SUBSTITUTION
			char *new_line = NULL;
			if (oph_term_full_var_expansion(line, hashtbl, &new_line)) {
				(print_json) ? my_fprintf(stderr, "Error performing variable substitution\\n") : fprintf(stderr, "\e[1;31mError performing variable substitution\e[0m\n");
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_GENERIC_ERROR;
					break;
				}
				continue;
			}
			free(linecopy);
			linecopy = new_line;
			new_line = NULL;
			cursor = strtok_r(linecopy, " \t\n", &saveptr);

			if (watching && !exec_alias2 && cursor) {
				cursor = strtok_r(NULL, " \t\n", &saveptr);
				if (oph_get_winterval(&cursor, &saveptr, NULL)) {
					(print_json) ? my_fprintf(stderr, "Wrong use of option '-n' [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			   "\e[1;31mWrong use of option '-n' [CODE %d]\e[0m\n",
																			   OPH_TERM_INVALID_PARAM_VALUE);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
						break;
					}
					continue;
				}
			}
#ifndef INTERFACE_TYPE_IS_GSI
			if (!_user) {
				(print_json) ? my_fprintf(stderr, "OPH_USER not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mOPH_USER not set [CODE %d]\e[0m\n",
																	   OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!_passwd) {
				(print_json) ? my_fprintf(stderr, "OPH_PASSWD not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mOPH_PASSWD not set [CODE %d]\e[0m\n",
																	     OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
#endif
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST)) {
				(print_json) ? my_fprintf(stderr, "OPH_SERVER_HOST not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		  "\e[1;31mOPH_SERVER_HOST not set [CODE %d]\e[0m\n",
																		  OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT)) {
				(print_json) ? my_fprintf(stderr, "OPH_SERVER_PORT not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		  "\e[1;31mOPH_SERVER_PORT not set [CODE %d]\e[0m\n",
																		  OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER) ||
			    (strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "dump") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "basic") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "coloured") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "extended") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "extended_coloured"))) {
				(print_json) ? my_fprintf(stderr, "OPH_TERM_VIEWER not set or incorrect [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			       "\e[1;31mOPH_TERM_VIEWER not set or incorrect [CODE %d]\e[0m\n",
																			       OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS) ||
			    (strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "no_op") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "save") && strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "open"))) {
				(print_json) ? my_fprintf(stderr, "OPH_TERM_IMGS not set or incorrect [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			     "\e[1;31mOPH_TERM_IMGS not set or incorrect [CODE %d]\e[0m\n",
																			     OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}

			int n = 0;
			//set submission string
			memset(submission_string, 0, OPH_TERM_MAX_LEN);
			n += snprintf(submission_string + n, OPH_TERM_MAX_LEN - n, OPH_TERM_OPH_RESUME_STRING_NO_SAVE);

			//sessionid management
			if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID))
				n += snprintf(submission_string + n, OPH_TERM_MAX_LEN - n, "sessionid=%s;", (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID));

			cursor = strtok_r(NULL, " \t\n", &saveptr);
			if (!cursor) {
				(print_json) ? my_fprintf(stderr, "Job not specified [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mJob not specified [CODE %d]\e[0m\n",
																	    OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			} else {
				if (!strcmp(cursor, "-j")) {	//view -j JobID
					cursor = strtok_r(NULL, " \t\n", &saveptr);
					if (!cursor) {
						(print_json) ? my_fprintf(stderr, "JobID not specified [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			      "\e[1;31mJobID not specified [CODE %d]\e[0m\n",
																			      OPH_TERM_INVALID_PARAM_VALUE);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
							break;
						}
						continue;
					}
					char tmp_job_id[OPH_TERM_MAX_LEN];
					char tmp_submission_string[OPH_TERM_MAX_LEN];
					char tmp_session[OPH_TERM_MAX_LEN];
					char tmp_workflow[OPH_TERM_MAX_LEN];
					char tmp_marker[OPH_TERM_MAX_LEN];

					memset(tmp_job_id, 0, OPH_TERM_MAX_LEN);
					memset(tmp_submission_string, 0, OPH_TERM_MAX_LEN);
					memset(tmp_session, 0, OPH_TERM_MAX_LEN);
					memset(tmp_marker, 0, OPH_TERM_MAX_LEN);
					memset(tmp_workflow, 0, OPH_TERM_MAX_LEN);

					char *saveptr2 = NULL, *tmp_cursor;
					snprintf(tmp_job_id, OPH_TERM_MAX_LEN, "%s", cursor);
					tmp_cursor = strtok_r(cursor, OPH_TERM_WORKFLOW_DELIMITER, &saveptr2);
					if (!tmp_cursor) {
						(print_json) ? my_fprintf(stderr, "JobID not valid [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			  "\e[1;31mJobID not valid [CODE %d]\e[0m\n",
																			  OPH_TERM_INVALID_PARAM_VALUE);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
							break;
						}
						continue;
					}
					snprintf(tmp_session, OPH_TERM_MAX_LEN, "%s", tmp_cursor);
					tmp_cursor = strtok_r(NULL, OPH_TERM_MARKER_DELIMITER, &saveptr2);
					if (!tmp_cursor) {
						(print_json) ? my_fprintf(stderr, "JobID not valid [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			  "\e[1;31mJobID not valid [CODE %d]\e[0m\n",
																			  OPH_TERM_INVALID_PARAM_VALUE);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
							break;
						}
						continue;
					}
					snprintf(tmp_workflow, OPH_TERM_MAX_LEN, "%s", tmp_cursor);
					tmp_cursor = strtok_r(NULL, OPH_TERM_MARKER_DELIMITER, &saveptr2);
					if (!tmp_cursor) {
						(print_json) ? my_fprintf(stderr, "JobID not valid [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			  "\e[1;31mJobID not valid [CODE %d]\e[0m\n",
																			  OPH_TERM_INVALID_PARAM_VALUE);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
							break;
						}
						continue;
					}
					snprintf(tmp_marker, OPH_TERM_MAX_LEN, "%s", tmp_cursor);

					if (oph_term_check_wid_mkid
					    (tmp_session, tmp_workflow, tmp_marker, _user, _passwd, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST),
					     (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), &oph_term_return, hashtbl)) {
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement)
							break;
						continue;
					}

					char *tmp_command = NULL;
					if (oph_term_get_request_with_marker
					    (tmp_session, tmp_marker, _user, _passwd, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST),
					     (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), &oph_term_return, &tmp_command, NULL, hashtbl)) {
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement)
							break;
						continue;
					}
					(print_json) ? my_printf("[%s#%s] %s [%s]\\n\\n", tmp_workflow, tmp_marker, (tmp_command) ? tmp_command : "",
								 tmp_job_id) : printf("\e[1;34m[%s#%s]\e[0m %s \e[1;34m[%s]\e[0m\n\n", tmp_workflow, tmp_marker, (tmp_command) ? tmp_command : "",
										      tmp_job_id);
					if (tmp_command) {
						free(tmp_command);
						tmp_command = NULL;
					}

					snprintf(tmp_submission_string, OPH_TERM_MAX_LEN, "%ssession=%s;id=%s;id_type=marker;", submission_string, tmp_session, tmp_marker);

					// Retrieve iterations and interval
					cursor = strtok_r(NULL, " \t\n", &saveptr);
					if (cursor) {
						if (watching) {
							watching = 0;
							(print_json) ? my_fprintf(stderr, "This command cannot be repeated during watching [CODE %d]\\n",
										  OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
															  "\e[1;31mThis command cannot be repeated during watching [CODE %d]\e[0m\n",
															  OPH_TERM_INVALID_PARAM_VALUE);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
								break;
							}
							continue;
						}
						iterations_num = (int) strtol(cursor, NULL, 10);
						if (iterations_num < 0) {
							(print_json) ? my_fprintf(stderr, "Iterations must be >= 0 [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																					  "\e[1;31mIterations must be >= 1 [CODE %d]\e[0m\n",
																					  OPH_TERM_INVALID_PARAM_VALUE);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
								break;
							}
							continue;
						}
						cursor = strtok_r(NULL, " \t\n", &saveptr);
						if (cursor) {
							time_interval = (int) strtol(cursor, NULL, 10);
							if (time_interval < 1) {
								(print_json) ? my_fprintf(stderr, "Time interval must be >= 1 seconds [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																							     "\e[1;31mTime interval must be >= 1 seconds [CODE %d]\e[0m\n",
																							     OPH_TERM_INVALID_PARAM_VALUE);
								if (print_json)
									print_oph_term_output_json(hashtbl);
								if (exec_one_statement) {
									oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
									break;
								}
								continue;
							}
						}
					}

					int open_img = !strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "open");
					int save_img = open_img || !strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "save");
					int show_list = strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_FORMAT), "compact");
					if (view_status
					    (iterations_num, command_line, tmp_submission_string, hashtbl, &oph_term_return, tmp_session, tmp_workflow, save_img, open_img, show_list, time_interval,
					     NULL)) {
						(print_json) ? my_fprintf(stderr, "Error viewing status [CODE %d]\\n", OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																			 "\e[1;31mError viewing status [CODE %d]\e[0m\n",
																			 OPH_TERM_GENERIC_ERROR);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_GENERIC_ERROR;
							break;
						}
						continue;
					}
				} else {	//view wid[#mkid]
					char tmp_workflow[OPH_TERM_MAX_LEN];
					char tmp_marker[OPH_TERM_MAX_LEN];

					memset(tmp_marker, 0, OPH_TERM_MAX_LEN);
					memset(tmp_workflow, 0, OPH_TERM_MAX_LEN);

					char *saveptr2 = NULL, *tmp_cursor;
					tmp_cursor = strtok_r(cursor, OPH_TERM_MARKER_DELIMITER, &saveptr2);
					if (!tmp_cursor) {
#ifndef NO_WORKFLOW
						(print_json) ? my_fprintf(stderr, "Workflow id not valid [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																				"\e[1;31mWorkflow id not valid [CODE %d]\e[0m\n",
																				OPH_TERM_INVALID_PARAM_VALUE);
#else
						(print_json) ? my_fprintf(stderr, "Id not valid [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mId not valid [CODE %d]\e[0m\n",
																		       OPH_TERM_INVALID_PARAM_VALUE);
#endif
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
							break;
						}
						continue;
					}
					snprintf(tmp_workflow, OPH_TERM_MAX_LEN, "%s", tmp_cursor);
					tmp_cursor = strtok_r(NULL, OPH_TERM_MARKER_DELIMITER, &saveptr2);

					if (tmp_cursor) {	//view wid#mkid
						snprintf(tmp_marker, OPH_TERM_MAX_LEN, "%s", tmp_cursor);

						if (oph_term_check_wid_mkid
						    ((hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) ? ((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) : "", tmp_workflow, tmp_marker,
						     _user, _passwd, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST), (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT),
						     &oph_term_return, hashtbl)) {
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement)
								break;
							continue;
						}

						char *tmp_command = NULL;
						if (oph_term_get_request_with_marker
						    ((hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) ? ((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) : "", tmp_marker, _user,
						     _passwd, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST), (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT),
						     &oph_term_return, &tmp_command, NULL, hashtbl)) {
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement)
								break;
							continue;
						}
						(print_json) ? my_printf("[%s#%s] %s [%s?%s#%s]\\n\\n", tmp_workflow, tmp_marker, (tmp_command) ? tmp_command : "",
									 (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) ? ((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) : "",
									 tmp_workflow, tmp_marker) : printf("\e[1;34m[%s#%s]\e[0m %s \e[1;34m[%s?%s#%s]\e[0m\n\n", tmp_workflow, tmp_marker,
													    (tmp_command) ? tmp_command : "",
													    (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) ? ((char *)
																				   hashtbl_get(hashtbl,
																					       OPH_TERM_ENV_OPH_SESSION_ID))
													    : "", tmp_workflow, tmp_marker);
						if (tmp_command) {
							free(tmp_command);
							tmp_command = NULL;
						}

						n += snprintf(submission_string + n, OPH_TERM_MAX_LEN - n, "id=%s;id_type=marker;", tmp_marker);

						cursor = strtok_r(NULL, " \t\n", &saveptr);

						// Retrieve status filter
						if (cursor && !strcmp(cursor, "-s")) {
							cursor = strtok_r(NULL, " \t\n", &saveptr);
							if (cursor) {
								n += snprintf(submission_string + n, OPH_TERM_MAX_LEN - n, "status_filter=%s;", cursor);
								cursor = strtok_r(NULL, " \t\n", &saveptr);
							}
						}
						// Retrieve iterations and interval
						if (cursor) {
							if (watching) {
								watching = 0;
								(print_json) ? my_fprintf(stderr, "This command cannot be repeated during watching [CODE %d]\\n",
											  OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																  "\e[1;31mThis command cannot be repeated during watching [CODE %d]\e[0m\n",
																  OPH_TERM_INVALID_PARAM_VALUE);
								if (print_json)
									print_oph_term_output_json(hashtbl);
								if (exec_one_statement) {
									oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
									break;
								}
								continue;
							}
							iterations_num = (int) strtol(cursor, NULL, 10);
							if (iterations_num < 0) {
								(print_json) ? my_fprintf(stderr, "Iterations must be >= 0 [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																						  "\e[1;31mIterations must be >= 1 [CODE %d]\e[0m\n",
																						  OPH_TERM_INVALID_PARAM_VALUE);
								if (print_json)
									print_oph_term_output_json(hashtbl);
								if (exec_one_statement) {
									oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
									break;
								}
								continue;
							}
							cursor = strtok_r(NULL, " \t\n", &saveptr);
							if (cursor) {
								time_interval = (int) strtol(cursor, NULL, 10);
								if (time_interval < 1) {
									(print_json) ? my_fprintf(stderr, "Time interval must be >= 1 seconds [CODE %d]\\n",
												  OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																	  "\e[1;31mTime interval must be >= 1 seconds [CODE %d]\e[0m\n",
																	  OPH_TERM_INVALID_PARAM_VALUE);
									if (print_json)
										print_oph_term_output_json(hashtbl);
									if (exec_one_statement) {
										oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
										break;
									}
									continue;
								}
							}
						}

						int open_img = !strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "open");
						int save_img = open_img || !strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "save");
						int show_list = strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_FORMAT), "compact");
						if (view_status
						    (iterations_num, command_line, submission_string, hashtbl, &oph_term_return,
						     (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) ? ((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) : "", tmp_workflow, save_img,
						     open_img, show_list, time_interval, NULL)) {
							(print_json) ? my_fprintf(stderr, "Error viewing status [CODE %d]\\n", OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																				 "\e[1;31mError viewing status [CODE %d]\e[0m\n",
																				 OPH_TERM_GENERIC_ERROR);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_GENERIC_ERROR;
								break;
							}
							continue;
						}
					} else {	//view wid
						if (!strcmp(tmp_workflow, "0")) {
							(print_json) ? my_fprintf(stderr, "The identifier 0 is not valid\\n") : fprintf(stderr, "\e[1;31mThe identifier 0 is not valid\e[0m\n");
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
								break;
							}
							continue;
						}
						size_t k, stop = 0;
						for (k = 0; k < strlen(tmp_workflow); k++) {
							if (!isdigit(tmp_workflow[k])) {
								(print_json) ? my_fprintf(stderr, "The identifier is not a number\\n") : fprintf(stderr,
																		 "\e[1;31mThe identifier is not a number\e[0m\n");
								stop = 1;
								break;
							}
						}
						if (stop) {
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
								break;
							}
							continue;
						}

						char *tmp_command = NULL;
						char *tmp_jobid = NULL;
						if (oph_term_get_request
						    ((hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) ? ((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) : "", tmp_workflow, _user,
						     _passwd, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST), (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT),
						     &oph_term_return, &tmp_command, &tmp_jobid, NULL, hashtbl)) {
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement)
								break;
							continue;
						}
						(print_json) ? my_printf("[%s] %s [%s]\\n\\n", tmp_workflow, (tmp_command) ? tmp_command : "",
									 (tmp_jobid) ? tmp_jobid : "") : printf("\e[1;34m[%s]\e[0m %s \e[1;34m[%s]\e[0m\n\n", tmp_workflow,
														(tmp_command) ? tmp_command : "", (tmp_jobid) ? tmp_jobid : "");
						if (tmp_command) {
							free(tmp_command);
							tmp_command = NULL;
						}
						if (tmp_jobid) {
							free(tmp_jobid);
							tmp_jobid = NULL;
						}

						n += snprintf(submission_string + n, OPH_TERM_MAX_LEN - n, "id=%s;", tmp_workflow);

						cursor = strtok_r(NULL, " \t\n", &saveptr);

						// Retrieve status filter
						if (cursor && !strcmp(cursor, "-s")) {
							cursor = strtok_r(NULL, " \t\n", &saveptr);
							if (cursor) {
								n += snprintf(submission_string + n, OPH_TERM_MAX_LEN - n, "status_filter=%s;", cursor);
								cursor = strtok_r(NULL, " \t\n", &saveptr);
							}
						}
						// Retrieve iterations and interval
						if (cursor) {
							if (watching) {
								watching = 0;
								(print_json) ? my_fprintf(stderr, "This command cannot be repeated during watching [CODE %d]\\n",
											  OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																  "\e[1;31mThis command cannot be repeated during watching [CODE %d]\e[0m\n",
																  OPH_TERM_INVALID_PARAM_VALUE);
								if (print_json)
									print_oph_term_output_json(hashtbl);
								if (exec_one_statement) {
									oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
									break;
								}
								continue;
							}
							iterations_num = (int) strtol(cursor, NULL, 10);
							if (iterations_num < 0) {
								(print_json) ? my_fprintf(stderr, "Iterations must be >= 0 [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																						  "\e[1;31mIterations must be >= 1 [CODE %d]\e[0m\n",
																						  OPH_TERM_INVALID_PARAM_VALUE);
								if (print_json)
									print_oph_term_output_json(hashtbl);
								if (exec_one_statement) {
									oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
									break;
								}
								continue;
							}
							cursor = strtok_r(NULL, " \t\n", &saveptr);
							if (cursor) {
								time_interval = (int) strtol(cursor, NULL, 10);
								if (time_interval < 1) {
									(print_json) ? my_fprintf(stderr, "Time interval must be >= 1 seconds [CODE %d]\\n",
												  OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																	  "\e[1;31mTime interval must be >= 1 seconds [CODE %d]\e[0m\n",
																	  OPH_TERM_INVALID_PARAM_VALUE);
									if (print_json)
										print_oph_term_output_json(hashtbl);
									if (exec_one_statement) {
										oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
										break;
									}
									continue;
								}
							}
						}

						int open_img = !strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "open");
						int save_img = open_img || !strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "save");
						int show_list = strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_FORMAT), "compact");
						if (view_status
						    (iterations_num, command_line, submission_string, hashtbl, &oph_term_return,
						     (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) ? ((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) : "", tmp_workflow, save_img,
						     open_img, show_list, time_interval, NULL)) {
							(print_json) ? my_fprintf(stderr, "Error viewing status [CODE %d]\\n", OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																				 "\e[1;31mError viewing status [CODE %d]\e[0m\n",
																				 OPH_TERM_GENERIC_ERROR);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_GENERIC_ERROR;
								break;
							}
							continue;
						}
					}
				}
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_RESUME)) {	// RESUME
			alias_substitutions = 0;

			// DO VARIABLE SUBSTITUTION
			char *new_line = NULL;
			if (oph_term_full_var_expansion(line, hashtbl, &new_line)) {
				(print_json) ? my_fprintf(stderr, "Error performing variable substitution\\n") : fprintf(stderr, "\e[1;31mError performing variable substitution\e[0m\n");
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_GENERIC_ERROR;
					break;
				}
				continue;
			}
			free(linecopy);
			linecopy = new_line;
			new_line = NULL;
			cursor = strtok_r(linecopy, " \t\n", &saveptr);

			if (watching && !exec_alias2 && cursor) {
				cursor = strtok_r(NULL, " \t\n", &saveptr);
				if (oph_get_winterval(&cursor, &saveptr, NULL)) {
					(print_json) ? my_fprintf(stderr, "Wrong use of option '-n' [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			   "\e[1;31mWrong use of option '-n' [CODE %d]\e[0m\n",
																			   OPH_TERM_INVALID_PARAM_VALUE);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
						break;
					}
					continue;
				}
			}
#ifndef INTERFACE_TYPE_IS_GSI
			if (!_user) {
				(print_json) ? my_fprintf(stderr, "OPH_USER not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mOPH_USER not set [CODE %d]\e[0m\n",
																	   OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!_passwd) {
				(print_json) ? my_fprintf(stderr, "OPH_PASSWD not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mOPH_PASSWD not set [CODE %d]\e[0m\n",
																	     OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
#endif
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST)) {
				(print_json) ? my_fprintf(stderr, "OPH_SERVER_HOST not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		  "\e[1;31mOPH_SERVER_HOST not set [CODE %d]\e[0m\n",
																		  OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT)) {
				(print_json) ? my_fprintf(stderr, "OPH_SERVER_PORT not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		  "\e[1;31mOPH_SERVER_PORT not set [CODE %d]\e[0m\n",
																		  OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER) ||
			    (strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "dump") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "basic") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "coloured") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "extended") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "extended_coloured"))) {
				(print_json) ? my_fprintf(stderr, "OPH_TERM_VIEWER not set or incorrect [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			       "\e[1;31mOPH_TERM_VIEWER not set or incorrect [CODE %d]\e[0m\n",
																			       OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS) ||
			    (strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "no_op") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "save") && strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "open"))) {
				(print_json) ? my_fprintf(stderr, "OPH_TERM_IMGS not set or incorrect [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			     "\e[1;31mOPH_TERM_IMGS not set or incorrect [CODE %d]\e[0m\n",
																			     OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}

			int last_njobs = 10;	//default
			int is_verbose = 0;
			char tmp_session[OPH_TERM_MAX_LEN];
			memset(tmp_session, 0, OPH_TERM_MAX_LEN);
			cursor = strtok_r(NULL, " \t\n", &saveptr);
			if (!cursor) {	//resume
				snprintf(tmp_session, OPH_TERM_MAX_LEN, "%s", (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID));
			} else {
				if (strstr(cursor, "sessions")) {	//resume sessionid ...
					snprintf(tmp_session, OPH_TERM_MAX_LEN, "%s", cursor);
					cursor = strtok_r(NULL, " \t\n", &saveptr);
					if (cursor) {
						if (!strcmp(cursor, "-v")) {	//resume sessionid -v ...
							is_verbose = 1;
							cursor = strtok_r(NULL, " \t\n", &saveptr);
							if (cursor)	//resume sessionid -v n
								last_njobs = (int) strtol(cursor, NULL, 10);
						} else {	//resume sessionid n ...
							last_njobs = (int) strtol(cursor, NULL, 10);
							cursor = strtok_r(NULL, " \t\n", &saveptr);
							if (cursor && !strcmp(cursor, "-v"))	//resume sessionid n -v
								is_verbose = 1;
						}
					}
				} else if (!strcmp(cursor, "-v")) {	//resume -v ...
					is_verbose = 1;
					cursor = strtok_r(NULL, " \t\n", &saveptr);
					if (!cursor) {	//resume -v
						snprintf(tmp_session, OPH_TERM_MAX_LEN, "%s", (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID));
					} else {
						if (strstr(cursor, "sessions")) {	//resume -v sessionid ...
							snprintf(tmp_session, OPH_TERM_MAX_LEN, "%s", cursor);
							cursor = strtok_r(NULL, " \t\n", &saveptr);
							if (cursor)	//resume -v sessionid n
								last_njobs = (int) strtol(cursor, NULL, 10);
						} else {	//resume -v n
							snprintf(tmp_session, OPH_TERM_MAX_LEN, "%s", (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID));
							last_njobs = (int) strtol(cursor, NULL, 10);
						}
					}
				} else {	//resume n ...
					snprintf(tmp_session, OPH_TERM_MAX_LEN, "%s", (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID));
					last_njobs = (int) strtol(cursor, NULL, 10);
					cursor = strtok_r(NULL, " \t\n", &saveptr);
					if (cursor && !strcmp(cursor, "-v"))	//resume n -v
						is_verbose = 1;
				}
			}

			if (last_njobs > 0) {
				int end = 0;
				char **exit_status = NULL;

				//retrieve number of jobs of new session from server
				if (oph_term_get_session_size
				    (tmp_session, _user, _passwd, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST), (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT),
				     &oph_term_return, &end, 1, hashtbl, &exit_status)) {
					if (exit_status) {
						int jj;
						for (jj = 0; jj < end; ++jj)
							if (exit_status[jj])
								free(exit_status[jj]);
						free(exit_status);
					}
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_GENERIC_ERROR;
						break;
					}
					continue;
				}

				last_njobs = (last_njobs > end) ? end : last_njobs;
				int i, stop = 0, format;
				char *tmp_command = NULL;
				char *tmp_jobid = NULL;
				char *tmp_status = NULL;
				char *tmp_request_time = NULL;
				char buf[OPH_TERM_MAX_LEN];

				for (i = last_njobs - 1; i >= 0; i--) {
					memset(buf, 0, OPH_TERM_MAX_LEN);
					snprintf(buf, OPH_TERM_MAX_LEN, "%d", end - i);
					if (oph_term_get_request
					    (tmp_session, buf, _user, _passwd, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST), (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT),
					     &oph_term_return, &tmp_command, &tmp_jobid, &tmp_request_time, hashtbl)) {
						stop = 1;
						break;
					}

					format = 1;
					tmp_status = NULL;
					if (exit_status) {
						tmp_status = exit_status[end - i - 1];
						if (!strcmp(tmp_status, "OPH_STATUS_COMPLETED"))
							format = 2;
						else if (!strcmp(tmp_status, "OPH_STATUS_RUNNING"))
							format = 3;
						else if (!strcmp(tmp_status, "OPH_STATUS_WAITING"))
							format = 4;
						else if (!strcmp(tmp_status, "OPH_STATUS_PENDING"))
							format = 5;
						else if (!strcmp(tmp_status, "OPH_STATUS_RUNNING_ERROR")) {
							free(tmp_status);
							tmp_status = exit_status[end - i - 1] = strdup("OPH_STATUS_RUNNING");
						}
					}

					if (is_verbose)
						(print_json) ? my_printf("[%s] %s [%s]\\n%*s%s\\n%*s[%s]\\n", buf, tmp_request_time ? tmp_request_time : "", tmp_status ? tmp_status : "",
									 (int) (strlen(buf) + 3), " ", (tmp_command) ? tmp_command : "", (int) (strlen(buf) + 3), " ",
									 (tmp_jobid) ? tmp_jobid : "") : printf("\e[1;34m[%s]\e[0m %s \e[1;3%dm[%s]\e[0m\n%*s%s\n%*s\e[1;34m[%s]\e[0m\n", buf,
														tmp_request_time ? tmp_request_time : "", format, tmp_status ? tmp_status : "",
														(int) (strlen(buf) + 3), " ", (tmp_command) ? tmp_command : "", (int) (strlen(buf) + 3),
														" ", (tmp_jobid) ? tmp_jobid : "");
					else {
						if (tmp_command && (strlen(tmp_command) > OPH_TERM_CMD_MAX_LEN))
							snprintf(tmp_command + OPH_TERM_CMD_MAX_LEN - 5, 5, " ...");
						(print_json) ? my_printf("[%s] %s [%s]\t%s\\n", buf, tmp_request_time ? tmp_request_time : "", tmp_status ? tmp_status : "",
									 (tmp_command) ? tmp_command : "") : printf("\e[1;34m[%s]\e[0m %s \e[1;3%dm[%s]\e[0m\t%s\n", buf,
														    tmp_request_time ? tmp_request_time : "", format, tmp_status ? tmp_status : "",
														    (tmp_command) ? tmp_command : "");
					}

					if (tmp_command) {
						free(tmp_command);
						tmp_command = NULL;
					}
					if (tmp_jobid) {
						free(tmp_jobid);
						tmp_jobid = NULL;
					}
				}
				if (exit_status) {
					int jj;
					for (jj = 0; jj < end; ++jj)
						if (exit_status[jj])
							free(exit_status[jj]);
					free(exit_status);
				}
				if (stop) {
					stop = 0;
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_GENERIC_ERROR;
						break;
					}
					continue;
				}
			}
			// SWITCH SESSION IF DIFFERENT
			if ((hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID) && strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID), tmp_session))
			    || !hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) {
				if (oph_term_switch_remote_session
				    (tmp_session, _user, _passwd, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST), (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT),
				     &oph_term_return, 1, hashtbl)) {
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_GENERIC_ERROR;
						break;
					}
					continue;
				}
				if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID, tmp_session)) {
					(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_SESSION_ID, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																						  "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																						  OPH_TERM_ENV_OPH_SESSION_ID,
																						  OPH_TERM_MEMORY_ERROR);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_MEMORY_ERROR;
						break;
					}
					continue;
				}
				(print_json) ? my_printf("\\nCurrent session is now \\\"%s\\\".\\n", tmp_session) : printf("\n\e[2mCurrent session is now \"%s\".\e[0m\n", tmp_session);

				// Force to update configuration parameters
				get_config = 1;

				// Init OPH_CWD
				char *last_cwd = NULL;
				if (oph_term_env_oph_get_config
				    (OPH_TERM_OPH_CWD_KEY, hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST), hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), &oph_term_return, &last_cwd, _user,
				     _passwd, 1, hashtbl)) {
					(print_json) ? my_fprintf(stderr, "\\nWarning: Unable to resume last cwd\\n") : fprintf(stderr, "\e[2m\nWarning: Unable to resume last cwd\e[0m\n");
					oph_term_return = OPH_TERM_SUCCESS;
					if (last_cwd) {
						free(last_cwd);
						last_cwd = NULL;
					}
				}
				if (last_cwd && strlen(last_cwd)) {
					if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_CWD, last_cwd)) {
						(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_CWD, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																						   "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																						   OPH_TERM_ENV_OPH_CWD,
																						   OPH_TERM_MEMORY_ERROR);
						if (!print_json)
							printf("\e[0m");
						free(last_cwd);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_MEMORY_ERROR;
							break;
						}
						continue;
					}
					(print_json) ? my_printf("Last working directory was \\\"%s\\\".\\n", last_cwd) : printf("\e[2mLast working directory was \"%s\".\e[0m\n", last_cwd);
					free(last_cwd);
				} else {
					if (last_cwd) {
						free(last_cwd);
						last_cwd = NULL;
					}
					if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_CWD, "/")) {
						(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_CWD, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																						   "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																						   OPH_TERM_ENV_OPH_CWD,
																						   OPH_TERM_MEMORY_ERROR);
						if (!print_json)
							printf("\e[0m");
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_MEMORY_ERROR;
							break;
						}
						continue;
					}
					(print_json) ? my_printf("There is no cwd to resume. Current working directory is now \\\"/\\\" (session's root folder).\\n") :
					    printf("\e[2mThere is no cwd to resume. Current working directory is now \"/\" (session's root folder).\e[0m\n");
				}

				// Init OPH_DATACUBE
				char *last_cube = NULL;
				if (oph_term_env_oph_get_config
				    (OPH_TERM_OPH_DATACUBE_KEY, hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST), hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), &oph_term_return, &last_cube,
				     _user, _passwd, 1, hashtbl)) {
					(print_json) ? my_fprintf(stderr, "\\nWarning: Unable to resume last datacube\\n") : fprintf(stderr, "\e[2m\nWarning: Unable to resume last datacube\e[0m\n");
					oph_term_return = OPH_TERM_SUCCESS;
					if (last_cube) {
						free(last_cube);
						last_cube = NULL;
					}
				}
				if (last_cube && strlen(last_cube)) {
					if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_DATACUBE, last_cube)) {
						(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_DATACUBE, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																							"\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																							OPH_TERM_ENV_OPH_DATACUBE,
																							OPH_TERM_MEMORY_ERROR);
						if (!print_json)
							printf("\e[0m");
						free(last_cube);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_MEMORY_ERROR;
							break;
						}
						continue;
					}
					(print_json) ? my_printf("Last produced datacube was \\\"%s\\\".\\n", last_cube) : printf("\e[2mLast produced datacube was \"%s\".\e[0m\n", last_cube);
					free(last_cube);
				} else {
					if (last_cube) {
						free(last_cube);
						last_cube = NULL;
					}
					if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_DATACUBE))
						oph_term_unsetenv(hashtbl, OPH_TERM_ENV_OPH_DATACUBE);
					(print_json) ? my_fprintf(stderr, "\\nWarning: There is no datacube to resume\\n") : fprintf(stderr, "\e[2m\nWarning: There is no datacube to resume\e[0m\n");
				}
			}

			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_HELP)) {	// HELP
			alias_substitutions = 0;
			cursor = strtok_r(NULL, " \t\n\"", &saveptr);
			if (!cursor) {	// "help"
				oph_term_help(NULL);
			} else {	// "help abcd"
				if (oph_term_help(cursor)) {
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
						break;
					}
					continue;
				}
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_GETENV)) {	// GETENV
			alias_substitutions = 0;
			cursor = strtok_r(NULL, " \t\n\"", &saveptr);
			if (!cursor) {
				(print_json) ? my_fprintf(stderr, "Variable not present [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mVariable not present [CODE %d]\e[0m\n",
																	       OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			} else {
				oph_term_getenv(hashtbl, cursor);
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_ENV)) {	// ENV
			alias_substitutions = 0;
			cursor = strtok_r(NULL, " \t\n\"", &saveptr);
			if (cursor) {
				(print_json) ? my_fprintf(stderr, "No arguments required [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		"\e[1;31mNo arguments required [CODE %d]\e[0m\n",
																		OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			} else {
				oph_term_env(hashtbl);
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_SETENV)) {	// SETENV
			alias_substitutions = 0;
			cursor = strtok_r(NULL, " \t\n\"=", &saveptr);
			if (!cursor) {
				(print_json) ? my_fprintf(stderr, "Variable not present [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mVariable not present [CODE %d]\e[0m\n",
																	       OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			} else {
				char *key = strdup(cursor);
				if (!key) {
					(print_json) ? my_fprintf(stderr, "Could not extract key [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																		 "\e[1;31mCould not extract key [CODE %d]\e[0m\n",
																		 OPH_TERM_MEMORY_ERROR);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_MEMORY_ERROR;
						break;
					}
					continue;
				}
				cursor = strtok_r(NULL, "\t\n\"", &saveptr);
				if (!cursor) {
					(print_json) ? my_fprintf(stderr, "No value to set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mNo value to set [CODE %d]\e[0m\n",
																		  OPH_TERM_INVALID_PARAM_VALUE);
					free(key);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
						break;
					}
					continue;
				} else {
					if (oph_term_setenv(hashtbl, key, cursor)) {
						(print_json) ? my_fprintf(stderr, "Could not set variable [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			  "\e[1;31mCould not set variable [CODE %d]\e[0m\n",
																			  OPH_TERM_MEMORY_ERROR);
						free(key);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_MEMORY_ERROR;
							break;
						}
						continue;
					}
					free(key);
				}
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_UNSETENV)) {	// UNSETENV
			alias_substitutions = 0;
			cursor = strtok_r(NULL, " \t\n\"", &saveptr);
			if (!cursor) {
				(print_json) ? my_fprintf(stderr, "Variable not present [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mVariable not present [CODE %d]\e[0m\n",
																	       OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			} else {
				oph_term_unsetenv(hashtbl, cursor);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement)
					break;
				continue;
			}
		} else if (!strcmp(cursor, OPH_TERM_CMD_CLEAR)) {	// CLEAR
			alias_substitutions = 0;
			if (system("clear"))
				fprintf(stderr, "Error in executing command 'clear'\n");
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_UPDATE)) {	// UPDATE
			alias_substitutions = 0;
			cursor = strtok_r(NULL, " \t\n", &saveptr);
			if (!cursor) {	// "update"
#ifndef INTERFACE_TYPE_IS_GSI
				if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST) && hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT) && _user && _passwd) {
#else
				if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST) && hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT)) {
#endif
					// Force to update configuration parameters
					get_config = 1;

					// Retrieve correct URL from oph_server here
					char *tmpurl = NULL;
					if (oph_term_env_oph_get_config
					    (OPH_TERM_XML_URL_KEY, hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST), hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), &oph_term_return, &tmpurl,
					     _user, _passwd, 1, hashtbl)) {
						(print_json) ? my_fprintf(stderr, "Unable to get XML folder [CODE %d]\\n", OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																			     "\e[1;31mUnable to get XML folder [CODE %d]\e[0m\n",
																			     OPH_TERM_GENERIC_ERROR);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement)
							break;
						continue;
					}

					memset(xml_path_extended, 0, OPH_TERM_MAX_LEN);
					snprintf(xml_path_extended, OPH_TERM_MAX_LEN, "%s/%s", xml_path, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST));
					if (mkdir(xml_path, 0777)) {
						if (errno != EEXIST) {
							(print_json) ? my_fprintf(stderr, "Unable to create XML folder %s [CODE %d]\\n", xml_path, OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																						     "\e[1;31mUnable to create XML folder %s [CODE %d]\e[0m\n",
																						     xml_path,
																						     OPH_TERM_GENERIC_ERROR);
							free(tmpurl);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_GENERIC_ERROR;
								break;
							}
							continue;
						}
					}
					if (mkdir(xml_path_extended, 0777)) {
						if (errno != EEXIST) {
							(print_json) ? my_fprintf(stderr, "Unable to create XML folder %s [CODE %d]\\n", xml_path_extended, OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																							      "\e[1;31mUnable to create XML folder %s [CODE %d]\e[0m\n",
																							      xml_path_extended,
																							      OPH_TERM_GENERIC_ERROR);
							free(tmpurl);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_GENERIC_ERROR;
								break;
							}
							continue;
						}
					}
					oph_term_env_update_xml(tmpurl, NULL, xml_path_extended);
					free(tmpurl);
					oph_term_env_xmlauto_clear(xml_defs, operators_list, operators_list_size);
					if (oph_term_env_xmlauto_init(&xml_defs, xml_path_extended)) {
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_GENERIC_ERROR;
							break;
						}
						continue;
					}
					if (oph_term_env_xmlauto_get_operators(xml_defs, &operators_list, &operators_list_size)) {
						oph_term_env_xmlauto_clear(xml_defs, operators_list, operators_list_size);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_GENERIC_ERROR;
							break;
						}
						continue;
					}
				} else {
					(print_json) ? my_fprintf(stderr, "OPH_SERVER_HOST or OPH_SERVER_PORT not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																					     "\e[1;31mOPH_SERVER_HOST or OPH_SERVER_PORT not set [CODE %d]\e[0m\n",
																					     OPH_TERM_INVALID_PARAM_VALUE);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
						break;
					}
					continue;
				}
			} else if (!strcmp(cursor, "-f")) {	// "update -f"
#ifndef INTERFACE_TYPE_IS_GSI
				if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST) && hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT) && _user && _passwd) {
#else
				if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST) && hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT)) {
#endif
					// Force to update configuration parameters
					get_config = 1;

					// Retrieve correct URL from oph_server here
					char *tmpurl = NULL;
					if (oph_term_env_oph_get_config
					    (OPH_TERM_XML_URL_KEY, hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST), hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), &oph_term_return, &tmpurl,
					     _user, _passwd, 1, hashtbl)) {
						(print_json) ? my_fprintf(stderr, "Unable to get XML folder [CODE %d]\\n", OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																			     "\e[1;31mUnable to get XML folder [CODE %d]\e[0m\n",
																			     OPH_TERM_GENERIC_ERROR);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement)
							break;
						continue;
					}

					memset(xml_path_extended, 0, OPH_TERM_MAX_LEN);
					snprintf(xml_path_extended, OPH_TERM_MAX_LEN, "%s/%s", xml_path, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST));
					if (mkdir(xml_path, 0777)) {
						if (errno != EEXIST) {
							(print_json) ? my_fprintf(stderr, "Unable to create XML folder %s [CODE %d]\\n", xml_path, OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																						     "\e[1;31mUnable to create XML folder %s [CODE %d]\e[0m\n",
																						     xml_path,
																						     OPH_TERM_GENERIC_ERROR);
							free(tmpurl);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_GENERIC_ERROR;
								break;
							}
							continue;
						}
					}
					if (mkdir(xml_path_extended, 0777)) {
						if (errno != EEXIST) {
							(print_json) ? my_fprintf(stderr, "Unable to create XML folder %s [CODE %d]\\n", xml_path_extended, OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																							      "\e[1;31mUnable to create XML folder %s [CODE %d]\e[0m\n",
																							      xml_path_extended,
																							      OPH_TERM_GENERIC_ERROR);
							free(tmpurl);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_GENERIC_ERROR;
								break;
							}
							continue;
						}
					}
					if (oph_term_env_rmdir(xml_path_extended)) {
						(print_json) ? my_fprintf(stderr, "Unable to remove XML folder %s [CODE %d]\\n", xml_path_extended, OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																						      "\e[1;31mUnable to remove XML folder %s [CODE %d]\e[0m\n",
																						      xml_path_extended,
																						      OPH_TERM_GENERIC_ERROR);
						free(tmpurl);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_GENERIC_ERROR;
							break;
						}
						continue;
					}
					(print_json) ? my_printf("Directory %s and all its XML files removed.\\n", xml_path_extended) : printf("Directory %s and all its XML files removed.\n",
																	       xml_path_extended);
					if (mkdir(xml_path, 0777)) {
						if (errno != EEXIST) {
							(print_json) ? my_fprintf(stderr, "Unable to create XML folder %s [CODE %d]\\n", xml_path, OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																						     "\e[1;31mUnable to create XML folder %s [CODE %d]\e[0m\n",
																						     xml_path,
																						     OPH_TERM_GENERIC_ERROR);
							free(tmpurl);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_GENERIC_ERROR;
								break;
							}
							continue;
						}
					}
					if (mkdir(xml_path_extended, 0777)) {
						if (errno != EEXIST) {
							(print_json) ? my_fprintf(stderr, "Unable to create XML folder %s [CODE %d]\\n", xml_path_extended, OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																							      "\e[1;31mUnable to create XML folder %s [CODE %d]\e[0m\n",
																							      xml_path_extended,
																							      OPH_TERM_GENERIC_ERROR);
							free(tmpurl);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_GENERIC_ERROR;
								break;
							}
							continue;
						}
					}
					oph_term_env_update_xml(tmpurl, NULL, xml_path_extended);
					free(tmpurl);
					oph_term_env_xmlauto_clear(xml_defs, operators_list, operators_list_size);
					if (oph_term_env_xmlauto_init(&xml_defs, xml_path_extended)) {
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_GENERIC_ERROR;
							break;
						}
						continue;
					}
					if (oph_term_env_xmlauto_get_operators(xml_defs, &operators_list, &operators_list_size)) {
						oph_term_env_xmlauto_clear(xml_defs, operators_list, operators_list_size);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_GENERIC_ERROR;
							break;
						}
						continue;
					}
				} else {
					(print_json) ? my_fprintf(stderr, "OPH_SERVER_HOST or OPH_SERVER_PORT not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																					     "\e[1;31mOPH_SERVER_HOST or OPH_SERVER_PORT not set [CODE %d]\e[0m\n",
																					     OPH_TERM_INVALID_PARAM_VALUE);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
						break;
					}
					continue;
				}
			} else {
				(print_json) ? my_fprintf(stderr, "Option %s not recognized [CODE %d]\\n", cursor, OPH_TERM_OPTION_NOT_RECOGNIZED) : fprintf(stderr,
																			     "\e[1;31mOption %s not recognized [CODE %d]\e[0m\n",
																			     cursor, OPH_TERM_OPTION_NOT_RECOGNIZED);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_OPTION_NOT_RECOGNIZED;
					break;
				}
				continue;
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		}
#ifndef NO_WORKFLOW
		else if (cursor[0] == '.' || cursor[0] == '/') {	// ./WORKFLOW.JSON
			alias_substitutions = 0;

			// DO VARIABLE SUBSTITUTION
			char *new_line = NULL;
			if (oph_term_full_var_expansion(line, hashtbl, &new_line)) {
				(print_json) ? my_fprintf(stderr, "Error performing variable substitution\\n") : fprintf(stderr, "\e[1;31mError performing variable substitution\e[0m\n");
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_GENERIC_ERROR;
					break;
				}
				continue;
			}
			free(linecopy);
			linecopy = new_line;
			new_line = NULL;
			cursor = strtok_r(linecopy, " \t\n", &saveptr);

			if (watching && !exec_alias2 && cursor) {
				cursor = strtok_r(NULL, " \t\n", &saveptr);
				if (oph_get_winterval(&cursor, &saveptr, NULL)) {
					(print_json) ? my_fprintf(stderr, "Wrong use of option '-n' [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			   "\e[1;31mWrong use of option '-n' [CODE %d]\e[0m\n",
																			   OPH_TERM_INVALID_PARAM_VALUE);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
						break;
					}
					continue;
				}
			}
#ifndef INTERFACE_TYPE_IS_GSI
			if (!_user) {
				(print_json) ? my_fprintf(stderr, "OPH_USER not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mOPH_USER not set [CODE %d]\e[0m\n",
																	   OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!_passwd) {
				(print_json) ? my_fprintf(stderr, "OPH_PASSWD not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mOPH_PASSWD not set [CODE %d]\e[0m\n",
																	     OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
#endif
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST)) {
				(print_json) ? my_fprintf(stderr, "OPH_SERVER_HOST not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		  "\e[1;31mOPH_SERVER_HOST not set [CODE %d]\e[0m\n",
																		  OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT)) {
				(print_json) ? my_fprintf(stderr, "OPH_SERVER_PORT not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		  "\e[1;31mOPH_SERVER_PORT not set [CODE %d]\e[0m\n",
																		  OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER) ||
			    (strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "dump") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "basic") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "coloured") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "extended") &&
			     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), "extended_coloured"))) {
				(print_json) ? my_fprintf(stderr, "OPH_TERM_VIEWER not set or incorrect [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			       "\e[1;31mOPH_TERM_VIEWER not set or incorrect [CODE %d]\e[0m\n",
																			       OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			// LOAD WORKFLOW FROM FILE
			char *submission_workflow = NULL;
			if (oph_term_read_file(cursor, &submission_workflow)) {
				(print_json) ? my_fprintf(stderr, "Error loading workflow from file\\n") : fprintf(stderr, "\e[1;31mError loading workflow from file\e[0m\n");
				if (submission_workflow) {
					free(submission_workflow);
					submission_workflow = NULL;
				}
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_GENERIC_ERROR;
					break;
				}
				continue;
			}
			// DO ARGUMENT SUBSTITUTION
			char *tmpworkflow = NULL;
			int count = 1, stop = 0;
			char var[10];
			cursor = strtok_r(NULL, " \t\n\"", &saveptr);
			while (cursor) {
				memset(var, 0, 10);
				snprintf(var, 10, "%d", count);
				if (oph_term_var_expansion(submission_workflow, var, cursor, &tmpworkflow)) {
					(print_json) ? my_fprintf(stderr, "Error performing argument substitution\\n") : fprintf(stderr, "\e[1;31mError performing argument substitution\e[0m\n");
					if (submission_workflow) {
						free(submission_workflow);
						submission_workflow = NULL;
					}
					stop = 1;
					break;
				}
				free(submission_workflow);
				submission_workflow = tmpworkflow;
				tmpworkflow = NULL;
				cursor = strtok_r(NULL, " \t\n\"", &saveptr);
				count++;
			}
			if (stop) {
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_GENERIC_ERROR;
					break;
				}
				continue;
			}
			// DO VARIABLE SUBSTITUTION
			tmpworkflow = NULL;
			if (oph_term_full_var_expansion(submission_workflow, hashtbl, &tmpworkflow)) {
				(print_json) ? my_fprintf(stderr, "Error performing variable substitution\\n") : fprintf(stderr, "\e[1;31mError performing variable substitution\e[0m\n");
				if (submission_workflow) {
					free(submission_workflow);
					submission_workflow = NULL;
				}
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_GENERIC_ERROR;
					break;
				}
				continue;
			}
			free(submission_workflow);
			submission_workflow = tmpworkflow;
			tmpworkflow = NULL;

			// VALIDATION
			oph_workflow *tmp_workflow = NULL;
			if (oph_workflow_load(submission_workflow, "", &tmp_workflow)) {
				(print_json) ? my_fprintf(stderr, "Workflow is not a valid Ophidia Workflow JSON file.\\n") : fprintf(stderr,
																      "\e[1;31mWorkflow is not a valid Ophidia Workflow JSON file.\e[0m\n");
				if (submission_workflow) {
					free(submission_workflow);
					submission_workflow = NULL;
				}
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (oph_workflow_indexing(tmp_workflow->tasks, tmp_workflow->tasks_num)) {
				(print_json) ? my_fprintf(stderr, "There are some problems with the tasks (duplicates, loops, etc.)\\n") : fprintf(stderr,
																		   "\e[1;31mThere are some problems with the tasks (duplicates, loops, etc.)\e[0m\n");
				(print_json) ? my_fprintf(stderr, "Workflow is not a valid Ophidia Workflow JSON file.\\n") : fprintf(stderr,
																      "\e[1;31mWorkflow is not a valid Ophidia Workflow JSON file.\e[0m\n");
				if (submission_workflow) {
					free(submission_workflow);
					submission_workflow = NULL;
				}
				oph_workflow_free(tmp_workflow);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			int error;
			if ((error = oph_workflow_validate(tmp_workflow))) {
				switch (error) {
					case OPH_WORKFLOW_EXIT_TASK_NAME_ERROR:
						(print_json) ? my_fprintf(stderr, "There are tasks with the same name\\n") : fprintf(stderr, "\e[1;31mThere are tasks with the same name\e[0m\n");
						break;
					case OPH_WORKFLOW_EXIT_FLOW_CONTROL_ERROR:
						(print_json) ? my_fprintf(stderr, "Flow control operators are not set correctly\\n") : fprintf(stderr,
																	       "\e[1;31mFlow control operators are not set correctly\e[0m\n");
						break;
					default:
						(print_json) ? my_fprintf(stderr, "Probably the workflow is not a DAG\\n") : fprintf(stderr, "\e[1;31mProbably the workflow is not a DAG\e[0m\n");
				}
				(print_json) ? my_fprintf(stderr, "Workflow is not a valid Ophidia Workflow JSON file.\\n") : fprintf(stderr,
																      "\e[1;31mWorkflow is not a valid Ophidia Workflow JSON file.\e[0m\n");
				if (submission_workflow) {
					free(submission_workflow);
					submission_workflow = NULL;
				}
				oph_workflow_free(tmp_workflow);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			oph_workflow_free(tmp_workflow);

			// SUBMISSION
			char *response_for_viewer = NULL;
			char *session = NULL;
			int last_jobid = 0;
			if (hashtbl_insert(hashtbl, OPH_TERM_ENV_LAST_JOBID, "get", strlen("get") + 1)) {
				(print_json) ? my_fprintf(stderr, "Error retrieving jobid\\n") : fprintf(stderr, "\e[1;31mError retrieving jobid\e[0m\n");
				if (submission_workflow) {
					free(submission_workflow);
					submission_workflow = NULL;
				}
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_MEMORY_ERROR;
					break;
				}
				continue;
			}
			oph_term_client(command_line, submission_workflow, &session, _user, _passwd, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST),
					(char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), &oph_term_return, NULL, &response_for_viewer, 0, hashtbl);
			if (submission_workflow) {
				free(submission_workflow);
				submission_workflow = NULL;
			}
			if (session) {
				free(session);
				session = NULL;
			}
			if (hashtbl_get(hashtbl, OPH_TERM_ENV_LAST_JOBID)) {
				last_jobid = (int) strtol((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_LAST_JOBID), NULL, 10);
				hashtbl_remove(hashtbl, OPH_TERM_ENV_LAST_JOBID);
			}
			if (!print_debug_data && response_for_viewer && ((int) strlen(response_for_viewer) > 1024 * strtol((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_RESPONSE_BUFFER), NULL, 10))) {
				if (response_for_viewer)
					free(response_for_viewer);
				response_for_viewer = NULL;
				(print_json) ? my_fprintf(stderr, "Response exceeds the maximum size %s kB.\\n", (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_RESPONSE_BUFFER)) : fprintf(stderr,
																							    "\e[1;31mResponse exceeds the maximum size %s kB.\e[0m\n",
																							    (char *)
																							    hashtbl_get
																							    (hashtbl,
																							     OPH_TERM_ENV_OPH_RESPONSE_BUFFER));
			}
			// VISUALIZATION
			if (response_for_viewer) {
				char *newtoken = NULL, *exectime = NULL;
				int viewer_res = oph_term_viewer((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), &response_for_viewer,
								 (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1)) ? ((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1)) : "red", 0, 0, 1,
								 NULL, NULL,
								 NULL, &newtoken, &exectime, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_GRAPH_LAYOUT));
				if (viewer_res != 0 && viewer_res != OPH_TERM_ERROR_WITHIN_JSON) {
					(print_json) ? my_fprintf(stderr, "Could not render result [CODE %d]\\n", OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																		    "\e[1;31mCould not render result [CODE %d]\e[0m\n",
																		    OPH_TERM_GENERIC_ERROR);
					if (newtoken)
						free(newtoken);
					if (exectime)
						free(exectime);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_GENERIC_ERROR;
						break;
					}
					continue;
				}
				if (newtoken) {
					pthread_mutex_lock(&global_flag);
					hashtbl_remove(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
					hashtbl_insert(hashtbl, OPH_TERM_ENV_OPH_TOKEN, newtoken, strlen(newtoken) + 1);
					_passwd = _token = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
					pthread_mutex_unlock(&global_flag);
					free(newtoken);
				}
				if (exectime) {
					(print_json) ? my_printf("Execution time: %s\\n", oph_print_exectime(&exectime)) : printf("Execution time: %s\n", oph_print_exectime(&exectime));
					free(exectime);
				}
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);

			if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_WORKFLOW_AUTOVIEW) && last_jobid > 0) {
				char *autoview = strdup((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_WORKFLOW_AUTOVIEW));
				if (!autoview) {
					(print_json) ? my_fprintf(stderr, "Could not parse OPH_WORKFLOW_AUTOVIEW [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																				 "\e[1;31mCould not parse OPH_WORKFLOW_AUTOVIEW [CODE %d]\e[0m\n",
																				 OPH_TERM_MEMORY_ERROR);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_MEMORY_ERROR;
						break;
					}
					continue;
				}
				char *token;
				char *svptr;
				token = strtok_r(autoview, "_", &svptr);
				if (!token) {
					free(autoview);
					(print_json) ? my_fprintf(stderr, "Invalid OPH_WORKFLOW_AUTOVIEW value [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																				      "\e[1;31mInvalid OPH_WORKFLOW_AUTOVIEW value [CODE %d]\e[0m\n",
																				      OPH_TERM_INVALID_PARAM_VALUE);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
						break;
					}
					continue;
				}
				if (strcmp(token, "on") && strcmp(token, "off")) {
					free(autoview);
					(print_json) ? my_fprintf(stderr, "Invalid OPH_WORKFLOW_AUTOVIEW value [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																				      "\e[1;31mInvalid OPH_WORKFLOW_AUTOVIEW value [CODE %d]\e[0m\n",
																				      OPH_TERM_INVALID_PARAM_VALUE);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
						break;
					}
					continue;
				}
				if (!strcmp(token, "on")) {
					int iterations = 0;
					int interval = 5;
					token = strtok_r(NULL, "_", &svptr);
					if (token) {
						iterations = (int) strtol((const char *) token, NULL, 10);
						if ((iterations == 0 && strcmp((const char *) token, "0")) || iterations < 0) {
							free(autoview);
							(print_json) ? my_fprintf(stderr, "Invalid OPH_WORKFLOW_AUTOVIEW value [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																						      "\e[1;31mInvalid OPH_WORKFLOW_AUTOVIEW value [CODE %d]\e[0m\n",
																						      OPH_TERM_INVALID_PARAM_VALUE);
							if (print_json)
								print_oph_term_output_json(hashtbl);
							if (exec_one_statement) {
								oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
								break;
							}
							continue;
						}
						token = strtok_r(NULL, "_", &svptr);
						if (token) {
							interval = (int) strtol((const char *) token, NULL, 10);
							if (interval < 1) {
								free(autoview);
								(print_json) ? my_fprintf(stderr, "Invalid OPH_WORKFLOW_AUTOVIEW value [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																							      "\e[1;31mInvalid OPH_WORKFLOW_AUTOVIEW value [CODE %d]\e[0m\n",
																							      OPH_TERM_INVALID_PARAM_VALUE);
								if (print_json)
									print_oph_term_output_json(hashtbl);
								if (exec_one_statement) {
									oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
									break;
								}
								continue;
							}
						}
					}
					// call "view last_jobid iterations interval"
					free(line);
					char view_string[OPH_TERM_MAX_LEN];
					memset(view_string, 0, OPH_TERM_MAX_LEN);
					snprintf(view_string, OPH_TERM_MAX_LEN, "view %d %d %d", last_jobid, iterations, interval);
					line = strdup((const char *) view_string);
					if (!line) {
						free(autoview);
						(print_json) ? my_fprintf(stderr, "Memory error calling view [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			     "\e[1;31mMemory error calling view [CODE %d]\e[0m\n",
																			     OPH_TERM_MEMORY_ERROR);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_MEMORY_ERROR;
							break;
						}
						continue;
					}
					free(autoview);
					exec_alias = 1;
					continue;
				}
				free(autoview);
			}

			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_CHECK)) {	// CHECK
			alias_substitutions = 0;

			// DO VARIABLE SUBSTITUTION
			char *new_line = NULL;
			if (oph_term_full_var_expansion(line, hashtbl, &new_line)) {
				(print_json) ? my_fprintf(stderr, "Error performing variable substitution\\n") : fprintf(stderr, "\e[1;31mError performing variable substitution\e[0m\n");
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_GENERIC_ERROR;
					break;
				}
				continue;
			}
			free(linecopy);
			linecopy = new_line;
			new_line = NULL;
			cursor = strtok_r(linecopy, " \t\n", &saveptr);

			if (watching && !exec_alias2 && cursor) {
				cursor = strtok_r(NULL, " \t\n", &saveptr);
				if (oph_get_winterval(&cursor, &saveptr, NULL)) {
					(print_json) ? my_fprintf(stderr, "Wrong use of option '-n' [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			   "\e[1;31mWrong use of option '-n' [CODE %d]\e[0m\n",
																			   OPH_TERM_INVALID_PARAM_VALUE);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
						break;
					}
					continue;
				}
			}

			cursor = strtok_r(NULL, " \t\n\"", &saveptr);
			if (!cursor) {
				(print_json) ? my_fprintf(stderr, "Workflow filename not present [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			"\e[1;31mWorkflow filename not present [CODE %d]\e[0m\n",
																			OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			} else {
				int view_metadata = 0;
				if (!strcmp(cursor, "-m")) {	// check -m wf.json p1 p2 ...
					cursor = strtok_r(NULL, " \t\n\"", &saveptr);
					if (!cursor) {
						(print_json) ? my_fprintf(stderr, "Workflow filename not present [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																					"\e[1;31mWorkflow filename not present [CODE %d]\e[0m\n",
																					OPH_TERM_INVALID_PARAM_VALUE);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
							break;
						}
						continue;
					}
					view_metadata = 1;
				}
				// check [-m] wf.json p1 p2 ...
				if (!view_metadata) {
					if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS) ||
					    (strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "no_op") &&
					     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "save") && strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "open"))) {
						(print_json) ? my_fprintf(stderr, "OPH_TERM_IMGS not set or incorrect [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																					     "\e[1;31mOPH_TERM_IMGS not set or incorrect [CODE %d]\e[0m\n",
																					     OPH_TERM_INVALID_PARAM_VALUE);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
							break;
						}
						continue;
					}
					if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_GRAPH_LAYOUT) ||
					    (strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_GRAPH_LAYOUT), "horizontal_compact") &&
					     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_GRAPH_LAYOUT), "vertical_compact") &&
					     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_GRAPH_LAYOUT), "horizontal_ranked") &&
					     strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_GRAPH_LAYOUT), "vertical_ranked"))) {
						(print_json) ? my_fprintf(stderr, "OPH_GRAPH_LAYOUT not set or incorrect [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																						"\e[1;31mOPH_GRAPH_LAYOUT not set or incorrect [CODE %d]\e[0m\n",
																						OPH_TERM_INVALID_PARAM_VALUE);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
							break;
						}
						continue;
					}
				}
				// LOAD WORKFLOW FROM FILE
				char *submission_workflow = NULL;
				if (oph_term_read_file(cursor, &submission_workflow)) {
					(print_json) ? my_fprintf(stderr, "Error loading workflow from file\\n") : fprintf(stderr, "\e[1;31mError loading workflow from file\e[0m\n");
					if (submission_workflow) {
						free(submission_workflow);
						submission_workflow = NULL;
					}
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_GENERIC_ERROR;
						break;
					}
					continue;
				}
				// DO ARGUMENT SUBSTITUTION
				char *tmpworkflow = NULL;
				int count = 1, stop = 0;
				char var[10];
				cursor = strtok_r(NULL, " \t\n\"", &saveptr);
				while (cursor) {
					memset(var, 0, 10);
					snprintf(var, 10, "%d", count);
					if (oph_term_var_expansion(submission_workflow, var, cursor, &tmpworkflow)) {
						(print_json) ? my_fprintf(stderr, "Error performing argument substitution\\n") : fprintf(stderr,
																	 "\e[1;31mError performing argument substitution\e[0m\n");
						if (submission_workflow) {
							free(submission_workflow);
							submission_workflow = NULL;
						}
						stop = 1;
						break;
					}
					free(submission_workflow);
					submission_workflow = tmpworkflow;
					tmpworkflow = NULL;
					cursor = strtok_r(NULL, " \t\n\"", &saveptr);
					count++;
				}
				if (stop) {
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_GENERIC_ERROR;
						break;
					}
					continue;
				}
				// DO VARIABLE SUBSTITUTION
				tmpworkflow = NULL;
				if (oph_term_full_var_expansion(submission_workflow, hashtbl, &tmpworkflow)) {
					(print_json) ? my_fprintf(stderr, "Error performing variable substitution\\n") : fprintf(stderr, "\e[1;31mError performing variable substitution\e[0m\n");
					if (submission_workflow) {
						free(submission_workflow);
						submission_workflow = NULL;
					}
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_GENERIC_ERROR;
						break;
					}
					continue;
				}
				free(submission_workflow);
				submission_workflow = tmpworkflow;
				tmpworkflow = NULL;

				// VALIDATION
				oph_workflow *tmp_workflow = NULL;
				if (oph_workflow_load(submission_workflow, "", &tmp_workflow)) {
					(print_json) ? my_fprintf(stderr, "Workflow is NOT a VALID Ophidia Workflow.\\n") : fprintf(stderr,
																    "Workflow is \e[1;31mNOT\e[0m a \e[1;31mVALID\e[0m Ophidia Workflow.\n");
					if (submission_workflow) {
						free(submission_workflow);
						submission_workflow = NULL;
					}
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
						break;
					}
					continue;
				}
				if (submission_workflow) {
					free(submission_workflow);
					submission_workflow = NULL;
				}

				if (view_metadata) {
					char expanded_field[2 * OUTPUT_MAX_LEN] = "\0";
					expand_escapes(expanded_field, tmp_workflow->name);
					(print_json) ? my_printf("Name\\n----\\n%s\\n\\n", expanded_field) : printf("Name\n----\n%s\n\n", tmp_workflow->name);
					expand_escapes(expanded_field, tmp_workflow->author);
					(print_json) ? my_printf("Author\\n------\\n%s\\n\\n", expanded_field) : printf("Author\n------\n%s\n\n", tmp_workflow->author);
					expand_escapes(expanded_field, tmp_workflow->abstract);
					(print_json) ? my_printf("Abstract\\n--------\\n%s\\n\\n", expanded_field) : printf("Abstract\n--------\n%s\n\n", tmp_workflow->abstract);
					if (tmp_workflow->url) {
						expand_escapes(expanded_field, tmp_workflow->url);
						(print_json) ? my_printf("URL\\n--------\\n%s\\n\\n", expanded_field) : printf("URL\n--------\n%s\n\n", tmp_workflow->url);
					}

					(print_json) ? my_printf("Other info\\n----------\\n") : printf("Other info\n----------\n");

					if (tmp_workflow->sessionid) {
						expand_escapes(expanded_field, tmp_workflow->sessionid);
						(print_json) ? my_printf("sessionid: %s\\n", expanded_field) : printf("sessionid: %s\n", tmp_workflow->sessionid);
					}
					if (tmp_workflow->exec_mode) {
						expand_escapes(expanded_field, tmp_workflow->exec_mode);
						(print_json) ? my_printf("exec_mode: %s\\n", expanded_field) : printf("exec_mode: %s\n", tmp_workflow->exec_mode);
					}
					if (tmp_workflow->ncores != 0) {
						(print_json) ? my_printf("ncores: %d\\n", tmp_workflow->ncores) : printf("ncores: %d\n", tmp_workflow->ncores);
					}
					if (tmp_workflow->cwd) {
						expand_escapes(expanded_field, tmp_workflow->cwd);
						(print_json) ? my_printf("cwd: %s\\n", expanded_field) : printf("cwd: %s\n", tmp_workflow->cwd);
					}
					if (tmp_workflow->cube) {
						expand_escapes(expanded_field, tmp_workflow->cube);
						(print_json) ? my_printf("cube: %s\\n", expanded_field) : printf("cube: %s\n", tmp_workflow->cube);
					}
					if (tmp_workflow->callback_url) {
						expand_escapes(expanded_field, tmp_workflow->callback_url);
						(print_json) ? my_printf("callback_url: %s\\n", expanded_field) : printf("callback_url: %s\n", tmp_workflow->callback_url);
					}
					if (tmp_workflow->on_error) {
						expand_escapes(expanded_field, tmp_workflow->on_error);
						(print_json) ? my_printf("on_error: %s\\n", expanded_field) : printf("on_error: %s\n", tmp_workflow->on_error);
					}
					if (tmp_workflow->command) {
						expand_escapes(expanded_field, tmp_workflow->command);
						(print_json) ? my_printf("command: %s\\n", expanded_field) : printf("command: %s\n", tmp_workflow->command);
					}
					if (tmp_workflow->on_exit) {
						expand_escapes(expanded_field, tmp_workflow->on_exit);
						(print_json) ? my_printf("on_exit: %s\\n", expanded_field) : printf("on_exit: %s\n", tmp_workflow->on_exit);
					}
					if (tmp_workflow->run) {
						expand_escapes(expanded_field, tmp_workflow->run);
						(print_json) ? my_printf("run: %s\\n", expanded_field) : printf("run: %s\n", tmp_workflow->run);
					}
					if (tmp_workflow->host_partition) {
						expand_escapes(expanded_field, tmp_workflow->host_partition);
						(print_json) ? my_printf("host_partition: %s\\n", expanded_field) : printf("host_partition: %s\n", tmp_workflow->host_partition);
					}
					oph_workflow_free(tmp_workflow);
				} else {
					if (oph_workflow_indexing(tmp_workflow->tasks, tmp_workflow->tasks_num)) {
						(print_json) ? my_fprintf(stderr, "There are some problems with the tasks (duplicates, loops, etc.)\\n") : fprintf(stderr,
																				   "\e[1;31mThere are some problems with the tasks (duplicates, loops, etc.)\e[0m\n");
						(print_json) ? my_fprintf(stderr, "Workflow is NOT a VALID Ophidia Workflow.\\n") : fprintf(stderr,
																	    "Workflow is \e[1;31mNOT\e[0m a \e[1;31mVALID\e[0m Ophidia Workflow.\n");
						oph_workflow_free(tmp_workflow);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
							break;
						}
						continue;
					}
					int error;
					if ((error = oph_workflow_validate(tmp_workflow))) {
						switch (error) {
							case OPH_WORKFLOW_EXIT_TASK_NAME_ERROR:
								(print_json) ? my_fprintf(stderr, "There are tasks with the same name\\n") : fprintf(stderr,
																		     "\e[1;31mThere are tasks with the same name\e[0m\n");
								break;
							case OPH_WORKFLOW_EXIT_FLOW_CONTROL_ERROR:
								(print_json) ? my_fprintf(stderr, "Flow control operators are not set correctly\\n") : fprintf(stderr,
																			       "\e[1;31mFlow control operators are not set correctly\e[0m\n");
								break;
							default:
								(print_json) ? my_fprintf(stderr, "Probably the workflow is not a DAG\\n") : fprintf(stderr,
																		     "\e[1;31mProbably the workflow is not a DAG\e[0m\n");
						}
						(print_json) ? my_fprintf(stderr, "Workflow is NOT a VALID Ophidia Workflow.\\n") : fprintf(stderr,
																	    "Workflow is \e[1;31mNOT\e[0m a \e[1;31mVALID\e[0m Ophidia Workflow.\n");
						oph_workflow_free(tmp_workflow);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
							break;
						}
						continue;
					}
					(print_json) ? my_printf("Specified workflow (after parameter substitution) is a VALID Ophidia Workflow.\\n") :
					    printf("Specified workflow (after parameter substitution) is a \e[1;32mVALID\e[0m Ophidia Workflow.\n");

					// VISUALIZATION
					int open_img = !strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "open");
					int save_img = open_img || !strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "save");
					int show_list = strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_FORMAT), "compact");
					if (view_status(0, NULL, NULL, hashtbl, NULL, NULL, NULL, save_img, open_img, show_list, 0, tmp_workflow)) {
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_GENERIC_ERROR;
							break;
						}
						continue;
					}
				}
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		}
#endif
		else if (!strcmp(cursor, OPH_TERM_CMD_GETALIAS)) {	// GETALIAS
			alias_substitutions = 0;
			cursor = strtok_r(NULL, " \t\n\"", &saveptr);
			if (!cursor) {
				(print_json) ? my_fprintf(stderr, "Alias not present [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mAlias not present [CODE %d]\e[0m\n",
																	    OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			} else {
				oph_term_getalias(aliases, cursor);
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_ALIAS)) {	// ALIAS
			alias_substitutions = 0;
			cursor = strtok_r(NULL, " \t\n\"", &saveptr);
			if (cursor) {
				(print_json) ? my_fprintf(stderr, "No arguments required [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		"\e[1;31mNo arguments required [CODE %d]\e[0m\n",
																		OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			} else {
				oph_term_alias(aliases);
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_SETALIAS)) {	// SETALIAS
			alias_substitutions = 0;
			cursor = strtok_r(NULL, " \t\n\"=", &saveptr);
			if (!cursor) {
				(print_json) ? my_fprintf(stderr, "Alias not present [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mAlias not present [CODE %d]\e[0m\n",
																	    OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			} else {
				char *key = strdup(cursor);
				if (!key) {
					(print_json) ? my_fprintf(stderr, "Could not extract key [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																		 "\e[1;31mCould not extract key [CODE %d]\e[0m\n",
																		 OPH_TERM_MEMORY_ERROR);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_MEMORY_ERROR;
						break;
					}
					continue;
				}
				cursor = strtok_r(NULL, "\t\n\"", &saveptr);
				if (!cursor) {
					(print_json) ? my_fprintf(stderr, "No value to set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mNo value to set [CODE %d]\e[0m\n",
																		  OPH_TERM_INVALID_PARAM_VALUE);
					free(key);
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
						break;
					}
					continue;
				} else {
					if (oph_term_setalias(aliases, key, cursor)) {
						(print_json) ? my_fprintf(stderr, "Could not set alias [CODE %d]\\n", OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																		       "\e[1;31mCould not set alias [CODE %d]\e[0m\n",
																		       OPH_TERM_MEMORY_ERROR);
						free(key);
						if (print_json)
							print_oph_term_output_json(hashtbl);
						if (exec_one_statement) {
							oph_term_return = OPH_TERM_MEMORY_ERROR;
							break;
						}
						continue;
					}
					free(key);
				}
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_UNSETALIAS)) {	// UNSETALIAS
			alias_substitutions = 0;
			cursor = strtok_r(NULL, " \t\n\"", &saveptr);
			if (!cursor) {
				(print_json) ? my_fprintf(stderr, "Alias not present [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr, "\e[1;31mAlias not present [CODE %d]\e[0m\n",
																	    OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			} else {
				oph_term_unsetalias(aliases, cursor);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement)
					break;
				continue;
			}
		} else if (is_alias(aliases, cursor)) {	// alias routine

			// DO ARGUMENT SUBSTITUTION
			char *new_line = NULL;
			char *param_string = (char *) strdup((char *) hashtbl_get(aliases, cursor));
			if (!param_string) {
				(print_json) ? my_fprintf(stderr, "Error duplicating alias\\n") : fprintf(stderr, "\e[1;31mError duplicating alias\e[0m\n");
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_MEMORY_ERROR;
					break;
				}
				continue;
			}
			int count = 1, stop = 0;
			char var[10];
			cursor = strtok_r(NULL, " \t\n\"", &saveptr);
			while (cursor) {
				memset(var, 0, 10);
				snprintf(var, 10, "%d", count);
				if (oph_term_var_expansion(param_string, var, cursor, &new_line)) {
					(print_json) ? my_fprintf(stderr, "Error performing argument substitution\\n") : fprintf(stderr, "\e[1;31mError performing argument substitution\e[0m\n");
					stop = 1;
					break;
				}
				free(param_string);
				param_string = new_line;
				new_line = NULL;
				cursor = strtok_r(NULL, " \t\n\"", &saveptr);
				count++;
			}
			if (stop) {
				if (param_string)
					free(param_string);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_GENERIC_ERROR;
					break;
				}
				continue;
			}

			free(line);
			line = param_string;
			param_string = NULL;
			alias_substitutions++;
			if (alias_substitutions == OPH_TERM_SUBSTITUTION_MAX_CYCLES) {
				(print_json) ? my_fprintf(stderr, "Too much substitutions! Perhaps there are one or more cycles.\\n") : fprintf(stderr,
																		"\e[1;31mToo much substitutions! Perhaps there are one or more cycles.\e[0m\n");
				alias_substitutions = 0;
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			exec_alias = 1;

			continue;
		}
#ifdef WITH_IM_SUPPORT
		else if (!strcmp(cursor, OPH_TERM_CMD_DEPLOY)) {	// DEPLOY
			alias_substitutions = 0;

			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL)) {
				(print_json) ? my_fprintf(stderr, "OPH_INFRASTRUCTURE_URL not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			 "\e[1;31mOPH_INFRASTRUCTURE_URL not set [CODE %d]\e[0m\n",
																			 OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_AUTH_HEADER)) {
				(print_json) ? my_fprintf(stderr, "OPH_AUTH_HEADER not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		  "\e[1;31mOPH_AUTH_HEADER not set [CODE %d]\e[0m\n",
																		  OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}

			cursor = strtok_r(NULL, " \t\n\"", &saveptr);
			if (!cursor) {
				(print_json) ? my_fprintf(stderr, "RADL filename not present [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		    "\e[1;31mRADL filename not present [CODE %d]\e[0m\n",
																		    OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			} else {
				if (oph_term_env_deploy
				    ((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_AUTH_HEADER), (const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL), cursor, hashtbl)) {
					if (print_json)
						print_oph_term_output_json(hashtbl);
					if (exec_one_statement) {
						oph_term_return = OPH_TERM_GENERIC_ERROR;
						break;
					}
					continue;
				}
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement)
					break;
				continue;
			}
		} else if (!strcmp(cursor, OPH_TERM_CMD_DEPLOY_STATUS)) {	// DEPLOY_STATUS
			alias_substitutions = 0;

			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL)) {
				(print_json) ? my_fprintf(stderr, "OPH_INFRASTRUCTURE_URL not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			 "\e[1;31mOPH_INFRASTRUCTURE_URL not set [CODE %d]\e[0m\n",
																			 OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_AUTH_HEADER)) {
				(print_json) ? my_fprintf(stderr, "OPH_AUTH_HEADER not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		  "\e[1;31mOPH_AUTH_HEADER not set [CODE %d]\e[0m\n",
																		  OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}

			if (oph_term_env_deploy_status((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_AUTH_HEADER), (const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL))) {
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_GENERIC_ERROR;
					break;
				}
				continue;
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_GET_SERVER)) {	// GET_SERVER
			alias_substitutions = 0;

			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL)) {
				(print_json) ? my_fprintf(stderr, "OPH_INFRASTRUCTURE_URL not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			 "\e[1;31mOPH_INFRASTRUCTURE_URL not set [CODE %d]\e[0m\n",
																			 OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_AUTH_HEADER)) {
				(print_json) ? my_fprintf(stderr, "OPH_AUTH_HEADER not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		  "\e[1;31mOPH_AUTH_HEADER not set [CODE %d]\e[0m\n",
																		  OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}

			if (oph_term_env_get_server
			    ((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_AUTH_HEADER), (const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL), hashtbl)) {
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_GENERIC_ERROR;
					break;
				}
				continue;
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_UNDEPLOY)) {	// UNDEPLOY
			alias_substitutions = 0;

			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL)) {
				(print_json) ? my_fprintf(stderr, "OPH_INFRASTRUCTURE_URL not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			 "\e[1;31mOPH_INFRASTRUCTURE_URL not set [CODE %d]\e[0m\n",
																			 OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_AUTH_HEADER)) {
				(print_json) ? my_fprintf(stderr, "OPH_AUTH_HEADER not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		  "\e[1;31mOPH_AUTH_HEADER not set [CODE %d]\e[0m\n",
																		  OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}

			if (oph_term_env_undeploy((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_AUTH_HEADER), (const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL), hashtbl)) {
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_GENERIC_ERROR;
					break;
				}
				continue;
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_DEPLOYS_LIST)) {	// DEPLOYS_LIST
			alias_substitutions = 0;

			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL)) {
				(print_json) ? my_fprintf(stderr, "OPH_INFRASTRUCTURE_URL not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			 "\e[1;31mOPH_INFRASTRUCTURE_URL not set [CODE %d]\e[0m\n",
																			 OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_AUTH_HEADER)) {
				(print_json) ? my_fprintf(stderr, "OPH_AUTH_HEADER not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		  "\e[1;31mOPH_AUTH_HEADER not set [CODE %d]\e[0m\n",
																		  OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}

			if (oph_term_env_deploys_list((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_AUTH_HEADER), (const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL))) {
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_GENERIC_ERROR;
					break;
				}
				continue;
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		} else if (!strcmp(cursor, OPH_TERM_CMD_DEPLOY_VMS_LIST)) {	// DEPLOY_VMS_LIST
			alias_substitutions = 0;

			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL)) {
				(print_json) ? my_fprintf(stderr, "OPH_INFRASTRUCTURE_URL not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																			 "\e[1;31mOPH_INFRASTRUCTURE_URL not set [CODE %d]\e[0m\n",
																			 OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}
			if (!hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_AUTH_HEADER)) {
				(print_json) ? my_fprintf(stderr, "OPH_AUTH_HEADER not set [CODE %d]\\n", OPH_TERM_INVALID_PARAM_VALUE) : fprintf(stderr,
																		  "\e[1;31mOPH_AUTH_HEADER not set [CODE %d]\e[0m\n",
																		  OPH_TERM_INVALID_PARAM_VALUE);
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_INVALID_PARAM_VALUE;
					break;
				}
				continue;
			}

			if (oph_term_env_deploy_vms_list((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_AUTH_HEADER), (const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL))) {
				if (print_json)
					print_oph_term_output_json(hashtbl);
				if (exec_one_statement) {
					oph_term_return = OPH_TERM_GENERIC_ERROR;
					break;
				}
				continue;
			}
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement)
				break;
			continue;
		}
#endif
		else {		// command not recognized
			alias_substitutions = 0;
			(print_json) ? my_fprintf(stderr, "Command %s not recognized [CODE %d]\\n", cursor, OPH_TERM_COMMAND_NOT_RECOGNIZED) : fprintf(stderr,
																		       "\e[1;31mCommand %s not recognized [CODE %d]\e[0m\n",
																		       cursor, OPH_TERM_COMMAND_NOT_RECOGNIZED);
			if (print_json)
				print_oph_term_output_json(hashtbl);
			if (exec_one_statement) {
				oph_term_return = OPH_TERM_COMMAND_NOT_RECOGNIZED;
				break;
			}
			continue;
		}
	}

	/* CLEAR HISTORY */
	clear_history();
	if (hist_list) {
		free(hist_list);
		hist_list = NULL;
	}
	if (hist_path) {
		free(hist_path);
		hist_path = NULL;
	}

	/* CLEAR READLINE */
	if (line) {
		free(line);
		line = NULL;
	}
	if (linecopy) {
		free(linecopy);
		linecopy = NULL;
	}
	if (current_operator) {
		free(current_operator);
		current_operator = NULL;
	}

	/* CLEAR ENV */
	oph_term_env_clear(hashtbl);
	oph_term_alias_clear(aliases);
	if (xml_path) {
		free(xml_path);
		xml_path = NULL;
	}

	/* CLEAR XML DEFS */
	if (!exec_one_statement) {
		oph_term_env_end_xml_parser();
		oph_term_env_xmlauto_clear(xml_defs, operators_list, operators_list_size);
	}

	pthread_mutex_destroy(&global_flag);

	/* EXIT */
	if (exec_one_statement)
		return oph_term_return;
	return OPH_TERM_SUCCESS;
}
