/*
 * Traffic generator
 *
 *   Addy Bombeke <addy.bombeke@ugent.be>
 *   Dimitri Staessens <dimitri.staessens@intec.ugent.be>
 *   Douwe De Bock <douwe.debock@ugent.be>
 *   Sander Vrijders <sander.vrijders@intec.ugent.be>
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

using namespace std;
using namespace rina;

void Client::run()
{
        int port_id=0;

        if (registerClient)
                applicationRegister();

        if ((port_id = createFlow()) < 0) {
	        LOG_ERR("Problems creating flow, exiting...");
                return;
        }

        setup(port_id);
        if (!std::string("CBR").compare(distributionType))
                constantBitRate(port_id);
        else if (!std::string("poisson").compare(distributionType))
                poissonDistribution(port_id);
	/* FIXME: removed until we have a non-blocking readSDU ()
        receiveServerStats(port_id);
        */
        destroyFlow(port_id);
}

int Client::createFlow()
{
        AllocateFlowRequestResultEvent * afrrevent;
        FlowSpecification qosspec;
        IPCEvent * event;
        unsigned int seqnum;

        if (!std::string("reliable").compare(qoscube))
                qosspec.maxAllowableGap = 0;
        else if (!std::string("unreliable").compare(qoscube))
                qosspec.maxAllowableGap = -1;
        else
                throw IPCException("not a valid qoscube");

        if (difName != string()) {
                seqnum = ipcManager->requestFlowAllocationInDIF(
                                ApplicationProcessNamingInformation(appName, appInstance),
                                ApplicationProcessNamingInformation(serverName, serverInstance),
                                ApplicationProcessNamingInformation(difName, string()),
                                qosspec);
        } else {
                seqnum = ipcManager->requestFlowAllocation(
                                ApplicationProcessNamingInformation(appName, appInstance),
                                ApplicationProcessNamingInformation(serverName, serverInstance),
                                qosspec);
        }

	for (;;) {
                event = ipcEventProducer->eventWait();
                if (event && event->eventType == ALLOCATE_FLOW_REQUEST_RESULT_EVENT
                                && event->sequenceNumber == seqnum) {
                        break;
                }
                LOG_DBG("Client got new event %d", event->eventType);
        }

        afrrevent = dynamic_cast<AllocateFlowRequestResultEvent*>(event);

	rina::FlowInformation flow = ipcManager->commitPendingFlow(afrrevent->sequenceNumber,
                                                                   afrrevent->portId,
                                                                   afrrevent->difName);
        if (flow.portId < 0) {
                LOG_ERR("Failed to allocate a flow");
                return 0;
        } else
                LOG_DBG("Port id = %d", flow.portId);
        return flow.portId;
}

void Client::setup(int port_id)
{
        //TODO: clean up this fix, padding of 32 bytes added to avoid runt frames
        //when using the shim DIF over Ethernet directly
        char initData[sizeof(count) + sizeof(duration) + sizeof(sduSize) + 32];

        unsigned long long ncount = htobe64(count);
        unsigned int ndur = htobe32(duration);
        unsigned int nsize = htobe32(sduSize);

        memcpy(initData, &ncount, sizeof(ncount));
        memcpy(&initData[sizeof(ncount)], &ndur, sizeof(ndur));
        memcpy(&initData[sizeof(ncount) + sizeof(ndur)], &nsize, sizeof(nsize));

        ipcManager->writeSDU(port_id, initData,
                        sizeof(count) + sizeof(duration) + sizeof(sduSize) + 32);

        char response[128];
        response[127] = '\0';
        ipcManager->readSDU(port_id,response, 127);
        LOG_INFO("starting test");
}

