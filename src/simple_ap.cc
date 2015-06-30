/*
 * Simple Application Process
 *
 *   Addy Bombeke          <addy.bombeke@ugent.be>
 *   Douwe De Bock         <douwe.debock@ugent.be>
 *   Francesco Salvestrini <f.salvestrini@nextworks.it>
 *
 * This source code has been released under the GEANT outward license.
 * Refer to the accompanying LICENSE file for further information
 */

#define RINA_PREFIX "traffic-generator"

#include "simple_ap.h"

#include <iostream>
#include <librina/logs.h>
#include <librina/librina.h>


using namespace std;
using namespace rina;

simple_ap::~simple_ap(){
        /* nothing to destroy at the moment */
}

void simple_ap::register_ap()
{
        register_ap("");
}

void simple_ap::register_ap(const string& dif_name)
{
        ApplicationRegistrationInformation ari;
        RegisterApplicationResponseEvent * resp;
        unsigned int                       seqnum;
        IPCEvent *                         event;

        ari.ipcProcessId = 0;  // This is an AP, not an IPC process
        ari.appName = ApplicationProcessNamingInformation(
                this->name,
                this->instance);
        if (dif_name == string()) {
                ari.applicationRegistrationType =
                        APPLICATION_REGISTRATION_ANY_DIF;
        } else {
                ari.applicationRegistrationType =
                        APPLICATION_REGISTRATION_SINGLE_DIF;
                ari.difName = ApplicationProcessNamingInformation(
                        dif_name, string());
        }

        // Request the registration
        seqnum = ipcManager->requestApplicationRegistration(ari);

        // Wait for the response to come
        for (;;) {
                event = ipcEventProducer->eventWait();
                if (event && event->eventType ==
                                REGISTER_APPLICATION_RESPONSE_EVENT &&
                                event->sequenceNumber == seqnum) {
                        break;
                }
        }

        resp = dynamic_cast<RegisterApplicationResponseEvent*>(event);

        // Update librina state
        if (resp->result == 0) {
                ipcManager->commitPendingRegistration(
                        seqnum, resp->DIFName);
                this->reg_difs.push_back(dif_name);
        } else {
                ipcManager->withdrawPendingRegistration(seqnum);
                throw ApplicationRegistrationException(
                        "Failed to register application");
        }
}

void simple_ap::register_ap(const vector<string>& dif_names)
{
        for (unsigned int i=0; i< dif_names.size(); i++)
                this->register_ap(dif_names[i]);
}

void simple_ap::unregister_ap()
{
        unregister_ap(this->reg_difs);
}

/* FIXME: implement this stub */
void simple_ap::unregister_ap(const string& dif_name)
{
        (void) dif_name;
}

void simple_ap::unregister_ap(const vector<string>& dif_names)
{
        for (unsigned int i=0; i< dif_names.size(); i++)
                this->unregister_ap(dif_names[i]);
}

int simple_ap::request_flow(const std::string& apn,
                            const std::string& api,
                            const std::string& qos_cube)
{
        AllocateFlowRequestResultEvent * afrrevent;
        FlowSpecification qos_spec;
        IPCEvent * event;
        unsigned int seqnum;

        if (!std::string("reliable").compare(qos_cube))
                qos_spec.maxAllowableGap = 0;
        else if (!std::string("unreliable").compare(qos_cube))
                qos_spec.maxAllowableGap = -1;
        else
                throw IPCException("not a valid qoscube");

        seqnum = ipcManager->requestFlowAllocation(
                ApplicationProcessNamingInformation(this->name, this->instance),
                ApplicationProcessNamingInformation(apn, api),
                qos_spec);

	for (;;) {
                event = ipcEventProducer->eventWait();
                if (event &&
                    event->eventType == ALLOCATE_FLOW_REQUEST_RESULT_EVENT
                    && event->sequenceNumber == seqnum) {
                        break;
                }
                LOG_DBG("Client got new event %d", event->eventType);
        }

        afrrevent = dynamic_cast<AllocateFlowRequestResultEvent*>(event);

	rina::FlowInformation flow =
                ipcManager->commitPendingFlow(
                        afrrevent->sequenceNumber,
                        afrrevent->portId,
                        afrrevent->difName);
        if (flow.portId < 0) {
                LOG_ERR("Failed to allocate a flow");
                return 0;
        } else
                LOG_DBG("Port id = %d", flow.portId);
        return flow.portId;
}

/* FIXME: resolution of qos_cube to flowspec should be a function */
/* FIXME: update my_flows */
int simple_ap::request_flow(const std::string& apn,
                            const std::string& api,
                            const std::string& qos_cube,
                            const std::string& dif_name)
{
        AllocateFlowRequestResultEvent * afrrevent;
        FlowSpecification qos_spec;
        IPCEvent * event;
        unsigned int seqnum;

        if (!std::string("reliable").compare(qos_cube))
                qos_spec.maxAllowableGap = 0;
        else if (!std::string("unreliable").compare(qos_cube))
                qos_spec.maxAllowableGap = -1;
        else
                throw IPCException("not a valid qoscube");
        if (dif_name == string())
                return request_flow(apn, api, qos_cube);

        seqnum = ipcManager->requestFlowAllocationInDIF(
                ApplicationProcessNamingInformation(
                        this->name,
                        this->instance),
                ApplicationProcessNamingInformation(
                        apn,
                        api),
                ApplicationProcessNamingInformation(
                        dif_name,
                        string()),
                qos_spec);

	for (;;) {
                event = ipcEventProducer->eventWait();
                if (event &&
                    event->eventType == ALLOCATE_FLOW_REQUEST_RESULT_EVENT
                    && event->sequenceNumber == seqnum) {
                        break;
                }
                LOG_DBG("Got new event %d", event->eventType);
        }

        afrrevent =
                dynamic_cast<AllocateFlowRequestResultEvent*>(event);

	rina::FlowInformation flow =
                ipcManager->commitPendingFlow(
                        afrrevent->sequenceNumber,
                        afrrevent->portId,
                        afrrevent->difName);
        if (flow.portId < 0) {
                LOG_ERR("Failed to allocate a flow");
                return 0;
        } else
                LOG_DBG("Port id = %d", flow.portId);
        return flow.portId;
}

/* FIXME, correct return value, update my_flows */
int simple_ap::release_flow(const int port_id)
{
        DeallocateFlowResponseEvent * resp = 0;
        unsigned int seqNum;
        IPCEvent * event;

        seqNum = ipcManager->requestFlowDeallocation(port_id);

        for (;;) {
                event = ipcEventProducer->eventWait();
                if (event &&
                    event->eventType == DEALLOCATE_FLOW_RESPONSE_EVENT
                    && event->sequenceNumber == seqNum) {
                        break;
                }
                LOG_DBG("Got new event %d", event->eventType);
        }
        resp = dynamic_cast<DeallocateFlowResponseEvent*>(event);

        ipcManager->flowDeallocationResult(port_id, resp->result == 0);
        return 0;
}

/* TODO: implement this stub */
void simple_ap::release_all_flows()
{
        return;
}
