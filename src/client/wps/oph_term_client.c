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

#define _GNU_SOURCE

#include "oph_term_client.h"
#include "oph_workflow_define.h"
#include "oph_term_defs.h"

#include <unistd.h>		/* defines _POSIX_THREADS if pthreads are available */
#if defined(_POSIX_THREADS) || defined(_SC_THREADS)
#include <pthread.h>
#endif
#include <signal.h>		/* defines SIGPIPE */

#include <inttypes.h>
#include <string.h>
#include <curl/curl.h>
#include <stdio.h>

#define OPH_DEFAULT_NLOOPS 1
#define OPH_DEFAULT_QUERY "OPH_NULL"
#define OPH_WPS_TITLE "<title>"
#define OPH_WPS_TITLE_END "</title>"

#define UNUSED(x) {(void)(x);}

extern char print_debug_data;

extern pthread_mutex_t global_flag;
extern size_t max_size;
extern int last_workflow_id;

void sigpipe_handle(int);

struct oph__ophResponse {
	char *jobid;
	char *response;
	long error;
	char *xml;
	char *buffer;
	size_t size;
} response_global;

char server_global[OPH_MAX_STRING_SIZE];
int wps_call_oph__ophExecuteMain_return;
char username_global[OPH_MAX_STRING_SIZE];
char password_global[OPH_MAX_STRING_SIZE];
int store_result_global;

#ifdef AUTH_BEARER

#define OPH_AUTH_BEARER "Authorization: Bearer %s"

#define OPH_WPS_XML_REQUEST "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\
<wps:Execute service=\"WPS\" version=\"1.0.0\" xmlns:wps=\"http://www.opengis.net/wps/1.0.0\" xmlns:ows=\"http://www.opengis.net/ows/1.1\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.opengis.net/wps/1.0.0\
../wpsExecute_request.xsd\">\
	<ows:Identifier>ophexecutemain</ows:Identifier>\
	<wps:DataInputs>\
		<wps:Input>\
			<ows:Identifier>request</ows:Identifier>\
			<ows:Title>JSON Request</ows:Title>\
			<wps:Data>\
				<wps:ComplexData mimeType=\"application/json\" encoding=\"base64\">%s</wps:ComplexData>\
			</wps:Data>\
		</wps:Input>\
	</wps:DataInputs>\
	<wps:ResponseForm>\
		<wps:ResponseDocument lineage=\"true\" %s>\
			<wps:Output>\
				<ows:Identifier>jobid</ows:Identifier>\
				<ows:Title>Ophidia JobID</ows:Title>\
			</wps:Output>\
			<wps:Output asReference=\"false\">\
				<ows:Identifier>response</ows:Identifier>\
				<ows:Title>JSON Response</ows:Title>\
			</wps:Output>\
			<wps:Output>\
				<ows:Identifier>return</ows:Identifier>\
				<ows:Title>Return code</ows:Title>\
			</wps:Output>\
		</wps:ResponseDocument>\
	</wps:ResponseForm>\
</wps:Execute>"

#else

#define OPH_WPS_XML_REQUEST "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\
<wps:Execute service=\"WPS\" version=\"1.0.0\" xmlns:wps=\"http://www.opengis.net/wps/1.0.0\" xmlns:ows=\"http://www.opengis.net/ows/1.1\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.opengis.net/wps/1.0.0\
../wpsExecute_request.xsd\">\
	<ows:Identifier>ophexecutemain</ows:Identifier>\
	<wps:DataInputs>\
		<wps:Input>\
			<ows:Identifier>request</ows:Identifier>\
			<ows:Title>JSON Request</ows:Title>\
			<wps:Data>\
				<wps:ComplexData mimeType=\"application/json\" encoding=\"base64\">%s</wps:ComplexData>\
			</wps:Data>\
		</wps:Input>\
		<wps:Input>\
			<ows:Identifier>userid</ows:Identifier>\
			<ows:Title>Username</ows:Title>\
			<wps:Data>\
				<wps:LiteralData>%s</wps:LiteralData>\
			</wps:Data>\
		</wps:Input>\
		<wps:Input>\
			<ows:Identifier>passwd</ows:Identifier>\
			<ows:Title>Password</ows:Title>\
			<wps:Data>\
				<wps:LiteralData>%s</wps:LiteralData>\
			</wps:Data>\
		</wps:Input>\
	</wps:DataInputs>\
	<wps:ResponseForm>\
		<wps:ResponseDocument lineage=\"true\" %s>\
			<wps:Output>\
				<ows:Identifier>jobid</ows:Identifier>\
				<ows:Title>Ophidia JobID</ows:Title>\
			</wps:Output>\
			<wps:Output asReference=\"false\">\
				<ows:Identifier>response</ows:Identifier>\
				<ows:Title>JSON Response</ows:Title>\
			</wps:Output>\
			<wps:Output>\
				<ows:Identifier>return</ows:Identifier>\
				<ows:Title>Return code</ows:Title>\
			</wps:Output>\
		</wps:ResponseDocument>\
	</wps:ResponseForm>\
</wps:Execute>"

