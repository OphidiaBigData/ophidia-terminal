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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

/* Jansson header to manipulate JSONs */
#include <jansson.h>

#include "oph_workflow_functions.h"
#include "oph_workflow_define.h"

/* Alloc oph_workflow struct */
int _oph_workflow_alloc(oph_workflow ** workflow);
/* Add key and value to list of arguments for each task not yet comprising that key */
int _oph_workflow_substitute_var(char *key, char *value, oph_workflow_task * tasks, int tasks_num);
/* Add cube to list of arguments for each task with no explicit cube argument and with none or embedded-only deps */
int _oph_workflow_substitute_cube(char *pid, oph_workflow_task * tasks, int tasks_num);

int oph_workflow_load(char *json_string, char *username, oph_workflow ** workflow)
{
	if (!json_string || !username || !workflow) {
		(print_json) ? my_fprintf(stderr, "Error: null input parameters\\n\\n") : fprintf(stderr, "\e[1;31mError: null input parameters\e[0m\n\n");
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}

	*workflow = NULL;

	//alloc and init
	if (_oph_workflow_alloc(workflow)) {
		(print_json) ? my_fprintf(stderr, "Error: workflow allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: workflow allocations\e[0m\n\n");
		return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
	}
	//add username
	(*workflow)->username = (char *) strdup((const char *) username);
	if (!((*workflow)->username)) {
		oph_workflow_free(*workflow);
		(print_json) ? my_fprintf(stderr, "Error: username allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: username allocation\e[0m\n\n");
		return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
	}
	//load json_t from json_string
	json_error_t error;
	json_t *jansson = json_loads((const char *) json_string, 0, &error);
	if (!jansson) {
		oph_workflow_free(*workflow);
		(print_json) ? my_fprintf(stderr, "Error: %s Line: %d Column: %d\\n\\n", error.text, error.line, error.column) : fprintf(stderr, "\e[1;31mError: %s Line: %d Column: %d\e[0m\n\n",
																	 error.text, error.line, error.column);
		return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
	}
	//unpack global vars
	char *name = NULL, *author = NULL, *abstract = NULL, *sessionid = NULL, *exec_mode = NULL, *ncores = NULL, *cwd = NULL, *cdd = NULL, *cube = NULL, *callback_url = NULL, *on_error =
	    NULL, *command = NULL, *on_exit = NULL, *run = NULL, *output_format = NULL, *host_partition = NULL, *url = NULL;
	json_unpack(jansson, "{s?s,s?s,s?s,s?s,s?s,s?s,s?s,s?s,s?s,s?s,s?s,s?s,s?s,s?s,s?s,s?s,s?s}", "name", &name, "author", &author, "abstract", &abstract, "sessionid", &sessionid, "exec_mode",
		    &exec_mode, "ncores", &ncores, "cwd", &cwd, "cdd", &cdd, "cube", &cube, "callback_url", &callback_url, "on_error", &on_error, "command", &command, "on_exit", &on_exit, "run", &run,
		    "output_format", &output_format, "host_partition", &host_partition, "url", &url);

	//add global vars
	if (!name || !author || !abstract) {
		oph_workflow_free(*workflow);
		if (jansson)
			json_decref(jansson);
		(print_json) ? my_fprintf(stderr, "Error: there is no name or author or abstract\\n\\n") : fprintf(stderr, "\e[1;31mError: there is no name or author or abstract\e[0m\n\n");
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}
	(*workflow)->name = (char *) strdup((const char *) name);
	if (!((*workflow)->name)) {
		oph_workflow_free(*workflow);
		if (jansson)
			json_decref(jansson);
		(print_json) ? my_fprintf(stderr, "Error: name allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: name allocation\e[0m\n\n");
		return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
	}
	(*workflow)->author = (char *) strdup((const char *) author);
	if (!((*workflow)->author)) {
		oph_workflow_free(*workflow);
		if (jansson)
			json_decref(jansson);
		(print_json) ? my_fprintf(stderr, "Error: author allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: author allocation\e[0m\n\n");
		return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
	}
	(*workflow)->abstract = (char *) strdup((const char *) abstract);
	if (!((*workflow)->abstract)) {
		oph_workflow_free(*workflow);
		if (jansson)
			json_decref(jansson);
		(print_json) ? my_fprintf(stderr, "Error: abstract allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: abstract allocation\e[0m\n\n");
		return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
	}
	if (url && strlen(url)) {
		(*workflow)->url = (char *) strdup((const char *) url);
		if (!((*workflow)->url)) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: url allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: url allocation\e[0m\n\n");
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
	}
	if (sessionid && strlen(sessionid)) {
		(*workflow)->sessionid = (char *) strdup((const char *) sessionid);
		if (!((*workflow)->sessionid)) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: sessionid allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: sessionid allocation\e[0m\n\n");
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
	}
	if (exec_mode && strlen(exec_mode)) {
		(*workflow)->exec_mode = (char *) strdup((const char *) exec_mode);
		if (!((*workflow)->exec_mode)) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: exec_mode allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: exec_mode allocation\e[0m\n\n");
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
	}
	if (ncores) {
		(*workflow)->ncores = (int) strtol((const char *) ncores, NULL, 10);
	}
	if (cwd && strlen(cwd)) {
		(*workflow)->cwd = (char *) strdup((const char *) cwd);
		if (!((*workflow)->cwd)) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: cwd allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: cwd allocation\e[0m\n\n");
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
	}
	if (cdd && strlen(cdd)) {
		(*workflow)->cdd = (char *) strdup((const char *) cdd);
		if (!((*workflow)->cdd)) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: cdd allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: cdd allocation\e[0m\n\n");
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
	}
	if (cube && strlen(cube)) {
		(*workflow)->cube = (char *) strdup((const char *) cube);
		if (!((*workflow)->cube)) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: cube allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: cube allocation\e[0m\n\n");
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
	}
	if (callback_url && strlen(callback_url)) {
		(*workflow)->callback_url = (char *) strdup((const char *) callback_url);
		if (!((*workflow)->callback_url)) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: callback_url allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: callback_url allocation\e[0m\n\n");
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
	}
	if (command && strlen(command)) {
		(*workflow)->command = (char *) strdup((const char *) command);
		if (!((*workflow)->command)) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: command allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: command allocation\e[0m\n\n");
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
	}
	if (on_error && strlen(on_error)) {
		(*workflow)->on_error = (char *) strdup((const char *) on_error);
		if (!((*workflow)->on_error)) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: on_error allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: on_error allocation\e[0m\n\n");
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
	}
	if (on_exit && strlen(on_exit)) {
		(*workflow)->on_exit = (char *) strdup((const char *) on_exit);
		if (!((*workflow)->on_exit)) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: on_exit allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: on_exit allocation\e[0m\n\n");
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
	}
	if (run && strlen(run)) {
		(*workflow)->run = (char *) strdup((const char *) run);
		if (!((*workflow)->run)) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: run allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: run allocation\e[0m\n\n");
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
	}
	if (output_format && strlen(output_format)) {
		if (!strcmp(output_format, OPH_WORKFLOW_COMPACT))
			(*workflow)->output_format = 1;
		else if (strcmp(output_format, OPH_WORKFLOW_CLASSIC)) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: wrong parameter 'output_format'\\n\\n") : fprintf(stderr, "\e[1;31mError: wrong parameter 'output_format'\e[0m\n\n");
			return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
		}
	}
	if (host_partition && strlen(host_partition)) {
		(*workflow)->host_partition = (char *) strdup((const char *) host_partition);
		if (!((*workflow)->host_partition)) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: host_partition allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: host_partition allocation\e[0m\n\n");
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
	}
	//unpack tasks
	json_t *tasks = NULL;
	json_unpack(jansson, "{s?o}", "tasks", &tasks);
	if (!tasks) {
		oph_workflow_free(*workflow);
		if (jansson)
			json_decref(jansson);
		(print_json) ? my_fprintf(stderr, "Error: there are no tasks\\n\\n") : fprintf(stderr, "\e[1;31mError: there are no tasks\e[0m\n\n");
		return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
	}
	//add tasks
	(*workflow)->tasks_num = (int) json_array_size(tasks);
	(*workflow)->residual_tasks_num = (*workflow)->tasks_num;
	if ((*workflow)->tasks_num < 1) {
		oph_workflow_free(*workflow);
		if (jansson)
			json_decref(jansson);
		(print_json) ? my_fprintf(stderr, "Error: number of tasks must be >=1\\n\\n") : fprintf(stderr, "\e[1;31mError: number of tasks must be >=1\e[0m\n\n");
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}
	(*workflow)->tasks = (oph_workflow_task *) calloc((*workflow)->tasks_num, sizeof(oph_workflow_task));
	if (!((*workflow)->tasks)) {
		(*workflow)->tasks_num = 0;
		(*workflow)->residual_tasks_num = (*workflow)->tasks_num;
		oph_workflow_free(*workflow);
		if (jansson)
			json_decref(jansson);
		(print_json) ? my_fprintf(stderr, "Error: tasks allocation\\n\\n") : fprintf(stderr, "\e[1;31mError: tasks allocation\e[0m\n\n");
		return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
	}
	json_t *task = NULL;
	int i, j;
	for (i = 0; i < (*workflow)->tasks_num; i++) {
		task = json_array_get(tasks, i);
		if (!task) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: task %d not found\\n\\n", i) : fprintf(stderr, "\e[1;31mError: task %d not found\e[0m\n\n", i);
			return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
		}
		//unpack name and operator
		char *name = NULL, *operator= NULL, *on_error_task = NULL, *on_exit_task = NULL, *run_task = NULL;
		json_unpack(task, "{s?s,s?s,s?s,s?s,s?s}", "name", &name, "operator", &operator, "on_error", &on_error_task, "on_exit", &on_exit_task, "run", &run_task);

		//add name and operator
		if (!name || !operator) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: task %d has no name or operator\\n\\n", i) : fprintf(stderr, "\e[1;31mError: task %d has no name or operator\e[0m\n\n", i);
			return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
		}
		(*workflow)->tasks[i].name = (char *) strdup((const char *) name);
		if (!((*workflow)->tasks[i].name)) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: task %d name allocation\\n\\n", i) : fprintf(stderr, "\e[1;31mError: task %d name allocation\e[0m\n\n", i);
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
		(*workflow)->tasks[i].operator =(char *) strdup((const char *) operator);
		if (!((*workflow)->tasks[i].operator)) {
			oph_workflow_free(*workflow);
			if (jansson)
				json_decref(jansson);
			(print_json) ? my_fprintf(stderr, "Error: task %d operator allocation\\n\\n", i) : fprintf(stderr, "\e[1;31mError: task %d operator allocation\e[0m\n\n", i);
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
		//unpack arguments
		json_t *arguments = NULL;
		json_unpack(task, "{s?o}", "arguments", &arguments);
		if (arguments) {
			(*workflow)->tasks[i].arguments_num = (int) json_array_size(arguments);
			if ((*workflow)->tasks[i].arguments_num >= 1) {
				(*workflow)->tasks[i].arguments_keys = (char **) calloc((*workflow)->tasks[i].arguments_num, sizeof(char *));
				if (!((*workflow)->tasks[i].arguments_keys)) {
					(*workflow)->tasks[i].arguments_num = 0;
					oph_workflow_free(*workflow);
					if (jansson)
						json_decref(jansson);
					(print_json) ? my_fprintf(stderr, "Error: task %d arguments keys allocation\\n\\n", i) : fprintf(stderr,
																	 "\e[1;31mError: task %d arguments keys allocation\e[0m\n\n",
																	 i);
					return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
				}
				(*workflow)->tasks[i].arguments_values = (char **) calloc((*workflow)->tasks[i].arguments_num, sizeof(char *));
				if (!((*workflow)->tasks[i].arguments_values)) {
					(*workflow)->tasks[i].arguments_num = 0;
					free((*workflow)->tasks[i].arguments_keys);
					(*workflow)->tasks[i].arguments_keys = NULL;
					oph_workflow_free(*workflow);
					if (jansson)
						json_decref(jansson);
					(print_json) ? my_fprintf(stderr, "Error: task %d arguments values allocation\\n\\n", i) : fprintf(stderr,
																	   "\e[1;31mError: task %d arguments values allocation\e[0m\n\n",
																	   i);
					return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
				}
				for (j = 0; j < (*workflow)->tasks[i].arguments_num; j++) {
					char *argument = NULL;
					json_unpack(json_array_get(arguments, j), "s", &argument);
					if (argument) {
						size_t k;
						int ok = 0;
						for (k = 0; k < strlen(argument); k++) {
							if (argument[k] == '=') {
								(*workflow)->tasks[i].arguments_keys[j] = (char *) strndup(argument, k);
								if (!((*workflow)->tasks[i].arguments_keys[j])) {
									oph_workflow_free(*workflow);
									if (jansson)
										json_decref(jansson);
									(print_json) ? my_fprintf(stderr, "Error: task %d argument key %d allocation\\n\\n", i, j) : fprintf(stderr,
																					     "\e[1;31mError: task %d argument key %d allocation\e[0m\n\n",
																					     i, j);
									return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
								}
								(*workflow)->tasks[i].arguments_values[j] = (char *) strdup(argument + k + 1);
								if (!((*workflow)->tasks[i].arguments_values[j])) {
									oph_workflow_free(*workflow);
									if (jansson)
										json_decref(jansson);
									(print_json) ? my_fprintf(stderr, "Error: task %d argument value %d allocation\\n\\n", i, j) : fprintf(stderr,
																					       "\e[1;31mError: task %d argument value %d allocation\e[0m\n\n",
																					       i, j);
									return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
								}
								ok = 1;
								break;
							}
						}
						if (!ok) {
							oph_workflow_free(*workflow);
							if (jansson)
								json_decref(jansson);
							(print_json) ? my_fprintf(stderr, "Error: task %d argument %d invalid format\\n\\n", i, j) : fprintf(stderr,
																			     "\e[1;31mError: task %d argument %d invalid format\e[0m\n\n",
																			     i, j);
							return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
						}
					}
				}
			}
		}
		//unpack dependencies
		json_t *dependencies = NULL;
		json_unpack(task, "{s?o}", "dependencies", &dependencies);
		if (dependencies) {
			(*workflow)->tasks[i].deps_num = (int) json_array_size(dependencies);
			(*workflow)->tasks[i].residual_deps_num = (*workflow)->tasks[i].deps_num;
			if ((*workflow)->tasks[i].deps_num >= 1) {
				(*workflow)->tasks[i].deps = (oph_workflow_dep *) calloc((*workflow)->tasks[i].deps_num, sizeof(oph_workflow_dep));
				if (!((*workflow)->tasks[i].deps)) {
					(*workflow)->tasks[i].deps_num = 0;
					(*workflow)->tasks[i].residual_deps_num = (*workflow)->tasks[i].deps_num;
					oph_workflow_free(*workflow);
					if (jansson)
						json_decref(jansson);
					(print_json) ? my_fprintf(stderr, "Error: task %d dependencies allocation\\n\\n", i) : fprintf(stderr,
																       "\e[1;31mError: task %d dependencies allocation\e[0m\n\n", i);
					return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
				}
				for (j = 0; j < (*workflow)->tasks[i].deps_num; j++) {
					json_t *dependency = NULL;
					dependency = json_array_get(dependencies, j);
					if (dependency) {
						char *argument = NULL, *order = NULL, *task_name = NULL, *type = NULL, *filter = NULL, *output_argument = NULL, *output_order = NULL;
						json_unpack(dependency, "{s?s,s?s,s?s,s?s,s?s,s?s,s?s}", "argument", &argument, "order", &order, "task", &task_name, "type", &type, "filter", &filter,
							    "output_argument", &output_argument, "output_order", &output_order);

						//add task_name
						if (!task_name) {
							oph_workflow_free(*workflow);
							if (jansson)
								json_decref(jansson);
							(print_json) ? my_fprintf(stderr, "Error: task %d dependency %d has no task name\\n\\n", i, j) : fprintf(stderr,
																				 "\e[1;31mError: task %d dependency %d has no task name\e[0m\n\n",
																				 i, j);
							return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
						}
						(*workflow)->tasks[i].deps[j].task_name = (char *) strdup((const char *) task_name);
						if (!((*workflow)->tasks[i].deps[j].task_name)) {
							oph_workflow_free(*workflow);
							if (jansson)
								json_decref(jansson);
							(print_json) ? my_fprintf(stderr, "Error: task %d dependency %d task name allocation\\n\\n", i, j) : fprintf(stderr,
																				     "\e[1;31mError: task %d dependency %d task name allocation\e[0m\n\n",
																				     i, j);
							return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
						}
						//add argument
						if (argument) {
							(*workflow)->tasks[i].deps[j].argument = (char *) strdup((const char *) argument);
						} else {
							(*workflow)->tasks[i].deps[j].argument = (char *) strdup((const char *) "cube");
						}
						if (!((*workflow)->tasks[i].deps[j].argument)) {
							oph_workflow_free(*workflow);
							if (jansson)
								json_decref(jansson);
							(print_json) ? my_fprintf(stderr, "Error: task %d dependency %d argument allocation\\n\\n", i, j) : fprintf(stderr,
																				    "\e[1;31mError: task %d dependency %d argument allocation\e[0m\n\n",
																				    i, j);
							return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
						}
						//add order
						if (order) {
							(*workflow)->tasks[i].deps[j].order = (char *) strdup((const char *) order);
						} else {
							(*workflow)->tasks[i].deps[j].order = (char *) strdup((const char *) "0");
						}
						if (!((*workflow)->tasks[i].deps[j].order)) {
							oph_workflow_free(*workflow);
							if (jansson)
								json_decref(jansson);
							(print_json) ? my_fprintf(stderr, "Error: task %d dependency %d order allocation\\n\\n", i, j) : fprintf(stderr,
																				 "\e[1;31mError: task %d dependency %d order allocation\e[0m\n\n",
																				 i, j);
							return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
						}
						//add type
						if (type) {
							if (strcmp(type, "all") && strcmp(type, "single") && strcmp(type, "embedded")) {
								oph_workflow_free(*workflow);
								if (jansson)
									json_decref(jansson);
								(print_json) ? my_fprintf(stderr, "Error: task %d dependency %d invalid dependency type\\n\\n", i, j) : fprintf(stderr,
																						"\e[1;31mError: task %d dependency %d invalid dependency type\e[0m\n\n",
																						i, j);
								return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
							}
							(*workflow)->tasks[i].deps[j].type = (char *) strdup((const char *) type);
						} else {
							(*workflow)->tasks[i].deps[j].type = (char *) strdup((const char *) "embedded");
						}
						if (!((*workflow)->tasks[i].deps[j].type)) {
							oph_workflow_free(*workflow);
							if (jansson)
								json_decref(jansson);
							(print_json) ? my_fprintf(stderr, "Error: task %d dependency %d dependency type allocation\\n\\n", i, j) : fprintf(stderr,
																					   "\e[1;31mError: task %d dependency %d dependency type allocation\e[0m\n\n",
																					   i, j);
							return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
						}
						//add filter
						if (filter) {
							(*workflow)->tasks[i].deps[j].filter = (char *) strdup((const char *) filter);
						} else {
							(*workflow)->tasks[i].deps[j].filter = (char *) strdup((const char *) "all");
						}
						if (!((*workflow)->tasks[i].deps[j].filter)) {
							oph_workflow_free(*workflow);
							if (jansson)
								json_decref(jansson);
							(print_json) ? my_fprintf(stderr, "Error: task %d dependency %d filter allocation\\n\\n", i, j) : fprintf(stderr,
																				  "\e[1;31mError: task %d dependency %d filter allocation\e[0m\n\n",
																				  i, j);
							return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
						}
						//add output_argument
						if (output_argument) {
							(*workflow)->tasks[i].deps[j].output_argument = (char *) strdup((const char *) output_argument);
						} else {
							(*workflow)->tasks[i].deps[j].output_argument = (char *) strdup((const char *) "cube");
						}
						if (!((*workflow)->tasks[i].deps[j].output_argument)) {
							oph_workflow_free(*workflow);
							if (jansson)
								json_decref(jansson);
							(print_json) ? my_fprintf(stderr, "Error: task %d dependency %d output argument allocation\\n\\n", i, j) : fprintf(stderr,
																					   "\e[1;31mError: task %d dependency %d output argument allocation\e[0m\n\n",
																					   i, j);
							return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
						}
						//add output_order
						if (output_order) {
							(*workflow)->tasks[i].deps[j].output_order = (char *) strdup((const char *) output_order);
						} else {
							(*workflow)->tasks[i].deps[j].output_order = (char *) strdup((const char *) "0");
						}
						if (!((*workflow)->tasks[i].deps[j].output_order)) {
							oph_workflow_free(*workflow);
							if (jansson)
								json_decref(jansson);
							(print_json) ? my_fprintf(stderr, "Error: task %d dependency %d output order allocation\\n\\n", i, j) : fprintf(stderr,
																					"\e[1;31mError: task %d dependency %d output order allocation\e[0m\n\n",
																					i, j);
							return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
						}
					}
				}
			}
		}
		// Set the retry number
		if (!on_error_task)
			on_error_task = on_error;
		if (on_error_task) {
			if (!strcmp(on_error_task, OPH_WORKFLOW_SKIP))
				(*workflow)->tasks[i].retry_num = -1;
			else if (!strcmp(on_error_task, OPH_WORKFLOW_CONTINUE))
				(*workflow)->tasks[i].retry_num = 0;
			else if (!strcmp(on_error_task, OPH_WORKFLOW_BREAK))
				(*workflow)->tasks[i].retry_num = 1;
			else if (!strncmp(on_error_task, OPH_WORKFLOW_REPEAT, strlen(OPH_WORKFLOW_REPEAT))) {
				on_error_task += strlen(OPH_WORKFLOW_REPEAT);
				(*workflow)->tasks[i].retry_num = 1 + (int) strtol(on_error_task, NULL, 10);
				if ((*workflow)->tasks[i].retry_num < 1) {
					oph_workflow_free(*workflow);
					if (jansson)
						json_decref(jansson);
					(print_json) ? my_fprintf(stderr, "Error: task %d invalid on_error\\n\\n", i) : fprintf(stderr, "\e[1;31mError: task %d invalid on_error\e[0m\n\n", i);
					return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
				}
			} else {
				oph_workflow_free(*workflow);
				if (jansson)
					json_decref(jansson);
				(print_json) ? my_fprintf(stderr, "Error: task %d invalid on_error\\n\\n", i) : fprintf(stderr, "\e[1;31mError: task %d invalid on_error\e[0m\n\n", i);
				return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
			}
		} else
			(*workflow)->tasks[i].retry_num = 1;	// Default value
		(*workflow)->tasks[i].residual_retry_num = (*workflow)->tasks[i].retry_num;

		// Set the exit code
		if (!on_exit_task)
			on_exit_task = on_exit;
		if (on_exit_task) {
			if (!strlen(on_exit_task) || !strcmp(on_exit_task, OPH_WORKFLOW_NOP))
				(*workflow)->tasks[i].exit_action = 0;
			else if (!strcmp(on_exit_task, OPH_WORKFLOW_DELETE))
				(*workflow)->tasks[i].exit_action = -1;
			else {
				oph_workflow_free(*workflow);
				if (jansson)
					json_decref(jansson);
				(print_json) ? my_fprintf(stderr, "Error: task %d invalid on_exit_task\\n\\n", i) : fprintf(stderr, "\e[1;31mError: task %d invalid on_exit_task\e[0m\n\n", i);
				return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
			}
		} else
			(*workflow)->tasks[i].exit_action = 0;	// Default value (no operation)

		if (!run_task)
			run_task = run;
		if (run_task && strlen(run_task)) {
			if (!strcmp(run_task, OPH_WORKFLOW_NO))
				(*workflow)->tasks[i].run = 0;
			else if (strcmp(run_task, OPH_WORKFLOW_YES)) {
				oph_workflow_free(*workflow);
				if (jansson)
					json_decref(jansson);
				(print_json) ? my_fprintf(stderr, "Error: task %d invalid run value\\n\\n", i) : fprintf(stderr, "\e[1;31mError: task %d invalid run value\e[0m\n\n", i);
				return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
			}
		} else
			(*workflow)->tasks[i].run = 1;	// Default value (yes)

		(*workflow)->tasks[i].parent = -1;
	}

	//cleanup
	if (jansson)
		json_decref(jansson);

	//global var substitution
	//ncores
	if ((*workflow)->ncores != 0) {
		char buf[OPH_WORKFLOW_MIN_STRING];
		snprintf(buf, OPH_WORKFLOW_MIN_STRING, "%d", (*workflow)->ncores);
		if (_oph_workflow_substitute_var("ncores", buf, (*workflow)->tasks, (*workflow)->tasks_num)) {
			oph_workflow_free(*workflow);
			(print_json) ? my_fprintf(stderr, "Error: ncores global substitution\\n\\n") : fprintf(stderr, "\e[1;31mError: ncores global substitution\e[0m\n\n");
			return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
		}
	}
	// finalize ncores
	for (i = 0; i < (*workflow)->tasks_num; i++) {
		if (!(*workflow)->tasks[i].ncores) {
			for (j = 0; j < (*workflow)->tasks[i].arguments_num; j++) {
				if (!strcmp((*workflow)->tasks[i].arguments_keys[j], "ncores")) {
					(*workflow)->tasks[i].ncores = (int) strtol((*workflow)->tasks[i].arguments_values[j], NULL, 10);
					break;
				}
			}
		}
	}
	//cwd
	if (!(*workflow)->cwd)
		(*workflow)->cwd = strdup(OPH_WORKFLOW_ROOT_FOLDER);
	if (_oph_workflow_substitute_var("cwd", (*workflow)->cwd, (*workflow)->tasks, (*workflow)->tasks_num)) {
		oph_workflow_free(*workflow);
		(print_json) ? my_fprintf(stderr, "Error: cwd global substitution\\n\\n") : fprintf(stderr, "\e[1;31mError: cwd global substitution\e[0m\n\n");
		return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
	}
	//cdd
	if (!(*workflow)->cdd)
		(*workflow)->cdd = strdup(OPH_WORKFLOW_ROOT_FOLDER);
	if (_oph_workflow_substitute_var("cdd", (*workflow)->cdd, (*workflow)->tasks, (*workflow)->tasks_num)) {
		oph_workflow_free(*workflow);
		(print_json) ? my_fprintf(stderr, "Error: cdd global substitution\\n\\n") : fprintf(stderr, "\e[1;31mError: cdd global substitution\e[0m\n\n");
		return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
	}
	//cube
	if ((*workflow)->cube) {
		if (_oph_workflow_substitute_cube((*workflow)->cube, (*workflow)->tasks, (*workflow)->tasks_num)) {
			oph_workflow_free(*workflow);
			(print_json) ? my_fprintf(stderr, "Error: cube global substitution\\n\\n") : fprintf(stderr, "\e[1;31mError: cube global substitution\e[0m\n\n");
			return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
		}
	}

	return OPH_WORKFLOW_EXIT_SUCCESS;
}

// OTHER INTERNAL FUNCTIONS

int _oph_workflow_alloc(oph_workflow ** workflow)
{
	if (!workflow) {
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}
	//alloc
	*workflow = (oph_workflow *) calloc(1, sizeof(oph_workflow));
	if (!(*workflow)) {
		return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
	}
	//init
	(*workflow)->url = NULL;
	(*workflow)->abstract = NULL;
	(*workflow)->author = NULL;
	(*workflow)->callback_url = NULL;
	(*workflow)->cube = NULL;
	(*workflow)->cwd = NULL;
	(*workflow)->cdd = NULL;
	(*workflow)->exec_mode = NULL;
	(*workflow)->idjob = -1;
	(*workflow)->markerid = -1;
	(*workflow)->name = NULL;
	(*workflow)->ncores = 0;
	(*workflow)->residual_tasks_num = 0;
	(*workflow)->sessionid = NULL;
	(*workflow)->status = OPH_WORKFLOW_STATUS_PENDING;
	(*workflow)->tasks = NULL;
	(*workflow)->tasks_num = 0;
	(*workflow)->username = NULL;
	(*workflow)->response = NULL;
	(*workflow)->command = NULL;
	(*workflow)->exit_values = NULL;
	(*workflow)->workflowid = -1;
	(*workflow)->on_error = NULL;
	(*workflow)->on_exit = NULL;
	(*workflow)->run = NULL;
	(*workflow)->output_format = 0;

	return OPH_WORKFLOW_EXIT_SUCCESS;
}

int _oph_workflow_substitute_var(char *key, char *value, oph_workflow_task * tasks, int tasks_num)
{
	if (!key || !value || !tasks || tasks_num < 1) {
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}

	int i, j;
	for (i = 0; i < tasks_num; i++) {
		if (tasks[i].arguments_num == 0) {
			if (!strcmp(key, "ncores")) {
				tasks[i].ncores = (int) strtol(value, NULL, 10);
			} else {
				tasks[i].arguments_keys = (char **) calloc(1, sizeof(char *));
				if (!(tasks[i].arguments_keys)) {
					return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
				}
				tasks[i].arguments_values = (char **) calloc(1, sizeof(char *));
				if (!(tasks[i].arguments_values)) {
					free(tasks[i].arguments_keys);
					tasks[i].arguments_keys = NULL;
					return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
				}
				tasks[i].arguments_num++;
				tasks[i].arguments_keys[0] = (char *) strdup((const char *) key);
				if (!(tasks[i].arguments_keys[0])) {
					return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
				}
				tasks[i].arguments_values[0] = (char *) strdup((const char *) value);
				if (!(tasks[i].arguments_values[0])) {
					return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
				}
			}
		} else {
			int found = 0;
			for (j = 0; j < tasks[i].arguments_num; j++) {
				if (!strcmp(tasks[i].arguments_keys[j], key)) {
					found = 1;
					if (!strcmp(key, "ncores")) {
						tasks[i].ncores = (int) strtol(tasks[i].arguments_values[j], NULL, 10);
						free(tasks[i].arguments_keys[j]);
						tasks[i].arguments_keys[j] = NULL;
						free(tasks[i].arguments_values[j]);
						tasks[i].arguments_values[j] = NULL;
						char **tmpkeys = (char **) calloc(tasks[i].arguments_num - 1, sizeof(char *));
						if (!tmpkeys) {
							return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
						}
						char **tmpvalues = (char **) calloc(tasks[i].arguments_num - 1, sizeof(char *));
						if (!tmpvalues) {
							free(tmpkeys);
							return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
						}
						int k, q = 0;
						for (k = 0; k < tasks[i].arguments_num; k++) {
							if (tasks[i].arguments_keys[k]) {
								tmpkeys[q] = tasks[i].arguments_keys[k];
								tmpvalues[q] = tasks[i].arguments_values[k];
								q++;
							}
						}
						free(tasks[i].arguments_keys);
						free(tasks[i].arguments_values);
						tasks[i].arguments_keys = tmpkeys;
						tasks[i].arguments_values = tmpvalues;
						tasks[i].arguments_num--;
					}
					break;
				}
			}
			if (!found) {
				if (!strcmp(key, "ncores")) {
					tasks[i].ncores = (int) strtol(value, NULL, 10);
				} else {
					char **tmpkeys = tasks[i].arguments_keys;
					char **tmpvalues = tasks[i].arguments_values;
					tasks[i].arguments_keys = (char **) realloc(tasks[i].arguments_keys, sizeof(char *) * (tasks[i].arguments_num + 1));
					if (!(tasks[i].arguments_keys)) {
						tasks[i].arguments_keys = tmpkeys;
						return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
					}
					tasks[i].arguments_keys[tasks[i].arguments_num] = NULL;
					tasks[i].arguments_values = (char **) realloc(tasks[i].arguments_values, sizeof(char *) * (tasks[i].arguments_num + 1));
					if (!(tasks[i].arguments_values)) {
						tasks[i].arguments_values = tmpvalues;
						return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
					}
					tasks[i].arguments_values[tasks[i].arguments_num] = NULL;
					tasks[i].arguments_num++;
					tasks[i].arguments_keys[tasks[i].arguments_num - 1] = (char *) strdup((const char *) key);
					if (!(tasks[i].arguments_keys[tasks[i].arguments_num - 1])) {
						return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
					}
					tasks[i].arguments_values[tasks[i].arguments_num - 1] = (char *) strdup((const char *) value);
					if (!(tasks[i].arguments_values[tasks[i].arguments_num - 1])) {
						return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
					}
				}
			}
		}
	}

	return OPH_WORKFLOW_EXIT_SUCCESS;
}

int _oph_workflow_substitute_cube(char *pid, oph_workflow_task * tasks, int tasks_num)
{
	if (!pid || !tasks || tasks_num < 1) {
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}

	int i, j;
	for (i = 0; i < tasks_num; i++) {
		if (tasks[i].arguments_num == 0) {
			int k;
			int ok = 1;
			for (k = 0; k < tasks[i].deps_num; k++) {
				if (strcmp(tasks[i].deps[k].type, "embedded")) {
					ok = 0;
					break;
				}
			}
			if (ok) {
				tasks[i].arguments_keys = (char **) calloc(1, sizeof(char *));
				if (!(tasks[i].arguments_keys)) {
					return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
				}
				tasks[i].arguments_values = (char **) calloc(1, sizeof(char *));
				if (!(tasks[i].arguments_values)) {
					free(tasks[i].arguments_keys);
					tasks[i].arguments_keys = NULL;
					return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
				}
				tasks[i].arguments_num++;
				tasks[i].arguments_keys[0] = (char *) strdup((const char *) "cube");
				if (!(tasks[i].arguments_keys[0])) {
					return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
				}
				tasks[i].arguments_values[0] = (char *) strdup((const char *) pid);
				if (!(tasks[i].arguments_values[0])) {
					return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
				}
			}
		} else {
			int found = 0;
			for (j = 0; j < tasks[i].arguments_num; j++) {
				if (!strcmp(tasks[i].arguments_keys[j], "cube")) {
					found = 1;
					break;
				}
			}
			if (!found) {
				int k;
				int ok = 1;
				for (k = 0; k < tasks[i].deps_num; k++) {
					if (strcmp(tasks[i].deps[k].type, "embedded")) {
						ok = 0;
						break;
					}
				}
				if (ok) {
					char **tmpkeys = tasks[i].arguments_keys;
					char **tmpvalues = tasks[i].arguments_values;
					tasks[i].arguments_keys = (char **) realloc(tasks[i].arguments_keys, sizeof(char *) * (tasks[i].arguments_num + 1));
					if (!(tasks[i].arguments_keys)) {
						tasks[i].arguments_keys = tmpkeys;
						return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
					}
					tasks[i].arguments_keys[tasks[i].arguments_num] = NULL;
					tasks[i].arguments_values = (char **) realloc(tasks[i].arguments_values, sizeof(char *) * (tasks[i].arguments_num + 1));
					if (!(tasks[i].arguments_values)) {
						tasks[i].arguments_values = tmpvalues;
						return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
					}
					tasks[i].arguments_values[tasks[i].arguments_num] = NULL;
					tasks[i].arguments_num++;
					tasks[i].arguments_keys[tasks[i].arguments_num - 1] = (char *) strdup((const char *) "cube");
					if (!(tasks[i].arguments_keys[tasks[i].arguments_num - 1])) {
						return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
					}
					tasks[i].arguments_values[tasks[i].arguments_num - 1] = (char *) strdup((const char *) pid);
					if (!(tasks[i].arguments_values[tasks[i].arguments_num - 1])) {
						return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
					}
				}
			}
		}
	}

	return OPH_WORKFLOW_EXIT_SUCCESS;
}
