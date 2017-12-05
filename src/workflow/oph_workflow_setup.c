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
#include <ctype.h>

#include <gtk/gtk.h>

#include <gvc.h>

#include "oph_workflow_functions.h"
#include "oph_workflow_define.h"

#include "../viewer/oph_json/oph_json_library.h"
#include "../viewer/oph_term_viewer.h"
#include "../env/oph_term_env.h"
#include "../client/oph_term_client.h"

#define UNUSED(x) {(void)(x);}

GtkWidget *window;
GtkWidget *image;
GdkPixbuf *pixbuf;
GVC_t *gvc = NULL;
char filename[OPH_WORKFLOW_BASIC_SIZE];
int abort_view;

extern char *_passwd;
extern pthread_mutex_t global_flag;

typedef struct _gtkstruct {
	int iterations_num;
	char *command_line;
	char *tmp_submission_string;
	HASHTBL *hashtbl;
	int *oph_term_return;
	char *tmp_session;
	char *tmp_workflow;
	int save_img;
	int open_img;
	int show_list;
	int time_interval;
	oph_workflow *wf;
} gtkstruct;

typedef struct _gtkstruct2 {
	int rows;
	int cols;
} gtkstruct2;

// Internal structures

typedef struct _workflow_node {
	int *out_edges;
	int out_edges_num;
	int out_edges_size;
	int *in_edges;
	int in_edges_num;
	int in_edges_size;
	int index;
} workflow_node;

typedef struct _workflow_s_node {
	workflow_node *node;
	struct _workflow_s_node *next;
} workflow_s_node;

typedef struct _workflow_s_nodes {
	workflow_s_node *head;
	workflow_s_node *tail;
	int nodes_num;
} workflow_s_nodes;

int workflow_s_add(workflow_s_nodes * s, workflow_node * node);
int workflow_s_remove(workflow_s_nodes * s, workflow_node ** node);
int workflow_s_nodes_free(workflow_s_nodes * s);
int workflow_node_free(workflow_node * node);
int workflow_validate_fco(oph_workflow * wf);

// API functions

char *oph_print_exectime(char **exectime)
{
	if (!exectime)
		return NULL;
	char _exectime[OPH_WORKFLOW_BASIC_SIZE];
	double n_exectime = strtod(*exectime, NULL);
	if (n_exectime < 60.0)
		snprintf(_exectime, OPH_WORKFLOW_BASIC_SIZE, "%.2f seconds", n_exectime);
	else {
		n_exectime /= 60.0;	// minutes
		if (n_exectime < 60.0) {
			double f_n_exectime = floor(n_exectime);
			snprintf(_exectime, OPH_WORKFLOW_BASIC_SIZE, "%.0f minute%s %.2f seconds", f_n_exectime, f_n_exectime == 1 ? "" : "s", (n_exectime - floor(n_exectime)) * 60.0);
		} else {
			n_exectime /= 60.0;	// hours
			double f_n_exectime = floor(n_exectime);
			double s_exectime = (n_exectime - floor(n_exectime)) * 60.0;	// minutes
			double f_s_exectime = floor(s_exectime);
			snprintf(_exectime, OPH_WORKFLOW_BASIC_SIZE, "%.0f hour%s %.0f minute%s %.2f seconds", f_n_exectime, f_n_exectime == 1 ? "" : "s", f_s_exectime, f_s_exectime == 1 ? "" : "s",
				 (s_exectime - floor(s_exectime)) * 60.0);
		}
	}
	free(*exectime);
	*exectime = strdup(_exectime);
	return *exectime;
}

int oph_workflow_indexing(oph_workflow_task * tasks, int tasks_num)
{
	if (!tasks || tasks_num < 1) {
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}

	int i, j, k;
	oph_workflow_task *curtask = NULL;

	for (i = 0; i < tasks_num; i++) {
		curtask = &(tasks[i]);
		for (j = 0; j < curtask->deps_num; j++) {
			if (!strcmp(curtask->deps[j].task_name, curtask->name)) {
				return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
			}
			for (k = 0; k < tasks_num; k++) {
				if (!strcmp(curtask->deps[j].task_name, tasks[k].name)) {
					curtask->deps[j].task_index = k;
					if (tasks[k].dependents_indexes_num == 0) {
						tasks[k].dependents_indexes = (int *) calloc(1, sizeof(int));
						if (!(tasks[k].dependents_indexes)) {
							return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
						}
						tasks[k].dependents_indexes[0] = i;
						tasks[k].dependents_indexes_num++;
					} else {
						int *tmp = tasks[k].dependents_indexes;
						tasks[k].dependents_indexes = (int *) realloc(tasks[k].dependents_indexes, (tasks[k].dependents_indexes_num + 1) * sizeof(int));
						if (!(tasks[k].dependents_indexes)) {
							tasks[k].dependents_indexes = tmp;
							return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
						}
						tasks[k].dependents_indexes[tasks[k].dependents_indexes_num] = i;
						tasks[k].dependents_indexes_num++;
					}
					break;
				}
			}
			if (k >= tasks_num) {
				return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
			}
		}
	}

	return OPH_WORKFLOW_EXIT_SUCCESS;
}

