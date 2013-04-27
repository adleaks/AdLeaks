AdLeaks
=======

AdLeaks is a secure submission system for online whistleblowing platforms. Please see [http://adleaks.org](adleaks.org) for more information.

---

#### NOTE and WARNING
This source code depicts a research prototype and is still under active development. So be warned to not use it on a production system.

#### License
AdLeaks is released under the version 3 of the GNU general public license. For details please see the *COPYING* and 
the *attributions_and_licenses.txt* files.

---

#### ALTools
##### Prerequisites
Libraries:

* libgmp
* fcgi
* libb64
* libevent

Tools:

  * cmake
  * check (unit testing)
  * ctest (runner for unit tests?)

##### Build instructions:
In the Folder *ALTools/ALTools* type
	mkdir build
	cmake ..
	make

#### Clients
##### Prerequisites
* mocha (unit testing)

##### Build instructions
Simply type *make* in the Folder *Clients/*.

#### Setup instructions
will come soon...