#endif

int base64encode(const void *data_buf, size_t dataLength, char *result, size_t resultSize)
{
	const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	const uint8_t *data = (const uint8_t *) data_buf;
	size_t resultIndex = 0;
	size_t x;
	uint32_t n = 0;
	int padCount = dataLength % 3;
	uint8_t n0, n1, n2, n3;

	/* increment over the length of the string, three characters at a time */
	for (x = 0; x < dataLength; x += 3) {
		/* these three 8-bit (ASCII) characters become one 24-bit number */
		n = ((uint32_t) data[x]) << 16;	//parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0

		if ((x + 1) < dataLength)
			n += ((uint32_t) data[x + 1]) << 8;	//parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0

		if ((x + 2) < dataLength)
			n += data[x + 2];

		/* this 24-bit number gets separated into four 6-bit numbers */
		n0 = (uint8_t) (n >> 18) & 63;
		n1 = (uint8_t) (n >> 12) & 63;
		n2 = (uint8_t) (n >> 6) & 63;
		n3 = (uint8_t) n & 63;

		/*
		 * if we have one byte available, then its encoding is spread
		 * out over two characters
		 */
		if (resultIndex >= resultSize)
			return 1;	/* indicate failure: buffer too small */
		result[resultIndex++] = base64chars[n0];
		if (resultIndex >= resultSize)
			return 1;	/* indicate failure: buffer too small */
		result[resultIndex++] = base64chars[n1];

		/*
		 * if we have only two bytes available, then their encoding is
		 * spread out over three chars
		 */
		if ((x + 1) < dataLength) {
			if (resultIndex >= resultSize)
				return 1;	/* indicate failure: buffer too small */
			result[resultIndex++] = base64chars[n2];
		}

		/*
		 * if we have all three bytes available, then their encoding is spread
		 * out over four characters
		 */
		if ((x + 2) < dataLength) {
			if (resultIndex >= resultSize)
				return 1;	/* indicate failure: buffer too small */
			result[resultIndex++] = base64chars[n3];
		}
	}

	/*
	 * create and add padding that is required if we did not have a multiple of 3
	 * number of characters available
	 */
	if (padCount > 0) {
		for (; padCount < 3; padCount++) {
			if (resultIndex >= resultSize)
				return 1;	/* indicate failure: buffer too small */
			result[resultIndex++] = '=';
		}
	}
	if (resultIndex >= resultSize)
		return 1;	/* indicate failure: buffer too small */
	result[resultIndex] = 0;
	return 0;		/* indicate success */
}

#define WHITESPACE 64
#define EQUALS     65
#define INVALID    66

static const unsigned char d[] = {
	66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 64, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
	66, 66, 66, 66, 66, 66, 66, 64, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 62, 66, 66, 66, 63, 52, 53,
	54, 55, 56, 57, 58, 59, 60, 61, 66, 66, 66, 65, 66, 66, 66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 66, 66, 66, 66, 66, 66, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 66, 66,
	66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
	66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
	66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
	66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
	66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
	66, 66, 66, 66, 66, 66
};

int base64decode(const char *in, size_t inLen, char *out, size_t * outLen)
{
	const char *end = in + inLen;
	char iter = 0;
	size_t buf = 0, len = 0;

	while (in < end) {
		unsigned char c = d[(int) (*in++)];

		switch (c) {
			case WHITESPACE:
				continue;	/* skip whitespace */
			case INVALID:
				return 1;	/* invalid input, return error */
			case EQUALS:	/* pad character, end of data */
				in = end;
				continue;
			default:
				buf = buf << 6 | c;
				iter++;	// increment the number of iteration
				/* If the buffer is full, split it into bytes */
				if (iter == 4) {
					if ((len += 3) > *outLen)
						return 2;	/* buffer overflow */
					(*out) = (buf >> 16) & 255;
					out++;
					(*out) = (buf >> 8) & 255;
					out++;
					(*out) = buf & 255;
					out++;
					buf = 0;
					iter = 0;

				}
		}
	}

	if (iter == 3) {
		if ((len += 2) > *outLen)
			return 3;	/* buffer overflow */
		(*out) = (buf >> 10) & 255;
		out++;
		(*out) = (buf >> 2) & 255;
		out++;
	} else if (iter == 2) {
		if (++len > *outLen)
			return 4;	/* buffer overflow */
		(*out) = (buf >> 4) & 255;
		out++;
	}

	if (++len > *outLen)
		return 5;	/* buffer overflow */
	(*out) = 0;
	out++;

	*outLen = len;		/* modify to reflect the actual output size */

	return 0;
}

