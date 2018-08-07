timelim
========
**The primary repo has been moved to https://gitlab.com/JohnoKing/timelim**
_A program capable of setting very long time limits_

## What is this
This is a random program I made to sort of function as a 
stopwatch, but with time limits that can span up to a century. 
There is no real point to this program, it is just something 
to experiment with.

## Building and Usage
Run `make` and the timelim binary will be compiled.  
Output from --help:

`Usage: timelim [-cdhmorswy?] length ...`  
`  -c, --century       Add one century to the time limit`  
`  -d, --days          Number of days`  
`  -h, --hours         Number of hours`  
`  -m, --minutes       Number of minutes`  
`  -o, --months        Number of months`  
`  -r, --run           Run the specified command when the time runs out`  
`  -s, --seconds       Number of seconds`  
`  -w, --weeks         Number of weeks`  
`  -y, --years         Number of years`  
`  -?, --help          Display this text`  

## Examples
Set a time limit to last a minute: 
`./timelim -m 1` or `./timelim -s 60`

Set a time limit to last one year and three weeks:
`./timelim -y 1 -w 3`

Set a time limit to last one century and 4 months (long options):
`./timelim --century --months 4`

Run uname after 27 seconds:
`./timelim --seconds 27 --run "uname -a"`
