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

#include "oph_term_env.h"

#include <ctype.h>

#define UNUSED(x) {(void)(x);}

HASHTBL *conf_hashtbl = NULL;

extern char *_passwd;
extern pthread_mutex_t global_flag;
extern char get_config;
extern char *oph_base_src_path;

//Alloc hashtable
int _oph_term_env_init(HASHTBL ** hashtbl, int hashtbl_size)
{
	if (!(*hashtbl = hashtbl_create(hashtbl_size, NULL))) {
		(print_json) ? my_fprintf(stderr, "Error: env_init failed\\n") : fprintf(stderr, "\e[1;31mError: env_init failed\e[0m\n");
		return OPH_TERM_MEMORY_ERROR;
	}
	return OPH_TERM_SUCCESS;
}

int oph_term_env_init(HASHTBL ** hashtbl)
{
	return _oph_term_env_init(hashtbl, env_vars_num * 2);
}

//Free hashtable
int oph_term_env_clear(HASHTBL * hashtbl)
{
	hashtbl_destroy(hashtbl);
	if (conf_hashtbl) {
		hashtbl_destroy(conf_hashtbl);
		conf_hashtbl = NULL;
		get_config = 1;
	}
	return OPH_TERM_SUCCESS;
}

int is_env_var(const char *key)
{
	int i;
	for (i = 0; i < env_vars_num; i++) {
		if (!strcmp(key, env_vars[i])) {
			return 1;
		}
	}
	return 0;
}

// Check if key does not start with "oph_" or "/" or "." and is not a number
int is_env_var_ok(const char *key)
{
	if (!key)
		return 0;
	if (strlen(key) == 0)
		return 0;
	if (key[0] == '/' || key[0] == '.')
		return 0;
	if (!strncmp(key, OPH_TERM_ENV_OPH_PREFIX, OPH_TERM_ENV_OPH_PREFIX_LEN))
		return 0;
	int i;
	for (i = 0; i < (int) strlen(key); i++) {
		if (!isdigit(key[i]))
			return 1;
	}
	return 0;
}

// Get the list of user-defined variables
int oph_term_get_user_vars(HASHTBL * hashtbl, char ***user_vars, int *user_vars_num)
{
	hash_size n;
	struct hashnode_s *node;
	int i = 0, j;

	*user_vars_num = 0;
	*user_vars = NULL;

	for (n = 0; n < hashtbl->size; n++) {
		node = hashtbl->nodes[n];
		while (node) {
			if (!is_env_var(node->key))
				(*user_vars_num)++;
			node = node->next;
		}
	}

	if ((*user_vars_num) == 0)
		return OPH_TERM_SUCCESS;

	*user_vars = (char **) calloc(*user_vars_num, sizeof(char *));
	if (!(*user_vars)) {
		*user_vars_num = 0;
		return OPH_TERM_MEMORY_ERROR;
	}

	for (n = 0; n < hashtbl->size; n++) {
		node = hashtbl->nodes[n];
		while (node) {
			if (!is_env_var(node->key)) {
				(*user_vars)[i] = (char *) strdup(node->key);
				if (!((*user_vars)[i])) {
					for (j = 0; j < i; j++)
						free((*user_vars)[j]);
					free(*user_vars);
					*user_vars = NULL;
					*user_vars_num = 0;
					return OPH_TERM_MEMORY_ERROR;
				}
				i++;
			}
			node = node->next;
		}
	}

	return OPH_TERM_SUCCESS;
}

//Print all environment variables
int oph_term_env(HASHTBL * hashtbl)
{
	char *value;
	hash_size n;
	struct hashnode_s *node;
	int i;

	for (i = 0; i < env_vars_num; i++) {
		value = (char *) hashtbl_get(hashtbl, env_vars[i]);
		if (!value)
			(print_json) ? my_printf("%s=\\n", env_vars[i]) : printf("%s=\n", env_vars[i]);
		else if (strcmp(env_vars[i], OPH_TERM_ENV_OPH_PASSWD))
			(print_json) ? my_printf("%s=%s\\n", env_vars[i], value) : printf("%s=%s\n", env_vars[i], value);
		else
			(print_json) ? my_printf("%s=***\\n", env_vars[i]) : printf("%s=***\n", env_vars[i]);
	}

	for (n = 0; n < hashtbl->size; n++) {
		node = hashtbl->nodes[n];
		while (node) {
			if (!is_env_var(node->key)) {
				(print_json) ? my_printf("%s=", node->key) : printf("%s=", node->key);
				if (strcmp(node->key, OPH_TERM_ENV_OPH_PASSWD))
					(print_json) ? my_printf("%s\\n", (char *) node->data) : printf("%s\n", (char *) node->data);
				else
					(print_json) ? my_printf("***\\n") : printf("***\n");
			}
			node = node->next;
		}
	}
	return OPH_TERM_SUCCESS;
}

