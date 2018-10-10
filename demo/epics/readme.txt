Using the epics app
===================

EKH 2017.07.04 

Microsoft MSVC 

Unfortunately environment settings are in the *.user files, so this file is included in the source tree for example.

The environment variables are set to specify the remote BBMD address/port
The debug command line (in the project) specifies the rest of the information

Compile time note:
Note that this application needs BACNET_PROPERTY_LISTS=1
	so this is set in the PROJECT properties
	and the MSVC filters include src/proplist.c to override that module in the _lib library.

