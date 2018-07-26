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
 * timelim - wait for the specified period of time
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
	printf("Usage: timelim [-hmsv?] length ...\n");
	printf("  -h, --hours         Number of hours\n");
	printf("  -m, --minutes       Number of minutes\n");
	printf("  -s, --seconds       Number of seconds\n");
	printf("  -v, --verbose       Display extra info\n");
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

	// bool and int variables
	unsigned int seconds = 0;
	unsigned int minutes = 0;
	unsigned int hours   = 0;
	bool verbose = false;

	// Long options for getopt_long
	struct option long_opts[] = {
	{ "hours",   required_argument, NULL, 'h' },
	{ "minutes", required_argument, NULL, 'm' },
	{ "seconds", required_argument, NULL, 's' },
	{ "verbose", required_argument, NULL, 'v' },
	{ "help",    no_argument,       NULL, '?' },
	};

	// Parse the options
	int args;
	while((args = getopt_long(argc, argv, "h:m:s:?", long_opts, NULL)) != -1) {
		switch(args) {

			// Hours
			case 'h':
				hours = atoi(optarg);
				break;

			// Minutes
			case 'm':
				minutes = atoi(optarg);
				break;

			// Seconds
			case 's':
				seconds = atoi(optarg);
				break;

			// Display extra info
			case 'v':
				verbose = true;
				break;

			// Usage (return 0)
			case '?':
				return usage(0, "");

			// Fallback to usage (return 1)
			default:
				return usage(1, "");
		}
	}

	// Add up the total number of seconds to wait
	unsigned int total_seconds = hours * 3600 + minutes * 60 + seconds;

	// For -v
	if(verbose == true)
		printf("Waiting for %u hours %u minutes and %u seconds\n", hours, minutes, seconds);

	// sleep(3) for total_seconds, then notify on completion
	sleep(total_seconds);
	printf("Time's up!\n");

	// Finish
	if(verbose == true)
		printf("Waited for a total of %u seconds\n", total_seconds);
	return 0;
}