size_t _write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	UNUSED(userdata);

	size_t realsize = size * nmemb;
	struct oph__ophResponse *mem = &response_global;
	mem->buffer = (char *) realloc(mem->buffer, mem->size + realsize + 1);
	if (mem->buffer == NULL)
		return 0;
	memcpy(mem->buffer + mem->size, ptr, realsize);
	mem->size += realsize;
	mem->buffer[mem->size] = 0;
	return realsize;
}

int process_response()
{
	struct oph__ophResponse *mem = &response_global;

	if (!mem->size || !mem->buffer)
		return 1;

	if (mem->response)
		free(mem->response);
	mem->response = NULL;
	if (mem->jobid)
		free(mem->jobid);
	mem->jobid = NULL;
	mem->error = OPH_SERVER_ERROR;
	if (mem->xml)
		free(mem->xml);
	mem->xml = NULL;

	if (print_debug_data)
		(print_json) ? my_fprintf(stderr, "XML Response:\\n%s\\n\\n", mem->buffer) : fprintf(stderr, "\e[2mXML Response:\n%s\e[0m\n\n", mem->buffer);

	xmlParserCtxtPtr ctxt;
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlXPathContextPtr xpathCtx;
	xmlXPathObjectPtr xpathObj;

	/* Try for HTML pages */
	char *title = strstr(mem->buffer, OPH_WPS_TITLE);
	if (title) {
		char *etitle = strstr(title, OPH_WPS_TITLE_END);
		if (etitle) {
			*etitle = 0;
			(print_json) ? my_fprintf(stderr, "Error: %s\\n", title + strlen(OPH_WPS_TITLE)) : fprintf(stderr, "\e[1;31mError: %s\e[0m\n", title + strlen(OPH_WPS_TITLE));
			return 1;
		}
	}

	/* create a parser context */
	ctxt = xmlNewParserCtxt();
	if (ctxt == NULL) {
		(print_json) ? my_fprintf(stderr, "Failed to allocate parser context\\n") : fprintf(stderr, "\e[1;31mFailed to allocate parser context\e[0m\n");
		return 1;
	}

	doc = xmlCtxtReadMemory(ctxt, mem->buffer, mem->size, NULL, NULL, 0);

	/* Create xpath evaluation context */
	xpathCtx = xmlXPathNewContext(doc);
	if (xpathCtx == NULL) {
		(print_json) ? my_fprintf(stderr, "Error: unable to create new XPath context\\n") : fprintf(stderr, "\e[1;31mError: unable to create new XPath context\e[0m\n");
		xmlFreeDoc(doc);
		xmlFreeParserCtxt(ctxt);
		return 1;
	}

	if (xmlXPathRegisterNs(xpathCtx, (const xmlChar *) "wps", (const xmlChar *) "http://www.opengis.net/wps/1.0.0")) {
		(print_json) ? my_fprintf(stderr, "Error: unable to add namespace\\n") : fprintf(stderr, "\e[1;31mError: unable to add namespace\e[0m\n");
		xmlFreeDoc(doc);
		xmlFreeParserCtxt(ctxt);
		return 1;
	}
	if (xmlXPathRegisterNs(xpathCtx, (const xmlChar *) "ows", (const xmlChar *) "http://www.opengis.net/ows/1.1")) {
		(print_json) ? my_fprintf(stderr, "Error: unable to add namespace\\n") : fprintf(stderr, "\e[1;31mError: unable to add namespace\e[0m\n");
		xmlFreeDoc(doc);
		xmlFreeParserCtxt(ctxt);
		return 1;
	}

	xmlChar *content = NULL;

	// Check for errors
	xpathObj = xmlXPathEvalExpression((const xmlChar *) "/html/head/title", xpathCtx);
	if (!xpathObj) {
		(print_json) ? my_fprintf(stderr, "Error: unable to evaluate xpath expression\\n") : fprintf(stderr, "\e[1;31mError: unable to evaluate xpath expression\e[0m\n");
		xmlXPathFreeContext(xpathCtx);
		xmlFreeDoc(doc);
		xmlFreeParserCtxt(ctxt);
		return 1;
	}
	if (xpathObj->nodesetval && xpathObj->nodesetval->nodeNr && (node = xpathObj->nodesetval->nodeTab[0]) && (content = xmlNodeGetContent(node))) {
		(print_json) ? my_fprintf(stderr, "Error: %s\\n", content) : fprintf(stderr, "\e[1;31mError: %s\e[0m\n", content);
		xmlFree(content);
		xmlXPathFreeObject(xpathObj);
		xmlXPathFreeContext(xpathCtx);
		xmlFreeDoc(doc);
		xmlFreeParserCtxt(ctxt);
		return 1;
	}
	xmlXPathFreeObject(xpathObj);

	// Check for exceptions
	char processFailed = 0;
	xpathObj = xmlXPathEvalExpression((const xmlChar *) "/wps:ExecuteResponse/wps:Status/wps:ProcessFailed", xpathCtx);
	if (!xpathObj) {
		(print_json) ? my_fprintf(stderr, "Error: unable to evaluate xpath expression\\n") : fprintf(stderr, "\e[1;31mError: unable to evaluate xpath expression\e[0m\n");
		xmlXPathFreeContext(xpathCtx);
		xmlFreeDoc(doc);
		xmlFreeParserCtxt(ctxt);
		return 1;
	}
	if (xpathObj->nodesetval && xpathObj->nodesetval->nodeNr)
		processFailed = 1;
	xmlXPathFreeObject(xpathObj);

	if (processFailed) {
		xpathObj = xmlXPathEvalExpression((const xmlChar *) "/wps:ExecuteResponse/wps:Status/wps:ProcessFailed/wps:ExceptionReport/ows:Exception/ows:ExceptionText/text()", xpathCtx);
		if (!xpathObj || !xpathObj->nodesetval || !xpathObj->nodesetval->nodeNr) {
			(print_json) ? my_fprintf(stderr, "Error: unable to evaluate xpath expression\\n") : fprintf(stderr, "\e[1;31mError: unable to evaluate xpath expression\e[0m\n");
			xmlXPathFreeContext(xpathCtx);
			xmlFreeDoc(doc);
			xmlFreeParserCtxt(ctxt);
			return 1;
		}

		node = xpathObj->nodesetval->nodeTab[0];
		if (node) {
			content = xmlNodeGetContent(node);
			if (content) {
				const char *_content = (const char *) content;
				size_t i, j, len = strlen(_content);
				char tmp[1 + len];
				for (i = j = 0; i < len; i++)
					if (_content[i] != '\\')
						tmp[j++] = ((const char *) content)[i];
				tmp[j] = 0;
				(print_json) ? my_fprintf(stderr, "Error: %s\\n", tmp) : fprintf(stderr, "\e[1;31mError: %s\e[0m\n", tmp);
				xmlFree(content);
			}
		}
		xmlXPathFreeObject(xpathObj);
	} else if (store_result_global)	// Async mode
	{
		xpathObj = xmlXPathEvalExpression((const xmlChar *) "/wps:ExecuteResponse/wps:Status/wps:ProcessAccepted", xpathCtx);
		if (!xpathObj) {
			(print_json) ? my_fprintf(stderr, "Error: unable to evaluate xpath expression\\n") : fprintf(stderr, "\e[1;31mError: unable to evaluate xpath expression\e[0m\n");
			xmlXPathFreeContext(xpathCtx);
			xmlFreeDoc(doc);
			xmlFreeParserCtxt(ctxt);
			return 1;
		}
		if (xpathObj->nodesetval)
			mem->error = OPH_SERVER_OK;
		xmlXPathFreeObject(xpathObj);

		xpathObj = xmlXPathEvalExpression((const xmlChar *) "/wps:ExecuteResponse", xpathCtx);
		if (!xpathObj || !xpathObj->nodesetval || !xpathObj->nodesetval->nodeNr) {
			(print_json) ? my_fprintf(stderr, "Error: unable to evaluate xpath expression\\n") : fprintf(stderr, "\e[1;31mError: unable to evaluate xpath expression\e[0m\n");
			xmlXPathFreeContext(xpathCtx);
			xmlFreeDoc(doc);
			xmlFreeParserCtxt(ctxt);
			return 1;
		}
		node = xpathObj->nodesetval->nodeTab[0];
		if (node) {
			xmlAttrPtr attr = xmlHasProp(node, (const xmlChar *) "statusLocation");
			if (attr && attr->children)
				mem->xml = strdup((const char *) attr->children->content);
		}
		xmlXPathFreeObject(xpathObj);
	} else			// Sync mode
	{
		xpathObj =
		    xmlXPathEvalExpression((const xmlChar *) "/wps:ExecuteResponse/wps:ProcessOutputs/wps:Output[ows:Identifier/text()='return_code']/wps:Data/wps:LiteralData/text()", xpathCtx);
		if (!xpathObj || !xpathObj->nodesetval || !xpathObj->nodesetval->nodeNr) {
			(print_json) ? my_fprintf(stderr, "Error: unable to evaluate xpath expression\\n") : fprintf(stderr, "\e[1;31mError: unable to evaluate xpath expression\e[0m\n");
			xmlXPathFreeContext(xpathCtx);
			xmlFreeDoc(doc);
			xmlFreeParserCtxt(ctxt);
			return 1;
		}

		node = xpathObj->nodesetval->nodeTab[0];
		if (node) {
			content = xmlNodeGetContent(node);
			if (content) {
				if (strlen((const char *) content))
					mem->error = (int) strtol((const char *) content, NULL, 10);
				xmlFree(content);
			}
		}
		xmlXPathFreeObject(xpathObj);

		if (!mem->error) {
			xpathObj =
			    xmlXPathEvalExpression((const xmlChar *) "/wps:ExecuteResponse/wps:ProcessOutputs/wps:Output[ows:Identifier/text()='jobid']/wps:Data/wps:LiteralData/text()", xpathCtx);
			if (!xpathObj) {
				(print_json) ? my_fprintf(stderr, "Error: unable to evaluate xpath expression\\n") : fprintf(stderr, "\e[1;31mError: unable to evaluate xpath expression\e[0m\n");
				xmlXPathFreeContext(xpathCtx);
				xmlFreeDoc(doc);
				xmlFreeParserCtxt(ctxt);
				return 1;
			}
			if (xpathObj->nodesetval && xpathObj->nodesetval->nodeNr) {
				node = xpathObj->nodesetval->nodeTab[0];
				if (node) {
					content = xmlNodeGetContent(node);
					if (content) {
						if (strlen((const char *) content))
							mem->jobid = strdup((const char *) content);
						xmlFree(content);
					}
				}
			}
			xmlXPathFreeObject(xpathObj);

			xpathObj =
			    xmlXPathEvalExpression((const xmlChar *) "/wps:ExecuteResponse/wps:ProcessOutputs/wps:Output[ows:Identifier/text()='response']/wps:Data/wps:ComplexData/text()", xpathCtx);
			if (!xpathObj || !xpathObj->nodesetval || !xpathObj->nodesetval->nodeNr) {
				(print_json) ? my_fprintf(stderr, "Error: unable to evaluate xpath expression\\n") : fprintf(stderr, "\e[1;31mError: unable to evaluate xpath expression\e[0m\n");
				xmlXPathFreeContext(xpathCtx);
				xmlFreeDoc(doc);
				xmlFreeParserCtxt(ctxt);
				return 1;
			}
			node = xpathObj->nodesetval->nodeTab[0];
			if (node) {
				content = xmlNodeGetContent(node);
				if (content) {
					if (strlen((const char *) content))
						mem->response = strdup((const char *) content);
					xmlFree(content);
				}
			}
			xmlXPathFreeObject(xpathObj);
		}
	}

	/* Cleanup of XPath context */
	xmlXPathFreeContext(xpathCtx);
	/* free up the resulting document */
	xmlFreeDoc(doc);
	/* free up the parser context */
	xmlFreeParserCtxt(ctxt);

	return 0;
}

