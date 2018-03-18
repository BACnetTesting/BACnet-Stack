# Phases of BITS BACnet Reference Stack

This "BACnet Reference Stack" is based on Steve Karg's original work on SourceForge.

It has been pummelled, fixed and upgraded to support various more advanced applications, as well as 
matching the coding style and testing requirements of BACnet Interoperability Testing Services, Inc.

We only support a subset of the demonstrations and ports that the original stack provided, and where
we do not support a function, we have removed the code in order to avoid ambiguity.

In the spirit, and legal requirements, of the original open source licensing terms, the code changes
have been commented and placed back in the public domain and can be found on GitHub at:

    https://github.com/BACnetTesting/BACnet-Stack

Any modifications to the original Steve Karg stack are made in a precedence from top to bottom below.
Relevant changes are made as early as possible, but not so soon that they 'break' the flow of changes
that are migrated through the rest of the repository via automatic branch merges.

                        ----------------------------------------------------------------

The project is arranged in branches, each branch with additional functionality. Pick the branch that
suits your application the best and start there.

## Branch name
### Description

Shadow of Steve Kargs stack
    Straight copy of Steve's latest release from https://sourceforge.net/projects/bacnet/

###Compile MSVC
    * Add MSVC projects, solution, NO FURTHER CHANGES apart from getting MSVC to compile.
    * A note on the Microsoft solution layout (applies to this, and the rest of the branches)
        * In ports\win32\Microsoft Visual Studio\ there is a Microsoft Visual Studio Solution file (*.sln)that contains multiple projects
            * BACnet Stack Library project - common Stack Files
            * Projects to build executables - e.g. readprop, server
        * A common Visual Studion "properties" file ("BACnet Solution Settings.props") containing common settings, such as include paths, key #defines to be shared across all projects.
        * Individual MVS project files contain more specific project settings

###Global Renames
	(This is not really a useful branch, use "Compile MSVC C++ branch as your starting point for BITS codebase")
	BITS copyright message
	NPDU -> NPCI

###Syntax Fixups
	(This is not really a useful branch, use "Compile MSVC C++ branch as your starting point for BITS codebase")
	Remove 'return'
	Remove unnecessary initializations
	{ }

###Compile C++
	Only uses C++ features for better type checking during compiles
	One can switch between projects within solution to run different examples

###Feature Creep
	Intent:
		Uses C++ Objects (only for BACnet Objects)
		This is the last 'single datalink, non routing, non virtual device' configuration.
	Removing
		All references to UCI
		Removing unsupported platforms
		Removing 'old' printf tracing and debugging
	Adding:
		Better user menus on examples
		Finer grained debug #defines (dbTraffic()) etc.
        BITS utilities and 'helpers'
        BTA support
		Dynamic object creation
		Common function calls
		Breaking out datalink into module
		Datalink, BACnet 'main' run in threads

				------
				Status: 2017.10.01 Runs vs2017 - server, x64 debug

###MultipleDatalinks
	Added dlcb, removed Tx buffers
    Note: This has a VERY NARROW use-case. A server device with a serial port and an Ethernet port, and the plugged in port 'goes live'
    All other dual-port applications have to be router applications..
    If the intent is to choose at STARTUP then Steve Karg's original dlenv-commandline approach could be adapted.

		------
		Status: 2017.09.30 Does not compile, just using as a "Hg merge" stepping-stone to "Full Routing"

Virtual Devices
	-- obsolete - there cannot be a concept of virtual devices without full routing.... Virtual Devices
	-- and the step between full routing (and one Application Entity) and full routing and one application entity and other virtual entities is so small we are not going to even bother. Roll it all up into "Full Routing" phase just before this one.

###Full Routing
    And _this_ is where the action happens. Virtual devices, multiple datalinks, routing.


# Compiling and sanity test under Ubuntu

## Make sure all the tools are installed:
    sudo apt-get install gcc
    sudo apt-get install git

## Get the sources from the GitHub repository
    git clone https://github.com/BACnetTesting/BACnet-Stack.git dev

## Build the executables
    cd dev
    make clean all (note, there is a problem if you run make twice!)

## Run the 'server' example
    demo/server/bacserv

## Now that the server is running, go to another machine, and run a BACnet client and "discover"
    or use the BACnet stack Who-Is
    demo/whois/bacwi

for more information, email info@bac-test.com

