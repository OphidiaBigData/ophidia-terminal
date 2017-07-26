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

#include "oph_json_multigrid.h"

/* Standard C99 headers */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

/***********OPH_JSON_OBJ_MULTIGRID INTERNAL FUNCTIONS***********/

// Free a multigrid object contents
int oph_json_free_multigrid(oph_json_obj_multigrid * obj)
{
	if (obj) {
		if (obj->colfieldtypes) {
			size_t i;
			for (i = 0; i < obj->colfieldtypes_num; i++) {
				if (obj->colfieldtypes[i]) {
					free(obj->colfieldtypes[i]);
					obj->colfieldtypes[i] = NULL;
				}
			}
			free(obj->colfieldtypes);
			obj->colfieldtypes = NULL;
		}
		obj->colfieldtypes_num = 0;
		if (obj->colkeys) {
			size_t i;
			for (i = 0; i < obj->colkeys_num; i++) {
				if (obj->colkeys[i]) {
					free(obj->colkeys[i]);
					obj->colkeys[i] = NULL;
				}
			}
			free(obj->colkeys);
			obj->colkeys = NULL;
		}
		obj->colkeys_num = 0;
		if (obj->colvalues) {
			size_t i;
			for (i = 0; i < obj->colvalues_num1; i++) {
				if (obj->colvalues[i]) {
					size_t j;
					for (j = 0; j < obj->colvalues_num2; j++) {
						if (obj->colvalues[i][j]) {
							free(obj->colvalues[i][j]);
							obj->colvalues[i][j] = NULL;
						}
					}
					free(obj->colvalues[i]);
					obj->colvalues[i] = NULL;
				}
			}
			free(obj->colvalues);
			obj->colvalues = NULL;
		}
		obj->colvalues_num1 = 0;
		obj->colvalues_num2 = 0;
		if (obj->description) {
			free(obj->description);
			obj->description = NULL;
		}
		if (obj->measurename) {
			free(obj->measurename);
			obj->measurename = NULL;
		}
		if (obj->measuretype) {
			free(obj->measuretype);
			obj->measuretype = NULL;
		}
		if (obj->measurevalues) {
			size_t i;
			for (i = 0; i < obj->measurevalues_num1; i++) {
				if (obj->measurevalues[i]) {
					size_t j;
					for (j = 0; j < obj->measurevalues_num2; j++) {
						if (obj->measurevalues[i][j]) {
							free(obj->measurevalues[i][j]);
							obj->measurevalues[i][j] = NULL;
						}
					}
					free(obj->measurevalues[i]);
					obj->measurevalues[i] = NULL;
				}
			}
			free(obj->measurevalues);
			obj->measurevalues = NULL;
		}
		obj->measurevalues_num1 = 0;
		obj->measurevalues_num2 = 0;
		if (obj->rowfieldtypes) {
			size_t i;
			for (i = 0; i < obj->rowfieldtypes_num; i++) {
				if (obj->rowfieldtypes[i]) {
					free(obj->rowfieldtypes[i]);
					obj->rowfieldtypes[i] = NULL;
				}
			}
			free(obj->rowfieldtypes);
			obj->rowfieldtypes = NULL;
		}
		obj->rowfieldtypes_num = 0;
		if (obj->rowkeys) {
			size_t i;
			for (i = 0; i < obj->rowkeys_num; i++) {
				if (obj->rowkeys[i]) {
					free(obj->rowkeys[i]);
					obj->rowkeys[i] = NULL;
				}
			}
			free(obj->rowkeys);
			obj->rowkeys = NULL;
		}
		obj->rowkeys_num = 0;
		if (obj->rowvalues) {
			size_t i;
			for (i = 0; i < obj->rowvalues_num1; i++) {
				if (obj->rowvalues[i]) {
					size_t j;
					for (j = 0; j < obj->rowvalues_num2; j++) {
						if (obj->rowvalues[i][j]) {
							free(obj->rowvalues[i][j]);
							obj->rowvalues[i][j] = NULL;
						}
					}
					free(obj->rowvalues[i]);
					obj->rowvalues[i] = NULL;
				}
			}
			free(obj->rowvalues);
			obj->rowvalues = NULL;
		}
		obj->rowvalues_num1 = 0;
		obj->rowvalues_num2 = 0;
		if (obj->title) {
			free(obj->title);
			obj->title = NULL;
		}
	}
	return OPH_JSON_SUCCESS;
}

