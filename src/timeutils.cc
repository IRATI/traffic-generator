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

#include "timeutils.h"

/* functions for timespecs */

/* add intv to t and store it in res*/
void ts_add(const struct timespec * t,
	    const struct timespec * intv,
	    struct timespec       * res)
{
        long nanos = 0;

        if (!(t && intv && res))
                return;

        nanos = t->tv_nsec + intv->tv_nsec;

        res->tv_sec = t->tv_sec + intv->tv_sec;
	while (nanos > BILLION) {
                nanos -= BILLION;
                ++(res->tv_sec);
        }
	res->tv_nsec=nanos;
}

/* subtract intv from t and stores it in res */
void ts_diff(const struct timespec * t,
             const struct timespec * intv,
             struct timespec       * res)
{
        long nanos = 0;

        if (!(t && intv && res))
                return;

        nanos = t->tv_nsec - intv->tv_nsec;

        res->tv_sec = t->tv_sec - intv->tv_sec;
        while (nanos < 0) {
                nanos += BILLION;
		--(res->tv_sec);
        }
	res->tv_nsec=nanos;
}

/* subtracting fields is faster than using ts_diff */
long ts_diff_ns(const struct timespec &start,
                const struct timespec &end)
{
        return (end.tv_sec-start.tv_sec)*BILLION
                + (end.tv_nsec-start.tv_nsec);
}

/* subtracting fields is faster than using ts_diff */
long ts_diff_us(const struct timespec &start,
                const struct timespec &end)
{
        return (end.tv_sec-start.tv_sec)*MILLION
                + (end.tv_nsec - start.tv_nsec)/1000L;
}

/* subtracting fields is faster than using ts_diff */
long ts_diff_ms(const struct timespec &start,
		const struct timespec &end)
{
        return (end.tv_sec-start.tv_sec)*1000L
                + (end.tv_nsec-start.tv_nsec)/MILLION;
}

/* functions for timevals */

/* add intv to t and store it in res*/
void tv_add(const struct timeval * t,
	    const struct timeval * intv,
	    struct timeval       * res)
{
        long micros = 0;

        if (!(t && intv && res))
                return;

        micros = t->tv_usec + intv->tv_usec;

        res->tv_sec = t->tv_sec + intv->tv_sec;
	while (micros > MILLION) {
                micros -= MILLION;
                --(res->tv_sec);
        }
	res->tv_usec=micros;
}

/* subtract intv from t and stores it in res */
void tv_diff(const struct timeval * t,
             const struct timeval * intv,
             struct timeval       * res)
{
        long micros = 0;

        if (!(t && intv && res))
                return;

        micros = t->tv_usec - intv->tv_usec;

        res->tv_sec = t->tv_sec - intv->tv_sec;
        while (micros < 0) {
                micros += MILLION;
		--(res->tv_sec);
        }
	res->tv_usec=micros;
}

/* subtracting fields is faster than using tv_diff */
long tv_diff_us(const struct timeval &start,
                const struct timeval &end)
{
        return (end.tv_sec-start.tv_sec)*MILLION
                + (end.tv_usec-start.tv_usec);
}

/* subtracting fields is faster than using tv_diff */
long tv_diff_ms(const struct timeval &start,
		const struct timeval &end)
{
        return (end.tv_sec-start.tv_sec)*1000L
                + (end.tv_usec-start.tv_usec)/1000L;
}

/* FIXME: not sure about the next two functions */
/* copying a timeval into a timespec */
void tv_to_ts(const struct timeval * src,
              struct timespec * dst)
{
        dst->tv_sec=src->tv_sec;
        dst->tv_nsec = src->tv_usec*1000L;
}

/* copying a timespec into a timeval (loss of resolution) */
void ts_to_tv(const struct timespec * src,
              struct timeval * dst)
{
        dst->tv_sec=src->tv_sec;
        dst->tv_usec = src->tv_nsec/1000L;
}
