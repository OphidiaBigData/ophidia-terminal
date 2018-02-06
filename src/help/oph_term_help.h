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

#ifndef OPH_TERM_HELP_H_
#define OPH_TERM_HELP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Command names
#include "../oph_term.h"
//Status codes
#include "../env/oph_term_status.h"

//Print usage for cmd or the list of commands if NULL
int oph_term_help(const char *cmd);

//COMMANDS HELP
#define OPH_TERM_HELP_VERSION_SHORT "\"show current version\"\n"
#define OPH_TERM_HELP_VERSION_LONG \
"\e[1mUSAGE OF version COMMAND\e[0m\n\n\
\tversion\n\n\
\tShow the current Oph_Term version.\n"
#define OPH_TERM_HELP_VERSION_LONG2 \
"USAGE OF version COMMAND\\n\\n\
\\tversion\\n\\n\
\\tShow the current Oph_Term version.\\n"

#define OPH_TERM_HELP_WARRANTY_SHORT "\"show the disclaimer of warranty\"\n"
#define OPH_TERM_HELP_WARRANTY_LONG \
"\e[1mUSAGE OF warranty COMMAND\e[0m\n\n\
\twarranty\n\n\
\tShow the Disclaimer of Warranty.\n"
#define OPH_TERM_HELP_WARRANTY_LONG2 \
"USAGE OF warranty COMMAND\\n\\n\
\\twarranty\\n\\n\
\\tShow the Disclaimer of Warranty.\\n"

#define OPH_TERM_HELP_WATCH_SHORT "\"repeat command submission automatically\"\n"
#define OPH_TERM_HELP_WATCH_LONG \
"\e[1mUSAGE OF watch COMMAND\e[0m\n\n\
\twatch [-n seconds] cmd\n\n\
\tRuns the command \"cmd\" repeatedly, displaying its output. By default, the program is run every 2 seconds; use \"-n\" to specify a different interval.\n"
#define OPH_TERM_HELP_WATCH_LONG2 \
"USAGE OF watch COMMAND\\n\\n\
\\twatch [-n seconds] cmd\\n\\n\
\\tRuns the command \\\"cmd\\\" repeatedly, displaying its output. By default, the program is run every 2 seconds; use \\\"-n\\\" to specify a different interval.\\n"

#define OPH_TERM_HELP_CONDITIONS_SHORT "\"show the conditions to redistribute the software\"\n"
#define OPH_TERM_HELP_CONDITIONS_LONG \
"\e[1mUSAGE OF conditions COMMAND\e[0m\n\n\
\tconditions\n\n\
\tShow the conditions to redistribute the software.\n"
#define OPH_TERM_HELP_CONDITIONS_LONG2 \
"USAGE OF conditions COMMAND\\n\\n\
\\tconditions\\n\\n\
\\tShow the conditions to redistribute the software.\\n"

#define OPH_TERM_HELP_HELP_SHORT "\"get the description of a command or variable\"\n"
#define OPH_TERM_HELP_HELP_LONG \
"\e[1mUSAGE OF help COMMAND\e[0m\n\n\
\thelp [cmd|var]\n\n\
\tWithout arguments, list all available commands and the environment variables.\n\
\tOptionally append a command/variable name to get its usage.\n"
#define OPH_TERM_HELP_HELP_LONG2 \
"USAGE OF help COMMAND\\n\\n\
\\thelp [cmd|var]\\n\\n\
\\tWithout arguments, list all available commands and the environment variables.\\n\
\\tOptionally append a command/variable name to get its usage.\\n"

#define OPH_TERM_HELP_HISTORY_SHORT "\"manage the history of commands\"\n"
#define OPH_TERM_HELP_HISTORY_LONG \
"\e[1mUSAGE OF history COMMAND\e[0m\n\n\
\thistory [-c]\n\n\
\tWithout options, print the content of the file \".oph_term_history\" located in your HOME.\n\
\tThat file is created the first time Oph_Term is successfully launched.\n\
\tAll records are printed in a human-readable format with a command and its date and time.\n\
\tWith -c, \".oph_term_history\" is truncated (content cleared but file not removed) as well as current local Oph_Term session history.\n"
#define OPH_TERM_HELP_HISTORY_LONG2 \
"USAGE OF history COMMAND\\n\\n\
\\thistory [-c]\\n\\n\
\\tWithout options, print the content of the file \\\".oph_term_history\\\" located in your HOME.\\n\
\\tThat file is created the first time Oph_Term is successfully launched.\\n\
\\tAll records are printed in a human-readable format with a command and its date and time.\\n\
\\tWith -c, \\\".oph_term_history\\\" is truncated (content cleared but file not removed) as well as current local Oph_Term session history.\\n"

#define OPH_TERM_HELP_ENV_SHORT "\"list environment variables\"\n"
#define OPH_TERM_HELP_ENV_LONG \
"\e[1mUSAGE OF env COMMAND\e[0m\n\n\
\tenv\n\n\
\tList all environment variables with their values.\n"
#define OPH_TERM_HELP_ENV_LONG2 \
"USAGE OF env COMMAND\\n\\n\
\\tenv\\n\\n\
\\tList all environment variables with their values.\\n"

#define OPH_TERM_HELP_SETENV_SHORT "\"set or change the value of a variable\"\n"
#define OPH_TERM_HELP_SETENV_LONG \
"\e[1mUSAGE OF setenv COMMAND\e[0m\n\n\
\tsetenv ENV_VAR=\"value\"\n\n\
\tSet the environment variable ENV_VAR to \"value\".\n\
\tFor example, <setenv OPH_USER=\"user1\"> or <setenv OPH_USER=user1> are valid commands.\n\
\tPre-defined variable names are always capitalized with no characters except letters, numbers and underscores.\n\
\tThe value can be surrounded by double quotes but cannot contain double/single quotes or semicolon.\n\
\tTo insert a special character, just escape it like \"\\t\".\n\
\tSo these commands are correct:\n\
\t<setenv OPH_USER=username>, <setenv OPH_USER=\"username\">,\n\
\t<setenv OPH_USER=userwith space>, <setenv OPH_USER=\"userwith space\">,<setenv OPH_USER =\"user name\">...\n\
\tAnd these commands are invalid:\n\
\t<setenv OPH_USER=\"use\"rname>, <setenv OPH_USER = \"username\">,\n\
\t<setenv OPH_USER= username>, <setenv OPH_USER = username>...\n\
\tUse this command also to create new user-defined variables. Each variable does not have to start with\n\
\t\"oph_\",\"/\" or \".\" and it does not have to be a number.\n"
#define OPH_TERM_HELP_SETENV_LONG2 \
"USAGE OF setenv COMMAND\\n\\n\
\\tsetenv ENV_VAR=\\\"value\\\"\\n\\n\
\\tSet the environment variable ENV_VAR to \\\"value\\\".\\n\
\\tFor example, <setenv OPH_USER=\\\"user1\\\"> or <setenv OPH_USER=user1> are valid commands.\\n\
\\tPre-defined variable names are always capitalized with no characters except letters, numbers and underscores.\\n\
\\tThe value can be surrounded by double quotes but cannot contain double/single quotes or semicolon.\\n\
\\tTo insert a special character, just escape it like \\\"\\\\t\\\".\\n\
\\tSo these commands are correct:\\n\
\\t<setenv OPH_USER=username>, <setenv OPH_USER=\\\"username\\\">,\\n\
\\t<setenv OPH_USER=userwith space>, <setenv OPH_USER=\\\"userwith space\\\">,<setenv OPH_USER =\\\"user name\\\">...\\n\
\\tAnd these commands are invalid:\\n\
\\t<setenv OPH_USER=\\\"use\\\"rname>, <setenv OPH_USER = \\\"username\\\">,\\n\
\\t<setenv OPH_USER= username>, <setenv OPH_USER = username>...\\n\
\\tUse this command also to create new user-defined variables. Each variable does not have to start with\\n\
\\t\\\"oph_\\\",\\\"/\\\" or \\\".\\\" and it does not have to be a number.\\n"

#define OPH_TERM_HELP_UNSETENV_SHORT "\"unset an environment variable\"\n"
#define OPH_TERM_HELP_UNSETENV_LONG \
"\e[1mUSAGE OF unsetenv COMMAND\e[0m\n\n\
\tunsetenv ENV_VAR\n\n\
\tClear the content of the environment variable ENV_VAR.\n\
\tENV_VAR cannot be surrounded by or contain single or double/single quotes,spaces or other characters except letters, numbers and underscore.\n"
#define OPH_TERM_HELP_UNSETENV_LONG2 \
"USAGE OF unsetenv COMMAND\\n\\n\
\\tunsetenv ENV_VAR\\n\\n\
\\tClear the content of the environment variable ENV_VAR.\\n\
\\tENV_VAR cannot be surrounded by or contain single or double/single quotes,spaces or other characters except letters, numbers and underscore.\\n"

