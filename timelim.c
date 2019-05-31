/*
 * Copyright (c) 2018-2019 Johnothan King. All rights reserved.
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
#include <time.h>
#include <unistd.h>

// Timelim's version number
#define TIMELIM_VERSION "1.1.0"

// Colors
#define CYAN  "\x1b[1;36m"
#define WHITE "\x1b[1;37m"
#define RESET "\x1b[m"

// Variables
static int current_signal = 0;
static char *cmd = NULL;
extern char *__progname;

// Display usage of timelim
static int usage(void)
{
	// Usage info
	printf("Usage: %s [-rsvV?] length[suffix] ...\n", __progname);
	printf("  -r, --run           Run the specified command when the time runs out\n");
	printf("  -s, --sidereal      Use sidereal units of time\n");
	printf("  -v, --verbose       Enable verbose output\n");
	printf("  -V, --version       Show timelim's version number\n");
	printf("  -?, --help          Display this text\n");
	return 1;
}

// Print seconds and nanoseconds remaining
static void lprint(unsigned long length, const char *unit)
{
	printf("%lu %s", length, unit);
	if(length != 1)
		printf("s");
}

// This function properly parses decimal arguments (such as 1.12 or 4.5w)
static long decimal(char *arg)
{
	// If there is no decimal, return
	if(strchr(arg, '.') == NULL) return 0;

	// Set a char variable called 'base' to the relevant position
	strsep(&arg, ".");
	const char *base = strsep(&arg, ".");

	// Convert base into a number that is the proper length
	long num = atol(base);
	if(num < 1e9)
		num = num * 1e8;
	while(num > 999999999)
		num = num / 10;

	// Return the number
	return num;
}

// Set current_signal to the signal that was sent to timelim
static void sighandle(int sig)
{
	current_signal = sig;
	return;
}

// Main function
int main(int argc, char *argv[])
{
	// Arguments are required
	if(argc < 2)
		return usage();

	// Variables defining the length of various units of time (fortnights, years, etc.) in seconds
	int minute    = 60;
	int hour      = 3600;
	int day       = 86400;
	int week      = 604800;
	int fortnight = 1209600;
	int month     = 2629800;
	int year      = 31557600;

	// General variables
	unsigned long centuries = 0;
	unsigned int  verbose   = 1;
	struct timespec time    = {0};

	// Long options for getopt_long
	struct option long_opts[] = {
	{ "run",      required_argument, NULL, 'r' },
	{ "sidereal", no_argument, NULL, 's' },
	{ "verbose",  no_argument, NULL, 'v' },
	{ "version",  no_argument, NULL, 'V' },
	{ "help",     no_argument, NULL, '?' },
	};

	// Parse the options
	int args;
	while((args = getopt_long(argc, argv, "r:svV?", long_opts, NULL)) != -1) {
		switch(args) {

			// Usage
			case '?':
				return usage();

			// Version info
			case 'V':
				printf(WHITE "Timelim version " CYAN TIMELIM_VERSION RESET "\n");
				return 0;

			// Run a command on completion
			case 'r':
				cmd = optarg;
				break;

			// Use sidereal measurements
			case 's':
				year = 31558150;
				break;

			// Verbose output
			case 'v':
				verbose = 0;
				break;
		}
	}

	// Function as sleep(1)
	args = argc - 1;
	while(args != 0) {
		int multiplier = 1;

		// If the argument has a dash, skip it
		if(strchr(argv[args], '-') != NULL) break;

		if(strchr(argv[args],      'm') != NULL)  multiplier = minute;    // Minutes
		else if(strchr(argv[args], 'h') != NULL)  multiplier = hour;      // Hours
		else if(strchr(argv[args], 'd') != NULL)  multiplier = day;       // Days
		else if(strchr(argv[args], 'w') != NULL)  multiplier = week;      // Weeks
		else if(strchr(argv[args], 'f') != NULL)  multiplier = fortnight; // Fortnights
		else if(strchr(argv[args], 'o') != NULL)  multiplier = month;     // Months
		else if(strchr(argv[args], 'y') != NULL)  multiplier = year;      // Years
		else if(strchr(argv[args], 'D') != NULL)  multiplier = year * 10; // Decades

		// Microseconds
		else if(strchr(argv[args], 'n') != NULL) {
			time.tv_nsec += atol(argv[args]);
			goto end;

		// Centuries
		} else if(strchr(argv[args], 'c') != NULL) {
			centuries    += strtoul(argv[args], NULL, 10);
			multiplier = year * 100;
			goto tv_nsec_end;

		// Millennia
		} else if(strchr(argv[args], 'M') != NULL) {
			centuries    += strtoul(argv[args], NULL, 10) * 10;
			multiplier = year * 1e3;
			goto tv_nsec_end;
		}

		// Set tv_sec if goto wasn't used
		time.tv_sec  += atoi(argv[args])    * multiplier;

tv_nsec_end:
		// Set tv_nsec
		time.tv_nsec += decimal(argv[args]) * multiplier;

end:
		// Subtract 1 from args (for args != 0)
		--args;
	}

	// time.tv_nsec cannot exceed one billion
	while(time.tv_nsec > 999999999) {
		time_t esec  = time.tv_nsec / 1e9;
		time.tv_sec += esec;
		time.tv_nsec = time.tv_nsec - (esec * 1e9);
	}

	// Verbose output
	if(verbose == 0) {
		printf("Sleeping for ");
		lprint(centuries * (year * 100) + time.tv_sec, "second");
		printf(" and ");
		lprint(time.tv_nsec, "nanosecond");
		printf("\n");
	}

	// Catch SIGINFO, SIGPWR and SIGALRM
	struct sigaction actor;
	memset(&actor, 0, sizeof(actor));
	actor.sa_handler = sighandle;
	sigaction(SIGALRM, &actor, NULL);
	sigaction(SIGPWR,  &actor, NULL);
#	ifdef SIGINFO
	sigaction(SIGINFO, &actor, NULL);
#	endif

	// Sleep
	while(nanosleep(&time, &time) != 0) {
		if(current_signal == SIGALRM) break;
		printf("Remaining seconds: %ld\n", (long)time.tv_sec);
		printf("Remaining nanoseconds: %ld\n", time.tv_nsec);
	}

	// Sleep for multiple centuries (workaround for 32-bit int)
	while(centuries != 0) {
		sleep(year * 100);
		--centuries;
	}

	// Notify completion
	if(verbose == 0)
		printf("Time's up!\n");

	// Run command
	if(cmd != NULL)
		return execl("/bin/sh", "/bin/sh", "-c", cmd, NULL);

	return 0;
}
