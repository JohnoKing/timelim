/*
 * Copyright © 2018-2020 Johnothan King. All rights reserved.
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
 * timelim -- A program capable of setting very long time limits
 * This program can function as a replacement for sleep(1)
 */

#define _GNU_SOURCE // Timelim uses the strcasestr extension
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Timelim's version number
#define TIMELIM_VERSION "v3.0.2"

/*
 * Define the number of nanoseconds wasted during execution to subtract from the total time to sleep
 * This number is rather conservative, most machines will benefit from increasing the OVERHEAD_MASK
 */
#ifndef OVERHEAD_MASK
#define OVERHEAD_MASK 330000
#endif

// Colors
#define CYAN  "\x1b[1;36m"
#define WHITE "\x1b[1;37m"
#define RESET "\x1b[m"

// Macros defining the length of various units of time (weeks, years, etc.) in seconds.
#define MINUTE    60
#define HOUR      3600
#define DAY       86400
#define WEEK      604800
#define FORTNIGHT 1209600

// Universal variables
static int current_signal = 0;
static bool suffix;
extern char *__progname;

// Display usage of Timelim
noreturn static void usage(void)
{
    // Usage info
    printf("Usage: %s [-jsvV?] number[suffix] ...\n"
           "  -j, --julian     Use the Julian calendar instead of the Gregorian calendar\n"
           "  -s, --signal     Sleep until Timelim receives a signal or times out\n"
           "  -S, --sidereal   Use the Sidereal year instead of the Gregorian year\n"
           "  -v, --verbose    Enable verbose output\n"
           "  -V, --version    Show Timelim's version number\n"
           "  -?, --help       Display this text\n", __progname);
    exit(1);
}

// Print the number of seconds and nanoseconds remaining
static void nprint(unsigned long length, const char *unit)
{
    if(length == 1)
        printf("%lu %s", length, unit);
    else
        printf("%lu %ss", length, unit);
}

// This function parses all numbers after the decimal (such as 1.12 or 4.5w)
static long parse_float(char *arg)
{
    // If there is no decimal, return
    if(strchr(arg, '.') == NULL)
        return 0;

    // Set a char variable called 'base' to the relevant position
    strsep(&arg, ".");
    const char *base = strsep(&arg, ".");
    size_t sz = strlen(base);
    if(suffix)
        --sz;

    // Set the multiplier depending on the length of base
    long num = atol(base);
    switch(sz) {
       case 1:
           return num * 100000000;
       case 2:
           return num * 10000000;
       case 3:
           return num * 1000000;
       case 4:
           return num * 100000;
       case 5:
           return num * 10000;
       case 6:
           return num * 1000;
       case 7:
           return num * 100;
       case 8:
           return num * 10;
       case 9:
           return num;
       default:
           while(num > 999999999)
               num = num / 10;
           return num;
    }
}

// Set current_signal to the signal that was sent to Timelim
static void sighandle(int sig)
{
    current_signal = sig;
}