#define OPH_TERM_HELP_GETENV_SHORT "\"print the value of a variable\"\n"
#define OPH_TERM_HELP_GETENV_LONG \
"\e[1mUSAGE OF getenv COMMAND\e[0m\n\n\
\tgetenv ENV_VAR\n\n\
\tPrint the value of the environment variable ENV_VAR (if it exists).\n\
\tFor example, to view the current user just type <getenv OPH_USER>.\n\
\tENV_VAR cannot be surrounded by or contain single or double/single quotes,spaces or other characters except letters, numbers and underscore.\n"
#define OPH_TERM_HELP_GETENV_LONG2 \
"USAGE OF getenv COMMAND\\n\\n\
\\tgetenv ENV_VAR\\n\\n\
\\tPrint the value of the environment variable ENV_VAR (if it exists).\\n\
\\tFor example, to view the current user just type <getenv OPH_USER>.\\n\
\\tENV_VAR cannot be surrounded by or contain single or double/single quotes,spaces or other characters except letters, numbers and underscore.\\n"

#define OPH_TERM_HELP_QUIT_SHORT "\"quit Oph_Term\"\n"
#define OPH_TERM_HELP_QUIT_LONG \
"\e[1mUSAGE OF quit COMMAND\e[0m\n\n\
\tquit\n\n\
\tClear environment, free memory and close Oph_Term.\n"
#define OPH_TERM_HELP_QUIT_LONG2 \
"USAGE OF quit COMMAND\\n\\n\
\\tquit\\n\\n\
\\tClear environment, free memory and close Oph_Term.\\n"

#define OPH_TERM_HELP_EXIT_SHORT "\"quit Oph_Term\"\n"
#define OPH_TERM_HELP_EXIT_LONG \
"\e[1mUSAGE OF exit COMMAND\e[0m\n\n\
\texit\n\n\
\tClear environment, free memory and close Oph_Term.\n"
#define OPH_TERM_HELP_EXIT_LONG2 \
"USAGE OF exit COMMAND\\n\\n\
\\texit\\n\\n\
\\tClear environment, free memory and close Oph_Term.\\n"

#define OPH_TERM_HELP_CLEAR_SHORT "\"clear screen\"\n"
#define OPH_TERM_HELP_CLEAR_LONG \
"\e[1mUSAGE OF clear COMMAND\e[0m\n\n\
\tclear\n\n\
\tClear screen.\n"
#define OPH_TERM_HELP_CLEAR_LONG2 \
"USAGE OF clear COMMAND\\n\\n\
\\tclear\\n\\n\
\\tClear screen.\\n"

#define OPH_TERM_HELP_UPDATE_SHORT "\"update local XML files\"\n"
#define OPH_TERM_HELP_UPDATE_LONG \
"\e[1mUSAGE OF update COMMAND\e[0m\n\n\
\tupdate [-f]\n\n\
\tDownload the latest version of the XML files of all active operators for the specified server (OPH_SERVER_HOST:OPH_SERVER_PORT)\n\
\tin order to use the Oph_Term autocompletion feature on Ophidia operators. OPH_SERVER_HOST and OPH_SERVER_PORT must be set before launching this command.\n\
\tLocal files removed from server are NOT preserved locally.\n\
\tIn case of -f option, force Oph_Term to completely remove all local files and download the latest version of the remote XMLs.\n"
#define OPH_TERM_HELP_UPDATE_LONG2 \
"USAGE OF update COMMAND\\n\\n\
\\tupdate [-f]\\n\\n\
\\tDownload the latest version of the XML files of all active operators for the specified server (OPH_SERVER_HOST:OPH_SERVER_PORT)\\n\
\\tin order to use the Oph_Term autocompletion feature on Ophidia operators. OPH_SERVER_HOST and OPH_SERVER_PORT must be set before launching this command.\\n\
\\tLocal files removed from server are NOT preserved locally.\\n\
\\tIn case of -f option, force Oph_Term to completely remove all local files and download the latest version of the remote XMLs.\\n"

#ifndef NO_WORKFLOW
#define OPH_TERM_HELP_RESUME_SHORT "\"resume session\"\n"
#define OPH_TERM_HELP_RESUME_LONG \
"\e[1mUSAGE OF resume COMMAND\e[0m\n\n\
\tresume [SessionID] [n] [-v]\n\n\
\tSwitch to another session specifying a new SessionID (or consider the current session) and view at the same time\n\
\t(at most) the last \"n\" commands launched in that session (default=10).\n\
\tThe output will have the format \"[workflowid] command\".\n\
\tWith \"-v\" the output will have the format \"[workflowid] command <newline> [jobid]\".\n"
#define OPH_TERM_HELP_RESUME_LONG2 \
"USAGE OF resume COMMAND\\n\\n\
\\tresume [SessionID] [n] [-v]\\n\\n\
\\tSwitch to another session specifying a new SessionID (or consider the current session) and view at the same time\\n\
\\t(at most) the last \\\"n\\\" commands launched in that session (default=10).\\n\
\\tThe output will have the format \\\"[workflowid] command\\\".\\n\
\\tWith \\\"-v\\\" the output will have the format \\\"[workflowid] command <newline> [jobid]\\\".\\n"
#else
#define OPH_TERM_HELP_RESUME_SHORT "\"resume session\"\n"
#define OPH_TERM_HELP_RESUME_LONG \
"\e[1mUSAGE OF resume COMMAND\e[0m\n\n\
\tresume [SessionID] [n] [-v]\n\n\
\tSwitch to another session specifying a new SessionID (or consider the current session) and view at the same time\n\
\t(at most) the last \"n\" commands launched in that session (default=10).\n\
\tThe output will have the format \"[id] command\".\n\
\tWith \"-v\" the output will have the format \"[id] command <newline> [jobid]\".\n"
#define OPH_TERM_HELP_RESUME_LONG2 \
"USAGE OF resume COMMAND\\n\\n\
\\tresume [SessionID] [n] [-v]\\n\\n\
\\tSwitch to another session specifying a new SessionID (or consider the current session) and view at the same time\\n\
\\t(at most) the last \\\"n\\\" commands launched in that session (default=10).\\n\
\\tThe output will have the format \\\"[id] command\\\".\\n\
\\tWith \\\"-v\\\" the output will have the format \\\"[id] command <newline> [jobid]\\\".\\n"
#endif

