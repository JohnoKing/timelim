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
 * timelim - A program capable of setting very long time limits
 * This program can function as a replacement for sleep(1)
 */

#define _GNU_SOURCE // Timelim uses the strcasestr extension
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Timelim's version number
#define TIMELIM_VERSION "v3.0.0"

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

// Macros defining the length of various units of time (fortnights, years, etc.) in seconds.
#define MINUTE    60
#define HOUR      3600
#define DAY       86400
#define WEEK      604800
#define FORTNIGHT 1209600
#define MONTH     18144000

// Universal variables
static unsigned long centuries = 0;
static int current_signal = 0;
static long nanoseconds = 0;
static int seconds      = 0;
static int year         = 31556952; // This is a universal variable as it can be changed with -S and -j
extern char *__progname;

// Display usage of Timelim
static int usage(void)
{
    // Usage info
    printf("Usage: %s [-jsvV?] number[suffix] ...\n", __progname);
    printf("  -j, --julian     Use the Julian calendar instead of the Gregorian calendar\n");
    printf("  -s, --signal     Sleep until Timelim receives a signal or times out\n");
    printf("  -S, --sidereal   Use the Sidereal year instead of the Gregorian year\n");
    printf("  -v, --verbose    Enable verbose output\n");
    printf("  -V, --version    Show Timelim's version number\n");
    printf("  -?, --help       Display this text\n");
    return 1;
}

// Print the number of seconds and nanoseconds remaining
static void nprint(unsigned long length, const char *unit)
{
    printf("%lu %s", length, unit);
    if(length != 1)
        printf("s");
}

