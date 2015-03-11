/*
 * RINA Traffic generator
 *
 *   Addy Bombeke          <addy.bombeke@ugent.be>
 *   Douwe De Bock         <douwe.debock@ugent.be>
 *   Francesco Salvestrini <f.salvestrini@nextworks.it>
 *
 * This source code has been released under the GEANT outward license.
 * Refer to the accompanying LICENSE file for further information
 */

#include <iostream>

#define RINA_PREFIX "traffic-generator"

#include <librina/logs.h>
#include <librina/librina.h>

#include "application.h"

using namespace std;
using namespace rina;

void Application::applicationRegister()
{
        ApplicationRegistrationInformation ari;
        RegisterApplicationResponseEvent * resp;
        unsigned int seqnum;
        IPCEvent * event;

        ari.ipcProcessId = 0;  // This is an application, not an IPC process
        ari.appName = ApplicationProcessNamingInformation(appName,
                        appInstance);

        if (difName == string()) {
                ari.applicationRegistrationType =
                        APPLICATION_REGISTRATION_ANY_DIF;
        } else {
                ari.applicationRegistrationType =
                        APPLICATION_REGISTRATION_SINGLE_DIF;
                ari.difName = ApplicationProcessNamingInformation(difName, string());
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
                ipcManager->commitPendingRegistration(seqnum, resp->DIFName);
        } else {
                ipcManager->withdrawPendingRegistration(seqnum);
                throw ApplicationRegistrationException("Failed to register application");
        }
}

// FIXME: Are we sure ?
const unsigned int Application::maxBufferSize = 1 << 16;
