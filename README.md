# libblossom—extremely parallel thread spawning

[![Build Status](https://drone.dsscaw.com:4443/api/badges/dankamongmen/libblossom/status.svg)](https://drone.dsscaw.com:4443/dankamongmen/libblossom)

libblossom automates the highly parallel spawning of threads. This is
useful on machines with many processing elements, or NUMA machines where
locality among processor subgroups is critical. Either some absolute
number of threads can be spawned, or a number per processing element.

## BUILDING

A C compiler, autotools, and xsltproc are required.

If building from a git checkout, run `autoreconf -fis`.

Run `make all`. GNU make is required.

## INSTALLING

Run `make install`.
