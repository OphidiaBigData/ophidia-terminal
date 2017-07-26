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

#include "oph_json_grid.h"

/* Standard C99 headers */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

/***********OPH_JSON_OBJ_GRID INTERNAL FUNCTIONS***********/

// Free a grid object contents
int oph_json_free_grid(oph_json_obj_grid * obj)
{
	if (obj) {
		if (obj->description) {
			free(obj->description);
			obj->description = NULL;
		}
		if (obj->fieldtypes) {
			size_t i;
			for (i = 0; i < obj->fieldtypes_num; i++) {
				if (obj->fieldtypes[i]) {
					free(obj->fieldtypes[i]);
					obj->fieldtypes[i] = NULL;
				}
			}
			free(obj->fieldtypes);
			obj->fieldtypes = NULL;
		}
		obj->fieldtypes_num = 0;
		if (obj->keys) {
			size_t i;
			for (i = 0; i < obj->keys_num; i++) {
				if (obj->keys[i]) {
					free(obj->keys[i]);
					obj->keys[i] = NULL;
				}
			}
			free(obj->keys);
			obj->keys = NULL;
		}
		obj->keys_num = 0;
		if (obj->title) {
			free(obj->title);
			obj->title = NULL;
		}
		if (obj->values) {
			size_t i;
			for (i = 0; i < obj->values_num1; i++) {
				if (obj->values[i]) {
					size_t j;
					for (j = 0; j < obj->values_num2; j++) {
						if (obj->values[i][j]) {
							free(obj->values[i][j]);
							obj->values[i][j] = NULL;
						}
					}
					free(obj->values[i]);
					obj->values[i] = NULL;
				}
			}
			free(obj->values);
			obj->values = NULL;
		}
		obj->values_num1 = 0;
		obj->values_num2 = 0;
	}
	return OPH_JSON_SUCCESS;
}

/***********OPH_JSON_OBJ_GRID FUNCTIONS***********/

