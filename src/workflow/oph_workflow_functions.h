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

#ifndef __OPH_WORKFLOW_FUNCTIONS_H
#define __OPH_WORKFLOW_FUNCTIONS_H

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

#include "../env/hashtbl.h"
#include "oph_workflow_structs.h"

/* \brief Function for JSON parsing and global variables substitution (ncores,cwd and cube)
 * \param json_string Input JSON string
 * \param username Input username
 * \param workflow Output OPH_WORKFLOW
 * \return 0 if successful
 */
int oph_workflow_load(char *json_string, char *username, oph_workflow ** workflow);

/* \brief Function to index task names in deps and init dependents_indexes
 * \param tasks Array of tasks
 * \param tasks_num Number of tasks
 * \return 0 if successful
 */
int oph_workflow_indexing(oph_workflow_task * tasks, int tasks_num);

/* \brief Function for cyclic dependencies check
 * \param workflow Input OPH_WORKFLOW to be validated
 * \return 0 if successful
 */
int oph_workflow_validate(oph_workflow * workflow);

/* \brief Function for the setup of the initial array of independent tasks
 * \param tasks Array of tasks
 * \param tasks_num Number of tasks
 * \param initial_tasks_indexes Output array of initial tasks indexes
 * \param initial_tasks_indexes_num Number of initial tasks
 * \return 0 if successful
 */
int oph_workflow_init(oph_workflow_task * tasks, int tasks_num, int **initial_tasks_indexes, int *initial_tasks_indexes_num);

/* \brief Function for printing a workflow as an image
 * \param workflow Input OPH_WORKFLOW to be printed
 * \param save_img 1 to save image to file, 0 otherwise
 * \param open_img 1 to auto-open with xdg-open the image, 0 otherwise. Considered only when save_img=1
 * \param layout Layout of the graph
 * \return 0 if successful
 */
int oph_workflow_print(oph_workflow * workflow, int save_img, int open_img, char *layout);

/* \brief Function for printing a workflow status as an image
 * \param workflow Input OPH_WORKFLOW to be printed
 * \param save_img 1 to save image to file, 0 otherwise
 * \param open_img 1 to auto-open with xdg-open the image, 0 otherwise. Considered only when save_img=1
 * \param show_list Flag set in case Workflow Task List has to be shown
 * \param json_string_with_status JSON response with the list of tasks
 * \param layout Layout of the graph
 * \return 0 if successful
 */
int oph_workflow_print_status(oph_workflow * workflow, int save_img, int open_img, char *json_string_with_status, char *layout);

int view_status(int iterations_num, char *command_line, char *tmp_submission_string, HASHTBL * hashtbl, int *oph_term_return, char *tmp_session, char *tmp_workflow, int save_img, int open_img,
		int show_list, int time_interval, oph_workflow * wf);

int oph_workflow_get_ranks_string(oph_workflow_task * tasks, int tasks_num, char **ranks_string);

char *oph_print_exectime(char **exectime);

#endif				//__OPH_WORKFLOW_FUNCTIONS_H
