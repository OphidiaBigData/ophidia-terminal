#
#    Ophidia Terminal
#    Copyright (C) 2012-2017 CMCC Foundation
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

IFLAG =
LIBRARY =

LIBRARY += liboph_term_client.la

if INTERFACE_TYPE_IS_SSL
IFLAG += -DINTERFACE_TYPE_IS_SSL
liboph_term_client_la_SOURCES = client/$(INTERFACE_TYPE)/oph_term_client.c client/stdsoap2.c client/soapClient.c client/soapC.c
liboph_term_client_la_CFLAGS= -prefer-pic -I. -Iclient -Iclient/$(INTERFACE_TYPE) -Ienv ${lib_CFLAGS} $(OPT) -DWITH_DOM $(LIBXML_INCLUDE) $(LIBCURL_INCLUDE) -DWITH_OPENSSL -DHAVE_OPENSSL_SSL_H $(LIBSSL_INCLUDE) $(IFLAG)
liboph_term_client_la_LDFLAGS = -static
liboph_term_client_la_LIBADD = $(LIBXML_LIB) $(LIBCURL_LIB) -lpthread $(LIBSSL_LIB) -lcrypto
endif

if INTERFACE_TYPE_IS_GSI
DEFINE = -DGSI_PLUGIN_DEBUG=1
IFLAG += -DINTERFACE_TYPE_IS_GSI
if VOMS
GLOBUS_LIBS = -lglobus_gssapi_gsi -lglobus_gsi_proxy_core -lglobus_gsi_credential -lglobus_gsi_callback -lglobus_oldgaa -lglobus_gsi_sysconfig -lglobus_gsi_cert_utils -lglobus_openssl -lglobus_proxy_ssl -lglobus_openssl_error -lssl -lcrypto -lglobus_common
GLOBUS_GRAM_LIBS = -lglobus_gram_client -lglobus_gram_protocol -lglobus_io -lglobus_gss_assist -lglobus_gssapi_gsi -lglobus_gsi_proxy_core -lglobus_gsi_credential -lglobus_gsi_callback -lglobus_oldgaa -lglobus_gsi_sysconfig -lglobus_gsi_cert_utils -lglobus_openssl -lglobus_proxy_ssl -lglobus_openssl_error -lssl -lcrypto -lglobus_common
AM_CFLAGS = $(OPT) -D$(PLATFORM) -D$(VOMS_DEFINE) -I../include -I$(VOMS_INCLUDE) -Wall
MYCFLAGS = $(AM_CFLAGS) -I$(GLOBUS_INCLUDE) $(DEFINE)
VOMS_LINK = -lvomsapi
MYLDFLAGS  = -L$(VOMS_LIBS) $(VOMS_LINK) -L$(GLOBUS_LIB) $(GLOBUS_LIBS)
MYGRAMLDFLAGS  = -L$(VOMS_LIBS) $(VOMS_LINK) -L$(GLOBUS_LIB) $(GLOBUS_GRAM_LIBS)
else
GLOBUS_LIBS = -lglobus_gssapi_gsi -lglobus_gsi_proxy_core -lglobus_gsi_credential -lglobus_gsi_callback -lglobus_oldgaa -lglobus_gsi_sysconfig -lglobus_gsi_cert_utils -lglobus_openssl -lglobus_openssl_error -lglobus_proxy_ssl -lglobus_common -lssl -lcrypto -lltdl
GLOBUS_GRAM_LIBS = -lglobus_gram_client -lglobus_gram_protocol -lglobus_io -lglobus_xio -lgssapi_error -lglobus_gss_assist -lglobus_gssapi_gsi -lglobus_gsi_proxy_core -lglobus_gsi_credential -lglobus_gsi_callback -lglobus_oldgaa -lglobus_gsi_sysconfig -lglobus_gsi_cert_utils -lglobus_openssl -lglobus_openssl_error -lglobus_proxy_ssl -lglobus_common -lssl -lcrypto -lltdl -lm
AM_CFLAGS = $(OPT) -D$(PLATFORM) -I../include -Wall
MYCFLAGS = $(AM_CFLAGS) -I$(GLOBUS_INCLUDE) $(DEFINE)
MYLDFLAGS  = -L$(GLOBUS_LIB) $(GLOBUS_LIBS)
MYGRAMLDFLAGS  = -L$(GLOBUS_LIB) $(GLOBUS_GRAM_LIBS)
endif
liboph_term_client_la_SOURCES = client/$(INTERFACE_TYPE)/oph_term_client.c client/stdsoap2.c client/$(INTERFACE_TYPE)/gsi.c client/soapClient.c client/soapC.c
liboph_term_client_la_CFLAGS= -prefer-pic -I. -Iclient -Iclient/$(INTERFACE_TYPE) -Ienv ${lib_CFLAGS} $(OPT) -DWITH_DOM $(LIBXML_INCLUDE) $(LIBCURL_INCLUDE) $(MYCFLAGS) $(IFLAG)
liboph_term_client_la_LDFLAGS = -static $(MYLDFLAGS)
liboph_term_client_la_LIBADD = $(LIBXML_LIB) $(LIBCURL_LIB) -lpthread
endif