int oph_json_add_grid(oph_json * json, const char *objkey, const char *title, const char *description, const char **keys, int keys_num, const char **fieldtypes, int fieldtypes_num)
{
	if (!json || !objkey || !title || !keys || keys_num < 1 || !fieldtypes || fieldtypes_num < 1) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}

	if (json->response_num == 0) {
		json->response = (oph_json_response *) malloc(sizeof(oph_json_response));
		if (!json->response) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->response[0].objclass = NULL;
		json->response[0].objcontent = NULL;
		json->response[0].objcontent_num = 0;
		json->response[0].objkey = NULL;

		json->response[0].objclass = (char *) strdup(OPH_JSON_GRID);
		if (!json->response[0].objclass) {
			return OPH_JSON_MEMORY_ERROR;
		}

		json->response_num++;

		json->response[0].objkey = (char *) strdup(objkey);
		if (!json->response[0].objkey) {
			return OPH_JSON_MEMORY_ERROR;
		}
		if (oph_json_add_responseKey(json, objkey)) {
			return OPH_JSON_MEMORY_ERROR;
		}

		json->response[0].objcontent = malloc(sizeof(oph_json_obj_grid));
		if (!json->response[0].objcontent) {
			return OPH_JSON_MEMORY_ERROR;
		}
		((oph_json_obj_grid *) json->response[0].objcontent)[0].description = NULL;
		((oph_json_obj_grid *) json->response[0].objcontent)[0].fieldtypes = NULL;
		((oph_json_obj_grid *) json->response[0].objcontent)[0].fieldtypes_num = 0;
		((oph_json_obj_grid *) json->response[0].objcontent)[0].keys = NULL;
		((oph_json_obj_grid *) json->response[0].objcontent)[0].keys_num = 0;
		((oph_json_obj_grid *) json->response[0].objcontent)[0].title = NULL;
		((oph_json_obj_grid *) json->response[0].objcontent)[0].values = NULL;
		((oph_json_obj_grid *) json->response[0].objcontent)[0].values_num1 = 0;
		((oph_json_obj_grid *) json->response[0].objcontent)[0].values_num2 = 0;

		((oph_json_obj_grid *) json->response[0].objcontent)[0].title = (char *) strdup(title);
		if (!((oph_json_obj_grid *) json->response[0].objcontent)[0].title) {
			return OPH_JSON_MEMORY_ERROR;
		}

		json->response[0].objcontent_num++;

		if (description) {
			((oph_json_obj_grid *) json->response[0].objcontent)[0].description = (char *) strdup(description);
			if (!((oph_json_obj_grid *) json->response[0].objcontent)[0].description) {
				return OPH_JSON_MEMORY_ERROR;
			}
		}

		size_t k, q;

		((oph_json_obj_grid *) json->response[0].objcontent)[0].keys = (char **) malloc(sizeof(char *) * keys_num);
		if (!((oph_json_obj_grid *) json->response[0].objcontent)[0].keys) {
			return OPH_JSON_MEMORY_ERROR;
		}
		for (k = 0; k < (size_t) keys_num; k++) {
			for (q = 0; q < k; q++) {
				if (!strcmp(keys[q], keys[k])) {
					return OPH_JSON_BAD_PARAM_ERROR;
				}
			}
			((oph_json_obj_grid *) json->response[0].objcontent)[0].keys[k] = (char *) strdup(keys[k]);
			if (!((oph_json_obj_grid *) json->response[0].objcontent)[0].keys[k]) {
				return OPH_JSON_MEMORY_ERROR;
			}
			((oph_json_obj_grid *) json->response[0].objcontent)[0].keys_num++;
		}

		((oph_json_obj_grid *) json->response[0].objcontent)[0].fieldtypes = (char **) malloc(sizeof(char *) * fieldtypes_num);
		if (!((oph_json_obj_grid *) json->response[0].objcontent)[0].fieldtypes) {
			return OPH_JSON_MEMORY_ERROR;
		}
		for (k = 0; k < (size_t) fieldtypes_num; k++) {
			if (!oph_json_is_type_correct(fieldtypes[k])) {
				return OPH_JSON_BAD_PARAM_ERROR;
			}
			((oph_json_obj_grid *) json->response[0].objcontent)[0].fieldtypes[k] = (char *) strdup(fieldtypes[k]);
			if (!((oph_json_obj_grid *) json->response[0].objcontent)[0].fieldtypes[k]) {
				return OPH_JSON_MEMORY_ERROR;
			}
			((oph_json_obj_grid *) json->response[0].objcontent)[0].fieldtypes_num++;
		}

		if (keys_num != fieldtypes_num || (size_t) keys_num != ((oph_json_obj_grid *) json->response[0].objcontent)[0].keys_num
		    || (size_t) fieldtypes_num != ((oph_json_obj_grid *) json->response[0].objcontent)[0].fieldtypes_num) {
			return OPH_JSON_BAD_PARAM_ERROR;
		}

		((oph_json_obj_grid *) json->response[0].objcontent)[0].values_num2 = ((oph_json_obj_grid *) json->response[0].objcontent)[0].keys_num;
	} else {
		size_t i;
		int add_frag = 0;
		for (i = 0; i < json->response_num; i++) {
			if (!strcmp(json->response[i].objkey, objkey)) {
				if (!strcmp(json->response[i].objclass, OPH_JSON_GRID)) {
					add_frag = 1;
					break;
				}
				return OPH_JSON_BAD_PARAM_ERROR;
			}
		}
		if (add_frag) {
			void *tmp = json->response[i].objcontent;
			unsigned int index = json->response[i].objcontent_num;
			json->response[i].objcontent = realloc(json->response[i].objcontent, sizeof(oph_json_obj_grid) * (json->response[i].objcontent_num + 1));
			if (!json->response[i].objcontent) {
				json->response[i].objcontent = tmp;
				return OPH_JSON_MEMORY_ERROR;
			}
			((oph_json_obj_grid *) json->response[i].objcontent)[index].description = NULL;
			((oph_json_obj_grid *) json->response[i].objcontent)[index].fieldtypes = NULL;
			((oph_json_obj_grid *) json->response[i].objcontent)[index].fieldtypes_num = 0;
			((oph_json_obj_grid *) json->response[i].objcontent)[index].keys = NULL;
			((oph_json_obj_grid *) json->response[i].objcontent)[index].keys_num = 0;
			((oph_json_obj_grid *) json->response[i].objcontent)[index].title = NULL;
			((oph_json_obj_grid *) json->response[i].objcontent)[index].values = NULL;
			((oph_json_obj_grid *) json->response[i].objcontent)[index].values_num1 = 0;
			((oph_json_obj_grid *) json->response[i].objcontent)[index].values_num2 = 0;

			((oph_json_obj_grid *) json->response[i].objcontent)[index].title = (char *) strdup(title);
			if (!((oph_json_obj_grid *) json->response[i].objcontent)[index].title) {
				return OPH_JSON_MEMORY_ERROR;
			}

			json->response[i].objcontent_num++;

			if (description) {
				((oph_json_obj_grid *) json->response[i].objcontent)[index].description = (char *) strdup(description);
				if (!((oph_json_obj_grid *) json->response[i].objcontent)[index].description) {
					return OPH_JSON_MEMORY_ERROR;
				}
			}

			size_t k, q;

			((oph_json_obj_grid *) json->response[i].objcontent)[index].keys = (char **) malloc(sizeof(char *) * keys_num);
			if (!((oph_json_obj_grid *) json->response[i].objcontent)[index].keys) {
				return OPH_JSON_MEMORY_ERROR;
			}
			for (k = 0; k < (size_t) keys_num; k++) {
				for (q = 0; q < k; q++) {
					if (!strcmp(keys[q], keys[k])) {
						return OPH_JSON_BAD_PARAM_ERROR;
					}
				}
				((oph_json_obj_grid *) json->response[i].objcontent)[index].keys[k] = (char *) strdup(keys[k]);
				if (!((oph_json_obj_grid *) json->response[i].objcontent)[index].keys[k]) {
					return OPH_JSON_MEMORY_ERROR;
				}
				((oph_json_obj_grid *) json->response[i].objcontent)[index].keys_num++;
			}

			((oph_json_obj_grid *) json->response[i].objcontent)[index].fieldtypes = (char **) malloc(sizeof(char *) * fieldtypes_num);
			if (!((oph_json_obj_grid *) json->response[i].objcontent)[index].fieldtypes) {
				return OPH_JSON_MEMORY_ERROR;
			}
			for (k = 0; k < (size_t) fieldtypes_num; k++) {
				if (!oph_json_is_type_correct(fieldtypes[k])) {
					return OPH_JSON_BAD_PARAM_ERROR;
				}
				((oph_json_obj_grid *) json->response[i].objcontent)[index].fieldtypes[k] = (char *) strdup(fieldtypes[k]);
				if (!((oph_json_obj_grid *) json->response[i].objcontent)[index].fieldtypes[k]) {
					return OPH_JSON_MEMORY_ERROR;
				}
				((oph_json_obj_grid *) json->response[i].objcontent)[index].fieldtypes_num++;
			}

			if (keys_num != fieldtypes_num || (size_t) keys_num != ((oph_json_obj_grid *) json->response[i].objcontent)[index].keys_num
			    || (size_t) fieldtypes_num != ((oph_json_obj_grid *) json->response[i].objcontent)[index].fieldtypes_num) {
				return OPH_JSON_BAD_PARAM_ERROR;
			}

			((oph_json_obj_grid *) json->response[i].objcontent)[index].values_num2 = ((oph_json_obj_grid *) json->response[i].objcontent)[index].keys_num;
		} else {
			oph_json_response *tmp = json->response;
			unsigned int index = json->response_num;
			json->response = (oph_json_response *) realloc(json->response, sizeof(oph_json_response) * (json->response_num + 1));
			if (!json->response) {
				json->response = tmp;
				return OPH_JSON_MEMORY_ERROR;
			}
			json->response[index].objclass = NULL;
			json->response[index].objcontent = NULL;
			json->response[index].objcontent_num = 0;
			json->response[index].objkey = NULL;

			json->response[index].objclass = (char *) strdup(OPH_JSON_GRID);
			if (!json->response[index].objclass) {
				return OPH_JSON_MEMORY_ERROR;
			}

			json->response_num++;

			json->response[index].objkey = (char *) strdup(objkey);
			if (!json->response[index].objkey) {
				return OPH_JSON_MEMORY_ERROR;
			}
			if (oph_json_add_responseKey(json, objkey)) {
				return OPH_JSON_MEMORY_ERROR;
			}

			json->response[index].objcontent = malloc(sizeof(oph_json_obj_grid));
			if (!json->response[index].objcontent) {
				return OPH_JSON_MEMORY_ERROR;
			}
			((oph_json_obj_grid *) json->response[index].objcontent)[0].description = NULL;
			((oph_json_obj_grid *) json->response[index].objcontent)[0].fieldtypes = NULL;
			((oph_json_obj_grid *) json->response[index].objcontent)[0].fieldtypes_num = 0;
			((oph_json_obj_grid *) json->response[index].objcontent)[0].keys = NULL;
			((oph_json_obj_grid *) json->response[index].objcontent)[0].keys_num = 0;
			((oph_json_obj_grid *) json->response[index].objcontent)[0].title = NULL;
			((oph_json_obj_grid *) json->response[index].objcontent)[0].values = NULL;
			((oph_json_obj_grid *) json->response[index].objcontent)[0].values_num1 = 0;
			((oph_json_obj_grid *) json->response[index].objcontent)[0].values_num2 = 0;

			((oph_json_obj_grid *) json->response[index].objcontent)[0].title = (char *) strdup(title);
			if (!((oph_json_obj_grid *) json->response[index].objcontent)[0].title) {
				return OPH_JSON_MEMORY_ERROR;
			}

			json->response[index].objcontent_num++;

			if (description) {
				((oph_json_obj_grid *) json->response[index].objcontent)[0].description = (char *) strdup(description);
				if (!((oph_json_obj_grid *) json->response[index].objcontent)[0].description) {
					return OPH_JSON_MEMORY_ERROR;
				}
			}

			size_t k, q;

			((oph_json_obj_grid *) json->response[index].objcontent)[0].keys = (char **) malloc(sizeof(char *) * keys_num);
			if (!((oph_json_obj_grid *) json->response[index].objcontent)[0].keys) {
				return OPH_JSON_MEMORY_ERROR;
			}
			for (k = 0; k < (size_t) keys_num; k++) {
				for (q = 0; q < k; q++) {
					if (!strcmp(keys[q], keys[k])) {
						return OPH_JSON_BAD_PARAM_ERROR;
					}
				}
				((oph_json_obj_grid *) json->response[index].objcontent)[0].keys[k] = (char *) strdup(keys[k]);
				if (!((oph_json_obj_grid *) json->response[index].objcontent)[0].keys[k]) {
					return OPH_JSON_MEMORY_ERROR;
				}
				((oph_json_obj_grid *) json->response[index].objcontent)[0].keys_num++;
			}

			((oph_json_obj_grid *) json->response[index].objcontent)[0].fieldtypes = (char **) malloc(sizeof(char *) * fieldtypes_num);
			if (!((oph_json_obj_grid *) json->response[index].objcontent)[0].fieldtypes) {
				return OPH_JSON_MEMORY_ERROR;
			}
			for (k = 0; k < (size_t) fieldtypes_num; k++) {
				if (!oph_json_is_type_correct(fieldtypes[k])) {
					return OPH_JSON_BAD_PARAM_ERROR;
				}
				((oph_json_obj_grid *) json->response[index].objcontent)[0].fieldtypes[k] = (char *) strdup(fieldtypes[k]);
				if (!((oph_json_obj_grid *) json->response[index].objcontent)[0].fieldtypes[k]) {
					return OPH_JSON_MEMORY_ERROR;
				}
				((oph_json_obj_grid *) json->response[index].objcontent)[0].fieldtypes_num++;
			}

			if (keys_num != fieldtypes_num || (size_t) keys_num != ((oph_json_obj_grid *) json->response[index].objcontent)[0].keys_num
			    || (size_t) fieldtypes_num != ((oph_json_obj_grid *) json->response[index].objcontent)[0].fieldtypes_num) {
				return OPH_JSON_BAD_PARAM_ERROR;
			}

			((oph_json_obj_grid *) json->response[index].objcontent)[0].values_num2 = ((oph_json_obj_grid *) json->response[index].objcontent)[0].keys_num;
		}
	}

	return OPH_JSON_SUCCESS;
}

