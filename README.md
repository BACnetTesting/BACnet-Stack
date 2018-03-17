# BACnet-Stack

This "BACnet Reference Stack" is based on Steve Karg's original work on SourceForge.

It has been pummelled, fixed and upgraded to support various more advanced applications, as well as
matching the coding style and testing requirements of BACnet Interoperability Testing Services, Inc.

In the spirit, and legal requirements, of the original open source licensing terms, the code changes
have been commented and placed back in the public domain and can be found on GitHub at:

    https://github.com/BACnetTesting/BACnet-Stack

Any modifications to the original Steve Karg stack are made in a precedence from top to bottom below.
Relevant changes are made as early as possible, but not so soon that they 'break' the flow of changes
that are migrated through the rest of the repository via automatic branch merges.

                        ----------------------------------------------------------------

## Branch name
### Description

Shadow of Steve Kargs stack
    Straight copy of Steve's latest release from https://sourceforge.net/projects/bacnet/

Compile MSVC
    * Add MSVC projects, solution, NO FURTHER CHANGES apart from getting MSVC to compile.
    * A note on the Microsoft solution layout (applies to this, and the rest of the branches)
        * In ports\win32\Microsoft Visual Studio\ there is a Microsoft Visual Studio Solution file (*.sln)that contains multiple projects
            * BACnet Stack Library project - common Stack Files
            * Projects to build executables - e.g. readprop, server
        * A common Visual Studion "properties" file ("BACnet Solution Settings.props") containing common settings, such as include paths, key #defines to be shared across all projects.
        * Individual MVS project files contain more specific project settings


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




