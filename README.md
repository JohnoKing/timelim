timelim
========
_A program capable of setting very long time limits_

## What is this
This is a version of the sleep(1) utility that can set time limits that are as short as a few nanoseconds or as long as multiple centuries. 
There is no real point to this program, it is just something to experiment with.

## Building and Usage
Run `make` and the timelim binary will be compiled.  

## Usage
`Usage: sleep/timelim [-rv?] length[suffix] ...`  
`  -r, --run           Run the specified command when the time runs out`  
`  -v, --verbose       Enable verbose output`  
`  -?, --help          Display this text`  

Timelim also accepts suffix arguments such as `10h` for 10 hours when used in place of the above options.

## Examples
Sleep for 2 seconds with verbose output (when run as sleep):
`sleep -v 2` or `sleep -v 2s`

Sleep for 4 months, 2 weeks and 2000 nanoseconds (with verbose output, suffixes):
`timelim --verbose 4m 2w 2000n`

Sleep for 7 millennia and a month:
`timelim 7M 1o`

Set a time limit to last a minute: 
`timelim 1m` or `./timelim 60s`

Set a time limit to last one year and three weeks:
`timelim 1y 3w`

Set a time limit to only last 24000 nanoseconds:
`timelim 24000n`

Set a time limit to last 3.6 centuries and 4.1 months:
`timelim 3.6c 4.1o`

Run uname after 27 seconds:
`timelim --run "uname -a" 27s`
