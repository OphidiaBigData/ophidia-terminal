/*
    Ophidia Terminal
    Copyright (C) 2012-2024 CMCC Foundation

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
#include "oph_workflow_define.h"
#include "soapH.h"
#include "oph.nsmap"

#include <unistd.h>		/* defines _POSIX_THREADS if pthreads are available */
#if defined(_POSIX_THREADS) || defined(_SC_THREADS)
#include <pthread.h>
#endif
#include <signal.h>		/* defines SIGPIPE */

#define OPH_DEFAULT_NLOOPS 1
#define OPH_DEFAULT_QUERY "OPH_NULL"

#define UNUSED(x) {(void)(x);}

extern pthread_mutex_t global_flag;
extern size_t max_size;
extern int last_workflow_id;

int CRYPTO_thread_setup();
void CRYPTO_thread_cleanup();
void sigpipe_handle(int);

void cleanup(struct soap *soap)
{
	soap_destroy(soap);
	soap_end(soap);
	soap_done(soap);	/* MUST call before CRYPTO_thread_cleanup */
	CRYPTO_thread_cleanup();
}

struct soap soap_global;
char server_global[OPH_MAX_STRING_SIZE];
struct oph__ophResponse response_global;
int soap_call_oph__ophExecuteMain_return;

void *soapthread(void *query)
{
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	soap_call_oph__ophExecuteMain_return = soap_call_oph__ophExecuteMain(&soap_global, server_global, "", (char *) query, &response_global);
	free(query);
	return NULL;
}

