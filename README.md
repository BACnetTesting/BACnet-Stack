# BACnet-Stack

Open Source BACnet stack based Steve Karg's SourceForge project

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


	

