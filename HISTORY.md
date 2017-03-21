
## v1.0.0 - 2017-03-21

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

- Bug [\#3](https://github.com/OphidiaBigData/ophidia-terminal/issues/3)
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