#ifndef NO_WORKFLOW
#define OPH_TERM_HELP_VIEW_SHORT "\"view jobs output/status\"\n"
#define OPH_TERM_HELP_VIEW_LONG \
"\e[1mUSAGE OF view COMMAND\e[0m\n\n\
\tview WorkflowID[#MarkerID] [-s status] [iterations [interval]]\n\
\tview -j JobID [-s status] [iterations [interval]]\n\n\
\tView the output/status of a workflow associated to the current session, identified by a workflow id, or view\n\
\tthe output/status of a particular task associated to the current session, identified by a workflow id and a marker id.\n\
\tWith option \"-j\" view the output/status associated to a particular JobID, even belonging to a different session.\n\
\tThe output of a single-and-simple-task workflow will be the output of the task, the output of a single-and-massive-task\n\
\tworkflow will be the list of tasks generated by the massive task (with marker ids), the output of a multi-task workflow will be\n\
\tthe list of the workflow tasks (with marker ids). To view the output of a task within a generic workflow, use the JobID or\n\
\ta combination of workflow id and marker id.\n\
\tThe output will be prefixed with a string with the format \"[workflowid#markerid] command [jobid]\".\n\
\tAccording to the value of OPH_TERM_IMGS, it can produce an image representing the real-time status of a multi-task workflow.\n\
\tFor running workflows, the option \"-s\" can be used to set a bitmap \"status\" and filter jobs based on their status\n\
\t(type \"man OPH_RESUME\" and refer to argument \"status_filter\" for further information).\n\
\tIt is also possible to specify the number of iterations the command will perform (0 for viewing status until workflow ends)\n\
\tand the time interval (in seconds) between iterations (default is 5 seconds).\n"
#define OPH_TERM_HELP_VIEW_LONG2 \
"USAGE OF view COMMAND\\n\\n\
\\tview WorkflowID[#MarkerID] [-s status] [iterations [interval]]\\n\
\\tview -j JobID [-s status] [iterations [interval]]\\n\\n\
\\tView the output/status of a workflow associated to the current session, identified by a workflow id, or view\\n\
\\tthe output/status of a particular task associated to the current session, identified by a workflow id and a marker id.\\n\
\\tWith option \\\"-j\\\" view the output/status associated to a particular JobID, even belonging to a different session.\\n\
\\tThe output of a single-and-simple-task workflow will be the output of the task, the output of a single-and-massive-task\\n\
\\tworkflow will be the list of tasks generated by the massive task (with marker ids), the output of a multi-task workflow will be\\n\
\\tthe list of the workflow tasks (with marker ids). To view the output of a task within a generic workflow, use the JobID or\\n\
\\ta combination of workflow id and marker id.\\n\
\\tThe output will be prefixed with a string with the format \\\"[workflowid#markerid] command [jobid]\\\".\\n\
\\tAccording to the value of OPH_TERM_IMGS, it can produce an image representing the real-time status of a multi-task workflow.\\n\
\\tFor running workflows, the option \\\"-s\\\" can be used to set a bitmap \\\"status\\\" and filter jobs based on their status\\n\
\\t(type \\\"man OPH_RESUME\\\" and refer to argument \\\"status_filter\\\" for further information).\\n\
\\tIt is also possible to specify the number of iterations the command will perform (0 for viewing status until workflow ends)\\n\
\\tand the time interval (in seconds) between iterations (default is 5 seconds).\\n"
#else
#define OPH_TERM_HELP_VIEW_SHORT "\"view jobs output/status\"\n"
#define OPH_TERM_HELP_VIEW_LONG \
"\e[1mUSAGE OF view COMMAND\e[0m\n\n\
\tview CommandID[#MarkerID] [iterations [interval]]\n\
\tview -j JobID [iterations [interval]]\n\n\
\tView the output/status of a command associated to the current session, identified by a command id, or view\n\
\tthe output/status of a particular task associated to the current session, identified by a command id and a marker id.\n\
\tWith option \"-j\" view the output/status associated to a particular JobID, even belonging to a different session.\n\
\tThe output of a simple command will be the output of the task, the output of a massive command\n\
\twill be the list of tasks generated by the massive task (with marker ids).\n\
\tThe output will be prefixed with a string with the format \"[commandid#markerid] command [jobid]\".\n\
\tIt is also possible to specify the number of iterations the command will perform (0 for viewing status until command ends)\n\
\tand the time interval (in seconds) between iterations (default is 5 seconds).\n"
#define OPH_TERM_HELP_VIEW_LONG2 \
"USAGE OF view COMMAND\\n\\n\
\\tview CommandID[#MarkerID] [iterations [interval]]\\n\
\\tview -j JobID [iterations [interval]]\\n\\n\
\\tView the output/status of a command associated to the current session, identified by a command id, or view\\n\
\\tthe output/status of a particular task associated to the current session, identified by a command id and a marker id.\\n\
\\tWith option \\\"-j\\\" view the output/status associated to a particular JobID, even belonging to a different session.\\n\
\\tThe output of a simple command will be the output of the task, the output of a massive command\\n\
\\twill be the list of tasks generated by the massive task (with marker ids).\\n\
\\tThe output will be prefixed with a string with the format \\\"[commandid#markerid] command [jobid]\\\".\\n\
\\tIt is also possible to specify the number of iterations the command will perform (0 for viewing status until command ends)\\n\
\\tand the time interval (in seconds) between iterations (default is 5 seconds).\\n"
#endif

#ifndef NO_WORKFLOW
#define OPH_TERM_HELP_CHECK_SHORT "\"validate a workflow\"\n"
#define OPH_TERM_HELP_CHECK_LONG \
"\e[1mUSAGE OF check COMMAND\e[0m\n\n\
\tcheck [-m] workflow.json [param1 [param2 [...]]]\n\n\
\tValidate \"workflow.json\" against the Ophidia Workflow JSON Schema.\n\
\tWith one or more parameters, the parameter substitution ($1,$2 etc.) is performed\n\
\tbefore the validation phase together with variable substitution.\n\
\tAfter the validation phase, according to the value of OPH_TERM_IMGS, it can produce an image representing the workflow.\n\
\tWith the option -m it returns the metadata relative to name,author and abstract and, if present,\n\
\tURL,sessionid,exec_mode,ncores,cwd,cdd,cube,callback_url,on_error,command,on_exit and run. In this case validation is not performed.\n"
#define OPH_TERM_HELP_CHECK_LONG2 \
"USAGE OF check COMMAND\\n\\n\
\\tcheck [-m] workflow.json [param1 [param2 [...]]]\\n\\n\
\\tValidate \\\"workflow.json\\\" against the Ophidia Workflow JSON Schema.\\n\
\\tWith one or more parameters, the parameter substitution ($1,$2 etc.) is performed\\n\
\\tbefore the validation phase together with variable substitution.\\n\
\\tAfter the validation phase, according to the value of OPH_TERM_IMGS, it can produce an image representing the workflow.\\n\
\\tWith the option -m it returns the metadata relative to name,author and abstract and, if present,\\n\
\\tURL,sessionid,exec_mode,ncores,cwd,cdd,cube,callback_url,on_error,command,on_exit and run. In this case validation is not performed.\\n"
#endif

#define OPH_TERM_HELP_ALIAS_SHORT "\"list aliases\"\n"
#define OPH_TERM_HELP_ALIAS_LONG \
"\e[1mUSAGE OF alias COMMAND\e[0m\n\n\
\talias\n\n\
\tList all aliases with their values.\n"
#define OPH_TERM_HELP_ALIAS_LONG2 \
"USAGE OF alias COMMAND\\n\\n\
\\talias\\n\\n\
\\tList all aliases with their values.\\n"

#define OPH_TERM_HELP_SETALIAS_SHORT "\"set or change the value of an alias\"\n"
#define OPH_TERM_HELP_SETALIAS_LONG \
"\e[1mUSAGE OF setalias COMMAND\e[0m\n\n\
\tsetalias ALIAS=\"value\"\n\n\
\tSet the alias ALIAS to \"value\".\n\
\tFor example, <setalias my_alias=\"command1\"> or <setalias my_alias=command1> are valid commands.\n\
\tThe value can be surrounded by double quotes but cannot contain double/single quotes or semicolon.\n\
\tTo insert a special character, just escape it like \"\\t\".\n\
\tSo these commands are correct:\n\
\t<setalias my_alias=command>, <setalias my_alias=\"command\">,\n\
\t<setalias my_alias=command with space>, <setalias my_alias=\"command with space\">...\n\
\tAnd these commands are invalid:\n\
\t<setalias my_alias=\"com\"mand>, <setalias my_alias = \"command\">,\n\
\t<setalias my_alias= command>, <setalias my_alias = command>...\n\
\tUse this command also to create new aliases. Each alias does not have to start with\n\
\t\"oph_\",\"/\" or \".\" and it does not have to be a number or equal to any Oph_Term local command.\n"
#define OPH_TERM_HELP_SETALIAS_LONG2 \
"USAGE OF setalias COMMAND\\n\\n\
\\tsetalias ALIAS=\\\"value\\\"\\n\\n\
\\tSet the alias ALIAS to \\\"value\\\".\\n\
\\tFor example, <setalias my_alias=\\\"command1\\\"> or <setalias my_alias=command1> are valid commands.\\n\
\\tThe value can be surrounded by double quotes but cannot contain double/single quotes or semicolon.\\n\
\\tTo insert a special character, just escape it like \\\"\\\\t\\\".\\n\
\\tSo these commands are correct:\\n\
\\t<setalias my_alias=command>, <setalias my_alias=\\\"command\\\">,\\n\
\\t<setalias my_alias=command with space>, <setalias my_alias=\\\"command with space\\\">...\\n\
\\tAnd these commands are invalid:\\n\
\\t<setalias my_alias=\\\"com\\\"mand>, <setalias my_alias = \\\"command\\\">,\\n\
\\t<setalias my_alias= command>, <setalias my_alias = command>...\\n\
\\tUse this command also to create new aliases. Each alias does not have to start with\\n\
\\t\\\"oph_\\\",\\\"/\\\" or \\\".\\\" and it does not have to be a number or equal to any Oph_Term local command.\\n"

#define OPH_TERM_HELP_UNSETALIAS_SHORT "\"unset an alias\"\n"
#define OPH_TERM_HELP_UNSETALIAS_LONG \
"\e[1mUSAGE OF unsetalias COMMAND\e[0m\n\n\
\tunsetalias ALIAS\n\n\
\tClear the content of the alias ALIAS.\n\
\tALIAS cannot be surrounded by or contain single or double/single quotes,spaces or other characters except letters, numbers and underscore.\n"
#define OPH_TERM_HELP_UNSETALIAS_LONG2 \
"USAGE OF unsetalias COMMAND\\n\\n\
\\tunsetalias ALIAS\\n\\n\
\\tClear the content of the alias ALIAS.\\n\
\\tALIAS cannot be surrounded by or contain single or double/single quotes,spaces or other characters except letters, numbers and underscore.\\n"

