# grabtrafficmanagement
Grab traffic management challenge

>> Prequisite
You need a ANSI C compliant compiler, and preferrable a GNU/Linux, Unix like or MacOS environment.

The following assumes that your c compiler is cc

>> How to compile
cc trafficdemand.c -o a.out

>> How to run it
OVERVIEW: Trafic demand management and filtering tool

USAGE: a.out [options] file ...

[options]
    -gqp03tu,qp09fu                    Filter demands matching the geohash6 values qp03tu or qp09fu
    -d1..3,5..6,9                      Filter demands matching day 1 to 3, 5 to 6 or 9
    -t1000..1045,1315..1345            Filter demands matching time 10:00 to 10:45 or 13:15 to 13:45


If file is not given, it is reading from standard input


