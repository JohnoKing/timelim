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
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// This is used for usage info
extern char *__progname;

// Used for accurate output
static int iand = 0;

// Display usage of timelim
static int usage(void)
{
	// Usage info
	printf("Usage: %s [-cdhmnorsvwy?] length ...\n", __progname);
	printf("  -c, --century       Add one century to the time limit\n");
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
static void lprint(unsigned int length, const char *length_c)
{
	if(length != 0) {
		printf("    %u %s\n", length, length_c);
		iand++;
	}
}

// Main function
int main(int argc, char *argv[])
{
	// Arguments are required
	if(argc == 1)
		return usage();

	// Variables
	bool verbose          =  false;
	char *cmd             =  NULL;
	char *useconds_c      = "microseconds";
	char *seconds_c       = "seconds";
	char *minutes_c       = "minutes";
	char *hours_c         = "hours";
	char *days_c          = "days";
	char *weeks_c         = "weeks";
	char *months_c        = "months";
	char *years_c         = "years";
	char *century_c       = "centurys";
	useconds_t   useconds = 0;
	unsigned int seconds  = 0;
	unsigned int minutes  = 0;
	unsigned int hours    = 0;
	unsigned int days     = 0;
	unsigned int weeks    = 0;
	unsigned int months   = 0;
	unsigned int years    = 0;
	unsigned int century  = 0;

	// Long options for getopt_long
	struct option long_opts[] = {
	{ "century",  no_argument,       NULL, 'c' },
	{ "days",     required_argument, NULL, 'd' },
	{ "hours",    required_argument, NULL, 'h' },
	{ "minutes",  required_argument, NULL, 'm' },
	{ "micro",    required_argument, NULL, 'n' },
	{ "months",   required_argument, NULL, 'o' },
	{ "run",      required_argument, NULL, 'r' },
	{ "seconds",  required_argument, NULL, 's' },
	{ "verbose",  no_argument,       NULL, 'v' },
	{ "weeks",    required_argument, NULL, 'w' },
	{ "years",    required_argument, NULL, 'y' },
	{ "help",     no_argument,       NULL, '?' },
	};

	// Parse the options
	int args;
	while((args = getopt_long(argc, argv, "cd:h:m:n:o:r:s:vw:y:?", long_opts, NULL)) != -1) {
		switch(args) {

			// Usage (return 0)
			case '?':
				return usage();

			// Centuries
			case 'c':
				century = 1;
				century_c = "century";
				break;

			// Days
			case 'd':
				days = (unsigned)atoi(optarg);
				if(days == 1)
					days_c = "day";
				break;

			// Hours
			case 'h':
				hours = (unsigned)atoi(optarg);
				if(hours == 1)
					hours_c = "hour";
				break;

			// Minutes
			case 'm':
				minutes = (unsigned)atoi(optarg);
				if(days == 1)
					minutes_c = "minute";
				break;

			// Microseconds
			case 'n':
				useconds = (useconds_t)atoi(optarg);
				if(useconds == 1)
					useconds_c = "microsecond";
				break;

			// Months
			case 'o':
				months = (unsigned)atoi(optarg);
				if(months == 1)
					months_c = "month";
				break;

			// Run command on completion
			case 'r':
				cmd = optarg;
				break;

			// Seconds
			case 's':
				seconds = (unsigned)atoi(optarg);
				if(seconds == 1)
					seconds_c = "second";
				break;

			case 'v':
				verbose = true;
				break;

			// Weeks
			case 'w':
				weeks = (unsigned)atoi(optarg);
				if(weeks == 1)
					weeks_c = "week";
				break;

			// Years
			case 'y':
				years = (unsigned)atoi(optarg);
				if(years == 1)
					years_c = "year";
				break;
		}
	}

	// Add up the total number of seconds to wait
	unsigned int total_seconds = century * 3110400000 + years * 31104000 + months * 2592000 + weeks * 604800 + days * 86400 + hours * 3600 + minutes * 60 + seconds;

	// Function as sleep(1)
	if(total_seconds == 0) {
		if(argc == 2)
			total_seconds = (unsigned)atoi(argv[1]);
		else
			total_seconds = (unsigned)atoi(argv[2]);

		seconds = total_seconds;
	}

	if(total_seconds == 1)
		seconds_c = "second";

	// Verbose output
	if(verbose == true) {
		printf("Waiting for a total of %u %s, consisting of:\n", total_seconds, seconds_c);
		lprint(century,  century_c);
		lprint(years,    years_c);
		lprint(months,   months_c);
		lprint(weeks,    weeks_c);
		lprint(days,     days_c);
		lprint(hours,    hours_c);
		lprint(minutes,  minutes_c);
		lprint(seconds,  seconds_c);
		lprint(useconds, useconds_c);
	}

	// Sleep for total_seconds and useconds
	sleep(total_seconds);
	if(useconds != 0)
		usleep(useconds);

	// Notify completion
	if(verbose == true)
		printf("Time's up!\n");

	// Run command
	if(cmd != NULL)
		return system(cmd);

	// Exit
	return 0;
}