#define OPH_TERM_HELP_GETALIAS_SHORT "\"print the value of an alias\"\n"
#define OPH_TERM_HELP_GETALIAS_LONG \
"\e[1mUSAGE OF getalias COMMAND\e[0m\n\n\
\tgetalias ALIAS\n\n\
\tPrint the value of the alias ALIAS (if it exists).\n\
\tFor example, to view the current value of an alias called \"my_alias\" just type <getalias my_alias>.\n\
\tALIAS cannot be surrounded by or contain single or double/single quotes,spaces or other characters except letters, numbers and underscore.\n"
#define OPH_TERM_HELP_GETALIAS_LONG2 \
"USAGE OF getalias COMMAND\\n\\n\
\\tgetalias ALIAS\\n\\n\
\\tPrint the value of the alias ALIAS (if it exists).\\n\
\\tFor example, to view the current value of an alias called \\\"my_alias\\\" just type <getalias my_alias>.\\n\
\\tALIAS cannot be surrounded by or contain single or double/single quotes,spaces or other characters except letters, numbers and underscore.\\n"

#ifdef WITH_IM_SUPPORT
#define OPH_TERM_HELP_DEPLOY_SHORT "\"deploy an Ophidia cluster with IM\"\n"
#define OPH_TERM_HELP_DEPLOY_LONG \
"\e[1mUSAGE OF deploy COMMAND\e[0m\n\n\
\tdeploy /path/to/file.radl\n\n\
\tDeploy an Ophidia cluster with IM using the configuration specified in the RADL file.\n\
\tOPH_AUTH_HEADER and OPH_INFRASTRUCTURE_URL must be set before launching this command.\n\
\tCorrect values for these variables, as well as for OPH_USER and OPH_PASSWD, should have been provided by the IM admin.\n\
\tCurrent implementation expects the Ophidia server to be the first node in the configuration, listening on port 11732.\n\
\tThe output will follow the format <The new infrastructure URL is now \"http://hostname:port/infrastructures/id\".>.\n"
#define OPH_TERM_HELP_DEPLOY_LONG2 \
"USAGE OF deploy COMMAND\\n\\n\
\\tdeploy /path/to/file.radl\\n\\n\
\\tDeploy an Ophidia cluster with IM using the configuration specified in the RADL file.\\n\
\\tOPH_AUTH_HEADER and OPH_INFRASTRUCTURE_URL must be set before launching this command.\\n\
\\tCorrect values for these variables, as well as for OPH_USER and OPH_PASSWD, should have been provided by the IM admin.\\n\
\\tCurrent implementation expects the Ophidia server to be the first node in the configuration, listening on port 11732.\\n\
\\tThe output will follow the format <The new infrastructure URL is now \\\"http://hostname:port/infrastructures/id\\\".>.\\n"

#define OPH_TERM_HELP_DEPLOY_STATUS_SHORT "\"check the status of an Ophidia deploy\"\n"
#define OPH_TERM_HELP_DEPLOY_STATUS_LONG \
"\e[1mUSAGE OF deploy_status COMMAND\e[0m\n\n\
\tdeploy_status\n\n\
\tCheck the status of an Ophidia deploy.\n\
\tOPH_AUTH_HEADER and OPH_INFRASTRUCTURE_URL must be set before launching this command.\n\
\tOPH_INFRASTRUCTURE_URL must follow the URL format as returned by the deploy command.\n\
\tThe output will follow the format <Installation and configuration STATUS.>,\n\
\twith STATUS set to RUNNING, FINISHED, ERROR or EMPTY.\n"
#define OPH_TERM_HELP_DEPLOY_STATUS_LONG2 \
"USAGE OF deploy_status COMMAND\\n\\n\
\\tdeploy_status\\n\\n\
\\tCheck the status of an Ophidia deploy.\\n\
\\tOPH_AUTH_HEADER and OPH_INFRASTRUCTURE_URL must be set before launching this command.\\n\
\\tOPH_INFRASTRUCTURE_URL must follow the URL format as returned by the deploy command.\\n\
\\tThe output will follow the format <Installation and configuration STATUS.>,\\n\
\\twith STATUS set to RUNNING, FINISHED, ERROR or EMPTY.\\n"

#define OPH_TERM_HELP_GET_SERVER_SHORT "\"retrieve the server address from an Ophidia deploy\"\n"
#define OPH_TERM_HELP_GET_SERVER_LONG \
"\e[1mUSAGE OF get_server COMMAND\e[0m\n\n\
\tget_server\n\n\
\tRetrieve the server address from an Ophidia deploy.\n\
\tOPH_AUTH_HEADER and OPH_INFRASTRUCTURE_URL must be set before launching this command.\n\
\tOPH_INFRASTRUCTURE_URL must follow the URL format as returned by the deploy command.\n\
\tThe output will follow the format <The new server address is now \"server_address\".>,\n\
\tand the OPH_SERVER_HOST variable will be set accordingly.\n"
#define OPH_TERM_HELP_GET_SERVER_LONG2 \
"USAGE OF get_server COMMAND\\n\\n\
\\tget_server\\n\\n\
\\tRetrieve the server address from an Ophidia deploy.\\n\
\\tOPH_AUTH_HEADER and OPH_INFRASTRUCTURE_URL must be set before launching this command.\\n\
\\tOPH_INFRASTRUCTURE_URL must follow the URL format as returned by the deploy command.\\n\
\\tThe output will follow the format <The new server address is now \\\"server_address\\\".>,\\n\
\\tand the OPH_SERVER_HOST variable will be set accordingly.\\n"

#define OPH_TERM_HELP_UNDEPLOY_SHORT "\"undeploy an Ophidia cluster\"\n"
#define OPH_TERM_HELP_UNDEPLOY_LONG \
"\e[1mUSAGE OF undeploy COMMAND\e[0m\n\n\
\tundeploy\n\n\
\tUndeploy an Ophidia cluster.\n\
\tOPH_AUTH_HEADER and OPH_INFRASTRUCTURE_URL must be set before launching this command.\n\
\tOPH_INFRASTRUCTURE_URL must follow the URL format as returned by the deploy command.\n"
#define OPH_TERM_HELP_UNDEPLOY_LONG2 \
"USAGE OF undeploy COMMAND\\n\\n\
\\tundeploy\\n\\n\
\\tUndeploy an Ophidia cluster.\\n\
\\tOPH_AUTH_HEADER and OPH_INFRASTRUCTURE_URL must be set before launching this command.\\n\
\\tOPH_INFRASTRUCTURE_URL must follow the URL format as returned by the deploy command.\\n"

#define OPH_TERM_HELP_DEPLOYS_LIST_SHORT "\"list all Ophidia cluster instances\"\n"
#define OPH_TERM_HELP_DEPLOYS_LIST_LONG \
"\e[1mUSAGE OF deploys_list COMMAND\e[0m\n\n\
\tdeploys_list\n\n\
\tList all Ophidia cluster instances.\n\
\tOPH_AUTH_HEADER and OPH_INFRASTRUCTURE_URL must be set before launching this command.\n\
\tOPH_INFRASTRUCTURE_URL must follow the URL format as returned by the deploy command, with or without the infrastructure id.\n"
#define OPH_TERM_HELP_DEPLOYS_LIST_LONG2 \
"USAGE OF deploys_list COMMAND\\n\\n\
\\tdeploys_list\\n\\n\
\\tList all Ophidia cluster instances.\\n\
\\tOPH_AUTH_HEADER and OPH_INFRASTRUCTURE_URL must be set before launching this command.\\n\
\\tOPH_INFRASTRUCTURE_URL must follow the URL format as returned by the deploy command, with or without the infrastructure id.\\n"

#define OPH_TERM_HELP_DEPLOY_VMS_LIST_SHORT "\"list all VMs for an Ophidia cluster instance\"\n"
#define OPH_TERM_HELP_DEPLOY_VMS_LIST_LONG \
"\e[1mUSAGE OF deploy_vms_list COMMAND\e[0m\n\n\
\tdeploy_vms_list\n\n\
\tList all VMs for an Ophidia cluster instance.\n\
\tOPH_AUTH_HEADER and OPH_INFRASTRUCTURE_URL must be set before launching this command.\n\
\tOPH_INFRASTRUCTURE_URL must follow the URL format as returned by the deploy command.\n"
#define OPH_TERM_HELP_DEPLOY_VMS_LIST_LONG2 \
"USAGE OF deploy_vms_list COMMAND\\n\\n\
\\tdeploy_vms_list\\n\\n\
\\tList all VMs for an Ophidia cluster instance.\\n\
\\tOPH_AUTH_HEADER and OPH_INFRASTRUCTURE_URL must be set before launching this command.\\n\
\\tOPH_INFRASTRUCTURE_URL must follow the URL format as returned by the deploy command.\\n"
#endif