// Main function
int main(int argc, char *argv[])
{
    // Arguments are required
    if(argc < 2)
        usage();

    // General variables
    struct timespec timer = { 0 };
    unsigned long centuries = 0;
    bool signal_wait = false, verbose = false;
    int year = 31556952; // Gregorian year default

    // Long options for getopt_long
    struct option long_opts[] = {
    { "julian",   no_argument, NULL, 'j' },
    { "signal",   no_argument, NULL, 's' },
    { "sidereal", no_argument, NULL, 'S' },
    { "verbose",  no_argument, NULL, 'v' },
    { "version",  no_argument, NULL, 'V' },
    { "help",     no_argument, NULL, '?' },
    {  NULL,                0, NULL,  0  }
    };

    // Parse the options
    int args;
    while((args = getopt_long(argc, argv, "jsSvV?", long_opts, NULL)) != -1)
        switch(args) {

            // Version info
            case 'V':
                printf(WHITE "Timelim " CYAN TIMELIM_VERSION RESET "\n");
                return 0;

            // Use the Julian calendar
            case 'j':
                year = 31557600;
                break;

            // Implement ksh's sleep -s flag
            case 's':
                signal_wait = true;
                break;

            // Use the Sidereal year
            case 'S':
                year = 31558150;
                break;

            // Verbose output
            case 'v':
                verbose = true;
                break;

            // Usage
            case '?':
                usage();
        }

    // Parse suffixes
    int multiplier;
    args = argc - 1;
    while(args != 0) {
        multiplier = 1;
        suffix = true;

        // If the argument has a dash, skip it
        if(strchr(argv[args], '-') != NULL) break;

        // Parse GNU-style suffixes
        if(strcasestr(argv[args],      "M") != NULL) multiplier = MINUTE;    // Minutes
        else if(strcasestr(argv[args], "H") != NULL) multiplier = HOUR;      // Hours
        else if(strcasestr(argv[args], "D") != NULL) multiplier = DAY;       // Days
        else if(strcasestr(argv[args], "W") != NULL) multiplier = WEEK;      // Weeks
        else if(strcasestr(argv[args], "F") != NULL) multiplier = FORTNIGHT; // Fortnights
        else if(strcasestr(argv[args], "O") != NULL) multiplier = year / 12; // Months (different from ISO 8601)
        else if(strcasestr(argv[args], "Y") != NULL) multiplier = year;      // Years
        else if(strcasestr(argv[args], "X") != NULL) multiplier = year * 10; // Decades

        // Milliseconds (different from ISO 8601)
        else if(strcasestr(argv[args], "L") != NULL) {
            timer.tv_nsec += atol(argv[args]) * 1000;
            goto end;

        // Microseconds
        } else if(strcasestr(argv[args], "U") != NULL) {
            timer.tv_nsec += atol(argv[args]) * 1000;
            goto end;

        // Nanoseconds
        } else if(strcasestr(argv[args], "N") != NULL) {
            timer.tv_nsec += atol(argv[args]);
            goto end;

        // Centuries
        } else if(strcasestr(argv[args], "C") != NULL) {
            centuries += strtoul(argv[args], NULL, 10);
            multiplier = year * 100;
            goto nano;

        // Millennia
        } else if(strcasestr(argv[args], "A") != NULL) {
            centuries += strtoul(argv[args], NULL, 10) * 10;
            multiplier = year * 1000;
            goto nano;

        // Normal seconds
        } else
            if(strcasestr(argv[args], "S") == NULL) suffix = false;

        // Set the number of seconds and nanoseconds
        timer.tv_sec  += atoi(argv[args]) * multiplier;
nano:
        timer.tv_nsec += parse_float(argv[args]) * multiplier;

end:
        // Go to the next argument
        --args;
    }

    // To improve accuracy, subtract 330,000 nanoseconds to account for overhead
    if(timer.tv_nsec > OVERHEAD_MASK) {
        if(timer.tv_sec > 0) {
            timer.tv_sec   = timer.tv_sec - 1;
            timer.tv_nsec = timer.tv_nsec + 1000000000 - OVERHEAD_MASK;
        } else
            timer.tv_nsec = timer.tv_nsec - OVERHEAD_MASK;

    // The overhead of just executing causes inaccuracy at this point, so just set this to zero
    } else
        timer.tv_nsec = 0;

    // The number of nanoseconds cannot exceed one billion
    while(timer.tv_nsec > 999999999) {
        time_t esec   = timer.tv_nsec / 1000000000;
        timer.tv_sec += esec;
        timer.tv_nsec = timer.tv_nsec - (esec * 1000000000);
    }

    // Set up the signal handler now
    struct sigaction actor;
    actor.sa_handler = sighandle;
    actor.sa_flags   = 0;

    // When -s was passed, handle all POSIX signals that do not kill Timelim
    if(signal_wait) {
        sigaction(SIGCHLD, &actor, NULL);
        sigaction(SIGCONT, &actor, NULL);
        sigaction(SIGQUIT, &actor, NULL);
        sigaction(SIGTSTP, &actor, NULL);
        sigaction(SIGURG,  &actor, NULL);
    }

    // Handle SIGINFO or SIGPWR, depending on which is available
#ifdef SIGINFO
    sigaction(SIGINFO, &actor, NULL);
#else
    sigaction(SIGPWR,  &actor, NULL);
#endif

    // Wait indefinitely if -s was passed without a defined timeout
    if(signal_wait && timer.tv_sec == 0 && timer.tv_nsec == 0) {
        if(verbose) printf("Waiting for a signal...\n");
        pause();
        return 0; // pause(2) does not return 0, so it must be done separately
    }

    // Print out the number of seconds to sleep
    if(verbose) {
        printf("Sleeping for ");
        nprint(centuries * ((unsigned)year * 100) + (unsigned)timer.tv_sec, "second");
        printf(" and ");
        nprint((unsigned)timer.tv_nsec, "nanosecond");
        printf("\n");
    }

    // Sleep
    while(nanosleep(&timer, &timer) != 0) {
        if(signal_wait || current_signal == SIGALRM) return 0;

        printf("Remaining seconds: %ld\n"
               "Remaining nanoseconds: %ld\n", (long)timer.tv_sec, timer.tv_nsec);
    }

    // Sleep for multiple centuries (workaround for 32-bit int)
    while(centuries != 0) {
        sleep((unsigned)year * 100);
        --centuries;
    }

    // Notify the user on completion
    if(verbose)
        printf("Time's up!\n");
    return 0;
}
