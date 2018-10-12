1. Kamailio Compilation
----------------------

  * make proper  	- clean also the dependencies and the config, but not the module list
  * make cfg  	 	- generate configuration with default modules
  * cp ../cfg/modules.lst ./ 	- replace the default modules list with IMS modules list.
  * make all 		- build all modules 

  Make tags:
	make TAGS

  Create a tar.gz with the sources (in ../):
	make tar

  Create a tar.gz with the binary distribution (in ../):
	make bin

  Create a gzipped solaris package (in ../):
	make sunpkg

  Create debian packages (in ../):
	make deb

  or
	dpkg-buildpackage


2. Kamailio Clean:
------

  * make clean   	- clean the base and modules too
  * make proper  	- clean also the dependencies and the config, but not the module list
  * make distclean 	- the same as proper
  * make maintainer-clean - clean everything, including make's config, saved 
  			  module list, auto generated files, tags, *.dbg a.s.o
  * make clean-all	- clean all the modules in modules/*
  * make proper-all 	- like make proper but for all the  modules in modules/*

  Config clean:

  * make clean-cfg (cleans the compile config)
  * make clean-modules-cfg (cleans the modules list)

  Reduced" clean:

  * make local-clean  	- cleans only the core, no libs, utils or modules
  * make clean-modules  - like make clean, but cleans only the modules
  * make clean-libs     - like make clean, but cleans only the libs
  * make clean-utils    - like make clean, but cleans only the utils
  * make proper-modules - like make proper, but only for modules
  * make proper-libs    - like make proper, but only for libs
  * make proper-utils   - like make proper, but only for utils