int oph_workflow_validate(oph_workflow * workflow)
{
	if (!workflow || !(workflow->tasks)) {
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}
	// Check for uniqueness of task name
	int i, j, k;
	for (i = 0; i < workflow->tasks_num; i++)
		for (j = 0; j < workflow->tasks_num; j++)
			if ((i != j) && !strcmp(workflow->tasks[i].name, workflow->tasks[j].name))
				return OPH_WORKFLOW_EXIT_TASK_NAME_ERROR;

	// Create graph from tasks
	workflow_node *graph = NULL;
	graph = (workflow_node *) calloc(workflow->tasks_num, sizeof(workflow_node));
	if (!graph) {
		return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
	}

	for (i = 0; i < workflow->tasks_num; i++) {
		if (workflow->tasks[i].deps_num >= 1) {
			graph[i].in_edges = (int *) calloc(workflow->tasks[i].deps_num, sizeof(int));
			if (!(graph[i].in_edges)) {
				for (k = 0; k < workflow->tasks_num; k++)
					workflow_node_free(&(graph[k]));
				free(graph);
				graph = NULL;
				return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
			}
			graph[i].in_edges_size = workflow->tasks[i].deps_num;
			graph[i].in_edges_num = workflow->tasks[i].deps_num;
			for (j = 0; j < workflow->tasks[i].deps_num; j++)
				graph[i].in_edges[j] = workflow->tasks[i].deps[j].task_index;
		}
		if (workflow->tasks[i].dependents_indexes_num >= 1) {
			graph[i].out_edges = (int *) calloc(workflow->tasks[i].dependents_indexes_num, sizeof(int));
			if (!(graph[i].out_edges)) {
				for (k = 0; k < workflow->tasks_num; k++)
					workflow_node_free(&(graph[k]));
				free(graph);
				graph = NULL;
				return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
			}
			graph[i].out_edges_size = workflow->tasks[i].dependents_indexes_num;
			graph[i].out_edges_num = workflow->tasks[i].dependents_indexes_num;
			for (j = 0; j < workflow->tasks[i].dependents_indexes_num; j++)
				graph[i].out_edges[j] = workflow->tasks[i].dependents_indexes[j];
		}
		graph[i].index = i;
	}

	/*  Test for DAG through Topological Sort
	 *
	 *      S ← Set of all nodes with no incoming edges
	 *      while S is non-empty do
	 *          remove a node n from S
	 *          for each node m with an edge e from n to m do
	 *              remove edge e from the graph
	 *              if m has no other incoming edges then
	 *                  insert m into S
	 *      if graph has edges then
	 *          return error (graph has at least one cycle)
	 *      else
	 *          return success (graph has no cycles)
	 */

	//      S ← Set of all nodes with no incoming edges
	workflow_s_nodes S;
	S.head = NULL;
	S.tail = NULL;
	S.nodes_num = 0;
	for (i = 0; i < workflow->tasks_num; i++) {
		if (graph[i].in_edges_num == 0) {
			if (workflow_s_add(&S, &(graph[i]))) {
				for (k = 0; k < workflow->tasks_num; k++)
					workflow_node_free(&(graph[k]));
				free(graph);
				graph = NULL;
				workflow_s_nodes_free(&S);
				return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
			}
		}
	}

	workflow_node *n = NULL;

	//      while S is non-empty do
	while (S.nodes_num != 0) {
		//          remove a node n from S
		if (workflow_s_remove(&S, &n)) {
			for (k = 0; k < workflow->tasks_num; k++)
				workflow_node_free(&(graph[k]));
			free(graph);
			graph = NULL;
			workflow_s_nodes_free(&S);
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
		//          for each node m with an edge e from n to m do
		for (i = 0; i < n->out_edges_size; i++) {
			if (n->out_edges[i] != -1) {
				//              remove edge e from the graph
				int index = n->out_edges[i];
				n->out_edges[i] = -1;
				n->out_edges_num--;
				for (j = 0; j < graph[index].in_edges_size; j++) {
					if (graph[index].in_edges[j] == n->index) {
						graph[index].in_edges[j] = -1;
						graph[index].in_edges_num--;
						//              if m has no other incoming edges then
						if (graph[index].in_edges_num == 0) {
							//                  insert m into S
							if (workflow_s_add(&S, &(graph[index]))) {
								for (k = 0; k < workflow->tasks_num; k++)
									workflow_node_free(&(graph[k]));
								free(graph);
								graph = NULL;
								workflow_s_nodes_free(&S);
								return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
							}
						}
						break;
					}
				}
			}
		}
	}

	for (i = 0; i < workflow->tasks_num; i++) {
		//      if graph has edges then
		if (graph[i].in_edges_num != 0 || graph[i].out_edges_num != 0) {
			for (k = 0; k < workflow->tasks_num; k++)
				workflow_node_free(&(graph[k]));
			free(graph);
			graph = NULL;
			workflow_s_nodes_free(&S);
			//          return error (graph has at least one cycle)
			return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
		}
	}

	//      else
	for (k = 0; k < workflow->tasks_num; k++)
		workflow_node_free(&(graph[k]));
	free(graph);
	graph = NULL;
	workflow_s_nodes_free(&S);
	//          return success (graph has no cycles)

	// Check for flow control operators
	if (workflow_validate_fco(workflow))
		return OPH_WORKFLOW_EXIT_FLOW_CONTROL_ERROR;

	return OPH_WORKFLOW_EXIT_SUCCESS;
}

int oph_workflow_init(oph_workflow_task * tasks, int tasks_num, int **initial_tasks_indexes, int *initial_tasks_indexes_num)
{
	if (!tasks || tasks_num < 1 || !initial_tasks_indexes || !initial_tasks_indexes_num) {
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}

	(*initial_tasks_indexes) = NULL;
	(*initial_tasks_indexes_num) = 0;

	int i;
	for (i = 0; i < tasks_num; i++)
		if (tasks[i].deps_num < 1)
			(*initial_tasks_indexes_num)++;

	if ((*initial_tasks_indexes_num) == 0) {
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}

	(*initial_tasks_indexes) = (int *) calloc((*initial_tasks_indexes_num), sizeof(int));
	if (!(*initial_tasks_indexes)) {
		(*initial_tasks_indexes_num) = 0;
		return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
	}

	int j = 0;
	for (i = 0; i < tasks_num; i++) {
		if (tasks[i].deps_num < 1) {
			(*initial_tasks_indexes)[j] = i;
			j++;
			if (j == (*initial_tasks_indexes_num))
				break;
		}
	}

	return OPH_WORKFLOW_EXIT_SUCCESS;
}

int oph_workflow_get_subgraphs_string(oph_workflow * workflow, char **subgraphs_string, int count, int k, int *subgraphs, int *visited, int flag, int openfor)
{
	if (!workflow || !subgraphs_string)
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;

	int cc = count;
	int openf = openfor;

	if (!(*subgraphs_string)) {
		*subgraphs_string = (char *) calloc(OPH_WORKFLOW_DOT_MAX_LEN, sizeof(char));
		if (!(*subgraphs_string))
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		visited = (int *) calloc(workflow->tasks_num, sizeof(int));
		if (!visited) {
			free(*subgraphs_string);
			*subgraphs_string = NULL;
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
	}

	if (k == -1) {
		int i, res;
		for (i = 0; i < workflow->tasks_num; i++) {
			if ((!strcasecmp(workflow->tasks[i].operator, OPH_OPERATOR_FOR) || !strcasecmp(workflow->tasks[i].operator, OPH_OPERATOR_IF)) && !visited[i]) {
				res = oph_workflow_get_subgraphs_string(workflow, subgraphs_string, cc, i, subgraphs, visited, 0, openfor);
				if (res) {
					if (visited) {
						free(visited);
						visited = NULL;
					}
					if (*subgraphs_string) {
						free(*subgraphs_string);
						*subgraphs_string = NULL;
					}
					return res;
				}
			}
		}
		if (visited) {
			free(visited);
			visited = NULL;
		}
		return OPH_WORKFLOW_EXIT_SUCCESS;
	} else if (flag == 1) {
		int i, kk, res;
		for (i = 0; i < workflow->tasks[k].dependents_indexes_num; i++) {
			kk = workflow->tasks[k].dependents_indexes[i];
			if (!strcasecmp(workflow->tasks[kk].operator, OPH_OPERATOR_FOR) || !strcasecmp(workflow->tasks[kk].operator, OPH_OPERATOR_IF)) {
				res = oph_workflow_get_subgraphs_string(workflow, subgraphs_string, cc, kk, subgraphs, visited, 0, openfor - 1);
				if (res) {
					if (visited) {
						free(visited);
						visited = NULL;
					}
					if (*subgraphs_string) {
						free(*subgraphs_string);
						*subgraphs_string = NULL;
					}
					return res;
				}
				openf++;
			}
			cc = strlen(*subgraphs_string);
			cc += snprintf((*subgraphs_string) + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, " %d", kk);
			if (cc > OPH_WORKFLOW_DOT_MAX_LEN - 2) {
				if (visited) {
					free(visited);
					visited = NULL;
				}
				if (*subgraphs_string) {
					free(*subgraphs_string);
					*subgraphs_string = NULL;
				}
				return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
			}
			if (strcasecmp(workflow->tasks[kk].operator, OPH_OPERATOR_ENDFOR) && strcasecmp(workflow->tasks[kk].operator, OPH_OPERATOR_ENDIF)) {
				res = oph_workflow_get_subgraphs_string(workflow, subgraphs_string, cc, kk, subgraphs, visited, 1, openf);
				if (res) {
					if (visited) {
						free(visited);
						visited = NULL;
					}
					if (*subgraphs_string) {
						free(*subgraphs_string);
						*subgraphs_string = NULL;
					}
					return res;
				}
			} else {
				if ((openf - 1) != 0) {
					res = oph_workflow_get_subgraphs_string(workflow, subgraphs_string, cc, kk, subgraphs, visited, 1, openf - 1);
					if (res) {
						if (visited) {
							free(visited);
							visited = NULL;
						}
						if (*subgraphs_string) {
							free(*subgraphs_string);
							*subgraphs_string = NULL;
						}
						return res;
					}
				}
			}
		}
		return OPH_WORKFLOW_EXIT_SUCCESS;
	} else {		// flag == 0
		cc = strlen(*subgraphs_string);
		cc += snprintf((*subgraphs_string) + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, " subgraph cluster_%d {%d", *subgraphs, k);
		if (cc > OPH_WORKFLOW_DOT_MAX_LEN - 2) {
			if (visited) {
				free(visited);
				visited = NULL;
			}
			if (*subgraphs_string) {
				free(*subgraphs_string);
				*subgraphs_string = NULL;
			}
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
		(*subgraphs)++;
		visited[k] = 1;
		int res;
		res = oph_workflow_get_subgraphs_string(workflow, subgraphs_string, cc, k, subgraphs, visited, 1, openfor + 1);
		if (res) {
			if (visited) {
				free(visited);
				visited = NULL;
			}
			if (*subgraphs_string) {
				free(*subgraphs_string);
				*subgraphs_string = NULL;
			}
			return res;
		}
		cc = snprintf((*subgraphs_string) + strlen(*subgraphs_string), OPH_WORKFLOW_DOT_MAX_LEN - strlen(*subgraphs_string), "} ");
		if (cc > OPH_WORKFLOW_DOT_MAX_LEN - 2) {
			if (visited) {
				free(visited);
				visited = NULL;
			}
			if (*subgraphs_string) {
				free(*subgraphs_string);
				*subgraphs_string = NULL;
			}
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
		return OPH_WORKFLOW_EXIT_SUCCESS;
	}
}

int oph_workflow_print(oph_workflow * workflow, int save_img, int open_img, char *layout)
{
	if (!workflow || (save_img != 0 && save_img != 1) || (open_img != 0 && open_img != 1)) {
		(print_json) ? my_fprintf(stderr, "Unable to create the image.\\n") : fprintf(stderr, "\n\e[1;31mUnable to create the image.\e[0m\n");
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}

	if (save_img) {
		// create dot string for dag
		char dot_string[2 * OPH_WORKFLOW_DOT_MAX_LEN];
		size_t cc = 0, i, j;
		int k, kk;

		// name
		for (i = 0; i < strlen(workflow->name); i++)
			if (!isalnum(workflow->name[i]))
				workflow->name[i] = '_';
		cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, "digraph %s {style=rounded; node [shape=circle,width=2.3,penwidth=3] ", workflow->name);

		// nodes
		for (i = 0; i < (size_t) workflow->tasks_num; i++) {
			cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, "%d [label=\"", (int) i);
			cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, "%.20s\\n%.20s", workflow->tasks[i].name, workflow->tasks[i].operator);
			if (!strcasecmp(workflow->tasks[i].operator, OPH_OPERATOR_FOR) || !strcasecmp(workflow->tasks[i].operator, OPH_OPERATOR_ENDFOR)) {
				cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, "\",shape=hexagon] ");
			} else if (!strcasecmp(workflow->tasks[i].operator, OPH_OPERATOR_IF) || !strcasecmp(workflow->tasks[i].operator, OPH_OPERATOR_ELSEIF)
				   || !strcasecmp(workflow->tasks[i].operator, OPH_OPERATOR_ELSE) || !strcasecmp(workflow->tasks[i].operator, OPH_OPERATOR_ENDIF)) {
				cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, "\",shape=diamond] ");
			} else {
				cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, "\"] ");
			}
		}

		// edges
		char buffer[OPH_WORKFLOW_RANK_SIZE];
		for (i = 0; i < (size_t) workflow->tasks_num; i++) {
			for (j = 0; j < (size_t) workflow->tasks[i].dependents_indexes_num; j++) {
				kk = workflow->tasks[i].dependents_indexes[j];
				for (k = 0; k < workflow->tasks[kk].deps_num; k++) {
					if ((size_t) workflow->tasks[kk].deps[k].task_index == i) {
						if (!workflow->tasks[kk].deps[k].type || !strcmp(workflow->tasks[kk].deps[k].type, "embedded"))
							snprintf(buffer, OPH_WORKFLOW_RANK_SIZE, "dashed");
						else
							snprintf(buffer, OPH_WORKFLOW_RANK_SIZE, "solid");
						break;
					}
				}
				cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, "%d->%d [penwidth=3,style=%s] ", (int) i, kk, buffer);
			}
		}

		if (layout) {
			if (strstr(layout, "ranked")) {
				char *ranks_string = NULL;
				if (oph_workflow_get_ranks_string(workflow->tasks, workflow->tasks_num, &ranks_string)) {
					(print_json) ? my_fprintf(stderr, "Unable to create the image.\\n") : fprintf(stderr, "\n\e[1;31mUnable to create the image.\e[0m\n");
					return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
				}
				cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, "%s", ranks_string);
				free(ranks_string);
				ranks_string = NULL;
			}
			if (strstr(layout, "horizontal")) {
				cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, " rankdir=LR; ");
			}
		}

		if (cc > OPH_WORKFLOW_DOT_MAX_LEN - 2) {
			(print_json) ? my_fprintf(stderr, "Unable to create the image (buffer overflow).\\n") : fprintf(stderr, "\n\e[1;31mUnable to create the image (buffer overflow).\e[0m\n");
			return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
		}
		// group FOR/ENDFOR as subgraphs
		char *subgraphs_string = NULL;
		int subgraphs = 0;
		int *visited = NULL;
		if (oph_workflow_get_subgraphs_string(workflow, &subgraphs_string, 0, -1, &subgraphs, visited, 0, 0)) {
			(print_json) ? my_fprintf(stderr, "Unable to create the image.\\n") : fprintf(stderr, "\n\e[1;31mUnable to create the image.\e[0m\n");
			return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
		}

		cc += snprintf(dot_string + cc, 2 * OPH_WORKFLOW_DOT_MAX_LEN - cc, "%s}\n", subgraphs_string);
		free(subgraphs_string);
		subgraphs_string = NULL;

		if (cc > 2 * OPH_WORKFLOW_DOT_MAX_LEN - 2) {
			(print_json) ? my_fprintf(stderr, "Unable to create the image (buffer overflow).\\n") : fprintf(stderr, "\n\e[1;31mUnable to create the image (buffer overflow).\e[0m\n");
			return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
		}

		Agraph_t *g;
		FILE *fp;
		memset(filename, 0, OPH_WORKFLOW_BASIC_SIZE);
		snprintf(filename, OPH_WORKFLOW_BASIC_SIZE, "%s.svg", workflow->name);
		fp = fopen(filename, "w");
		if (!fp) {
			(print_json) ? my_fprintf(stderr, "Unable to create the new image file %s.\\n", filename) : fprintf(stderr, "Unable to create the new image file %s.\n", filename);
			return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
		}

		if (!gvc)
			gvc = gvContext();	// It should be free with gvFreeContext(gvc); on exit

		// create digraph
		g = agmemread(dot_string);
		if (!g) {
			fclose(fp);
			(print_json) ? my_fprintf(stderr, "Unable to create the image.\\n") : fprintf(stderr, "\n\e[1;31mUnable to create the image.\e[0m\n");
			return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
		}

		gvLayout(gvc, g, "dot");
		gvRender(gvc, g, "svg:cairo", fp);

		gvFreeLayout(gvc, g);
		agclose(g);
		fclose(fp);

		// print output filename
		(print_json) ? my_printf("Workflow Image File : %s\\n", filename) : printf("Workflow Image File : %s\n", filename);

		if (open_img) {
			char command[OPH_WORKFLOW_BASIC_SIZE];
			snprintf(command, OPH_WORKFLOW_BASIC_SIZE, "xdg-open %s", filename);
			// open image file
			if (system(command))
				fprintf(stderr, "Error in executing command '%s'\n", command);
		}
	}

	return OPH_WORKFLOW_EXIT_SUCCESS;
}