if INTERFACE_TYPE_IS_WPS
IFLAG += -DINTERFACE_TYPE_IS_WPS
liboph_term_client_la_SOURCES = client/$(INTERFACE_TYPE)/oph_term_client.c
liboph_term_client_la_CFLAGS= -prefer-pic -I. -Iclient -Iclient/$(INTERFACE_TYPE) -Ienv ${lib_CFLAGS} $(OPT) -DWITH_DOM $(LIBXML_INCLUDE) $(LIBCURL_INCLUDE) $(IFLAG)
liboph_term_client_la_LDFLAGS = -static
liboph_term_client_la_LIBADD = $(LIBXML_LIB) $(LIBCURL_LIB) -lpthread
endif

LIBRARY += liboph_term_env.la
liboph_term_env_la_SOURCES = env/oph_term_env.c env/hashtbl.c
liboph_term_env_la_CFLAGS= -prefer-pic -I. -Ienv -Iclient ${lib_CFLAGS} $(OPT) $(LIBXML_INCLUDE) $(LIBCURL_INCLUDE) $(IFLAG)
liboph_term_env_la_LDFLAGS = -static
liboph_term_env_la_LIBADD = $(LIBXML_LIB) $(LIBCURL_LIB) -lm -lz -L. -loph_term_client

LIBRARY += liboph_term_help.la
liboph_term_help_la_SOURCES = help/oph_term_help.c
liboph_term_help_la_CFLAGS= -prefer-pic -I. -Ihelp ${lib_CFLAGS} $(OPT) $(LIBXML_INCLUDE) $(IFLAG)
liboph_term_help_la_LDFLAGS = -static
liboph_term_help_la_LIBADD = -lm -lz

LIBRARY += liboph_term_viewer.la
liboph_term_viewer_la_SOURCES = viewer/oph_json/oph_json_common.c viewer/oph_json/oph_json_graph.c viewer/oph_json/oph_json_grid.c viewer/oph_json/oph_json_multigrid.c viewer/oph_json/oph_json_text.c viewer/oph_json/oph_json_tree.c viewer/oph_term_viewer.c
liboph_term_viewer_la_CFLAGS= -prefer-pic -I. -Iviewer/oph_json -Iviewer $(JANSSON_CFLAGS) ${lib_CFLAGS} $(OPT) $(LIBGVC_INCLUDE) $(IFLAG)
liboph_term_viewer_la_LDFLAGS = -static -Wl,-rpath -Wl,$(JANSSON_LIBDIR)
liboph_term_viewer_la_LIBADD = $(JANSSON_LIBS) $(LIBGVC_LIB)

LIBRARY += liboph_term_workflow.la
liboph_term_workflow_la_SOURCES = workflow/oph_workflow_parsing.c workflow/oph_workflow_setup.c workflow/oph_workflow_structs.c
liboph_term_workflow_la_CFLAGS= -prefer-pic -I. -Iclient -Iworkflow -Iviewer/oph_json -Iviewer -Ienv $(JANSSON_CFLAGS) ${lib_CFLAGS} $(OPT) $(LIBGVC_INCLUDE) $(GTK2_INCLUDE) $(LIBXML_INCLUDE) $(LIBCURL_INCLUDE) $(IFLAG)
liboph_term_workflow_la_LDFLAGS = -static -Wl,-rpath -Wl,$(JANSSON_LIBDIR)
liboph_term_workflow_la_LIBADD = $(LIBXML_LIB) $(LIBCURL_LIB) $(JANSSON_LIBS) $(LIBGVC_LIB) $(GTK2_LIB) -lpthread -loph_term_client

noinst_LTLIBRARIES = $(LIBRARY)

bin_PROGRAMS=oph_term
bindir=${prefix}/bin

oph_term_SOURCES= oph_term.c
oph_term_CFLAGS= $(OPT) -I. -Iclient -Ienv -Ihelp -Iviewer -Iworkflow ${lib_CFLAGS} $(LIBZ_INCLUDE) $(LIBXML_INCLUDE) $(FLAG) -DPACKAGE_VERSION=\"@PACKAGE_VERSION@\"
oph_term_LDADD= -L. -loph_term_client -loph_term_env -loph_term_help -loph_term_viewer -loph_term_workflow -lreadline -lhistory $(LIBZ_LIB) -lncurses -lm -lpthread

distclean-local:
	cd client && rm -f *~ #*.nsmap *.xsd *.xml soap* *.wsdl