int wps_call_oph__ophExecuteMain(char *server_global, char *query, char *username, char *password, int store_result)
{
	if (!server_global || !query)
		return 1;

	if (max_size <= 0)
		return 2;

	// Build XML Request
	char *wpsRequest = (char *) malloc(max_size);
	if (!wpsRequest)
		return 2;

#ifdef AUTH_BEARER
	snprintf(wpsRequest, max_size, OPH_WPS_XML_REQUEST, query, store_result ? "storeExecuteResponse=\"true\" status=\"true\"" : "storeExecuteResponse=\"false\"");
	char auth_header[OPH_MAX_STRING_SIZE];
	snprintf(auth_header, OPH_MAX_STRING_SIZE, OPH_AUTH_BEARER, password);
	if (print_debug_data)
		(print_json) ? my_fprintf(stderr, "\\nHTTP Header\\n%s\\n", auth_header) : fprintf(stderr, "\e[2m\nHTTP Header:\n%s\e[0m\n", auth_header);
#else
	snprintf(wpsRequest, max_size, OPH_WPS_XML_REQUEST, query, username, password, store_result ? "storeExecuteResponse=\"true\" status=\"true\"" : "storeExecuteResponse=\"false\"");
#endif

	if (print_debug_data)
		(print_json) ? my_fprintf(stderr, "\\nSend WPS Request to %s\\n", server_global) : fprintf(stderr, "\e[2m\nSend WPS Request to %s\e[0m\n", server_global);

	if (print_debug_data)
		(print_json) ? my_fprintf(stderr, "\\nXML Request:\\n%s\\n\\n", wpsRequest) : fprintf(stderr, "\e[2m\nXML Request:\n%s\e[0m\n\n", wpsRequest);

	// Send the request 
	CURLcode ret;
	CURL *hnd = curl_easy_init();
	curl_easy_setopt(hnd, CURLOPT_INFILESIZE_LARGE, (curl_off_t) - 1);
	curl_easy_setopt(hnd, CURLOPT_URL, server_global);
	curl_easy_setopt(hnd, CURLOPT_PROXY, NULL);
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(hnd, CURLOPT_HEADER, 0);
	curl_easy_setopt(hnd, CURLOPT_FAILONERROR, 0);
	curl_easy_setopt(hnd, CURLOPT_UPLOAD, 0);
	curl_easy_setopt(hnd, CURLOPT_DIRLISTONLY, 0);
	curl_easy_setopt(hnd, CURLOPT_APPEND, 0);
	curl_easy_setopt(hnd, CURLOPT_NETRC, 0);
	curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(hnd, CURLOPT_UNRESTRICTED_AUTH, 0);
	curl_easy_setopt(hnd, CURLOPT_TRANSFERTEXT, 0);
	curl_easy_setopt(hnd, CURLOPT_USERPWD, NULL);
	curl_easy_setopt(hnd, CURLOPT_PROXYUSERPWD, NULL);
	curl_easy_setopt(hnd, CURLOPT_NOPROXY, NULL);
	curl_easy_setopt(hnd, CURLOPT_RANGE, NULL);
	curl_easy_setopt(hnd, CURLOPT_TIMEOUT, 0);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, wpsRequest);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t) - 1);
	curl_easy_setopt(hnd, CURLOPT_REFERER, NULL);
	curl_easy_setopt(hnd, CURLOPT_AUTOREFERER, 0);
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.19.7 (x86_64-redhat-linux-gnu) libcurl/7.19.7 NSS/3.16.2.3 Basic ECC zlib/1.2.3 libidn/1.18 libssh2/1.4.2");
	curl_easy_setopt(hnd, CURLOPT_FTPPORT, NULL);
	curl_easy_setopt(hnd, CURLOPT_LOW_SPEED_LIMIT, 0);
	curl_easy_setopt(hnd, CURLOPT_LOW_SPEED_TIME, 0);
	curl_easy_setopt(hnd, CURLOPT_MAX_SEND_SPEED_LARGE, (curl_off_t) 0);
	curl_easy_setopt(hnd, CURLOPT_MAX_RECV_SPEED_LARGE, (curl_off_t) 0);
	curl_easy_setopt(hnd, CURLOPT_RESUME_FROM_LARGE, (curl_off_t) 0);
	curl_easy_setopt(hnd, CURLOPT_COOKIE, NULL);
	curl_easy_setopt(hnd, CURLOPT_SSLCERT, NULL);
	curl_easy_setopt(hnd, CURLOPT_SSLCERTTYPE, NULL);
	curl_easy_setopt(hnd, CURLOPT_SSLKEY, NULL);
	curl_easy_setopt(hnd, CURLOPT_SSLKEYTYPE, NULL);
	curl_easy_setopt(hnd, CURLOPT_KEYPASSWD, NULL);
	curl_easy_setopt(hnd, CURLOPT_SSH_PRIVATE_KEYFILE, NULL);
	curl_easy_setopt(hnd, CURLOPT_SSH_PUBLIC_KEYFILE, NULL);
	curl_easy_setopt(hnd, CURLOPT_SSH_HOST_PUBLIC_KEY_MD5, NULL);
	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 2);
	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 1);
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50);
	curl_easy_setopt(hnd, CURLOPT_CRLF, 0);
	curl_easy_setopt(hnd, CURLOPT_QUOTE, NULL);
	curl_easy_setopt(hnd, CURLOPT_POSTQUOTE, NULL);
	curl_easy_setopt(hnd, CURLOPT_PREQUOTE, NULL);
	curl_easy_setopt(hnd, CURLOPT_WRITEHEADER, NULL);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, NULL);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, _write_callback);
	curl_easy_setopt(hnd, CURLOPT_COOKIEFILE, NULL);
	curl_easy_setopt(hnd, CURLOPT_COOKIESESSION, 0);
	curl_easy_setopt(hnd, CURLOPT_SSLVERSION, 0);
	curl_easy_setopt(hnd, CURLOPT_TIMECONDITION, 0);
	curl_easy_setopt(hnd, CURLOPT_TIMEVALUE, 0);
	curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(hnd, CURLOPT_HTTPPROXYTUNNEL, 0);
	curl_easy_setopt(hnd, CURLOPT_INTERFACE, NULL);
	curl_easy_setopt(hnd, CURLOPT_KRBLEVEL, NULL);
	curl_easy_setopt(hnd, CURLOPT_TELNETOPTIONS, NULL);
	curl_easy_setopt(hnd, CURLOPT_RANDOM_FILE, NULL);
	curl_easy_setopt(hnd, CURLOPT_EGDSOCKET, NULL);
	curl_easy_setopt(hnd, CURLOPT_CONNECTTIMEOUT, 0);
	curl_easy_setopt(hnd, CURLOPT_ENCODING, NULL);
	curl_easy_setopt(hnd, CURLOPT_FTP_CREATE_MISSING_DIRS, 0);
	curl_easy_setopt(hnd, CURLOPT_IPRESOLVE, 0);
	curl_easy_setopt(hnd, CURLOPT_FTP_ACCOUNT, NULL);
	curl_easy_setopt(hnd, CURLOPT_IGNORE_CONTENT_LENGTH, 0);
	curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 0);
	curl_easy_setopt(hnd, CURLOPT_FTP_FILEMETHOD, 0);
	curl_easy_setopt(hnd, CURLOPT_FTP_ALTERNATIVE_TO_USER, NULL);
	curl_easy_setopt(hnd, CURLOPT_SSL_SESSIONID_CACHE, 1);
	curl_easy_setopt(hnd, CURLOPT_POSTREDIR, 0);

