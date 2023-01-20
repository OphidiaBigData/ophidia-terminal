
## v1.7.1 - 2023-01-20

### Fixed:

- Bug in parsing for comments in JSON Requests

### Added:

- Support for CentOS 9

### Changed:

- Enable the possibility to use '=' in parameter values

## v1.7.0 - 2022-07-01

### Fixed:

- Bug in job status colours in OPH_RESUME
- Bugs in configuration script for code building

### Changed:

- Improve JSON Request handler [#22](https://github.com/OphidiaBigData/ophidia-terminal/pull/22/)

## v1.6.0 - 2021-03-05

### Added:

- Support to append Authorization Bearer to WPS Requests [#21](https://github.com/OphidiaBigData/ophidia-terminal/pull/21/)
- Field 'project' to header of JSON Request as well as new environment variable OPH_PROJECT [#20](https://github.com/OphidiaBigData/ophidia-terminal/pull/20/)
- Option '-d' for debug mode
- New environment variable 'OPH_REQUEST_BUFFER' to define request max size [#18](https://github.com/OphidiaBigData/ophidia-terminal/pull/18/)
- Default value for 'view' as the workflowid of last submitted command
- Check for arguments 'nhost' and 'nthreads'

### Fixed:

- Output format when OPH_TERM_FORMAT is set to 'compact'
- Bug in executing 'resume' [#17](https://github.com/OphidiaBigData/ophidia-terminal/issues/17)
- Memory leak raised in case XML description of an operator is not correctly formatted
- Bug related to allocation of char buffers in stack memory

### Changed:

- Update 'deploy_status' command to be compliant with IM interface [#19](https://github.com/OphidiaBigData/ophidia-terminal/pull/19/) 
- Name of parameter 'return' and mimeType of WPS interface
- Color associated with status 'OPH_STATUS_SKIPPED'
- Default host partition to 'auto'
- Drop alias in order to use async execution mode
- Check for argument 'ncores'

### Removed:

- Necessity of the suffix '/wps' in WPS URLs

## v1.5.0 - 2019-01-24

### Fixed:

- Various warning when building on Ubuntu 18
- Global variable definition in header files

### Added:

- A new alias 'rmr' [#11](https://github.com/OphidiaBigData/ophidia-terminal/pull/11/)
- Check for OPH_DELETECONTAINER as workflow exit action [#11](https://github.com/OphidiaBigData/ophidia-terminal/pull/11/)

### Changed:

- Soap interface files with gSOAP version 2.8.76 [#16](https://github.com/OphidiaBigData/ophidia-analytics-framework/pull/16)
- 'rm' alias to drop references to hidden containers [#15](https://github.com/OphidiaBigData/ophidia-terminal/pull/15/)
- Default host partition name to 'main' [#14](https://github.com/OphidiaBigData/ophidia-terminal/pull/14/)
- Display of status in command outputs with different colors [#13](https://github.com/OphidiaBigData/ophidia-terminal/pull/13/)
- 'rc' alias [#12](https://github.com/OphidiaBigData/ophidia-terminal/pull/12/)
- Path completion to add the key-word separator at the end
- Raise an error in case any task parameter contains special characters

## v1.4.0 - 2018-07-27

### Fixed:

- Bug in workflow comments parsing

## v1.3.0 - 2018-06-18

### Added:

- Environment variable OPH_TIMEOUT to handle connection timeout [#10](https://github.com/OphidiaBigData/ophidia-terminal/pull/10/)
- Environment variable OPH_HOST_PARTITION to handle reserved host partition [#9](https://github.com/OphidiaBigData/ophidia-terminal/pull/9)
- Support to update some environment variables based on extra fields in json response [#8](https://github.com/OphidiaBigData/ophidia-terminal/pull/8)
- Support for comments in workflow requests [#7](https://github.com/OphidiaBigData/ophidia-terminal/pull/7)

### Fixed:

- Algorithm used to check workflow validity
- Warnings and configuration checks when building
- Wrong behavior of 'resume' command [#6](https://github.com/OphidiaBigData/ophidia-terminal/issues/6)
- Reset locale to 'C' before parsing strings
- Avoid printing command execution time in case dump viewer is used

## v1.2.0 - 2018-02-16

### Fixed:

- Bug in buffer overflow check during creation of dot string 
- Bug [#4](https://github.com/OphidiaBigData/ophidia-terminal/issues/4)
- Memory leaks when loading XML files
- Bug in handling oph_base_src_path in case of workflows

### Added:

- URL to workflow header [#5](https://github.com/OphidiaBigData/ophidia-terminal/pull/5)
- Control for known errors in WPS client
- 'creation time' to history when calling 'resume' command
- Print execution time of commands 
- OPH_CDD in submitted workflows
- 3 new aliases: mkdird, rmd, mvd
- Single call to get all configuration parameters from server
- Auto-setting of cdd from server configuration
- cdd parameter to workflow struct


## v1.1.0 - 2017-07-28

### Fixed:

- Hide OPH_PASSWD

### Added:

- New aliases: getprogress, show, new, drop, jobs, ncdump
- Support for OpenId Connect (INDIGO-DataCloud Project)
- Support for OPH_FS: setting and automatic update of OPH_CDD
- Configuration option to disable SSL certificates check for XML download
- Support for WPS interface

### Changed:

- Exploit OPH_BASE_SRC_PATH to set the parameter automatically

## v1.0.0 - 2017-03-23

### Changed:

- Code indentation style

## v0.11.0 - 2017-01-31

### Fixed:

- Memory leaks in graphic libraries usage

### Added:

- Watch command
- Support for OPH_RUNNING_ERROR status in case of workflow is still running
- Disk color for OPH_STATUS_ABORTED status

### Changed:

- View command to filter jobs based on their status

## v0.10.6 - 2016-10-20

### Fixed:

- Bug [#3](https://github.com/OphidiaBigData/ophidia-terminal/issues/3)
- Bug in gtk handler when window is closed while running in auto-view mode

### Added:

- Environmental variable OPH_TERM_FORMAT to format workflow response format
- Support for new state OPH_STATUS_WAITING (INDIGO-DataCloud Project)

### Changed:

- Output of resume command to also show the current workflow status 

## v0.10.5 - 2016-08-24

### Fixed:

- Plotting (nested) rectangles in case of OPH_FOR (INDIGO-DataCloud Project)
- Bug in color of skipped tasks
- Several warnings when building

### Added:

- Support for data types 'short' and 'bytes' in JSON Responses
- Graphical support for OPH_IF (INDIGO-DataCloud Project)
- Support for OPH_IF in workflow check (INDIGO-DataCloud Project)

### Changed:

- Help description for OPH_WORKFLOW_AUTOVIEW
- Colors of pending tasks

## v0.10.1 - 2016-06-27

### Fixed:
 
- Version number in files

## v0.10.0 - 2016-06-23

### Added:

- Support for building on CentOS7 and Ubuntu

## v0.9.0 - 2016-02-01

 - Initial public release 
