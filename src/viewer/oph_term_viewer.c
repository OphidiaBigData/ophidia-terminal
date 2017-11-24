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

#include "oph_term_viewer.h"
#include "oph_json/oph_json_library.h"

#include <sys/ioctl.h>
#include <gvc.h>

#define OPH_TERM_VIEWER_SIGN_LEN 20
#define OPH_TERM_VIEWER_DOT_MAX_LEN 50*1024
#define OPH_TERM_VIEWER_MINUS_SIGN "-"
#define OPH_TERM_VIEWER_PLUS_SIGN "+"
#define OPH_TERM_VIEWER_PIPE_SIGN "|"
#define OPH_TERM_VIEWER_EQUAL_SIGN "="
#define OPH_TERM_VIEWER_MACRON_SIGN "¯"
#define OPH_TERM_VIEWER_UPPER_LEFT_SIGN "⌜"
#define OPH_TERM_VIEWER_UPPER_RIGHT_SIGN "⌝"
#define OPH_TERM_VIEWER_LOWER_LEFT_SIGN "⌞"
#define OPH_TERM_VIEWER_LOWER_RIGHT_SIGN "⌟"
#define OPH_TERM_VIEWER_SESSION_CODE_STRING "Session Code"
#define OPH_TERM_VIEWER_MARKERID_STRING "Marker"

#define OPH_TERM_VIEWER_OPENED_BRAKE '{'
#define OPH_TERM_VIEWER_CLOSED_BRAKE '}'

#define OPH_TERM_VIEWER_TITLE_FOR_CUBE 	"Output Cube"
#define OPH_TERM_VIEWER_TITLE_FOR_CWD 	"Current Working Directory"
#define OPH_TERM_VIEWER_TITLE_FOR_CDD 	"Current Data Directory"

#define OPH_TERM_TOKEN_JSON	"access_token"
#define OPH_TERM_EXEC_TIME "execution_time"

int oph_viewer_get_ranks_string(oph_json_links * nodelinks, unsigned int nodelinks_num, char **ranks_string)
{
	if (!nodelinks || nodelinks_num < 1 || !ranks_string) {
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	(*ranks_string) = NULL;

	*ranks_string = (char *) calloc(nodelinks_num, 1000);
	if (!(*ranks_string)) {
		return OPH_TERM_MEMORY_ERROR;
	}

	unsigned int i, j, q, initial_nodes_num = 0;
	int found;
	for (q = 0; q < nodelinks_num; q++) {
		found = 0;
		for (i = 0; i < nodelinks_num; i++) {
			for (j = 0; j < nodelinks[i].links_num; j++) {
				if (strtol(nodelinks[i].links[j].node, NULL, 10) == q) {
					found = 1;
					break;
				}
			}
			if (found)
				break;
		}
		if (!found)
			initial_nodes_num++;
	}

	if (initial_nodes_num == 0) {
		if (*ranks_string) {
			free(*ranks_string);
			*ranks_string = NULL;
		}
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	int *initial_nodes = (int *) calloc(initial_nodes_num, sizeof(int));
	if (!initial_nodes) {
		if (*ranks_string) {
			free(*ranks_string);
			*ranks_string = NULL;
		}
		return OPH_TERM_MEMORY_ERROR;
	}

	unsigned int z = 0;
	for (q = 0; q < nodelinks_num; q++) {
		found = 0;
		for (i = 0; i < nodelinks_num; i++) {
			for (j = 0; j < nodelinks[i].links_num; j++) {
				if (strtol(nodelinks[i].links[j].node, NULL, 10) == q) {
					found = 1;
					break;
				}
			}
			if (found)
				break;
		}
		if (!found) {
			initial_nodes[z] = q;
			z++;
			if (z == initial_nodes_num)
				break;
		}
	}

	int n = 0;

	//Add nodes with rank 0
	n += snprintf((*ranks_string) + n, (1000 * nodelinks_num) - n, "{rank=same");
	for (i = 0; i < initial_nodes_num; i++) {
		n += snprintf((*ranks_string) + n, (1000 * nodelinks_num) - n, ";%d", initial_nodes[i]);
	}
	n += snprintf((*ranks_string) + n, (1000 * nodelinks_num) - n, "} ");

	unsigned int prev_nodes_num = initial_nodes_num;
	int *prev_nodes = initial_nodes;
	initial_nodes = NULL;
	unsigned int cur_nodes_num = 0;
	int *cur_nodes = NULL;
	unsigned int k = 0;
	int stop;
	while (prev_nodes_num) {
		k = 0;
		for (i = 0; i < prev_nodes_num; i++)
			cur_nodes_num += nodelinks[prev_nodes[i]].links_num;
		if (!cur_nodes_num) {
			if (prev_nodes) {
				free(prev_nodes);
				prev_nodes = NULL;
			}
			break;
		}

		cur_nodes = (int *) calloc(cur_nodes_num, sizeof(int));
		if (!cur_nodes) {
			if (*ranks_string) {
				free(*ranks_string);
				*ranks_string = NULL;
			}
			if (initial_nodes) {
				free(initial_nodes);
				initial_nodes = NULL;
			}
			if (prev_nodes) {
				free(prev_nodes);
				prev_nodes = NULL;
			}
			return OPH_TERM_MEMORY_ERROR;
		}
		stop = 0;
		for (i = 0; i < prev_nodes_num; i++) {
			for (j = 0; j < nodelinks[prev_nodes[i]].links_num; j++) {
				cur_nodes[k] = (int) strtol(nodelinks[prev_nodes[i]].links[j].node, NULL, 10);
				k++;
				if (k == cur_nodes_num) {
					stop = 1;
					break;
				}
			}
			if (stop)
				break;
		}

		//Add nodes with higher rank
		n += snprintf((*ranks_string) + n, (1000 * nodelinks_num) - n, "{rank=same");
		char needle[1024];
		char needle2[1024];
		for (i = 0; i < cur_nodes_num; i++) {
			snprintf(needle, 1024, ";%d}", cur_nodes[i]);
			snprintf(needle2, 1024, ";%d;", cur_nodes[i]);
			size_t needle_len = strlen(needle) - 1;
			char *needle_ptr = NULL;
			if ((needle_ptr = strstr(*ranks_string, needle)) || (needle_ptr = strstr(*ranks_string, needle2))) {
				char buffer[1024];
				snprintf(buffer, 1024, "%s", needle_ptr + needle_len);
				n -= (strlen(buffer) + needle_len);
				n += snprintf((*ranks_string) + n, (1000 * nodelinks_num) - n, "%s", buffer);
			}
			n += snprintf((*ranks_string) + n, (1000 * nodelinks_num) - n, ";%d", cur_nodes[i]);
		}
		n += snprintf((*ranks_string) + n, (1000 * nodelinks_num) - n, "} ");

		if (prev_nodes)
			free(prev_nodes);
		prev_nodes = cur_nodes;
		prev_nodes_num = cur_nodes_num;
		cur_nodes = NULL;
		cur_nodes_num = 0;
	}

	if (initial_nodes) {
		free(initial_nodes);
		initial_nodes = NULL;
	}

	return OPH_TERM_SUCCESS;
}

void printnstr(const char *token, int ntimes)
{
	if (!token || ntimes < 0)
		return;
	size_t i;
	for (i = 0; i < (size_t) ntimes; i++) {
		printf("%s", token);
	}
	fflush(stdout);
	return;
}

void get_term_size(int *rows, int *cols)
{
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	if (rows)
		*rows = w.ws_row;
	if (cols)
		*cols = w.ws_col;
	return;
}

void adjust_string(char **string)
{
	if (!string || !*string)
		return;
	size_t i;
	for (i = 0; i < strlen(*string); i++) {
		if ((*string)[i] == '\t') {
			char *tmpbuf = (char *) calloc(strlen(*string) + 1 + 3, sizeof(char));
			if (!tmpbuf) {
				(*string)[i] = ' ';
			} else {
				size_t j;
				for (j = 0; j < i; j++) {
					tmpbuf[j] = (*string)[j];
				}
				tmpbuf[i] = ' ';
				tmpbuf[i + 1] = ' ';
				tmpbuf[i + 2] = ' ';
				tmpbuf[i + 3] = ' ';
				for (j = i + 1; j <= strlen(*string); j++) {
					tmpbuf[j + 3] = (*string)[j];
				}
				free(*string);
				*string = tmpbuf;
			}
		} else if ((*string)[i] == '\v' || (*string)[i] == '\r' || (*string)[i] == '\f' || (*string)[i] == '\b' || (*string)[i] == '\a') {
			(*string)[i] = ' ';
		}
	}
	return;
}

void adjust_string_array(char ***string_array, char **input_strings, int input_strings_num)
{
	if (!string_array || !input_strings || input_strings_num < 1)
		return;
	size_t i;
	for (i = 0; i < (size_t) input_strings_num; i++)
		if (!input_strings[i])
			return;

	*string_array = (char **) calloc(input_strings_num, sizeof(char *));
	if (!*string_array)
		return;

	for (i = 0; i < (size_t) input_strings_num; i++) {
		(*string_array)[i] = (char *) strdup(input_strings[i]);
		if (!(*string_array)[i]) {
			size_t j;
			for (j = 0; j < i; j++) {
				if ((*string_array)[j]) {
					free((*string_array)[j]);
					(*string_array)[j] = NULL;
				}
			}
			free(*string_array);
			*string_array = NULL;
			return;
		}
	}

	for (i = 0; i < (size_t) input_strings_num; i++) {
		adjust_string(&((*string_array)[i]));
	}

	return;
}

int adjusted_string_max_len(char *not_adjusted_string)
{
	if (!not_adjusted_string)
		return 0;
	int max_len = 0;
	int cur_len = 0;
	size_t i;
	for (i = 0; i < strlen(not_adjusted_string); i++) {
		if (not_adjusted_string[i] == '\t') {
			cur_len += 4;
		} else if (not_adjusted_string[i] == '\n') {
			max_len = (cur_len > max_len) ? cur_len : max_len;
			cur_len = 0;
		} else {
			cur_len++;
		}
	}
	max_len = (cur_len > max_len) ? cur_len : max_len;
	return max_len;
}

void print_text(oph_json_obj_text * obj, const char *color_string)
{
	// print title
	if (obj->title && strlen(obj->title)) {
		printf(color_string, obj->title);
		printf("\n");
		char buf[OPH_TERM_VIEWER_SIGN_LEN];
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_MACRON_SIGN);
		printnstr(buf, strlen(obj->title));
		printf("\n");
		fflush(stdout);
	}
	// print message
	if (obj->message && strlen(obj->message)) {
		printf("%s\n", obj->message);
		fflush(stdout);
	}

	return;
}

void print_grid(oph_json_obj_grid * obj, const char *color_string, int show_list)
{

	// print title
	if (obj->title && strlen(obj->title)) {

		if (!show_list && (!strcasecmp(obj->title, OPH_TERM_VIEWER_WORKFLOW_TASK_LIST) || !strcasecmp(obj->title, OPH_TERM_VIEWER_WORKFLOW_TASK_LIST2)))
			return;

		printf(color_string, obj->title);
		printf("\n");
		char buf[OPH_TERM_VIEWER_SIGN_LEN];
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_MACRON_SIGN);
		printnstr(buf, strlen(obj->title));
		printf("\n");
		fflush(stdout);
	}
	// print description
	if (obj->description && strlen(obj->description)) {
		printf("%s\n", obj->description);
		fflush(stdout);
	}
	// compute max length for each column
	size_t i, j;
	int *max_lengths = (int *) calloc(obj->keys_num, sizeof(int));
	if (!max_lengths)
		return;
	for (j = 0; j < obj->keys_num; j++)
		max_lengths[j] = adjusted_string_max_len(obj->keys[j]);
	for (i = 0; i < obj->values_num1; i++) {
		for (j = 0; j < obj->keys_num; j++) {
			int tmp = adjusted_string_max_len(obj->values[i][j]);
			max_lengths[j] = (max_lengths[j] < tmp) ? tmp : max_lengths[j];
		}
	}

	// compute real lengths according to term window width
	int term_cols;
	get_term_size(NULL, &term_cols);
	int available_space = term_cols - ((obj->keys_num - 1) * 3 + 4);
	available_space = (available_space < 0) ? 0 : available_space;
	int total_space = 0;
	for (j = 0; j < obj->keys_num; j++)
		total_space += max_lengths[j];
	int *real_lengths = (int *) calloc(obj->keys_num, sizeof(int));
	if (!real_lengths) {
		free(max_lengths);
		return;
	}
	for (j = 0; j < obj->keys_num; j++) {
		if (total_space > available_space)
			real_lengths[j] = (int) ((max_lengths[j] / (float) total_space) * available_space);
		else
			real_lengths[j] = max_lengths[j];
	}

	// prepare useful chars for table printing
	char plus_sign[OPH_TERM_VIEWER_SIGN_LEN];
	snprintf(plus_sign, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_PLUS_SIGN);
	char pipe_sign[OPH_TERM_VIEWER_SIGN_LEN];
	snprintf(pipe_sign, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_PIPE_SIGN);
	char minus_sign[OPH_TERM_VIEWER_SIGN_LEN];
	snprintf(minus_sign, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_MINUS_SIGN);
	char equal_sign[OPH_TERM_VIEWER_SIGN_LEN];
	snprintf(equal_sign, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_EQUAL_SIGN);

	// PRINT HEADERS

	// print header start
	for (j = 0; j < obj->keys_num; j++) {
		printf("%s%s", plus_sign, equal_sign);
		printnstr(equal_sign, real_lengths[j] + 1);
	}
	printf("%s\n", plus_sign);

	// print header contents
	char **adjusted_header = NULL;
	adjust_string_array(&adjusted_header, obj->keys, obj->keys_num);
	if (!adjusted_header) {
		free(max_lengths);
		free(real_lengths);
		return;
	}
	int *cursors = (int *) calloc(obj->keys_num, sizeof(int));
	if (!cursors) {
		free(max_lengths);
		free(real_lengths);
		size_t k;
		for (k = 0; k < obj->keys_num; k++)
			free(adjusted_header[k]);
		free(adjusted_header);
		return;
	}
	int completed = 0;
	int newline_found = 0;
	char single_char[2];
	for (i = 0; i < obj->values_num2; i++)
		if (strlen(adjusted_header[i]) == 0 || real_lengths[i] == 0)
			completed++;
	while (completed < (int) obj->keys_num) {
		printf("%s", pipe_sign);
		for (i = 0; i < obj->keys_num; i++) {
			printf(" ");
			for (j = 0; j < (size_t) real_lengths[i]; j++) {
				if ((strlen(adjusted_header[i]) - cursors[i]) > 0) {
					if (adjusted_header[i][cursors[i]] == '\n') {
						printf(" ");
						newline_found = 1;
					} else {
						snprintf(single_char, 2, "%c", adjusted_header[i][cursors[i]]);
						printf(color_string, single_char);
						cursors[i]++;
						if (cursors[i] == (int) strlen(adjusted_header[i]))
							completed++;
					}
				} else {
					printf(" ");
				}
			}
			if (newline_found) {
				cursors[i]++;
				if (cursors[i] == (int) strlen(adjusted_header[i]))
					completed++;
				newline_found = 0;
			}
			printf(" %s", pipe_sign);
		}
		printf("\n");
	}
	for (j = 0; j < obj->keys_num; j++)
		free(adjusted_header[j]);
	free(adjusted_header);
	free(cursors);

	// print header end
	for (j = 0; j < obj->keys_num; j++) {
		printf("%s%s", plus_sign, equal_sign);
		printnstr(equal_sign, real_lengths[j] + 1);
	}
	printf("%s\n", plus_sign);
	fflush(stdout);

	// PRINT ROWS

	// print rows contents
	size_t r;
	for (r = 0; r < obj->values_num1; r++) {
		char **adjusted_row = NULL;
		adjust_string_array(&adjusted_row, obj->values[r], obj->values_num2);
		if (!adjusted_row) {
			free(max_lengths);
			free(real_lengths);
			return;
		}
		int *cursors = (int *) calloc(obj->values_num2, sizeof(int));
		if (!cursors) {
			free(max_lengths);
			free(real_lengths);
			size_t k;
			for (k = 0; k < obj->values_num2; k++)
				free(adjusted_row[k]);
			free(adjusted_row);
			return;
		}
		int completed = 0;
		int newline_found = 0;
		for (i = 0; i < obj->values_num2; i++)
			if (strlen(adjusted_row[i]) == 0 || real_lengths[i] == 0)
				completed++;
		while (completed < (int) obj->values_num2) {
			printf("%s", pipe_sign);
			for (i = 0; i < obj->values_num2; i++) {
				printf(" ");
				for (j = 0; j < (size_t) real_lengths[i]; j++) {
					if ((strlen(adjusted_row[i]) - cursors[i]) > 0) {
						if (adjusted_row[i][cursors[i]] == '\n') {
							printf(" ");
							newline_found = 1;
						} else {
							printf("%c", adjusted_row[i][cursors[i]]);
							cursors[i]++;
							if (cursors[i] == (int) strlen(adjusted_row[i]))
								completed++;
						}
					} else {
						printf(" ");
					}
				}
				if (newline_found) {
					cursors[i]++;
					if (cursors[i] == (int) strlen(adjusted_row[i]))
						completed++;
					newline_found = 0;
				}
				printf(" %s", pipe_sign);
			}
			printf("\n");
		}
		//row separator
		if (r != (obj->values_num1 - 1)) {
			printf("%s", pipe_sign);
			for (i = 0; i < obj->values_num2; i++) {
				printnstr(minus_sign, real_lengths[i] + 2);
				printf("%s", pipe_sign);
			}
			printf("\n");
		}
		//row cleanup
		for (i = 0; i < obj->values_num2; i++)
			free(adjusted_row[i]);
		free(adjusted_row);
		free(cursors);
	}

	// print table end
	for (j = 0; j < obj->keys_num; j++) {
		printf("%s%s", plus_sign, equal_sign);
		printnstr(equal_sign, real_lengths[j] + 1);
	}
	printf("%s\n", plus_sign);
	fflush(stdout);

	free(max_lengths);
	free(real_lengths);

	return;
}