int oph_json_add_grid_row(oph_json * json, const char *objkey, const char **values)
{
	if (!json || !objkey || !values) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}

	if (json->response_num == 0) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}

	size_t i;
	int grid_present = 0;
	for (i = 0; i < json->response_num; i++) {
		if (!strcmp(json->response[i].objkey, objkey)) {
			if (!strcmp(json->response[i].objclass, OPH_JSON_GRID)) {
				grid_present = 1;
				break;
			}
			return OPH_JSON_BAD_PARAM_ERROR;
		}
	}
	if (grid_present) {
		if (json->response[i].objcontent_num < 1) {
			return OPH_JSON_BAD_PARAM_ERROR;
		}
		unsigned int index = 0;
		if (((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values_num1 == 0) {
			((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values = (char ***) malloc(sizeof(char **));
			if (!((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values) {
				return OPH_JSON_MEMORY_ERROR;
			}
			index = 0;
		} else {
			char ***tmp = ((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values;
			((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values =
			    (char ***) realloc(((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values,
					       sizeof(char **) * (((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values_num1 + 1));
			if (!((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values) {
				((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values = tmp;
				return OPH_JSON_MEMORY_ERROR;
			}
			index = ((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values_num1;
		}

		size_t k;

		((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values[index] =
		    (char **) malloc(sizeof(char *) * (((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values_num2));
		if (!((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values[index]) {
			return OPH_JSON_MEMORY_ERROR;
		}
		((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values_num1++;

		for (k = 0; k < ((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values_num2; k++) {
			((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values[index][k] = NULL;
		}
		for (k = 0; k < ((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values_num2; k++) {
			((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values[index][k] = (char *) strdup(values[k]);
			if (!((oph_json_obj_grid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].values[index][k]) {
				return OPH_JSON_MEMORY_ERROR;
			}
		}
	} else {
		return OPH_JSON_BAD_PARAM_ERROR;
	}

	return OPH_JSON_SUCCESS;
}
