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

#include "oph_json_common.h"
#include "oph_json_text.h"
#include "oph_json_grid.h"
#include "oph_json_multigrid.h"
#include "oph_json_tree.h"
#include "oph_json_graph.h"

/* Standard C99 headers */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

/* Jansson header to manipulate JSONs */
#include <jansson.h>

/***********OPH_JSON INTERNAL FUNCTIONS***********/

// Check if measure type does exist
int oph_json_is_measuretype_correct(const char *measuretype)
{
	int res;
	if (!strcmp(measuretype, OPH_JSON_INT))
		res = 1;
	else if (!strcmp(measuretype, OPH_JSON_LONG))
		res = 1;
	else if (!strcmp(measuretype, OPH_JSON_SHORT))
		res = 1;
	else if (!strcmp(measuretype, OPH_JSON_BYTE))
		res = 1;
	else if (!strcmp(measuretype, OPH_JSON_FLOAT))
		res = 1;
	else if (!strcmp(measuretype, OPH_JSON_DOUBLE))
		res = 1;
	else if (!strcmp(measuretype, OPH_JSON_STRING))
		res = 1;
	else if (!strcmp(measuretype, OPH_JSON_BLOB))
		res = 1;
	else
		res = 0;
	return res;
}

// Check if type does exist
int oph_json_is_type_correct(const char *type)
{
	int res;
	if (!strcmp(type, OPH_JSON_INT))
		res = 1;
	else if (!strcmp(type, OPH_JSON_LONG))
		res = 1;
	else if (!strcmp(type, OPH_JSON_SHORT))
		res = 1;
	else if (!strcmp(type, OPH_JSON_BYTE))
		res = 1;
	else if (!strcmp(type, OPH_JSON_FLOAT))
		res = 1;
	else if (!strcmp(type, OPH_JSON_DOUBLE))
		res = 1;
	else if (!strcmp(type, OPH_JSON_STRING))
		res = 1;
	else if (!strcmp(type, OPH_JSON_BLOB))
		res = 1;
	else
		res = 0;
	return res;
}

// Add an objkey to the responseKeyset if new
int oph_json_add_responseKey(oph_json * json, const char *responseKey)
{
	if (!json || !responseKey) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}
	if (json->responseKeyset_num == 0) {
		json->responseKeyset = (char **) malloc(sizeof(char *));
		if (!json->responseKeyset) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->responseKeyset[0] = (char *) strdup(responseKey);
		if (!json->responseKeyset[0]) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->responseKeyset_num++;
	} else {
		int i;
		for (i = 0; i < (int) json->responseKeyset_num; i++) {
			if (!strcmp(json->responseKeyset[i], responseKey)) {
				return OPH_JSON_BAD_PARAM_ERROR;
			}
		}
		char **tmp = json->responseKeyset;
		json->responseKeyset = (char **) realloc(json->responseKeyset, sizeof(char *) * (json->responseKeyset_num + 1));
		if (!json->responseKeyset) {
			json->responseKeyset = tmp;
			return OPH_JSON_MEMORY_ERROR;
		}
		json->responseKeyset[json->responseKeyset_num] = (char *) strdup(responseKey);
		if (!json->responseKeyset[json->responseKeyset_num]) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->responseKeyset_num++;
	}
	return OPH_JSON_SUCCESS;
}

// Free consumers
int oph_json_free_consumers(oph_json * json)
{
	if (!json) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}
	if (json->consumers) {
		int i;
		for (i = 0; i < (int) json->consumers_num; i++) {
			if (json->consumers[i]) {
				free(json->consumers[i]);
				json->consumers[i] = NULL;
			}
		}
		free(json->consumers);
		json->consumers = NULL;
	}
	json->consumers_num = 0;
	return OPH_JSON_SUCCESS;
}

// Free responseKeyset
int oph_json_free_responseKeyset(oph_json * json)
{
	if (!json) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}
	if (json->responseKeyset) {
		int i;
		for (i = 0; i < (int) json->responseKeyset_num; i++) {
			if (json->responseKeyset[i]) {
				free(json->responseKeyset[i]);
				json->responseKeyset[i] = NULL;
			}
		}
		free(json->responseKeyset);
		json->responseKeyset = NULL;
	}
	json->responseKeyset_num = 0;
	return OPH_JSON_SUCCESS;
}

// Free source
int oph_json_free_source(oph_json * json)
{
	if (!json) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}
	if (json->source) {
		if (json->source->description) {
			free(json->source->description);
			json->source->description = NULL;
		}
		if (json->source->keys) {
			int i;
			for (i = 0; i < (int) json->source->keys_num; i++) {
				if (json->source->keys[i]) {
					free(json->source->keys[i]);
					json->source->keys[i] = NULL;
				}
			}
			free(json->source->keys);
			json->source->keys = NULL;
		}
		json->source->keys_num = 0;
		if (json->source->producer) {
			free(json->source->producer);
			json->source->producer = NULL;
		}
		if (json->source->srckey) {
			free(json->source->srckey);
			json->source->srckey = NULL;
		}
		if (json->source->srcname) {
			free(json->source->srcname);
			json->source->srcname = NULL;
		}
		if (json->source->srcurl) {
			free(json->source->srcurl);
			json->source->srcurl = NULL;
		}
		if (json->source->values) {
			int i;
			for (i = 0; i < (int) json->source->values_num; i++) {
				if (json->source->values[i]) {
					free(json->source->values[i]);
					json->source->values[i] = NULL;
				}
			}
			free(json->source->values);
			json->source->values = NULL;
		}
		json->source->values_num = 0;

		free(json->source);
		json->source = NULL;
	}
	return OPH_JSON_SUCCESS;
}

// Free extra
int oph_json_free_extra(oph_json * json)
{
	if (!json) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}
	if (json->extra) {
		if (json->extra->keys) {
			int i;
			for (i = 0; i < (int) json->extra->keys_num; i++) {
				if (json->extra->keys[i]) {
					free(json->extra->keys[i]);
					json->extra->keys[i] = NULL;
				}
			}
			free(json->extra->keys);
			json->extra->keys = NULL;
		}
		json->extra->keys_num = 0;
		if (json->extra->values) {
			int i;
			for (i = 0; i < (int) json->extra->values_num; i++) {
				if (json->extra->values[i]) {
					free(json->extra->values[i]);
					json->extra->values[i] = NULL;
				}
			}
			free(json->extra->values);
			json->extra->values = NULL;
		}
		json->extra->values_num = 0;

		free(json->extra);
		json->extra = NULL;
	}
	return OPH_JSON_SUCCESS;
}

