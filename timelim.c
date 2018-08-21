/*
 * Copyright (c) 2018 Johnothan King. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * timelim - A program capable of setting very long time limits
 *  This program can function as a replacement for sleep(1)
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Used for converting arguments into unsigned ints
#define OPTARG (unsigned)atoi(optarg)
#define ARGV   (unsigned)atoi(argv[args])

// This is used for usage info
extern char *__progname;

// Display usage of timelim
static int usage(void)
{
	// Usage info
	printf("Usage: %s [-cdhmnorsvwy?] length ...\n", __progname);
	printf("  -c, --centuries     Number of centuries\n");
	printf("  -d, --days          Number of days\n");
	printf("  -h, --hours         Number of hours\n");
	printf("  -m, --minutes       Number of minutes\n");
	printf("  -n, --micro         Number of microseconds\n");
	printf("  -o, --months        Number of months\n");
	printf("  -r, --run           Run the specified command when the time runs out\n");
	printf("  -s, --seconds       Number of seconds\n");
	printf("  -v, --verbose       Enable verbose output\n");
	printf("  -w, --weeks         Number of weeks\n");
	printf("  -y, --years         Number of years\n");
	printf("  -?, --help          Display this text\n");

	// Return 1
	return 1;
}

// length cannot be 0, or else lprint doesn't occur
static void lprint(unsigned int length, const char *length_c, const char *length_custom)
{
	if(length == 0) return;
	if(length == 1)
		printf("    %u %s\n", length, length_c);
	else if(length_custom != NULL)
		printf("    %u %s\n", length, length_custom);
	else
		printf("    %u %ss\n", length, length_c);
}

// Main function
int main(int argc, char *argv[])
{
	// Arguments are required
	if(argc == 1)
		return usage();

	// Variables
	char *cmd              =  NULL;
	int verbose            = 1;
	useconds_t   useconds  = 0;
	unsigned int centuries = 0;
	unsigned int seconds   = 0;
	unsigned int minutes   = 0;
	unsigned int hours     = 0;
	unsigned int days      = 0;
	unsigned int weeks     = 0;
	unsigned int months    = 0;
	unsigned int years     = 0;

	// Long options for getopt_long
	struct option long_opts[] = {
	{ "centuries", required_argument, NULL, 'c' },
	{ "days",      required_argument, NULL, 'd' },
	{ "hours",     required_argument, NULL, 'h' },
	{ "minutes",   required_argument, NULL, 'm' },
	{ "micro",     required_argument, NULL, 'n' },
	{ "months",    required_argument, NULL, 'o' },
	{ "run",       required_argument, NULL, 'r' },
	{ "seconds",   required_argument, NULL, 's' },
	{ "verbose",   no_argument,       NULL, 'v' },
	{ "weeks",     required_argument, NULL, 'w' },
	{ "years",     required_argument, NULL, 'y' },
	{ "help",      no_argument,       NULL, '?' },
	};

	// Parse the options
	int args;
	while((args = getopt_long(argc, argv, "c:d:h:m:n:o:r:s:vw:y:?", long_opts, NULL)) != -1) {
		switch(args) {

			// Usage (return 0)
			case '?':
				return usage();

			// Centuries
			case 'c':
				centuries = OPTARG;
				break;

			// Days
			case 'd':
				days = OPTARG;
				break;

			// Hours
			case 'h':
				hours = OPTARG;
				break;

			// Minutes
			case 'm':
				minutes = OPTARG;
				break;

			// Microseconds
			case 'n':
				useconds = OPTARG;
				break;

			// Months
			case 'o':
				months = OPTARG;
				break;

			// Run command on completion
			case 'r':
				cmd = optarg;
				break;

			// Seconds
			case 's':
				seconds = OPTARG;
				break;

			case 'v':
				verbose = 0;
				break;

			// Weeks
			case 'w':
				weeks = OPTARG;
				break;

			// Years
			case 'y':
				years = OPTARG;
				break;
		}
	}

	// Add up the total number of seconds to wait
	unsigned int total_seconds = years * 31104000 + months * 2592000 + weeks * 604800 + days * 86400 + hours * 3600 + minutes * 60 + seconds;

	// Function as sleep(1)
	if((total_seconds == 0) && (centuries == 0) && (useconds == 0)) {
		int args = --argc;
		while(args != 0) {
			if(strchr(argv[args], '-') != NULL) break;
			if(strchr(argv[args], 'm') != NULL)
				minutes = ARGV;
			else if(strchr(argv[args], 'h') != NULL)
				hours = ARGV;
			else if(strchr(argv[args], 'd') != NULL)
				days = ARGV;
			else if(strchr(argv[args], 'w') != NULL)
				weeks = ARGV;
			else if(strchr(argv[args], 'o') != NULL)
				months = ARGV;
			else if(strchr(argv[args], 'n') != NULL)
				useconds = ARGV;
			else if(strchr(argv[args], 'c') != NULL)
				centuries = ARGV;
			else if(strchr(argv[args], 'y') != NULL)
				years = ARGV;
			else
				seconds = ARGV;

			--args;
		}

		total_seconds = years * 31104000 + months * 2592000 + weeks * 604800 + days * 86400 + hours * 3600 + minutes * 60 + seconds;
	}

	// Verbose output
	if(verbose == 0) {

		// Variables
		char tseconds_c[8];
		if(total_seconds == 1)
			memcpy(tseconds_c, "second", 7);
		else
			memcpy(tseconds_c, "seconds", 8);
		char *centuries_c = "century";
		char *useconds_c  = "microsecond";
		char *seconds_c   = "second";
		char *minutes_c   = "minute";
		char *hours_c     = "hour";
		char *days_c      = "day";
		char *weeks_c     = "week";
		char *months_c    = "month";
		char *years_c     = "year";

		// Print
		unsigned long true_seconds = total_seconds + centuries * 3110400000;
		printf("Waiting for a total of %lu %s, consisting of:\n", true_seconds, tseconds_c);
		lprint(centuries, centuries_c, "centuries");
		lprint(years,    years_c,    NULL);
		lprint(months,   months_c,   NULL);
		lprint(weeks,    weeks_c,    NULL);
		lprint(days,     days_c,     NULL);
		lprint(hours,    hours_c,    NULL);
		lprint(minutes,  minutes_c,  NULL);
		lprint(seconds,  seconds_c,  NULL);
		lprint(useconds, useconds_c, NULL);
	}

	// Sleep for total_seconds and useconds
	sleep(total_seconds);
	usleep(useconds);

	// Sleep for multiple centuries (workaround for 32-bit int)
	while(centuries != 0) {
		sleep(3110400000);
		--centuries;
	}

	// Notify completion
	if(verbose == 0)
		printf("Time's up!\n");

	// Run command
	if(cmd != NULL)
		return execl("/bin/sh", "/bin/sh", "-c", cmd, (char*)0);

	// Exit
	return 0;
}
