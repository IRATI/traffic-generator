/*
 * Traffic generator
 *
 *   Addy Bombeke      <addy.bombeke@ugent.be>
 *   Dimitri Staessens <dimitri.staessens@intec.ugent.be>
 *   Douwe De Bock     <douwe.debock@ugent.be>
 *   Sander Vrijders   <sander.vrijders@intec.ugent.be>
 *
 * This source code has been released under the GEANT outward license.
 * Refer to the accompanying LICENSE file for further information
 */

#include <cstring>
#include <iostream>
#include <sstream>
#include <string.h>
#include <endian.h>
#include <ctime>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/poisson_distribution.hpp>
#include <boost/random/variate_generator.hpp>

#define RINA_PREFIX     "traffic-generator"

#include <librina/logs.h>

#include "client.h"
#include "timeutils.h"

using namespace std;
using namespace rina;

/*
void client::run()
{
        int port_id=0;

        if ((port_id = request_flow()) < 0) {
	        LOG_ERR("Problems creating flow, exiting...");
                return;
        }

        setup(port_id);
        if (!std::string("CBR").compare(distributionType))
                constantBitRate(port_id);
        else if (!std::string("poisson").compare(distributionType))
        poissonDistribution(port_id); */
	/* FIXME: removed until we have a non-blocking readSDU ()
        receiveServerStats(port_id);
        destroyFlow(port_id);
} */

/* this needs to be redesigned, we need test templates */
int client::negotiate_test(long long count, int duration, int sdu_size, int port_id)
{
        //TODO: clean up this fix, padding of 32 bytes added to avoid runt frames
        //when using the shim DIF over Ethernet directly
        char init_data[sizeof(count) + sizeof(duration) + sizeof(sdu_size) + 32];

        unsigned long long ncount = htobe64(count);
        unsigned int ndur = htobe32(duration);
        unsigned int nsize = htobe32(sdu_size);

        memcpy(init_data, &ncount, sizeof(ncount));
        memcpy(&init_data[sizeof(ncount)], &ndur, sizeof(ndur));
        memcpy(&init_data[sizeof(ncount) + sizeof(ndur)], &nsize, sizeof(nsize));

        ipcManager->writeSDU(port_id, init_data,
                        sizeof(count) + sizeof(duration) + sizeof(sdu_size) + 32);

        char response[128];
        response[127] = '\0';
        ipcManager->readSDU(port_id, response, 127);
        LOG_INFO("starting test");
        return 0; //ALL IS WELL
}

/* needs common code extracted and function ptrs to
   interval generator and packet generator functions
   probably out of scope for PRISTINE */
/* void client::test(struct test_specs specs, void (*pkt_gen)(args), void (*intv_gen)(args)) */
void client::single_cbr_test(unsigned int size,
                             unsigned long long count,
                             unsigned int duration, /* ms */
                             unsigned int rate,
                             bool busy,
                             int port_id)
{
        unsigned long long seq = 0;
        struct timespec start;
        struct timespec end;
	struct timespec deadline;
        bool stop = 0;
        double byterate;
        double interval_time = 0;
        char to_send[size];

        if (negotiate_test(count, duration, size, port_id) < 0)
                return;

        if (rate) {
                byterate = rate / 8.0; /*kB/s */
                interval_time = size / byterate; /* ms */
        }

        clock_gettime(CLOCK_REALTIME, &start);
        while (!stop) {
                memcpy(to_send, &seq, sizeof(seq));
                ipcManager->writeSDU(port_id, to_send, size);
		long nanos = seq*interval_time*MILLION;
	        int seconds = 0;
		while (nanos > BILLION) {
			seconds++;
			nanos -= BILLION;
		}
		const struct timespec interval = {seconds, nanos};
		ts_add(&start, &interval, &deadline);
		if (busy)
		        busy_wait_until(deadline);
		else
		        sleep_until(deadline);
                seq++;
		clock_gettime(CLOCK_REALTIME, &end);
                if (duration &&
                		ts_diff_us(start, end)/MILLION >= (long)duration)
                        stop = true;
                if (!duration && count && seq >= count)
		  stop = true;
        }
        clock_gettime(CLOCK_REALTIME, &end);

        unsigned int us = ts_diff_us(start, end);
        LOG_INFO("sent statistics: %llu SDUs, %llu bytes in %u us",
                        seq, seq * size, us);
        LOG_INFO("\t=> %.4f Mb/s",
                        static_cast<float>((seq*size * 8.0)/(us)));
}