int oph_workflow_print_status(oph_workflow * workflow, int save_img, int open_img, char *json_string_with_status, char *layout)
{
	if (!workflow || (save_img != 0 && save_img != 1) || (open_img != 0 && open_img != 1) || !json_string_with_status) {
		(print_json) ? my_fprintf(stderr, "Unable to create the image.\\n") : fprintf(stderr, "\n\e[1;31mUnable to create the image.\e[0m\n");
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}

	char *tmp_json_string = strdup(json_string_with_status);
	if (!tmp_json_string) {
		(print_json) ? my_fprintf(stderr, "Unable to create the image.\\n") : fprintf(stderr, "\n\e[1;31mUnable to create the image.\e[0m\n");
		return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
	}

	oph_json *json = NULL;
	if (oph_json_from_json_string(&json, &tmp_json_string)) {
		(print_json) ? my_fprintf(stderr, "Unable to create the image.\\n") : fprintf(stderr, "\n\e[1;31mUnable to create the image.\e[0m\n");
		if (json)
			oph_json_free(json);
		return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
	}

	if (save_img) {
		// create dot string for dag
		char dot_string[2 * OPH_WORKFLOW_DOT_MAX_LEN];
		size_t cc = 0, i, j;
		int k, kk, offset = workflow->output_format ? 4 : 0;

		// name
		for (i = 0; i < strlen(workflow->name); i++)
			if (!isalnum(workflow->name[i]))
				workflow->name[i] = '_';
		cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, "digraph %s {style=rounded; node [style=filled,width=2.3,penwidth=3] ", workflow->name);

		char color[OPH_WORKFLOW_RANK_SIZE], doublecircle;

		// nodes
		for (i = 0; i < (size_t) workflow->tasks_num; i++) {
			doublecircle = 0;
			cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, "%d [label=", (int) i);
			cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, "\"%.20s\\n%.20s\"", workflow->tasks[i].name, workflow->tasks[i].operator);
			memset(color, 0, OPH_WORKFLOW_RANK_SIZE);
			snprintf(color, OPH_WORKFLOW_RANK_SIZE, "%s", "grey90");	//default
			for (j = 0; j < json->response_num; j++) {
				if (!strcmp(json->response[j].objkey, "workflow_list")) {
					int z;
					for (z = 0; z < (int) ((oph_json_obj_grid *) json->response[j].objcontent)[0].values_num1; z++) {
						if (!strcmp(workflow->tasks[i].name, ((oph_json_obj_grid *) json->response[j].objcontent)[0].values[z][5 - offset])) {
							if (!strcmp(((oph_json_obj_grid *) json->response[j].objcontent)[0].values[z][6 - offset], "MASSIVE"))
								doublecircle = 1;
							if (!strcmp(((oph_json_obj_grid *) json->response[j].objcontent)[0].values[z][7 - offset], "OPH_STATUS_COMPLETED")) {
								memset(color, 0, OPH_WORKFLOW_RANK_SIZE);
								snprintf(color, OPH_WORKFLOW_RANK_SIZE, "%s", "palegreen");
							} else if (!strcmp(((oph_json_obj_grid *) json->response[j].objcontent)[0].values[z][7 - offset], "OPH_STATUS_RUNNING")) {
								memset(color, 0, OPH_WORKFLOW_RANK_SIZE);
								snprintf(color, OPH_WORKFLOW_RANK_SIZE, "%s", "orange");
							} else if (strstr(((oph_json_obj_grid *) json->response[j].objcontent)[0].values[z][7 - offset], "ERROR")
								   || strstr(((oph_json_obj_grid *) json->response[j].objcontent)[0].values[z][7 - offset], "ABORTED")) {
								memset(color, 0, OPH_WORKFLOW_RANK_SIZE);
								snprintf(color, OPH_WORKFLOW_RANK_SIZE, "%s", "brown1");
							} else if (strstr(((oph_json_obj_grid *) json->response[j].objcontent)[0].values[z][7 - offset], "SKIPPED")) {
								memset(color, 0, OPH_WORKFLOW_RANK_SIZE);
								snprintf(color, OPH_WORKFLOW_RANK_SIZE, "%s", "khaki1");
							} else if (strstr(((oph_json_obj_grid *) json->response[j].objcontent)[0].values[z][7 - offset], "WAITING")) {
								memset(color, 0, OPH_WORKFLOW_RANK_SIZE);
								snprintf(color, OPH_WORKFLOW_RANK_SIZE, "%s", "cyan");
							} else if (strstr(((oph_json_obj_grid *) json->response[j].objcontent)[0].values[z][7 - offset], "PENDING")) {
								memset(color, 0, OPH_WORKFLOW_RANK_SIZE);
								snprintf(color, OPH_WORKFLOW_RANK_SIZE, "%s", "peachpuff");
							}
							break;
						}
					}
					break;
				}
			}
			cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, " fillcolor=\"%s\"", color);
			if (doublecircle)
				cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, " shape=doublecircle");
			else {
				if (!strcasecmp(workflow->tasks[i].operator, OPH_OPERATOR_FOR) || !strcasecmp(workflow->tasks[i].operator, OPH_OPERATOR_ENDFOR)) {
					cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, " shape=hexagon");
				} else if (!strcasecmp(workflow->tasks[i].operator, OPH_OPERATOR_IF) || !strcasecmp(workflow->tasks[i].operator, OPH_OPERATOR_ELSEIF)
					   || !strcasecmp(workflow->tasks[i].operator, OPH_OPERATOR_ELSE) || !strcasecmp(workflow->tasks[i].operator, OPH_OPERATOR_ENDIF)) {
					cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, " shape=diamond");
				} else {
					cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, " shape=circle");
				}
			}
			cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, "] ");
		}

		// edges
		for (i = 0; i < (size_t) workflow->tasks_num; i++) {
			for (j = 0; j < (size_t) workflow->tasks[i].dependents_indexes_num; j++) {
				kk = workflow->tasks[i].dependents_indexes[j];
				for (k = 0; k < workflow->tasks[kk].deps_num; k++) {
					if ((size_t) workflow->tasks[kk].deps[k].task_index == i) {
						if (!workflow->tasks[kk].deps[k].type || !strcmp(workflow->tasks[kk].deps[k].type, "embedded"))
							snprintf(color, OPH_WORKFLOW_RANK_SIZE, "dashed");
						else
							snprintf(color, OPH_WORKFLOW_RANK_SIZE, "solid");
						break;
					}
				}
				cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, "%d->%d [penwidth=3,style=%s] ", (int) i, kk, color);
			}
		}

		if (layout) {
			if (strstr(layout, "ranked")) {
				char *ranks_string = NULL;
				if (oph_workflow_get_ranks_string(workflow->tasks, workflow->tasks_num, &ranks_string)) {
					(print_json) ? my_fprintf(stderr, "Unable to create the image.\\n") : fprintf(stderr, "\n\e[1;31mUnable to create the image.\e[0m\n");
					if (json)
						oph_json_free(json);
					return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
				}
				cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, "%s", ranks_string);
				free(ranks_string);
				ranks_string = NULL;
			}
			if (strstr(layout, "horizontal")) {
				cc += snprintf(dot_string + cc, OPH_WORKFLOW_DOT_MAX_LEN - cc, " rankdir=LR; ");
			}
		}

		if (cc > OPH_WORKFLOW_DOT_MAX_LEN - 2) {
			(print_json) ? my_fprintf(stderr, "Unable to create the image (buffer overflow).\\n") : fprintf(stderr, "\n\e[1;31mUnable to create the image (buffer overflow).\e[0m\n");
			if (json)
				oph_json_free(json);
			return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
		}
		// group FOR/ENDFOR as subgraphs
		char *subgraphs_string = NULL;
		int subgraphs = 0;
		int *visited = NULL;
		if (oph_workflow_get_subgraphs_string(workflow, &subgraphs_string, 0, -1, &subgraphs, visited, 0, 0)) {
			(print_json) ? my_fprintf(stderr, "Unable to create the image.\\n") : fprintf(stderr, "\n\e[1;31mUnable to create the image.\e[0m\n");
			if (json)
				oph_json_free(json);
			return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
		}
		cc += snprintf(dot_string + cc, 2 * OPH_WORKFLOW_DOT_MAX_LEN - cc, "%s}\n", subgraphs_string);
		free(subgraphs_string);
		subgraphs_string = NULL;

		if (cc > 2 * OPH_WORKFLOW_DOT_MAX_LEN - 2) {
			(print_json) ? my_fprintf(stderr, "Unable to create the image (buffer overflow).\\n") : fprintf(stderr, "\n\e[1;31mUnable to create the image (buffer overflow).\e[0m\n");
			if (json)
				oph_json_free(json);
			return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
		}

		Agraph_t *g;
		FILE *fp;
		memset(filename, 0, OPH_WORKFLOW_BASIC_SIZE);
		snprintf(filename, OPH_WORKFLOW_BASIC_SIZE, "%s.svg", workflow->name);
		fp = fopen(filename, "w");
		if (!fp) {
			(print_json) ? my_fprintf(stderr, "Unable to create the new image file %s.\\n", filename) : fprintf(stderr, "Unable to create the new image file %s.\n", filename);
			if (json)
				oph_json_free(json);
			return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
		}

		if (!gvc)
			gvc = gvContext();	// It should be free with gvFreeContext(gvc); on exit

		// create digraph
		g = agmemread(dot_string);
		if (!g) {
			fclose(fp);
			if (json)
				oph_json_free(json);
			(print_json) ? my_fprintf(stderr, "Unable to create the image.\\n") : fprintf(stderr, "\n\e[1;31mUnable to create the image.\e[0m\n");
			return OPH_WORKFLOW_EXIT_GENERIC_ERROR;
		}

		gvLayout(gvc, g, "dot");
		gvRender(gvc, g, "svg:cairo", fp);
		gvFreeLayout(gvc, g);
		agclose(g);
		fclose(fp);

		// print output filename
		(print_json) ? my_printf("Workflow Image File : %s\\n", filename) : printf("Workflow Image File : %s\n", filename);

		if (open_img) {
			gdk_threads_enter();
			g_signal_emit_by_name(G_OBJECT(window), "state-changed");
			gdk_threads_leave();
		}
	}

	if (json)
		oph_json_free(json);

	return OPH_WORKFLOW_EXIT_SUCCESS;
}