// Free response
int oph_json_free_response(oph_json * json)
{
	if (!json) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}
	if (json->response) {
		int i;
		for (i = 0; i < (int) json->response_num; i++) {
			if (json->response[i].objkey) {
				free(json->response[i].objkey);
				json->response[i].objkey = NULL;
			}
			if (json->response[i].objcontent) {
				int j;
				if (!strcmp(json->response[i].objclass, OPH_JSON_TEXT)) {
					for (j = 0; j < (int) json->response[i].objcontent_num; j++) {
						oph_json_free_text(&((oph_json_obj_text *) json->response[i].objcontent)[j]);
					}
				} else if (!strcmp(json->response[i].objclass, OPH_JSON_GRID)) {
					for (j = 0; j < (int) json->response[i].objcontent_num; j++) {
						oph_json_free_grid((&((oph_json_obj_grid *) json->response[i].objcontent)[j]));
					}
				} else if (!strcmp(json->response[i].objclass, OPH_JSON_MULTIGRID)) {
					for (j = 0; j < (int) json->response[i].objcontent_num; j++) {
						oph_json_free_multigrid((&((oph_json_obj_multigrid *) json->response[i].objcontent)[j]));
					}
				} else if (!strcmp(json->response[i].objclass, OPH_JSON_TREE)) {
					for (j = 0; j < (int) json->response[i].objcontent_num; j++) {
						oph_json_free_tree((&((oph_json_obj_tree *) json->response[i].objcontent)[j]));
					}
				} else if (!strcmp(json->response[i].objclass, OPH_JSON_DGRAPH) || !strcmp(json->response[i].objclass, OPH_JSON_GRAPH)) {
					for (j = 0; j < (int) json->response[i].objcontent_num; j++) {
						oph_json_free_graph((&((oph_json_obj_graph *) json->response[i].objcontent)[j]));
					}
				}
				free(json->response[i].objcontent);
				json->response[i].objcontent = NULL;
			}
			json->response[i].objcontent_num = 0;
			if (json->response[i].objclass) {
				free(json->response[i].objclass);
				json->response[i].objclass = NULL;
			}
		}
		free(json->response);
		json->response = NULL;
	}
	json->response_num = 0;
	return OPH_JSON_SUCCESS;
}

/***********OPH_JSON FUNCTIONS***********/

int oph_json_alloc(oph_json ** json)
{
	*json = (oph_json *) malloc(sizeof(oph_json));
	if (!*json) {
		return OPH_JSON_MEMORY_ERROR;
	}
	(*json)->consumers = NULL;
	(*json)->consumers_num = 0;
	(*json)->response = NULL;
	(*json)->responseKeyset = NULL;
	(*json)->responseKeyset_num = 0;
	(*json)->response_num = 0;
	(*json)->source = NULL;
	(*json)->extra = NULL;
	return OPH_JSON_SUCCESS;
}

int oph_json_free(oph_json * json)
{
	if (json) {
		oph_json_free_consumers(json);
		oph_json_free_response(json);
		oph_json_free_responseKeyset(json);
		oph_json_free_source(json);
		oph_json_free_extra(json);
		free(json);
		json = NULL;
	}
	return OPH_JSON_SUCCESS;
}

int oph_json_add_consumer(oph_json * json, const char *consumer)
{
	if (!json || !consumer) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}
	if (json->consumers_num == 0) {
		json->consumers = (char **) malloc(sizeof(char *));
		if (!json->consumers) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->consumers[0] = (char *) strdup(consumer);
		if (!json->consumers[0]) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->consumers_num++;
	} else {
		int i;
		for (i = 0; i < (int) json->consumers_num; i++) {
			if (!strcmp(json->consumers[i], consumer)) {
				return OPH_JSON_BAD_PARAM_ERROR;
			}
		}
		char **tmp = json->consumers;
		json->consumers = (char **) realloc(json->consumers, sizeof(char *) * (json->consumers_num + 1));
		if (!json->consumers) {
			json->consumers = tmp;
			return OPH_JSON_MEMORY_ERROR;
		}
		json->consumers[json->consumers_num] = (char *) strdup(consumer);
		if (!json->consumers[json->consumers_num]) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->consumers_num++;
	}
	return OPH_JSON_SUCCESS;
}

int oph_json_set_source(oph_json * json, const char *srckey, const char *srcname, const char *srcurl, const char *description, const char *producer)
{
	if (!json || !srckey || !srcname) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}
	if (json->source) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}
	json->source = (oph_json_source *) malloc(sizeof(oph_json_source));
	if (!json->source) {
		return OPH_JSON_MEMORY_ERROR;
	}
	json->source->description = NULL;
	json->source->keys = NULL;
	json->source->keys_num = 0;
	json->source->producer = NULL;
	json->source->srckey = NULL;
	json->source->srcname = NULL;
	json->source->srcurl = NULL;
	json->source->values = NULL;
	json->source->values_num = 0;

	json->source->srckey = (char *) strdup(srckey);
	if (!json->source->srckey) {
		return OPH_JSON_MEMORY_ERROR;
	}
	json->source->srcname = (char *) strdup(srcname);
	if (!json->source->srcname) {
		return OPH_JSON_MEMORY_ERROR;
	}

	if (srcurl) {
		json->source->srcurl = (char *) strdup(srcurl);
		if (!json->source->srcurl) {
			return OPH_JSON_MEMORY_ERROR;
		}
	}
	if (description) {
		json->source->description = (char *) strdup(description);
		if (!json->source->description) {
			return OPH_JSON_MEMORY_ERROR;
		}
	}
	if (producer) {
		json->source->producer = (char *) strdup(producer);
		if (!json->source->producer) {
			return OPH_JSON_MEMORY_ERROR;
		}
	}

	return OPH_JSON_SUCCESS;
}

int oph_json_add_source_detail(oph_json * json, const char *key, const char *value)
{
	if (!json || !key || !value) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}

	if (!json->source) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}

	if (json->source->keys_num == 0) {
		json->source->keys = (char **) malloc(sizeof(char *));
		if (!json->source->keys) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->source->keys[0] = (char *) strdup(key);
		if (!json->source->keys[0]) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->source->keys_num++;
		json->source->values = (char **) malloc(sizeof(char *));
		if (!json->source->values) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->source->values[0] = (char *) strdup(value);
		if (!json->source->values[0]) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->source->values_num++;
	} else {
		int i;
		for (i = 0; i < (int) json->source->keys_num; i++) {
			if (!strcmp(json->source->keys[i], key)) {
				return OPH_JSON_BAD_PARAM_ERROR;
			}
		}
		char **tmp = json->source->keys;
		json->source->keys = (char **) realloc(json->source->keys, sizeof(char *) * (json->source->keys_num + 1));
		if (!json->source->keys) {
			json->source->keys = tmp;
			return OPH_JSON_MEMORY_ERROR;
		}
		json->source->keys[json->source->keys_num] = (char *) strdup(key);
		if (!json->source->keys[json->source->keys_num]) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->source->keys_num++;
		char **tmp2 = json->source->values;
		json->source->values = (char **) realloc(json->source->values, sizeof(char *) * (json->source->values_num + 1));
		if (!json->source->values) {
			json->source->values = tmp2;
			return OPH_JSON_MEMORY_ERROR;
		}
		json->source->values[json->source->values_num] = (char *) strdup(value);
		if (!json->source->values[json->source->values_num]) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->source->values_num++;
	}

	return OPH_JSON_SUCCESS;
}

