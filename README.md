
The Orocos Real-Time Toolkit
============================

Quick Installation
------------------

The Orocos RTT requires the [cmake](http://www.cmake.org) program to be present, version
2.8.3 or later is required.

Download the latest version from https://github.com/orocos-toolchain/rtt/releases

```
tar -xvjf <version>.tar.gz
cd rtt-<version>
mkdir build
cd build
cmake ..
make
make check
make install
```

The installation manual can be found in the doc dir.  You can also
consult it online on the http://www.orocos.org website.


Configuring Orocos RTT
----------------------

Instead of `cmake ..`, issue `ccmake ..` from your build
directory.  The keys to use are 'arrows'/'enter' to modify a setting,
'c' to run a configuration check (may be required multiple times), 'g'
to generate the makefiles. If an additional configuration check is
required, the 'g' key can not be used and you must press 'c' and
examine the output.

CORBA
-----

CORBA is not enabled by default. If you have [ACE/TAO](https://objectcomputing.com/products/tao) or [omniORB](http://omniorb.sourceforge.net/) installed, use

```
cmake .. -DENABLE_CORBA=ON -DCORBA_IMPLEMENTATION=TAO
```

or

```
cmake .. -DENABLE_CORBA=ON -DCORBA_IMPLEMENTATION=OMNIORB
```