void print_multigrid(oph_json_obj_multigrid * obj, const char *color_string)
{
	// print title
	if (obj->title && strlen(obj->title)) {
		printf(color_string, obj->title);
		printf("\n");
		char buf[OPH_TERM_VIEWER_SIGN_LEN];
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_MACRON_SIGN);
		printnstr(buf, strlen(obj->title));
		printf("\n");
		fflush(stdout);
	}
	// print description
	if (obj->description && strlen(obj->description)) {
		printf("%s\n", obj->description);
		fflush(stdout);
	}


	size_t i, j;
	// compute max length for each column of rowkeys
	int *max_lengths_rowkeys = (int *) calloc(obj->rowkeys_num, sizeof(int));
	if (!max_lengths_rowkeys)
		return;
	for (j = 0; j < obj->rowkeys_num; j++)
		max_lengths_rowkeys[j] = adjusted_string_max_len(obj->rowkeys[j]);
	for (i = 0; i < obj->rowvalues_num1; i++) {
		for (j = 0; j < obj->rowkeys_num; j++) {
			max_lengths_rowkeys[j] = (max_lengths_rowkeys[j] < adjusted_string_max_len(obj->rowvalues[i][j])) ? adjusted_string_max_len(obj->rowvalues[i][j]) : max_lengths_rowkeys[j];
		}
	}
	// compute length for implicit dimension headers
	int imp_dim_header_length = (obj->rowkeys_num - 1) * 3;
	for (j = 0; j < obj->rowkeys_num; j++)
		imp_dim_header_length += max_lengths_rowkeys[j];
	// compute max length for each column of measurevalues/colvalues
	int *max_lengths_values = (int *) calloc(obj->measurevalues_num2, sizeof(int));
	if (!max_lengths_values) {
		if (max_lengths_rowkeys)
			free(max_lengths_rowkeys);
		return;
	}
	for (i = 0; i < obj->colkeys_num; i++) {
		for (j = 0; j < obj->measurevalues_num2; j++) {
			max_lengths_values[j] = (max_lengths_values[j] < adjusted_string_max_len(obj->colvalues[j][i])) ? adjusted_string_max_len(obj->colvalues[j][i]) : max_lengths_values[j];
		}
	}
	for (i = 0; i < obj->measurevalues_num1; i++) {
		for (j = 0; j < obj->measurevalues_num2; j++) {
			max_lengths_values[j] = (max_lengths_values[j] < adjusted_string_max_len(obj->measurevalues[i][j])) ? adjusted_string_max_len(obj->measurevalues[i][j]) : max_lengths_values[j];
		}
	}
	// compute length for measure header
	int measure_header_length = (obj->measurevalues_num2 - 1) * 3;
	for (j = 0; j < obj->measurevalues_num2; j++)
		measure_header_length += max_lengths_values[j];


	// compute REAL lengths according to term window width
	int term_cols;
	get_term_size(NULL, &term_cols);
	int available_space = term_cols - ((obj->rowkeys_num + obj->measurevalues_num2 - 1) * 3 + 4);
	available_space = (available_space < 0) ? 0 : available_space;
	int total_space = 0;
	for (j = 0; j < obj->rowkeys_num; j++)
		total_space += max_lengths_rowkeys[j];
	for (j = 0; j < obj->measurevalues_num2; j++)
		total_space += max_lengths_values[j];
	int *real_lengths_rowkeys = (int *) calloc(obj->rowkeys_num, sizeof(int));
	if (!real_lengths_rowkeys) {
		free(max_lengths_rowkeys);
		free(max_lengths_values);
		return;
	}
	for (j = 0; j < obj->rowkeys_num; j++) {
		if (total_space > available_space)
			real_lengths_rowkeys[j] = (int) ((max_lengths_rowkeys[j] / (float) total_space) * available_space);
		else
			real_lengths_rowkeys[j] = max_lengths_rowkeys[j];
	}
	int *real_lengths_values = (int *) calloc(obj->measurevalues_num2, sizeof(int));
	if (!real_lengths_values) {
		free(max_lengths_rowkeys);
		free(max_lengths_values);
		free(real_lengths_rowkeys);
		return;
	}
	for (j = 0; j < obj->measurevalues_num2; j++) {
		if (total_space > available_space)
			real_lengths_values[j] = (int) ((max_lengths_values[j] / (float) total_space) * available_space);
		else
			real_lengths_values[j] = max_lengths_values[j];
	}
	// compute real length for implicit dimension headers
	int real_imp_dim_header_length = (obj->rowkeys_num - 1) * 3;
	for (j = 0; j < obj->rowkeys_num; j++)
		real_imp_dim_header_length += real_lengths_rowkeys[j];
	// compute real length for measure header
	int real_measure_header_length = (obj->measurevalues_num2 - 1) * 3;
	for (j = 0; j < obj->measurevalues_num2; j++)
		real_measure_header_length += real_lengths_values[j];


	// prepare useful chars for table printing
	char plus_sign[OPH_TERM_VIEWER_SIGN_LEN];
	snprintf(plus_sign, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_PLUS_SIGN);
	char pipe_sign[OPH_TERM_VIEWER_SIGN_LEN];
	snprintf(pipe_sign, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_PIPE_SIGN);
	char minus_sign[OPH_TERM_VIEWER_SIGN_LEN];
	snprintf(minus_sign, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_MINUS_SIGN);
	char equal_sign[OPH_TERM_VIEWER_SIGN_LEN];
	snprintf(equal_sign, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_EQUAL_SIGN);

	// PRINT IMP DIMENSIONS

	// print start
	printf("%s%s", plus_sign, equal_sign);
	printnstr(equal_sign, real_imp_dim_header_length + 1);
	for (j = 0; j < obj->measurevalues_num2; j++) {
		printf("%s%s", plus_sign, equal_sign);
		printnstr(equal_sign, real_lengths_values[j] + 1);
	}
	printf("%s\n", plus_sign);

	// print contents
	for (i = 0; i < obj->colkeys_num; i++) {
		char **tmprow = (char **) calloc(obj->measurevalues_num2 + 1, sizeof(char *));
		if (!tmprow) {
			free(max_lengths_rowkeys);
			free(max_lengths_values);
			free(real_lengths_rowkeys);
			free(real_lengths_values);
			return;
		}
		tmprow[0] = obj->colkeys[i];
		for (j = 0; j < obj->measurevalues_num2; j++)
			tmprow[j + 1] = obj->colvalues[j][i];

		char **adjusted_row = NULL;
		adjust_string_array(&adjusted_row, tmprow, obj->measurevalues_num2 + 1);
		if (!adjusted_row) {
			free(max_lengths_rowkeys);
			free(max_lengths_values);
			free(real_lengths_rowkeys);
			free(real_lengths_values);
			free(tmprow);
			return;
		}
		int *cursors = (int *) calloc(obj->measurevalues_num2 + 1, sizeof(int));
		if (!cursors) {
			free(max_lengths_rowkeys);
			free(max_lengths_values);
			free(real_lengths_rowkeys);
			free(real_lengths_values);
			free(tmprow);
			size_t k;
			for (k = 0; k < obj->measurevalues_num2 + 1; k++)
				free(adjusted_row[k]);
			free(adjusted_row);
			return;
		}
		int completed = 0;
		int newline_found = 0;
		char single_char[2];
		for (j = 0; j < obj->measurevalues_num2; j++)
			if (strlen(adjusted_row[j + 1]) == 0 || real_lengths_values[j + 1] == 0)
				completed++;
		if (strlen(adjusted_row[0]) == 0 || real_imp_dim_header_length == 0)
			completed++;
		while (completed < (int) obj->measurevalues_num2 + 1) {
			printf("%s ", pipe_sign);
			for (j = 0; j < (size_t) real_imp_dim_header_length; j++) {
				if ((strlen(adjusted_row[0]) - cursors[0]) > 0) {
					if (adjusted_row[0][cursors[0]] == '\n') {
						printf(" ");
						newline_found = 1;
					} else {
						snprintf(single_char, 2, "%c", adjusted_row[0][cursors[0]]);
						printf(color_string, single_char);
						cursors[0]++;
						if (cursors[0] == (int) strlen(adjusted_row[0]))
							completed++;
					}
				} else {
					printf(" ");
				}
			}
			if (newline_found) {
				cursors[0]++;
				if (cursors[0] == (int) strlen(adjusted_row[0]))
					completed++;
				newline_found = 0;
			}
			printf(" %s", pipe_sign);
			size_t k;
			for (k = 1; k <= obj->measurevalues_num2; k++) {
				printf(" ");
				for (j = 0; j < (size_t) real_lengths_values[k - 1]; j++) {
					if ((strlen(adjusted_row[k]) - cursors[k]) > 0) {
						if (adjusted_row[k][cursors[k]] == '\n') {
							printf(" ");
							newline_found = 1;
						} else {
							printf("%c", adjusted_row[k][cursors[k]]);
							cursors[k]++;
							if (cursors[k] == (int) strlen(adjusted_row[k]))
								completed++;
						}
					} else {
						printf(" ");
					}
				}
				if (newline_found) {
					cursors[k]++;
					if (cursors[k] == (int) strlen(adjusted_row[k]))
						completed++;
					newline_found = 0;
				}
				printf(" %s", pipe_sign);
			}
			printf("\n");
		}
		//row separator
		if (i != (obj->colkeys_num - 1)) {
			printf("%s", pipe_sign);
			printnstr(minus_sign, real_imp_dim_header_length + 2);
			printf("%s", pipe_sign);
			for (j = 0; j < obj->measurevalues_num2; j++) {
				printnstr(minus_sign, real_lengths_values[j] + 2);
				printf("%s", pipe_sign);
			}
			printf("\n");
		}
		//row cleanup
		free(tmprow);
		for (j = 0; j < obj->measurevalues_num2 + 1; j++)
			free(adjusted_row[j]);
		free(adjusted_row);
		free(cursors);
	}

	// print end
	printf("%s%s", plus_sign, equal_sign);
	printnstr(equal_sign, real_imp_dim_header_length + 1);
	for (j = 0; j < obj->measurevalues_num2; j++) {
		printf("%s%s", plus_sign, equal_sign);
		printnstr(equal_sign, real_lengths_values[j] + 1);
	}
	printf("%s\n", plus_sign);


	// PRINT EXP DIMENSIONS AND MEASURES HEADERS

	char **tmprow = (char **) calloc(obj->rowkeys_num + 1, sizeof(char *));
	if (!tmprow) {
		free(max_lengths_rowkeys);
		free(max_lengths_values);
		free(real_lengths_rowkeys);
		free(real_lengths_values);
		return;
	}
	tmprow[obj->rowkeys_num] = obj->measurename;
	for (j = 0; j < obj->rowkeys_num; j++)
		tmprow[j] = obj->rowkeys[j];

	char **adjusted_row = NULL;
	adjust_string_array(&adjusted_row, tmprow, obj->rowkeys_num + 1);
	if (!adjusted_row) {
		free(max_lengths_rowkeys);
		free(max_lengths_values);
		free(real_lengths_rowkeys);
		free(real_lengths_values);
		free(tmprow);
		return;
	}
	int *cursors = (int *) calloc(obj->rowkeys_num + 1, sizeof(int));
	if (!cursors) {
		free(max_lengths_rowkeys);
		free(max_lengths_values);
		free(real_lengths_rowkeys);
		free(real_lengths_values);
		free(tmprow);
		size_t k;
		for (k = 0; k < obj->rowkeys_num + 1; k++)
			free(adjusted_row[k]);
		free(adjusted_row);
		return;
	}
	int completed = 0;
	int newline_found = 0;
	char single_char[2];
	for (j = 0; j < obj->rowkeys_num; j++)
		if (strlen(adjusted_row[j]) == 0 || real_lengths_rowkeys[j] == 0)
			completed++;
	if (strlen(adjusted_row[obj->rowkeys_num]) == 0 || real_measure_header_length == 0)
		completed++;
	while (completed < (int) obj->rowkeys_num + 1) {
		printf("%s", pipe_sign);
		size_t k;
		for (k = 0; k < obj->rowkeys_num; k++) {
			printf(" ");
			for (j = 0; j < (size_t) real_lengths_rowkeys[k]; j++) {
				if ((strlen(adjusted_row[k]) - cursors[k]) > 0) {
					if (adjusted_row[k][cursors[k]] == '\n') {
						printf(" ");
						newline_found = 1;
					} else {
						snprintf(single_char, 2, "%c", adjusted_row[k][cursors[k]]);
						printf(color_string, single_char);
						cursors[k]++;
						if (cursors[k] == (int) strlen(adjusted_row[k]))
							completed++;
					}
				} else {
					printf(" ");
				}
			}
			if (newline_found) {
				cursors[k]++;
				if (cursors[k] == (int) strlen(adjusted_row[k]))
					completed++;
				newline_found = 0;
			}
			printf(" %s", pipe_sign);
		}
		printf(" ");
		for (j = 0; j < (size_t) real_measure_header_length; j++) {
			if ((strlen(adjusted_row[obj->rowkeys_num]) - cursors[obj->rowkeys_num]) > 0) {
				if (adjusted_row[obj->rowkeys_num][cursors[obj->rowkeys_num]] == '\n') {
					printf(" ");
					newline_found = 1;
				} else {
					snprintf(single_char, 2, "%c", adjusted_row[obj->rowkeys_num][cursors[obj->rowkeys_num]]);
					printf(color_string, single_char);
					cursors[obj->rowkeys_num]++;
					if (cursors[obj->rowkeys_num] == (int) strlen(adjusted_row[obj->rowkeys_num]))
						completed++;
				}
			} else {
				printf(" ");
			}
		}
		if (newline_found) {
			cursors[obj->rowkeys_num]++;
			if (cursors[obj->rowkeys_num] == (int) strlen(adjusted_row[obj->rowkeys_num]))
				completed++;
			newline_found = 0;
		}
		printf(" %s", pipe_sign);
		printf("\n");
	}
	// cleanup
	free(tmprow);
	for (j = 0; j < obj->rowkeys_num + 1; j++)
		free(adjusted_row[j]);
	free(adjusted_row);
	free(cursors);

	// print end
	for (j = 0; j < obj->rowkeys_num; j++) {
		printf("%s%s", plus_sign, equal_sign);
		printnstr(equal_sign, real_lengths_rowkeys[j] + 1);
	}
	for (j = 0; j < obj->measurevalues_num2; j++) {
		printf("%s%s", plus_sign, equal_sign);
		printnstr(equal_sign, real_lengths_values[j] + 1);
	}
	printf("%s\n", plus_sign);
	fflush(stdout);

	// PRINT VALUES

	// print contents
	for (i = 0; i < obj->measurevalues_num1; i++) {
		char **tmprow = (char **) calloc(obj->rowkeys_num + obj->measurevalues_num2, sizeof(char *));
		if (!tmprow) {
			free(max_lengths_rowkeys);
			free(max_lengths_values);
			free(real_lengths_rowkeys);
			free(real_lengths_values);
			return;
		}
		for (j = 0; j < obj->rowkeys_num; j++)
			tmprow[j] = obj->rowvalues[i][j];
		for (j = obj->rowkeys_num; j < obj->rowkeys_num + obj->measurevalues_num2; j++)
			tmprow[j] = obj->measurevalues[i][j];

		char **adjusted_row = NULL;
		adjust_string_array(&adjusted_row, tmprow, obj->rowkeys_num + obj->measurevalues_num2);
		if (!adjusted_row) {
			free(max_lengths_rowkeys);
			free(max_lengths_values);
			free(real_lengths_rowkeys);
			free(real_lengths_values);
			free(tmprow);
			return;
		}
		int *cursors = (int *) calloc(obj->rowkeys_num + obj->measurevalues_num2, sizeof(int));
		if (!cursors) {
			free(max_lengths_rowkeys);
			free(max_lengths_values);
			free(real_lengths_rowkeys);
			free(real_lengths_values);
			free(tmprow);
			size_t k;
			for (k = 0; k < obj->rowkeys_num + obj->measurevalues_num2; k++)
				free(adjusted_row[k]);
			free(adjusted_row);
			return;
		}
		int completed = 0;
		int newline_found = 0;
		for (j = 0; j < obj->rowkeys_num; j++)
			if (strlen(adjusted_row[j]) == 0 || real_lengths_rowkeys[j] == 0)
				completed++;
		for (j = obj->rowkeys_num; j < obj->rowkeys_num + obj->measurevalues_num2; j++)
			if (strlen(adjusted_row[j]) == 0 || real_lengths_values[j] == 0)
				completed++;
		while (completed < (int) (obj->rowkeys_num + obj->measurevalues_num2)) {
			printf("%s", pipe_sign);
			size_t k;
			for (k = 0; k < obj->rowkeys_num; k++) {
				printf(" ");
				for (j = 0; j < (size_t) real_lengths_rowkeys[k]; j++) {
					if ((strlen(adjusted_row[k]) - cursors[k]) > 0) {
						if (adjusted_row[k][cursors[k]] == '\n') {
							printf(" ");
							newline_found = 1;
						} else {
							printf("%c", adjusted_row[k][cursors[k]]);
							cursors[k]++;
							if (cursors[k] == (int) strlen(adjusted_row[k]))
								completed++;
						}
					} else {
						printf(" ");
					}
				}
				if (newline_found) {
					cursors[k]++;
					if (cursors[k] == (int) strlen(adjusted_row[k]))
						completed++;
					newline_found = 0;
				}
				printf(" %s", pipe_sign);
			}
			for (k = obj->rowkeys_num; k < obj->rowkeys_num + obj->measurevalues_num2; k++) {
				printf(" ");
				for (j = 0; j < (size_t) real_lengths_values[k - obj->rowkeys_num]; j++) {
					if ((strlen(adjusted_row[k]) - cursors[k]) > 0) {
						if (adjusted_row[k][cursors[k]] == '\n') {
							printf(" ");
							newline_found = 1;
						} else {
							printf("%c", adjusted_row[k][cursors[k]]);
							cursors[k]++;
							if (cursors[k] == (int) strlen(adjusted_row[k]))
								completed++;
						}
					} else {
						printf(" ");
					}
				}
				if (newline_found) {
					cursors[k]++;
					if (cursors[k] == (int) strlen(adjusted_row[k]))
						completed++;
					newline_found = 0;
				}
				printf(" %s", pipe_sign);
			}
			printf("\n");
		}
		//row separator
		if (i != (obj->measurevalues_num1 - 1)) {
			printf("%s", pipe_sign);
			for (j = 0; j < obj->rowkeys_num; j++) {
				printnstr(minus_sign, real_lengths_rowkeys[j] + 2);
				printf("%s", pipe_sign);
			}
			for (j = 0; j < obj->measurevalues_num2; j++) {
				printnstr(minus_sign, real_lengths_values[j] + 2);
				printf("%s", pipe_sign);
			}
			printf("\n");
		}
		//row cleanup
		free(tmprow);
		for (j = 0; j < obj->rowkeys_num + obj->measurevalues_num2; j++)
			free(adjusted_row[j]);
		free(adjusted_row);
		free(cursors);
	}

	// print end
	for (j = 0; j < obj->rowkeys_num; j++) {
		printf("%s%s", plus_sign, equal_sign);
		printnstr(equal_sign, real_lengths_rowkeys[j] + 1);
	}
	for (j = 0; j < obj->measurevalues_num2; j++) {
		printf("%s%s", plus_sign, equal_sign);
		printnstr(equal_sign, real_lengths_values[j] + 1);
	}
	printf("%s\n", plus_sign);
	fflush(stdout);

	free(max_lengths_rowkeys);
	free(max_lengths_values);
	free(real_lengths_rowkeys);
	free(real_lengths_values);

	return;
}