// This function parses all numbers after the decimal (such as 1.12 or 4.5w)
static long parse_decimal(char *arg)
{
    // If there is no decimal, return
    if(strchr(arg, '.') == NULL)
        return 0;

    // Set a char variable called 'base' to the relevant position
    strsep(&arg, ".");
    const char *base = strsep(&arg, ".");
    size_t sz = strlen(base);
    if(strchr(base, 'm') != NULL || strchr(base, 'h') != NULL || strchr(base, 'd') != NULL || strchr(base, 'w') != NULL || \
        strchr(base, 'o') != NULL || strchr(base, 'y') != NULL || strchr(base, 'c') != NULL || strchr(base, 's') != NULL || \
        strchr(base, 'D') != NULL || strchr(base, 'f') != NULL || strchr(base, 'M') != NULL)
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

// Get the duration of an argument (attempting to use strcasestr will cause memory errors)
static long get_duration(const char *arg, char duration)
{
    // If the desired duration is not in the string, return 0
    if(strchr(arg, duration) == NULL)
        return 0;

    // Get the number
    unsigned int index = 0;
    char *modarg = malloc(strlen(arg));
    while(arg[index] != duration) {
        modarg[index] = arg[index + 1];
        index++;
    }

    // Free memory of modarg and return the long number
    long result = atol(modarg);
    free(modarg);
    return result;
}

// Strict ISO 8061 string parsing (does not support decades or millennia)
static void parse_iso(char *arg, unsigned int mode)
{
    // Parse P arguments
    if(mode == 0) {
        centuries += get_duration(arg, 'C');
        centuries += get_duration(arg, 'c');
        seconds   += get_duration(arg, 'Y') * year;
        seconds   += get_duration(arg, 'y') * year;
        seconds   += get_duration(arg, 'M') * MONTH;
        seconds   += get_duration(arg, 'm') * MONTH;
        seconds   += get_duration(arg, 'F') * FORTNIGHT;
        seconds   += get_duration(arg, 'f') * FORTNIGHT;
        seconds   += get_duration(arg, 'W') * WEEK;
        seconds   += get_duration(arg, 'w') * WEEK;
        seconds   += get_duration(arg, 'D') * DAY;
        seconds   += get_duration(arg, 'd') * DAY;
    }

    // TODO: Parse T arguments
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
        return usage();

    // General variables
    unsigned int signal_wait = 1;
    unsigned int verbose     = 1;
    struct timespec timer    = {0};

    // Long options for getopt_long
    struct option long_opts[] = {
    { "julian",   no_argument, NULL, 'j' },
    { "signal",   no_argument, NULL, 's' },
    { "sidereal", no_argument, NULL, 'S' },
    { "verbose",  no_argument, NULL, 'v' },
    { "version",  no_argument, NULL, 'V' },
    { "help",     no_argument, NULL, '?' },
    {  0,                   0,    0,  0  }
    };

    // Parse the options
    int args;
    while((args = getopt_long(argc, argv, "jsSvV?", long_opts, NULL)) != -1) {
        switch(args) {

            // Usage
            case '?':
                return usage();

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
                signal_wait = 0;
                break;

            // Use the Sidereal year
            case 'S':
                year = 31558150;
                break;

            // Verbose output
            case 'v':
                verbose = 0;
                break;
        }
    }

    // Parse suffixes
    args = argc - 1;
    while(args != 0) {
        int multiplier = 1;

        // If the argument has a dash, skip it
        if(strchr(argv[args], '-') != NULL) break;

        // Parse ISO 8601 arguments in a dedicated function
        if(strcasestr(argv[args], "P") != NULL) {
            parse_iso(argv[args], 0);
            goto end;
        } else if(strcasestr(argv[args], "T") != NULL) {
            parse_iso(argv[args], 1);
            goto end;
        }

        // GNU suffix parsing
        if(strchr(argv[args],      'm') != NULL) multiplier = MINUTE;    // Minutes
        else if(strchr(argv[args], 'h') != NULL) multiplier = HOUR;      // Hours
        else if(strchr(argv[args], 'd') != NULL) multiplier = DAY;       // Days
        else if(strchr(argv[args], 'w') != NULL) multiplier = WEEK;      // Weeks
        else if(strchr(argv[args], 'f') != NULL) multiplier = FORTNIGHT; // Fortnights
        else if(strchr(argv[args], 'o') != NULL) multiplier = MONTH;     // Months
        else if(strchr(argv[args], 'y') != NULL) multiplier = year;      // Years
        else if(strchr(argv[args], 'D') != NULL) multiplier = year * 10; // Decades

        // Nanoseconds
        else if(strchr(argv[args], 'n') != NULL) {
            nanoseconds += atol(argv[args]);
            goto end;

        // Centuries
        } else if(strchr(argv[args], 'c') != NULL) {
            centuries += strtoul(argv[args], NULL, 10);
            multiplier = year * 100;
            goto nano;

        // Millennia
        } else if(strchr(argv[args], 'M') != NULL) {
            centuries += strtoul(argv[args], NULL, 10) * 10;
            multiplier = year * 1000;
            goto nano;
        }

        // Set the number of floating point seconds and nanoseconds
        seconds += atoi(argv[args]) * multiplier;
nano:
        nanoseconds += parse_decimal(argv[args]) * multiplier;

end:
        // Go to the next argument
        --args;
    }

    // Store the total number of seconds and nanoseconds in the timer struct
    timer.tv_sec  = seconds;
    timer.tv_nsec = nanoseconds;

    // To improve accuracy, subtract 490,000 nanoseconds to account for overhead
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
    if(signal_wait == 0) {
        sigaction(SIGCHLD, &actor, NULL);
        sigaction(SIGCONT, &actor, NULL);
        sigaction(SIGQUIT, &actor, NULL);
        sigaction(SIGTSTP, &actor, NULL);
        sigaction(SIGURG,  &actor, NULL);
    }

    // Handle SIGINFO on BSD and treat SIGPWR as SIGINFO due to the lack of a System V equivalent
#   ifdef SIGINFO
    sigaction(SIGINFO, &actor, NULL);
#   else
    sigaction(SIGPWR,  &actor, NULL);
#   endif

    // Wait indefinitely if -s was passed without a defined timeout
    if((signal_wait == 0) && (timer.tv_sec == 0) && (timer.tv_nsec == 0)) {
        if(verbose == 0) printf("Waiting for a signal...\n");
        pause();
        return 0;
    }

    // Print out the number of seconds to sleep
    if(verbose == 0) {
        printf("Sleeping for ");
        nprint(centuries * ((unsigned)year * 100) + (unsigned)timer.tv_sec, "second");
        printf(" and ");
        nprint((unsigned)timer.tv_nsec, "nanosecond");
        printf("\n");
    }

    // Sleep
    while(nanosleep(&timer, &timer) != 0) {
        if(signal_wait == 0) return 0;

        printf("Remaining seconds: %ld\n", (long)timer.tv_sec);
        printf("Remaining nanoseconds: %ld\n", timer.tv_nsec);
    }

    // Sleep for multiple centuries (workaround for 32-bit int)
    while(centuries != 0) {
        sleep((unsigned)year * 100);
        --centuries;
    }

    // Notify the user on completion
    if(verbose == 0)
        printf("Time's up!\n");
    return 0;
}
