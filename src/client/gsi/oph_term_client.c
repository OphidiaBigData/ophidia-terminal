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

#include "oph_term_client.h"
#include "soapH.h"
#include "oph.nsmap"
#include "gsi.h"

#include <unistd.h>		/* defines _POSIX_THREADS if pthreads are available */
#if defined(_POSIX_THREADS) || defined(_SC_THREADS)
#include <pthread.h>
#endif
#include <signal.h>		/* defines SIGPIPE */

#define OPH_DEFAULT_NLOOPS 1
#define OPH_DEFAULT_QUERY "OPH_NULL"

#define UNUSED(x) {(void)(x);}

int CRYPTO_thread_setup();
void CRYPTO_thread_cleanup();
void sigpipe_handle(int);

/* authorization callback prototype */
int gsi_authorization_callback(struct soap *soap, char *distinguished_name);

/* credential renewal callback */
int gsi_plugin_credential_renew_callback(struct soap *soap, int lifetime);

void cleanup(struct soap *soap)
{
	soap_destroy(soap);
	soap_end(soap);
	soap_done(soap);	/* MUST call before CRYPTO_thread_cleanup */
	globus_module_deactivate(GLOBUS_GSI_GSSAPI_MODULE);
}

struct soap soap_global;
char server_global[OPH_MAX_STRING_SIZE];
char query_global[WORKFLOW_MAX_LEN];
struct oph__ophResponse response_global;
int soap_call_oph__ophExecuteMain_return;

void *soapthread(void *ptr)
{
	UNUSED(ptr);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	soap_call_oph__ophExecuteMain_return = soap_call_oph__ophExecuteMain(&soap_global, server_global, "", query_global, &response_global);
	return NULL;
}