void print_tree(oph_json_obj_tree * obj, const char *color_string, int save_img, int open_img, char *session_code, char *markerid, char *layout)
{
	// print title
	if (obj->title && strlen(obj->title)) {
		printf(color_string, obj->title);
		printf("\n");
		char buf[OPH_TERM_VIEWER_SIGN_LEN];
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_MACRON_SIGN);
		printnstr(buf, strlen(obj->title));
		printf("\n");
		fflush(stdout);
	}
	// print description
	if (obj->description && strlen(obj->description)) {
		printf("%s\n", obj->description);
		fflush(stdout);
	}
	// create dot string for tree
	char dot_string[OPH_TERM_VIEWER_DOT_MAX_LEN];
	char printable_dot_string[OPH_TERM_VIEWER_DOT_MAX_LEN];
	size_t cc = 0, i, j;
	int ccc = 0;
	cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "digraph T {node [shape=box] ");
	ccc += snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "digraph T {\n\tnode\t[shape=box]\n\n");
	for (i = 0; i < obj->nodevalues_num1; i++) {
		cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "%d [label=\"", (int) i);
		ccc += snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "\t%d\t[label=\"", (int) i);
		for (j = 0; j < obj->nodevalues_num2; j++) {
			if (obj->nodekeys[j] && obj->nodevalues[i][j]) {
				if (strcmp(obj->nodekeys[j], "") && strcmp(obj->nodekeys[j], "-") && strcmp(obj->nodevalues[i][j], "") && strcmp(obj->nodevalues[i][j], "-")) {
					cc +=
					    snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "%s : %s\\n", (obj->nodekeys[j]) ? (obj->nodekeys[j]) : "-",
						     (obj->nodevalues[i][j]) ? (obj->nodevalues[i][j]) : "-");
					ccc +=
					    snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "%s : %s\\n", (obj->nodekeys[j]) ? (obj->nodekeys[j]) : "-",
						     (obj->nodevalues[i][j]) ? (obj->nodevalues[i][j]) : "-");
				}
			}
		}
		cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "\"] ");
		ccc += snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "\"]\n");
	}
	ccc += snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "\n");
	for (i = 0; i < obj->nodelinks_num; i++) {
		for (j = 0; j < obj->nodelinks[i].links_num; j++) {
			cc +=
			    snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "%d->%s [label=\"%s\"] ", (int) i, (obj->nodelinks[i].links[j].node) ? (obj->nodelinks[i].links[j].node) : "0",
				     (obj->nodelinks[i].links[j].description) ? (obj->nodelinks[i].links[j].description) : "");
			ccc +=
			    snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "\t%d->%s\t[label=\"%s\"]\n", (int) i,
				     (obj->nodelinks[i].links[j].node) ? (obj->nodelinks[i].links[j].node) : "0",
				     (obj->nodelinks[i].links[j].description) ? (obj->nodelinks[i].links[j].description) : "");
		}
	}

	if (layout) {
		if (strstr(layout, "ranked")) {
			char *ranks_string = NULL;
			if (oph_viewer_get_ranks_string(obj->nodelinks, obj->nodelinks_num, &ranks_string)) {
				fprintf(stderr, "Unable to create the image.\n");
				return;
			}
			cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "%s", ranks_string);
			free(ranks_string);
			ranks_string = NULL;
		}
		if (strstr(layout, "horizontal")) {
			cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, " rankdir=LR; ");
		}
	}

	cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "}\n");
	ccc += snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "}");

	// print output DOT
	printf(color_string, "Tree DOT string");
	printf(" :\n%s\n", printable_dot_string);
	fflush(stdout);

	if (save_img) {
		GVC_t *gvc;
		Agraph_t *g;
		FILE *fp;
		char filename[256];
		snprintf(filename, 256, "%s_%s.svg", (session_code) ? (session_code) : "undefined", (markerid) ? (markerid) : "undefined");
		fp = fopen(filename, "w");
		if (!fp) {
			fprintf(stderr, "\n\e[1;31mUnable to create the new image file %s.\e[0m\n", filename);
			return;
		}

		gvc = gvContext();
		if (!gvc) {
			fclose(fp);
			fprintf(stderr, "\n\e[1;31mUnable to create the image.\e[0m\n");
			return;
		}
		// create tree
		g = agmemread(dot_string);
		if (!g) {
			fclose(fp);
			gvFreeContext(gvc);
			fprintf(stderr, "\n\e[1;31mUnable to create the image.\e[0m\n");
			return;
		}

		gvLayout(gvc, g, "dot");
		gvRender(gvc, g, "svg:cairo", fp);
		gvFreeLayout(gvc, g);
		agclose(g);
		gvFreeContext(gvc);

		fclose(fp);

		// print output filename
		printf(color_string, "\nImage File");
		printf(" : %s\n", filename);
		fflush(stdout);

		if (open_img) {
			char command[280];
			snprintf(command, 280, "xdg-open %s", filename);
			// open image file
			if (system(command))
				fprintf(stderr, "Error in executing command '%s'\n", command);
		}
	}

	return;
}

