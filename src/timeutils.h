/*
 * Utilities for math with timespecs and timevals
 *
 *   Addy Bombeke          <addy.bombeke@ugent.be>
 *   Dimitri Staessens     <dimitri.staessens@intec.ugent.be>
 *   Douwe De Bock         <douwe.debock@ugent.be>
 *   Francesco Salvestrini <f.salvestrini@nextworks.it>
 *
 * This source code has been released under the GEANT outward license.
 * Refer to the accompanying LICENSE file for further information
 */

#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#define MILLION 1000000L
#define BILLION 1000000000L

#include <time.h>

/* functions for timespecs */

/* add intv to t and store it in res*/
void ts_add(const struct timespec * t,
	    const struct timespec * intv,
	    struct timespec       * res);

/* subtract intv from t and stores it in res */
void ts_diff(const struct timespec * t,
             const struct timespec * intv,
             struct timespec       * res);

/* subtracting fields is faster than using ts_diff */
long ts_diff_ns(const struct timespec &start,
                const struct timespec &end);

/* subtracting fields is faster than using ts_diff */
long ts_diff_us(const struct timespec &start,
                const struct timespec &end);

/* subtracting fields is faster than using ts_diff */
long ts_diff_ms(const struct timespec &start,
		const struct timespec &end);

/* functions for timevals */

/* add intv to t and store it in res*/
void tv_add(const struct timeval * t,
	    const struct timeval * intv,
	    struct timeval       * res);

/* subtract intv from t and stores it in res */
void tv_diff(const struct timeval * t,
             const struct timeval * intv,
             struct timeval       * res);

/* subtracting fields is faster than using tv_diff */
long tv_diff_us(const struct timeval &start,
                const struct timeval &end);

/* subtracting fields is faster than using tv_diff */
long tv_diff_ms(const struct timeval &start,
		const struct timeval &end);

/* FIXME: not sure about the next two functions */
/* copying a timeval into a timespec */
void tv_to_ts(const struct timeval * src,
              struct timespec * dst);

/* copying a timespec into a timeval (loss of resolution) */
void ts_to_tv(const struct timespec * src,
              struct timeval * dst);

#endif /* TIME_UTILS_H */
