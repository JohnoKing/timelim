timelim
========
_A program capable of setting very long time limits_

## What is this
This is a random program I made to sort of function as a 
stopwatch, but with time limits that can span centuries. 
There is no real point to this program, it is just something 
to experiment with.

## Building and Usage
Run `make` and the timelim binary will be compiled.  
Output from --help:

`Usage: timelim [-cdhmosvwy?] length ...`  
`  -c, --centuries     Number of centuries (may break things)`  
`  -d, --days          Number of days`  
`  -h, --hours         Number of hours`  
`  -m, --minutes       Number of minutes`  
`  -o, --months        Number of months`  
`  -s, --seconds       Number of seconds`  
`  -v, --verbose       Display extra info`  
`  -w, --weeks         Number of weeks`  
`  -y, --years         Number of years`  
`  -?, --help          Display this text`  

## Examples
Set a time limit to last a minute: 
`./timelim -m 1` or `./timelim -s 60`

Set a time limit to last one year and three weeks:
`./timelim -y 1 -w 3`

Set a time limit to last one century and 4 months (long options):
`./timelim --centuries 1 --months 4`