void destroy(GtkWidget * window, GtkWidget * widget)
{
	UNUSED(window) UNUSED(widget)
	    gtk_main_quit();
}

gboolean resize_image(GtkWidget * window, GdkEvent * event, GtkWidget * widget)
{
	UNUSED(event)
	GError *error = NULL;
	if (strlen(filename) > 0) {
		if (pixbuf) {
			g_object_unref(G_OBJECT(pixbuf));
		}
		pixbuf = gdk_pixbuf_new_from_file_at_size(filename, window->allocation.width - 4, window->allocation.height - 4, &error);
		if (pixbuf == NULL) {
			//g_printerr ("Error loading file: #%d %s\n", error->code, error->message);
			g_error_free(error);
			return 1;
		}
		gtk_image_set_from_pixbuf(GTK_IMAGE(widget), pixbuf);
	}

	return 0;
}

void set_image(GtkWidget * widget, gpointer pixbuf)
{
	gtk_image_set_from_pixbuf(GTK_IMAGE(widget), (GdkPixbuf *) pixbuf);
}

void resize_image2(GtkWidget * window, GtkWidget * widget)
{
	UNUSED(widget)
	GError *error = NULL;
	if (strlen(filename) > 0) {
		if (pixbuf) {
			g_object_unref(G_OBJECT(pixbuf));
		}
		pixbuf = gdk_pixbuf_new_from_file_at_size(filename, window->allocation.width - 4, window->allocation.height - 4, &error);
		if (pixbuf == NULL) {
			//g_printerr ("Error loading file: #%d %s\n", error->code, error->message);
			g_error_free(error);
			return;
		}
		gtk_container_foreach(GTK_CONTAINER(window), set_image, (gpointer) pixbuf);
	}

	return;
}