void oph_execute(struct soap *soap, xsd__string query, char *wps, char **newsession, int *return_value, char **out_response, char **out_response_for_viewer, int workflow_wrap, char *username,
		 HASHTBL * hashtbl, char *cmd_line)
{
	if (max_size <= 0) {
		*return_value = OPH_TERM_GENERIC_ERROR;
		return;
	}
	//If requested, wrap query in a 1-task workflow
	char *wrapped_query = (char *) malloc(max_size);
	if (!wrapped_query) {
		*return_value = OPH_TERM_MEMORY_ERROR;
		return;
	}
	memset(wrapped_query, 0, max_size);
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

	      n += snprintf(wrapped_query + n, max_size - n, WRAPPING_WORKFLOW1, operator? operator:"", username);

		//insert sessionid if present
		tmp = strstr(query, "sessionid=http");
		if (tmp) {
			char *tmp2 = NULL;
			tmp2 = strchr(tmp, ';');
			int size = strlen((tmp + 10)) - ((tmp2) ? strlen(tmp2) : 0);
			char *tmp3 = tmp + 10;
			n += snprintf(wrapped_query + n, max_size - n, WRAPPING_WORKFLOW2);
			int k;
			for (k = 0; k < size; k++) {
				n += snprintf(wrapped_query + n, max_size - n, "%c", tmp3[k]);
			}
			n += snprintf(wrapped_query + n, max_size - n, WRAPPING_WORKFLOW2_1);
		}
		//insert exec_mode if present
		tmp = strstr(query, "exec_mode=");
		if (tmp) {
			char *tmp2 = NULL;
			tmp2 = strchr(tmp, ';');
			int size = strlen((tmp + 10)) - ((tmp2) ? strlen(tmp2) : 0);
			char *tmp3 = tmp + 10;
			n += snprintf(wrapped_query + n, max_size - n, WRAPPING_WORKFLOW3);
			int k;
			for (k = 0; k < size; k++) {
				n += snprintf(wrapped_query + n, max_size - n, "%c", tmp3[k]);
			}
			n += snprintf(wrapped_query + n, max_size - n, WRAPPING_WORKFLOW3_1);
		}
		//insert callback_url if present
		tmp = strstr(query, "callback_url=");
		if (tmp) {
			char *tmp2 = NULL;
			tmp2 = strchr(tmp, ';');
			int size = strlen((tmp + 13)) - ((tmp2) ? strlen(tmp2) : 0);
			char *tmp3 = tmp + 13;
			n += snprintf(wrapped_query + n, max_size - n, WRAPPING_WORKFLOW4);
			int k;
			for (k = 0; k < size; k++) {
				n += snprintf(wrapped_query + n, max_size - n, "%c", tmp3[k]);
			}
			n += snprintf(wrapped_query + n, max_size - n, WRAPPING_WORKFLOW4_1);
		}

	      n += snprintf(wrapped_query + n, max_size - n, WRAPPING_WORKFLOW5, operator? operator:"Task 0");
	      n += snprintf(wrapped_query + n, max_size - n, "%s", operator? operator:"");
		n += snprintf(wrapped_query + n, max_size - n, WRAPPING_WORKFLOW5_1);

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
										n += snprintf(wrapped_query + n, max_size - n, WRAPPING_WORKFLOW6, tmp_keyvalue);
									} else {
										n += snprintf(wrapped_query + n, max_size - n, WRAPPING_WORKFLOW7, tmp_keyvalue);
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

		n += snprintf(wrapped_query + n, max_size - n, WRAPPING_WORKFLOW8);

		if (n >= max_size)
			(print_json) ? my_fprintf(stderr, "Error in compiling the JSON Request\\n") : fprintf(stderr, "\e[1;31mError in compiling the JSON Request\e[0m\n");

		snprintf(query, max_size, "%s", wrapped_query);
	}
	free(wrapped_query);
	wrapped_query = NULL;
	// If workflow then insert available env vars and cmd_line in query
	char *fixed_query = (char *) malloc(max_size);
	if (!fixed_query) {
		*return_value = OPH_TERM_MEMORY_ERROR;
		return;
	}
	memset(fixed_query, 0, max_size);
	if (strstr(query, "\"name\"")) {
		char *query_start = strchr(query, '{'), *value;
		if (query_start) {

			int n = 0;
			n += snprintf(fixed_query + n, max_size - n, "{");

			value = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_SESSION_ID);
			if (value && !strstr(query, "\"sessionid\""))
				n += snprintf(fixed_query + n, max_size - n, "%s%s%s", WRAPPING_WORKFLOW2, value, WRAPPING_WORKFLOW2_1);

			value = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_EXEC_MODE);
			if (value && !strstr(query, "\"exec_mode\""))
				n += snprintf(fixed_query + n, max_size - n, "%s%s%s", WRAPPING_WORKFLOW3, value, WRAPPING_WORKFLOW3_1);

			value = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_NCORES);
			if (value && !strstr(query, "\"ncores\""))
				n += snprintf(fixed_query + n, max_size - n, "%s%s%s", WRAPPING_WORKFLOW4c, value, WRAPPING_WORKFLOW4c_1);

			value = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CWD);
			if (value && !strstr(query, "\"cwd\""))
				n += snprintf(fixed_query + n, max_size - n, "%s%s%s", WRAPPING_WORKFLOW4d, value, WRAPPING_WORKFLOW4d_1);

			value = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_DATACUBE);
			if (value && !strstr(query, "\"cube\""))
				n += snprintf(fixed_query + n, max_size - n, "%s%s%s", WRAPPING_WORKFLOW4e, value, WRAPPING_WORKFLOW4e_1);

			value = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_CDD);
			if (value && !strstr(query, "\"cdd\""))
				n += snprintf(fixed_query + n, max_size - n, "%s%s%s", WRAPPING_WORKFLOW4f, value, WRAPPING_WORKFLOW4f_1);

			value = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_HOST_PARTITION);
			if (value && strlen(value) && !strstr(query, "\"host_partition\"") && strcmp(value, OPH_TERM_ENV_OPH_MAIN_PARTITION))
				n += snprintf(fixed_query + n, max_size - n, "%s%s%s", WRAPPING_WORKFLOW4h, value, WRAPPING_WORKFLOW4h_1);

			value = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TERM_FORMAT);
			if (value && strlen(value) && !strstr(query, "\"output_format\""))
				n += snprintf(fixed_query + n, max_size - n, "%s%s%s", WRAPPING_WORKFLOW4i, value, WRAPPING_WORKFLOW4i_1);

			value = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_PROJECT);
			if (value && strlen(value) && !strstr(query, "\"project\""))
				n += snprintf(fixed_query + n, max_size - n, "%s%s%s", WRAPPING_WORKFLOW4l, value, WRAPPING_WORKFLOW4l_1);

			if (cmd_line && !strstr(query, "\"command\""))
				n += snprintf(fixed_query + n, max_size - n, "%s%s%s", WRAPPING_WORKFLOW4b, cmd_line, WRAPPING_WORKFLOW4b_1);

			n += snprintf(fixed_query + n, max_size - n, "%s", query_start + 1);

			if (n >= max_size)
				(print_json) ? my_fprintf(stderr, "Error in compiling the JSON Request\\n") : fprintf(stderr, "\e[1;31mError in compiling the JSON Request\e[0m\n");

			snprintf(query, max_size, "%s", fixed_query);
		}
	}
	free(fixed_query);
	fixed_query = NULL;

	if (out_response_for_viewer && !strstr(query, "\"sessionid\""))
		(print_json) ? my_fprintf(stderr, "[WARNING] Session not specified. A new session will be created!\\n\\n") : fprintf(stderr,
																     "[WARNING] Session not specified. A new session will be created!\n\n");

	if (!wps) {
		pthread_create(&tid, NULL, &soapthread, strdup(query));
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
								&& strstr(response_global.response, "\"message\": \"" OPH_ODB_STATUS_ERROR_STR "\"")))
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
								&& strstr(response_global.response, "\"message\": \"" OPH_ODB_STATUS_ERROR_STR "\"")))
								*return_value = OPH_TERM_GENERIC_ERROR;
						}

						// Retrieve last workflowid
						char *tmp = NULL, *ptr = NULL;
						if (response_global.jobid) {
							tmp = strstr(response_global.jobid, "?");
							if (tmp && *tmp)
								last_workflow_id = strtol(1 + tmp, NULL, 10);
						}

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
	if (max_size <= 0)
		return 2;

	char *username = user, *wps = 0;

	char *query_global = (char *) malloc(max_size);
	if (!query_global)
		return 2;

	snprintf(query_global, max_size, OPH_DEFAULT_QUERY);
	if (command)
		snprintf(query_global, max_size, "%s", command);
	char *query = query_global;
	if (!strcasecmp(query, OPH_DEFAULT_QUERY)) {
		free(query_global);
		return 1;
	}

	/* Need SIGPIPE handler on Unix/Linux systems to catch broken pipes: */
	signal(SIGPIPE, sigpipe_handle);

