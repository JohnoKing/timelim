timelim
========
_A program capable of setting very long time limits_

## What is this
This is a version of the sleep(1) utility that can set time limits that are as short as a few microseconds or as long as multiple centuries. 
There is no real point to this program, it is just something to experiment with.

## Building and Usage
Run `make` and the timelim binary will be compiled.  

## Usage
`Usage: sleep/timelim [-cdhmnorsvwy?] length ...`  
`  -c, --centuries     Number of centuries`  
`  -d, --days          Number of days`  
`  -h, --hours         Number of hours`  
`  -m, --minutes       Number of minutes`  
`  -n, --micro         Number of microseconds`  
`  -o, --months        Number of months`  
`  -r, --run           Run the specified command when the time runs out`  
`  -s, --seconds       Number of seconds`  
`  -v, --verbose       Enable verbose output`  
`  -w, --weeks         Number of weeks`  
`  -y, --years         Number of years`  
`  -?, --help          Display this text`  

## Examples
Sleep for 2 seconds with verbose output (when run as sleep):
`sleep -v 2`

Set a time limit to last a minute: 
`timelim -m 1` or `./timelim -s 60`

Set a time limit to last one year and three weeks:
`timelim -y 1 -w 3`

Set a time limit to only last 24 microseconds:
`timelim --micro 24`

Set a time limit to last one century and 4 months:
`timelim --century --months 4`

Run uname after 27 seconds:
`timelim --seconds 27 --run "uname -a"`