void *gtk_main_thread(void *ptr)
{
	/* Obtain gtk's global lock */
	gdk_threads_enter();

	/* Creat Widgets */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), ((gtkstruct2 *) ptr)->rows * 6, ((gtkstruct2 *) ptr)->rows * 12);
	gtk_window_set_title(GTK_WINDOW(window), "Workflow Status");
	gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

	image = gtk_image_new_from_file(NULL);
	if (image == NULL) {
		//g_printerr("Could not open \"%s\"\n", filename);
		return NULL;
	}

	pixbuf = NULL;

	/* attach standard event handlers */
	g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);
	g_signal_connect(window, "configure-event", G_CALLBACK(resize_image), (gpointer) image);
	g_signal_connect(window, "state-changed", G_CALLBACK(resize_image2), (gpointer) image);

	gtk_container_add(GTK_CONTAINER(window), image);

	gtk_widget_show_all(GTK_WIDGET(window));

	gtk_main();

	/* Release gtk's global lock */
	gdk_threads_leave();

	return NULL;
}

void *main_loop(void *ptr)
{
	int z, ended = 0;
	gtkstruct *container = (gtkstruct *) ptr;
	int iterations_num = container->iterations_num;
	char *command_line = container->command_line;
	char *tmp_submission_string = container->tmp_submission_string;
	HASHTBL *hashtbl = container->hashtbl;
	int *oph_term_return = container->oph_term_return;
	char *tmp_session = container->tmp_session;
	char *tmp_workflow = container->tmp_workflow;
	int save_img = container->save_img;
	int open_img = container->open_img;
	int show_list = container->show_list;
	int time_interval = container->time_interval;
	short start_gtk = 0;
	GThread *Thread1 = NULL;
	GError *err1 = NULL;
	gtkstruct2 term_size;
	oph_workflow *wf = container->wf;

	if (!gtk_init_check(NULL, NULL))
		open_img = 0;

	char *_user = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_USER);
	char token_user[10], *_token = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
	if (_token && strlen(_token)) {
		strcpy(token_user, OPH_TERM_TOKEN_USER);
		_user = token_user;
	}

	if (wf) {
		oph_workflow_print(wf, save_img, open_img, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_GRAPH_LAYOUT));
		oph_workflow_free(wf);
	} else {
		abort_view = 0;
		for (z = 0; z < iterations_num || iterations_num == 0; z++) {
			// SUBMISSION
			char *response_for_viewer = NULL;
			oph_term_client(command_line, tmp_submission_string, NULL, _user, _passwd, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST),
					(char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), oph_term_return, NULL, &response_for_viewer, 1, hashtbl);

			if (!response_for_viewer) {
				(print_json) ? my_fprintf(stderr, "Could not get response from server [CODE %d]\\n", OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																		       "\e[1;31mCould not get response from server [CODE %d]\e[0m\n",
																		       OPH_TERM_GENERIC_ERROR);
				if (start_gtk) {
					if (!abort_view)
						(print_json) ? my_printf("Close image to continue...\\n") : printf("Close image to continue...\n");
					g_thread_join(Thread1);
					g_object_unref(G_OBJECT(pixbuf));
					pixbuf = NULL;
				}
				return NULL;
			}
			// VIEW STATUS if it is the case
			if (!oph_term_viewer_check_workflow_status(response_for_viewer) && strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "no_op")) {
				if (z == 0 && !strcmp((char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_IMGS), "open")) {
					start_gtk = 1;
					get_term_size(&(term_size.rows), &(term_size.cols));
#ifndef GLIB_VERSION_2_32
					if ((Thread1 = g_thread_create((GThreadFunc) gtk_main_thread, &term_size, TRUE, &err1)) == NULL)
#else
					if ((Thread1 = g_thread_try_new(NULL, (GThreadFunc) gtk_main_thread, &term_size, &err1)) == NULL)
#endif
					{
						//printf("Thread create failed: %s!!\n", err1->message );
						if (response_for_viewer)
							free(response_for_viewer);
						g_error_free(err1);
						return NULL;
					}
				}
				char *full_request = NULL;
				if (oph_term_get_full_request
				    (tmp_session, tmp_workflow, _user, _passwd, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST),
				     (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT), oph_term_return, &full_request, NULL, hashtbl)) {
					(print_json) ? my_fprintf(stderr, "Could not get full request [CODE %d]\\n", OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																		       "\e[1;31mCould not get full request [CODE %d]\e[0m\n",
																		       OPH_TERM_GENERIC_ERROR);
					if (response_for_viewer)
						free(response_for_viewer);
					if (full_request)
						free(full_request);
					if (start_gtk) {
						(print_json) ? my_printf("Close image to continue...\\n") : printf("Close image to continue...\n");
						g_thread_join(Thread1);
						g_object_unref(G_OBJECT(pixbuf));
						pixbuf = NULL;
					}
					return NULL;
				}
				oph_workflow *full_workflow = NULL;
				if (oph_workflow_load(full_request, "", &full_workflow)) {
					(print_json) ? my_fprintf(stderr, "Workflow is NOT a VALID Ophidia Workflow.\\n") : fprintf(stderr,
																    "Workflow is \e[1;31mNOT\e[0m a \e[1;31mVALID\e[0m Ophidia Workflow.\n");
					if (full_request) {
						free(full_request);
						full_request = NULL;
					}
					if (response_for_viewer)
						free(response_for_viewer);
					if (start_gtk) {
						(print_json) ? my_printf("Close image to continue...\\n") : printf("Close image to continue...\n");
						g_thread_join(Thread1);
						g_object_unref(G_OBJECT(pixbuf));
						pixbuf = NULL;
					}
					return NULL;
				}
				if (full_request) {
					free(full_request);
					full_request = NULL;
				}
				if (oph_workflow_indexing(full_workflow->tasks, full_workflow->tasks_num)) {
					(print_json) ? my_fprintf(stderr, "Workflow is NOT a VALID Ophidia Workflow.\\n") : fprintf(stderr,
																    "Workflow is \e[1;31mNOT\e[0m a \e[1;31mVALID\e[0m Ophidia Workflow.\n");
					oph_workflow_free(full_workflow);
					if (response_for_viewer)
						free(response_for_viewer);
					if (start_gtk) {
						(print_json) ? my_printf("Close image to continue...\\n") : printf("Close image to continue...\n");
						g_thread_join(Thread1);
						g_object_unref(G_OBJECT(pixbuf));
						pixbuf = NULL;
					}
					return NULL;
				}
				if (oph_workflow_validate(full_workflow)) {
					(print_json) ? my_fprintf(stderr, "Workflow is NOT a VALID Ophidia Workflow.\\n") : fprintf(stderr,
																    "Workflow is \e[1;31mNOT\e[0m a \e[1;31mVALID\e[0m Ophidia Workflow.\n");
					oph_workflow_free(full_workflow);
					if (response_for_viewer)
						free(response_for_viewer);
					if (start_gtk) {
						(print_json) ? my_printf("Close image to continue...\\n") : printf("Close image to continue...\n");
						g_thread_join(Thread1);
						g_object_unref(G_OBJECT(pixbuf));
						pixbuf = NULL;
					}
					return NULL;
				}
				if (oph_workflow_print_status(full_workflow, save_img, open_img, response_for_viewer, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_GRAPH_LAYOUT))) {
					oph_workflow_free(full_workflow);
					if (response_for_viewer)
						free(response_for_viewer);
					if (start_gtk) {
						(print_json) ? my_printf("Close image to continue...\\n") : printf("Close image to continue...\n");
						g_thread_join(Thread1);
						g_object_unref(G_OBJECT(pixbuf));
						pixbuf = NULL;
					}
					return NULL;
				}
				oph_workflow_free(full_workflow);
			} else
				filename[0] = '\0';

			// VISUALIZATION
			char *newtoken = NULL, *exectime = NULL;
			ended = oph_term_viewer_check_workflow_ended(response_for_viewer);
			if (ended == 0) {
				int viewer_res = oph_term_viewer((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), &response_for_viewer,
								 (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1)) ? ((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1)) : "red", save_img,
								 open_img,
								 show_list, NULL, NULL, NULL, &newtoken, &exectime, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_GRAPH_LAYOUT));
				if (viewer_res != 0 && viewer_res != OPH_TERM_ERROR_WITHIN_JSON) {
					(print_json) ? my_fprintf(stderr, "Could not render result [CODE %d]\\n", OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																		    "\e[1;31mCould not render result [CODE %d]\e[0m\n",
																		    OPH_TERM_GENERIC_ERROR);
					if (newtoken) {
						free(newtoken);
						newtoken = NULL;
					}
					if (exectime) {
						free(exectime);
						exectime = NULL;
					}
					if (start_gtk) {
						(print_json) ? my_printf("Close image to continue...\\n") : printf("Close image to continue...\n");
						g_thread_join(Thread1);
						g_object_unref(G_OBJECT(pixbuf));
						pixbuf = NULL;
					}
					return NULL;
				}
				if (newtoken) {
					pthread_mutex_lock(&global_flag);
					hashtbl_remove(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
					hashtbl_insert(hashtbl, OPH_TERM_ENV_OPH_TOKEN, newtoken, strlen(newtoken) + 1);
					_passwd = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
					pthread_mutex_unlock(&global_flag);
					free(newtoken);
				}
				if (exectime) {
					(print_json) ? my_printf("Execution time: %s\\n", oph_print_exectime(&exectime)) : printf("Execution time: %s\n", oph_print_exectime(&exectime));
					free(exectime);
				}
				break;
			} else {
				if (!start_gtk) {
					int viewer_res = oph_term_viewer((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_VIEWER), &response_for_viewer,
									 (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1)) ? ((const char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_PS1)) : "red",
									 save_img,
									 open_img, show_list, NULL, NULL, NULL, &newtoken, &exectime, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_GRAPH_LAYOUT));
					if (viewer_res != 0 && viewer_res != OPH_TERM_ERROR_WITHIN_JSON) {
						(print_json) ? my_fprintf(stderr, "Could not render result [CODE %d]\\n", OPH_TERM_GENERIC_ERROR) : fprintf(stderr,
																			    "\e[1;31mCould not render result [CODE %d]\e[0m\n",
																			    OPH_TERM_GENERIC_ERROR);
						if (newtoken) {
							free(newtoken);
							newtoken = NULL;
						}
						if (exectime) {
							free(exectime);
							exectime = NULL;
						}
						return NULL;
					}
					if (newtoken) {
						pthread_mutex_lock(&global_flag);
						hashtbl_remove(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
						hashtbl_insert(hashtbl, OPH_TERM_ENV_OPH_TOKEN, newtoken, strlen(newtoken) + 1);
						_passwd = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
						pthread_mutex_unlock(&global_flag);
						free(newtoken);
					}
					if (exectime) {
						(print_json) ? my_printf("Execution time: %s\\n", oph_print_exectime(&exectime)) : printf("Execution time: %s\n", oph_print_exectime(&exectime));
						free(exectime);
					}
				} else if (response_for_viewer) {
					free(response_for_viewer);
					response_for_viewer = NULL;
				}
			}

			if (iterations_num == 0 || z != iterations_num - 1)
				sleep(time_interval);
			if (abort_view)
				break;
		}
	}

	if (start_gtk) {
		(print_json) ? my_printf("Close image to continue...\\n") : printf("Close image to continue...\n");
		g_thread_join(Thread1);
		g_object_unref(G_OBJECT(pixbuf));
		pixbuf = NULL;
	}

	return NULL;
}