/***********OPH_JSON_OBJ_MULTIGRID FUNCTIONS***********/

int oph_json_add_multigrid(oph_json * json, const char *objkey, const char *title, const char *description, const char **rowkeys, int rowkeys_num, const char **rowfieldtypes, int rowfieldtypes_num,
			   const char **colkeys, int colkeys_num, const char **colfieldtypes, int colfieldtypes_num, const char ***colvalues, int colvalues_num, const char *measurename,
			   const char *measuretype)
{
	if (!json || !objkey || !title || !rowkeys || rowkeys_num < 1 || !rowfieldtypes || rowfieldtypes_num < 1 || !colkeys || colkeys_num < 1 || !colfieldtypes || colfieldtypes_num < 1 || !colvalues
	    || colvalues_num < 1 || !measurename || !measuretype) {
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

		json->response[0].objclass = (char *) strdup(OPH_JSON_MULTIGRID);
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

		json->response[0].objcontent = malloc(sizeof(oph_json_obj_multigrid));
		if (!json->response[0].objcontent) {
			return OPH_JSON_MEMORY_ERROR;
		}
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colfieldtypes = NULL;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colfieldtypes_num = 0;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colkeys = NULL;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colkeys_num = 0;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colvalues = NULL;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colvalues_num1 = 0;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colvalues_num2 = 0;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].description = NULL;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].measurename = NULL;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].measuretype = NULL;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].measurevalues = NULL;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].measurevalues_num1 = 0;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].measurevalues_num2 = 0;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowfieldtypes = NULL;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowfieldtypes_num = 0;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowkeys = NULL;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowkeys_num = 0;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowvalues = NULL;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowvalues_num1 = 0;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowvalues_num2 = 0;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].title = NULL;

		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].title = (char *) strdup(title);
		if (!((oph_json_obj_multigrid *) json->response[0].objcontent)[0].title) {
			return OPH_JSON_MEMORY_ERROR;
		}

		json->response[0].objcontent_num++;

		if (description) {
			((oph_json_obj_multigrid *) json->response[0].objcontent)[0].description = (char *) strdup(description);
			if (!((oph_json_obj_multigrid *) json->response[0].objcontent)[0].description) {
				return OPH_JSON_MEMORY_ERROR;
			}
		}

		size_t k, q;

		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowkeys = (char **) malloc(sizeof(char *) * rowkeys_num);
		if (!((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowkeys) {
			return OPH_JSON_MEMORY_ERROR;
		}
		for (k = 0; k < (size_t) rowkeys_num; k++) {
			for (q = 0; q < k; q++) {
				if (!strcmp(rowkeys[q], rowkeys[k])) {
					return OPH_JSON_BAD_PARAM_ERROR;
				}
			}
			((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowkeys[k] = (char *) strdup(rowkeys[k]);
			if (!((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowkeys[k]) {
				return OPH_JSON_MEMORY_ERROR;
			}
			((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowkeys_num++;
		}

		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowfieldtypes = (char **) malloc(sizeof(char *) * rowfieldtypes_num);
		if (!((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowfieldtypes) {
			return OPH_JSON_MEMORY_ERROR;
		}
		for (k = 0; k < (size_t) rowfieldtypes_num; k++) {
			if (!oph_json_is_type_correct(rowfieldtypes[k])) {
				return OPH_JSON_BAD_PARAM_ERROR;
			}
			((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowfieldtypes[k] = (char *) strdup(rowfieldtypes[k]);
			if (!((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowfieldtypes[k]) {
				return OPH_JSON_MEMORY_ERROR;
			}
			((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowfieldtypes_num++;
		}

		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colkeys = (char **) malloc(sizeof(char *) * colkeys_num);
		if (!((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colkeys) {
			return OPH_JSON_MEMORY_ERROR;
		}
		for (k = 0; k < (size_t) colkeys_num; k++) {
			for (q = 0; q < k; q++) {
				if (!strcmp(colkeys[q], colkeys[k])) {
					return OPH_JSON_BAD_PARAM_ERROR;
				}
			}
			((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colkeys[k] = (char *) strdup(colkeys[k]);
			if (!((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colkeys[k]) {
				return OPH_JSON_MEMORY_ERROR;
			}
			((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colkeys_num++;
		}

		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colfieldtypes = (char **) malloc(sizeof(char *) * colfieldtypes_num);
		if (!((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colfieldtypes) {
			return OPH_JSON_MEMORY_ERROR;
		}
		for (k = 0; k < (size_t) colfieldtypes_num; k++) {
			if (!oph_json_is_type_correct(colfieldtypes[k])) {
				return OPH_JSON_BAD_PARAM_ERROR;
			}
			((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colfieldtypes[k] = (char *) strdup(colfieldtypes[k]);
			if (!((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colfieldtypes[k]) {
				return OPH_JSON_MEMORY_ERROR;
			}
			((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colfieldtypes_num++;
		}

		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colvalues = (char ***) malloc(sizeof(char **) * colvalues_num);
		if (!((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colvalues) {
			return OPH_JSON_MEMORY_ERROR;
		}
		for (k = 0; k < (size_t) colvalues_num; k++) {
			((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colvalues[k] = NULL;
		}
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colvalues_num1 = colvalues_num;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colvalues_num2 = colkeys_num;
		for (k = 0; k < (size_t) colvalues_num; k++) {
			((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colvalues[k] = (char **) malloc(sizeof(char *) * colkeys_num);
			if (!((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colvalues[k]) {
				return OPH_JSON_MEMORY_ERROR;
			}

			for (q = 0; q < (size_t) colkeys_num; q++) {
				((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colvalues[k][q] = NULL;
			}
			for (q = 0; q < (size_t) colkeys_num; q++) {
				((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colvalues[k][q] = (char *) strdup(colvalues[k][q]);
				if (!((oph_json_obj_multigrid *) json->response[0].objcontent)[0].colvalues[k][q]) {
					return OPH_JSON_MEMORY_ERROR;
				}
			}
		}

		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].measurename = (char *) strdup(measurename);
		if (!((oph_json_obj_multigrid *) json->response[0].objcontent)[0].measurename) {
			return OPH_JSON_MEMORY_ERROR;
		}

		if (!oph_json_is_measuretype_correct(measuretype)) {
			return OPH_JSON_BAD_PARAM_ERROR;
		}
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].measuretype = (char *) strdup(measuretype);
		if (!((oph_json_obj_multigrid *) json->response[0].objcontent)[0].measuretype) {
			return OPH_JSON_MEMORY_ERROR;
		}

		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].measurevalues_num2 = colvalues_num;
		((oph_json_obj_multigrid *) json->response[0].objcontent)[0].rowvalues_num2 = rowkeys_num;

	} else {
		size_t i;
		int add_frag = 0;
		for (i = 0; i < json->response_num; i++) {
			if (!strcmp(json->response[i].objkey, objkey)) {
				if (!strcmp(json->response[i].objclass, OPH_JSON_MULTIGRID)) {
					add_frag = 1;
					break;
				}
				return OPH_JSON_BAD_PARAM_ERROR;
			}
		}
		if (add_frag) {
			void *tmp = json->response[i].objcontent;
			unsigned int index = json->response[i].objcontent_num;
			json->response[i].objcontent = realloc(json->response[i].objcontent, sizeof(oph_json_obj_multigrid) * (json->response[i].objcontent_num + 1));
			if (!json->response[i].objcontent) {
				json->response[i].objcontent = tmp;
				return OPH_JSON_MEMORY_ERROR;
			}
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colfieldtypes = NULL;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colfieldtypes_num = 0;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colkeys = NULL;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colkeys_num = 0;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colvalues = NULL;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colvalues_num1 = 0;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colvalues_num2 = 0;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].description = NULL;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].measurename = NULL;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].measuretype = NULL;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].measurevalues = NULL;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].measurevalues_num1 = 0;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].measurevalues_num2 = 0;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowfieldtypes = NULL;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowfieldtypes_num = 0;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowkeys = NULL;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowkeys_num = 0;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowvalues = NULL;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowvalues_num1 = 0;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowvalues_num2 = 0;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].title = NULL;

			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].title = (char *) strdup(title);
			if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[index].title) {
				return OPH_JSON_MEMORY_ERROR;
			}

			json->response[i].objcontent_num++;

			if (description) {
				((oph_json_obj_multigrid *) json->response[i].objcontent)[index].description = (char *) strdup(description);
				if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[index].description) {
					return OPH_JSON_MEMORY_ERROR;
				}
			}

			size_t k, q;

			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowkeys = (char **) malloc(sizeof(char *) * rowkeys_num);
			if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowkeys) {
				return OPH_JSON_MEMORY_ERROR;
			}
			for (k = 0; k < (size_t) rowkeys_num; k++) {
				for (q = 0; q < k; q++) {
					if (!strcmp(rowkeys[q], rowkeys[k])) {
						return OPH_JSON_BAD_PARAM_ERROR;
					}
				}
				((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowkeys[k] = (char *) strdup(rowkeys[k]);
				if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowkeys[k]) {
					return OPH_JSON_MEMORY_ERROR;
				}
				((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowkeys_num++;
			}

			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowfieldtypes = (char **) malloc(sizeof(char *) * rowfieldtypes_num);
			if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowfieldtypes) {
				return OPH_JSON_MEMORY_ERROR;
			}
			for (k = 0; k < (size_t) rowfieldtypes_num; k++) {
				if (!oph_json_is_type_correct(rowfieldtypes[k])) {
					return OPH_JSON_BAD_PARAM_ERROR;
				}
				((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowfieldtypes[k] = (char *) strdup(rowfieldtypes[k]);
				if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowfieldtypes[k]) {
					return OPH_JSON_MEMORY_ERROR;
				}
				((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowfieldtypes_num++;
			}

			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colkeys = (char **) malloc(sizeof(char *) * colkeys_num);
			if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colkeys) {
				return OPH_JSON_MEMORY_ERROR;
			}
			for (k = 0; k < (size_t) colkeys_num; k++) {
				for (q = 0; q < k; q++) {
					if (!strcmp(colkeys[q], colkeys[k])) {
						return OPH_JSON_BAD_PARAM_ERROR;
					}
				}
				((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colkeys[k] = (char *) strdup(colkeys[k]);
				if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colkeys[k]) {
					return OPH_JSON_MEMORY_ERROR;
				}
				((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colkeys_num++;
			}

			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colfieldtypes = (char **) malloc(sizeof(char *) * colfieldtypes_num);
			if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colfieldtypes) {
				return OPH_JSON_MEMORY_ERROR;
			}
			for (k = 0; k < (size_t) colfieldtypes_num; k++) {
				if (!oph_json_is_type_correct(colfieldtypes[k])) {
					return OPH_JSON_BAD_PARAM_ERROR;
				}
				((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colfieldtypes[k] = (char *) strdup(colfieldtypes[k]);
				if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colfieldtypes[k]) {
					return OPH_JSON_MEMORY_ERROR;
				}
				((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colfieldtypes_num++;
			}

			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colvalues = (char ***) malloc(sizeof(char **) * colvalues_num);
			if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colvalues) {
				return OPH_JSON_MEMORY_ERROR;
			}
			for (k = 0; k < (size_t) colvalues_num; k++) {
				((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colvalues[k] = NULL;
			}
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colvalues_num1 = colvalues_num;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colvalues_num2 = colkeys_num;
			for (k = 0; k < (size_t) colvalues_num; k++) {
				((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colvalues[k] = (char **) malloc(sizeof(char *) * colkeys_num);
				if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colvalues[k]) {
					return OPH_JSON_MEMORY_ERROR;
				}

				for (q = 0; q < (size_t) colkeys_num; q++) {
					((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colvalues[k][q] = NULL;
				}
				for (q = 0; q < (size_t) colkeys_num; q++) {
					((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colvalues[k][q] = (char *) strdup(colvalues[k][q]);
					if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[index].colvalues[k][q]) {
						return OPH_JSON_MEMORY_ERROR;
					}
				}
			}

			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].measurename = (char *) strdup(measurename);
			if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[index].measurename) {
				return OPH_JSON_MEMORY_ERROR;
			}

			if (!oph_json_is_measuretype_correct(measuretype)) {
				return OPH_JSON_BAD_PARAM_ERROR;
			}
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].measuretype = (char *) strdup(measuretype);
			if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[index].measuretype) {
				return OPH_JSON_MEMORY_ERROR;
			}

			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].measurevalues_num2 = colvalues_num;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[index].rowvalues_num2 = rowkeys_num;

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

			json->response[index].objclass = (char *) strdup(OPH_JSON_MULTIGRID);
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

			json->response[index].objcontent = malloc(sizeof(oph_json_obj_multigrid));
			if (!json->response[index].objcontent) {
				return OPH_JSON_MEMORY_ERROR;
			}
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colfieldtypes = NULL;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colfieldtypes_num = 0;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colkeys = NULL;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colkeys_num = 0;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colvalues = NULL;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colvalues_num1 = 0;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colvalues_num2 = 0;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].description = NULL;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].measurename = NULL;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].measuretype = NULL;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].measurevalues = NULL;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].measurevalues_num1 = 0;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].measurevalues_num2 = 0;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowfieldtypes = NULL;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowfieldtypes_num = 0;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowkeys = NULL;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowkeys_num = 0;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowvalues = NULL;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowvalues_num1 = 0;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowvalues_num2 = 0;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].title = NULL;

			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].title = (char *) strdup(title);
			if (!((oph_json_obj_multigrid *) json->response[index].objcontent)[0].title) {
				return OPH_JSON_MEMORY_ERROR;
			}

			json->response[index].objcontent_num++;

			if (description) {
				((oph_json_obj_multigrid *) json->response[index].objcontent)[0].description = (char *) strdup(description);
				if (!((oph_json_obj_multigrid *) json->response[index].objcontent)[0].description) {
					return OPH_JSON_MEMORY_ERROR;
				}
			}

			size_t k, q;

			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowkeys = (char **) malloc(sizeof(char *) * rowkeys_num);
			if (!((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowkeys) {
				return OPH_JSON_MEMORY_ERROR;
			}
			for (k = 0; k < (size_t) rowkeys_num; k++) {
				for (q = 0; q < k; q++) {
					if (!strcmp(rowkeys[q], rowkeys[k])) {
						return OPH_JSON_BAD_PARAM_ERROR;
					}
				}
				((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowkeys[k] = (char *) strdup(rowkeys[k]);
				if (!((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowkeys[k]) {
					return OPH_JSON_MEMORY_ERROR;
				}
				((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowkeys_num++;
			}

			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowfieldtypes = (char **) malloc(sizeof(char *) * rowfieldtypes_num);
			if (!((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowfieldtypes) {
				return OPH_JSON_MEMORY_ERROR;
			}
			for (k = 0; k < (size_t) rowfieldtypes_num; k++) {
				if (!oph_json_is_type_correct(rowfieldtypes[k])) {
					return OPH_JSON_BAD_PARAM_ERROR;
				}
				((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowfieldtypes[k] = (char *) strdup(rowfieldtypes[k]);
				if (!((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowfieldtypes[k]) {
					return OPH_JSON_MEMORY_ERROR;
				}
				((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowfieldtypes_num++;
			}

			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colkeys = (char **) malloc(sizeof(char *) * colkeys_num);
			if (!((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colkeys) {
				return OPH_JSON_MEMORY_ERROR;
			}
			for (k = 0; k < (size_t) colkeys_num; k++) {
				for (q = 0; q < k; q++) {
					if (!strcmp(colkeys[q], colkeys[k])) {
						return OPH_JSON_BAD_PARAM_ERROR;
					}
				}
				((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colkeys[k] = (char *) strdup(colkeys[k]);
				if (!((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colkeys[k]) {
					return OPH_JSON_MEMORY_ERROR;
				}
				((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colkeys_num++;
			}

			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colfieldtypes = (char **) malloc(sizeof(char *) * colfieldtypes_num);
			if (!((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colfieldtypes) {
				return OPH_JSON_MEMORY_ERROR;
			}
			for (k = 0; k < (size_t) colfieldtypes_num; k++) {
				if (!oph_json_is_type_correct(colfieldtypes[k])) {
					return OPH_JSON_BAD_PARAM_ERROR;
				}
				((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colfieldtypes[k] = (char *) strdup(colfieldtypes[k]);
				if (!((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colfieldtypes[k]) {
					return OPH_JSON_MEMORY_ERROR;
				}
				((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colfieldtypes_num++;
			}

			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colvalues = (char ***) malloc(sizeof(char **) * colvalues_num);
			if (!((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colvalues) {
				return OPH_JSON_MEMORY_ERROR;
			}
			for (k = 0; k < (size_t) colvalues_num; k++) {
				((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colvalues[k] = NULL;
			}
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colvalues_num1 = colvalues_num;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colvalues_num2 = colkeys_num;
			for (k = 0; k < (size_t) colvalues_num; k++) {
				((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colvalues[k] = (char **) malloc(sizeof(char *) * colkeys_num);
				if (!((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colvalues[k]) {
					return OPH_JSON_MEMORY_ERROR;
				}

				for (q = 0; q < (size_t) colkeys_num; q++) {
					((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colvalues[k][q] = NULL;
				}
				for (q = 0; q < (size_t) colkeys_num; q++) {
					((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colvalues[k][q] = (char *) strdup(colvalues[k][q]);
					if (!((oph_json_obj_multigrid *) json->response[index].objcontent)[0].colvalues[k][q]) {
						return OPH_JSON_MEMORY_ERROR;
					}
				}
			}

			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].measurename = (char *) strdup(measurename);
			if (!((oph_json_obj_multigrid *) json->response[index].objcontent)[0].measurename) {
				return OPH_JSON_MEMORY_ERROR;
			}

			if (!oph_json_is_measuretype_correct(measuretype)) {
				return OPH_JSON_BAD_PARAM_ERROR;
			}
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].measuretype = (char *) strdup(measuretype);
			if (!((oph_json_obj_multigrid *) json->response[index].objcontent)[0].measuretype) {
				return OPH_JSON_MEMORY_ERROR;
			}

			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].measurevalues_num2 = colvalues_num;
			((oph_json_obj_multigrid *) json->response[index].objcontent)[0].rowvalues_num2 = rowkeys_num;

		}
	}

	return OPH_JSON_SUCCESS;
}

int oph_json_add_multigrid_row(oph_json * json, const char *objkey, const char **rowvalues, const char **measurevalues)
{
	if (!json || !objkey || !rowvalues || !measurevalues) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}

	if (json->response_num == 0) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}

	size_t i;
	int grid_present = 0;
	for (i = 0; i < json->response_num; i++) {
		if (!strcmp(json->response[i].objkey, objkey)) {
			if (!strcmp(json->response[i].objclass, OPH_JSON_MULTIGRID)) {
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
		if (((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues_num1 == 0) {
			((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues = (char ***) malloc(sizeof(char **));
			if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues) {
				return OPH_JSON_MEMORY_ERROR;
			}
			((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].rowvalues = (char ***) malloc(sizeof(char **));
			if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].rowvalues) {
				return OPH_JSON_MEMORY_ERROR;
			}
			index = 0;
		} else {
			char ***tmp = ((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues =
			    (char ***) realloc(((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues,
					       sizeof(char **) * (((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues_num1 + 1));
			if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues) {
				((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues = tmp;
				return OPH_JSON_MEMORY_ERROR;
			}
			char ***tmp2 = ((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].rowvalues;
			((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].rowvalues =
			    (char ***) realloc(((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].rowvalues,
					       sizeof(char **) * (((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues_num1 + 1));
			if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].rowvalues) {
				((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].rowvalues = tmp2;
				return OPH_JSON_MEMORY_ERROR;
			}
			index = ((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues_num1;
		}

		size_t k;

		((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues[index] =
		    (char **) malloc(sizeof(char *) * (((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues_num2));
		if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues[index]) {
			return OPH_JSON_MEMORY_ERROR;
		}
		((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues_num1++;
		for (k = 0; k < ((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues_num2; k++) {
			((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues[index][k] = NULL;
		}

		((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].rowvalues[index] =
		    (char **) malloc(sizeof(char *) * (((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].rowvalues_num2));
		if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].rowvalues[index]) {
			return OPH_JSON_MEMORY_ERROR;
		}
		((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].rowvalues_num1++;
		for (k = 0; k < ((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].rowvalues_num2; k++) {
			((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].rowvalues[index][k] = NULL;
		}

		for (k = 0; k < ((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues_num2; k++) {
			((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues[index][k] = (char *) strdup(measurevalues[k]);
			if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].measurevalues[index][k]) {
				return OPH_JSON_MEMORY_ERROR;
			}
		}

		for (k = 0; k < ((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].rowvalues_num2; k++) {
			((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].rowvalues[index][k] = (char *) strdup(rowvalues[k]);
			if (!((oph_json_obj_multigrid *) json->response[i].objcontent)[json->response[i].objcontent_num - 1].rowvalues[index][k]) {
				return OPH_JSON_MEMORY_ERROR;
			}
		}
	} else {
		return OPH_JSON_BAD_PARAM_ERROR;
	}

	return OPH_JSON_SUCCESS;
}