#define OPH_TERM_HELP_REMOTE_COMMAND \
"\e[1mHOW TO SUBMIT A REMOTE COMMAND\e[0m\n\n\
\toph_operator ...\n\n\
\tFollow these instructions to send a request to the Ophidia server.\n\
\tThe first part is the lowercase name of the Ophidia operator, such as \"oph_list\".\n\
\tThe ellipsis instead is a string that has to adhere to the following format:\n\
\t  param1=val1;param2=val2;...paramN=valN;\n\
\twhere \"paramX\" sequence is the sequence of arguments needed by the operator.\n\
\tFor a complete description of the correct use with mandatory and optional arguments for each operator, please use the command\n\
\t<oph_man function=name;exec_mode=sync;>.\n\
\tThe operator name must be used as an Oph_Term command and not as a parameter.\n\
\tThe other parameters can follow any order.\n\
\tIn case of optional arguments, they can be omitted and then their default values are used.\n\
\tSpecial arguments are:\n\
\t  \"exec_mode\": it specifies if we want synchronous mode (\"sync\") or asynchronous mode (\"async\") which is the server default;\n\
\t  \"ncores\": it specifies the number of parallel processes requested for the execution of the operator (default is 1);\n\
\t  \"sessionid\": it specifies the current session;\n\
\t  \"cwd\": it specifies the current working directory;\n\
\t  \"cdd\": it specifies the pathname working directory on data reporitory;\n\
\t  \"cube\": it specifies the input datacube.\n\
\tThey are special arguments that the user can explicitly write into the submission string or not,\n\
\tin which case Oph_Term will look up and use the content of the variables OPH_SESSION_ID, OPH_EXEC_MODE, OPH_NCORES, OPH_CWD, OPH_CDD or OPH_DATACUBE if existent.\n\
\tIn synchronous mode the call is blocking and waits the server response, while in asynchronous mode it is non-blocking and returns just a link\n\
\tto the file where output will be written by the operator for future analysis.\n"
#define OPH_TERM_HELP_REMOTE_COMMAND2 \
"HOW TO SUBMIT A REMOTE COMMAND\\n\\n\
\\toph_operator ...\\n\\n\
\\tFollow these instructions to send a request to the Ophidia server.\\n\
\\tThe first part is the lowercase name of the Ophidia operator, such as \\\"oph_list\\\".\\n\
\\tThe ellipsis instead is a string that has to adhere to the following format:\\n\
\\t  param1=val1;param2=val2;...paramN=valN;\\n\
\\twhere \\\"paramX\\\" sequence is the sequence of arguments needed by the operator.\\n\
\\tFor a complete description of the correct use with mandatory and optional arguments for each operator, please use the command\\n\
\\t<oph_man function=name;exec_mode=sync;>.\\n\
\\tThe operator name must be used as an Oph_Term command and not as a parameter.\\n\
\\tThe other parameters can follow any order.\\n\
\\tIn case of optional arguments, they can be omitted and then their default values are used.\\n\
\\tSpecial arguments are:\\n\
\\t  \\\"exec_mode\\\": it specifies if we want synchronous mode (\\\"sync\\\") or asynchronous mode (\\\"async\\\") which is the server default;\\n\
\\t  \\\"ncores\\\": it specifies the number of parallel processes requested for the execution of the operator (default is 1);\\n\
\\t  \\\"sessionid\\\": it specifies the current session;\\n\
\\t  \\\"cwd\\\": it specifies the current working directory;\\n\
\\t  \\\"cdd\\\": it specifies the pathname working directory on data reporitory;\\n\
\\t  \\\"cube\\\": it specifies the input datacube.\\n\
\\tThey are special arguments that the user can explicitly write into the submission string or not,\\n\
\\tin which case Oph_Term will look up and use the content of the variables OPH_SESSION_ID, OPH_EXEC_MODE, OPH_NCORES, OPH_CWD, OPH_CCD or OPH_DATACUBE if existent.\\n\
\\tIn synchronous mode the call is blocking and waits the server response, while in asynchronous mode it is non-blocking and returns just a link\\n\
\\tto the file where output will be written by the operator for future analysis.\\n"

#ifndef NO_WORKFLOW
#define OPH_TERM_HELP_REMOTE_WORKFLOW \
"\e[1mHOW TO SUBMIT AN ENTIRE WORKFLOW\e[0m\n\n\
\t./workflow.json [param1 [param2 [...]]]\n\n\
\tSubmit an entire workflow to the Ophidia server.\n\
\tOPH_SESSION_ID,OPH_EXEC_MODE,OPH_NCORES,OPH_CWD,OPH_CDD and OPH_DATACUBE will be inserted in\n\
\tthe JSON request (original file will remain untouched) if not already present.\n\
\t\"./workflow.json\" must be the name of a valid local JSON file according to the Ophidia Workflow JSON Schema.\n\
\tWith one or more parameters, the parameter substitution ($1,$2 etc.) is performed before the validation phase\n\
\ttogether with variable substitution.\n\
\tAn invalid workflow will not be submitted.\n"
#define OPH_TERM_HELP_REMOTE_WORKFLOW2 \
"HOW TO SUBMIT AN ENTIRE WORKFLOW\\n\\n\
\\t./workflow.json [param1 [param2 [...]]]\\n\\n\
\\tSubmit an entire workflow to the Ophidia server.\\n\
\\tOPH_SESSION_ID,OPH_EXEC_MODE,OPH_NCORES,OPH_CWD,OPH_CDD and OPH_DATACUBE will be inserted in\\n\
\\tthe JSON request (original file will remain untouched) if not already present.\\n\
\\t\\\"./workflow.json\\\" must be the name of a valid local JSON file according to the Ophidia Workflow JSON Schema.\\n\
\\tWith one or more parameters, the parameter substitution ($1,$2 etc.) is performed before the validation phase\\n\
\\ttogether with variable substitution.\\n\
\\tAn invalid workflow will not be submitted.\\n"
#endif