int oph_json_add_extra_detail(oph_json * json, const char *key, const char *value)
{
	if (!json || !key || !value) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}

	if (!json->extra) {
		json->extra = (oph_json_extra *) malloc(sizeof(oph_json_extra));
		if (!json->extra) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->extra->keys = NULL;
		json->extra->keys_num = 0;
		json->extra->values = NULL;
		json->extra->values_num = 0;
	}

	if (json->extra->keys_num == 0) {
		json->extra->keys = (char **) malloc(sizeof(char *));
		if (!json->extra->keys) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->extra->keys[0] = (char *) strdup(key);
		if (!json->extra->keys[0]) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->extra->keys_num++;
		json->extra->values = (char **) malloc(sizeof(char *));
		if (!json->extra->values) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->extra->values[0] = (char *) strdup(value);
		if (!json->extra->values[0]) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->extra->values_num++;
	} else {
		int i;
		for (i = 0; i < (int) json->extra->keys_num; i++) {
			if (!strcmp(json->extra->keys[i], key)) {
				return OPH_JSON_BAD_PARAM_ERROR;
			}
		}
		char **tmp = json->extra->keys;
		json->extra->keys = (char **) realloc(json->extra->keys, sizeof(char *) * (json->extra->keys_num + 1));
		if (!json->extra->keys) {
			json->extra->keys = tmp;
			return OPH_JSON_MEMORY_ERROR;
		}
		json->extra->keys[json->extra->keys_num] = (char *) strdup(key);
		if (!json->extra->keys[json->extra->keys_num]) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->extra->keys_num++;
		char **tmp2 = json->extra->values;
		json->extra->values = (char **) realloc(json->extra->values, sizeof(char *) * (json->extra->values_num + 1));
		if (!json->extra->values) {
			json->extra->values = tmp2;
			return OPH_JSON_MEMORY_ERROR;
		}
		json->extra->values[json->extra->values_num] = (char *) strdup(value);
		if (!json->extra->values[json->extra->values_num]) {
			return OPH_JSON_MEMORY_ERROR;
		}
		json->extra->values_num++;
	}

	return OPH_JSON_SUCCESS;
}

