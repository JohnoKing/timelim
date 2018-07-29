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
	printf("Usage: timelim [-cdhmorsvwy?] length ...\n");
	printf("  -c, --century       Add a century to the time limit (cannot exceed one century)\n");
	printf("  -d, --days          Number of days\n");
	printf("  -h, --hours         Number of hours\n");
	printf("  -m, --minutes       Number of minutes\n");
	printf("  -o, --months        Number of months\n");
	printf("  -r, --run           Run the specified command when the time runs out\n");
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

	// bool, char* and long variables
	unsigned int seconds   = 0;
	unsigned int minutes   = 0;
	unsigned int hours     = 0;
	unsigned int days      = 0;
	unsigned int weeks     = 0;
	unsigned int months    = 0;
	unsigned int years     = 0;
	unsigned int century   = 0;
	bool verbose       = false;
	char *cmd          =  NULL;

	// Long options for getopt_long
	struct option long_opts[] = {
	{ "century",   no_argument,       NULL, 'c' },
	{ "days",      required_argument, NULL, 'd' },
	{ "hours",     required_argument, NULL, 'h' },
	{ "minutes",   required_argument, NULL, 'm' },
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
	while((args = getopt_long(argc, argv, "cd:h:m:o:r:s:vw:y:?", long_opts, NULL)) != -1) {
		switch(args) {

			// Usage (return 0)
			case '?':
				return usage(0, "");

			// Centuries
			case 'c':
				century = 1;
				break;

			// Days
			case 'd':
				days = (unsigned)atoi(optarg);
				break;

			// Hours
			case 'h':
				hours = (unsigned)atoi(optarg);
				break;

			// Minutes
			case 'm':
				minutes = (unsigned)atoi(optarg);
				break;

			// Months
			case 'o':
				months = (unsigned)atoi(optarg);
				break;

			// Run command on completion
			case 'r':
				cmd = optarg;
				break;

			// Seconds
			case 's':
				seconds = (unsigned)atoi(optarg);
				break;

			// Display extra info
			case 'v':
				verbose = true;
				break;

			// Weeks
			case 'w':
				weeks = (unsigned)atoi(optarg);
				break;

			// Years
			case 'y':
				years = (unsigned)atoi(optarg);
				break;

			// Fallback to usage (return 1)
			default:
				return usage(1, "");
		}
	}

	// Add up the total number of seconds to wait
	unsigned int total_seconds = century * 3110400000 + years * 31104000 + months * 2592000 + weeks * 604800 + days * 86400 + hours * 3600 + minutes * 60 + seconds;

	// For -v
	if(verbose == true) {
		printf("Waiting for a total of %u seconds, consisting of:\n", total_seconds);
		printf("    %u century\n",   century);
		printf("    %u years\n",     years);
		printf("    %u months\n",    months);
		printf("    %u weeks\n",     weeks);
		printf("    %u days\n",      days);
		printf("    %u hours\n",     hours);
		printf("    %u minutes\n",   minutes);
		printf("and %u seconds\n",   seconds);
	}

	// sleep(3) for total_seconds, then notify on completion
	sleep(total_seconds);
	printf("Time's up!\n");

	// Finish
	if(cmd != NULL)
		return system(cmd);
	return 0;
}
