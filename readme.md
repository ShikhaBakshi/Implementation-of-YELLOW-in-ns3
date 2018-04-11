# Implementation-of-YELLOW-in-NS3
## Course Code: CS704
## Seminar Project
## Overview

Yellow [1] is a rate based AQM which takes into consideration the link utilization factor as a primary credit and also introduces a queue
ccontrol function (QCF) as a secondary merit to mark the packet with certain probability.

To simulate Yellow algorithm, the attribute queueDiscType must be set to YELLOW in the example yellow-vs-blue, as shown below:

## Example
An example program for YELLOW has been provided in

src/traffic-control/examples/yellow-vs-blue.cc

and should be executed as

./waf --run "yellow-vs-blue --queueDiscType=YELLOW"

./waf --run "yellow-vs-blue --queueDiscType=BLUE"

## References

[1] The Yellow active queue management algorithm, Chengnian Long * , Bin Zhao, Xinping Guan * , Jun Yang

[2] https://www.nsnam.org/ns-3-26/