#ifdef WITH_OPENSSL
	/* Init SSL */
	soap_ssl_init();
#endif
	if (CRYPTO_thread_setup()) {
		free(query_global);
		return 1;
	}
	soap_init(&soap_global);
#ifdef WITH_OPENSSL
	/* Init gSOAP context */
	if (soap_ssl_client_context(&soap_global, SOAP_TLSv1_2 | SOAP_SSL_SKIP_HOST_CHECK, NULL, NULL, NULL, NULL, NULL)) {
		soap_print_fault(&soap_global, stderr);
		cleanup(&soap_global);
		free(query_global);
		return 1;
	}
#endif
	char *timeout = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TIMEOUT);
	int connection_timeout = timeout ? (int) strtol(timeout, NULL, 10) : OPH_TERM_DEFAULT_TIMEOUT;
	if (connection_timeout <= 0)
		connection_timeout = OPH_TERM_DEFAULT_TIMEOUT;

	soap_global.connect_timeout = 60;	/* try to connect for 1 minute */
	soap_global.send_timeout = soap_global.recv_timeout = connection_timeout;

#ifdef WITH_OPENSSL
	snprintf(server_global, OPH_MAX_STRING_SIZE, "https://%s:%s", host, port);
#else
	snprintf(server_global, OPH_MAX_STRING_SIZE, "http://%s:%s", host, port);
#endif

	char _password[OPH_MAX_STRING_SIZE];
	pthread_mutex_lock(&global_flag);
	char *_token = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
	if (_token && strlen(_token)) {
		snprintf(_password, OPH_MAX_STRING_SIZE, "%s", _token);
		password = _password;
	}
	pthread_mutex_unlock(&global_flag);

	soap_global.userid = username;	// Username has to set for each serve
	soap_global.passwd = password;	// Password has to set for each serve

	oph_execute(&soap_global, query, wps, newsession, return_value, out_response, out_response_for_viewer, workflow_wrap, username, hashtbl, cmd_line);

	cleanup(&soap_global);
	free(query_global);
	return 0;
}