void Client::constantBitRate(int port_id)
{
        unsigned long long seq = 0;
        struct timespec start;
        struct timespec end;
	struct timespec deadline;
        bool stop = 0;
        double byteMilliRate;
        double intervalTime = 0;
        char toSend[sduSize];
        if (rate) {
                byteMilliRate = rate / 8.0; /*kB/s */
                intervalTime = sduSize / byteMilliRate; /* ms */
        }

        clock_gettime(CLOCK_REALTIME, &start);
        while (!stop) {
                memcpy(toSend, &seq, sizeof(seq));
                ipcManager->writeSDU(port_id, toSend, sduSize);
		long nanos = seq*intervalTime*MILLION;
	        int seconds = 0;
		while (nanos > BILLION) {
			seconds++;
			nanos -= BILLION;
		}
		const struct timespec interval = {seconds, nanos};
		addtime (&start, &interval, &deadline);
		if (busy)
		        busyWaitUntil(deadline);
		else
		        sleepUntil(deadline);
                seq++;
		clock_gettime(CLOCK_REALTIME, &end);
                if (duration && usElapsed(start, end)/MILLION >= duration)
                        stop = 1;
                if (count && seq >= count)
		  stop = 1;
        }
        clock_gettime(CLOCK_REALTIME, &end);

        unsigned int us = usElapsed(start, end);
        LOG_INFO("sent statistics: %llu SDUs, %llu bytes in %u us",
                        seq, seq * sduSize, us);
        LOG_INFO("\t=> %.4f Mb/s",
                        static_cast<float>((seq*sduSize * 8.0)/(us)));
}

void Client::poissonDistribution(int port_id)
{
        unsigned long long seq = 0;
        struct timespec start;
        struct timespec end;
        bool stop = 0;
        double byteMilliRate;
        double intervalTime = 0;
        char toSend[sduSize];

        if (rate) {
                byteMilliRate = rate / 8.0;
                intervalTime = sduSize / byteMilliRate; /* ms */
        }

        boost::mt19937 gen;
        gen.seed(time(NULL));
        boost::poisson_distribution<int> pdist(poissonmean);
        boost::variate_generator<boost::mt19937,
                boost::poisson_distribution<int> > rvt(gen, pdist);

        clock_gettime(CLOCK_REALTIME, &start);
	struct timespec next = start;
        while (!stop) {
                memcpy(toSend, &seq, sizeof(seq));
                ipcManager->writeSDU(port_id, toSend, sduSize);
		long nanos = rvt()*MILLION/poissonmean*intervalTime;
		struct timespec interval = {nanos / BILLION, nanos % BILLION};
		addtime(&next,&interval,&next);
		if (busy)
		        busyWaitUntil(next);
		else
		        sleepUntil(next);
                seq++;
                clock_gettime(CLOCK_REALTIME, &end);
                if (duration != 0 && usElapsed(start, end)/MILLION >= duration)
                        stop = 1;
                if (count != 0 && seq >= count)
		        stop = 1;
        }
        clock_gettime(CLOCK_REALTIME, &end);

        unsigned int us = usElapsed(start, end);
        LOG_INFO("sent statistics: %llu SDUs, %llu bytes in %u us",
                        seq, seq * sduSize, us);
        LOG_INFO("\t=> %.4f Mb/s",
                        static_cast<float>((seq*sduSize * 8.0)/us));
}

void Client::receiveServerStats(int port_id)
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
}

/* if deadline is in the past, this function will just return */
void Client::busyWaitUntil(const struct timespec &deadline)
{
        struct timespec now;
        clock_gettime(CLOCK_REALTIME, &now);
        while (now.tv_sec < deadline.tv_sec)
		clock_gettime(CLOCK_REALTIME, &now);
	while (now.tv_sec == deadline.tv_sec && now.tv_nsec<deadline.tv_nsec)
		clock_gettime(CLOCK_REALTIME, &now);
}

void Client::sleepUntil(const struct timespec &deadline)
{
        /* TODO: use sleep for longer waits to avoid burning the CPU */
        struct timespec now;
	struct timespec diff;
        clock_gettime(CLOCK_REALTIME, &now);
	subtime(&deadline,&now,&diff);
	nanosleep (&diff, NULL);
}

void Client::destroyFlow(int port_id)
{
        DeallocateFlowResponseEvent * resp = 0;
        unsigned int seqNum;
        IPCEvent * event;

        seqNum = ipcManager->requestFlowDeallocation(port_id);

        for (;;) {
                event = ipcEventProducer->eventWait();
                if (event && event->eventType == DEALLOCATE_FLOW_RESPONSE_EVENT
                                && event->sequenceNumber == seqNum) {
                        break;
                }
                LOG_DBG("Client got new event %d", event->eventType);
        }
        resp = dynamic_cast<DeallocateFlowResponseEvent*>(event);

        ipcManager->flowDeallocationResult(port_id, resp->result == 0);
}