#define OPH_TERM_HELP_AUTOCOMPLETION \
"\e[1mHOW TO USE OPH_TERM AUTOCOMPLETION FEATURE\e[0m\n\n\
\t1) [OPH_TERM] >>[a-zA-Z]<TAB>\n\
\t\tPerform autocompletion over Oph_Term specific commands (local commands),\n\
\t\tenvironment variables (pre-defined and user-defined) and aliases.\n\
\t\tIn case of \"o\" as first character autocompletion will return the prefix \"oph_\".\n\
\t2) [OPH_TERM] >>$[a-zA-Z]<TAB>\n\
\t\tPerform autocompletion over Oph_Term environment variables (pre-defined and user-defined).\n\
\t\tThe same kind of autocompletion occurs even if there is the following pattern:\n\
\t\t[OPH_TERM] >>${[a-zA-Z]<TAB>\n\
\t\tIn this case a match is completed with the symbol \"}\".\n\
\t3) [OPH_TERM] >>[./]<TAB>\n\
\t\tWith a full stop or a slash perform autocompletion over local filesystem.\n\
\t4) [OPH_TERM] >>oph_<TAB>\n\
\t\tWith the \"oph_\" prefix perform autocompletion over Ophidia operators (remote commands).\n\
\t5) [OPH_TERM] >>oph_<operator> [a-z]<TAB>\n\
\t\tIn case of an operator specified at the beginning of the line perform autocompletion over its arguments.\n\
\t\tIn case of 1 possible match the line is automatically updated with the symbol \"=\".\n\
\t\tIn case of more than 1 possible matches they are all printed with useful info:\n\
\t\t\t**: mandatory argument;\n\
\t\t\t(x): default value for optional argument;\n\
\t\t\t[x|y|...]: list of admitted values for an argument.\n\
\t\tThe same kind of autocompletion occurs even if there is the following pattern:\n\
\t\t[OPH_TERM] >>oph_<operator> arg1=val1;[a-z]<TAB>\n\
\t6) [OPH_TERM] >>oph_<operator> arg=[a-zA-Z0-9_]<TAB>\n\
\t\tIn this case perform autocompletion over the default value or all possible values of an operator argument.\n\
\t\tIn case of 1 possible match the line is automatically updated with the symbol \";\".\n\
\t\tIn case of more than 1 possible matches they are all printed with default values between parenthesis.\n\
\t\tThe same kind of autocompletion occurs even if there is the following pattern:\n\
\t\t[OPH_TERM] >>oph_<operator> arg1=val1;arg2=[a-zA-Z0-9_]<TAB>\n\
\t7) ...<Shift><TAB>\n\
\t\tInstead of the \"classic\" autocompletion, perform a \"menu\" autocompletion,\n\
\t\tcycling over possible matches directly inline.\n\
\t\tWith this form of autocompletion additional characters are never appended.\n"
#define OPH_TERM_HELP_AUTOCOMPLETION2 \
"HOW TO USE OPH_TERM AUTOCOMPLETION FEATURE\\n\\n\
\\t1) [OPH_TERM] >>[a-zA-Z]<TAB>\\n\
\\t\\tPerform autocompletion over Oph_Term specific commands (local commands),\\n\
\\t\\tenvironment variables (pre-defined and user-defined) and aliases.\\n\
\\t\\tIn case of \\\"o\\\" as first character autocompletion will return the prefix \\\"oph_\\\".\\n\
\\t2) [OPH_TERM] >>$[a-zA-Z]<TAB>\\n\
\\t\\tPerform autocompletion over Oph_Term environment variables (pre-defined and user-defined).\\n\
\\t\\tThe same kind of autocompletion occurs even if there is the following pattern:\\n\
\\t\\t[OPH_TERM] >>${[a-zA-Z]<TAB>\\n\
\\t\\tIn this case a match is completed with the symbol \\\"}\\\".\\n\
\\t3) [OPH_TERM] >>[./]<TAB>\\n\
\\t\\tWith a full stop or a slash perform autocompletion over local filesystem.\\n\
\\t4) [OPH_TERM] >>oph_<TAB>\\n\
\\t\\tWith the \\\"oph_\\\" prefix perform autocompletion over Ophidia operators (remote commands).\\n\
\\t5) [OPH_TERM] >>oph_<operator> [a-z]<TAB>\\n\
\\t\\tIn case of an operator specified at the beginning of the line perform autocompletion over its arguments.\\n\
\\t\\tIn case of 1 possible match the line is automatically updated with the symbol \\\"=\\\".\\n\
\\t\\tIn case of more than 1 possible matches they are all printed with useful info:\\n\
\\t\\t\\t**: mandatory argument;\\n\
\\t\\t\\t(x): default value for optional argument;\\n\
\\t\\t\\t[x|y|...]: list of admitted values for an argument.\\n\
\\t\\tThe same kind of autocompletion occurs even if there is the following pattern:\\n\
\\t\\t[OPH_TERM] >>oph_<operator> arg1=val1;[a-z]<TAB>\\n\
\\t6) [OPH_TERM] >>oph_<operator> arg=[a-zA-Z0-9_]<TAB>\\n\
\\t\\tIn this case perform autocompletion over the default value or all possible values of an operator argument.\\n\
\\t\\tIn case of 1 possible match the line is automatically updated with the symbol \\\";\\\".\\n\
\\t\\tIn case of more than 1 possible matches they are all printed with default values between parenthesis.\\n\
\\t\\tThe same kind of autocompletion occurs even if there is the following pattern:\\n\
\\t\\t[OPH_TERM] >>oph_<operator> arg1=val1;arg2=[a-zA-Z0-9_]<TAB>\\n\
\\t7) ...<Shift><TAB>\\n\
\\t\\tInstead of the \\\"classic\\\" autocompletion, perform a \\\"menu\\\" autocompletion,\\n\
\\t\\tcycling over possible matches directly inline.\\n\
\\t\\tWith this form of autocompletion additional characters are never appended.\\n"

#ifndef NO_WORKFLOW
#define OPH_TERM_HELP_VARIABLE_SUBSTITUTION \
"\e[1mHOW TO USE THE VARIABLE SUBSTITUTION\e[0m\n\n\
\t$varname or ${varname}\n\n\
\tWith the commands resume, view and check and with workflows or remote operators Oph_Term will try to\n\
\tsubstitute the value of <varname> if present or the empty string \"\" into the submission string.\n\
\tWithout braces the first character that is not a letter, a number or an underscore will be used as end of the variable name (excluded).\n\
\tVariable substitution is recursive until all $s are expanded, so if the value of a variable is another \"$varname\",\n\
\tOph_Term will recursively substitute the other value.\n\
\tWith all the other commands there is no variable substitution, so that it is possible to do for example\n\
\t<setenv var1=$var2> to dynamically bind a variable to another one and always use the most recent value of var2.\n"
#define OPH_TERM_HELP_VARIABLE_SUBSTITUTION2 \
"HOW TO USE THE VARIABLE SUBSTITUTION\\n\\n\
\\t$varname or ${varname}\\n\\n\
\\tWith the commands resume, view and check and with workflows or remote operators Oph_Term will try to\\n\
\\tsubstitute the value of <varname> if present or the empty string \\\"\\\" into the submission string.\\n\
\\tWithout braces the first character that is not a letter, a number or an underscore will be used as end of the variable name (excluded).\\n\
\\tVariable substitution is recursive until all $s are expanded, so if the value of a variable is another \\\"$varname\\\",\\n\
\\tOph_Term will recursively substitute the other value.\\n\
\\tWith all the other commands there is no variable substitution, so that it is possible to do for example\\n\
\\t<setenv var1=$var2> to dynamically bind a variable to another one and always use the most recent value of var2.\\n"
#else
#define OPH_TERM_HELP_VARIABLE_SUBSTITUTION \
"\e[1mHOW TO USE THE VARIABLE SUBSTITUTION\e[0m\n\n\
\t$varname or ${varname}\n\n\
\tWith the commands resume and view and with remote operators Oph_Term will try to\n\
\tsubstitute the value of <varname> if present or the empty string \"\" into the submission string.\n\
\tWithout braces the first character that is not a letter, a number or an underscore will be used as end of the variable name (excluded).\n\
\tVariable substitution is recursive until all $s are expanded, so if the value of a variable is another \"$varname\",\n\
\tOph_Term will recursively substitute the other value.\n\
\tWith all the other commands there is no variable substitution, so that it is possible to do for example\n\
\t<setenv var1=$var2> to dynamically bind a variable to another one and always use the most recent value of var2.\n"
#define OPH_TERM_HELP_VARIABLE_SUBSTITUTION2 \
"HOW TO USE THE VARIABLE SUBSTITUTION\\n\\n\
\\t$varname or ${varname}\\n\\n\
\\tWith the commands resume and view and with remote operators Oph_Term will try to\\n\
\\tsubstitute the value of <varname> if present or the empty string \\\"\\\" into the submission string.\\n\
\\tWithout braces the first character that is not a letter, a number or an underscore will be used as end of the variable name (excluded).\\n\
\\tVariable substitution is recursive until all $s are expanded, so if the value of a variable is another \\\"$varname\\\",\\n\
\\tOph_Term will recursively substitute the other value.\\n\
\\tWith all the other commands there is no variable substitution, so that it is possible to do for example\\n\
\\t<setenv var1=$var2> to dynamically bind a variable to another one and always use the most recent value of var2.\\n"
#endif

#define OPH_TERM_HELP_ALIAS_SUBSTITUTION \
"\e[1mHOW TO USE THE ALIAS SUBSTITUTION\e[0m\n\n\
\tAn alias is a particular command that encapsulates other commands.\n\
\tIt is possible to specify only 1 alias per submission string.\n\
\tOph_Term will look at the first word of the submission string and if it finds it is an alias\n\
\tit will try to substitute specified arguments into the alias definition string where $1,$2 etc. are located.\n\
\tAfter the alias substitution, Oph_Term will recursively perform the variable substitution according to the resulting command.\n"
#define OPH_TERM_HELP_ALIAS_SUBSTITUTION2 \
"HOW TO USE THE ALIAS SUBSTITUTION\\n\\n\
\\tAn alias is a particular command that encapsulates other commands.\\n\
\\tIt is possible to specify only 1 alias per submission string.\\n\
\\tOph_Term will look at the first word of the submission string and if it finds it is an alias\\n\
\\tit will try to substitute specified arguments into the alias definition string where $1,$2 etc. are located.\\n\
\\tAfter the alias substitution, Oph_Term will recursively perform the variable substitution according to the resulting command.\\n"