int oph_json_from_json_string(oph_json ** json, char **jstring)
{
	if (!jstring || !*jstring || !json) {
		return OPH_JSON_BAD_PARAM_ERROR;
	}

	*json = NULL;

	// ALLOC OPH_JSON
	if (oph_json_alloc(json)) {
		if (*jstring) {
			free(*jstring);
			*jstring = NULL;
		}
		return OPH_JSON_MEMORY_ERROR;
	}

	/* LOAD JSON_T */
	json_t *jansson = json_loads((const char *) *jstring, 0, NULL);
	if (!jansson) {
		if (*jstring) {
			free(*jstring);
			*jstring = NULL;
		}
		return OPH_JSON_GENERIC_ERROR;
	}
	// JSON string no more necessary
	if (*jstring) {
		free(*jstring);
		*jstring = NULL;
	}

	json_t *extra = NULL;
	json_t *source = NULL;
	json_t *consumers = NULL;
	json_t *response = NULL;

	//GET SOURCE DATA FROM JSON_T
	json_unpack(jansson, "{s?o}", "source", &source);
	if (source) {
		char *srckey = NULL, *srcname = NULL, *srcurl = NULL, *description = NULL, *producer = NULL;
		json_unpack(source, "{s?s,s?s,s?s,s?s,s?s}", "srckey", &srckey, "srcname", &srcname, "srcurl", &srcurl, "description", &description, "producer", &producer);

		if (oph_json_set_source(*json, (const char *) srckey, (const char *) srcname, (const char *) srcurl, (const char *) description, (const char *) producer)) {
			if (jansson)
				json_decref(jansson);
			return OPH_JSON_GENERIC_ERROR;
		}

		json_t *keys = NULL;
		json_t *values = NULL;
		json_unpack(source, "{s?o,s?o}", "keys", &keys, "values", &values);

		if (keys && values) {
			char *key = NULL, *value = NULL;
			size_t index;
			for (index = 0; index < json_array_size(keys); index++) {
				json_unpack(json_array_get(keys, index), "s", &key);
				json_unpack(json_array_get(values, index), "s", &value);

				if (oph_json_add_source_detail(*json, (const char *) key, (const char *) value)) {
					if (jansson)
						json_decref(jansson);
					return OPH_JSON_GENERIC_ERROR;
				}
			}
		} else if ((keys && !values) || (!keys && values)) {
			if (jansson)
				json_decref(jansson);
			return OPH_JSON_GENERIC_ERROR;
		}
	}
	//GET CONSUMERS DATA FROM JSON_T
	json_unpack(jansson, "{s?o}", "consumers", &consumers);
	if (consumers) {
		char *consumer = NULL;
		size_t index;
		for (index = 0; index < json_array_size(consumers); index++) {
			json_unpack(json_array_get(consumers, index), "s", &consumer);

			if (oph_json_add_consumer(*json, (const char *) consumer)) {
				if (jansson)
					json_decref(jansson);
				return OPH_JSON_GENERIC_ERROR;
			}
		}
	}
	//GET RESPONSE DATA FROM JSON_T
	json_unpack(jansson, "{s?o}", "response", &response);
	if (response) {
		json_t *obj = NULL;
		size_t index1;
		for (index1 = 0; index1 < json_array_size(response); index1++) {
			json_unpack(json_array_get(response, index1), "o", &obj);

			if (obj) {
				char *objkey = NULL, *objclass = NULL;
				json_t *objcontent = NULL;
				json_unpack(obj, "{s?s,s?s,s?o}", "objclass", &objclass, "objkey", &objkey, "objcontent", &objcontent);
				if (!objkey || !objclass || !objcontent) {
					if (jansson)
						json_decref(jansson);
					return OPH_JSON_GENERIC_ERROR;
				}

				if (!strcmp(objclass, OPH_JSON_TEXT)) {	//OBJCLASS TEXT
					json_t *objcontentfrag = NULL;
					size_t index2;
					for (index2 = 0; index2 < json_array_size(objcontent); index2++) {
						json_unpack(json_array_get(objcontent, index2), "o", &objcontentfrag);

						if (objcontentfrag) {
							char *title = NULL, *message = NULL;
							json_unpack(objcontentfrag, "{s?s,s?s}", "title", &title, "message", &message);

							if (oph_json_add_text(*json, (const char *) objkey, (const char *) title, (const char *) message)) {
								if (jansson)
									json_decref(jansson);
								return OPH_JSON_GENERIC_ERROR;
							}
						}
					}
				} else if (!strcmp(objclass, OPH_JSON_GRID)) {	//OBJCLASS GRID
					json_t *objcontentfrag = NULL;
					size_t index2;
					for (index2 = 0; index2 < json_array_size(objcontent); index2++) {
						json_unpack(json_array_get(objcontent, index2), "o", &objcontentfrag);

						if (objcontentfrag) {
							char *title = NULL, *description = NULL, **keys = NULL, **fieldtypes = NULL;
							int keys_num = 0, fieldtypes_num = 0;
							json_t *rowkeys = NULL;
							json_t *rowfieldtypes = NULL;
							json_unpack(objcontentfrag, "{s?s,s?s,s?o,s?o}", "title", &title, "description", &description, "rowkeys", &rowkeys, "rowfieldtypes",
								    &rowfieldtypes);
							if (!rowkeys || !rowfieldtypes) {
								if (jansson)
									json_decref(jansson);
								return OPH_JSON_MEMORY_ERROR;
							}

							keys_num = json_array_size(rowkeys);
							fieldtypes_num = json_array_size(rowfieldtypes);

							// fill keys and fieldtypes
							keys = (char **) calloc(keys_num, sizeof(char *));
							if (!keys) {
								if (jansson)
									json_decref(jansson);
								return OPH_JSON_MEMORY_ERROR;
							}
							fieldtypes = (char **) calloc(fieldtypes_num, sizeof(char *));
							if (!fieldtypes) {
								if (keys)
									free(keys);
								if (jansson)
									json_decref(jansson);
								return OPH_JSON_MEMORY_ERROR;
							}
							int index3;
							for (index3 = 0; index3 < keys_num; index3++) {
								json_unpack(json_array_get(rowkeys, index3), "s", &(keys[index3]));
								if (!keys[index3]) {
									if (keys)
										free(keys);
									if (fieldtypes)
										free(fieldtypes);
									if (jansson)
										json_decref(jansson);
									return OPH_JSON_GENERIC_ERROR;
								}
							}
							for (index3 = 0; index3 < fieldtypes_num; index3++) {
								json_unpack(json_array_get(rowfieldtypes, index3), "s", &(fieldtypes[index3]));
								if (!fieldtypes[index3]) {
									if (keys)
										free(keys);
									if (fieldtypes)
										free(fieldtypes);
									if (jansson)
										json_decref(jansson);
									return OPH_JSON_GENERIC_ERROR;
								}
							}

							// add grid
							if (oph_json_add_grid
							    (*json, (const char *) objkey, (const char *) title, (const char *) description, (const char **) keys, (int) keys_num,
							     (const char **) fieldtypes, (int) fieldtypes_num)) {
								if (keys)
									free(keys);
								if (fieldtypes)
									free(fieldtypes);
								if (jansson)
									json_decref(jansson);
								return OPH_JSON_GENERIC_ERROR;
							}
							if (keys)
								free(keys);
							if (fieldtypes)
								free(fieldtypes);

							json_t *rowvalues = NULL;
							json_unpack(objcontentfrag, "{s?o}", "rowvalues", &rowvalues);
							if (rowvalues) {
								size_t index4;
								json_t *rowvalues_i = NULL;
								for (index4 = 0; index4 < json_array_size(rowvalues); index4++) {	// for each row of values
									json_unpack(json_array_get(rowvalues, index4), "o", &rowvalues_i);	// get i-th row
									if (rowvalues_i) {
										char **values = NULL;
										int values_num = 0;

										values_num = json_array_size(rowvalues_i);

										// fill row with values
										values = (char **) calloc(values_num, sizeof(char *));
										if (!values) {
											if (jansson)
												json_decref(jansson);
											return OPH_JSON_MEMORY_ERROR;
										}
										int index5;
										for (index5 = 0; index5 < values_num; index5++) {
											json_unpack(json_array_get(rowvalues_i, index5), "s", &(values[index5]));
											if (!values[index5]) {
												if (values)
													free(values);
												if (jansson)
													json_decref(jansson);
												return OPH_JSON_GENERIC_ERROR;
											}
										}

										// add grid row
										if (oph_json_add_grid_row(*json, (const char *) objkey, (const char **) values)) {
											if (values)
												free(values);
											if (jansson)
												json_decref(jansson);
											return OPH_JSON_GENERIC_ERROR;
										}
										if (values)
											free(values);
									}
								}
							}
						}
					}
				} else if (!strcmp(objclass, OPH_JSON_MULTIGRID)) {	//OBJCLASS MULTIGRID
					json_t *objcontentfrag = NULL;
					size_t index2;
					for (index2 = 0; index2 < json_array_size(objcontent); index2++) {
						json_unpack(json_array_get(objcontent, index2), "o", &objcontentfrag);

						if (objcontentfrag) {
							char *title = NULL, *description = NULL, **row_keys = NULL, **row_fieldtypes = NULL, **col_keys = NULL, **col_fieldtypes = NULL, ***col_values =
							    NULL, *measurename = NULL, *measuretype = NULL;
							int row_keys_num = 0, row_fieldtypes_num = 0, col_keys_num = 0, col_fieldtypes_num = 0, col_values_num = 0;
							json_t *rowkeys = NULL;
							json_t *rowfieldtypes = NULL;
							json_t *colkeys = NULL;
							json_t *colfieldtypes = NULL;
							json_t *colvalues = NULL;
							json_unpack(objcontentfrag, "{s?s,s?s,s?o,s?o,s?o,s?o,s?o,s?s,s?s}", "title", &title, "description", &description, "rowkeys", &rowkeys,
								    "rowfieldtypes", &rowfieldtypes, "colkeys", &colkeys, "colfieldtypes", &colfieldtypes, "colvalues", &colvalues, "measurename",
								    &measurename, "measuretype", &measuretype);
							if (!rowkeys || !rowfieldtypes || !colkeys || !colfieldtypes || !colvalues) {
								if (jansson)
									json_decref(jansson);
								return OPH_JSON_MEMORY_ERROR;
							}

							row_keys_num = json_array_size(rowkeys);
							row_fieldtypes_num = json_array_size(rowfieldtypes);
							col_keys_num = json_array_size(colkeys);
							col_fieldtypes_num = json_array_size(colfieldtypes);
							col_values_num = json_array_size(colvalues);

							// alloc row_keys,row_fieldtypes,col_keys and col_fieldtypes
							row_keys = (char **) calloc(row_keys_num, sizeof(char *));
							if (!row_keys) {
								if (jansson)
									json_decref(jansson);
								return OPH_JSON_MEMORY_ERROR;
							}
							row_fieldtypes = (char **) calloc(row_fieldtypes_num, sizeof(char *));
							if (!row_fieldtypes) {
								if (row_keys)
									free(row_keys);
								if (jansson)
									json_decref(jansson);
								return OPH_JSON_MEMORY_ERROR;
							}
							col_keys = (char **) calloc(col_keys_num, sizeof(char *));
							if (!col_keys) {
								if (row_keys)
									free(row_keys);
								if (row_fieldtypes)
									free(row_fieldtypes);
								if (jansson)
									json_decref(jansson);
								return OPH_JSON_MEMORY_ERROR;
							}
							col_fieldtypes = (char **) calloc(col_fieldtypes_num, sizeof(char *));
							if (!col_fieldtypes) {
								if (row_keys)
									free(row_keys);
								if (row_fieldtypes)
									free(row_fieldtypes);
								if (col_keys)
									free(col_keys);
								if (jansson)
									json_decref(jansson);
								return OPH_JSON_MEMORY_ERROR;
							}
							// fill row_keys,row_fieldtypes,col_keys and col_fieldtypes
							int index3;
							for (index3 = 0; index3 < row_keys_num; index3++) {
								json_unpack(json_array_get(rowkeys, index3), "s", &(row_keys[index3]));
								if (!row_keys[index3]) {
									if (row_keys)
										free(row_keys);
									if (row_fieldtypes)
										free(row_fieldtypes);
									if (col_keys)
										free(col_keys);
									if (col_fieldtypes)
										free(col_fieldtypes);
									if (jansson)
										json_decref(jansson);
									return OPH_JSON_GENERIC_ERROR;
								}
							}
							for (index3 = 0; index3 < row_fieldtypes_num; index3++) {
								json_unpack(json_array_get(rowfieldtypes, index3), "s", &(row_fieldtypes[index3]));
								if (!row_fieldtypes[index3]) {
									if (row_keys)
										free(row_keys);
									if (row_fieldtypes)
										free(row_fieldtypes);
									if (col_keys)
										free(col_keys);
									if (col_fieldtypes)
										free(col_fieldtypes);
									if (jansson)
										json_decref(jansson);
									return OPH_JSON_GENERIC_ERROR;
								}
							}
							for (index3 = 0; index3 < col_keys_num; index3++) {
								json_unpack(json_array_get(colkeys, index3), "s", &(col_keys[index3]));
								if (!col_keys[index3]) {
									if (row_keys)
										free(row_keys);
									if (row_fieldtypes)
										free(row_fieldtypes);
									if (col_keys)
										free(col_keys);
									if (col_fieldtypes)
										free(col_fieldtypes);
									if (jansson)
										json_decref(jansson);
									return OPH_JSON_GENERIC_ERROR;
								}
							}
							for (index3 = 0; index3 < col_fieldtypes_num; index3++) {
								json_unpack(json_array_get(colfieldtypes, index3), "s", &(col_fieldtypes[index3]));
								if (!col_fieldtypes[index3]) {
									if (row_keys)
										free(row_keys);
									if (row_fieldtypes)
										free(row_fieldtypes);
									if (col_keys)
										free(col_keys);
									if (col_fieldtypes)
										free(col_fieldtypes);
									if (jansson)
										json_decref(jansson);
									return OPH_JSON_GENERIC_ERROR;
								}
							}

							// alloc col_values
							col_values = (char ***) calloc(col_values_num, sizeof(char **));
							if (!col_values) {
								if (row_keys)
									free(row_keys);
								if (row_fieldtypes)
									free(row_fieldtypes);
								if (col_keys)
									free(col_keys);
								if (col_fieldtypes)
									free(col_fieldtypes);
								if (jansson)
									json_decref(jansson);
								return OPH_JSON_MEMORY_ERROR;
							}
							// fill col_values
							json_t *colvalues_i = NULL;
							int q;
							for (q = 0; q < col_values_num; q++) {
								json_unpack(json_array_get(colvalues, q), "o", &colvalues_i);
								if (colvalues_i) {
									int colvalues_i_num = json_array_size(colvalues_i);
									col_values[q] = (char **) calloc(colvalues_i_num, sizeof(char *));
									if (!col_values[q]) {
										int n;
										for (n = 0; n < q; n++) {
											if (col_values[n])
												free(col_values[n]);
										}
										if (col_values)
											free(col_values);
										if (row_keys)
											free(row_keys);
										if (row_fieldtypes)
											free(row_fieldtypes);
										if (col_keys)
											free(col_keys);
										if (col_fieldtypes)
											free(col_fieldtypes);
										if (jansson)
											json_decref(jansson);
										return OPH_JSON_MEMORY_ERROR;
									}

									int index4;
									for (index4 = 0; index4 < colvalues_i_num; index4++) {
										json_unpack(json_array_get(colvalues_i, index4), "s", &(col_values[q][index4]));
										if (!col_values[q][index4]) {
											int n;
											for (n = 0; n < q; n++) {
												if (col_values[n])
													free(col_values[n]);
											}
											if (col_values)
												free(col_values);
											if (row_keys)
												free(row_keys);
											if (row_fieldtypes)
												free(row_fieldtypes);
											if (col_keys)
												free(col_keys);
											if (col_fieldtypes)
												free(col_fieldtypes);
											if (jansson)
												json_decref(jansson);
											return OPH_JSON_GENERIC_ERROR;
										}
									}
								}
							}

							// add multigrid
							if (oph_json_add_multigrid
							    (*json, (const char *) objkey, (const char *) title, (const char *) description, (const char **) row_keys, (int) row_keys_num,
							     (const char **) row_fieldtypes, (int) row_fieldtypes_num, (const char **) col_keys, (int) col_keys_num, (const char **) col_fieldtypes,
							     (int) col_fieldtypes_num, (const char ***) col_values, (int) col_values_num, (const char *) measurename, (const char *) measuretype)) {
								if (row_keys)
									free(row_keys);
								if (row_fieldtypes)
									free(row_fieldtypes);
								if (col_keys)
									free(col_keys);
								if (col_fieldtypes)
									free(col_fieldtypes);
								if (col_values) {
									int ii;
									for (ii = 0; ii < col_values_num; ii++) {
										if (col_values[ii])
											free(col_values[ii]);
									}
									free(col_values);
								}
								if (jansson)
									json_decref(jansson);
								return OPH_JSON_GENERIC_ERROR;
							}
							if (row_keys)
								free(row_keys);
							if (row_fieldtypes)
								free(row_fieldtypes);
							if (col_keys)
								free(col_keys);
							if (col_fieldtypes)
								free(col_fieldtypes);
							if (col_values) {
								int ii;
								for (ii = 0; ii < col_values_num; ii++) {
									if (col_values[ii])
										free(col_values[ii]);
								}
								free(col_values);
							}
							// manage rows of values
							json_t *rowvalues = NULL;
							json_t *measurevalues = NULL;
							json_unpack(objcontentfrag, "{s?o,s?o}", "rowvalues", &rowvalues, "measurevalues", &measurevalues);
							if (rowvalues && measurevalues) {
								size_t index4;
								json_t *rowvalues_i = NULL;
								json_t *measurevalues_i = NULL;
								for (index4 = 0; index4 < json_array_size(rowvalues); index4++) {	// for each row of values
									json_unpack(json_array_get(rowvalues, index4), "o", &rowvalues_i);
									json_unpack(json_array_get(measurevalues, index4), "o", &measurevalues_i);	// get i-th row
									if (rowvalues_i && measurevalues_i) {
										char **row_values = NULL, **measure_values = NULL;

										// alloc row
										row_values = (char **) calloc(json_array_size(rowvalues_i), sizeof(char *));
										if (!row_values) {
											if (jansson)
												json_decref(jansson);
											return OPH_JSON_MEMORY_ERROR;
										}
										measure_values = (char **) calloc(json_array_size(measurevalues_i), sizeof(char *));
										if (!measure_values) {
											if (row_values)
												free(row_values);
											if (jansson)
												json_decref(jansson);
											return OPH_JSON_MEMORY_ERROR;
										}
										// fill row with values
										size_t index5;
										for (index5 = 0; index5 < json_array_size(rowvalues_i); index5++) {
											json_unpack(json_array_get(rowvalues_i, index5), "s", &(row_values[index5]));
											if (!row_values[index5]) {
												if (row_values)
													free(row_values);
												if (measure_values)
													free(measure_values);
												if (jansson)
													json_decref(jansson);
												return OPH_JSON_GENERIC_ERROR;
											}
										}
										for (index5 = 0; index5 < json_array_size(measurevalues_i); index5++) {
											json_unpack(json_array_get(measurevalues_i, index5), "s", &(measure_values[index5]));
											if (!measure_values[index5]) {
												if (row_values)
													free(row_values);
												if (measure_values)
													free(measure_values);
												if (jansson)
													json_decref(jansson);
												return OPH_JSON_GENERIC_ERROR;
											}
										}

										// add multigrid row
										if (oph_json_add_multigrid_row
										    (*json, (const char *) objkey, (const char **) row_values, (const char **) measure_values)) {
											if (row_values)
												free(row_values);
											if (measure_values)
												free(measure_values);
											if (jansson)
												json_decref(jansson);
											return OPH_JSON_GENERIC_ERROR;
										}
										if (row_values)
											free(row_values);
										if (measure_values)
											free(measure_values);
									}
								}
							}
						}
					}
				} else if (!strcmp(objclass, OPH_JSON_TREE)) {	//OBJCLASS TREE
					json_t *objcontentfrag = NULL;
					size_t index2;
					for (index2 = 0; index2 < json_array_size(objcontent); index2++) {
						json_unpack(json_array_get(objcontent, index2), "o", &objcontentfrag);

						if (objcontentfrag) {
							char *title = NULL, *description = NULL, **node_keys = NULL;
							int node_keys_num = 0;
							json_t *nodekeys = NULL;
							json_unpack(objcontentfrag, "{s?s,s?s,s?o}", "title", &title, "description", &description, "nodekeys", &nodekeys);

							if (nodekeys) {
								node_keys_num = json_array_size(nodekeys);
								// alloc node_keys
								node_keys = (char **) calloc(node_keys_num, sizeof(char *));
								if (!node_keys) {
									if (jansson)
										json_decref(jansson);
									return OPH_JSON_MEMORY_ERROR;
								}
								// fill node_keys
								int index3;
								for (index3 = 0; index3 < node_keys_num; index3++) {
									json_unpack(json_array_get(nodekeys, index3), "s", &(node_keys[index3]));
									if (!node_keys[index3]) {
										if (node_keys)
											free(node_keys);
										if (jansson)
											json_decref(jansson);
										return OPH_JSON_GENERIC_ERROR;
									}
								}

								// add tree
								if (oph_json_add_tree
								    (*json, (const char *) objkey, (const char *) title, (const char *) description, (const char **) node_keys, (int) node_keys_num)) {
									if (node_keys)
										free(node_keys);
									if (jansson)
										json_decref(jansson);
									return OPH_JSON_GENERIC_ERROR;
								}
								if (node_keys)
									free(node_keys);

								json_t *nodevalues = NULL;
								json_unpack(objcontentfrag, "{s?o}", "nodevalues", &nodevalues);
								if (nodevalues) {
									size_t index4;
									json_t *nodevalues_i = NULL;
									for (index4 = 0; index4 < json_array_size(nodevalues); index4++) {	// for each row of values
										json_unpack(json_array_get(nodevalues, index4), "o", &nodevalues_i);	// get i-th row
										if (nodevalues_i) {
											char **node_values = NULL;

											// alloc row
											node_values = (char **) calloc(json_array_size(nodevalues_i), sizeof(char *));
											if (!node_values) {
												if (jansson)
													json_decref(jansson);
												return OPH_JSON_MEMORY_ERROR;
											}
											// fill row with values
											size_t index5;
											for (index5 = 0; index5 < json_array_size(nodevalues_i); index5++) {
												json_unpack(json_array_get(nodevalues_i, index5), "s", &(node_values[index5]));
												if (!node_values[index5]) {
													if (node_values)
														free(node_values);
													if (jansson)
														json_decref(jansson);
													return OPH_JSON_GENERIC_ERROR;
												}
											}

											// add tree node
											if (oph_json_add_tree_node(*json, (const char *) objkey, (const char **) node_values)) {
												if (node_values)
													free(node_values);
												if (jansson)
													json_decref(jansson);
												return OPH_JSON_GENERIC_ERROR;
											}
											if (node_values)
												free(node_values);
										}
									}

									char *rootnode = NULL;
									json_t *nodelinks = NULL;
									int nodelinks_num = 0;
									json_unpack(objcontentfrag, "{s?o,s?s}", "nodelinks", &nodelinks, "rootnode", &rootnode);
									nodelinks_num = json_array_size(nodelinks);

									// set tree root
									if (oph_json_set_tree_root(*json, (const char *) objkey, (int) strtol(rootnode, NULL, 10))) {
										if (jansson)
											json_decref(jansson);
										return OPH_JSON_GENERIC_ERROR;
									}
									// add tree links
									int sourcenode;
									json_t *nodelinks_i = NULL;
									for (sourcenode = 0; sourcenode < nodelinks_num; sourcenode++) {
										json_unpack(json_array_get(nodelinks, sourcenode), "o", &nodelinks_i);

										if (nodelinks_i) {
											size_t s;
											for (s = 0; s < json_array_size(nodelinks_i); s++) {
												char *targetnode = NULL, *linkdescription = NULL;
												json_unpack(json_array_get(nodelinks_i, s), "{s?s,s?s}", "node", &targetnode, "description",
													    &linkdescription);
												if (!targetnode) {
													if (jansson)
														json_decref(jansson);
													return OPH_JSON_GENERIC_ERROR;
												}

												if (oph_json_add_tree_link
												    (*json, (const char *) objkey, sourcenode, (int) strtol(targetnode, NULL, 10),
												     (const char *) linkdescription)) {
													if (jansson)
														json_decref(jansson);
													return OPH_JSON_GENERIC_ERROR;
												}
											}
										}
									}
								} else {
									char *rootnode = NULL;
									json_t *nodelinks = NULL;
									int nodelinks_num = 0;
									json_unpack(objcontentfrag, "{s?o,s?s}", "nodelinks", &nodelinks, "rootnode", &rootnode);
									nodelinks_num = json_array_size(nodelinks);

									// add empty tree nodes
									int index4;
									for (index4 = 0; index4 < nodelinks_num; index4++) {
										if (oph_json_add_tree_node(*json, (const char *) objkey, NULL)) {
											if (jansson)
												json_decref(jansson);
											return OPH_JSON_GENERIC_ERROR;
										}
									}

									// set tree root
									if (oph_json_set_tree_root(*json, (const char *) objkey, (int) strtol(rootnode, NULL, 10))) {
										if (jansson)
											json_decref(jansson);
										return OPH_JSON_GENERIC_ERROR;
									}
									// add tree links
									int sourcenode;
									json_t *nodelinks_i = NULL;
									for (sourcenode = 0; sourcenode < nodelinks_num; sourcenode++) {
										json_unpack(json_array_get(nodelinks, sourcenode), "o", &nodelinks_i);

										if (nodelinks_i) {
											size_t s;
											for (s = 0; s < json_array_size(nodelinks_i); s++) {
												char *targetnode = NULL, *linkdescription = NULL;
												json_unpack(json_array_get(nodelinks_i, s), "{s?s,s?s}", "node", &targetnode, "description",
													    &linkdescription);
												if (!targetnode) {
													if (jansson)
														json_decref(jansson);
													return OPH_JSON_GENERIC_ERROR;
												}

												if (oph_json_add_tree_link
												    (*json, (const char *) objkey, sourcenode, (int) strtol(targetnode, NULL, 10),
												     (const char *) linkdescription)) {
													if (jansson)
														json_decref(jansson);
													return OPH_JSON_GENERIC_ERROR;
												}
											}
										}
									}
								}
							} else {
								// add empty tree
								if (oph_json_add_tree(*json, (const char *) objkey, (const char *) title, (const char *) description, NULL, 0)) {
									if (jansson)
										json_decref(jansson);
									return OPH_JSON_GENERIC_ERROR;
								}

								char *rootnode = NULL;
								json_t *nodelinks = NULL;
								int nodelinks_num = 0;
								json_unpack(objcontentfrag, "{s?o,s?s}", "nodelinks", &nodelinks, "rootnode", &rootnode);
								nodelinks_num = json_array_size(nodelinks);

								// add empty tree nodes
								int index3;
								for (index3 = 0; index3 < nodelinks_num; index3++) {
									if (oph_json_add_tree_node(*json, (const char *) objkey, NULL)) {
										if (jansson)
											json_decref(jansson);
										return OPH_JSON_GENERIC_ERROR;
									}
								}

								// set tree root
								if (oph_json_set_tree_root(*json, (const char *) objkey, (int) strtol(rootnode, NULL, 10))) {
									if (jansson)
										json_decref(jansson);
									return OPH_JSON_GENERIC_ERROR;
								}
								// add tree links
								int sourcenode;
								json_t *nodelinks_i = NULL;
								for (sourcenode = 0; sourcenode < nodelinks_num; sourcenode++) {
									json_unpack(json_array_get(nodelinks, sourcenode), "o", &nodelinks_i);

									if (nodelinks_i) {
										size_t s;
										for (s = 0; s < json_array_size(nodelinks_i); s++) {
											char *targetnode = NULL, *linkdescription = NULL;
											json_unpack(json_array_get(nodelinks_i, s), "{s?s,s?s}", "node", &targetnode, "description", &linkdescription);
											if (!targetnode) {
												if (jansson)
													json_decref(jansson);
												return OPH_JSON_GENERIC_ERROR;
											}

											if (oph_json_add_tree_link
											    (*json, (const char *) objkey, sourcenode, (int) strtol(targetnode, NULL, 10),
											     (const char *) linkdescription)) {
												if (jansson)
													json_decref(jansson);
												return OPH_JSON_GENERIC_ERROR;
											}
										}
									}
								}
							}
						}
					}
				} else if (!strcmp(objclass, OPH_JSON_DGRAPH) || !strcmp(objclass, OPH_JSON_GRAPH)) {	//OBJCLASS (DI)GRAPH
					int is_digraph = (!strcmp(objclass, OPH_JSON_DGRAPH)) ? 1 : 0;
					json_t *objcontentfrag = NULL;
					size_t index2;
					for (index2 = 0; index2 < json_array_size(objcontent); index2++) {
						json_unpack(json_array_get(objcontent, index2), "o", &objcontentfrag);

						if (objcontentfrag) {
							char *title = NULL, *description = NULL, **node_keys = NULL;
							int node_keys_num = 0;
							json_t *nodekeys = NULL;
							json_unpack(objcontentfrag, "{s?s,s?s,s?o}", "title", &title, "description", &description, "nodekeys", &nodekeys);

							if (nodekeys) {
								node_keys_num = json_array_size(nodekeys);
								// alloc node_keys
								node_keys = (char **) calloc(node_keys_num, sizeof(char *));
								if (!node_keys) {
									if (jansson)
										json_decref(jansson);
									return OPH_JSON_MEMORY_ERROR;
								}
								// fill node_keys
								int index3;
								for (index3 = 0; index3 < node_keys_num; index3++) {
									json_unpack(json_array_get(nodekeys, index3), "s", &(node_keys[index3]));
									if (!node_keys[index3]) {
										if (node_keys)
											free(node_keys);
										if (jansson)
											json_decref(jansson);
										return OPH_JSON_GENERIC_ERROR;
									}
								}

								// add graph
								if (oph_json_add_graph
								    (*json, (const char *) objkey, is_digraph, (const char *) title, (const char *) description, (const char **) node_keys,
								     (int) node_keys_num)) {
									if (node_keys)
										free(node_keys);
									if (jansson)
										json_decref(jansson);
									return OPH_JSON_GENERIC_ERROR;
								}
								if (node_keys)
									free(node_keys);

								json_t *nodevalues = NULL;
								json_unpack(objcontentfrag, "{s?o}", "nodevalues", &nodevalues);
								if (nodevalues) {
									size_t index4;
									json_t *nodevalues_i = NULL;
									for (index4 = 0; index4 < json_array_size(nodevalues); index4++) {	// for each row of values
										json_unpack(json_array_get(nodevalues, index4), "o", &nodevalues_i);	// get i-th row
										if (nodevalues_i) {
											char **node_values = NULL;

											// alloc row
											node_values = (char **) calloc(json_array_size(nodevalues_i), sizeof(char *));
											if (!node_values) {
												if (jansson)
													json_decref(jansson);
												return OPH_JSON_MEMORY_ERROR;
											}
											// fill row with values
											size_t index5;
											for (index5 = 0; index5 < json_array_size(nodevalues_i); index5++) {
												json_unpack(json_array_get(nodevalues_i, index5), "s", &(node_values[index5]));
												if (!node_values[index5]) {
													if (node_values)
														free(node_values);
													if (jansson)
														json_decref(jansson);
													return OPH_JSON_GENERIC_ERROR;
												}
											}

											// add graph node
											if (oph_json_add_graph_node(*json, (const char *) objkey, (const char **) node_values)) {
												if (node_values)
													free(node_values);
												if (jansson)
													json_decref(jansson);
												return OPH_JSON_GENERIC_ERROR;
											}
											if (node_values)
												free(node_values);
										}
									}

									json_t *nodelinks = NULL;
									int nodelinks_num = 0;
									json_unpack(objcontentfrag, "{s?o}", "nodelinks", &nodelinks);
									nodelinks_num = json_array_size(nodelinks);

									// add graph links
									int sourcenode;
									json_t *nodelinks_i = NULL;
									for (sourcenode = 0; sourcenode < nodelinks_num; sourcenode++) {
										json_unpack(json_array_get(nodelinks, sourcenode), "o", &nodelinks_i);

										if (nodelinks_i) {
											size_t s;
											for (s = 0; s < json_array_size(nodelinks_i); s++) {
												char *targetnode = NULL, *linkdescription = NULL;
												json_unpack(json_array_get(nodelinks_i, s), "{s?s,s?s}", "node", &targetnode, "description",
													    &linkdescription);
												if (!targetnode) {
													if (jansson)
														json_decref(jansson);
													return OPH_JSON_GENERIC_ERROR;
												}

												if (oph_json_add_graph_link
												    (*json, (const char *) objkey, sourcenode, (int) strtol(targetnode, NULL, 10),
												     (const char *) linkdescription)) {
													if (jansson)
														json_decref(jansson);
													return OPH_JSON_GENERIC_ERROR;
												}
											}
										}
									}
								} else {
									json_t *nodelinks = NULL;
									int nodelinks_num = 0;
									json_unpack(objcontentfrag, "{s?o}", "nodelinks", &nodelinks);
									nodelinks_num = json_array_size(nodelinks);

									// add empty graph nodes
									int index4;
									for (index4 = 0; index4 < nodelinks_num; index4++) {
										if (oph_json_add_graph_node(*json, (const char *) objkey, NULL)) {
											if (jansson)
												json_decref(jansson);
											return OPH_JSON_GENERIC_ERROR;
										}
									}

									// add graph links
									int sourcenode;
									json_t *nodelinks_i = NULL;
									for (sourcenode = 0; sourcenode < nodelinks_num; sourcenode++) {
										json_unpack(json_array_get(nodelinks, sourcenode), "o", &nodelinks_i);

										if (nodelinks_i) {
											size_t s;
											for (s = 0; s < json_array_size(nodelinks_i); s++) {
												char *targetnode = NULL, *linkdescription = NULL;
												json_unpack(json_array_get(nodelinks_i, s), "{s?s,s?s}", "node", &targetnode, "description",
													    &linkdescription);
												if (!targetnode) {
													if (jansson)
														json_decref(jansson);
													return OPH_JSON_GENERIC_ERROR;
												}

												if (oph_json_add_graph_link
												    (*json, (const char *) objkey, sourcenode, (int) strtol(targetnode, NULL, 10),
												     (const char *) linkdescription)) {
													if (jansson)
														json_decref(jansson);
													return OPH_JSON_GENERIC_ERROR;
												}
											}
										}
									}
								}
							} else {
								// add empty graph
								if (oph_json_add_graph(*json, (const char *) objkey, is_digraph, (const char *) title, (const char *) description, NULL, 0)) {
									if (jansson)
										json_decref(jansson);
									return OPH_JSON_GENERIC_ERROR;
								}

								json_t *nodelinks = NULL;
								int nodelinks_num = 0;
								json_unpack(objcontentfrag, "{s?o}", "nodelinks", &nodelinks);
								nodelinks_num = json_array_size(nodelinks);

								// add empty graph nodes
								int index3;
								for (index3 = 0; index3 < nodelinks_num; index3++) {
									if (oph_json_add_graph_node(*json, (const char *) objkey, NULL)) {
										if (jansson)
											json_decref(jansson);
										return OPH_JSON_GENERIC_ERROR;
									}
								}

								// add graph links
								int sourcenode;
								json_t *nodelinks_i = NULL;
								for (sourcenode = 0; sourcenode < nodelinks_num; sourcenode++) {
									json_unpack(json_array_get(nodelinks, sourcenode), "o", &nodelinks_i);

									if (nodelinks_i) {
										size_t s;
										for (s = 0; s < json_array_size(nodelinks_i); s++) {
											char *targetnode = NULL, *linkdescription = NULL;
											json_unpack(json_array_get(nodelinks_i, s), "{s?s,s?s}", "node", &targetnode, "description", &linkdescription);
											if (!targetnode) {
												if (jansson)
													json_decref(jansson);
												return OPH_JSON_GENERIC_ERROR;
											}

											if (oph_json_add_graph_link
											    (*json, (const char *) objkey, sourcenode, (int) strtol(targetnode, NULL, 10),
											     (const char *) linkdescription)) {
												if (jansson)
													json_decref(jansson);
												return OPH_JSON_GENERIC_ERROR;
											}
										}
									}
								}
							}
						}
					}
				} else {
					if (jansson)
						json_decref(jansson);
					return OPH_JSON_GENERIC_ERROR;
				}
			}
		}
	}
	//GET EXTRA DATA FROM JSON_T
	json_unpack(jansson, "{s?o}", "extra", &extra);
	if (extra) {

		json_t *keys = NULL;
		json_t *values = NULL;
		json_unpack(extra, "{s?o,s?o}", "keys", &keys, "values", &values);

		if (keys && values) {
			char *key = NULL, *value = NULL;
			size_t index;
			for (index = 0; index < json_array_size(keys); index++) {
				json_unpack(json_array_get(keys, index), "s", &key);
				json_unpack(json_array_get(values, index), "s", &value);
				if (oph_json_add_extra_detail(*json, (const char *) key, (const char *) value))
					break;
			}
		} else if ((keys && !values) || (!keys && values)) {
			if (jansson)
				json_decref(jansson);
			return OPH_JSON_GENERIC_ERROR;
		}
	}

	/* CLEANUP */
	if (jansson)
		json_decref(jansson);

	return OPH_JSON_SUCCESS;
}