int view_status(int iterations_num, char *command_line, char *tmp_submission_string, HASHTBL * hashtbl, int *oph_term_return, char *tmp_session, char *tmp_workflow, int save_img, int open_img,
		int show_list, int time_interval, oph_workflow * wf)
{
	UNUSED(cmds) UNUSED(env_vars)
	    UNUSED(pre_defined_aliases_keys)
	    UNUSED(pre_defined_aliases_values)
	    UNUSED(env_vars_ptr) UNUSED(alias_ptr)
	    UNUSED(xml_defs) UNUSED(operators_list)
	    UNUSED(operators_list_size)

	GThread *Thread2 = NULL;
	GError *err2 = NULL;
	gtkstruct container;
	gtkstruct2 term_size;

	get_term_size(&(term_size.rows), &(term_size.cols));

	container.command_line = command_line;
	container.hashtbl = hashtbl;
	container.iterations_num = iterations_num;
	container.open_img = open_img;
	container.oph_term_return = oph_term_return;
	container.save_img = save_img;
	container.show_list = show_list;
	container.time_interval = time_interval;
	container.tmp_session = tmp_session;
	container.tmp_submission_string = tmp_submission_string;
	container.tmp_workflow = tmp_workflow;
	container.wf = wf;

	if (!g_thread_supported()) {
#ifndef GLIB_VERSION_2_32
		g_type_init();
		g_thread_init(NULL);
#endif
		gdk_threads_init();	// Called to initialize internal mutex "gdk_threads_mutex".
	}
#ifndef GLIB_VERSION_2_32
	if ((Thread2 = g_thread_create((GThreadFunc) main_loop, &container, TRUE, &err2)) == NULL)
#else
	if ((Thread2 = g_thread_try_new(NULL, (GThreadFunc) main_loop, &container, &err2)) == NULL)
#endif
	{
		//printf("Thread create failed: %s!!\n", err2->message );
		g_error_free(err2);
		return OPH_TERM_GENERIC_ERROR;
	}

	g_thread_join(Thread2);

	return OPH_TERM_SUCCESS;
}

// Other internal functions

int gparent_of(oph_workflow * wf, int k)
{
	if (k < 0)
		return -2;
	int p = wf->tasks[k].parent;
	if (p < 0)
		return -2;
	do {
		k = p;
		p = wf->tasks[k].parent;
	}
	while (p >= 0);
	return k;
}

