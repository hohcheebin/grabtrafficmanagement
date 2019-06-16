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


>> Example run with the sample training dataset

./a.out -gqp098p -d1 -t0200..0245 training.csv 

will result in the following output:

qp098p,01,02:00,0.227952237769632110
qp098p,01,02:15,0.316001351815636045
qp098p,01,02:30,0.249384694063620904
qp098p,01,02:45,0.238222675389425292


>> How to sum up values?

a.out -gqp098p -d1 -t0200..0245 ../TrafficManagement/training.csv | awk 'BEGIN { FS="," } {sum+=$4} END {print sum}'

this will give us sum up of 1.031560959038314

By adding "NR" to awk, we can get the average

a.out -gqp098p -d1 -t0200..0245 ../TrafficManagement/training.csv | awk 'BEGIN { FS="," } {sum+=$4} END {print sum / NR}'

this will give us 0.257890239759579