void print_digraph(oph_json_obj_graph * obj, const char *color_string, int save_img, int open_img, char *session_code, char *markerid, char *layout)
{
	// print title
	if (obj->title && strlen(obj->title)) {
		printf(color_string, obj->title);
		printf("\n");
		char buf[OPH_TERM_VIEWER_SIGN_LEN];
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_MACRON_SIGN);
		printnstr(buf, strlen(obj->title));
		printf("\n");
		fflush(stdout);
	}
	// print description
	if (obj->description && strlen(obj->description)) {
		printf("%s\n", obj->description);
		fflush(stdout);
	}
	// create dot string for tree
	char dot_string[OPH_TERM_VIEWER_DOT_MAX_LEN];
	char printable_dot_string[OPH_TERM_VIEWER_DOT_MAX_LEN];
	size_t cc = 0, i, j;
	int ccc = 0;
	cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "digraph DG {node [shape=box] ");
	ccc += snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "digraph DG {\n\tnode\t[shape=box]\n\n");
	for (i = 0; i < obj->nodevalues_num1; i++) {
		cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "%d [label=\"", (int) i);
		ccc += snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "\t%d\t[label=\"", (int) i);
		for (j = 0; j < obj->nodevalues_num2; j++) {
			if (obj->nodekeys[j] && obj->nodevalues[i][j]) {
				if (strcmp(obj->nodekeys[j], "") && strcmp(obj->nodekeys[j], "-") && strcmp(obj->nodevalues[i][j], "") && strcmp(obj->nodevalues[i][j], "-")) {
					cc +=
					    snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "%s : %s\\n", (obj->nodekeys[j]) ? (obj->nodekeys[j]) : "-",
						     (obj->nodevalues[i][j]) ? (obj->nodevalues[i][j]) : "-");
					ccc +=
					    snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "%s : %s\\n", (obj->nodekeys[j]) ? (obj->nodekeys[j]) : "-",
						     (obj->nodevalues[i][j]) ? (obj->nodevalues[i][j]) : "-");
				}
			}
		}
		cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "\"] ");
		ccc += snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "\"]\n");
	}
	ccc += snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "\n");
	for (i = 0; i < obj->nodelinks_num; i++) {
		for (j = 0; j < obj->nodelinks[i].links_num; j++) {
			cc +=
			    snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "%d->%s [label=\"%s\"] ", (int) i, (obj->nodelinks[i].links[j].node) ? (obj->nodelinks[i].links[j].node) : "0",
				     (obj->nodelinks[i].links[j].description) ? (obj->nodelinks[i].links[j].description) : "");
			ccc +=
			    snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "\t%d->%s\t[label=\"%s\"]\n", (int) i,
				     (obj->nodelinks[i].links[j].node) ? (obj->nodelinks[i].links[j].node) : "0",
				     (obj->nodelinks[i].links[j].description) ? (obj->nodelinks[i].links[j].description) : "");
		}
	}

	if (layout) {
		if (strstr(layout, "ranked")) {
			char *ranks_string = NULL;
			if (oph_viewer_get_ranks_string(obj->nodelinks, obj->nodelinks_num, &ranks_string)) {
				fprintf(stderr, "Unable to create the image.\n");
				return;
			}
			cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "%s", ranks_string);
			free(ranks_string);
			ranks_string = NULL;
		}
		if (strstr(layout, "horizontal")) {
			cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, " rankdir=LR; ");
		}
	}

	cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "}\n");
	ccc += snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "}");

	// print output DOT
	printf(color_string, "Directed Graph DOT string");
	printf(" :\n%s\n", printable_dot_string);
	fflush(stdout);

	if (save_img) {
		GVC_t *gvc;
		Agraph_t *g;
		FILE *fp;
		char filename[256];
		snprintf(filename, 256, "%s_%s.svg", (session_code) ? (session_code) : "undefined", (markerid) ? (markerid) : "undefined");
		fp = fopen(filename, "w");
		if (!fp) {
			fprintf(stderr, "\n\e[1;31mUnable to create the new image file %s.\n", filename);
			return;
		}

		gvc = gvContext();
		if (!gvc) {
			fclose(fp);
			fprintf(stderr, "\n\e[1;31mUnable to create the image.\e[0m\n");
			return;
		}
		// create digraph
		g = agmemread(dot_string);
		if (!g) {
			fclose(fp);
			gvFreeContext(gvc);
			fprintf(stderr, "\n\e[1;31mUnable to create the image.\e[0m\n");
			return;
		}

		gvLayout(gvc, g, "dot");
		gvRender(gvc, g, "svg:cairo", fp);
		gvFreeLayout(gvc, g);
		agclose(g);
		gvFreeContext(gvc);

		fclose(fp);

		// print output filename
		printf(color_string, "\nImage File");
		printf(" : %s\n", filename);
		fflush(stdout);

		if (open_img) {
			char command[280];
			snprintf(command, 280, "xdg-open %s", filename);
			// open image file
			if (system(command))
				fprintf(stderr, "Error in executing command '%s'\n", command);
		}
	}

	return;
}

