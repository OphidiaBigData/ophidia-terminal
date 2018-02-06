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

#ifndef __OPH_WORKFLOW_STRUCTS_H
#define __OPH_WORKFLOW_STRUCTS_H

#include "oph_workflow_exit_codes.h"
#include "oph_workflow_status_codes.h"

/* \brief Struct for an OPH_WORKFLOW light task for massive operations
 * \param idjob ID of the job as appears in the DB
 * \param markerid Marker ID of the job
 * \param status Status of the task
 * \param ncores Number of ncores to be used in task execution
 * \param arguments_keys Array of explicit parameters keys for the operator
 * \param arguments_values Array of explicit parameters values for the operator
 * \param arguments_num Number of explicit parameters for the operator
 * \param response Output of the execution
 */
typedef struct _oph_workflow_light_task {
	int idjob;
	int markerid;
	int status;
	int ncores;
	char **arguments_keys;
	char **arguments_values;
	int arguments_num;
	char *response;
} oph_workflow_light_task;

/* \brief Struct for an OPH_WORKFLOW dependency
 * \param argument Name of the argument of the current task (default=cube)
 * \param order Argument position in case of multiple arguments (default=0)
 * \param task_name Name of task the current task depends on. It is initialized to -1, set correctly by the function oph_workflow_indexing and re-set to -1 by the engine when the dependence is satisfied
 * \param task_index Index of task the current task depends on
 * \param type Type of dependency: all, single or embedded (default=embedded)
 * \param filter Optional filter (default=all)
 * \param output_argument Name of the output of the "pointed" task (default=cube)
 * \param output_order Output position in case of multiple outputs (default=0)
 */
typedef struct _oph_workflow_dep {
	char *argument;
	char *order;
	char *task_name;
	int task_index;
	char *type;
	char *filter;
	char *output_argument;
	char *output_order;
} oph_workflow_dep;

/* \brief Struct for an OPH_WORKFLOW task
 * \param idjob ID of the job as appears in the DB
 * \param markerid Marker ID of the job
 * \param status Status of the task
 * \param name Name of the task
 * \param operator Name of the operator called by task
 * \param role Permission needed to execute the operator
 * \param ncores Number of ncores to be used in task execution
 * \param arguments_keys Array of explicit parameters keys for the operator
 * \param arguments_values Array of explicit parameters values for the operator
 * \param arguments_num Number of explicit parameters for the operator
 * \param deps Array of dependencies the current task depends on
 * \param deps_num Number of dependencies
 * \param residual_deps_num Number of residual dependencies to be satisfied (initialized to deps_num)
 * \param dependents_indexes Array of the indexes of all tasks depending on the current task
 * \param dependents_indexes_num Number of dependents
 * \param outputs_keys Array of output keys for the current task (default=cube)
 * \param outputs_values Array of output values for the current task (default=PID)
 * \param outputs_num Number of outputs for the current task
 * \param light_tasks Light tasks, used for massive operations
 * \param light_tasks_num Number of light tasks 
 * \param residual_light_tasks_num Number of residual light tasks
 * \param retry_num Number of attempts in case of error 
 * \param residual_retry_num Number of residual attempts in case of error
 * \param response Output of the execution
 * \param exit_action Code of the operation to be executed on the end of workflow 
 * \param run Flag used to enable/disable execution
 * \param parent Id of parent of a flow control operator
 */
typedef struct _oph_workflow_task {
	int idjob;
	int markerid;
	int status;
	char *name;
	char *operator;
	int role;
	int ncores;
	char **arguments_keys;
	char **arguments_values;
	int arguments_num;
	oph_workflow_dep *deps;
	int deps_num;
	int residual_deps_num;
	int *dependents_indexes;
	int dependents_indexes_num;
	char **outputs_keys;
	char **outputs_values;
	int outputs_num;
	oph_workflow_light_task *light_tasks;
	int light_tasks_num;
	int residual_light_tasks_num;
	int retry_num;
	int residual_retry_num;
	char *response;
	int exit_action;
	int run;
	int parent;
} oph_workflow_task;

/* \brief Struct for an OPH_WORKFLOW
 * \param idjob ID of the job as appears in the DB
 * \param workflowid Workflow ID of the job
 * \param markerid Marker ID of the job
 * \param status Status of the workflow
 * \param username User executing the workflow
 * \param userrole User role
 * \param name Name of the workflow
 * \param author Author of the workflow
 * \param abstract Abstract of the workflow
 * \param url URL of the workflow
 * \param sessionid SessionID for the entire workflow
 * \param exec_mode Execution mode for the entire workflow
 * \param ncores Number of cores for the entire workflow
 * \param cwd CWD for the entire workflow
 * \param cdd CDD for the entire workflow
 * \param command Original command submitted by the user
 * \param cube Cube PID for the entire workflow
 * \param callback_url Callback URL for the entire workflow
 * \param tasks Array of tasks related to the workflow
 * \param tasks_num Number of tasks
 * \param residual_tasks_num Number of residual tasks (initialized to tasks_num)
 * \param response Output of the execution
 * \param exit_values Values to be used in the operation executed on the end of workflow
 * \param output_format Format to code workflow output
 * \param host_partition Host partition to be used during the workflow
 */
typedef struct _oph_workflow {
	int idjob;
	int workflowid;
	int markerid;
	int status;
	char *username;
	int userrole;
	char *name;
	char *author;
	char *abstract;
	char *url;
	char *sessionid;
	char *exec_mode;
	int ncores;
	char *command;
	char *cwd;
	char *cdd;
	char *cube;
	char *callback_url;
	oph_workflow_task *tasks;
	int tasks_num;
	int residual_tasks_num;
	char *response;
	char *exit_values;
	char *on_error;
	char *on_exit;
	char *run;
	int output_format;
	char *host_partition;
} oph_workflow;

/* Functions for structs cleanup */
int oph_workflow_free(oph_workflow * workflow);
int oph_workflow_task_free(oph_workflow_task * task);
int oph_workflow_dep_free(oph_workflow_dep * dep);
int oph_workflow_light_task_free(oph_workflow_light_task * light_task);

#endif				//__OPH_WORKFLOW_STRUCTS_H
