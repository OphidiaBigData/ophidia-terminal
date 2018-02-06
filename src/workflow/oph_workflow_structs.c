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

#include "oph_workflow_structs.h"

int oph_workflow_free(oph_workflow * workflow)
{
	if (!workflow)
		return OPH_WORKFLOW_EXIT_SUCCESS;
	int i;
	if (workflow->url) {
		free(workflow->url);
		workflow->url = NULL;
	}
	if (workflow->abstract) {
		free(workflow->abstract);
		workflow->abstract = NULL;
	}
	if (workflow->author) {
		free(workflow->author);
		workflow->author = NULL;
	}
	if (workflow->callback_url) {
		free(workflow->callback_url);
		workflow->callback_url = NULL;
	}
	if (workflow->cube) {
		free(workflow->cube);
		workflow->cube = NULL;
	}
	if (workflow->cwd) {
		free(workflow->cwd);
		workflow->cwd = NULL;
	}
	if (workflow->cdd) {
		free(workflow->cdd);
		workflow->cdd = NULL;
	}
	if (workflow->exec_mode) {
		free(workflow->exec_mode);
		workflow->exec_mode = NULL;
	}
	if (workflow->name) {
		free(workflow->name);
		workflow->name = NULL;
	}
	if (workflow->sessionid) {
		free(workflow->sessionid);
		workflow->sessionid = NULL;
	}
	if (workflow->username) {
		free(workflow->username);
		workflow->username = NULL;
	}
	if (workflow->tasks_num) {
		for (i = 0; i < workflow->tasks_num; i++) {
			oph_workflow_task_free(&(workflow->tasks[i]));
		}
		free(workflow->tasks);
		workflow->tasks = NULL;
	}
	if (workflow->response) {
		free(workflow->response);
		workflow->response = NULL;
	}
	if (workflow->command) {
		free(workflow->command);
		workflow->command = NULL;
	}
	if (workflow->exit_values) {
		free(workflow->exit_values);
		workflow->exit_values = NULL;
	}
	if (workflow->on_error) {
		free(workflow->on_error);
		workflow->on_error = NULL;
	}
	if (workflow->on_exit) {
		free(workflow->on_exit);
		workflow->on_exit = NULL;
	}
	if (workflow->run) {
		free(workflow->run);
		workflow->run = NULL;
	}
	if (workflow->host_partition) {
		free(workflow->host_partition);
		workflow->host_partition = NULL;
	}
	free(workflow);
	workflow = NULL;
	return OPH_WORKFLOW_EXIT_SUCCESS;
}

int oph_workflow_task_free(oph_workflow_task * task)
{
	if (!task)
		return OPH_WORKFLOW_EXIT_SUCCESS;
	int i;
	if (task->arguments_num) {
		for (i = 0; i < task->arguments_num; i++) {
			if (task->arguments_keys[i]) {
				free(task->arguments_keys[i]);
				task->arguments_keys[i] = NULL;
			}
			if (task->arguments_values[i]) {
				free(task->arguments_values[i]);
				task->arguments_values[i] = NULL;
			}
		}
		free(task->arguments_keys);
		task->arguments_keys = NULL;
		free(task->arguments_values);
		task->arguments_values = NULL;
	}
	if (task->dependents_indexes_num) {
		free(task->dependents_indexes);
		task->dependents_indexes = NULL;
	}
	if (task->deps_num) {
		for (i = 0; i < task->deps_num; i++) {
			oph_workflow_dep_free(&(task->deps[i]));
		}
		free(task->deps);
		task->deps = NULL;
	}
	if (task->light_tasks_num) {
		for (i = 0; i < task->light_tasks_num; i++) {
			oph_workflow_light_task_free(&(task->light_tasks[i]));
		}
		free(task->light_tasks);
		task->light_tasks = NULL;
	}
	if (task->name) {
		free(task->name);
		task->name = NULL;
	}
	if (task->operator) {
		free(task->operator);
		task->operator= NULL;
	}
	if (task->outputs_num) {
		for (i = 0; i < task->outputs_num; i++) {
			if (task->outputs_keys[i]) {
				free(task->outputs_keys[i]);
				task->outputs_keys[i] = NULL;
			}
			if (task->outputs_values[i]) {
				free(task->outputs_values[i]);
				task->outputs_values[i] = NULL;
			}
		}
		free(task->outputs_keys);
		task->outputs_keys = NULL;
		free(task->outputs_values);
		task->outputs_values = NULL;
	}
	if (task->response) {
		free(task->response);
		task->response = NULL;
	}
	return OPH_WORKFLOW_EXIT_SUCCESS;
}

int oph_workflow_dep_free(oph_workflow_dep * dep)
{
	if (!dep)
		return OPH_WORKFLOW_EXIT_SUCCESS;
	if (dep->argument) {
		free(dep->argument);
		dep->argument = NULL;
	}
	if (dep->filter) {
		free(dep->filter);
		dep->filter = NULL;
	}
	if (dep->order) {
		free(dep->order);
		dep->order = NULL;
	}
	if (dep->output_argument) {
		free(dep->output_argument);
		dep->output_argument = NULL;
	}
	if (dep->output_order) {
		free(dep->output_order);
		dep->output_order = NULL;
	}
	if (dep->task_name) {
		free(dep->task_name);
		dep->task_name = NULL;
	}
	if (dep->type) {
		free(dep->type);
		dep->type = NULL;
	}
	return OPH_WORKFLOW_EXIT_SUCCESS;
}

int oph_workflow_light_task_free(oph_workflow_light_task * light_task)
{
	if (!light_task)
		return OPH_WORKFLOW_EXIT_SUCCESS;
	int i;
	if (light_task->arguments_num) {
		for (i = 0; i < light_task->arguments_num; i++) {
			if (light_task->arguments_keys[i]) {
				free(light_task->arguments_keys[i]);
				light_task->arguments_keys[i] = NULL;
			}
			if (light_task->arguments_values[i]) {
				free(light_task->arguments_values[i]);
				light_task->arguments_values[i] = NULL;
			}
		}
		free(light_task->arguments_keys);
		light_task->arguments_keys = NULL;
		free(light_task->arguments_values);
		light_task->arguments_values = NULL;
	}
	if (light_task->response) {
		free(light_task->response);
		light_task->response = NULL;
	}
	return OPH_WORKFLOW_EXIT_SUCCESS;
}