int workflow_s_add(workflow_s_nodes * s, workflow_node * node)
{
	if (!s || !node) {
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}

	if (s->tail) {
		workflow_s_node *s_node = (workflow_s_node *) calloc(1, sizeof(workflow_s_node));
		if (!s_node) {
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
		s_node->node = node;
		s_node->next = NULL;
		s->tail->next = s_node;
		s->tail = s_node;
		s->nodes_num++;
	} else {
		workflow_s_node *s_node = (workflow_s_node *) calloc(1, sizeof(workflow_s_node));
		if (!s_node) {
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
		s_node->node = node;
		s_node->next = NULL;
		s->head = s_node;
		s->tail = s_node;
		s->nodes_num++;
	}

	return OPH_WORKFLOW_EXIT_SUCCESS;
}

int workflow_s_remove(workflow_s_nodes * s, workflow_node ** node)
{
	if (!s || !node) {
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}

	if (s->head) {
		*node = s->head->node;
	} else {
		*node = NULL;
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}

	workflow_s_node *tmp = s->head->next;
	free(s->head);
	s->head = tmp;
	if (tmp == NULL)
		s->tail = NULL;
	s->nodes_num--;

	return OPH_WORKFLOW_EXIT_SUCCESS;
}

int workflow_s_nodes_free(workflow_s_nodes * s)
{
	if (!s)
		return OPH_WORKFLOW_EXIT_SUCCESS;
	workflow_s_node *tmp = NULL;
	workflow_s_node *ptr = s->head;
	while (ptr) {
		tmp = ptr->next;
		free(ptr);
		ptr = tmp;
	}
	s->head = NULL;
	s->tail = NULL;
	s->nodes_num = 0;
	return OPH_WORKFLOW_EXIT_SUCCESS;
}

int workflow_node_free(workflow_node * node)
{
	if (!node)
		return OPH_WORKFLOW_EXIT_SUCCESS;
	if (node->in_edges) {
		free(node->in_edges);
		node->in_edges = NULL;
		node->in_edges_num = 0;
		node->in_edges_size = 0;
	}
	if (node->out_edges) {
		free(node->out_edges);
		node->out_edges = NULL;
		node->out_edges_num = 0;
		node->out_edges_size = 0;
	}
	return OPH_WORKFLOW_EXIT_SUCCESS;
}

int oph_workflow_get_ranks_string(oph_workflow_task * tasks, int tasks_num, char **ranks_string)
{
	if (!tasks || tasks_num < 1 || !ranks_string) {
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}

	(*ranks_string) = NULL;

	*ranks_string = (char *) calloc(tasks_num * OPH_WORKFLOW_RANK_SIZE, sizeof(char));
	if (!(*ranks_string)) {
		return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
	}

	int i, initial_tasks_indexes_num = 0;
	for (i = 0; i < tasks_num; i++)
		if (tasks[i].deps_num < 1)
			initial_tasks_indexes_num++;

	if (initial_tasks_indexes_num == 0) {
		if (*ranks_string) {
			free(*ranks_string);
			*ranks_string = NULL;
		}
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;
	}

	int *initial_tasks_indexes = (int *) calloc(initial_tasks_indexes_num, sizeof(int));
	if (!initial_tasks_indexes) {
		if (*ranks_string) {
			free(*ranks_string);
			*ranks_string = NULL;
		}
		return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
	}

	int j = 0;
	for (i = 0; i < tasks_num; i++) {
		if (tasks[i].deps_num < 1) {
			initial_tasks_indexes[j] = i;
			j++;
			if (j == initial_tasks_indexes_num)
				break;
		}
	}

	int n = 0;

	//Add nodes with rank 0
	n += snprintf((*ranks_string) + n, (OPH_WORKFLOW_RANK_SIZE * tasks_num) - n, "{rank=same");
	for (i = 0; i < initial_tasks_indexes_num; i++) {
		n += snprintf((*ranks_string) + n, (OPH_WORKFLOW_RANK_SIZE * tasks_num) - n, ";%d", initial_tasks_indexes[i]);
	}
	n += snprintf((*ranks_string) + n, (OPH_WORKFLOW_RANK_SIZE * tasks_num) - n, "} ");

	int prev_tasks_num = initial_tasks_indexes_num;
	int *prev_tasks = initial_tasks_indexes;
	initial_tasks_indexes = NULL;
	int cur_tasks_num = 0;
	int *cur_tasks = NULL;
	int k = 0;
	int stop;
	while (prev_tasks_num) {
		k = 0;
		for (i = 0; i < prev_tasks_num; i++)
			cur_tasks_num += tasks[prev_tasks[i]].dependents_indexes_num;
		if (!cur_tasks_num) {
			if (prev_tasks) {
				free(prev_tasks);
				prev_tasks = NULL;
			}
			break;
		}

		cur_tasks = (int *) calloc(cur_tasks_num, sizeof(int));
		if (!cur_tasks) {
			if (*ranks_string) {
				free(*ranks_string);
				*ranks_string = NULL;
			}
			if (initial_tasks_indexes) {
				free(initial_tasks_indexes);
				initial_tasks_indexes = NULL;
			}
			if (prev_tasks) {
				free(prev_tasks);
				prev_tasks = NULL;
			}
			return OPH_WORKFLOW_EXIT_MEMORY_ERROR;
		}
		stop = 0;
		for (i = 0; i < prev_tasks_num; i++) {
			for (j = 0; j < tasks[prev_tasks[i]].dependents_indexes_num; j++) {
				cur_tasks[k] = tasks[prev_tasks[i]].dependents_indexes[j];
				k++;
				if (k == cur_tasks_num) {
					stop = 1;
					break;
				}
			}
			if (stop)
				break;
		}

		//Add nodes with higher rank
		n += snprintf((*ranks_string) + n, (OPH_WORKFLOW_RANK_SIZE * tasks_num) - n, "{rank=same");
		char needle[OPH_TERM_MAX_LEN];
		char needle2[OPH_TERM_MAX_LEN];
		for (i = 0; i < cur_tasks_num; i++) {
			snprintf(needle, OPH_TERM_MAX_LEN, ";%d}", cur_tasks[i]);
			snprintf(needle2, OPH_TERM_MAX_LEN, ";%d;", cur_tasks[i]);
			size_t needle_len = strlen(needle) - 1;
			char *needle_ptr = NULL;
			if ((needle_ptr = strstr(*ranks_string, needle)) || (needle_ptr = strstr(*ranks_string, needle2))) {
				char buffer[OPH_TERM_MAX_LEN];
				snprintf(buffer, OPH_TERM_MAX_LEN, "%s", needle_ptr + needle_len);
				n -= (strlen(buffer) + needle_len);
				n += snprintf((*ranks_string) + n, (OPH_WORKFLOW_RANK_SIZE * tasks_num) - n, "%s", buffer);
			}
			n += snprintf((*ranks_string) + n, (OPH_WORKFLOW_RANK_SIZE * tasks_num) - n, ";%d", cur_tasks[i]);
		}
		n += snprintf((*ranks_string) + n, (OPH_WORKFLOW_RANK_SIZE * tasks_num) - n, "} ");

		if (prev_tasks)
			free(prev_tasks);
		prev_tasks = cur_tasks;
		prev_tasks_num = cur_tasks_num;
		cur_tasks = NULL;
		cur_tasks_num = 0;
	}

	if (initial_tasks_indexes) {
		free(initial_tasks_indexes);
		initial_tasks_indexes = NULL;
	}

	return OPH_WORKFLOW_EXIT_SUCCESS;
}

int workflow_is_child_of(oph_workflow * wf, int p, int c)
{
	if (!wf || (p < 0) || (p >= wf->tasks_num) || (c < 0) || (c >= wf->tasks_num))
		return 0;
	if (p == c)
		return 1;
	int i, j;
	for (i = 0; i < wf->tasks[p].dependents_indexes_num; ++i) {
		j = wf->tasks[p].dependents_indexes[i];
		if (workflow_is_child_of(wf, j, c))
			return 1;
	}
	return 0;
}

unsigned int workflow_number_of(oph_workflow * wf, int k, int p, int gp, const char *op, const char *nop, char *flag, int bracket_number, int *child)
{
	if (!wf || (k < 0) || (k >= wf->tasks_num))
		return 0;
	int i, j, res = 0, bn;
	for (i = 0; i < wf->tasks[k].dependents_indexes_num; ++i) {
		j = wf->tasks[k].dependents_indexes[i];
		if (!strncasecmp(wf->tasks[j].operator, op, OPH_WORKFLOW_MAX_STRING))	// Found an "end-task"
		{
			if (bracket_number)
				res += workflow_number_of(wf, j, p, gp, op, nop, flag, bracket_number - 1, child);
			else if (flag[j]) {
				res++;
				flag[j] = 0;	// Mark this task in order to avoid to count it more times
				if ((wf->tasks[j].parent < 0) || (wf->tasks[j].parent == p)) {
					wf->tasks[j].parent = p;
					if (child)
						*child = j;
				} else if ((wf->tasks[j].parent != p) && (wf->tasks[j].parent != gp))
					res++;	// Performance improvement
			}
		} else {
			bn = bracket_number;
			char tmp[1 + strlen(nop)], check = 0;
			strcpy(tmp, nop);
			char *save_pointer = NULL, *pch = strtok_r(tmp, "|", &save_pointer);
			while (pch) {
				if (!strncasecmp(wf->tasks[j].operator, pch, OPH_WORKFLOW_MAX_STRING)) {
					check = 1;
					break;
				}
				pch = strtok_r(NULL, "|", &save_pointer);
			}
			if (check)
				bn++;
			res += workflow_number_of(wf, j, p, gp, op, nop, flag, bn, child);
		}
		if (res > 1)
			break;	// Performance improvement
	}
	return res;
}

int workflow_validate_fco(oph_workflow * wf)
{
	if (!wf)
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;

	int i, k, kk, child;
	char flag[wf->tasks_num];
	unsigned int number;

	for (k = 0; k < wf->tasks_num; k++)
		wf->tasks[k].parent = -1;

	for (k = 0; k < wf->tasks_num; k++) {
		if (!strncasecmp(wf->tasks[k].operator, OPH_OPERATOR_FOR, OPH_WORKFLOW_MAX_STRING)) {
			for (i = 0; i < wf->tasks_num; ++i)
				flag[i] = 1;
			number = workflow_number_of(wf, k, k, k, OPH_OPERATOR_ENDFOR, OPH_OPERATOR_FOR, flag, 0, &child);
			if (!number || (number > 1))
				break;
			for (i = 0; i < wf->tasks_num; ++i)
				if ((wf->tasks[i].parent == k) && strncasecmp(wf->tasks[k].operator, OPH_OPERATOR_ENDFOR, OPH_WORKFLOW_MAX_STRING) && !workflow_is_child_of(wf, i, child))
					break;
			if (i < wf->tasks_num)
				break;
		} else if (!strncasecmp(wf->tasks[k].operator, OPH_OPERATOR_IF, OPH_WORKFLOW_MAX_STRING)) {
			for (i = 0; i < wf->tasks_num; ++i)
				flag[i] = 1;
			child = -1;
			number = workflow_number_of(wf, k, k, k, OPH_OPERATOR_ELSEIF, OPH_OPERATOR_IF, flag, 0, &child);
			if (number > 1)
				break;
			if (child >= 0) {
				for (i = 0; i < wf->tasks_num; ++i)
					if ((wf->tasks[i].parent == k) && strncasecmp(wf->tasks[k].operator, OPH_OPERATOR_ELSEIF, OPH_WORKFLOW_MAX_STRING) && !workflow_is_child_of(wf, i, child))
						break;
				if (i < wf->tasks_num)
					break;
			} else {
				for (i = 0; i < wf->tasks_num; ++i)
					flag[i] = 1;
				child = -1;
				number = workflow_number_of(wf, k, k, k, OPH_OPERATOR_ELSE, OPH_OPERATOR_IF, flag, 0, &child);
				if (number > 1)
					break;
				if (child >= 0) {
					for (i = 0; i < wf->tasks_num; ++i)
						if ((wf->tasks[i].parent == k) && strncasecmp(wf->tasks[k].operator, OPH_OPERATOR_ELSE, OPH_WORKFLOW_MAX_STRING) && !workflow_is_child_of(wf, i, child))
							break;
					if (i < wf->tasks_num)
						break;
				}
			}
			for (i = 0; i < wf->tasks_num; ++i)
				flag[i] = 1;
			number = workflow_number_of(wf, k, k, k, OPH_OPERATOR_ENDIF, OPH_OPERATOR_IF, flag, 0, &child);
			if (!number && (number > 1))
				break;
			for (i = 0; i < wf->tasks_num; ++i)
				if ((wf->tasks[i].parent == k) && strncasecmp(wf->tasks[k].operator, OPH_OPERATOR_ENDIF, OPH_WORKFLOW_MAX_STRING) && !workflow_is_child_of(wf, i, child))
					break;
			if (i < wf->tasks_num)
				break;
		} else if (!strncasecmp(wf->tasks[k].operator, OPH_OPERATOR_ELSEIF, OPH_WORKFLOW_MAX_STRING)) {
			kk = gparent_of(wf, k);
			for (i = 0; i < wf->tasks_num; ++i)
				flag[i] = 1;
			child = -1;
			number = workflow_number_of(wf, k, k, kk, OPH_OPERATOR_ELSEIF, OPH_OPERATOR_IF, flag, 0, &child);
			if (number > 1)
				break;
			if (child >= 0) {
				for (i = 0; i < wf->tasks_num; ++i)
					if ((wf->tasks[i].parent == k) && strncasecmp(wf->tasks[k].operator, OPH_OPERATOR_ELSEIF, OPH_WORKFLOW_MAX_STRING) && !workflow_is_child_of(wf, i, child))
						break;
				if (i < wf->tasks_num)
					break;
			} else {
				for (i = 0; i < wf->tasks_num; ++i)
					flag[i] = 1;
				child = -1;
				number = workflow_number_of(wf, k, k, kk, OPH_OPERATOR_ELSE, OPH_OPERATOR_IF, flag, 0, &child);
				if (number > 1)
					break;
				if (child >= 0) {
					for (i = 0; i < wf->tasks_num; ++i)
						if ((wf->tasks[i].parent == k) && strncasecmp(wf->tasks[k].operator, OPH_OPERATOR_ELSE, OPH_WORKFLOW_MAX_STRING) && !workflow_is_child_of(wf, i, child))
							break;
					if (i < wf->tasks_num)
						break;
				} else {
					for (i = 0; i < wf->tasks_num; ++i)
						flag[i] = 1;
					number = workflow_number_of(wf, k, k, kk, OPH_OPERATOR_ENDIF, OPH_OPERATOR_IF, flag, 0, &child);
					if (!number || (number > 1))
						break;
					for (i = 0; i < wf->tasks_num; ++i)
						if ((wf->tasks[i].parent == k) && strncasecmp(wf->tasks[k].operator, OPH_OPERATOR_ENDIF, OPH_WORKFLOW_MAX_STRING)
						    && !workflow_is_child_of(wf, i, child))
							break;
					if (i < wf->tasks_num)
						break;
				}
			}
		} else if (!strncasecmp(wf->tasks[k].operator, OPH_OPERATOR_ELSE, OPH_WORKFLOW_MAX_STRING)) {
			kk = gparent_of(wf, k);
			for (i = 0; i < wf->tasks_num; ++i)
				flag[i] = 1;
			number = workflow_number_of(wf, k, k, kk, OPH_OPERATOR_ENDIF, OPH_OPERATOR_IF, flag, 0, &child);
			if (!number || (number > 1))
				break;
			for (i = 0; i < wf->tasks_num; ++i)
				if ((wf->tasks[i].parent == k) && strncasecmp(wf->tasks[k].operator, OPH_OPERATOR_ENDIF, OPH_WORKFLOW_MAX_STRING) && !workflow_is_child_of(wf, i, child))
					break;
			if (i < wf->tasks_num)
				break;
		}
	}
	if (k < wf->tasks_num)
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;

	for (k = 0; k < wf->tasks_num; k++)
		if (wf->tasks[k].parent < 0) {
			if (!strncasecmp(wf->tasks[k].operator, OPH_OPERATOR_ENDFOR, OPH_WORKFLOW_MAX_STRING))
				break;
			else if (!strncasecmp(wf->tasks[k].operator, OPH_OPERATOR_ELSEIF, OPH_WORKFLOW_MAX_STRING))
				break;
			else if (!strncasecmp(wf->tasks[k].operator, OPH_OPERATOR_ELSE, OPH_WORKFLOW_MAX_STRING))
				break;
			else if (!strncasecmp(wf->tasks[k].operator, OPH_OPERATOR_ENDIF, OPH_WORKFLOW_MAX_STRING))
				break;
		}
	if (k < wf->tasks_num)
		return OPH_WORKFLOW_EXIT_BAD_PARAM_ERROR;

	return OPH_WORKFLOW_EXIT_SUCCESS;
}