void oph_execute(struct soap *soap, xsd__string query, char *wps, char **newsession, int *return_value, char **out_response, char **out_response_for_viewer, int workflow_wrap, char *username,
		 HASHTBL * hashtbl, char *cmd_line)
{
	//If requested, wrap query in a 1-task workflow
	char wrapped_query[WORKFLOW_MAX_LEN];
	memset(wrapped_query, 0, WORKFLOW_MAX_LEN);
	if (workflow_wrap) {
		int n = 0;
		char *tmp = NULL;

		//detect operator
		char *operator = NULL;
		int j;
		for (j = 9; j < (int) strlen(query); j++) {
			if (query[j] == ';')
				break;
		}
		operator = strndup(query + 9, j - 9);

	      n += snprintf(wrapped_query + n, WORKFLOW_MAX_LEN - n, WRAPPING_WORKFLOW1, operator? operator:"", username);

		//insert sessionid if present
		tmp = strstr(query, "sessionid=http");
		if (tmp) {
			char *tmp2 = NULL;
			tmp2 = strchr(tmp, ';');
			int size = strlen((tmp + 10)) - ((tmp2) ? strlen(tmp2) : 0);
			char *tmp3 = tmp + 10;
			n += snprintf(wrapped_query + n, WORKFLOW_MAX_LEN - n, WRAPPING_WORKFLOW2);
			int k;
			for (k = 0; k < size; k++) {
				n += snprintf(wrapped_query + n, WORKFLOW_MAX_LEN - n, "%c", tmp3[k]);
			}
			n += snprintf(wrapped_query + n, WORKFLOW_MAX_LEN - n, WRAPPING_WORKFLOW2_1);
		}
		//insert exec_mode if present
		tmp = strstr(query, "exec_mode=");
		if (tmp) {
			char *tmp2 = NULL;
			tmp2 = strchr(tmp, ';');
			int size = strlen((tmp + 10)) - ((tmp2) ? strlen(tmp2) : 0);
			char *tmp3 = tmp + 10;
			n += snprintf(wrapped_query + n, WORKFLOW_MAX_LEN - n, WRAPPING_WORKFLOW3);
			int k;
			for (k = 0; k < size; k++) {
				n += snprintf(wrapped_query + n, WORKFLOW_MAX_LEN - n, "%c", tmp3[k]);
			}
			n += snprintf(wrapped_query + n, WORKFLOW_MAX_LEN - n, WRAPPING_WORKFLOW3_1);
		}
		//insert callback_url if present
		tmp = strstr(query, "callback_url=");
		if (tmp) {
			char *tmp2 = NULL;
			tmp2 = strchr(tmp, ';');
			int size = strlen((tmp + 13)) - ((tmp2) ? strlen(tmp2) : 0);
			char *tmp3 = tmp + 13;
			n += snprintf(wrapped_query + n, WORKFLOW_MAX_LEN - n, WRAPPING_WORKFLOW4);
			int k;
			for (k = 0; k < size; k++) {
				n += snprintf(wrapped_query + n, WORKFLOW_MAX_LEN - n, "%c", tmp3[k]);
			}
			n += snprintf(wrapped_query + n, WORKFLOW_MAX_LEN - n, WRAPPING_WORKFLOW4_1);
		}

		n += snprintf(wrapped_query + n, WORKFLOW_MAX_LEN - n, WRAPPING_WORKFLOW5);
	      n += snprintf(wrapped_query + n, WORKFLOW_MAX_LEN - n, "%s", operator? operator:"");
		n += snprintf(wrapped_query + n, WORKFLOW_MAX_LEN - n, WRAPPING_WORKFLOW5_1);

		if (operator) {
			free(operator);
			operator = NULL;
		}
		//insert all remaining arguments
		char *tmp_query = NULL;
		char *tmp_keyvalue = NULL;
		int substitute = 0;
		int substituted = 0;
		tmp_query = (char *) strdup(query);
		if (tmp_query) {
			int len = strlen(tmp_query);
			int i = 0, j, skip;
			char *ptr = tmp_query;

			while (i < len) {
				if (tmp_query[i] == '=') {
					tmp_query[i] = '\0';
					if (strcmp(ptr, "operator") && strcmp(ptr, "sessionid") && strcmp(ptr, "exec_mode") && strcmp(ptr, "callback_url")) {
						substitute = 1;
					} else {
						substitute = 0;
					}
					tmp_query[i] = '=';
					skip = 0;
					for (j = i + 1; j < len; j++) {
						if (tmp_query[j] == '[')
							skip = 1;
						else if (tmp_query[j] == ']')
							skip = 0;
						else if (!skip && (tmp_query[j] == ';')) {
							if (substitute) {
								tmp_query[j] = '\0';
								tmp_keyvalue = (char *) strdup(ptr);
								if (tmp_keyvalue) {
									substituted++;
									if (substituted == 1) {
										n += snprintf(wrapped_query + n, WORKFLOW_MAX_LEN - n, WRAPPING_WORKFLOW6, tmp_keyvalue);
									} else {
										n += snprintf(wrapped_query + n, WORKFLOW_MAX_LEN - n, WRAPPING_WORKFLOW7, tmp_keyvalue);
									}
									free(tmp_keyvalue);
									tmp_keyvalue = NULL;
								}
							}
							ptr = tmp_query + j + 1;
							i = j + 1;
							break;
						}
					}
					if (j == len)
						i++;
				} else {
					i++;
				}
			}
			free(tmp_query);
			tmp_query = NULL;
		}

		n += snprintf(wrapped_query + n, WORKFLOW_MAX_LEN - n, WRAPPING_WORKFLOW8);

		snprintf(query, WORKFLOW_MAX_LEN, "%s", wrapped_query);
	}
	// If workflow then insert available env vars and cmd_line in query
	char fixed_query[WORKFLOW_MAX_LEN];
	memset(fixed_query, 0, WORKFLOW_MAX_LEN);
	if (strstr(query, "\"name\"")) {
		char *query_start = strchr(query, '{');
		if (query_start) {
			int n = 0;
			n += snprintf(fixed_query + n, WORKFLOW_MAX_LEN - n, "{");
			if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID)) {
				if (!strstr(query, "\"sessionid\"")) {
					n += snprintf(fixed_query + n, WORKFLOW_MAX_LEN - n, "%s%s%s", WRAPPING_WORKFLOW2, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID),
						      WRAPPING_WORKFLOW2_1);
				}
			}
			if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_EXEC_MODE)) {
				if (!strstr(query, "\"exec_mode\"")) {
					n += snprintf(fixed_query + n, WORKFLOW_MAX_LEN - n, "%s%s%s", WRAPPING_WORKFLOW3, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_EXEC_MODE),
						      WRAPPING_WORKFLOW3_1);
				}
			}
			if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_NCORES)) {
				if (!strstr(query, "\"ncores\"")) {
					n += snprintf(fixed_query + n, WORKFLOW_MAX_LEN - n, "%s%s%s", WRAPPING_WORKFLOW4c, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_NCORES),
						      WRAPPING_WORKFLOW4c_1);
				}
			}
			if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CWD)) {
				if (!strstr(query, "\"cwd\"")) {
					n += snprintf(fixed_query + n, WORKFLOW_MAX_LEN - n, "%s%s%s", WRAPPING_WORKFLOW4d, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CWD), WRAPPING_WORKFLOW4d_1);
				}
			}
			if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_DATACUBE)) {
				if (!strstr(query, "\"cube\"")) {
					n += snprintf(fixed_query + n, WORKFLOW_MAX_LEN - n, "%s%s%s", WRAPPING_WORKFLOW4e, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_DATACUBE),
						      WRAPPING_WORKFLOW4e_1);
				}
			}
			if (hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CDD)) {
				if (!strstr(query, "\"cdd\"")) {
					n += snprintf(fixed_query + n, WORKFLOW_MAX_LEN - n, "%s%s%s", WRAPPING_WORKFLOW4f, (char *) hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CDD), WRAPPING_WORKFLOW4f_1);
				}
			}
			if (cmd_line) {
				if (!strstr(query, "\"command\"")) {
					n += snprintf(fixed_query + n, WORKFLOW_MAX_LEN - n, "%s%s%s", WRAPPING_WORKFLOW4b, cmd_line, WRAPPING_WORKFLOW4b_1);
				}
			}
			n += snprintf(fixed_query + n, WORKFLOW_MAX_LEN - n, "%s", query_start + 1);
			snprintf(query, WORKFLOW_MAX_LEN, "%s", fixed_query);
		}
	}

	if (out_response_for_viewer && !strstr(query, "\"sessionid\"")) {
		(print_json) ? my_fprintf(stderr, "[WARNING] Session not specified. A new session will be created!\\n\\n") : fprintf(stderr,
																     "[WARNING] Session not specified. A new session will be created!\n\n");
	}

	if (!wps) {
		pthread_create(&tid, NULL, &soapthread, NULL);
		pthread_join(tid, NULL);

		if (soap_call_oph__ophExecuteMain_return == SOAP_OK) {
			switch (response_global.error) {
				case OPH_SERVER_OK:
					if (out_response) {
						if (!response_global.response) {
							*out_response = strdup("");
							if (!*out_response) {
								(print_json) ? my_fprintf(stderr, "Memory error with out_response\\n") : fprintf(stderr,
																		 "\e[1;31mMemory error with out_response\e[0m\n");
								*return_value = OPH_TERM_MEMORY_ERROR;
								break;
							}
						} else {
							*out_response = strdup((const char *) response_global.response);
							if (!*out_response) {
								(print_json) ? my_fprintf(stderr, "Memory error with out_response\\n") : fprintf(stderr,
																		 "\e[1;31mMemory error with out_response\e[0m\n");
								*return_value = OPH_TERM_MEMORY_ERROR;
								break;
							}
							if (strstr(response_global.response, "\"title\": \"ERROR\"")
							    || (strstr(response_global.response, "\"title\": \"Workflow Status\"")
								&& strstr(response_global.response, "\"message\": \"OPH_STATUS_ERROR\"")))
								*return_value = OPH_TERM_GENERIC_ERROR;
						}
					} else {
						if (!response_global.response) {
							if (out_response_for_viewer)
								*out_response_for_viewer = NULL;
							if (response_global.jobid && strlen(response_global.jobid) != 0) {
								(print_json) ? snprintf(oph_term_jobid, OUTPUT_MAX_LEN, "%s", response_global.jobid) : printf("\e[1;34m[JobID]:\e[0m\n%s\n\n",
																			      response_global.jobid);
							}
						} else {
							if (out_response_for_viewer) {
								*out_response_for_viewer = strdup((const char *) response_global.response);
								if (!*out_response_for_viewer) {
									(print_json) ? my_fprintf(stderr, "Memory error with out_response_for_viewer\\n") : fprintf(stderr,
																				    "\e[1;31mMemory error with out_response_for_viewer\e[0m\n");
									*return_value = OPH_TERM_MEMORY_ERROR;
									break;
								}
							}
							if (response_global.jobid && strlen(response_global.jobid) != 0) {
								(print_json) ? snprintf(oph_term_jobid, OUTPUT_MAX_LEN, "%s",
											response_global.jobid) : printf("\e[1;34m[JobID]:\e[0m\n%s\n\n\e[1;34m[Response]:\e[0m\n",
															response_global.jobid);
							} else {
								if (!print_json)
									printf("\e[1;34m[Response]:\e[0m\n");
							}
							if (strstr(response_global.response, "\"title\": \"ERROR\"")
							    || (strstr(response_global.response, "\"title\": \"Workflow Status\"")
								&& strstr(response_global.response, "\"message\": \"OPH_STATUS_ERROR\"")))
								*return_value = OPH_TERM_GENERIC_ERROR;
						}

						char *tmp = NULL;
						char *ptr = NULL;

						if (response_global.jobid && newsession) {
							//retrieve newsession
							if (strlen(response_global.jobid) == 0) {
								*newsession = strdup("");
								if (!*newsession) {
									(print_json) ? my_fprintf(stderr, "Memory error retrieving sessionid\\n") : fprintf(stderr,
																			    "\e[1;31mMemory error retrieving sessionid\e[0m\n");
									if (out_response_for_viewer && *out_response_for_viewer) {
										free(*out_response_for_viewer);
										*out_response_for_viewer = NULL;
									}
									*return_value = OPH_TERM_MEMORY_ERROR;
									break;
								}
							} else {
								tmp = strdup((char *) response_global.jobid);
								if (!tmp) {
									(print_json) ? my_fprintf(stderr, "Memory error retrieving sessionid\\n") : fprintf(stderr,
																			    "\e[1;31mMemory error retrieving sessionid\e[0m\n");
									if (out_response_for_viewer && *out_response_for_viewer) {
										free(*out_response_for_viewer);
										*out_response_for_viewer = NULL;
									}
									*return_value = OPH_TERM_MEMORY_ERROR;
									break;
								}
								char *saveptr;
								ptr = strtok_r(tmp, OPH_TERM_WORKFLOW_DELIMITER, &saveptr);
								if (!ptr) {
									(print_json) ? my_fprintf(stderr, "Invalid jobid string\\n") : fprintf(stderr, "\e[1;31mInvalid jobid string\e[0m\n");
									free(tmp);
									tmp = NULL;
									if (out_response_for_viewer && *out_response_for_viewer) {
										free(*out_response_for_viewer);
										*out_response_for_viewer = NULL;
									}
									*return_value = OPH_TERM_INVALID_PARAM_VALUE;
									break;
								}
								*newsession = strdup(ptr);
								if (!*newsession) {
									(print_json) ? my_fprintf(stderr, "Memory error retrieving sessionid\\n") : fprintf(stderr,
																			    "\e[1;31mMemory error retrieving sessionid\e[0m\n");
									free(tmp);
									tmp = NULL;
									ptr = NULL;
									if (out_response_for_viewer && *out_response_for_viewer) {
										free(*out_response_for_viewer);
										*out_response_for_viewer = NULL;
									}
									*return_value = OPH_TERM_MEMORY_ERROR;
									break;
								}
								// retrieve last jobid if requested
								if (hashtbl_get(hashtbl, OPH_TERM_ENV_LAST_JOBID)) {
									ptr = strtok_r(NULL, OPH_TERM_MARKER_DELIMITER, &saveptr);
									if (!ptr) {
										(print_json) ? my_fprintf(stderr, "Invalid jobid string\\n") : fprintf(stderr, "\e[1;31mInvalid jobid string\e[0m\n");
										free(tmp);
										tmp = NULL;
										ptr = NULL;
										if (out_response_for_viewer && *out_response_for_viewer) {
											free(*out_response_for_viewer);
											*out_response_for_viewer = NULL;
										}
										*return_value = OPH_TERM_INVALID_PARAM_VALUE;
										break;
									}
									if (hashtbl_insert(hashtbl, OPH_TERM_ENV_LAST_JOBID, (void *) ptr, strlen(ptr) + 1)) {
										(print_json) ? my_fprintf(stderr, "Error retrieving jobid\\n") : fprintf(stderr,
																			 "\e[1;31mError retrieving jobid\e[0m\n");
										free(tmp);
										tmp = NULL;
										ptr = NULL;
										if (out_response_for_viewer && *out_response_for_viewer) {
											free(*out_response_for_viewer);
											*out_response_for_viewer = NULL;
										}
										*return_value = OPH_TERM_MEMORY_ERROR;
										break;
									}
								}
								free(tmp);
								tmp = NULL;
								ptr = NULL;
							}
						}
					}
					break;
				case OPH_SERVER_UNKNOWN:
					(print_json) ? my_fprintf(stderr, "Error on serving request [%ld]: server unknown\\n", response_global.error) : fprintf(stderr,
																				"\e[1;31mError on serving request [%ld]: server unknown\e[0m\n",
																				response_global.error);
					*return_value = OPH_SERVER_UNKNOWN;
					break;
				case OPH_SERVER_NULL_POINTER:
					(print_json) ? my_fprintf(stderr, "Error on serving request [%ld]: server null pointer\\n", response_global.error) : fprintf(stderr,
																				     "\e[1;31mError on serving request [%ld]: server null pointer\e[0m\n",
																				     response_global.error);
					*return_value = OPH_SERVER_NULL_POINTER;
					break;
				case OPH_SERVER_ERROR:
					(print_json) ? my_fprintf(stderr, "Error on serving request [%ld]: server error\\n", response_global.error) : fprintf(stderr,
																			      "\e[1;31mError on serving request [%ld]: server error\e[0m\n",
																			      response_global.error);
					*return_value = OPH_SERVER_ERROR;
					break;
				case OPH_SERVER_IO_ERROR:
					(print_json) ? my_fprintf(stderr, "Error on serving request [%ld]: server IO error\\n", response_global.error) : fprintf(stderr,
																				 "\e[1;31mError on serving request [%ld]: server IO error\e[0m\n",
																				 response_global.error);
					*return_value = OPH_SERVER_IO_ERROR;
					break;
				case OPH_SERVER_AUTH_ERROR:
					(print_json) ? my_fprintf(stderr, "Error on serving request [%ld]: server authentication error\\n", response_global.error) : fprintf(stderr,
																					     "\e[1;31mError on serving request [%ld]: server authentication error\e[0m\n",
																					     response_global.error);
					*return_value = OPH_SERVER_AUTH_ERROR;
					break;
				case OPH_SERVER_SYSTEM_ERROR:
					(print_json) ? my_fprintf(stderr, "Error on serving request [%ld]: server system error\\n", response_global.error) : fprintf(stderr,
																				     "\e[1;31mError on serving request [%ld]: server system error\e[0m\n",
																				     response_global.error);
					*return_value = OPH_SERVER_SYSTEM_ERROR;
					break;
				case OPH_SERVER_WRONG_PARAMETER_ERROR:
					(print_json) ? my_fprintf(stderr, "Error on serving request [%ld]: server wrong parameter error\\n", response_global.error) : fprintf(stderr,
																					      "\e[1;31mError on serving request [%ld]: server wrong parameter error\e[0m\n",
																					      response_global.error);
					*return_value = OPH_SERVER_WRONG_PARAMETER_ERROR;
					break;
				case OPH_SERVER_NO_RESPONSE:
					(print_json) ? my_fprintf(stderr, "Error on serving request [%ld]: server no response\\n", response_global.error) : fprintf(stderr,
																				    "\e[1;31mError on serving request [%ld]: server no response\e[0m\n",
																				    response_global.error);
					*return_value = OPH_SERVER_NO_RESPONSE;
					break;
			}
		} else {
			soap_print_fault(soap, stderr);
			*return_value = SOAP_SVR_FAULT;
		}
	}
}