#ifdef AUTH_BEARER
	struct curl_slist *slist = curl_slist_append(NULL, auth_header);
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist);
#endif

	ret = curl_easy_perform(hnd);

#ifdef AUTH_BEARER
	curl_slist_free_all(slist);
#endif

	curl_easy_cleanup(hnd);
	free(wpsRequest);

	if (ret)
		return ret;
	return process_response();
}

void *wpsthread(void *query)
{
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	wps_call_oph__ophExecuteMain_return = wps_call_oph__ophExecuteMain(server_global, (char *) query, username_global, password_global, store_result_global);
	free(query);
	return NULL;
}

void oph_execute(char *query, char **newsession, int *return_value, char **out_response, char **out_response_for_viewer, int workflow_wrap, char *username, char *password, HASHTBL * hashtbl,
		 char *cmd_line)
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

	store_result_global = 0;
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

			if (!(value = strstr(query, "\"exec_mode\""))) {
				n += snprintf(fixed_query + n, max_size - n, "%s%s%s", WRAPPING_WORKFLOW3, "sync", WRAPPING_WORKFLOW3_1);
				if ((value = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_EXEC_MODE)) && strstr(value, "async"))
					store_result_global = 1;
			} else if ((value = strstr(value, "\"async\""))) {
				*value = ' ';
				*(value + 1) = '\"';
				store_result_global = 1;
			}

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

	// Encoding
	char *result = (char *) malloc(max_size);
	if (!result) {
		(print_json) ? my_fprintf(stderr, "Encoding error\\n") : fprintf(stderr, "\e[1;31mEncoding error\e[0m\n");
		*return_value = OPH_TERM_MEMORY_ERROR;
		return;
	}

	if (base64encode(query, strlen(query), result, max_size)) {
		(print_json) ? my_fprintf(stderr, "Encoding error\\n") : fprintf(stderr, "\e[1;31mEncoding error\e[0m\n");
		free(result);
		*return_value = OPH_TERM_GENERIC_ERROR;
		return;
	}

	response_global.response = NULL;
	response_global.jobid = NULL;
	response_global.xml = NULL;
	response_global.buffer = (char *) malloc(1);
	*response_global.buffer = response_global.size = 0;
	response_global.error = OPH_SERVER_OK;

	char _password[OPH_MAX_STRING_SIZE];
	pthread_mutex_lock(&global_flag);
	char *_token = hashtbl_get(hashtbl, OPH_TERM_ENV_OPH_TOKEN);
	if (_token && strlen(_token)) {
		snprintf(_password, OPH_MAX_STRING_SIZE, "%s", _token);
		password = _password;
	}
	pthread_mutex_unlock(&global_flag);

	snprintf(username_global, OPH_MAX_STRING_SIZE, "%s", username);
	snprintf(password_global, OPH_MAX_STRING_SIZE, "%s", password);

	pthread_create(&tid, NULL, &wpsthread, strdup(result));
	pthread_join(tid, NULL);
	free(result);

	// Decoding
	if (!response_global.error && response_global.response) {
		int ret;
		size_t outLen = strlen(response_global.response);
		char result[1 + outLen];
		if ((ret = base64decode(response_global.response, outLen, result, &outLen))) {
			(print_json) ? my_fprintf(stderr, "Decoding error %d\\n", ret) : fprintf(stderr, "\e[1;31mDecoding error %d\e[0m\n%s\n", ret, response_global.response);
			*return_value = OPH_TERM_GENERIC_ERROR;
			if (response_global.response)
				free(response_global.response);
			if (response_global.jobid)
				free(response_global.jobid);
			if (response_global.xml)
				free(response_global.xml);
			if (response_global.buffer)
				free(response_global.buffer);
			return;
		}
		free(response_global.response);
		if (outLen)
			response_global.response = strdup(result);
		else
			response_global.response = NULL;
	}

	if (!wps_call_oph__ophExecuteMain_return) {
		switch (response_global.error) {
			case OPH_SERVER_OK:
				if (out_response) {
					if (!response_global.response) {
						*out_response = strdup("");
						if (!*out_response) {
							(print_json) ? my_fprintf(stderr, "Memory error with out_response\\n") : fprintf(stderr, "\e[1;31mMemory error with out_response\e[0m\n");
							*return_value = OPH_TERM_MEMORY_ERROR;
							break;
						}
					} else {
						*out_response = strdup((const char *) response_global.response);
						if (!*out_response) {
							(print_json) ? my_fprintf(stderr, "Memory error with out_response\\n") : fprintf(stderr, "\e[1;31mMemory error with out_response\e[0m\n");
							*return_value = OPH_TERM_MEMORY_ERROR;
							break;
						}
						if (strstr(response_global.response, "\"title\": \"ERROR\"")
						    || (strstr(response_global.response, "\"title\": \"Workflow Status\"")
							&& strstr(response_global.response, "\"message\": \"" OPH_ODB_STATUS_ERROR_STR "\"")))
							*return_value = OPH_TERM_GENERIC_ERROR;
					}
				} else {
					if (print_json)
						*oph_term_jobid = 0;
					if (!response_global.response) {
						if (out_response_for_viewer)
							*out_response_for_viewer = NULL;
						if (response_global.jobid && strlen(response_global.jobid)) {
							if (print_json)
								strncat(oph_term_jobid, response_global.jobid, OUTPUT_MAX_LEN);
							else
								printf("\e[1;34m[JobID]:\e[0m\n%s\n\n", response_global.jobid);
						}
						if (response_global.xml && strlen(response_global.xml)) {
							if (print_json)
								strncat(oph_term_jobid, response_global.xml, OUTPUT_MAX_LEN - strlen(oph_term_jobid));
							else
								printf("\e[1;34m[XML]:\e[0m\n%s\n\n", response_global.xml);
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
						if (response_global.jobid && strlen(response_global.jobid)) {
							if (print_json)
								strncat(oph_term_jobid, response_global.jobid, OUTPUT_MAX_LEN);
							else
								printf("\e[1;34m[JobID]:\e[0m\n%s\n\n", response_global.jobid);
						}
						if (response_global.xml && strlen(response_global.xml)) {
							if (print_json)
								strncat(oph_term_jobid, response_global.xml, OUTPUT_MAX_LEN - strlen(oph_term_jobid));
							else
								printf("\e[1;34m[XML]:\e[0m\n%s\n\n", response_global.xml);
						}
						if (!print_json)
							printf("\e[1;34m[Response]:\e[0m\n");
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

					if (newsession) {
						//retrieve newsession
						if (!response_global.jobid || strlen(response_global.jobid) == 0) {
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
									(print_json) ? my_fprintf(stderr, "Error retrieving jobid\\n") : fprintf(stderr, "\e[1;31mError retrieving jobid\e[0m\n");
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
				(print_json) ? my_fprintf(stderr, "Error on serving request [%ld]: undefined error\\n", response_global.error) : fprintf(stderr,
																			 "\e[1;31mError on serving request [%ld]: undefined error\e[0m\n",
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
		(print_json) ? my_fprintf(stderr, "Error on serving request [%d]: server error\\n", OPH_SERVER_ERROR) : fprintf(stderr, "\e[1;31mError on serving request [%d]: server error\e[0m\n",
																OPH_SERVER_ERROR);
		*return_value = OPH_SERVER_ERROR;
	}

	if (response_global.response)
		free(response_global.response);
	if (response_global.jobid)
		free(response_global.jobid);
	if (response_global.xml)
		free(response_global.xml);
	if (response_global.buffer)
		free(response_global.buffer);
}

int oph_term_client(char *cmd_line, char *command, char **newsession, char *user, char *password, char *host, char *port, int *return_value, char **out_response, char **out_response_for_viewer,
		    int workflow_wrap, HASHTBL * hashtbl)
{
	if (max_size <= 0)
		return 2;

	char *username = user;

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

	snprintf(server_global, OPH_MAX_STRING_SIZE, "https://%s:%s", host, port);

	oph_execute(query, newsession, return_value, out_response, out_response_for_viewer, workflow_wrap, username, password, hashtbl, cmd_line);

	free(query_global);
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