void print_graph(oph_json_obj_graph * obj, const char *color_string, int save_img, int open_img, char *session_code, char *markerid, char *layout)
{
	// print title
	if (obj->title && strlen(obj->title)) {
		printf(color_string, obj->title);
		printf("\n");
		char buf[OPH_TERM_VIEWER_SIGN_LEN];
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_MACRON_SIGN);
		printnstr(buf, strlen(obj->title));
		printf("\n");
		fflush(stdout);
	}
	// print description
	if (obj->description && strlen(obj->description)) {
		printf("%s\n", obj->description);
		fflush(stdout);
	}
	// create dot string for tree
	char dot_string[OPH_TERM_VIEWER_DOT_MAX_LEN];
	char printable_dot_string[OPH_TERM_VIEWER_DOT_MAX_LEN];
	size_t cc = 0, i, j;
	int ccc = 0;
	cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "graph G {node [shape=box] ");
	ccc += snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "graph G {\n\tnode\t[shape=box]\n\n");
	for (i = 0; i < obj->nodevalues_num1; i++) {
		cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "%d [label=\"", (int) i);
		ccc += snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "\t%d\t[label=\"", (int) i);
		for (j = 0; j < obj->nodevalues_num2; j++) {
			if (obj->nodekeys[j] && obj->nodevalues[i][j]) {
				if (strcmp(obj->nodekeys[j], "") && strcmp(obj->nodekeys[j], "-") && strcmp(obj->nodevalues[i][j], "") && strcmp(obj->nodevalues[i][j], "-")) {
					cc +=
					    snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "%s : %s\\n", (obj->nodekeys[j]) ? (obj->nodekeys[j]) : "-",
						     (obj->nodevalues[i][j]) ? (obj->nodevalues[i][j]) : "-");
					ccc +=
					    snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "%s : %s\\n", (obj->nodekeys[j]) ? (obj->nodekeys[j]) : "-",
						     (obj->nodevalues[i][j]) ? (obj->nodevalues[i][j]) : "-");
				}
			}
		}
		cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "\"] ");
		ccc += snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "\"]\n");
	}
	ccc += snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "\n");
	for (i = 0; i < obj->nodelinks_num; i++) {
		for (j = 0; j < obj->nodelinks[i].links_num; j++) {
			cc +=
			    snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "%d--%s [label=\"%s\"] ", (int) i, (obj->nodelinks[i].links[j].node) ? (obj->nodelinks[i].links[j].node) : "0",
				     (obj->nodelinks[i].links[j].description) ? (obj->nodelinks[i].links[j].description) : "");
			ccc +=
			    snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "\t%d--%s\t[label=\"%s\"]\n", (int) i,
				     (obj->nodelinks[i].links[j].node) ? (obj->nodelinks[i].links[j].node) : "0",
				     (obj->nodelinks[i].links[j].description) ? (obj->nodelinks[i].links[j].description) : "");
		}
	}

	if (layout) {
		if (strstr(layout, "ranked")) {
			char *ranks_string = NULL;
			if (oph_viewer_get_ranks_string(obj->nodelinks, obj->nodelinks_num, &ranks_string)) {
				fprintf(stderr, "Unable to create the image.\n");
				return;
			}
			cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "%s", ranks_string);
			free(ranks_string);
			ranks_string = NULL;
		}
		if (strstr(layout, "horizontal")) {
			cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, " rankdir=LR; ");
		}
	}

	cc += snprintf(dot_string + cc, OPH_TERM_VIEWER_DOT_MAX_LEN - cc, "}\n");
	ccc += snprintf(printable_dot_string + ccc, OPH_TERM_VIEWER_DOT_MAX_LEN - ccc, "}");

	// print output DOT
	printf(color_string, "Undirected Graph DOT string");
	printf(" :\n%s\n", printable_dot_string);
	fflush(stdout);

	if (save_img) {
		GVC_t *gvc;
		Agraph_t *g;
		FILE *fp;
		char filename[256];
		snprintf(filename, 256, "%s_%s.svg", (session_code) ? (session_code) : "undefined", (markerid) ? (markerid) : "undefined");
		fp = fopen(filename, "w");
		if (!fp) {
			fprintf(stderr, "\n\e[1;31mUnable to create the new image file %s.\e[0m\n", filename);
			return;
		}

		gvc = gvContext();
		if (!gvc) {
			fclose(fp);
			fprintf(stderr, "\n\e[1;31mUnable to create the image.\e[0m\n");
			return;
		}
		// create graph
		g = agmemread(dot_string);
		if (!g) {
			fclose(fp);
			gvFreeContext(gvc);
			fprintf(stderr, "\n\e[1;31mUnable to create the image.\e[0m\n");
			return;
		}

		gvLayout(gvc, g, "dot");
		gvRender(gvc, g, "svg:cairo", fp);
		gvFreeLayout(gvc, g);
		agclose(g);
		gvFreeContext(gvc);

		fclose(fp);

		// print output filename
		printf(color_string, "\nImage File");
		printf(" : %s\n", filename);
		fflush(stdout);

		if (open_img) {
			char command[280];
			snprintf(command, 280, "xdg-open %s", filename);
			// open image file
			if (system(command))
				fprintf(stderr, "Error in executing command '%s'\n", command);
		}
	}

	return;
}

void oph_term_viewer_retrieve_info(oph_json * json, char *txtstring, char **newdatacube, char **newcwd, char **newcdd, char **newtoken, char **exectime)
{
	if (!json && !txtstring) {
		if (newdatacube)
			*newdatacube = NULL;
		if (newcwd)
			*newcwd = NULL;
		if (newcdd)
			*newcdd = NULL;
		if (newtoken)
			*newtoken = NULL;
		if (exectime)
			*exectime = NULL;
		return;
	}

	if (json) {
		if (newdatacube) {
			*newdatacube = NULL;
			if (json->response_num >= 1) {
				size_t i;
				for (i = 0; i < json->response_num; i++) {
					if (!strcmp(json->response[i].objclass, OPH_JSON_TEXT)) {
						if (json->response[i].objcontent_num >= 1) {
							if ((((oph_json_obj_text *) json->response[i].objcontent)[0]).title && (((oph_json_obj_text *) json->response[i].objcontent)[0]).message) {
								if (!strcmp((((oph_json_obj_text *) json->response[i].objcontent)[0]).title, OPH_TERM_VIEWER_TITLE_FOR_CUBE)) {
									*newdatacube = (char *) strdup((const char *) (((oph_json_obj_text *) json->response[i].objcontent)[0]).message);
									break;
								}
							}
						}
					}
				}
			}
		}

		if (newcwd) {
			*newcwd = NULL;
			if (json->response_num >= 1) {
				size_t i;
				for (i = 0; i < json->response_num; i++) {
					if (!strcmp(json->response[i].objclass, OPH_JSON_TEXT)) {
						if (json->response[i].objcontent_num >= 1) {
							if ((((oph_json_obj_text *) json->response[i].objcontent)[0]).title && (((oph_json_obj_text *) json->response[i].objcontent)[0]).message) {
								if (!strcmp((((oph_json_obj_text *) json->response[i].objcontent)[0]).title, OPH_TERM_VIEWER_TITLE_FOR_CWD)) {
									*newcwd = (char *) strdup((const char *) (((oph_json_obj_text *) json->response[i].objcontent)[0]).message);
									break;
								}
							}
						}
					}
				}
			}
		}

		if (newcdd) {
			*newcdd = NULL;
			if (json->response_num >= 1) {
				size_t i;
				for (i = 0; i < json->response_num; i++) {
					if (!strcmp(json->response[i].objclass, OPH_JSON_TEXT)) {
						if (json->response[i].objcontent_num >= 1) {
							if ((((oph_json_obj_text *) json->response[i].objcontent)[0]).title && (((oph_json_obj_text *) json->response[i].objcontent)[0]).message) {
								if (!strcmp((((oph_json_obj_text *) json->response[i].objcontent)[0]).title, OPH_TERM_VIEWER_TITLE_FOR_CDD)) {
									*newcdd = (char *) strdup((const char *) (((oph_json_obj_text *) json->response[i].objcontent)[0]).message);
									break;
								}
							}
						}
					}
				}
			}
		}

		unsigned int i;

		if (newtoken) {
			*newtoken = NULL;
			if (json->extra) {
				for (i = 0; i < json->extra->keys_num; i++) {
					if (!strcmp(json->extra->keys[i], OPH_TERM_TOKEN_JSON)) {
						*newtoken = (char *) strdup(json->extra->values[i]);
						break;
					}
				}
			}
		}

		if (exectime) {
			*exectime = NULL;
			if (json->extra) {
				for (i = 0; i < json->extra->keys_num; i++) {
					if (!strcmp(json->extra->keys[i], OPH_TERM_EXEC_TIME)) {
						*exectime = (char *) strdup(json->extra->values[i]);
						break;
					}
				}
			}
		}

	} else {

		if (newdatacube) {
			*newdatacube = NULL;
			//retrieve newdatacube
			char *tmp = strdup((char *) txtstring);
			if (tmp) {
				char *ptr = strstr(tmp, "PID of output datacube is: http");
				if (!ptr) {
					free(tmp);
					tmp = NULL;
				} else {
					ptr = strstr(ptr, "http");
					char *tmp2 = strdup(ptr);
					if (!tmp2) {
						free(tmp);
						tmp = NULL;
					} else {
						char *saveptr;
						ptr = strtok_r(tmp2, "\n", &saveptr);
						if (!ptr) {
							free(tmp);
							tmp = NULL;
							free(tmp2);
							tmp2 = NULL;
						} else {
							*newdatacube = strdup(ptr);
							free(tmp);
							tmp = NULL;
							free(tmp2);
							tmp2 = NULL;
							ptr = NULL;
						}
					}
				}
			}
		}

		if (newcwd) {
			*newcwd = NULL;
			//retrieve newcwd
			char *tmp = strdup((char *) txtstring);
			if (tmp) {
				char *ptr = strstr(tmp, "Current working directory is: /");
				if (!ptr) {
					free(tmp);
					tmp = NULL;
				} else {
					ptr = strstr(ptr, "/");
					char *tmp2 = strdup(ptr);
					if (!tmp2) {
						free(tmp);
						tmp = NULL;
					} else {
						char *saveptr;
						ptr = strtok_r(tmp2, "\n", &saveptr);
						if (!ptr) {
							free(tmp);
							tmp = NULL;
							free(tmp2);
							tmp2 = NULL;
						} else {
							*newcwd = strdup(ptr);
							free(tmp);
							tmp = NULL;
							free(tmp2);
							tmp2 = NULL;
							ptr = NULL;
						}
					}
				}
			}
		}
	}

	return;
}