/******************************************************************************\
 *
 *  OpenSSL
 *
\******************************************************************************/

#ifdef WITH_OPENSSL

#if defined(WIN32)
#define MUTEX_TYPE     HANDLE
#define MUTEX_SETUP(x)     (x) = CreateMutex(NULL, FALSE, NULL)
#define MUTEX_CLEANUP(x)   CloseHandle(x)
#define MUTEX_LOCK(x)      WaitForSingleObject((x), INFINITE)
#define MUTEX_UNLOCK(x)    ReleaseMutex(x)
#define THREAD_ID      GetCurrentThreadId()
#elif defined(_POSIX_THREADS) || defined(_SC_THREADS)
#define MUTEX_TYPE     pthread_mutex_t
#define MUTEX_SETUP(x)     pthread_mutex_init(&(x), NULL)
#define MUTEX_CLEANUP(x)   pthread_mutex_destroy(&(x))
#define MUTEX_LOCK(x)      pthread_mutex_lock(&(x))
#define MUTEX_UNLOCK(x)    pthread_mutex_unlock(&(x))
#define THREAD_ID      pthread_self()
#else
#error "You must define mutex operations appropriate for your platform"
#error "See OpenSSL /threads/th-lock.c on how to implement mutex on your platform"
#endif

struct CRYPTO_dynlock_value {
	MUTEX_TYPE mutex;
};

static MUTEX_TYPE *mutex_buf;

static struct CRYPTO_dynlock_value *dyn_create_function(const char *file, int line)
{
	UNUSED(file);
	UNUSED(line);
	struct CRYPTO_dynlock_value *value;
	value = (struct CRYPTO_dynlock_value *) malloc(sizeof(struct CRYPTO_dynlock_value));
	if (value)
		MUTEX_SETUP(value->mutex);
	return value;
}

static void dyn_lock_function(int mode, struct CRYPTO_dynlock_value *l, const char *file, int line)
{
	UNUSED(file);
	UNUSED(line);
	if (mode & CRYPTO_LOCK)
		MUTEX_LOCK(l->mutex);
	else
		MUTEX_UNLOCK(l->mutex);
}

static void dyn_destroy_function(struct CRYPTO_dynlock_value *l, const char *file, int line)
{
	UNUSED(file);
	UNUSED(line);
	MUTEX_CLEANUP(l->mutex);
	free(l);
}

void locking_function(int mode, int n, const char *file, int line)
{
	UNUSED(file);
	UNUSED(line);
	if (mode & CRYPTO_LOCK)
		MUTEX_LOCK(mutex_buf[n]);
	else
		MUTEX_UNLOCK(mutex_buf[n]);
}

unsigned long id_function()
{
	return (unsigned long) THREAD_ID;
}

int CRYPTO_thread_setup()
{
	int i;
	mutex_buf = (MUTEX_TYPE *) malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
	if (!mutex_buf)
		return SOAP_EOM;
	for (i = 0; i < CRYPTO_num_locks(); i++)
		MUTEX_SETUP(mutex_buf[i]);
	CRYPTO_set_id_callback(id_function);
	CRYPTO_set_locking_callback(locking_function);
	CRYPTO_set_dynlock_create_callback(dyn_create_function);
	CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
	CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);
	return SOAP_OK;
}

void CRYPTO_thread_cleanup()
{
	int i;
	if (!mutex_buf)
		return;
	CRYPTO_set_id_callback(NULL);
	CRYPTO_set_locking_callback(NULL);
	CRYPTO_set_dynlock_create_callback(NULL);
	CRYPTO_set_dynlock_lock_callback(NULL);
	CRYPTO_set_dynlock_destroy_callback(NULL);
	for (i = 0; i < CRYPTO_num_locks(); i++)
		MUTEX_CLEANUP(mutex_buf[i]);
	free(mutex_buf);
	mutex_buf = NULL;
}

#else

/* OpenSSL not used, e.g. GNUTLS is used */

int CRYPTO_thread_setup()
{
	return SOAP_OK;
}

void CRYPTO_thread_cleanup()
{
}

#endif

/******************************************************************************\
 *
 *  SIGPIPE
 *
\******************************************************************************/

void sigpipe_handle(int x)
{
	UNUSED(x);
}
