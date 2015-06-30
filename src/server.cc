/*
 * Traffic generator
 *
 *   Addy Bombeke <addy.bombeke@ugent.be>
 *   Dimitri Staessens <dimitri.staessens@intec.ugent.be>
 *   Douwe De Bock <douwe.debock@ugent.be>
 *
 * This source code has been released under the GEANT outward license.
 * Refer to the accompanying LICENSE file for further information
 */

#include <time.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <endian.h>

#include <iostream>
#include <boost/thread.hpp>

#define RINA_PREFIX "traffic-generator"
#include <librina/logs.h>

#include "server.h"
#include "timeutils.h"


using namespace std;
using namespace rina;

void server::configure(unsigned int interval)
{
        this->stat_interval = interval;
}

void server::run()
{
        for(;;) {
                IPCEvent * event = ipcEventProducer->eventWait();
                int        port_id = 0;

                if (!event)
                        return;

                switch (event->eventType) {

                case REGISTER_APPLICATION_RESPONSE_EVENT:
                        ipcManager->commitPendingRegistration(event->sequenceNumber,
                                dynamic_cast<RegisterApplicationResponseEvent*>(event)->DIFName);
                        break;

                case UNREGISTER_APPLICATION_RESPONSE_EVENT:
                        ipcManager->appUnregistrationResult(event->sequenceNumber,
                                dynamic_cast<UnregisterApplicationResponseEvent*>(event)->result == 0);
                                break;

                case FLOW_ALLOCATION_REQUESTED_EVENT: {
                        rina::FlowInformation flow = ipcManager->allocateFlowResponse(
                                *dynamic_cast<FlowRequestEvent*>(event), 0, true);
                        LOG_INFO("New flow allocated [port-id = %d]", flow.portId);
                        boost::thread t(&server::handle_flow, this, flow.portId);
                        t.detach();
                        break;
                }
                case FLOW_DEALLOCATED_EVENT:
                        port_id = dynamic_cast<FlowDeallocatedEvent*>(event)->portId;
                        ipcManager->flowDeallocated(port_id);
                        LOG_INFO("Flow torn down remotely [port-id = %d]", port_id);
                        break;

                default:
                        LOG_INFO("Server got new event of type %d",
                                 event->eventType);
                        break;
                }
        }
}

void server::handle_flow(int port_id)
{
        /* negotiate test with client */
        unsigned long long count;
        unsigned int       duration;
        unsigned int       sdu_size;

        /* FIXME: clean up this junk ---> */
        /* receive test parameters from client */
        char initData[sizeof(count) + sizeof(duration) + sizeof(sdu_size) + 32];

        ipcManager->readSDU(port_id,
                            initData,
                            sizeof(count) + sizeof(duration) + sizeof(sdu_size) + 32);

        memcpy(&count, initData, sizeof(count));
        memcpy(&duration, &initData[sizeof(count)], sizeof(duration));
        memcpy(&sdu_size, &initData[sizeof(count) + sizeof(duration)], sizeof(sdu_size));

        count = be64toh(count);
        duration = be32toh(duration);
        sdu_size = be32toh(sdu_size);

        char response[] = "Go ahead!                                               ";
        struct timespec start;

        LOG_INFO("Starting test:\n\tduration: %u\n\tcount: %llu\n\tsize: %u", duration, count, sdu_size);
        LOG_INFO("\n\tInterval: %u", stat_interval);
        ipcManager->writeSDU(port_id, response, sizeof(response));

        /* <--- FIXME */

        char data[sdu_size];
        bool               stop            = false;
        bool               timed_test      = (duration > 0);
        unsigned long long sdus            = 0; /* total # sdus in test */
        unsigned long long sdus_intv       = 0; /* # sdus up to previous interval */
        unsigned long long bytes_read      = 0; /* total bytes read */
        unsigned long long bytes_read_intv = 0; /* bytes read up to previous interval */

        struct timespec    end;      /* end of test */
        struct timespec    intv;     /* reporting interval */
        struct timespec    iv_start; /* start reporting intv */
        struct timespec    iv_end;   /* reporting deadline */
        struct timespec    now;

        clock_gettime(CLOCK_REALTIME, &start);
        if (timed_test) {
                intv.tv_sec = duration;
                ts_add(&start, &intv, &end);
                if (!count)
                        count=(unsigned long long) -1L;
        }
        /* set reporting interval */
        intv.tv_sec = stat_interval/1000;
        intv.tv_nsec = (stat_interval%1000)*MILLION;
        ts_add(&start, &intv, &iv_end); /* next deadline for reporting */
        clock_gettime(CLOCK_REALTIME, &iv_start);
        try {
                while (!stop) {
                        clock_gettime(CLOCK_REALTIME, &now);
                        bytes_read +=  ipcManager->readSDU(port_id, data, sdu_size);
                        sdus++;
                        if (!timed_test && sdus >= count)
                                stop = true;
                        if (timed_test && (sdus >= count || ts_diff_us(now,end) < 0))
                                stop = true;
                        if (stat_interval && (stop || ts_diff_us(now, iv_end) < 0)) {
                                int us = ts_diff_us(iv_start, now);
				LOG_INFO("%llu SDUs (%llu bytes) in %lu us => %.4f p/s,  %.4f Mb/s",
                                         sdus-sdus_intv,
                                         bytes_read-bytes_read_intv,
                                         us,
                                         ((sdus-sdus_intv) / (float) us) * MILLION,
                                         8 * (bytes_read-bytes_read_intv) / (float)us);
                                iv_start=iv_end;
				sdus_intv = sdus;
                                bytes_read_intv = bytes_read;
                                ts_add(&iv_start, &intv, &iv_end); /* end time for next interval */
                        }
                }
                clock_gettime(CLOCK_REALTIME, &end);
                unsigned int ms = ts_diff_ms(start, end);
                /* FIXME: removed until we have non-blocking readSDU()
                   unsigned int nms = htobe32(ms);
                   unsigned long long ncount = htobe64(totalSdus);
                   unsigned long long nbytes = htobe64(totalBytes);

                   char statistics[sizeof(ncount) + sizeof(nbytes) + sizeof(nms) + 64];
                memcpy(statistics, &ncount, sizeof(ncount));
                memcpy(&statistics[sizeof(ncount)], &nbytes, sizeof(nbytes));
                memcpy(&statistics[sizeof(ncount) + sizeof(nbytes)], &nms, sizeof(nms));

                ipcManager->writeSDU(port_id, statistics, sizeof(statistics) + 64);
                */

                LOG_INFO("Result: %llu SDUs, %llu bytes in %lu ms", sdus, bytes_read, ms);
                LOG_INFO("\t=> %.4f Mb/s", static_cast<float>((bytes_read * 8.0) / (ms * 1000)));
        } catch (IPCException& ex) {
	}
}