//ENVIRONMENT VARIABLES HELP
#define OPH_TERM_HELP_OPH_TERM_PS1_SHORT "\"color of the prompt\"\n"
#define OPH_TERM_HELP_OPH_TERM_PS1_LONG \
"\e[1mOPH_TERM_PS1\e[0m\n\
\tOph_Term prompt.Default is \"[OPH_TERM] >> \" red coloured.\n\
\tIt can be set to: \"black\",\"red\",\"green\",\"yellow\",\"blue\",\"purple\",\"cyan\" or \"white\".\n\
\tWhen possible Oph_Term prompt will report the first 2 and the last 4 numbers of the code that identifies the current session.\n"
#define OPH_TERM_HELP_OPH_TERM_PS1_LONG2 \
"OPH_TERM_PS1\\n\
\\tOph_Term prompt.Default is \\\"[OPH_TERM] >> \\\" red coloured.\\n\
\\tIt can be set to: \\\"black\\\",\\\"red\\\",\\\"green\\\",\\\"yellow\\\",\\\"blue\\\",\\\"purple\\\",\\\"cyan\\\" or \\\"white\\\".\\n\
\\tWhen possible Oph_Term prompt will report the first 2 and the last 4 numbers of the code that identifies the current session.\\n"

#define OPH_TERM_HELP_OPH_CWD_SHORT "\"current working directory\"\n"
#define OPH_TERM_HELP_OPH_CWD_LONG \
"\e[1mOPH_CWD\e[0m\n\
\tCurrent working directory. It defaults to the session's root folder (/).\n"
#define OPH_TERM_HELP_OPH_CWD_LONG2 \
"OPH_CWD\\n\
\\tCurrent working directory. It defaults to the session's root folder (/).\\n"

#define OPH_TERM_HELP_OPH_CCD_SHORT "\"current data directory\"\n"
#define OPH_TERM_HELP_OPH_CCD_LONG \
"\e[1mOPH_CCD\e[0m\n\
\tCurrent data directory. It defaults to root folder (/).\n"
#define OPH_TERM_HELP_OPH_CCD_LONG2 \
"OPH_CCD\\n\
\\tCurrent data directory. It defaults to root folder (/).\\n"

#define OPH_TERM_HELP_OPH_USER_SHORT "\"username\"\n"
#define OPH_TERM_HELP_OPH_USER_LONG \
"\e[1mOPH_USER\e[0m\n\
\tUsername used to connect to the remote server when SSL is used (default). With the GSI interface it is ignored.\n"
#define OPH_TERM_HELP_OPH_USER_LONG2 \
"OPH_USER\\n\
\\tUsername used to connect to the remote server when SSL is used (default). With the GSI interface it is ignored.\\n"

#define OPH_TERM_HELP_OPH_PASSWD_SHORT "\"password\"\n"
#define OPH_TERM_HELP_OPH_PASSWD_LONG \
"\e[1mOPH_PASSWD\e[0m\n\
\tPassword used to connect to the remote server when SSL is used (default). With the GSI interface it is ignored.\n"
#define OPH_TERM_HELP_OPH_PASSWD_LONG2 \
"OPH_PASSWD\\n\
\\tPassword used to connect to the remote server when SSL is used (default). With the GSI interface it is ignored.\\n"

#define OPH_TERM_HELP_OPH_TOKEN_SHORT "\"token\"\n"
#define OPH_TERM_HELP_OPH_TOKEN_LONG \
"\e[1mOPH_TOKEN\e[0m\n\
\tToken used to connect to the remote server by using OpenId. If it is set, username and password will be ignored.\n"
#define OPH_TERM_HELP_OPH_TOKEN_LONG2 \
"OPH_TOKEN\\n\
\\tToken used to connect to the remote server by using OpenId. If it is set, username and password will be ignored.\\n"

#define OPH_TERM_HELP_OPH_SERVER_HOST_SHORT "\"server address\"\n"
#define OPH_TERM_HELP_OPH_SERVER_HOST_LONG \
"\e[1mOPH_SERVER_HOST\e[0m\n\
\tHostname of the Ophidia server.\n"
#define OPH_TERM_HELP_OPH_SERVER_HOST_LONG2 \
"OPH_SERVER_HOST\\n\
\\tHostname of the Ophidia server.\\n"

#define OPH_TERM_HELP_OPH_SERVER_PORT_SHORT "\"server port\"\n"
#define OPH_TERM_HELP_OPH_SERVER_PORT_LONG \
"\e[1mOPH_SERVER_PORT\e[0m\n\
\tPort number of the Ophidia server.\n"
#define OPH_TERM_HELP_OPH_SERVER_PORT_LONG2 \
"OPH_SERVER_PORT\\n\
\\tPort number of the Ophidia server.\\n"

#define OPH_TERM_HELP_OPH_SESSION_ID_SHORT "\"current session\"\n"
#define OPH_TERM_HELP_OPH_SESSION_ID_LONG \
"\e[1mOPH_SESSION_ID\e[0m\n\
\tCurrent ID of the remote session.\n\
\tIn addition to Oph_Term commands, use \"oph_manage_session\" and \"oph_resume\" to better control sessions.\n"
#define OPH_TERM_HELP_OPH_SESSION_ID_LONG2 \
"OPH_SESSION_ID\\n\
\\tCurrent ID of the remote session.\\n\
\\tIn addition to Oph_Term commands, use \\\"oph_manage_session\\\" and \\\"oph_resume\\\" to better control sessions.\\n"

#define OPH_TERM_HELP_OPH_EXEC_MODE_SHORT "\"execution mode\"\n"
#define OPH_TERM_HELP_OPH_EXEC_MODE_LONG \
"\e[1mOPH_EXEC_MODE\e[0m\n\
\tType of execution mode for operators (values=async|sync, Oph_Term default = sync).\n\
\tIn case of \"async\" a job would be executed in asynchronous mode with a link to the output file.\n\
\tIn case of \"sync\" a job would be executed in synchronous mode and the output rendered as soon as available.\n"
#define OPH_TERM_HELP_OPH_EXEC_MODE_LONG2 \
"OPH_EXEC_MODE\\n\
\\tType of execution mode for operators (values=async|sync, Oph_Term default = sync).\\n\
\\tIn case of \\\"async\\\" a job would be executed in asynchronous mode with a link to the output file.\\n\
\\tIn case of \\\"sync\\\" a job would be executed in synchronous mode and the output rendered as soon as available.\\n"

#define OPH_TERM_HELP_OPH_NCORES_SHORT "\"number of cores\"\n"
#define OPH_TERM_HELP_OPH_NCORES_LONG \
"\e[1mOPH_NCORES\e[0m\n\
\tNumber of parallel processes needed by a job (min=1,default=1).\n"
#define OPH_TERM_HELP_OPH_NCORES_LONG2 \
"OPH_NCORES\\n\
\\tNumber of parallel processes needed by a job (min=1,default=1).\\n"

#define OPH_TERM_HELP_OPH_DATACUBE_SHORT "\"current datacube\"\n"
#define OPH_TERM_HELP_OPH_DATACUBE_LONG \
"\e[1mOPH_DATACUBE\e[0m\n\
\tLast produced datacube (PID) used as input datacube for an Ophidia operator.\n"
#define OPH_TERM_HELP_OPH_DATACUBE_LONG2 \
"OPH_DATACUBE\\n\
\\tLast produced datacube (PID) used as input datacube for an Ophidia operator.\\n"

#define OPH_TERM_HELP_OPH_TERM_VIEWER_SHORT "\"output renderer\"\n"
#define OPH_TERM_HELP_OPH_TERM_VIEWER_LONG \
"\e[1mOPH_TERM_VIEWER\e[0m\n\
\tThe Oph_Term component used for output rendering.\n\
\tPossible values are:\n\
\t\t- dump : print received output as is;\n\
\t\t- basic : pretty print output in tabular format (default behaviour);\n\
\t\t- coloured : the same as basic but with colors (same color as prompt);\n\
\t\t- extended : the same as basic with other information regarding data sources, producers, consumers etc.;\n\
\t\t- extended_coloured : the same as extended but with colors (same color as prompt).\n"
#define OPH_TERM_HELP_OPH_TERM_VIEWER_LONG2 \
"OPH_TERM_VIEWER\\n\
\\tThe Oph_Term component used for output rendering.\\n\
\\tPossible values are:\\n\
\\t\\t- dump : print received output as is;\\n\
\\t\\t- basic : pretty print output in tabular format (default behaviour);\\n\
\\t\\t- coloured : the same as basic but with colors (same color as prompt);\\n\
\\t\\t- extended : the same as basic with other information regarding data sources, producers, consumers etc.;\\n\
\\t\\t- extended_coloured : the same as extended but with colors (same color as prompt).\\n"

#define OPH_TERM_HELP_OPH_TERM_FORMAT_SHORT "\"output format of command 'view'\"\n"
#define OPH_TERM_HELP_OPH_TERM_FORMAT_LONG \
"\e[1mOPH_TERM_FORMAT\e[0m\n\
\tOutput format of the command 'view'.\n\
\tPossible values are:\n\
\t\t- classic : print all the data in JSON Response;\n\
\t\t- compact : print the JSON Response without the Workflow Task List.\n"
#define OPH_TERM_HELP_OPH_TERM_FORMAT_LONG2 \
"OPH_TERM_FORMAT\\n\
\\tOutput format of the command 'view'.\\n\
\\tPossible values are:\\n\
\\t\\t- classic : print all the data in JSON Response;\\n\
\\t\\t- compact : print the JSON Response without the Workflow Task List.\\n"

