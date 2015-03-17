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

using namespace std;
using namespace rina;

void Server::run()
{
        applicationRegister();

        for(;;) {
                IPCEvent * event = ipcEventProducer->eventWait();
                Flow * flow = 0;
                unsigned int port_id;

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

                        case FLOW_ALLOCATION_REQUESTED_EVENT:
                                {
                                        flow = ipcManager->allocateFlowResponse(*dynamic_cast<FlowRequestEvent*>(event), 0, true);
                                        LOG_INFO("New flow allocated [port-id = %d]", flow->getPortId());
                                        boost::thread t(&Server::startReceive, this, flow);
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

void Server::startReceive(Flow * flow)
{
        unsigned long long count;
        unsigned int       duration;
        unsigned int       sduSize;

        char initData[sizeof(count) + sizeof(duration) + sizeof(sduSize)];

        flow->readSDU(initData, sizeof(count) + sizeof(duration) + sizeof(sduSize));

        memcpy(&count, initData, sizeof(count));
        memcpy(&duration, &initData[sizeof(count)], sizeof(duration));
        memcpy(&sduSize, &initData[sizeof(count) + sizeof(duration)], sizeof(sduSize));
        count = be64toh(count);
        duration = be32toh(duration);
        sduSize = be32toh(sduSize);

        char response[] = "Go ahead!";
        struct timespec start;
        struct timespec end;
        struct timespec tmp;

        LOG_INFO("Starting test:\n\tduration: %u\n\tcount: %llu\n\tsduSize: %u", duration, count, sduSize);

        clock_gettime(CLOCK_REALTIME, &start);
        flow->writeSDU(response, sizeof(response));

        int running = true;
        char data[sduSize];

        flow->readSDU(data, sduSize);
        //clock_gettime(CLOCK_REALTIME, &end);

        /*
           double timeout = 4000 * (((end.tv_sec - start.tv_sec) * 1000000
           - (end.tv_nsec - start.tv_nsec) / 1000));
           struct itimerspec itimer;
           itimer.it_interval.tv_sec = 0;
           itimer.it_interval.tv_nsec = 0;
           itimer.it_value.tv_sec = timeout / 1000000000;
           itimer.it_value.tv_nsec = timeout % 1000000000;
           */
        bool timeTest;
        timer_t timerId;
        if (duration) {
                timeTest = true;
                struct sigevent event;
                struct itimerspec durationTimer;

                memset(&event, 0, sizeof(event));
                event.sigev_notify = SIGEV_THREAD;
                event.sigev_value.sival_ptr = &running;
                event.sigev_notify_function = &timesUp;

                timer_create(CLOCK_REALTIME, &event, &timerId);

                durationTimer.it_interval.tv_sec = 0;
                durationTimer.it_interval.tv_nsec = 0;
                durationTimer.it_value.tv_sec = duration;
                durationTimer.it_value.tv_nsec = 0;

                timer_settime(timerId, 0, &durationTimer, NULL);
        } else
                timeTest = false;

        unsigned long long totalSdus = 1;
        unsigned long long totalBytes = 0;
	unsigned long long totalSdusInterval=0;
        unsigned int ms;
        try {
                clock_gettime(CLOCK_REALTIME, &start);
                clock_gettime(CLOCK_REALTIME, &tmp);
                while (running) {
			totalBytes += flow->readSDU(data, sduSize);
                        totalSdus++;

                        if (!timeTest) {
                                if (totalSdus >= count)
                                        running = 0;
                        }
                        if (interval && totalSdus % interval == 0) {
                                clock_gettime(CLOCK_REALTIME, &end);
                                int us = usElapsed(tmp, end);
				LOG_INFO("%llu SDUs in %lu us => %.4f Mb/s",
                                                totalSdus-totalSdusInterval, us,
                                                static_cast<float>(
                                                        ((totalSdus -totalSdusInterval)
							 * sduSize * 8.0) /
                                                        (us)));

                                clock_gettime(CLOCK_REALTIME, &tmp);
				totalSdusInterval = totalSdus;
                        }
                }
                clock_gettime(CLOCK_REALTIME, &end);
                ms = msElapsed(start, end);
                unsigned int nms = htobe32(ms);
                unsigned long long ncount = htobe64(totalSdus);
                unsigned long long nbytes = htobe64(totalBytes);

                char statistics[sizeof(ncount) + sizeof(nbytes) + sizeof(nms)];
                memcpy(statistics, &ncount, sizeof(ncount));
                memcpy(&statistics[sizeof(ncount)], &nbytes, sizeof(nbytes));
                memcpy(&statistics[sizeof(ncount) + sizeof(nbytes)], &nms, sizeof(nms));

                flow->writeSDU(statistics, sizeof(statistics));

                LOG_INFO("Result: %llu SDUs, %llu bytes in %lu ms", totalSdus, totalBytes, ms);
                LOG_INFO("\t=> %.4f Mb/s", static_cast<float>((totalBytes * 8.0) / (ms * 1000)));
        } catch (IPCException& ex) {
                timer_delete(timerId);
        }
}

void Server::timesUp(sigval_t val)
{
        int *running = (int *)val.sival_ptr;

        *running = 0;
}
