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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Used for converting arguments into unsigned ints
#define OPTARG (unsigned)atoi(optarg)
#define ARGV   (unsigned)atoi(argv[args])

// Muliply seconds by one of these for conversion into minutes, weeks, etc.
#define CINT 3110400000
#define MINT 60
#define HINT 3600
#define DINT 86400
#define WINT 604800
#define OINT 2592000
#define YINT 31104000

// This is used for usage info
extern char *__progname;

// Used for --run
static char *cmd = NULL;

// Display usage of timelim
static void usage(void)
{
	// Usage info
	printf("Usage: %s [-cdhmnorsvwy?] length[suffix] ...\n", __progname);
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

	// Exit with status 1
	exit(1);
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

// Finishes execution of timelim
static void finish(int sig)
{
	// Run command
	if(cmd != NULL)
		execl("/bin/sh", "/bin/sh", "-c", cmd, NULL);

	// Exit
	char *posix = getenv("POSIXLY_CORRECT");
	if((sig == SIGALRM) && (posix == NULL || strncmp(posix, "1", 1) != 0))
		exit(142);
	exit(0);
}

// This function properly parses decimal arguments (such as 1.12 or 4.5w)
static useconds_t decimal(char *arg)
{
	// If there is no decimal, return
	if(strchr(arg, '.') == NULL) return 0;

	// Set a char variable called 'base' to the relevant position
	strsep(&arg, ".");
	const char *base = strsep(&arg, ".");
	size_t sz = strlen(base);
	if(strchr(base, 'm') != NULL || strchr(base, 'h') != NULL || strchr(base, 'd') != NULL || strchr(base, 'w') != NULL || \
		 strchr(base, 'o') != NULL || strchr(base, 'y') != NULL || strchr(base, 'c') != NULL || strchr(base, 's') != NULL)
		--sz;

	// Set the multiplier depending on the length of base
	int multiplier;
	switch(sz) {
		case 1:
			multiplier = 1e5;
			break;
		case 2:
			multiplier = 1e4;
			break;
		case 3:
			multiplier = 1e3;
			break;
		case 4:
			multiplier = 1e2;
			break;
		case 5:
			multiplier = 10;
			break;
		case 6:
			multiplier = 1;
			break;
		default:
			usage();
			break;
	}

	// Return the microsecond value of base
	return (useconds_t)atoi(base) * multiplier;
}

// Main function
void main(int argc, char *argv[])
{
	// Arguments are required
	if(argc < 2)
		usage();

	// Variables
	useconds_t useconds = 0;
	unsigned int centuries, seconds, minutes, hours, days, weeks, months, years;
	centuries = seconds = minutes = hours = days = weeks = months = years = 0;
	int verbose = 1;

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

			// Usage
			case '?':
				usage();
				break;

			// Centuries
			case 'c':
				centuries += OPTARG;
				useconds += decimal(optarg) * CINT;
				break;

			// Days
			case 'd':
				days += OPTARG;
				useconds += decimal(optarg) * DINT;
				break;

			// Hours
			case 'h':
				hours += OPTARG;
				useconds += decimal(optarg) * HINT;
				break;

			// Minutes
			case 'm':
				minutes += OPTARG;
				useconds += decimal(optarg) * MINT;
				break;

			// Microseconds
			case 'n':
				useconds += OPTARG;
				break;

			// Months
			case 'o':
				months += OPTARG;
				useconds += decimal(optarg) * OINT;
				break;

			// Run command on completion
			case 'r':
				cmd = optarg;
				break;

			// Seconds
			case 's':
				seconds += OPTARG;
				useconds += decimal(optarg);
				break;

			case 'v':
				verbose = 0;
				break;

			// Weeks
			case 'w':
				weeks += OPTARG;
				useconds += decimal(optarg) * WINT;
				break;

			// Years
			case 'y':
				years += OPTARG;
				useconds += decimal(optarg) * YINT;
				break;
		}
	}

	// Add up the total number of seconds to wait
	unsigned int total_seconds = years * YINT + months * OINT + weeks * WINT + days * DINT + hours * HINT + minutes * MINT + seconds;

	// Function as sleep(1)
	if((total_seconds == 0) && (centuries == 0) && (useconds == 0)) {
		int args = --argc;
		while(args != 0) {

			// If the argument has a dash, skip it
			if(strchr(argv[args], '-') != NULL) break;

			// Minutes
			if(strchr(argv[args], 'm') != NULL) {
				minutes += ARGV;
				useconds += decimal(argv[args]) * MINT;

			// Hours
			} else if(strchr(argv[args], 'h') != NULL) {
				hours += ARGV;
				useconds += decimal(argv[args]) * HINT;

			// Days
			} else if(strchr(argv[args], 'd') != NULL) {
				days += ARGV;
				useconds += decimal(argv[args]) * DINT;

			// Weeks
			} else if(strchr(argv[args], 'w') != NULL) {
				weeks += ARGV;
				useconds += decimal(argv[args]) * WINT;

			// Months
			} else if(strchr(argv[args], 'o') != NULL) {
				months += ARGV;
				useconds += decimal(argv[args]) * OINT;

			// Microseconds
			} else if(strchr(argv[args], 'n') != NULL)
				useconds += ARGV;

			// Centuries
			else if(strchr(argv[args], 'c') != NULL) {
				centuries += ARGV;
				useconds += decimal(argv[args]) * CINT;

			// Years
			} else if(strchr(argv[args], 'y') != NULL) {
				years += ARGV;
				useconds += decimal(argv[args]) * YINT;

			// Seconds (fallback)
			} else {
				seconds += ARGV;
				useconds += decimal(argv[args]);
			}

			// Subtract 1 from args (for args != 0)
			--args;
		}

		// Re-add total_seconds
		total_seconds = years * YINT + months * OINT + weeks * WINT + days * DINT + hours * HINT + minutes * MINT + seconds;
	}

	// Verbose output
	if(verbose == 0) {

		// Use a long variable to get the full number of actual seconds
		unsigned long true_seconds = total_seconds + centuries * CINT + useconds / 1e6;

		// Print info
		if(true_seconds == 1)
			printf("Waiting for a total of %lu second, consisting of:\n", true_seconds);
		else
			printf("Waiting for a total of %lu seconds, consisting of:\n", true_seconds);
		lprint(centuries, "century",    "centuries");
		lprint(years,     "year",        NULL);
		lprint(months,    "month",       NULL);
		lprint(weeks,     "week",        NULL);
		lprint(days,      "day",         NULL);
		lprint(hours,     "hour",        NULL);
		lprint(minutes,   "minute",      NULL);
		lprint(seconds,   "second",      NULL);
		lprint(useconds,  "microsecond", NULL);
	}

	// Catch SIGALRM
	struct sigaction actor;
	memset(&actor, 0, sizeof(actor));
	actor.sa_handler = finish;
	sigaction(SIGALRM, &actor, NULL);

	// Sleep for total_seconds and useconds
	sleep(total_seconds);
	usleep(useconds);

	// Sleep for multiple centuries (workaround for 32-bit int)
	while(centuries != 0) {
		sleep(CINT);
		--centuries;
	}

	// Notify completion
	if(verbose == 0)
		printf("Time's up!\n");

	// Run finish
	finish(SIGTERM);
}
