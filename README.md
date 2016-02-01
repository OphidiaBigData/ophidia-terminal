<h1>Ophidia Terminal</h1>

<h3>Description</h3>
The Ophidia Terminal is an advanced bash-like interpreter through which it is possible to submit arbitrary requests to an Ophidia Server.</br>
A user can submit simple commands in an interactive and sequential manner as well as complex workflows, described by JSON files structured according to the Ophidia Workflow JSON Schema (look at the Ophidia Server for a complete syntax definition).</br>
It provides the possibility to use environment variables and aliases, keyboard shortcuts, recursive history search, coloured outputs, implicit session management etc.

<h3>Requirements</h3>
In order to compile and run the Ophidia Terminal, make sure you have the following packages (all available through CentOS official repositories and the epel repository) properly installed:
<ol>
  <li>jansson and jansson-devel</li>
  <li>graphviz and graphviz-devel</li>
  <li>gtk2 and gtk2-devel</li>
  <li>libxml2 and libxml2-devel</li>
  <li>libcurl and libcurl-devel</li>
  <li>openssl and openssl-devel</li>
  <li>readline and readline-devel</li>
  <li>libssh2 and libssh2-devel</li>
  <li>globus-common-devel (only for GSI support)</li>
  <li>globus-gsi-credential-devel (only for GSI support)</li>
  <li>globus-gsi-proxy-core-devel (only for GSI support)</li>
  <li>globus-gssapi-gsi-devel (only for GSI support)</li>
  <li>voms-devel (only for GSI support)</li>
</ol>
<b>Note</b>:</br>
This product includes software developed by the OpenSSL Project for use in the OpenSSL Toolkit.

<h3>How to Install</h3>
The source code has been packaged with GNU Autotools, so look at the INSTALL file or simply type:</br></br>
<code>./configure</br>
make</br>
make install</br></br></code>
Type:</br></br>
<code>./configure --help</code></br></br>
to see all available options, like --prefix for explicitly specifying the installation base directory.</br>

If you want to use the program system-wide, remember to add its installation directory to your PATH.</br>
Recall that the history file (<i>.oph\_term\_history</i>) and the XML folder used for autocompletion features (<i>.oph\_term\_xml</i>) will be located in your HOME.

<h3>How to Launch</h3>
<code>oph_term -u <i>username</i> -p <i>password</i> -H <i>server_address</i> -P <i>server_port</i></code></br></br>
Type:</br></br>
<code>oph_term --help</code></br></br>
to see all other available options.</br></br>

Further information can be found at <a href="http://ophidia.cmcc.it/documentation">http://ophidia.cmcc.it/documentation</a>.