/* DUMP VIEWER */
int oph_term_viewer_dump(char **json_string, char **newdatacube, char **newcwd, char **newcdd, char **newtoken, char **exectime)
{
	if (!json_string || !*json_string) {
		oph_term_viewer_retrieve_info(NULL, NULL, newdatacube, newcwd, newcdd, newtoken, exectime);
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	if (print_json) {
		if (oph_term_response)
			free(oph_term_response);
		oph_term_response = strdup(*json_string);
	} else
		printf("%s\n", *json_string);
	//fflush(stdout);

	// attempt 1: let json_string be TXT
	oph_term_viewer_retrieve_info(NULL, *json_string, newdatacube, newcwd, newcdd, newtoken, exectime);

	// attempt 2: perhaps json_string is indeed JSON
	if (!((newdatacube && *newdatacube) || (newcwd && *newcwd))) {
		// Conversion from JSON string to OPH_JSON structure
		oph_json *json = NULL;
		if (!oph_json_from_json_string(&json, json_string)) {
			// parse JSON
			oph_term_viewer_retrieve_info(json, NULL, newdatacube, newcwd, newcdd, newtoken, exectime);
		}
		if (json)
			oph_json_free(json);
	}

	if (*json_string) {
		free(*json_string);
		*json_string = NULL;
	}

	return OPH_TERM_SUCCESS;
}

/* BASIC VIEWER */
int oph_term_viewer_basic(char **json_string, const char *color, int save_img, int open_img, int show_list, char **newdatacube, char **newcwd, char **newcdd, char **newtoken, char **exectime,
			  char *layout)
{
	if (!json_string || !*json_string) {
		oph_term_viewer_retrieve_info(NULL, NULL, newdatacube, newcwd, newcdd, newtoken, exectime);
		return OPH_TERM_INVALID_PARAM_VALUE;
	}
	// Set right color or no color
	char *color_string = OPH_TERM_VIEWER_NO_COLOR_STRING;
	if (color) {
		if (!strcmp(color, "black")) {
			color_string = OPH_TERM_VIEWER_BLACK_STRING;
		} else if (!strcmp(color, "green")) {
			color_string = OPH_TERM_VIEWER_GREEN_STRING;
		} else if (!strcmp(color, "yellow")) {
			color_string = OPH_TERM_VIEWER_YELLOW_STRING;
		} else if (!strcmp(color, "blue")) {
			color_string = OPH_TERM_VIEWER_BLUE_STRING;
		} else if (!strcmp(color, "purple")) {
			color_string = OPH_TERM_VIEWER_PURPLE_STRING;
		} else if (!strcmp(color, "cyan")) {
			color_string = OPH_TERM_VIEWER_CYAN_STRING;
		} else if (!strcmp(color, "white")) {
			color_string = OPH_TERM_VIEWER_WHITE_STRING;
		} else {
			color_string = OPH_TERM_VIEWER_RED_STRING;
		}
	}
	// Conversion from JSON string to OPH_JSON structure
	oph_json *json = NULL;
	if (oph_json_from_json_string(&json, json_string)) {
		if (json)
			oph_json_free(json);
		oph_term_viewer_retrieve_info(NULL, NULL, newdatacube, newcwd, newcdd, newtoken, exectime);
		return OPH_TERM_GENERIC_ERROR;
	}
	// Check for status. In case of ERROR print and exit
	size_t n, valid = 0;
	for (n = 0; n < json->responseKeyset_num; n++) {
		if (!strcmp(json->responseKeyset[n], "status")) {
			valid = 1;
			break;
		}
	}
	if (!valid) {
		if (json)
			oph_json_free(json);
		oph_term_viewer_retrieve_info(NULL, NULL, newdatacube, newcwd, newcdd, newtoken, exectime);
		return OPH_TERM_GENERIC_ERROR;
	}
	valid = 0;
	for (n = 0; n < json->response_num; n++) {
		if (!strcmp(json->response[n].objkey, "status")) {
			if (!strcmp(((oph_json_obj_text *) (json->response[n].objcontent))[0].title, "SUCCESS")) {
				if (print_debug_data) {
					size_t j;
					for (j = 1; j < json->response[n].objcontent_num; j++) {
						print_text(&(((oph_json_obj_text *) (json->response[n].objcontent))[j]), (const char *) color_string);
						printf("\n");
					}
				}
				valid = 1;
				break;
			} else {
				size_t j;
				for (j = 0; j < json->response[n].objcontent_num; j++) {
					print_text(&(((oph_json_obj_text *) (json->response[n].objcontent))[j]), (const char *) color_string);
					printf("\n");
				}
				if (json)
					oph_json_free(json);
				oph_term_viewer_retrieve_info(NULL, NULL, newdatacube, newcwd, newcdd, newtoken, exectime);
				return OPH_TERM_ERROR_WITHIN_JSON;	//needed to track framework ERROR
			}
		}
	}
	if (!valid) {
		if (json)
			oph_json_free(json);
		oph_term_viewer_retrieve_info(NULL, NULL, newdatacube, newcwd, newcdd, newtoken, exectime);
		return OPH_TERM_GENERIC_ERROR;
	}
	// retrieve new cube or new cwd if any
	oph_term_viewer_retrieve_info(json, NULL, newdatacube, newcwd, newcdd, newtoken, exectime);

	// needed for image filename
	char *session_code = NULL;
	char *markerid = NULL;
	if (json->source) {
		for (n = 0; n < json->source->keys_num; n++) {
			if (!strcmp(json->source->keys[n], OPH_TERM_VIEWER_SESSION_CODE_STRING)) {
				session_code = json->source->values[n];
			} else if (!strcmp(json->source->keys[n], OPH_TERM_VIEWER_MARKERID_STRING)) {
				markerid = json->source->values[n];
			}
		}
	}
	// Print OPH_JSON_RESPONSE objects
	size_t i;
	oph_json_response *response_i = NULL;
	for (i = 0; i < json->response_num; i++) {
		response_i = &(json->response[i]);

		if (!strcmp(response_i->objkey, "status") && json->response_num > 1)
			continue;	// status already processed

		if (response_i->objcontent_num == 1) {	//only 1 fragment -> additional info not needed
			if (!strcmp(response_i->objclass, OPH_JSON_TEXT))
				print_text(&(((oph_json_obj_text *) (response_i->objcontent))[0]), (const char *) color_string);
			else if (!strcmp(response_i->objclass, OPH_JSON_GRID))
				print_grid(&(((oph_json_obj_grid *) (response_i->objcontent))[0]), (const char *) color_string, show_list);
			else if (!strcmp(response_i->objclass, OPH_JSON_MULTIGRID))
				print_multigrid(&(((oph_json_obj_multigrid *) (response_i->objcontent))[0]), (const char *) color_string);
			else if (!strcmp(response_i->objclass, OPH_JSON_TREE))
				print_tree(&(((oph_json_obj_tree *) (response_i->objcontent))[0]), (const char *) color_string, save_img, open_img, session_code, markerid, layout);
			else if (!strcmp(response_i->objclass, OPH_JSON_DGRAPH))
				print_digraph(&(((oph_json_obj_graph *) (response_i->objcontent))[0]), (const char *) color_string, save_img, open_img, session_code, markerid, layout);
			else if (!strcmp(response_i->objclass, OPH_JSON_GRAPH))
				print_graph(&(((oph_json_obj_graph *) (response_i->objcontent))[0]), (const char *) color_string, save_img, open_img, session_code, markerid, layout);
		} else {	// more than 1 fragments -> add info on current fragment
			size_t j;
			if (!strcmp(response_i->objclass, OPH_JSON_TEXT)) {
				oph_json_obj_text *objcontent_j = NULL;
				for (j = 0; j < response_i->objcontent_num; j++) {
					objcontent_j = &(((oph_json_obj_text *) (response_i->objcontent))[j]);
					//printf("\n--- Part %d of %d ---\n",j+1,response_i->objcontent_num);
					print_text(objcontent_j, (const char *) color_string);
				}
			} else if (!strcmp(response_i->objclass, OPH_JSON_GRID)) {
				oph_json_obj_grid *objcontent_j = NULL;
				for (j = 0; j < response_i->objcontent_num; j++) {
					objcontent_j = &(((oph_json_obj_grid *) (response_i->objcontent))[j]);
					//printf("\n--- Part %d of %d ---\n",j+1,response_i->objcontent_num);
					print_grid(objcontent_j, (const char *) color_string, show_list);
				}
			} else if (!strcmp(response_i->objclass, OPH_JSON_MULTIGRID)) {
				oph_json_obj_multigrid *objcontent_j = NULL;
				for (j = 0; j < response_i->objcontent_num; j++) {
					objcontent_j = &(((oph_json_obj_multigrid *) (response_i->objcontent))[j]);
					//printf("\n--- Part %d of %d ---\n",j+1,response_i->objcontent_num);
					print_multigrid(objcontent_j, (const char *) color_string);
				}
			} else if (!strcmp(response_i->objclass, OPH_JSON_TREE)) {
				oph_json_obj_tree *objcontent_j = NULL;
				for (j = 0; j < response_i->objcontent_num; j++) {
					objcontent_j = &(((oph_json_obj_tree *) (response_i->objcontent))[j]);
					//printf("\n--- Part %d of %d ---\n",j+1,response_i->objcontent_num);
					print_tree(objcontent_j, (const char *) color_string, save_img, open_img, session_code, markerid, layout);
				}
			} else if (!strcmp(response_i->objclass, OPH_JSON_DGRAPH)) {
				oph_json_obj_graph *objcontent_j = NULL;
				for (j = 0; j < response_i->objcontent_num; j++) {
					objcontent_j = &(((oph_json_obj_graph *) (response_i->objcontent))[j]);
					//printf("\n--- Part %d of %d ---\n",j+1,response_i->objcontent_num);
					print_digraph(objcontent_j, (const char *) color_string, save_img, open_img, session_code, markerid, layout);
				}
			} else if (!strcmp(response_i->objclass, OPH_JSON_GRAPH)) {
				oph_json_obj_graph *objcontent_j = NULL;
				for (j = 0; j < response_i->objcontent_num; j++) {
					objcontent_j = &(((oph_json_obj_graph *) (response_i->objcontent))[j]);
					//printf("\n--- Part %d of %d ---\n",j+1,response_i->objcontent_num);
					print_graph(objcontent_j, (const char *) color_string, save_img, open_img, session_code, markerid, layout);
				}
			}
		}

		printf("\n");	// separate responses with a new line

	}

	if (json)
		oph_json_free(json);

	return OPH_TERM_SUCCESS;
}

/* EXTENDED VIEWER */
int oph_term_viewer_extended(char **json_string, const char *color, int save_img, int open_img, int show_list, char **newdatacube, char **newcwd, char **newcdd, char **newtoken, char **exectime,
			     char *layout)
{
	if (!json_string || !*json_string) {
		oph_term_viewer_retrieve_info(NULL, NULL, newdatacube, newcwd, newcdd, newtoken, exectime);
		return OPH_TERM_INVALID_PARAM_VALUE;
	}
	// Set right color or no color
	char *color_string = OPH_TERM_VIEWER_NO_COLOR_STRING;
	if (color) {
		if (!strcmp(color, "black")) {
			color_string = OPH_TERM_VIEWER_BLACK_STRING;
		} else if (!strcmp(color, "green")) {
			color_string = OPH_TERM_VIEWER_GREEN_STRING;
		} else if (!strcmp(color, "yellow")) {
			color_string = OPH_TERM_VIEWER_YELLOW_STRING;
		} else if (!strcmp(color, "blue")) {
			color_string = OPH_TERM_VIEWER_BLUE_STRING;
		} else if (!strcmp(color, "purple")) {
			color_string = OPH_TERM_VIEWER_PURPLE_STRING;
		} else if (!strcmp(color, "cyan")) {
			color_string = OPH_TERM_VIEWER_CYAN_STRING;
		} else if (!strcmp(color, "white")) {
			color_string = OPH_TERM_VIEWER_WHITE_STRING;
		} else {
			color_string = OPH_TERM_VIEWER_RED_STRING;
		}
	}
	// Conversion from JSON string to OPH_JSON structure
	oph_json *json = NULL;
	if (oph_json_from_json_string(&json, json_string)) {
		if (json)
			oph_json_free(json);
		oph_term_viewer_retrieve_info(NULL, NULL, newdatacube, newcwd, newcdd, newtoken, exectime);
		return OPH_TERM_GENERIC_ERROR;
	}
	// Check for status. In case of ERROR print and exit
	size_t n, valid = 0;
	for (n = 0; n < json->responseKeyset_num; n++) {
		if (!strcmp(json->responseKeyset[n], "status")) {
			valid = 1;
			break;
		}
	}
	if (!valid) {
		if (json)
			oph_json_free(json);
		oph_term_viewer_retrieve_info(NULL, NULL, newdatacube, newcwd, newcdd, newtoken, exectime);
		return OPH_TERM_GENERIC_ERROR;
	}
	valid = 0;
	for (n = 0; n < json->response_num; n++) {
		if (!strcmp(json->response[n].objkey, "status")) {
			if (!strcmp(((oph_json_obj_text *) (json->response[n].objcontent))[0].title, "SUCCESS")) {
				if (print_debug_data) {
					size_t j;
					for (j = 1; j < json->response[n].objcontent_num; j++) {
						print_text(&(((oph_json_obj_text *) (json->response[n].objcontent))[j]), (const char *) color_string);
						printf("\n");
					}
				}
				valid = 1;
				break;
			} else {
				size_t j;
				for (j = 0; j < json->response[n].objcontent_num; j++) {
					print_text(&(((oph_json_obj_text *) (json->response[n].objcontent))[j]), (const char *) color_string);
					printf("\n");
				}
				if (json)
					oph_json_free(json);
				oph_term_viewer_retrieve_info(NULL, NULL, newdatacube, newcwd, newcdd, newtoken, exectime);
				return OPH_TERM_ERROR_WITHIN_JSON;	//needed to track framework ERROR
			}
		}
	}
	if (!valid) {
		if (json)
			oph_json_free(json);
		oph_term_viewer_retrieve_info(NULL, NULL, newdatacube, newcwd, newcdd, newtoken, exectime);
		return OPH_TERM_GENERIC_ERROR;
	}
	// retrieve new cube or new cwd if any
	oph_term_viewer_retrieve_info(json, NULL, newdatacube, newcwd, newcdd, newtoken, exectime);

	// needed for image filename
	char *session_code = NULL;
	char *markerid = NULL;
	if (json->source) {
		for (n = 0; n < json->source->keys_num; n++) {
			if (!strcmp(json->source->keys[n], OPH_TERM_VIEWER_SESSION_CODE_STRING)) {
				session_code = json->source->values[n];
			} else if (!strcmp(json->source->keys[n], OPH_TERM_VIEWER_MARKERID_STRING)) {
				markerid = json->source->values[n];
			}
		}
	}
	// Print OPH_JSON_RESPONSE objects
	size_t i;
	oph_json_response *response_i = NULL;
	for (i = 0; i < json->response_num; i++) {
		response_i = &(json->response[i]);

		if (!strcmp(response_i->objkey, "status") && json->response_num > 1)
			continue;	// status already processed

		// print objkey
		char objkeybuff[150];
		snprintf(objkeybuff, 150, "| OBJKEY : %s |", response_i->objkey);
		char buf[OPH_TERM_VIEWER_SIGN_LEN];
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_UPPER_LEFT_SIGN);
		printnstr(buf, 1);
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_MINUS_SIGN);
		printnstr(buf, strlen(objkeybuff) - 2);
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_UPPER_RIGHT_SIGN);
		printnstr(buf, 1);
		printf("\n");
		printf(color_string, objkeybuff);
		printf("\n");
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_LOWER_LEFT_SIGN);
		printnstr(buf, 1);
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_MINUS_SIGN);
		printnstr(buf, strlen(objkeybuff) - 2);
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_LOWER_RIGHT_SIGN);
		printnstr(buf, 1);
		printf("\n\n");
		fflush(stdout);

		if (response_i->objcontent_num == 1) {	//only 1 fragment -> additional info not needed
			if (!strcmp(response_i->objclass, OPH_JSON_TEXT))
				print_text(&(((oph_json_obj_text *) (response_i->objcontent))[0]), (const char *) color_string);
			else if (!strcmp(response_i->objclass, OPH_JSON_GRID))
				print_grid(&(((oph_json_obj_grid *) (response_i->objcontent))[0]), (const char *) color_string, show_list);
			else if (!strcmp(response_i->objclass, OPH_JSON_MULTIGRID))
				print_multigrid(&(((oph_json_obj_multigrid *) (response_i->objcontent))[0]), (const char *) color_string);
			else if (!strcmp(response_i->objclass, OPH_JSON_TREE))
				print_tree(&(((oph_json_obj_tree *) (response_i->objcontent))[0]), (const char *) color_string, save_img, open_img, session_code, markerid, layout);
			else if (!strcmp(response_i->objclass, OPH_JSON_DGRAPH))
				print_digraph(&(((oph_json_obj_graph *) (response_i->objcontent))[0]), (const char *) color_string, save_img, open_img, session_code, markerid, layout);
			else if (!strcmp(response_i->objclass, OPH_JSON_GRAPH))
				print_graph(&(((oph_json_obj_graph *) (response_i->objcontent))[0]), (const char *) color_string, save_img, open_img, session_code, markerid, layout);
		} else {	// more than 1 fragments -> add info on current fragment
			size_t j;
			if (!strcmp(response_i->objclass, OPH_JSON_TEXT)) {
				oph_json_obj_text *objcontent_j = NULL;
				for (j = 0; j < response_i->objcontent_num; j++) {
					objcontent_j = &(((oph_json_obj_text *) (response_i->objcontent))[j]);
					printf("--- Part %d of %d ---\n", (int) j + 1, response_i->objcontent_num);
					print_text(objcontent_j, (const char *) color_string);
				}
			} else if (!strcmp(response_i->objclass, OPH_JSON_GRID)) {
				oph_json_obj_grid *objcontent_j = NULL;
				for (j = 0; j < response_i->objcontent_num; j++) {
					objcontent_j = &(((oph_json_obj_grid *) (response_i->objcontent))[j]);
					printf("--- Part %d of %d ---\n", (int) j + 1, response_i->objcontent_num);
					print_grid(objcontent_j, (const char *) color_string, show_list);
				}
			} else if (!strcmp(response_i->objclass, OPH_JSON_MULTIGRID)) {
				oph_json_obj_multigrid *objcontent_j = NULL;
				for (j = 0; j < response_i->objcontent_num; j++) {
					objcontent_j = &(((oph_json_obj_multigrid *) (response_i->objcontent))[j]);
					printf("--- Part %d of %d ---\n", (int) j + 1, response_i->objcontent_num);
					print_multigrid(objcontent_j, (const char *) color_string);
				}
			} else if (!strcmp(response_i->objclass, OPH_JSON_TREE)) {
				oph_json_obj_tree *objcontent_j = NULL;
				for (j = 0; j < response_i->objcontent_num; j++) {
					objcontent_j = &(((oph_json_obj_tree *) (response_i->objcontent))[j]);
					printf("--- Part %d of %d ---\n", (int) j + 1, response_i->objcontent_num);
					print_tree(objcontent_j, (const char *) color_string, save_img, open_img, session_code, markerid, layout);
				}
			} else if (!strcmp(response_i->objclass, OPH_JSON_DGRAPH)) {
				oph_json_obj_graph *objcontent_j = NULL;
				for (j = 0; j < response_i->objcontent_num; j++) {
					objcontent_j = &(((oph_json_obj_graph *) (response_i->objcontent))[j]);
					printf("--- Part %d of %d ---\n", (int) j + 1, response_i->objcontent_num);
					print_digraph(objcontent_j, (const char *) color_string, save_img, open_img, session_code, markerid, layout);
				}
			} else if (!strcmp(response_i->objclass, OPH_JSON_GRAPH)) {
				oph_json_obj_graph *objcontent_j = NULL;
				for (j = 0; j < response_i->objcontent_num; j++) {
					objcontent_j = &(((oph_json_obj_graph *) (response_i->objcontent))[j]);
					printf("--- Part %d of %d ---\n", (int) j + 1, response_i->objcontent_num);
					print_graph(objcontent_j, (const char *) color_string, save_img, open_img, session_code, markerid, layout);
				}
			}
		}

		printf("\n");	// separate responses with a new line

	}

	// Print OPH_JSON_SOURCE
	if (json->source) {
		// print section title
		char buf[OPH_TERM_VIEWER_SIGN_LEN];
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_UPPER_LEFT_SIGN);
		printnstr(buf, 1);
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_MINUS_SIGN);
		printnstr(buf, strlen("| SOURCE |") - 2);
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_UPPER_RIGHT_SIGN);
		printnstr(buf, 1);
		printf("\n");
		printf(color_string, "| SOURCE |");
		printf("\n");
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_LOWER_LEFT_SIGN);
		printnstr(buf, 1);
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_MINUS_SIGN);
		printnstr(buf, strlen("| SOURCE |") - 2);
		snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_LOWER_RIGHT_SIGN);
		printnstr(buf, 1);
		printf("\n\n");
		fflush(stdout);

		// print key-value pairs
		printf(color_string, "Source Key");
		printf(" : %s\n", (json->source->srckey) ? (json->source->srckey) : OPH_TERM_VIEWER_MINUS_SIGN);
		printf(color_string, "Source Name");
		printf(" : %s\n", (json->source->srcname) ? (json->source->srcname) : OPH_TERM_VIEWER_MINUS_SIGN);
		printf(color_string, "Source URL");
		printf(" : %s\n", (json->source->srcurl) ? (json->source->srcurl) : OPH_TERM_VIEWER_MINUS_SIGN);
		printf(color_string, "Source Description");
		printf(" : %s\n", (json->source->description) ? (json->source->description) : OPH_TERM_VIEWER_MINUS_SIGN);
		printf(color_string, "Producer");
		printf(" : %s\n", (json->source->producer) ? (json->source->producer) : OPH_TERM_VIEWER_MINUS_SIGN);
		fflush(stdout);

		// print other key-value pairs
		for (n = 0; n < json->source->keys_num; n++) {
			printf(color_string, json->source->keys[n]);
			printf(" : %s\n", json->source->values[n]);
		}
		fflush(stdout);
	}
	// Print OPH_JSON_CONSUMERS
	printf("\n");
	// print section title
	char buf[OPH_TERM_VIEWER_SIGN_LEN];
	snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_UPPER_LEFT_SIGN);
	printnstr(buf, 1);
	snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_MINUS_SIGN);
	printnstr(buf, strlen("| CONSUMERS |") - 2);
	snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_UPPER_RIGHT_SIGN);
	printnstr(buf, 1);
	printf("\n");
	printf(color_string, "| CONSUMERS |");
	printf("\n");
	snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_LOWER_LEFT_SIGN);
	printnstr(buf, 1);
	snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_MINUS_SIGN);
	printnstr(buf, strlen("| CONSUMERS |") - 2);
	snprintf(buf, OPH_TERM_VIEWER_SIGN_LEN, color_string, OPH_TERM_VIEWER_LOWER_RIGHT_SIGN);
	printnstr(buf, 1);
	printf("\n\n");
	fflush(stdout);

	// print consumers
	for (n = 0; n < json->consumers_num; n++) {
		if (n != 0)
			printf(", ");
		printf("%s", json->consumers[n]);
	}
	printf("\n\n");
	fflush(stdout);

	if (json)
		oph_json_free(json);

	return OPH_TERM_SUCCESS;
}