int oph_term_client(char *cmd_line, char *command, char **newsession, char *user, char *password, char *host, char *port, int *return_value, char **out_response, char **out_response_for_viewer,
		    int workflow_wrap, HASHTBL * hashtbl)
{
	UNUSED(cmds);
	UNUSED(pre_defined_aliases_keys);
	UNUSED(pre_defined_aliases_values);
	UNUSED(env_vars);
	UNUSED(env_vars_ptr);
	UNUSED(alias_ptr);
	UNUSED(xml_defs);
	UNUSED(operators_list);
	UNUSED(operators_list_size);
	UNUSED(user);
	UNUSED(password);

	snprintf(query_global, WORKFLOW_MAX_LEN, OPH_DEFAULT_QUERY);
	char *wps = 0;

	if (command)
		snprintf(query_global, WORKFLOW_MAX_LEN, "%s", command);
	char *query = query_global;

	if (!strcasecmp(query, OPH_DEFAULT_QUERY))
		return 1;

	/* Need SIGPIPE handler on Unix/Linux systems to catch broken pipes: */
	signal(SIGPIPE, sigpipe_handle);

	globus_module_activate(GLOBUS_GSI_GSSAPI_MODULE);
	soap_init(&soap_global);

	/* now we register the GSI plugin */
	if (soap_register_plugin(&soap_global, globus_gsi)) {
		soap_print_fault(&soap_global, stderr);
		cleanup(&soap_global);
		return -3;
	}
	/* setup of authorization and credential renewal callbacks */
	gsi_authorization_callback_register(&soap_global, gsi_authorization_callback);
	gsi_credential_renew_callback_register(&soap_global, gsi_plugin_credential_renew_callback);

	/* we begin acquiring our credential */
	int rc = gsi_acquire_credential(&soap_global);
	if (rc) {
		cleanup(&soap_global);
		return -4;
	}

	/* setup of GSI channel */
	gsi_set_replay(&soap_global, GLOBUS_TRUE);
	gsi_set_sequence(&soap_global, GLOBUS_TRUE);
	gsi_set_confidentiality(&soap_global, GLOBUS_TRUE);
	gsi_set_integrity(&soap_global, GLOBUS_TRUE);

	/* Timeout after 2 minutes stall on send/recv */
	gsi_set_recv_timeout(&soap_global, 3600);
	gsi_set_send_timeout(&soap_global, 3600);

	soap_global.connect_timeout = 60;	/* try to connect for 1 minute */
	soap_global.send_timeout = soap_global.recv_timeout = 3600;	/* if I/O stalls, then timeout after 1 hour */

	snprintf(server_global, OPH_MAX_STRING_SIZE, "httpg://%s:%s", host, port);

	soap_global.userid = NULL;
	soap_global.passwd = NULL;

	struct gsi_plugin_data *data = (struct gsi_plugin_data *) soap_lookup_plugin(&soap_global, GSI_PLUGIN_ID);
	if (!data) {
		cleanup(&soap_global);
		return -4;
	}

	char username[OPH_TERM_MAX_LEN];
	snprintf(username, OPH_TERM_MAX_LEN, "%s", data->identity);

	oph_execute(&soap_global, query, wps, newsession, return_value, out_response, out_response_for_viewer, workflow_wrap, username, hashtbl, cmd_line);

	cleanup(&soap_global);
	return 0;
}

/******************************************************************************\
 *
 *  SIGPIPE
 *
\******************************************************************************/

void sigpipe_handle(int x)
{
	UNUSED(x);
}

/******************************************************************************\
 *
 *	GSI Specific
 *
\******************************************************************************/

int gsi_plugin_credential_renew_callback(struct soap *soap, int lifetime)
{
	UNUSED(soap);
	UNUSED(lifetime);
	return 0;
}

int gsi_authorization_callback(struct soap *soap, char *distinguished_name)
{
	UNUSED(soap);
	UNUSED(distinguished_name);
	return 0;
}
