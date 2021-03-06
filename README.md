timelim
========
_A program capable of setting ridiculous time limits_

## What is this
Timelim is an implementation of the Unix `sleep` utility capable of setting time limits as short as a few nanoseconds or as long as multiple centuries.
There is no real point to this program, it is just something to experiment with.

## Building and Usage
Run `make` and the timelim binary will be compiled.

## Usage
```
Usage: timelim [-jsSvV?] number[suffix] ...
  -j, --julian     Use the Julian calendar instead of the Gregorian calendar
  -s, --signal     Sleep until Timelim receives a signal or times out
  -S, --sidereal   Use the Sidereal year instead of the Gregorian year
  -v, --verbose    Enable verbose output
  -V, --version    Show Timelim's version number
  -?, --help       Display this text
```

## Suffixes
Timelim also accepts case insensitive suffix arguments such as `10h` for 10 hours. These are all of the supported suffixes:  
```
n: Nanoseconds
u: Microseconds
l: Milliseconds
s: Seconds
m: Minutes
h: Hours
d: Days
w: Weeks
f: Fortnights
o: Months
y: Years
x: Decades
c: Centuries
a: Millennia
```

## Examples
Sleep for 2 seconds with verbose output (when run as sleep):
`sleep -v 2` or `sleep -v 2s`

Sleep for 4 months, 1 fortnight, 2 weeks and 2000 nanoseconds (with verbose output, suffixes):
`timelim --verbose 4m 1f 2w 2000n`

Sleep for 7 millennia and a month, using the Julian calendar:
`timelim -j 7a 1o`

Set a time limit to last a minute unless a signal is received:
`timelim -s 1m` or `./timelim -s 60s`

Set a time limit to last one sidereal year and three weeks:
`timelim -S 1y 3w`

Set a time limit to 740000 nanoseconds:
`timelim 740000n`

Set the time limit to 3.6 centuries, 2.4 decades and 4.1 months:
`timelim 3.6c 2.4x 4.1o`