/* VIEWER CONTROLLER */
int oph_term_viewer(const char *viewer_type, char **json_string, const char *color, int save_img, int open_img, int show_list, char **newdatacube, char **newcwd, char **newcdd, char **newtoken,
		    char **exectime, char *layout)
{
	if (!viewer_type || !json_string || !*json_string || !color) {
		if (json_string && *json_string) {
			free(*json_string);
			*json_string = NULL;
		}
		oph_term_viewer_retrieve_info(NULL, NULL, newdatacube, newcwd, newcdd, newtoken, exectime);
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	if (!strcmp(viewer_type, OPH_TERM_VIEWER_TYPE_DUMP) || print_json) {
		return oph_term_viewer_dump(json_string, newdatacube, newcwd, newcdd, newtoken, exectime);
	} else if (!strcmp(viewer_type, OPH_TERM_VIEWER_TYPE_BASIC)) {
		return oph_term_viewer_basic(json_string, NULL, save_img, open_img, show_list, newdatacube, newcwd, newcdd, newtoken, exectime, layout);
	} else if (!strcmp(viewer_type, OPH_TERM_VIEWER_TYPE_COLOURED)) {
		return oph_term_viewer_basic(json_string, color, save_img, open_img, show_list, newdatacube, newcwd, newcdd, newtoken, exectime, layout);
	} else if (!strcmp(viewer_type, OPH_TERM_VIEWER_TYPE_EXTENDED)) {
		return oph_term_viewer_extended(json_string, NULL, save_img, open_img, show_list, newdatacube, newcwd, newcdd, newtoken, exectime, layout);
	} else if (!strcmp(viewer_type, OPH_TERM_VIEWER_TYPE_EXTENDED_COLOURED)) {
		return oph_term_viewer_extended(json_string, color, save_img, open_img, show_list, newdatacube, newcwd, newcdd, newtoken, exectime, layout);
	} else {
		free(*json_string);
		*json_string = NULL;
		oph_term_viewer_retrieve_info(NULL, NULL, newdatacube, newcwd, newcdd, newtoken, exectime);
		return OPH_TERM_INVALID_PARAM_VALUE;
	}
}

int oph_term_viewer_retrieve_command_jobid_creation(char *json_string, char **command, char **jobid, char **creation_time)
{
	if (!json_string)
		return OPH_TERM_INVALID_PARAM_VALUE;

	char *tmp_json_string = strdup(json_string);
	if (!tmp_json_string)
		return OPH_TERM_INVALID_PARAM_VALUE;

	oph_json *json = NULL;
	if (oph_json_from_json_string(&json, &tmp_json_string)) {
		if (json)
			oph_json_free(json);
		return OPH_TERM_GENERIC_ERROR;
	}

	int found = 0;
	if (json->response_num >= 1) {
		size_t i;
		for (i = 0; i < json->response_num; i++) {
			if (!strcmp(json->response[i].objkey, "resume")) {
				if (json->response[i].objcontent_num >= 1) {
					if ((((oph_json_obj_grid *) json->response[i].objcontent)[0]).values[0][0]) {
						if (jobid) {
							*jobid = (char *) strdup((((oph_json_obj_grid *) json->response[i].objcontent)[0]).values[0][0]);
							if (!(*jobid)) {
								if (command && *command) {
									free(*command);
									*command = NULL;
								}
								if (creation_time && *creation_time) {
									free(*creation_time);
									*creation_time = NULL;
								}
								if (json)
									oph_json_free(json);
								return OPH_TERM_MEMORY_ERROR;
							}
						}
					}
					if ((((oph_json_obj_grid *) json->response[i].objcontent)[0]).values[0][4]) {
						if (creation_time) {
							*creation_time = (char *) strdup((((oph_json_obj_grid *) json->response[i].objcontent)[0]).values[0][4]);
							if (!(*creation_time)) {
								if (command && *command) {
									free(*command);
									*command = NULL;
								}
								if (jobid && *jobid) {
									free(*jobid);
									*jobid = NULL;
								}
								if (json)
									oph_json_free(json);
								return OPH_TERM_MEMORY_ERROR;
							}
						}
					}
					if ((((oph_json_obj_grid *) json->response[i].objcontent)[0]).values[0][5]) {
						if (command) {
							*command = (char *) strdup((((oph_json_obj_grid *) json->response[i].objcontent)[0]).values[0][5]);
							if (!(*command)) {
								if (creation_time && *creation_time) {
									free(*creation_time);
									*creation_time = NULL;
								}
								if (jobid && *jobid) {
									free(*jobid);
									*jobid = NULL;
								}
								if (json)
									oph_json_free(json);
								return OPH_TERM_MEMORY_ERROR;
							}
						}
					}
					found = 1;
					break;
				}
			}
		}
	}
	if (!found) {
		if (command && *command) {
			free(*command);
			*command = NULL;
		}
		if (creation_time && *creation_time) {
			free(*creation_time);
			*creation_time = NULL;
		}
		if (jobid && *jobid) {
			free(*jobid);
			*jobid = NULL;
		}
		if (json)
			oph_json_free(json);
		return OPH_TERM_GENERIC_ERROR;
	}

	if (json)
		oph_json_free(json);
	return OPH_TERM_SUCCESS;
}

int oph_term_viewer_retrieve_command_jobid(char *json_string, char **command, char **jobid)
{
	return oph_term_viewer_retrieve_command_jobid_creation(json_string, command, jobid, NULL);
}

int oph_term_viewer_retrieve_session_size(char *json_string, int *session_size, char ***exit_status)
{
	if (!json_string || !session_size)
		return OPH_TERM_INVALID_PARAM_VALUE;

	char *tmp_json_string = strdup(json_string);
	if (!tmp_json_string)
		return OPH_TERM_INVALID_PARAM_VALUE;

	oph_json *json = NULL;
	if (oph_json_from_json_string(&json, &tmp_json_string)) {
		if (json)
			oph_json_free(json);
		return OPH_TERM_GENERIC_ERROR;
	}

	int found = 0;
	if (json->response_num >= 1) {
		size_t i;
		oph_json_obj_grid *grid = (oph_json_obj_grid *) json->response[i].objcontent;
		for (i = 0; i < json->response_num; i++) {
			if (!strcmp(json->response[i].objkey, "resume")) {
				if (json->response[i].objcontent_num >= 1) {
					grid = (oph_json_obj_grid *) json->response[i].objcontent;
					*session_size = grid->values_num1;
					if (exit_status) {
						int j;
						*exit_status = (char **) calloc(*session_size, sizeof(char *));
						for (j = 0; j < *session_size; j++)
							(*exit_status)[j] = strdup(grid->values[j][OPH_TERM_VIEWER_EXIT_STATUS_INDEX]);
					}
					found = 1;
					break;
				}
			}
		}
	}
	if (!found) {
		if (json)
			oph_json_free(json);
		return OPH_TERM_GENERIC_ERROR;
	}

	if (json)
		oph_json_free(json);
	return OPH_TERM_SUCCESS;
}

int oph_term_viewer_is_session_switched(char *json_string)
{
	if (!json_string)
		return 0;

	char *tmp_json_string = strdup(json_string);
	if (!tmp_json_string)
		return 0;

	oph_json *json = NULL;
	if (oph_json_from_json_string(&json, &tmp_json_string)) {
		if (json)
			oph_json_free(json);
		return 0;
	}

	if (json->response_num >= 1) {
		size_t i;
		for (i = 0; i < json->response_num; i++) {
			if (!strcmp(json->response[i].objkey, "status")) {
				if (json->response[i].objcontent_num == 1) {
					if (!strcmp((((oph_json_obj_text *) json->response[i].objcontent)[0]).title, "SUCCESS")) {
						if (json)
							oph_json_free(json);
						return 1;
					} else {
						if (json)
							oph_json_free(json);
						return 0;
					}
				} else {
					if (json)
						oph_json_free(json);
					return 0;
				}
			}
		}
	}

	if (json)
		oph_json_free(json);
	return 0;
}

int oph_term_viewer_retrieve_config(char *json_string, const char *key, char ***keys, char ***props, unsigned int *nprops, char **newtoken)
{
	if (!json_string || !key || !keys || !props || !nprops)
		return OPH_TERM_INVALID_PARAM_VALUE;

	*keys = *props = NULL;
	*nprops = 0;
	if (newtoken)
		*newtoken = NULL;

	char *tmp_json_string = strdup(json_string);
	if (!tmp_json_string)
		return OPH_TERM_INVALID_PARAM_VALUE;

	oph_json *json = NULL;
	if (oph_json_from_json_string(&json, &tmp_json_string)) {
		if (json)
			oph_json_free(json);
		return OPH_TERM_GENERIC_ERROR;
	}

	unsigned int i;

	if (newtoken) {
		*newtoken = NULL;
		if (json->extra) {
			for (i = 0; i < json->extra->keys_num; i++) {
				if (!strcmp(json->extra->keys[i], OPH_TERM_TOKEN_JSON)) {
					*newtoken = (char *) strdup(json->extra->values[i]);
					break;
				}
			}
		}
	}

	if (json->response_num >= 1) {
		unsigned int i, j;
		for (i = 0; i < json->response_num; i++) {
			if (!strcmp(json->response[i].objkey, "get_config")) {
				if (json->response[i].objcontent_num >= 1) {
					*nprops = (((oph_json_obj_grid *) json->response[i].objcontent)[0]).values_num1;
					*keys = (char **) calloc(*nprops, sizeof(char *));
					*props = (char **) calloc(*nprops, sizeof(char *));
					if (!*keys || !*props) {
						if (json)
							oph_json_free(json);
						if (*keys)
							free(*keys);
						if (*props)
							free(*props);
						*nprops = 0;
						return OPH_TERM_MEMORY_ERROR;
					}
					for (j = 0; j < *nprops; j++) {
						(*keys)[j] = (char *) strdup((((oph_json_obj_grid *) json->response[i].objcontent)[0]).values[j][0]);
						(*props)[j] = (char *) strdup((((oph_json_obj_grid *) json->response[i].objcontent)[0]).values[j][1]);
						if (!(*keys)[j] || !(*props)[j]) {
							if (json)
								oph_json_free(json);
							if (*keys) {
								for (j = 0; j < *nprops; j++)
									if ((*keys)[j])
										free((*keys)[j]);
								free(*keys);
							}
							if (*props) {
								for (j = 0; j < *nprops; j++)
									if ((*props)[j])
										free((*props)[j]);
								free(*props);
							}
							*nprops = 0;
							return OPH_TERM_MEMORY_ERROR;
						}
					}
				}
				break;
			}
		}
	}

	if (json)
		oph_json_free(json);

	return OPH_TERM_SUCCESS;
}

int oph_term_viewer_check_wid_mkid(char *json_string, char *wid, char *mkid)
{
	if (!json_string)
		return OPH_TERM_INVALID_PARAM_VALUE;

	char *tmp_json_string = strdup(json_string);
	if (!tmp_json_string)
		return OPH_TERM_INVALID_PARAM_VALUE;

	oph_json *json = NULL;
	if (oph_json_from_json_string(&json, &tmp_json_string)) {
		if (json)
			oph_json_free(json);
		return OPH_TERM_GENERIC_ERROR;
	}

	int found = 0;
	if (json->response_num >= 1) {
		size_t i, j;
		for (i = 0; i < json->response_num; i++) {
			if (!strcmp(json->response[i].objkey, "resume")) {
				if (json->response[i].objcontent_num >= 1) {
					for (j = 0; j < (((oph_json_obj_grid *) json->response[i].objcontent)[0]).values_num1; j++) {
						if (!strcmp(wid, (((oph_json_obj_grid *) json->response[i].objcontent)[0]).values[j][2])
						    && !strcmp(mkid, (((oph_json_obj_grid *) json->response[i].objcontent)[0]).values[j][3])) {
							found = 1;
							break;
						}
					}
					if (found)
						break;
				}
			}
		}
	}
	if (!found) {
		if (json)
			oph_json_free(json);
		return OPH_TERM_GENERIC_ERROR;
	}

	if (json)
		oph_json_free(json);
	return OPH_TERM_SUCCESS;
}

int oph_term_viewer_check_workflow_status(char *json_string)
{
	if (!json_string)
		return OPH_TERM_INVALID_PARAM_VALUE;

	char *tmp_json_string = strdup(json_string);
	if (!tmp_json_string)
		return OPH_TERM_INVALID_PARAM_VALUE;

	oph_json *json = NULL;
	if (oph_json_from_json_string(&json, &tmp_json_string)) {
		if (json)
			oph_json_free(json);
		return OPH_TERM_GENERIC_ERROR;
	}

	int found = 0;
	if (json->responseKeyset_num >= 1) {
		size_t i;
		for (i = 0; i < json->responseKeyset_num; i++) {
			if (!strcmp(json->responseKeyset[i], "workflow_status")) {
				found = 1;
				break;
			}
		}
	}
	if (!found) {
		if (json)
			oph_json_free(json);
		return OPH_TERM_GENERIC_ERROR;
	}

	if (json)
		oph_json_free(json);
	return OPH_TERM_SUCCESS;
}

int oph_term_viewer_check_workflow_ended(char *json_string)
{
	if (!json_string)
		return OPH_TERM_INVALID_PARAM_VALUE;

	char *tmp_json_string = strdup(json_string);
	if (!tmp_json_string)
		return OPH_TERM_INVALID_PARAM_VALUE;

	oph_json *json = NULL;
	if (oph_json_from_json_string(&json, &tmp_json_string)) {
		if (json)
			oph_json_free(json);
		return OPH_TERM_GENERIC_ERROR;
	}

	if (json->response_num >= 1) {
		size_t i;
		for (i = 0; i < json->response_num; i++) {
			if (!strcmp(json->response[i].objkey, "workflow_status")) {
				if (!strcmp(((oph_json_obj_text *) json->response[i].objcontent)[0].message, "OPH_STATUS_COMPLETED")
				    || strstr(((oph_json_obj_text *) json->response[i].objcontent)[0].message, "ERROR")) {
					if (json)
						oph_json_free(json);
					return OPH_TERM_SUCCESS;
				} else {
					if (json)
						oph_json_free(json);
					return OPH_TERM_GENERIC_ERROR;
				}
			}
		}
	}

	if (json)
		oph_json_free(json);
	return OPH_TERM_SUCCESS;
}
