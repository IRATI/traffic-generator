This software was jointly developed by iMinds and Nextworks s.r.l.
The research leading to these results has received funding from the European
Community's Seventh Framework Programme (FP7/2007-2013) under grant agreement
nº 238875 (GÉANT), Open Call 5 "IRINA".

See the LICENSE

The contributing members of the IRINA project are (alphabetical order):

* Addy Bombeke          - Ghent University/iMinds, BE
* Dimitri Staessens     - Ghent University/iMinds, BE
* Douwe De Bock         - Ghent University/iMinds, BE
* Francesco Salvestrini - Nextworks s.r.l., IT
* Sander Vrijders       - Ghent University/iMinds, BE
* Vincenzo Maffione     - Nextworks s.r.l., IT

This software has been further maintained and updated by the PRISTINE project,
funded by the EC under FP7 grant agreement nº 317814.


** Requirements:

IRATI should be installed. rina-tgen requires the boost C++ libraries,
version >= 1.55, to be installed.

** Quick Installation

You may need to configure the path for pkgconfig:

```
export PKG_CONFIG_PATH=/path/to/irati/lib/pkfconfig
```

Installation should work as follows:

```
./bootstrap
./configure --prefix=/path/to/irati/
make
make install
```
after this, the rina-tgen binaries are in /path/to/irati/bin

see the INSTALL file for more configure options.
