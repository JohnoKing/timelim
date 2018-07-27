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
 * timelim - a program capable of setting ridiculous time limits
 */

#include <getopt.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// This is used for usage info
extern char *__progname;

// Display usage of timelim
static int usage(int ret, const char *msg, ...)
{
	// Print msg
	va_list vargs;
	va_start(vargs, msg);
	vprintf(msg, vargs);
	va_end(vargs);

	// Usage info
	printf("Usage: timelim [-cdhmosvwy?] length ...\n");
	printf("  -c, --centuries     Number of centuries (may break things)\n");
	printf("  -d, --days          Number of days\n");
	printf("  -h, --hours         Number of hours\n");
	printf("  -m, --minutes       Number of minutes\n");
	printf("  -o, --months        Number of months\n");
	printf("  -s, --seconds       Number of seconds\n");
	printf("  -v, --verbose       Display extra info\n");
	printf("  -w, --weeks         Number of weeks\n");
	printf("  -y, --years         Number of years\n");
	printf("  -?, --help          Display this text\n");

	// Return ret
	return ret;
}

// Main function
int main(int argc, char *argv[])
{
	// Arguments are required
	if(argc == 1)
		return usage(1, "%s requires arguments\n", __progname);

	// bool and long variables
	unsigned long seconds   = 0;
	unsigned long minutes   = 0;
	unsigned long hours     = 0;
	unsigned long days      = 0;
	unsigned long weeks     = 0;
	unsigned long months    = 0;
	unsigned long years     = 0;
	unsigned long centuries = 0;
	bool verbose        = false;

	// Long options for getopt_long
	struct option long_opts[] = {
	{ "centuries", required_argument, NULL, 'c' },
	{ "days",      required_argument, NULL, 'd' },
	{ "hours",     required_argument, NULL, 'h' },
	{ "minutes",   required_argument, NULL, 'm' },
	{ "months",    required_argument, NULL, 'o' },
	{ "seconds",   required_argument, NULL, 's' },
	{ "verbose",   required_argument, NULL, 'v' },
	{ "weeks",     required_argument, NULL, 'w' },
	{ "years",     required_argument, NULL, 'y' },
	{ "help",      no_argument,       NULL, '?' },
	};

	// Parse the options
	int args;
	while((args = getopt_long(argc, argv, "c:d:h:m:o:s:vw:y:?", long_opts, NULL)) != -1) {
		switch(args) {

			// Usage (return 0)
			case '?':
				return usage(0, "");

			// Centuries
			case 'c':
				centuries = (unsigned)atol(optarg);
				break;

			// Days
			case 'd':
				days = (unsigned)atol(optarg);
				break;

			// Hours
			case 'h':
				hours = (unsigned)atol(optarg);
				break;

			// Minutes
			case 'm':
				minutes = (unsigned)atol(optarg);
				break;

			// Months
			case 'o':
				months = (unsigned)atol(optarg);
				break;

			// Seconds
			case 's':
				seconds = (unsigned)atol(optarg);
				break;

			// Display extra info
			case 'v':
				verbose = true;
				break;

			// Weeks
			case 'w':
				weeks = (unsigned)atol(optarg);
				break;

			// Years
			case 'y':
				years = (unsigned)atol(optarg);
				break;

			// Fallback to usage (return 1)
			default:
				return usage(1, "");
		}
	}

	// Add up the total number of seconds to wait
	unsigned long total_seconds = centuries * 3110400000 + years * 31104000 + months * 2592000 + weeks * 604800 + days * 86400 + hours * 3600 + minutes * 60 + seconds;

	// For -v
	if(verbose == true) {
		printf("Waiting for:\n");
		printf("    %lu centuries\n", centuries);
		printf("    %lu years\n",     years);
		printf("    %lu months\n",    months);
		printf("    %lu weeks\n",     weeks);
		printf("    %lu days\n",      days);
		printf("    %lu hours\n",     hours);
		printf("    %lu minutes\n",   minutes);
		printf("and %lu seconds\n",   seconds);
	}

	// sleep(3) for total_seconds, then notify on completion
	sleep((unsigned int)total_seconds);
	printf("Time's up!\n");

	// Finish
	if(verbose == true)
		printf("Waited for a total of %lu seconds\n", total_seconds);
	return 0;
}