//Set value for variable key (insert new variable if not present)
int oph_term_setenv(HASHTBL * hashtbl, const char *key, const char *value)
{
	if (!is_env_var_ok(key)) {
		(print_json) ? my_fprintf(stderr, "A variable does not have to start with \\\"oph_\\\",\\\"/\\\" or \\\".\\\" and it does not have to be a number.\\n") : fprintf(stderr,
																						  "\e[1;31mA variable does not have to start with \"oph_\",\"/\" or \".\" and it does not have to be a number.\e[0m\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}
	if (hashtbl_insert(hashtbl, key, (void *) value, strlen(value) + 1)) {
		(print_json) ? my_fprintf(stderr, "Error: setenv failed\\n") : fprintf(stderr, "\e[1;31mError: setenv failed\e[0m\n");
		return OPH_TERM_MEMORY_ERROR;
	}
	return OPH_TERM_SUCCESS;
}

//Clear variable key if present or do nothing
int oph_term_unsetenv(HASHTBL * hashtbl, const char *key)
{
	if (hashtbl_remove(hashtbl, key)) {
		(print_json) ? my_fprintf(stderr, "Warning: variable %s not found\\n", key) : fprintf(stderr, "\e[1;31mWarning: variable %s not found\e[0m\n", key);
	}
	return OPH_TERM_SUCCESS;
}

//Print value for variable key
int oph_term_getenv(HASHTBL * hashtbl, const char *key)
{
	char *value;
	value = (char *) hashtbl_get(hashtbl, key);
	if (!value) {
		(print_json) ? my_fprintf(stderr, "Warning: variable %s not found\\n", key) : fprintf(stderr, "\e[1;31mWarning: variable %s not found\e[0m\n", key);
	} else {
		(print_json) ? my_printf("%s\\n", value) : printf("%s\n", value);
	}
	return OPH_TERM_SUCCESS;
}

// Update XML definitions for Ophidia operators (userpwd="user:pwd")
int oph_term_env_update_xml(const char *repoURL, const char *userpwd, const char *local_dir)
{

	if (!repoURL || !local_dir) {
		(print_json) ? my_fprintf(stderr, "Null parameters\\n") : fprintf(stderr, "\e[1;31mNull parameters\e[0m\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	CURL *curl1 = NULL, *curl2 = NULL;
	CURLcode res;
	char download_log[OPH_TERM_MAX_LEN];
	char tmp_file_list[OPH_TERM_MAX_LEN];
	char fixed_repoURL[OPH_TERM_MAX_LEN];
	char *line = NULL;
	size_t n = 0;
	char *ptr = NULL;
	char *ptr2 = NULL;
	char filename[100];
	char fileURL[OPH_TERM_MAX_LEN];
	char localfilename[OPH_TERM_MAX_LEN];
	int size = 0;
	int remote_file_count = 0;
	int downloaded_count = 0;
	int local_file_count = 0;
	long timestamp = 0;
	long localtimestamp;
	struct stat filestats;
	struct timeval times[2];
	DIR *dirp = NULL;
	struct dirent *entry = NULL;
	time_t t;
	char timestr[40];
	char **filelist = NULL;
	int filelist_num = 0;

	memset(download_log, 0, OPH_TERM_MAX_LEN);
	memset(tmp_file_list, 0, OPH_TERM_MAX_LEN);
	memset(fixed_repoURL, 0, OPH_TERM_MAX_LEN);
	snprintf(download_log, OPH_TERM_MAX_LEN, "%s/updates.log", local_dir);
	snprintf(tmp_file_list, OPH_TERM_MAX_LEN, "%s/list.html", local_dir);
	snprintf(fixed_repoURL, OPH_TERM_MAX_LEN, "%s%s", repoURL, (repoURL[strlen(repoURL) - 1] != '/') ? "/" : "");

	FILE *log = fopen(download_log, "a");
	if (!log) {
		(print_json) ? my_fprintf(stderr, "Unable to open log file\\n") : fprintf(stderr, "\e[1;31mUnable to open log file\e[0m\n");
		return OPH_TERM_GENERIC_ERROR;
	}

	time(&t);
	memset(timestr, 0, 40);
	strftime(timestr, 40, "%c %Z", localtime(&t));
	fprintf(log, "\n\n--------------------[%s]--------------------\n", timestr);

	FILE *file = fopen(tmp_file_list, "w");
	if (!file) {
		(print_json) ? my_fprintf(stderr, "Unable to write to file %s\\n", tmp_file_list) : fprintf(stderr, "\e[1;31mUnable to write to file %s\e[0m\n", tmp_file_list);
		fprintf(log, "Unable to write to file %s\n", tmp_file_list);
		fclose(log);
		return OPH_TERM_GENERIC_ERROR;
	}
	//Index of remote directory
	curl1 = curl_easy_init();
	if (!curl1) {
		(print_json) ? my_fprintf(stderr, "Indexing of URL %s failed\\n", fixed_repoURL) : fprintf(stderr, "\e[1;31mIndexing of URL %s failed\e[0m\n", fixed_repoURL);
		fprintf(log, "Indexing of URL %s failed\n", fixed_repoURL);
		fclose(file);
		remove(tmp_file_list);
		fclose(log);
		return OPH_TERM_MEMORY_ERROR;
	}
	curl_easy_setopt(curl1, CURLOPT_URL, fixed_repoURL);
	curl_easy_setopt(curl1, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl1, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(curl1, CURLOPT_CONNECTTIMEOUT, 10);
#ifdef NO_XML_SSL_CHECK
	curl_easy_setopt(curl1, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

	if (userpwd)
		curl_easy_setopt(curl1, CURLOPT_USERPWD, userpwd);
	/* Perform the request, res will get the return code */
	(print_json) ? my_printf("Getting list of Ophidia operators XML files from \\\"%s\\\"... ", fixed_repoURL) : printf("Getting list of Ophidia operators XML files from \"%s\"... ",
															    fixed_repoURL);
	fflush(stdout);
	fprintf(log, "Getting list of Ophidia operators XML files from \"%s\"... ", fixed_repoURL);
	res = curl_easy_perform(curl1);
	/* Check for errors */
	if (res != CURLE_OK) {
		(print_json) ? my_fprintf(stderr, "\\nIndexing of URL %s failed: %s\\n", fixed_repoURL, curl_easy_strerror(res)) : fprintf(stderr, "\n\e[1;31mIndexing of URL %s failed: %s\e[0m\n",
																	   fixed_repoURL, curl_easy_strerror(res));
		fprintf(log, "\nIndexing of URL %s failed: %s\n", fixed_repoURL, curl_easy_strerror(res));
		curl_easy_cleanup(curl1);
		fclose(file);
		remove(tmp_file_list);
		fclose(log);
		return OPH_TERM_GENERIC_ERROR;
	}
	curl_easy_cleanup(curl1);
	fclose(file);
	(print_json) ? my_printf("Done.\\n") : printf("Done.\n");
	fflush(stdout);
	fprintf(log, "Done.\n");

	file = fopen(tmp_file_list, "r");
	if (!file) {
		(print_json) ? my_fprintf(stderr, "Unable to read file %s\\n", tmp_file_list) : fprintf(stderr, "\e[1;31mUnable to read file %s\e[0m\n", tmp_file_list);
		fprintf(log, "Unable to read file %s\n", tmp_file_list);
		remove(tmp_file_list);
		fclose(log);
		return OPH_TERM_GENERIC_ERROR;
	}

	curl1 = curl_easy_init();
	if (!curl1) {
		(print_json) ? my_fprintf(stderr, "Unable to retrieve files\\n") : fprintf(stderr, "\e[1;31mUnable to retrieve files\e[0m\n");
		fprintf(log, "Unable to retrieve files\n");
		fclose(file);
		remove(tmp_file_list);
		fclose(log);
		return OPH_TERM_MEMORY_ERROR;
	}
	curl_easy_setopt(curl1, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl1, CURLOPT_NOBODY, 1);
	curl_easy_setopt(curl1, CURLOPT_FILETIME, 1);
	curl_easy_setopt(curl1, CURLOPT_CONNECTTIMEOUT, 10);
#ifdef NO_XML_SSL_CHECK
	curl_easy_setopt(curl1, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

	if (userpwd)
		curl_easy_setopt(curl1, CURLOPT_USERPWD, userpwd);

	curl2 = curl_easy_init();
	if (!curl2) {
		(print_json) ? my_fprintf(stderr, "Unable to retrieve files\\n") : fprintf(stderr, "\e[1;31mUnable to retrieve files\e[0m\n");
		fprintf(log, "Unable to retrieve files\n");
		curl_easy_cleanup(curl1);
		fclose(file);
		remove(tmp_file_list);
		fclose(log);
		return OPH_TERM_MEMORY_ERROR;
	}
	curl_easy_setopt(curl2, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl2, CURLOPT_CONNECTTIMEOUT, 10);
#ifdef NO_XML_SSL_CHECK
	curl_easy_setopt(curl2, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

	if (userpwd)
		curl_easy_setopt(curl2, CURLOPT_USERPWD, userpwd);

	(print_json) ? my_printf("Downloading necessary files... ") : printf("Downloading necessary files... ");
	fflush(stdout);
	fprintf(log, "Downloading necessary files... \n");
	while (getline(&line, &n, file) != -1) {
		ptr = strstr(line, "<a href=\"OPH_");
		if (!ptr)
			continue;
		ptr += 9;
		ptr2 = strstr(ptr, ".xml\">");
		if (!ptr2)
			continue;
		ptr2 += 4;
		size = strlen(ptr) - strlen(ptr2);
		memset(filename, 0, 100);
		memcpy(filename, ptr, size);
		filename[size] = '\0';
		remote_file_count++;
		memset(fileURL, 0, OPH_TERM_MAX_LEN);
		snprintf(fileURL, OPH_TERM_MAX_LEN, "%s%s", fixed_repoURL, filename);

		// ADD TO FILE LIST
		if (filelist_num == 0) {
			filelist = (char **) malloc(sizeof(char *));
			if (!filelist) {
				(print_json) ? my_fprintf(stderr, "\\nError allocating memory for filelist file %s\\n", filename) : fprintf(stderr,
																	    "\n\e[1;31mError allocating memory for filelist file %s\e[0m\n",
																	    filename);
				fprintf(log, "Error allocating memory for file %s\n", filename);
				continue;
			}
		} else {
			char **tmpfilelist = filelist;
			filelist = (char **) realloc(filelist, sizeof(char *) * (filelist_num + 1));
			if (!filelist) {
				filelist = tmpfilelist;
				(print_json) ? my_fprintf(stderr, "\\nError allocating memory for filelist file %s\\n", filename) : fprintf(stderr,
																	    "\n\e[1;31mError allocating memory for filelist file %s\e[0m\n",
																	    filename);
				fprintf(log, "Error allocating memory for file %s\n", filename);
				continue;
			}
		}
		filelist[filelist_num] = NULL;
		filelist[filelist_num] = (char *) strdup(filename);
		if (!filelist[filelist_num]) {
			(print_json) ? my_fprintf(stderr, "\\nError allocating memory for file %s\\n", filename) : fprintf(stderr, "\n\e[1;31mError allocating memory for file %s\e[0m\n", filename);
			fprintf(log, "Error allocating memory for file %s\n", filename);
			continue;
		}
		filelist_num++;

		// HEAD Request
		fprintf(log, "HEAD request for file %s\n", fileURL);
		curl_easy_setopt(curl1, CURLOPT_URL, fileURL);
		res = curl_easy_perform(curl1);
		if (res != CURLE_OK) {
			(print_json) ? my_fprintf(stderr, "\\nHEAD request for file %s failed: %s\\n", fileURL, curl_easy_strerror(res)) : fprintf(stderr,
																		   "\n\e[1;31mHEAD request for file %s failed: %s\e[0m\n",
																		   fileURL, curl_easy_strerror(res));
			fprintf(log, "HEAD request for file %s failed: %s\n", fileURL, curl_easy_strerror(res));
			continue;
		}
		res = curl_easy_getinfo(curl1, CURLINFO_FILETIME, &timestamp);
		if (res != CURLE_OK) {
			(print_json) ? my_fprintf(stderr, "\\nUnable to verify time of last modification for file %s\\n", fileURL) : fprintf(stderr,
																	     "\n\e[1;31mUnable to verify time of last modification for file %s\e[0m\n",
																	     fileURL);
			fprintf(log, "Unable to verify time of last modification for file %s\n", fileURL);
			continue;
		}

		memset(localfilename, 0, OPH_TERM_MAX_LEN);
		snprintf(localfilename, OPH_TERM_MAX_LEN, "%s/%s", local_dir, filename);

		// Compare Last-Modified times
		if (!stat(localfilename, &filestats)) {
			localtimestamp = filestats.st_mtime;
			if (localtimestamp >= timestamp) {
				fprintf(log, "Local file modification time %ld >= remote file modification time %ld\n", localtimestamp, timestamp);
				continue;
			}
		}
		// DOWNLOAD file
		fprintf(log, "Downloading file %s\n", filename);
		FILE *newfile = fopen(localfilename, "w");
		if (!newfile) {
			(print_json) ? my_fprintf(stderr, "\\nUnable to create file %s\\n", filename) : fprintf(stderr, "\n\e[1;31mUnable to create file %s\e[0m\n", filename);
			fprintf(log, "Unable to create file %s\n", filename);
			continue;
		}
		curl_easy_setopt(curl2, CURLOPT_URL, fileURL);
		curl_easy_setopt(curl2, CURLOPT_WRITEDATA, newfile);
		res = curl_easy_perform(curl2);
		if (res != CURLE_OK) {
			(print_json) ? my_fprintf(stderr, "\\nDownload of file %s failed: %s\\n", fileURL, curl_easy_strerror(res)) : fprintf(stderr, "\n\e[1;31mDownload of file %s failed: %s\e[0m\n",
																	      fileURL, curl_easy_strerror(res));
			fprintf(log, "Download of file %s failed: %s\n", fileURL, curl_easy_strerror(res));
			fclose(newfile);
			continue;
		}
		fclose(newfile);
		downloaded_count++;
		times[0].tv_sec = timestamp;
		times[0].tv_usec = 0;
		times[1].tv_sec = timestamp;
		times[1].tv_usec = 0;
		if (utimes(localfilename, times)) {
			(print_json) ? my_fprintf(stderr, "\\nUnable to sync time of last modification for local and remote file %s\\n", filename) : fprintf(stderr,
																			     "\n\e[1;31mUnable to sync time of last modification for local and remote file %s\e[0m\n",
																			     filename);
			fprintf(log, "Unable to sync time of last modification for local and remote file %s\n", filename);
		}
	}
	(print_json) ? my_printf("Done.\\n") : printf("Done.\n");
	fflush(stdout);
	fprintf(log, "Done.\n");

	// Cleanup
	if (curl1)
		curl_easy_cleanup(curl1);
	if (curl2)
		curl_easy_cleanup(curl2);
	if (line) {
		free(line);
		line = NULL;
	}
	fclose(file);
	remove(tmp_file_list);

	// Remove other local files
	dirp = opendir(local_dir);
	if (!dirp) {
		(print_json) ? my_fprintf(stderr, "Unable to count local files\\n") : fprintf(stderr, "\e[1;31mUnable to count local files\e[0m\n");
		fprintf(log, "Unable to count local files\n");
		fclose(log);
		if (filelist) {
			int i;
			for (i = 0; i < filelist_num; i++) {
				if (filelist[i]) {
					free(filelist[i]);
					filelist[i] = NULL;
				}
			}
			free(filelist);
			filelist = NULL;
		}
		return OPH_TERM_GENERIC_ERROR;
	}
	while ((entry = readdir(dirp)) != NULL) {
		int i;
		int rem = 1;
		for (i = 0; i < filelist_num; i++) {
			if (!strcmp(entry->d_name, filelist[i])) {
				rem = 0;
				break;
			}
		}
		char buf[OPH_TERM_MAX_LEN];
		snprintf(buf, OPH_TERM_MAX_LEN, "%s/%s", local_dir, entry->d_name);
		struct stat info;
		if (stat(buf, &info) != 0)
			continue;
		if (rem && strcmp(entry->d_name, "updates.log") && !(info.st_mode & S_IFDIR)) {
			remove(buf);
			local_file_count++;
			fprintf(log, "File %s removed.\n", buf);
		}
	}
	closedir(dirp);

	// Print summary
	(print_json) ? my_printf("Remote XML files: %d - Downloaded XML files: %d - Removed XML files: %d\\n", remote_file_count, downloaded_count,
				 local_file_count) : printf("Remote XML files: %d - Downloaded XML files: %d - Removed XML files: %d\n", remote_file_count, downloaded_count, local_file_count);
	fflush(stdout);
	fprintf(log, "Remote XML files: %d - Downloaded XML files: %d - Removed XML files: %d\n", remote_file_count, downloaded_count, local_file_count);

	fclose(log);
	if (filelist) {
		int i;
		for (i = 0; i < filelist_num; i++) {
			if (filelist[i]) {
				free(filelist[i]);
				filelist[i] = NULL;
			}
		}
		free(filelist);
		filelist = NULL;
	}

	return OPH_TERM_SUCCESS;
}


/* XML autocompletion management */

//Alloc hashtable
int oph_term_env_xmlauto_init(HASHTBL ** hashtbl, const char *local_dir)
{

	if (!local_dir) {
		(print_json) ? my_fprintf(stderr, "Null parameters\\n") : fprintf(stderr, "\e[1;31mNull parameters\e[0m\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	if (!(*hashtbl = hashtbl_create(50, NULL))) {
		(print_json) ? my_fprintf(stderr, "Error allocating hashtable for XML defs\\n") : fprintf(stderr, "\e[1;31mError allocating hashtable for XML defs\e[0m\n");
		return OPH_TERM_MEMORY_ERROR;
	}

	DIR *dirp = NULL;
	struct dirent *entry = NULL;
	char filepath[OPH_TERM_MAX_LEN];
	char operator_name[OPH_TERM_MAX_LEN];
	operator_argument *operator_args = NULL;
	size_t operator_args_size = 0;
	char *opnameptr = operator_name;

	dirp = opendir(local_dir);
	if (!dirp) {
		(print_json) ? my_fprintf(stderr, "Unable to open directory %s\\n", local_dir) : fprintf(stderr, "\e[1;31mUnable to open directory %s\e[0m\n", local_dir);
		hashtbl_destroy(*hashtbl);
		*hashtbl = NULL;
		return OPH_TERM_GENERIC_ERROR;
	}
	while ((entry = readdir(dirp)) != NULL) {
		if (!strncmp(entry->d_name, "OPH_", 4) && !strncmp(entry->d_name + strlen(entry->d_name) - 4, ".xml", 4)) {
			memset(filepath, 0, OPH_TERM_MAX_LEN);
			snprintf(filepath, OPH_TERM_MAX_LEN, "%s/%s", local_dir, entry->d_name);
			memset(opnameptr, 0, OPH_TERM_MAX_LEN);
			//LOAD XML
			if (oph_term_env_load_xml(filepath, &opnameptr, &operator_args, &operator_args_size)) {
				(print_json) ? my_fprintf(stderr, "Unable to parse XML %s\\n", entry->d_name) : fprintf(stderr, "\e[1;31mUnable to parse XML %s\e[0m\n", entry->d_name);
				hashtbl_destroy(*hashtbl);
				*hashtbl = NULL;
				closedir(dirp);
				return OPH_TERM_GENERIC_ERROR;
			}
			//INSERT INTO HASHTABLE
			if (hashtbl_insert(*hashtbl, opnameptr, (void *) operator_args, operator_args_size)) {
				(print_json) ? my_fprintf(stderr, "Unable to load XML %s\\n", entry->d_name) : fprintf(stderr, "\e[1;31mUnable to load XML %s\e[0m\n", entry->d_name);
				hashtbl_destroy(*hashtbl);
				*hashtbl = NULL;
				closedir(dirp);
				if (operator_args)
					free(operator_args);
				operator_args = NULL;
				return OPH_TERM_GENERIC_ERROR;
			}
			if (operator_args)
				free(operator_args);
			operator_args = NULL;
		}
	}
	closedir(dirp);

	return OPH_TERM_SUCCESS;
}

//Free hashtable
int oph_term_env_xmlauto_clear(HASHTBL * hashtbl, char **oplist, int oplist_size)
{
	if (hashtbl)
		hashtbl_destroy(hashtbl);
	if (oplist) {
		int i;
		for (i = 0; i < oplist_size; i++) {
			if (oplist[i]) {
				free(oplist[i]);
				oplist[i] = NULL;
			}
		}
		free(oplist);
		oplist = NULL;
	}
	return OPH_TERM_SUCCESS;
}

//Get all operators names
int oph_term_env_xmlauto_get_operators(HASHTBL * hashtbl, char ***list, int *list_size)
{

	if (!hashtbl) {
		(print_json) ? my_fprintf(stderr, "Null parameters\\n") : fprintf(stderr, "\e[1;31mNull parameters\e[0m\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	hash_size n;
	struct hashnode_s *node;
	*list_size = 0;
	int i = 0;
	int j;

	//Count operators
	for (n = 0; n < hashtbl->size; n++) {
		node = hashtbl->nodes[n];
		while (node) {
			if (!strncmp(node->key, OPH_TERM_ENV_OPH_PREFIX, OPH_TERM_ENV_OPH_PREFIX_LEN) && strncmp(node->key, OPH_TERM_ENV_OPH_MASSIVE, OPH_TERM_ENV_OPH_MASSIVE_LEN))
				(*list_size)++;
			node = node->next;
		}
	}

	//Create list of operators
	*list = (char **) malloc(sizeof(char *) * (*list_size));
	if (!(*list)) {
		(print_json) ? my_fprintf(stderr, "Error allocating list of operators\\n") : fprintf(stderr, "\e[1;31mError allocating list of operators\e[0m\n");
		return OPH_TERM_MEMORY_ERROR;
	}
	for (n = 0; n < hashtbl->size; n++) {
		node = hashtbl->nodes[n];
		while (node) {
			if (!strncmp(node->key, OPH_TERM_ENV_OPH_PREFIX, OPH_TERM_ENV_OPH_PREFIX_LEN) && strncmp(node->key, OPH_TERM_ENV_OPH_MASSIVE, OPH_TERM_ENV_OPH_MASSIVE_LEN)) {
				(*list)[i] = strdup(node->key);
				if (!(*list)[i]) {
					for (j = 0; j < i; j++) {
						free((*list)[j]);
					}
					free(*list);
					(print_json) ? my_fprintf(stderr, "Error allocating name in operators list\\n") : fprintf(stderr, "\e[1;31mError allocating name in operators list\e[0m\n");
					return OPH_TERM_MEMORY_ERROR;
				}
				i++;
			}
			node = node->next;
		}
	}

	return OPH_TERM_SUCCESS;
}

//Get an operator parameter list
int oph_term_env_xmlauto_get_parameters(HASHTBL * hashtbl, const char *key, operator_argument ** args, int *args_size)
{

	if (!hashtbl || !key) {
		(print_json) ? my_fprintf(stderr, "Null parameters\\n") : fprintf(stderr, "\e[1;31mNull parameters\e[0m\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	*args_size = 0;
	*args = NULL;

	*args = (operator_argument *) hashtbl_get(hashtbl, key);
	if (!*args) {
		(print_json) ? my_fprintf(stderr, "Unable to retrieve parameters for operator %s\\n", key) : fprintf(stderr, "\e[1;31mUnable to retrieve parameters for operator %s\e[0m\n", key);
		return OPH_TERM_INVALID_PARAM_VALUE;
	}
	//Count parameters
	int flag = 0;
	while (!flag) {
		if ((*args)[(*args_size)].is_last)
			flag = 1;
		(*args_size)++;
	}

	return OPH_TERM_SUCCESS;
}

// Start XML parser
int oph_term_env_start_xml_parser()
{
	xmlInitParser();
	LIBXML_TEST_VERSION return OPH_TERM_SUCCESS;
}

// Close XML parser
int oph_term_env_end_xml_parser()
{
	xmlCleanupParser();
	return OPH_TERM_SUCCESS;
}

int oph_term_env_strtolower(char **string)
{
	short int i;
	for (i = 0; i < (int) strlen(*string); i++) {
		(*string)[i] = tolower((*string)[i]);
	}
	return OPH_TERM_SUCCESS;
}

// Load operator arguments from XML
int oph_term_env_load_xml(const char *xmlfilename, char **operator_name, operator_argument ** operator_args, size_t * operator_args_size)
{

	if (!xmlfilename) {
		(print_json) ? my_fprintf(stderr, "Null parameters\\n") : fprintf(stderr, "\e[1;31mNull parameters\e[0m\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	xmlParserCtxtPtr ctxt;
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlChar *content;
	xmlXPathContextPtr xpathCtx;
	xmlXPathObjectPtr xpathObj;
	int n;

	/* create a parser context */
	ctxt = xmlNewParserCtxt();
	if (ctxt == NULL) {
		(print_json) ? my_fprintf(stderr, "Failed to allocate parser context\\n") : fprintf(stderr, "\e[1;31mFailed to allocate parser context\e[0m\n");
		return OPH_TERM_GENERIC_ERROR;
	}

	/* open the file */
	doc = xmlCtxtReadFile(ctxt, xmlfilename, NULL, 0);
	if (doc == NULL) {
		(print_json) ? my_fprintf(stderr, "Error: unable to parse '%s'\\n", xmlfilename) : fprintf(stderr, "\e[1;31mError: unable to parse '%s'\e[0m\n", xmlfilename);
		xmlFreeParserCtxt(ctxt);
		return OPH_TERM_GENERIC_ERROR;
	}

	/* Create xpath evaluation context */
	xpathCtx = xmlXPathNewContext(doc);
	if (xpathCtx == NULL) {
		(print_json) ? my_fprintf(stderr, "Error: unable to create new XPath context\\n") : fprintf(stderr, "\e[1;31mError: unable to create new XPath context\e[0m\n");
		xmlFreeDoc(doc);
		xmlFreeParserCtxt(ctxt);
		return OPH_TERM_GENERIC_ERROR;
	}
	// GET NAME
	xpathObj = xmlXPathEvalExpression((const xmlChar *) "/operator", xpathCtx);
	if (xpathObj == NULL) {
		(print_json) ? my_fprintf(stderr, "Error: unable to evaluate xpath expression\\n") : fprintf(stderr, "\e[1;31mError: unable to evaluate xpath expression\e[0m\n");
		xmlXPathFreeContext(xpathCtx);
		xmlFreeDoc(doc);
		xmlFreeParserCtxt(ctxt);
		return OPH_TERM_GENERIC_ERROR;
	}

	node = xpathObj->nodesetval->nodeTab[0];
	if (node == NULL) {
		(print_json) ? my_fprintf(stderr, "Error: unable to extract node\\n") : fprintf(stderr, "\e[1;31mError: unable to extract node\e[0m\n");
		xmlXPathFreeContext(xpathCtx);
		xmlFreeDoc(doc);
		xmlFreeParserCtxt(ctxt);
		return OPH_TERM_GENERIC_ERROR;
	}

	content = xmlGetProp(node, (const xmlChar *) "name");
	if (content == NULL) {
		(print_json) ? my_fprintf(stderr, "Error: unable to extract content\\n") : fprintf(stderr, "\e[1;31mError: unable to extract content\e[0m\n");
		xmlXPathFreeContext(xpathCtx);
		xmlFreeDoc(doc);
		xmlFreeParserCtxt(ctxt);
		return OPH_TERM_GENERIC_ERROR;
	}

	oph_term_env_strtolower((char **) &content);
	snprintf(*operator_name, OPH_TERM_MAX_LEN, "%s", content);
	xmlFree(content);
	xmlXPathFreeObject(xpathObj);
	//NAME end

	// GET ARGS
	xpathObj = xmlXPathEvalExpression((const xmlChar *) "//args/argument", xpathCtx);
	if (xpathObj == NULL) {
		(print_json) ? my_fprintf(stderr, "Error: unable to evaluate xpath expression\\n") : fprintf(stderr, "\e[1;31mError: unable to evaluate xpath expression\e[0m\n");
		xmlXPathFreeContext(xpathCtx);
		xmlFreeDoc(doc);
		xmlFreeParserCtxt(ctxt);
		return OPH_TERM_GENERIC_ERROR;
	}
	if (xpathObj->nodesetval->nodeNr) {
		*operator_args = (operator_argument *) malloc(sizeof(operator_argument) * (xpathObj->nodesetval->nodeNr));
		if (!*operator_args) {
			(print_json) ? my_fprintf(stderr, "Error allocating argument list\\n") : fprintf(stderr, "\e[1;31mError allocating argument list\e[0m\n");
			xmlXPathFreeObject(xpathObj);
			xmlXPathFreeContext(xpathCtx);
			xmlFreeDoc(doc);
			xmlFreeParserCtxt(ctxt);
			return OPH_TERM_MEMORY_ERROR;
		}
		memset(*operator_args, 0, sizeof(operator_argument) * (xpathObj->nodesetval->nodeNr));
	} else
		*operator_args = NULL;
	*operator_args_size = sizeof(operator_argument) * (xpathObj->nodesetval->nodeNr);

	// SET ARGS
	for (n = 0; n < xpathObj->nodesetval->nodeNr; n++) {
		node = xpathObj->nodesetval->nodeTab[n];
		if (!node) {
			(print_json) ? my_fprintf(stderr, "Error: argument %d is empty\\n", n) : fprintf(stderr, "\e[1;31mError: argument %d is empty\e[0m\n", n);
			xmlXPathFreeObject(xpathObj);
			xmlXPathFreeContext(xpathCtx);
			xmlFreeDoc(doc);
			xmlFreeParserCtxt(ctxt);
			if (*operator_args)
				free(*operator_args);
			*operator_args = NULL;
			return OPH_TERM_GENERIC_ERROR;
		}
		// SET NAME
		content = xmlNodeGetContent(node);
		if (!content) {
			(print_json) ? my_fprintf(stderr, "Error: argument %d is empty\\n", n) : fprintf(stderr, "\e[1;31mError: argument %d is empty\e[0m\n", n);
			xmlXPathFreeObject(xpathObj);
			xmlXPathFreeContext(xpathCtx);
			xmlFreeDoc(doc);
			xmlFreeParserCtxt(ctxt);
			if (*operator_args)
				free(*operator_args);
			*operator_args = NULL;
			return OPH_TERM_GENERIC_ERROR;
		}
		snprintf((*operator_args)[n].name, 30, "%s", content);
		xmlFree(content);

		// SET TYPE
		content = xmlGetProp(node, (const xmlChar *) "type");
		if (!content) {
			(print_json) ? my_fprintf(stderr, "Error: unable to extract content\\n") : fprintf(stderr, "\e[1;31mError: unable to extract content\e[0m\n");
			xmlXPathFreeObject(xpathObj);
			xmlXPathFreeContext(xpathCtx);
			xmlFreeDoc(doc);
			xmlFreeParserCtxt(ctxt);
			if (*operator_args)
				free(*operator_args);
			*operator_args = NULL;
			return OPH_TERM_GENERIC_ERROR;
		}

		snprintf((*operator_args)[n].type, 15, "%s", content);
		xmlFree(content);

		// SET MANDATORY
		content = xmlGetProp(node, (const xmlChar *) "mandatory");
		if (!content) {
			(*operator_args)[n].mandatory = 'M';
		} else {
			if (!strcmp((char *) content, "yes"))
				(*operator_args)[n].mandatory = 'M';
			else
				(*operator_args)[n].mandatory = 'O';
			xmlFree(content);
		}

		// SET DEFAULT
		content = xmlGetProp(node, (const xmlChar *) "default");
		if (!content) {
			snprintf((*operator_args)[n].default_val, 40, "%s", "");
		} else {
			snprintf((*operator_args)[n].default_val, 40, "%s", content);
			xmlFree(content);
		}

		// SET MINVAL
		content = xmlGetProp(node, (const xmlChar *) "minvalue");
		if (!content) {
			snprintf((*operator_args)[n].min, 8, "%s", "");
		} else {
			snprintf((*operator_args)[n].min, 8, "%s", content);
			xmlFree(content);
		}

		// SET MAXVAL
		content = xmlGetProp(node, (const xmlChar *) "maxvalue");
		if (!content) {
			snprintf((*operator_args)[n].max, 8, "%s", "");
		} else {
			snprintf((*operator_args)[n].max, 8, "%s", content);
			xmlFree(content);
		}

		// SET VALUES
		content = xmlGetProp(node, (const xmlChar *) "values");
		if (!content) {
			snprintf((*operator_args)[n].values, 200, "%s", "");
		} else {
			snprintf((*operator_args)[n].values, 200, "%s", content);
			xmlFree(content);
		}

		// SET IS_LAST
		if (n == (xpathObj->nodesetval->nodeNr) - 1) {
			(*operator_args)[n].is_last = 1;
		} else {
			(*operator_args)[n].is_last = 0;
		}
	}

	/* Cleanup of XPath object */
	xmlXPathFreeObject(xpathObj);
	/* Cleanup of XPath context */
	xmlXPathFreeContext(xpathCtx);
	/* free up the resulting document */
	xmlFreeDoc(doc);
	/* free up the parser context */
	xmlFreeParserCtxt(ctxt);

	return OPH_TERM_SUCCESS;
}

// Remove a directory recursively
int oph_term_env_rmdir(const char *local_dir)
{
	struct stat info;
	if (stat(local_dir, &info) != 0) {
		(print_json) ? my_fprintf(stderr, "Could not retrieve stats for %s.\\n", local_dir) : fprintf(stderr, "Could not retrieve stats for %s.\n", local_dir);
		return OPH_TERM_GENERIC_ERROR;
	}
	if (info.st_mode & S_IFDIR) {
		DIR *dirp = NULL;
		struct dirent *entry = NULL;
		char buffer[OPH_TERM_MAX_LEN];

		dirp = opendir(local_dir);
		if (!dirp) {
			(print_json) ? my_fprintf(stderr, "Could not open %s.\\n", local_dir) : fprintf(stderr, "Could not open %s.\n", local_dir);
			return OPH_TERM_GENERIC_ERROR;
		}
		while ((entry = readdir(dirp)) != NULL) {
			memset(buffer, 0, OPH_TERM_MAX_LEN);
			snprintf(buffer, OPH_TERM_MAX_LEN, "%s/%s", local_dir, entry->d_name);
			struct stat info2;
			if (stat(buffer, &info2) != 0) {
				(print_json) ? my_fprintf(stderr, "Could not retrieve stats for %s.\\n", buffer) : fprintf(stderr, "Could not retrieve stats for %s.\n", buffer);
				closedir(dirp);
				return OPH_TERM_GENERIC_ERROR;
			}
			if (info2.st_mode & S_IFDIR) {
				if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
					if (oph_term_env_rmdir(buffer)) {
						(print_json) ? my_fprintf(stderr, "Could not remove directory %s.\\n", buffer) : fprintf(stderr, "Could not remove directory %s.\n", buffer);
						closedir(dirp);
						return OPH_TERM_GENERIC_ERROR;
					}
				}
			} else {
				if (remove(buffer)) {
					(print_json) ? my_fprintf(stderr, "Could not remove file %s.\\n", buffer) : fprintf(stderr, "Could not remove file %s.\n", buffer);
					closedir(dirp);
					return OPH_TERM_GENERIC_ERROR;
				}
			}
		}
		closedir(dirp);
		if (remove(local_dir)) {
			(print_json) ? my_fprintf(stderr, "Could not remove directory %s.\\n", local_dir) : fprintf(stderr, "Could not remove directory %s.\n", local_dir);
			return OPH_TERM_GENERIC_ERROR;
		}
	} else {
		(print_json) ? my_fprintf(stderr, "%s is not a directory.\\n", local_dir) : fprintf(stderr, "%s is not a directory.\n", local_dir);
		return OPH_TERM_GENERIC_ERROR;
	}

	return OPH_TERM_SUCCESS;
}

int oph_term_env_oph_get_config(const char *key, const char *host, const char *port, int *return_value, char **property, const char *user, const char *passwd, int workflow_wrap, HASHTBL * hashtbl)
{
	if (!key || !property || !host || !port) {
		fprintf(stderr, "NULL parameters.\n");
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}
	char query[OPH_TERM_MAX_LEN];
	snprintf(query, OPH_TERM_MAX_LEN, OPH_TERM_GET_CONFIG_PROP);
	char *value = NULL;

	if (get_config && oph_term_client(query, query, NULL, (char *) user, (char *) passwd, (char *) host, (char *) port, return_value, &value, NULL, workflow_wrap, hashtbl)) {
		if (value)
			free(value);
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}
	if (*return_value) {
		if (value)
			free(value);
		return OPH_TERM_GENERIC_ERROR;
	}

	unsigned int i, nprops = 0;
	char **keys = NULL, **props = NULL, *newtoken = NULL;
	if (get_config && oph_term_viewer_retrieve_config(value, key, &keys, &props, &nprops, &newtoken)) {
		if (value)
			free(value);
		if (newtoken)
			free(newtoken);
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	if (keys && props) {
		if (!_oph_term_env_init(&conf_hashtbl, nprops) && conf_hashtbl) {
			for (i = 0; i < nprops; i++) {
				hashtbl_remove(conf_hashtbl, keys[i]);
				hashtbl_insert(conf_hashtbl, keys[i], props[i], strlen(props[i]) + 1);
			}
			get_config = 0;	// No other request will be sent
		} else
			*return_value = OPH_TERM_GENERIC_ERROR;
	}
	if (keys) {
		for (i = 0; i < nprops; i++)
			if (keys[i])
				free(keys[i]);
		free(keys);
	}
	if (props) {
		for (i = 0; i < nprops; i++)
			if (props[i])
				free(props[i]);
		free(props);
	}

	if (newtoken) {
		pthread_mutex_lock(&global_flag);
		hashtbl_remove(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
		hashtbl_insert(hashtbl, OPH_TERM_ENV_OPH_TOKEN, newtoken, strlen(newtoken) + 1);
		_passwd = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
		pthread_mutex_unlock(&global_flag);
		free(newtoken);
	}

	if (value)
		free(value);

	if (*return_value)
		return *return_value;

	char *current_property = hashtbl_get(conf_hashtbl, key);
	if (!current_property) {
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	*property = strdup(current_property);
	if (!*property) {
		*return_value = OPH_TERM_MEMORY_ERROR;
		return OPH_TERM_MEMORY_ERROR;
	}
	return OPH_TERM_SUCCESS;
}

int oph_term_read_file(char *filename, char **buffer)
{
	if (!filename || !buffer) {
		(print_json) ? my_fprintf(stderr, "NULL parameters.\\n") : fprintf(stderr, "NULL parameters.\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	char _filename[OPH_TERM_MAX_LEN];
	if (oph_base_src_path && (strlen(oph_base_src_path) > 1) && (*filename == '/'))
		snprintf(_filename, OPH_TERM_MAX_LEN, "%s%s", oph_base_src_path, filename);
	else
		snprintf(_filename, OPH_TERM_MAX_LEN, "%s", filename);

	FILE *file;
	file = fopen(_filename, "rb");
	if (!file) {
		(print_json) ? my_fprintf(stderr, "Unable to open file %s\\n", filename) : fprintf(stderr, "Unable to open file %s\n", filename);
		return OPH_TERM_GENERIC_ERROR;
	}

	fseek(file, 0, SEEK_END);
	long buffer_len = ftell(file);
	fseek(file, 0, SEEK_SET);

	*buffer = (char *) calloc(buffer_len + 1, sizeof(char));
	if (!(*buffer)) {
		(print_json) ? my_fprintf(stderr, "Error allocating space for file contents\\n") : fprintf(stderr, "Error allocating space for file contents\n");
		fclose(file);
		return OPH_TERM_MEMORY_ERROR;
	}

	if (!fread(*buffer, buffer_len, 1, file))
		(print_json) ? my_fprintf(stderr, "Error reading buffer\\n") : fprintf(stderr, "Error reading buffer\n");
	fclose(file);

	return OPH_TERM_SUCCESS;
}

int oph_term_read_file_with_len(char *filename, char **buffer, long *alloc_size)
{
	if (!filename || !buffer || !alloc_size) {
		(print_json) ? my_fprintf(stderr, "NULL parameters.\\n") : fprintf(stderr, "NULL parameters.\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	char _filename[OPH_TERM_MAX_LEN];
	if (oph_base_src_path && (strlen(oph_base_src_path) > 1) && (*filename == '/'))
		snprintf(_filename, OPH_TERM_MAX_LEN, "%s%s", oph_base_src_path, filename);
	else
		snprintf(_filename, OPH_TERM_MAX_LEN, "%s", filename);

	FILE *file;
	file = fopen(_filename, "rb");
	if (!file) {
		(print_json) ? my_fprintf(stderr, "Unable to open file %s\\n", filename) : fprintf(stderr, "Unable to open file %s\n", filename);
		return OPH_TERM_GENERIC_ERROR;
	}

	fseek(file, 0, SEEK_END);
	long buffer_len = ftell(file);
	fseek(file, 0, SEEK_SET);

	*buffer = (char *) calloc(buffer_len + 1, sizeof(char));
	if (!(*buffer)) {
		(print_json) ? my_fprintf(stderr, "Error allocating space for file contents\\n") : fprintf(stderr, "Error allocating space for file contents\n");
		fclose(file);
		return OPH_TERM_MEMORY_ERROR;
	}
	*alloc_size = buffer_len + 1;

	if (!fread(*buffer, buffer_len, 1, file))
		(print_json) ? my_fprintf(stderr, "Error reading buffer\\n") : fprintf(stderr, "Error reading buffer\n");
	fclose(file);

	return OPH_TERM_SUCCESS;
}

int oph_term_view_filters_parsing(char *filters, int end, int **jobs, int *jobs_num)
{
	if (!filters || end <= 0 || !jobs || !jobs_num) {
		(print_json) ? my_fprintf(stderr, "Unable to parse markers: NULL parameters.\\n") : fprintf(stderr, "\e[1;31mUnable to parse markers: NULL parameters.\e[0m\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	int *job_flags = (int *) calloc(end + 1, sizeof(int));
	if (!job_flags)
		return OPH_TERM_MEMORY_ERROR;

	char single_filter[OPH_TERM_MAX_LEN];
	char single_extreme[OPH_TERM_MAX_LEN];
	char *cursor1, *cursor2, *cursor3;
	char *saveptr1, *saveptr2, *saveptr3;
	int left, right;
	int is_range;
	int i, j;

	cursor1 = strtok_r(filters, ",", &saveptr1);
	while (cursor1) {
		memset(single_filter, 0, OPH_TERM_MAX_LEN);
		snprintf(single_filter, OPH_TERM_MAX_LEN, "%s", cursor1);
		left = right = 0;
		is_range = 0;

		//extract left (& right)
		cursor2 = strtok_r(single_filter, ":", &saveptr2);
		left = (int) strtol(cursor2, NULL, 10);
		if (left == 0) {
			memset(single_extreme, 0, OPH_TERM_MAX_LEN);
			snprintf(single_extreme, OPH_TERM_MAX_LEN, "%s", cursor2);

			cursor3 = strtok_r(single_extreme, "-", &saveptr3);
			if (!cursor3 || strcmp(cursor3, "end")) {
				(print_json) ? my_fprintf(stderr, "Unable to parse markers: Invalid filter string.\\n") : fprintf(stderr,
																  "\e[1;31mUnable to parse markers: Invalid filter string.\e[0m\n");
				if (job_flags)
					free(job_flags);
				return OPH_TERM_INVALID_PARAM_VALUE;
			}
			cursor3 = strtok_r(NULL, "-", &saveptr3);
			left = end;
			if (cursor3) {
				int num;
				num = (int) strtol(cursor3, NULL, 10);
				if (num == 0) {
					(print_json) ? my_fprintf(stderr, "Unable to parse markers: Invalid filter string.\\n") : fprintf(stderr,
																	  "\e[1;31mUnable to parse markers: Invalid filter string.\e[0m\n");
					if (job_flags)
						free(job_flags);
					return OPH_TERM_INVALID_PARAM_VALUE;
				}
				left -= num;
			}
		}
		cursor2 = strtok_r(NULL, ":", &saveptr2);
		if (cursor2) {
			right = (int) strtol(cursor2, NULL, 10);
			if (right == 0) {
				memset(single_extreme, 0, OPH_TERM_MAX_LEN);
				snprintf(single_extreme, OPH_TERM_MAX_LEN, "%s", cursor2);

				cursor3 = strtok_r(single_extreme, "-", &saveptr3);
				if (!cursor3 || strcmp(cursor3, "end")) {
					(print_json) ? my_fprintf(stderr, "Unable to parse markers: Invalid filter string.\\n") : fprintf(stderr,
																	  "\e[1;31mUnable to parse markers: Invalid filter string.\e[0m\n");
					if (job_flags)
						free(job_flags);
					return OPH_TERM_INVALID_PARAM_VALUE;
				}
				cursor3 = strtok_r(NULL, "-", &saveptr3);
				right = end;
				if (cursor3) {
					int num;
					num = (int) strtol(cursor3, NULL, 10);
					if (num == 0) {
						(print_json) ? my_fprintf(stderr, "Unable to parse markers: Invalid filter string.\\n") : fprintf(stderr,
																		  "\e[1;31mUnable to parse markers: Invalid filter string.\e[0m\n");
						if (job_flags)
							free(job_flags);
						return OPH_TERM_INVALID_PARAM_VALUE;
					}
					right -= num;
				}
			}
			is_range = 1;
		}
		//check if left or left-right are valid
		//(1 <= left <= end) && ((left < right <= end && is_range) || !is_range)
		if (!((left >= 1 && left <= end) && ((right > left && right <= end && is_range) || !is_range))) {
			(print_json) ? my_fprintf(stderr, "Unable to parse markers: Out of range filter extremes.\\n") : fprintf(stderr,
																 "\e[1;31mUnable to parse markers: Out of range filter extremes.\e[0m\n");
			if (job_flags)
				free(job_flags);
			return OPH_TERM_INVALID_PARAM_VALUE;
		}
		//set flags accordingly
		if (is_range) {
			for (i = left; i <= right; i++)
				job_flags[i] = 1;
		} else {
			job_flags[left] = 1;
		}

		//next iteration
		cursor1 = strtok_r(NULL, ",", &saveptr1);
	}

	//compute jobs_num
	*jobs_num = 0;
	for (i = 1; i <= end; i++)
		if (job_flags[i])
			(*jobs_num)++;

	//alloc jobs
	*jobs = (int *) calloc((*jobs_num), sizeof(int));
	if (!(*jobs)) {
		(print_json) ? my_fprintf(stderr, "Unable to parse markers: Unable to alloc jobs.\\n") : fprintf(stderr, "\e[1;31mUnable to parse markers: Unable to alloc jobs.\e[0m\n");
		*jobs_num = 0;
		if (job_flags)
			free(job_flags);
		return OPH_TERM_MEMORY_ERROR;
	}
	//get all markers
	j = 0;
	for (i = 1; i <= end && j < (*jobs_num); i++) {
		if (job_flags[i]) {
			(*jobs)[j] = i;
			j++;
		}
	}

	if (job_flags)
		free(job_flags);
	return OPH_TERM_SUCCESS;
}

int oph_term_get_session_size(char *session, char *user, char *passwd, char *host, char *port, int *return_value, int *size, int workflow_wrap, HASHTBL * hashtbl, char ***exit_status)
{
	if (!session || !host || !port || !return_value || !size) {
		(print_json) ? my_fprintf(stderr, "NULL parameters.\\n") : fprintf(stderr, "\e[1;31mNULL parameters.\e[0m\n");
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	char query[OPH_TERM_MAX_LEN];
	snprintf(query, OPH_TERM_MAX_LEN, OPH_TERM_OPH_RESUME_STRING_NO_SAVE "session=%s;", session);
	char *value = NULL;

	if (oph_term_client(query, query, NULL, (char *) user, (char *) passwd, (char *) host, (char *) port, return_value, &value, NULL, workflow_wrap, hashtbl)) {
		if (value)
			free(value);
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	if (*return_value) {
		if (value)
			free(value);
		return OPH_TERM_GENERIC_ERROR;
	}

	if (oph_term_viewer_retrieve_session_size(value, size, exit_status)) {
		(print_json) ? my_fprintf(stderr, "Unable to get session size from json.\\n") : fprintf(stderr, "\e[1;31mUnable to get session size from json.\e[0m\n");
		if (value)
			free(value);
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	if (value)
		free(value);
	return OPH_TERM_SUCCESS;
}

int oph_term_switch_remote_session(char *session, char *user, char *passwd, char *host, char *port, int *return_value, int workflow_wrap, HASHTBL * hashtbl)
{
	if (!session || !host || !port || !return_value) {
		(print_json) ? my_fprintf(stderr, "NULL parameters.\\n") : fprintf(stderr, "\e[1;31mNULL parameters.\e[0m\n");
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	char query[OPH_TERM_MAX_LEN];
	snprintf(query, OPH_TERM_MAX_LEN, OPH_TERM_OPH_RESUME_STRING_SAVE "session=%s;", session);
	char *value = NULL;

	if (oph_term_client(query, query, NULL, (char *) user, (char *) passwd, (char *) host, (char *) port, return_value, &value, NULL, workflow_wrap, hashtbl)) {
		if (value)
			free(value);
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	if (*return_value) {
		if (value)
			free(value);
		return OPH_TERM_GENERIC_ERROR;
	}

	if (!oph_term_viewer_is_session_switched(value)) {
		(print_json) ? my_fprintf(stderr, "There was a problem switching session.\\n") : fprintf(stderr, "\e[1;31mThere was a problem switching session.\e[0m\n");
		if (value)
			free(value);
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	if (value)
		free(value);
	return OPH_TERM_SUCCESS;
}

int oph_term_var_expansion(char *param_string, char *variable, char *value, char **expanded_string)
{
	if (!param_string || !variable || !value || !expanded_string) {
		(print_json) ? my_fprintf(stderr, "NULL parameters.\\n") : fprintf(stderr, "NULL parameters.\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	char buf[OPH_TERM_WF_MAX_LEN];
	int i, j, m;
	char c;

	*expanded_string = (char *) calloc(OPH_TERM_WF_MAX_LEN, sizeof(char));
	if (!*expanded_string) {
		(print_json) ? my_fprintf(stderr, "Error allocating expanded string\\n") : fprintf(stderr, "Error allocating expanded string\n");
		return OPH_TERM_MEMORY_ERROR;
	}

	snprintf(*expanded_string, OPH_TERM_WF_MAX_LEN, "%s", param_string);

	for (i = 0; i < (int) strlen(*expanded_string); i++) {
		if ((*expanded_string)[i] == '$' && (*expanded_string)[i + 1] != '{') {
			for (j = i + 1; j < (int) strlen(*expanded_string); j++) {
				if (!isalnum((*expanded_string)[j]) && (*expanded_string)[j] != '_')
					break;
			}
			if (j != i + 1) {
				c = (*expanded_string)[j];
				(*expanded_string)[j] = 0;
				m = strcmp((*expanded_string) + i + 1, variable);
				(*expanded_string)[j] = c;
				if (m == 0) {
					(*expanded_string)[i] = 0;
					memset(buf, 0, OPH_TERM_WF_MAX_LEN);
					snprintf(buf, OPH_TERM_WF_MAX_LEN, "%s%s%s", *expanded_string, value, (*expanded_string) + j);
					memset(*expanded_string, 0, OPH_TERM_WF_MAX_LEN);
					snprintf(*expanded_string, OPH_TERM_WF_MAX_LEN, "%s", buf);
					i += strlen(value) - 1;
				}
			}
		} else if ((*expanded_string)[i] == '$' && (*expanded_string)[i + 1] == '{') {
			for (j = i + 2; j < (int) strlen(*expanded_string); j++) {
				if ((*expanded_string)[j] == '}')
					break;
			}
			if (j != i + 2) {
				c = (*expanded_string)[j];
				(*expanded_string)[j] = 0;
				m = strcmp((*expanded_string) + i + 2, variable);
				(*expanded_string)[j] = c;
				if (m == 0) {
					(*expanded_string)[i] = 0;
					memset(buf, 0, OPH_TERM_WF_MAX_LEN);
					snprintf(buf, OPH_TERM_WF_MAX_LEN, "%s%s%s", *expanded_string, value, (*expanded_string) + j + 1);
					memset(*expanded_string, 0, OPH_TERM_WF_MAX_LEN);
					snprintf(*expanded_string, OPH_TERM_WF_MAX_LEN, "%s", buf);
					i += strlen(value) - 1;
				}
			}
		}
	}

	return OPH_TERM_SUCCESS;
}

int oph_term_full_var_expansion(char *param_string, HASHTBL * hashtbl, char **expanded_string)
{
	if (!param_string || !hashtbl || !expanded_string) {
		(print_json) ? my_fprintf(stderr, "NULL parameters.\\n") : fprintf(stderr, "NULL parameters.\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	char buf[OPH_TERM_WF_MAX_LEN];
	char buf2[OPH_TERM_WF_MAX_LEN];
	int i, j, m, z;
	char c;

	*expanded_string = (char *) calloc(OPH_TERM_WF_MAX_LEN, sizeof(char));
	if (!*expanded_string) {
		(print_json) ? my_fprintf(stderr, "Error allocating expanded string\\n") : fprintf(stderr, "Error allocating expanded string\n");
		return OPH_TERM_MEMORY_ERROR;
	}

	snprintf(*expanded_string, OPH_TERM_WF_MAX_LEN, "%s", param_string);

	z = 0;
	while (strchr(*expanded_string, '$') && (z < OPH_TERM_SUBSTITUTION_MAX_CYCLES)) {
		for (i = 0; i < (int) strlen(*expanded_string); i++) {
			if ((*expanded_string)[i] == '$' && (*expanded_string)[i + 1] != '{') {
				for (j = i + 1; j < (int) strlen(*expanded_string); j++) {
					if (!isalnum((*expanded_string)[j]) && (*expanded_string)[j] != '_')
						break;
				}
				c = (*expanded_string)[j];
				(*expanded_string)[j] = 0;
				memset(buf2, 0, OPH_TERM_WF_MAX_LEN);
				snprintf(buf2, OPH_TERM_WF_MAX_LEN, "%s", (*expanded_string) + i + 1);
				m = (hashtbl_get(hashtbl, buf2)) ? 1 : 0;
				(*expanded_string)[j] = c;
				if (m) {
					(*expanded_string)[i] = 0;
					memset(buf, 0, OPH_TERM_WF_MAX_LEN);
					snprintf(buf, OPH_TERM_WF_MAX_LEN, "%s%s%s", *expanded_string, (char *) hashtbl_get(hashtbl, buf2), (*expanded_string) + j);
					memset(*expanded_string, 0, OPH_TERM_WF_MAX_LEN);
					snprintf(*expanded_string, OPH_TERM_WF_MAX_LEN, "%s", buf);
					i += strlen((char *) hashtbl_get(hashtbl, buf2)) - 1;
				} else {
					(*expanded_string)[i] = 0;
					memset(buf, 0, OPH_TERM_WF_MAX_LEN);
					snprintf(buf, OPH_TERM_WF_MAX_LEN, "%s%s", *expanded_string, (*expanded_string) + j);
					memset(*expanded_string, 0, OPH_TERM_WF_MAX_LEN);
					snprintf(*expanded_string, OPH_TERM_WF_MAX_LEN, "%s", buf);
					i -= 1;
				}
				z++;
			} else if ((*expanded_string)[i] == '$' && (*expanded_string)[i + 1] == '{') {
				for (j = i + 2; j < (int) strlen(*expanded_string); j++) {
					if ((*expanded_string)[j] == '}')
						break;
				}
				c = (*expanded_string)[j];
				(*expanded_string)[j] = 0;
				memset(buf2, 0, OPH_TERM_WF_MAX_LEN);
				snprintf(buf2, OPH_TERM_WF_MAX_LEN, "%s", (*expanded_string) + i + 2);
				m = (hashtbl_get(hashtbl, buf2)) ? 1 : 0;
				(*expanded_string)[j] = c;
				if (m) {
					(*expanded_string)[i] = 0;
					memset(buf, 0, OPH_TERM_WF_MAX_LEN);
					snprintf(buf, OPH_TERM_WF_MAX_LEN, "%s%s%s", *expanded_string, (char *) hashtbl_get(hashtbl, buf2),
						 ((*expanded_string)[j] == '\0') ? (*expanded_string) + j : (*expanded_string) + j + 1);
					memset(*expanded_string, 0, OPH_TERM_WF_MAX_LEN);
					snprintf(*expanded_string, OPH_TERM_WF_MAX_LEN, "%s", buf);
					i += strlen((char *) hashtbl_get(hashtbl, buf2)) - 1;
				} else {
					(*expanded_string)[i] = 0;
					memset(buf, 0, OPH_TERM_WF_MAX_LEN);
					snprintf(buf, OPH_TERM_WF_MAX_LEN, "%s%s", *expanded_string, ((*expanded_string)[j] == '\0') ? (*expanded_string) + j : (*expanded_string) + j + 1);
					memset(*expanded_string, 0, OPH_TERM_WF_MAX_LEN);
					snprintf(*expanded_string, OPH_TERM_WF_MAX_LEN, "%s", buf);
					i -= 1;
				}
				z++;
			}
		}
	}
	if (z >= OPH_TERM_SUBSTITUTION_MAX_CYCLES) {
		if (*expanded_string) {
			free(*expanded_string);
			*expanded_string = NULL;
		}
		(print_json) ? my_fprintf(stderr, "Too much substitutions! Perhaps there are one or more cycles.\\n") : fprintf(stderr,
																"Too much substitutions! Perhaps there are one or more cycles.\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	return OPH_TERM_SUCCESS;
}

int oph_term_get_session_code(char *sessionid, char *code)
{
	if (!sessionid || !code) {
		return OPH_TERM_INVALID_PARAM_VALUE;
	}
	char tmp[OPH_TERM_MAX_LEN];
	snprintf(tmp, OPH_TERM_MAX_LEN, "%s", sessionid);
	char *ptr = strrchr(tmp, '/');
	if (ptr) {
		ptr[0] = 0;
		ptr = strrchr(tmp, '/');
		if (ptr) {
			snprintf(code, OPH_TERM_MAX_LEN, "%s", ptr + 1);
		} else
			return OPH_TERM_INVALID_PARAM_VALUE;
	} else
		return OPH_TERM_INVALID_PARAM_VALUE;
	return OPH_TERM_SUCCESS;
}

int oph_term_get_request(char *session, char *wid, char *user, char *passwd, char *host, char *port, int *return_value, char **command, char **jobid, char **request_time, HASHTBL * hashtbl)
{
	if (!session || !wid || !host || !port || !return_value) {
		(print_json) ? my_fprintf(stderr, "NULL parameters.\\n") : fprintf(stderr, "\e[1;31mNULL parameters.\e[0m\n");
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	char query[OPH_TERM_MAX_LEN];
	snprintf(query, OPH_TERM_MAX_LEN, OPH_TERM_OPH_RESUME_STRING_NO_SAVE "document_type=request;id=%s;session=%s;", wid, session);
	char *value = NULL;

	if (oph_term_client(query, query, NULL, (char *) user, (char *) passwd, (char *) host, (char *) port, return_value, &value, NULL, 1, hashtbl)) {
		if (value)
			free(value);
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	if (*return_value) {
		if (value)
			free(value);
		return OPH_TERM_GENERIC_ERROR;
	}

	if (oph_term_viewer_retrieve_command_jobid_creation(value, command, jobid, request_time)) {
		(print_json) ? my_fprintf(stderr, "Unable to get command and/or jobid from json.\\n") : fprintf(stderr, "\e[1;31mUnable to get command and/or jobid from json.\e[0m\n");
		if (value)
			free(value);
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	if (value)
		free(value);
	return OPH_TERM_SUCCESS;
}

int oph_term_get_request_with_marker(char *session, char *mkid, char *user, char *passwd, char *host, char *port, int *return_value, char **command, char **jobid, HASHTBL * hashtbl)
{
	if (!session || !mkid || !host || !port || !return_value) {
		(print_json) ? my_fprintf(stderr, "NULL parameters.\\n") : fprintf(stderr, "\e[1;31mNULL parameters.\e[0m\n");
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	char query[OPH_TERM_MAX_LEN];
	snprintf(query, OPH_TERM_MAX_LEN, OPH_TERM_OPH_RESUME_STRING_NO_SAVE "document_type=request;id_type=marker;id=%s;session=%s;", mkid, session);
	char *value = NULL;

	if (oph_term_client(query, query, NULL, (char *) user, (char *) passwd, (char *) host, (char *) port, return_value, &value, NULL, 1, hashtbl)) {
		if (value)
			free(value);
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	if (*return_value) {
		if (value)
			free(value);
		return OPH_TERM_GENERIC_ERROR;
	}

	if (oph_term_viewer_retrieve_command_jobid(value, command, jobid)) {
		(print_json) ? my_fprintf(stderr, "Unable to get command and/or jobid from json.\\n") : fprintf(stderr, "\e[1;31mUnable to get command and/or jobid from json.\e[0m\n");
		if (value)
			free(value);
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	if (value)
		free(value);
	return OPH_TERM_SUCCESS;
}

int oph_term_get_full_request(char *session, char *wid, char *user, char *passwd, char *host, char *port, int *return_value, char **command, char **jobid, HASHTBL * hashtbl)
{
	if (!session || !wid || !host || !port || !return_value) {
		(print_json) ? my_fprintf(stderr, "NULL parameters.\\n") : fprintf(stderr, "\e[1;31mNULL parameters.\e[0m\n");
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	char query[OPH_TERM_MAX_LEN];
	snprintf(query, OPH_TERM_MAX_LEN, OPH_TERM_OPH_RESUME_STRING_NO_SAVE "document_type=request;level=3;id=%s;session=%s;", wid, session);
	char *value = NULL;

	if (oph_term_client(query, query, NULL, (char *) user, (char *) passwd, (char *) host, (char *) port, return_value, &value, NULL, 1, hashtbl)) {
		if (value)
			free(value);
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	if (oph_term_viewer_retrieve_command_jobid(value, command, jobid)) {
		(print_json) ? my_fprintf(stderr, "Unable to get command and/or jobid from json.\\n") : fprintf(stderr, "\e[1;31mUnable to get command and/or jobid from json.\e[0m\n");
		if (value)
			free(value);
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	if (value)
		free(value);
	return OPH_TERM_SUCCESS;
}

int oph_term_check_wid_mkid(char *session, char *wid, char *mkid, char *user, char *passwd, char *host, char *port, int *return_value, HASHTBL * hashtbl)
{
	if (!session || !wid || !mkid || !host || !port || !return_value) {
		(print_json) ? my_fprintf(stderr, "NULL parameters.\\n") : fprintf(stderr, "\e[1;31mNULL parameters.\e[0m\n");
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	char query[OPH_TERM_MAX_LEN];
	snprintf(query, OPH_TERM_MAX_LEN, OPH_TERM_OPH_RESUME_STRING_NO_SAVE "id=0;id_type=marker;session=%s;", session);
	char *value = NULL;

	if (oph_term_client(query, query, NULL, (char *) user, (char *) passwd, (char *) host, (char *) port, return_value, &value, NULL, 1, hashtbl)) {
		if (value)
			free(value);
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	if (*return_value) {
		if (value)
			free(value);
		return OPH_TERM_GENERIC_ERROR;
	}

	if (oph_term_viewer_check_wid_mkid(value, wid, mkid)) {
		(print_json) ? my_fprintf(stderr, "The identifiers %s#%s are not valid.\\n", wid, mkid) : fprintf(stderr, "\e[1;31mThe identifiers %s#%s are not valid.\e[0m\n", wid, mkid);
		if (value)
			free(value);
		*return_value = OPH_TERM_GENERIC_ERROR;
		return OPH_TERM_GENERIC_ERROR;
	}

	if (value)
		free(value);
	return OPH_TERM_SUCCESS;
}




/* ALIAS */

//Alloc hashtable
int oph_term_alias_init(HASHTBL ** hashtbl)
{
	if (!(*hashtbl = hashtbl_create(20, NULL))) {
		(print_json) ? my_fprintf(stderr, "Error: alias_init failed\\n") : fprintf(stderr, "\e[1;31mError: alias_init failed\e[0m\n");
		return OPH_TERM_MEMORY_ERROR;
	}
	return OPH_TERM_SUCCESS;
}

//Free hashtable
int oph_term_alias_clear(HASHTBL * hashtbl)
{
	hashtbl_destroy(hashtbl);
	return OPH_TERM_SUCCESS;
}

//Print all aliases
int oph_term_alias(HASHTBL * hashtbl)
{
	hash_size n;
	struct hashnode_s *node;
	//int i;

	for (n = 0; n < hashtbl->size; n++) {
		node = hashtbl->nodes[n];
		while (node) {
			(print_json) ? my_printf("%s=", node->key) : printf("%s=", node->key);
			(print_json) ? my_printf("%s\\n", (char *) node->data) : printf("%s\n", (char *) node->data);
			node = node->next;
		}
	}
	return OPH_TERM_SUCCESS;
}

//Set value for alias key (insert new variable if not present)
int oph_term_setalias(HASHTBL * hashtbl, const char *key, const char *value)
{
	if (!is_alias_ok(key)) {
		(print_json) ? my_fprintf(stderr,
					  "An alias does not have to start with \\\"oph_\\\",\\\"/\\\" or \\\".\\\", it does not have to be a number or equal to an Oph_Term local command.\\n") :
		    fprintf(stderr, "\e[1;31mAn alias does not have to start with \"oph_\",\"/\" or \".\", it does not have to be a number or equal to an Oph_Term local command.\e[0m\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}
	if (hashtbl_insert(hashtbl, key, (void *) value, strlen(value) + 1)) {
		(print_json) ? my_fprintf(stderr, "Error: setalias failed\\n") : fprintf(stderr, "\e[1;31mError: setalias failed\e[0m\n");
		return OPH_TERM_MEMORY_ERROR;
	}
	return OPH_TERM_SUCCESS;
}

//Clear alias key if present or do nothing
int oph_term_unsetalias(HASHTBL * hashtbl, const char *key)
{
	if (hashtbl_remove(hashtbl, key)) {
		(print_json) ? my_fprintf(stderr, "Warning: alias %s not found\\n", key) : fprintf(stderr, "\e[1;31mWarning: alias %s not found\e[0m\n", key);
	}
	return OPH_TERM_SUCCESS;
}

//Print value for alias key
int oph_term_getalias(HASHTBL * hashtbl, const char *key)
{
	char *value;
	value = (char *) hashtbl_get(hashtbl, key);
	if (!value) {
		(print_json) ? my_fprintf(stderr, "Warning: alias %s not found\\n", key) : fprintf(stderr, "\e[1;31mWarning: alias %s not found\e[0m\n", key);
	} else {
		(print_json) ? my_printf("%s\\n", value) : printf("%s\n", value);
	}
	return OPH_TERM_SUCCESS;
}

// Get the list of aliases
int oph_term_get_aliases(HASHTBL * hashtbl, char ***aliases, int *aliases_num)
{
	hash_size n;
	struct hashnode_s *node;
	int i = 0, j;

	*aliases_num = 0;
	*aliases = NULL;

	for (n = 0; n < hashtbl->size; n++) {
		node = hashtbl->nodes[n];
		while (node) {
			(*aliases_num)++;
			node = node->next;
		}
	}

	if ((*aliases_num) == 0)
		return OPH_TERM_SUCCESS;

	*aliases = (char **) calloc(*aliases_num, sizeof(char *));
	if (!(*aliases)) {
		*aliases_num = 0;
		return OPH_TERM_MEMORY_ERROR;
	}

	for (n = 0; n < hashtbl->size; n++) {
		node = hashtbl->nodes[n];
		while (node) {
			(*aliases)[i] = (char *) strdup(node->key);
			if (!((*aliases)[i])) {
				for (j = 0; j < i; j++)
					free((*aliases)[j]);
				free(*aliases);
				*aliases = NULL;
				*aliases_num = 0;
				return OPH_TERM_MEMORY_ERROR;
			}
			i++;
			node = node->next;
		}
	}

	return OPH_TERM_SUCCESS;
}

// Check if key does not start with "oph_" or "/" or "." and is not a number and is different from local commands
int is_alias_ok(const char *key)
{
	if (!key)
		return 0;
	if (strlen(key) == 0)
		return 0;
	if (key[0] == '/' || key[0] == '.')
		return 0;
	if (!strncmp(key, OPH_TERM_ENV_OPH_PREFIX, OPH_TERM_ENV_OPH_PREFIX_LEN))
		return 0;
	int i, is_number = 1;
	for (i = 0; i < (int) strlen(key); i++) {
		if (!isdigit(key[i])) {
			is_number = 0;
			break;
		}
	}
	if (is_number)
		return 0;
	for (i = 0; i < cmds_num; i++) {
		if (!strcmp(cmds[i], key))
			return 0;
	}
	return 1;
}

// Check if key is an alias
int is_alias(HASHTBL * hashtbl, const char *key)
{
	hash_size n;
	struct hashnode_s *node;
	//int i;

	for (n = 0; n < hashtbl->size; n++) {
		node = hashtbl->nodes[n];
		while (node) {
			if (!strcmp(node->key, key))
				return 1;
			node = node->next;
		}
	}
	return 0;
}

//IM related struct and functions TODO
#ifdef WITH_IM_SUPPORT
typedef struct {
	char *buffer;
	size_t len;
} curlbuf;

size_t _write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	size_t realsize = size * nmemb;
	curlbuf *mem = (curlbuf *) userdata;
	mem->buffer = realloc(mem->buffer, mem->len + realsize + 1);
	if (mem->buffer == NULL) {
		/* out of memory! */
		return 0;
	}
	memcpy(&(mem->buffer[mem->len]), ptr, realsize);
	mem->len += realsize;
	mem->buffer[mem->len] = 0;

	return realsize;
}

int oph_term_env_deploy(const char *auth_header, const char *infrastructure_url, const char *radl_filename, HASHTBL * hashtbl)
{
	if (!auth_header || !infrastructure_url || !radl_filename || !hashtbl) {
		(print_json) ? my_fprintf(stderr, "Null parameters\\n") : fprintf(stderr, "\e[1;31mNull parameters\e[0m\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	char *filebuf = NULL;
	long filebuf_len = 0;
	if (oph_term_read_file_with_len(radl_filename, &filebuf, &filebuf_len)) {
		(print_json) ? my_fprintf(stderr, "Error loading RADL file\\n") : fprintf(stderr, "\e[1;31mError loading RADL file\e[0m\n");
		if (filebuf) {
			free(filebuf);
			filebuf = NULL;
		}
		return OPH_TERM_MEMORY_ERROR;
	}

	CURLcode ret;
	struct curl_slist *slist = NULL;
	curlbuf mybuf;
	mybuf.buffer = malloc(1);	/* will be grown as needed by the realloc */
	mybuf.len = 0;		/* no data at this point */
	CURL *hnd = curl_easy_init();
	slist = curl_slist_append(slist, auth_header);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &mybuf);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, _write_callback);
	curl_easy_setopt(hnd, CURLOPT_INFILESIZE_LARGE, (curl_off_t) - 1);
	curl_easy_setopt(hnd, CURLOPT_URL, infrastructure_url);
	curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(hnd, CURLOPT_UNRESTRICTED_AUTH, 1L);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, filebuf);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t) filebuf_len - 1);
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50);
	curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(hnd, CURLOPT_CONNECTTIMEOUT, 10);
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist);
	ret = curl_easy_perform(hnd);
	curl_slist_free_all(slist);
	if (filebuf) {
		free(filebuf);
		filebuf = NULL;
	}

	long http_code = 0;
	curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &http_code);
	if (http_code != 200 || ret == CURLE_ABORTED_BY_CALLBACK) {
		curl_easy_cleanup(hnd);
		(print_json) ? my_fprintf(stderr, "Unable to do the POST request\\n") : fprintf(stderr, "\e[1;31mUnable to do the POST request\e[0m\n");
		if (mybuf.buffer)
			free(mybuf.buffer);
		return OPH_TERM_GENERIC_ERROR;
	}
	curl_easy_cleanup(hnd);

	char *suffix = strrchr(mybuf.buffer, '/');
	if (!suffix) {
		(print_json) ? my_fprintf(stderr, "Invalid response\\n") : fprintf(stderr, "\e[1;31mInvalid response\e[0m\n");
		if (mybuf.buffer)
			free(mybuf.buffer);
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	char buffer[OPH_IM_MAX_LEN];
	snprintf(buffer, OPH_IM_MAX_LEN, "%s%s", infrastructure_url, suffix);

	if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL, buffer)) {
		(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																				  "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																				  OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL,
																				  OPH_TERM_MEMORY_ERROR);
		if (mybuf.buffer)
			free(mybuf.buffer);
		return OPH_TERM_MEMORY_ERROR;
	}
	(print_json) ? my_printf("The new infrastructure URL is now \\\"%s\\\".\\n", buffer) : printf("The new infrastructure URL is now \"%s\".\n", buffer);
	if (mybuf.buffer)
		free(mybuf.buffer);

	return OPH_TERM_SUCCESS;
}

int oph_term_env_undeploy(const char *auth_header, const char *infrastructure_url, HASHTBL * hashtbl)
{
	if (!auth_header || !infrastructure_url || !hashtbl) {
		(print_json) ? my_fprintf(stderr, "Null parameters\\n") : fprintf(stderr, "\e[1;31mNull parameters\e[0m\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	CURLcode ret;
	struct curl_slist *slist = NULL;
	CURL *hnd = curl_easy_init();
	slist = curl_slist_append(slist, auth_header);
	curl_easy_setopt(hnd, CURLOPT_URL, infrastructure_url);
	curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(hnd, CURLOPT_UNRESTRICTED_AUTH, 1L);
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50);
	curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "DELETE");
	curl_easy_setopt(hnd, CURLOPT_CONNECTTIMEOUT, 10);
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist);
	ret = curl_easy_perform(hnd);
	curl_slist_free_all(slist);

	long http_code = 0;
	curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &http_code);
	if (http_code != 200 || ret == CURLE_ABORTED_BY_CALLBACK) {
		curl_easy_cleanup(hnd);
		(print_json) ? my_fprintf(stderr, "Unable to do the DELETE request\\n") : fprintf(stderr, "\e[1;31mUnable to do the DELETE request\e[0m\n");
		return OPH_TERM_GENERIC_ERROR;
	}
	curl_easy_cleanup(hnd);

	char buffer[OPH_IM_MAX_LEN];
	snprintf(buffer, OPH_IM_MAX_LEN, "%s", infrastructure_url);

	char *suffix = strrchr(buffer, '/');
	if (!suffix) {
		(print_json) ? my_fprintf(stderr, "Invalid URL\\n") : fprintf(stderr, "\e[1;31mInvalid URL\e[0m\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}
	suffix[0] = 0;

	if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL, buffer)) {
		(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																				  "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																				  OPH_TERM_ENV_OPH_INFRASTRUCTURE_URL,
																				  OPH_TERM_MEMORY_ERROR);
		return OPH_TERM_MEMORY_ERROR;
	}
	oph_term_unsetenv(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST);
	oph_term_unsetenv(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT);

	return OPH_TERM_SUCCESS;
}

int oph_term_env_deploy_status(const char *auth_header, const char *infrastructure_url)
{
	if (!auth_header || !infrastructure_url) {
		(print_json) ? my_fprintf(stderr, "Null parameters\\n") : fprintf(stderr, "\e[1;31mNull parameters\e[0m\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	CURLcode ret;
	struct curl_slist *slist = NULL;
	curlbuf mybuf;
	mybuf.buffer = malloc(1);	/* will be grown as needed by the realloc */
	mybuf.len = 0;		/* no data at this point */
	CURL *hnd = curl_easy_init();
	slist = curl_slist_append(slist, auth_header);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &mybuf);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, _write_callback);
	curl_easy_setopt(hnd, CURLOPT_URL, infrastructure_url);
	curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(hnd, CURLOPT_UNRESTRICTED_AUTH, 1L);
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50);
	curl_easy_setopt(hnd, CURLOPT_CONNECTTIMEOUT, 10);
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist);
	ret = curl_easy_perform(hnd);
	curl_slist_free_all(slist);

	long http_code = 0;
	curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &http_code);
	if (http_code != 200 || ret == CURLE_ABORTED_BY_CALLBACK) {
		curl_easy_cleanup(hnd);
		(print_json) ? my_fprintf(stderr, "Unable to do the GET request\\n") : fprintf(stderr, "\e[1;31mUnable to do the GET request\e[0m\n");
		if (mybuf.buffer)
			free(mybuf.buffer);
		return OPH_TERM_GENERIC_ERROR;
	}
	curl_easy_cleanup(hnd);

	if (strstr(mybuf.buffer, OPH_IM_STATUS_FINISHED)) {
		(print_json) ? my_printf("Installation and configuration FINISHED.\\n") : printf("Installation and configuration \e[1;32mFINISHED\e[0m.\n");
	} else if (strcasestr(mybuf.buffer, OPH_IM_STATUS_ERROR)) {
		(print_json) ? my_printf("Installation and configuration ERROR.\\n") : printf("Installation and configuration \e[1;31mERROR\e[0m.\n");
	} else if (strstr(mybuf.buffer, OPH_IM_STATUS_EMPTY)) {
		(print_json) ? my_printf("Installation and configuration EMPTY.\\n") : printf("Installation and configuration \e[1;31mEMPTY\e[0m.\n");
	} else {
		(print_json) ? my_printf("Installation and configuration RUNNING.\\n") : printf("Installation and configuration \e[1;36mRUNNING\e[0m.\n");
	}

	if (mybuf.buffer)
		free(mybuf.buffer);

	return OPH_TERM_SUCCESS;
}

int oph_term_env_get_server(const char *auth_header, const char *infrastructure_url, HASHTBL * hashtbl)
{
	if (!auth_header || !infrastructure_url || !hashtbl) {
		(print_json) ? my_fprintf(stderr, "Null parameters\\n") : fprintf(stderr, "\e[1;31mNull parameters\e[0m\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	char server_url[OPH_IM_MAX_LEN];
	snprintf(server_url, OPH_IM_MAX_LEN, "%s%s", infrastructure_url, OPH_IM_SERVER_SUFFIX);

	CURLcode ret;
	struct curl_slist *slist = NULL;
	curlbuf mybuf;
	mybuf.buffer = malloc(1);	/* will be grown as needed by the realloc */
	mybuf.len = 0;		/* no data at this point */
	CURL *hnd = curl_easy_init();
	slist = curl_slist_append(slist, auth_header);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &mybuf);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, _write_callback);
	curl_easy_setopt(hnd, CURLOPT_URL, server_url);
	curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(hnd, CURLOPT_UNRESTRICTED_AUTH, 1L);
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50);
	curl_easy_setopt(hnd, CURLOPT_CONNECTTIMEOUT, 10);
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist);
	ret = curl_easy_perform(hnd);
	curl_slist_free_all(slist);

	long http_code = 0;
	curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &http_code);
	if (http_code != 200 || ret == CURLE_ABORTED_BY_CALLBACK) {
		curl_easy_cleanup(hnd);
		(print_json) ? my_fprintf(stderr, "Unable to do the GET request\\n") : fprintf(stderr, "\e[1;31mUnable to do the GET request\e[0m\n");
		if (mybuf.buffer)
			free(mybuf.buffer);
		return OPH_TERM_GENERIC_ERROR;
	}
	curl_easy_cleanup(hnd);

	char *ptr = strstr(mybuf.buffer, OPH_IM_SERVER_INTERFACE);
	if (!ptr) {
		(print_json) ? my_fprintf(stderr, "Invalid response\\n") : fprintf(stderr, "\e[1;31mInvalid response\e[0m\n");
		if (mybuf.buffer)
			free(mybuf.buffer);
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	char *ptr2 = strchr(ptr + strlen(OPH_IM_SERVER_INTERFACE), '\'');
	if (!ptr2) {
		(print_json) ? my_fprintf(stderr, "Invalid response\\n") : fprintf(stderr, "\e[1;31mInvalid response\e[0m\n");
		if (mybuf.buffer)
			free(mybuf.buffer);
		return OPH_TERM_INVALID_PARAM_VALUE;
	}
	ptr2[0] = 0;

	if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_SERVER_HOST, ptr + strlen(OPH_IM_SERVER_INTERFACE))) {
		(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_SERVER_HOST, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			   "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																			   OPH_TERM_ENV_OPH_SERVER_HOST,
																			   OPH_TERM_MEMORY_ERROR);
		if (mybuf.buffer)
			free(mybuf.buffer);
		return OPH_TERM_MEMORY_ERROR;
	}

	(print_json) ? my_printf("The new server address is now \\\"%s\\\".\\n", ptr + strlen(OPH_IM_SERVER_INTERFACE)) : printf("The new server address is now \"%s\".\n",
																 ptr + strlen(OPH_IM_SERVER_INTERFACE));

	if (oph_term_setenv(hashtbl, OPH_TERM_ENV_OPH_SERVER_PORT, OPH_IM_SERVER_DEFAULT_PORT)) {
		(print_json) ? my_fprintf(stderr, "Could not set variable %s [CODE %d]\\n", OPH_TERM_ENV_OPH_SERVER_PORT, OPH_TERM_MEMORY_ERROR) : fprintf(stderr,
																			   "\e[1;31mCould not set variable %s [CODE %d]\e[0m\n",
																			   OPH_TERM_ENV_OPH_SERVER_PORT,
																			   OPH_TERM_MEMORY_ERROR);
		if (mybuf.buffer)
			free(mybuf.buffer);
		return OPH_TERM_MEMORY_ERROR;
	}

	if (mybuf.buffer)
		free(mybuf.buffer);

	return OPH_TERM_SUCCESS;
}

int oph_term_env_deploys_list(const char *auth_header, const char *infrastructure_url)
{
	if (!auth_header || !infrastructure_url) {
		(print_json) ? my_fprintf(stderr, "Null parameters\\n") : fprintf(stderr, "\e[1;31mNull parameters\e[0m\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	char infurl[OPH_IM_MAX_LEN];
	snprintf(infurl, OPH_IM_MAX_LEN, "%s", infrastructure_url);
	char *ptr = strstr(infurl, OPH_IM_INFRASTRUCTURES_NAME);
	if (!ptr) {
		(print_json) ? my_fprintf(stderr, "Invalid URL\\n") : fprintf(stderr, "\e[1;31mInvalid URL\e[0m\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}
	ptr[strlen(OPH_IM_INFRASTRUCTURES_NAME)] = 0;

	CURLcode ret;
	struct curl_slist *slist = NULL;
	curlbuf mybuf;
	mybuf.buffer = malloc(1);	/* will be grown as needed by the realloc */
	mybuf.len = 0;		/* no data at this point */
	CURL *hnd = curl_easy_init();
	slist = curl_slist_append(slist, auth_header);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &mybuf);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, _write_callback);
	curl_easy_setopt(hnd, CURLOPT_URL, infurl);
	curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(hnd, CURLOPT_UNRESTRICTED_AUTH, 1L);
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50);
	curl_easy_setopt(hnd, CURLOPT_CONNECTTIMEOUT, 10);
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist);
	ret = curl_easy_perform(hnd);
	curl_slist_free_all(slist);

	long http_code = 0;
	curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &http_code);
	if (http_code != 200 || ret == CURLE_ABORTED_BY_CALLBACK) {
		curl_easy_cleanup(hnd);
		(print_json) ? my_fprintf(stderr, "Unable to do the GET request\\n") : fprintf(stderr, "\e[1;31mUnable to do the GET request\e[0m\n");
		if (mybuf.buffer)
			free(mybuf.buffer);
		return OPH_TERM_GENERIC_ERROR;
	}
	curl_easy_cleanup(hnd);

	char *save_ptr = NULL;
	char *line = strtok_r(mybuf.buffer, "\n", &save_ptr);
	while (line) {
		char *ptr2 = strstr(line, OPH_IM_INFRASTRUCTURES_NAME);
		if (ptr2) {
			(print_json) ? my_printf("%s%s\\n", infurl, ptr2 + strlen(OPH_IM_INFRASTRUCTURES_NAME)) : printf("%s%s\n", infurl, ptr2 + strlen(OPH_IM_INFRASTRUCTURES_NAME));
		}
		line = strtok_r(NULL, "\n", &save_ptr);
	}

	if (mybuf.buffer)
		free(mybuf.buffer);

	return OPH_TERM_SUCCESS;
}

int oph_term_env_deploy_vms_list(const char *auth_header, const char *infrastructure_url)
{
	if (!auth_header || !infrastructure_url) {
		(print_json) ? my_fprintf(stderr, "Null parameters\\n") : fprintf(stderr, "\e[1;31mNull parameters\e[0m\n");
		return OPH_TERM_INVALID_PARAM_VALUE;
	}

	CURLcode ret;
	struct curl_slist *slist = NULL;
	curlbuf mybuf;
	mybuf.buffer = malloc(1);	/* will be grown as needed by the realloc */
	mybuf.len = 0;		/* no data at this point */
	CURL *hnd = curl_easy_init();
	slist = curl_slist_append(slist, auth_header);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &mybuf);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, _write_callback);
	curl_easy_setopt(hnd, CURLOPT_URL, infrastructure_url);
	curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(hnd, CURLOPT_UNRESTRICTED_AUTH, 1L);
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50);
	curl_easy_setopt(hnd, CURLOPT_CONNECTTIMEOUT, 10);
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist);
	ret = curl_easy_perform(hnd);
	curl_slist_free_all(slist);

	long http_code = 0;
	curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &http_code);
	if (http_code != 200 || ret == CURLE_ABORTED_BY_CALLBACK) {
		curl_easy_cleanup(hnd);
		(print_json) ? my_fprintf(stderr, "Unable to do the GET request\\n") : fprintf(stderr, "\e[1;31mUnable to do the GET request\e[0m\n");
		if (mybuf.buffer)
			free(mybuf.buffer);
		return OPH_TERM_GENERIC_ERROR;
	}
	curl_easy_cleanup(hnd);

	char *ptr = strchr(mybuf.buffer, ']');
	if (!ptr) {
		(print_json) ? my_fprintf(stderr, "Invalid response\\n") : fprintf(stderr, "\e[1;31mInvalid response\e[0m\n");
		if (mybuf.buffer)
			free(mybuf.buffer);
		return OPH_TERM_INVALID_PARAM_VALUE;
	}
	ptr[0] = 0;

	size_t vm_num = 0;
	char *ptr2 = strstr(mybuf.buffer, OPH_IM_VMS_TOKEN);
	while (ptr2) {
		vm_num++;
		ptr2 = strstr(ptr2 + strlen(OPH_IM_VMS_TOKEN), OPH_IM_VMS_TOKEN);
	}

	int i;
	for (i = 0; i < vm_num; i++) {
		(print_json) ? my_printf("%s%s%d\\n", infrastructure_url, OPH_IM_VMS_TOKEN, i) : printf("%s%s%d\n", infrastructure_url, OPH_IM_VMS_TOKEN, i);
	}

	if (mybuf.buffer)
		free(mybuf.buffer);

	return OPH_TERM_SUCCESS;
}
#endif
