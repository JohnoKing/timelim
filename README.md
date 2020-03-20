timelim
========
_A program capable of setting ridiculous time limits_

## What is this
Timelim is a POSIX-compliant implementation of the sleep(1) utility capable of setting time limits as short as a few nanoseconds or as long as multiple centuries.
There is no real point to this program, it is just something to experiment with.

## Building and Usage
Run `make` and the timelim binary will be compiled.

## Usage
`Usage: timelim [-jsvV?] length[suffix] ...`
`  -j, --julian        Use the Julian calendar instead of the Gregorian calendar`
`  -s, --sidereal      Use the Sidereal year instead of the Gregorian year`
`  -v, --verbose       Enable verbose output`
`  -V, --version       Show timelim's version number`
`  -?, --help          Display this text`

## Suffixes
Timelim also accepts suffix arguments such as `10h` for 10 hours when used in place of the above options. Here is a list of supported suffixes:
`n: Nanoseconds`
`s: Seconds`
`m: Minutes`
`h: Hours`
`d: Days`
`w: Weeks`
`f: Fortnights`
`o: Months`
`y: Years`
`D: Decades`
`c: Centuries`
`M: Millenia`

## Examples
Sleep for 2 seconds with verbose output (when run as sleep):
`sleep -v 2` or `sleep -v 2s`

Sleep for 4 months, 1 fortnight, 2 weeks and 2000 nanoseconds (with verbose output, suffixes):
`timelim --verbose 4m 1f 2w 2000n`

Sleep for 7 millennia and a month, using the Julian calendar:
`timelim -j 7M 1o`

Set a time limit to last a minute: 
`timelim 1m` or `./timelim 60s`

Set a time limit to last one sidereal year and three weeks:
`timelim -s 1y 3w`

Set a time limit to only last 24000 nanoseconds:
`timelim 24000n`

Set a time limit to last 3.6 centuries, 2.4 decades and 4.1 months:
`timelim 3.6c 2.4D 4.1o`
