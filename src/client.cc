//
// Traffic generator
//
// Addy Bombeke <addy.bombeke@ugent.be>
// Vincenzo Maffione <v.maffione@nextworks.it>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <cstring>
#include <iostream>
#include <sstream>
#include <cassert>
#include <random>
#include <thread>
#include <unistd.h>
#include <string.h>

#define RINA_PREFIX     "traffic-generator"
#include <librina/logs.h>

#include "client.h"

using namespace std;
using namespace rina;

void Client::run()
{
	Flow * flow;

	if (registerClient) {
		applicationRegister();
		LOG_DBG("registration finished");
	}
	flow = createFlow();
	LOG_DBG("flow created");

	if (flow) {
		generateTraffic(flow);
		LOG_INFO("traffic generated");
		waitForDeallocate();
		LOG_INFO("done");
	}
}

Flow * Client::createFlow()
{
	Flow * flow = nullptr;
	AllocateFlowRequestResultEvent * afrrevent;
	FlowSpecification qosspec;
	IPCEvent * event;
	uint seqnum;

	if (reliable)
		qosspec.maxAllowableGap = 0;
	else
		qosspec.maxAllowableGap = 1;

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

	flow = ipcManager->commitPendingFlow(afrrevent->sequenceNumber,
			afrrevent->portId,
			afrrevent->difName);
	if (!flow || flow->getPortId() == -1) {
		LOG_ERR("Failed to allocate a flow");
		return nullptr;
	} else
		LOG_DBG("Port id = %d", flow->getPortId());

	return flow;
}

void Client::generateTraffic(Flow * flow)
{
	char initData[sizeof(count) + sizeof(duration) + sizeof(sduSize)];

	memcpy(initData, &count, sizeof(count));
	memcpy(&initData[sizeof(count)], &duration, sizeof(duration));
	memcpy(&initData[sizeof(count) + sizeof(duration)], &sduSize, sizeof(sduSize));

	flow->writeSDU(initData, sizeof(count) + sizeof(duration) + sizeof(sduSize));
	LOG_INFO("starting test");

	char response[50];
	flow->readSDU(response, 50);

	LOG_INFO("Server response: %s", response);

	unsigned int long long seq = 0;
	struct timespec start;
	clock_gettime(CLOCK_REALTIME, &start);
	bool running = 1;

	double byteMilliRate;
	double intervalTime = 0;
	if (rate) {
		byteMilliRate = rate / 8.0;
		intervalTime = sduSize / byteMilliRate;
	}

	char toSend[sduSize];
	while (running) {
		memcpy(toSend, &seq, sizeof(seq));
		flow->writeSDU(toSend, sduSize);

		busyWait(start, seq * intervalTime);

		seq++;
		if (seq % 997 == 0) {
			if (duration != 0 && secondsElapsed(start) >= duration)
				running = 0;
			if (count != 0 && seq >= count)
				running = 0;
		}
	}
	LOG_INFO("I did a good job, sending %llu SDUs. Thats %llu bytes",
			seq, seq * sduSize);
}

void Client::busyWait(struct timespec &start, double deadline)
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);

	while (deadline > (((now.tv_sec - start.tv_sec) * 1000000
				- (now.tv_nsec - start.tv_nsec) / 1000) / 1000))
		clock_gettime(CLOCK_REALTIME, &now);
}

unsigned int Client::secondsElapsed(struct timespec &start)
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);

	int cor = (now.tv_nsec < start.tv_nsec) ? 1 : 0;

	return now.tv_sec - start.tv_sec - cor;
}

void Client::waitForDeallocate()
{
	int deallocated = 0;

        while (!deallocated) {
                IPCEvent* event = ipcEventProducer->eventWait();
                unsigned int port_id;

                if (!event)
                        return;

                switch (event->eventType) {

                case FLOW_DEALLOCATED_EVENT:
                        port_id = dynamic_cast<FlowDeallocatedEvent*>(event)->portId;
                        ipcManager->flowDeallocated(port_id);
                        LOG_INFO("Flow torn down remotely [port-id = %d]", port_id);
			deallocated = 1;
                        break;

                default:
                        LOG_INFO("Server got new event of type %d",
                                        event->eventType);
                        break;
                }
        }
}