#ifndef NO_WORKFLOW
#define OPH_TERM_HELP_OPH_TERM_IMGS_SHORT "\"save and/or auto-open images\"\n"
#define OPH_TERM_HELP_OPH_TERM_IMGS_LONG \
"\e[1mOPH_TERM_IMGS\e[0m\n\
\tIn case of non-dump viewer or the commands \"check\" and \"view\", with \"save\" eventually save image files when possible (as for trees/graphs outputs)\n\
\tinto the same directory from which the user launched Oph_Term.\n\
\tWith \"open\" automatically open image files in a separate window and save them to disk.\n\
\t\"no_op\" is the default value for not saving nor opening images.\n"
#define OPH_TERM_HELP_OPH_TERM_IMGS_LONG2 \
"OPH_TERM_IMGS\\n\
\\tIn case of non-dump viewer or the commands \\\"check\\\" and \\\"view\\\", with \\\"save\\\" eventually save image files when possible (as for trees/graphs outputs)\\n\
\\tinto the same directory from which the user launched Oph_Term.\\n\
\\tWith \\\"open\\\" automatically open image files in a separate window and save them to disk.\\n\
\\t\\\"no_op\\\" is the default value for not saving nor opening images.\\n"
#else
#define OPH_TERM_HELP_OPH_TERM_IMGS_SHORT "\"save and/or auto-open images\"\n"
#define OPH_TERM_HELP_OPH_TERM_IMGS_LONG \
"\e[1mOPH_TERM_IMGS\e[0m\n\
\tIn case of non-dump viewer, with \"save\" eventually save image files when possible (as for trees/graphs outputs)\n\
\tinto the same directory from which the user launched Oph_Term.\n\
\tWith \"open\" automatically open image files in a separate window and save them to disk.\n\
\t\"no_op\" is the default value for not saving nor opening images.\n"
#define OPH_TERM_HELP_OPH_TERM_IMGS_LONG2 \
"OPH_TERM_IMGS\\n\
\\tIn case of non-dump viewer, with \\\"save\\\" eventually save image files when possible (as for trees/graphs outputs)\\n\
\\tinto the same directory from which the user launched Oph_Term.\\n\
\\tWith \\\"open\\\" automatically open image files in a separate window and save them to disk.\\n\
\\t\\\"no_op\\\" is the default value for not saving nor opening images.\\n"
#endif

#define OPH_TERM_HELP_OPH_GRAPH_LAYOUT_SHORT "\"layout for graphs\"\n"
#define OPH_TERM_HELP_OPH_GRAPH_LAYOUT_LONG \
"\e[1mOPH_GRAPH_LAYOUT\e[0m\n\
\tIn case of a command producing an image file representing a graph, it is possible\n\
\tto set the graph layout to one of the following values:\n\
\t\t- horizontal_compact : nodes are arranged from left to right;\n\
\t\t- vertical_compact : nodes are arranged from top to bottom;\n\
\t\t- horizontal_ranked : nodes are arranged from left to right and according to their rank/depth;\n\
\t\t- vertical_ranked : nodes are arranged from top to bottom and according to their rank/depth.\n"
#define OPH_TERM_HELP_OPH_GRAPH_LAYOUT_LONG2 \
"OPH_GRAPH_LAYOUT\\n\
\\tIn case of a command producing an image file representing a graph, it is possible\\n\
\\tto set the graph layout to one of the following values:\\n\
\\t\\t- horizontal_compact : nodes are arranged from left to right;\\n\
\\t\\t- vertical_compact : nodes are arranged from top to bottom (default);\\n\
\\t\\t- horizontal_ranked : nodes are arranged from left to right and according to their rank/depth;\\n\
\\t\\t- vertical_ranked : nodes are arranged from top to bottom and according to their rank/depth.\\n"

#define OPH_TERM_HELP_OPH_RESPONSE_BUFFER_SHORT "\"response max size\"\n"
#define OPH_TERM_HELP_OPH_RESPONSE_BUFFER_LONG \
"\e[1mOPH_RESPONSE_BUFFER\e[0m\n\
\tIt respresents the maximum size (in kB) of a response in order to be rendered by Oph_Term.\n"
#define OPH_TERM_HELP_OPH_RESPONSE_BUFFER_LONG2 \
"OPH_RESPONSE_BUFFER\\n\
\\tIt respresents the maximum size (in kB) of a response in order to be rendered by Oph_Term.\\n"

#define OPH_TERM_HELP_OPH_WORKFLOW_AUTOVIEW_SHORT "\"auto-view status\"\n"
#define OPH_TERM_HELP_OPH_WORKFLOW_AUTOVIEW_LONG \
"\e[1mOPH_WORKFLOW_AUTOVIEW\e[0m\n\
\tIf set to \"on\" Oph_Term will automatically call the command \"view <wid> 0 5\" after a workflow submission\n\
\t(<wid> is the related workflow identifier).\n\
\tThis option enables the workflow to be automatically shown graphically, provided that \e[1mOPH_TERM_IMGS\e[0m is set to \"open\".\n\
\tIn case of synchronous execution mode (\"\e[1mexec_mode\e[0m\" set to \"sync\") only the final representation is shown.\n\
\tOtherwise, in case of asynchronous execution mode (\"\e[1mexec_mode\e[0m\" set to \"async\"), the workflow is shown even\n\
\tduring the execution; then, the image is periodically updated until the final representation.\n\
\tIf set to \"on_X_Y\" Oph_Term will call the command \"view <wid> <X> <Y>\", using\n\
\t\"X\" as the number of iterations and \"Y\" as the time interval [in seconds].\n\
\tThe default value \"off\" prevents this behaviour from happening.\n"
#define OPH_TERM_HELP_OPH_WORKFLOW_AUTOVIEW_LONG2 \
"OPH_WORKFLOW_AUTOVIEW\\n\
\\tIf set to \\\"on\\\" Oph_Term will automatically call the command \\\"view <wid> 0 5\\\" after a workflow submission\\n\
\\t(<wid> is the related workflow identifier).\\n\
\\tThis option enables the workflow to be automatically shown graphically, provided that \\e[1mOPH_TERM_IMGS\\e[0m is set to \\\"open\\\".\\n\
\\tIn case of synchronous execution mode (\\\"\\e[1mexec_mode\\e[0m\\\" set to \\\"sync\\\") only the final representation is shown.\\n\
\\tOtherwise, in case of asynchronous execution mode (\\\"\\e[1mexec_mode\\e[0m\\\" set to \\\"async\\\"), the workflow is shown even\\n\
\\tduring the execution; then, the image is periodically updated until the final representation.\\n\
\\tIf set to \\\"on_X_Y\\\" Oph_Term will call the command \\\"view <wid> <X> <Y>\\\", using\\n\
\\t\\\"X\\\" as the number of iterations and \\\"Y\\\" as the time interval [in seconds].\\n\
\\tThe default value \\\"off\\\" prevents this behaviour from happening.\\n"

#ifdef WITH_IM_SUPPORT
#define OPH_TERM_HELP_OPH_INFRASTRUCTURE_URL_SHORT "\"URL for IM instances\"\n"
#define OPH_TERM_HELP_OPH_INFRASTRUCTURE_URL_LONG \
"\e[1mOPH_INFRASTRUCTURE_URL\e[0m\n\
\tURL for IM instances with the format <http://hostname:port/infrastructures>.\n"
#define OPH_TERM_HELP_OPH_INFRASTRUCTURE_URL_LONG2 \
"OPH_INFRASTRUCTURE_URL\\n\
\\tURL for IM instances with the format <http://hostname:port/infrastructures>.\\n"

#define OPH_TERM_HELP_OPH_AUTH_HEADER_SHORT "\"Authentication header for IM\"\n"
#define OPH_TERM_HELP_OPH_AUTH_HEADER_LONG \
"\e[1mOPH_AUTH_HEADER\e[0m\n\
\tAuthentication header for IM with the format <\"Authorization: id=...;type=...;host=...;username=...;password=...\\ntype=...;username=...;password=...\">.\n"
#define OPH_TERM_HELP_OPH_AUTH_HEADER_LONG2 \
"OPH_AUTH_HEADER\\n\
\\tAuthentication header for IM with the format <\\\"Authorization: id=...;type=...;host=...;username=...;password=...\\\\ntype=...;username=...;password=...\\\">.\\n"
#endif

#endif