void client::single_poisson_test(unsigned int size,
                                unsigned long long count,
                                unsigned int duration, /* ms */
                                unsigned int rate, /* b/s */
                                bool busy,
                                double poisson_mean,
                                int port_id)
{
        unsigned long long seq = 0;
        struct timespec    start;
        struct timespec    end;
        bool               stop = 0;
        double             interval_time = 0;
        double             byterate; /* B/ms */
        char               to_send[size];

        if (negotiate_test(count, duration, size, port_id) < 0)
                return;

        if (rate) {
                byterate = rate / 8.0;
                interval_time = size / byterate; /* ms */
        }

        boost::mt19937 gen;
        gen.seed(time(NULL));
        boost::poisson_distribution<int> pdist(poisson_mean);
        boost::variate_generator<boost::mt19937,
                boost::poisson_distribution<int> > rvt(gen, pdist);

        clock_gettime(CLOCK_REALTIME, &start);
	struct timespec next = start;
        while (!stop) {
                memcpy(to_send, &seq, sizeof(seq));
                ipcManager->writeSDU(port_id, to_send, size);
		long nanos = rvt()*MILLION/poisson_mean*interval_time;
		struct timespec interval = {nanos / BILLION, nanos % BILLION};
		ts_add(&next,&interval,&next);
		if (busy)
		        busy_wait_until(next);
		else
		        sleep_until(next);
                seq++;
                clock_gettime(CLOCK_REALTIME, &end);
                if (duration != 0 &&
                		ts_diff_us(start, end)/MILLION >= (long)duration)
                        stop = 1;
                if (count != 0 && seq >= count)
		        stop = 1;
        }
        clock_gettime(CLOCK_REALTIME, &end);

        unsigned int us = ts_diff_us(start, end);
        LOG_INFO("sent statistics: %llu SDUs, %llu bytes in %u us",
                        seq, seq * size, us);
        LOG_INFO("\t=> %.4f Mb/s",
                        static_cast<float>((seq*size * 8.0)/us));
}
/* obsolete until we have non-blocking I/O
void client::receive_server_stats(int port_id)
{
        char response[128];
        unsigned long long totalBytes;
        unsigned long long sduCount;
        unsigned int ms;

        ipcManager->readSDU(port_id, response, 128);
        memcpy(&sduCount, response, sizeof(sduCount));
        memcpy(&totalBytes, &response[sizeof(sduCount)], sizeof(totalBytes));
        memcpy(&ms, &response[sizeof(sduCount) + sizeof(totalBytes)], sizeof(ms));
        sduCount = be64toh(sduCount);
        totalBytes = be64toh(totalBytes);
        ms = be32toh(ms);

        LOG_INFO("Server result: %llu SDUs, %llu bytes in %lu ms",
                        sduCount, totalBytes, ms);
        LOG_INFO("\t=> %.4f Mb/s",
                        static_cast<float>((totalBytes * 8.0) / (ms * 1000)));
} */

/* if deadline is in the past, this function will just return */

void client::busy_wait_until(const struct timespec &deadline)
{
        struct timespec now;
        clock_gettime(CLOCK_REALTIME, &now);
        while (now.tv_sec < deadline.tv_sec)
		clock_gettime(CLOCK_REALTIME, &now);
	while (now.tv_sec == deadline.tv_sec && now.tv_nsec<deadline.tv_nsec)
		clock_gettime(CLOCK_REALTIME, &now);
}

void client::sleep_until(const struct timespec &deadline)
{
        /* TODO: use sleep for longer waits to avoid burning the CPU */
        struct timespec now;
	struct timespec diff;
        clock_gettime(CLOCK_REALTIME, &now);
	ts_diff(&deadline,&now,&diff);
	nanosleep (&diff, NULL);
}
