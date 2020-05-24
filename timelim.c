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
#define TIMELIM_VERSION "v3.1.0"

// Macros for compiler optimization
#define likely(x) (__builtin_expect((x), 1))
#define unlikely(x) (__builtin_expect((x), 0))

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
extern char *__progname;

// Display usage of Timelim
static noreturn void usage(void)
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
    if unlikely(length == 1) // While `sleep 1` is common, `sleep -v 1` is not
        printf("%lu %s", length, unit);
    else
        printf("%lu %ss", length, unit);
}

// This function parses all numbers after the decimal (such as 1.12 or 4.5w)
static long parse_float(char *arg, bool suffix)
{
    // If there is no decimal, return
    if(!strchr(arg, '.'))
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
    if unlikely(argc < 2) {
        usage();
        __builtin_unreachable();
    }

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

            // Usage (with ksh93u+ compatibility)
            case ':':
            case '?':
                usage();
                __builtin_unreachable();
        }

    // Parse suffixes
    int multiplier;
    bool suffix;
    args = argc - 1;
    while(args != 0) {
        multiplier = 1;
        suffix = true;

        // If the argument has a dash, skip it
        if(strchr(argv[args], '-') != NULL)
            goto end;

        // GNU suffix parsing with partial compatibility for ksh93u+ behavior
        switch(argv[args][strlen(argv[args]) - 1]) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                suffix = false;
                // FALLTHRU
            case 'S':
            case 's':
                break; // Do nothing
            case 'M':
            case 'm':
                multiplier = MINUTE;
                break;
            case 'H':
            case 'h':
                multiplier = HOUR;
                break;
            case 'D':
            case 'd':
                multiplier = DAY;
                break;
            case 'W':
            case 'w':
                multiplier = WEEK;
                break;
            case 'F':
            case 'f':
                multiplier = FORTNIGHT;
                break;
            case 'O':
            case 'o':
                multiplier = year / 12; // Different from ISO 8601
                break;
            case 'Y':
            case 'y':
                multiplier = year;
                break;
            case 'X':
            case 'x':
                multiplier = year * 10;
                break;
            case 'L':
            case 'l':
                timer.tv_nsec += atol(argv[args]) * 1000000;
                goto end;
            case 'U':
            case 'u':
                timer.tv_nsec += atol(argv[args]) * 1000;
                goto end;
            case 'N':
            case 'n':
                timer.tv_nsec += atol(argv[args]);
                goto end;
            case 'C':
            case 'c':
                centuries += strtoul(argv[args], NULL, 10);
                multiplier = year * 100;
                goto nano;
            case 'A':
            case 'a':
                centuries += strtoul(argv[args], NULL, 10) * 10;
                multiplier = year * 1000;
                goto nano;
            default: // Reject invalid arguments
                printf("The suffix '%c' is invalid!\n", argv[args][strlen(argv[args]) - 1]);
                return 1;
        }

        // Set the number of seconds and nanoseconds
        timer.tv_sec  += atoi(argv[args]) * multiplier;
nano:
        timer.tv_nsec += parse_float(argv[args], suffix) * multiplier;

end:
        // Go to the next argument
        --args;
    }

    // To improve accuracy, subtract 330,000 nanoseconds to account for overhead
    if(timer.tv_nsec > OVERHEAD_MASK) {
        if likely(timer.tv_sec > 0) {
            timer.tv_sec  = timer.tv_sec - 1;
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
        if(signal_wait || current_signal == SIGALRM) {
            if(verbose)
                printf("Got signal %s!\n", strsignal(current_signal));
            return 0;
        }

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
