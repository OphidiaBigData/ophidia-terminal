# Ophidia Terminal

### Description

The Ophidia Terminal is an advanced bash-like interpreter through which it is possible to submit arbitrary requests to an Ophidia Server.

A user can submit simple commands in an interactive and sequential manner as well as complex workflows, described by JSON files structured according to the Ophidia Workflow JSON Schema (look at the Ophidia Server for a complete syntax definition).

It provides the possibility to use environment variables and aliases, keyboard shortcuts, recursive history search, coloured outputs, implicit session management etc.

### Requirements

In order to compile and run the Ophidia Terminal, make sure you have the following packages (all available through CentOS official repositories and the epel repository) properly installed:

1. jansson and jansson-devel
2. graphviz and graphviz-devel
3. gtk2 and gtk2-devel
4. libxml2 and libxml2-devel
5. libcurl and libcurl-devel
6. openssl and openssl-devel
7. readline and readline-devel
8. libssh2 and libssh2-devel
9. globus-common-devel (only for GSI support)
10. globus-gsi-credential-devel (only for GSI support)
11. globus-gsi-proxy-core-devel (only for GSI support)
12. globus-gssapi-gsi-devel (only for GSI support)
13. voms-devel (only for GSI support)

**Note**:

This product includes software developed by the OpenSSL Project for use in the OpenSSL Toolkit.

### How to Install

If you are building from git, you also need automake, autoconf and libtool. To prepare the code for building run:

```
$ ./bootstrap 
```

The source code has been packaged with GNU Autotools, so to install simply type:

```
$ ./configure
$ make
$ make install
```

Type:

```
$ ./configure --help
```

to see all available options, like *--prefix* for explicitly specifying the installation base directory.

If you want to use the program system-wide, remember to add its installation directory to your PATH.

Recall that the history file (*.oph\_term\_history*) and the XML folder used for autocompletion features (*.oph\_term\_xml*) will be located in your HOME.

### How to Launch

```
$ oph_term -u username -p password -H server_address -P server_port
```

Type:

```
$ oph_term --help
```

to see all other available options.

Further information can be found at [http://ophidia.cmcc.it/documentation](http://ophidia.cmcc.it/documentation).
