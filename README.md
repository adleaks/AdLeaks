AdLeaks
=======

AdLeaks is a secure submission system for online whistleblowing platforms. Please see [https://www.inf.fu-berlin.de/groups/ag-si/](https://www.inf.fu-berlin.de/groups/ag-si/) for more information.

---

#### WARNING
This source code depicts a research prototype and is still under active development. 
So be warned not to use it on a production system and BY NO MEANS roll out your own whistleblowing platform with it. 
For example: the implemented protocol is still missing the outer encryption mentioned in our [technical report](http://arxiv.org/abs/1301.6263). 

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
    cd build
    cmake ..
    make

#### Clients
##### Prerequisites
* mocha (unit testing)

##### Build instructions
Simply type *make* in the Folder *Clients/*.

#### Setup instructions
will come soon...
