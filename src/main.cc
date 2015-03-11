/*
 * Traffic generator main
 *
 * Douwe De Bock <douwe.debock@ugent.be>
 * 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <cstdlib>
#include <string>

#include <librina/librina.h>

#define RINA_PREFIX     "traffic-generator"
#include <librina/logs.h>

#include "tclap/CmdLine.h"

#include "config.h"
#include "client.h"
#include "server.h"

static bool listen;
static bool registration;
static unsigned long long count;
static unsigned int size;
static unsigned int rate;
static unsigned int duration;
static unsigned int interval;
static std::string serverApn;
static std::string serverApi;
static std::string clientApn;
static std::string clientApi;
static std::string difName;
static std::string distributionType;
static std::string qoscube;

void parseArgs(int argc, char *argv[])
{
        try {
                TCLAP::CmdLine cmd("traffic-generator", ' ', PACKAGE_VERSION);
                TCLAP::SwitchArg listenArg("l",
                                "listen",
                                "Run in server (consumer) mode",
                                false);
                TCLAP::ValueArg<std::string> difArg("d",
                                "dif",
                                "The name of the DIF to use (empty means 'any DIF')",
                                false,
                                "",
                                "string");
                TCLAP::ValueArg<std::string> serverApnArg("",
                                "server-apn",
                                "Application process name for the server",
                                false,
                                "traffic.generator.server",
                                "string");
                TCLAP::ValueArg<std::string> serverApiArg("",
                                "server-api",
                                "Application process instance for the server",
                                false,
                                "1",
                                "string");
                TCLAP::ValueArg<std::string> clientApnArg("",
                                "client-apn",
                                "Application process name for the client",
                                false,
                                "traffic.generator.client",
                                "string");
                TCLAP::ValueArg<std::string> clientApiArg("",
                                "client-api",
                                "Application process instance for the client",
                                false,
                                "1",
                                "string");
                TCLAP::SwitchArg registrationArg("r",
                                "register",
                                "Register the application to any dif",
                                false);
                TCLAP::ValueArg<unsigned int> sizeArg("s",
                                "size",
                                "Size of the SDUs to send",
                                false,
                                500,
                                "unsigned integer");
                TCLAP::ValueArg<unsigned long long> countArg("c",
                                "count",
                                "Number of SDUs to send, 0 = unlimited",
                                false,
                                0,
                                "unsigned integer");
                TCLAP::ValueArg<unsigned int> durationArg("",
                                "duration",
                                "Duration in seconds of the test, 0 = unlimited",
                                false,
                                60,
                                "unsigned integer");
                TCLAP::ValueArg<unsigned int> rateArg("",
                                "rate",
                                "Bitrate to send the SDUs, in Kbps, 0 = no limit",
                                false,
                                0,
                                "unsigned integer");
                TCLAP::ValueArg<std::string> qoscubeArg("",
                                "qoscube",
                                "Specify the qos cube to use for flow allocation",
                                false,
                                "unreliable",
                                "string");
                TCLAP::ValueArg<std::string> distributionArg("",
                                "distribution",
                                "Distribution type: CBR, poisson",
                                false,
                                "CBR",
                                "string");
                TCLAP::ValueArg<unsigned int> intervalArg("",
                                "interval",
                                "report statistics every x SDUs",
                                false,
                                100000000,
                                "unsigned integer");

                cmd.add(listenArg);
                cmd.add(countArg);
                cmd.add(registrationArg);
                cmd.add(sizeArg);
                cmd.add(serverApnArg);
                cmd.add(serverApiArg);
                cmd.add(clientApnArg);
                cmd.add(clientApiArg);
                cmd.add(difArg);
                cmd.add(rateArg);
                cmd.add(durationArg);
                cmd.add(qoscubeArg);
                cmd.add(distributionArg);
                cmd.add(intervalArg);

                cmd.parse(argc, argv);

                listen = listenArg.getValue();
                count = countArg.getValue();
                registration = registrationArg.getValue();
                size = sizeArg.getValue();
                serverApn = serverApnArg.getValue();
                serverApi = serverApiArg.getValue();
                clientApn = clientApnArg.getValue();
                clientApi = clientApiArg.getValue();
                difName = difArg.getValue();
                rate = rateArg.getValue();
                duration = durationArg.getValue();
                qoscube = qoscubeArg.getValue();
                distributionType = distributionArg.getValue();
                interval = intervalArg.getValue();

                if (size > Application::maxBufferSize) {
                        size = Application::maxBufferSize;
                        LOG_INFO("Packet size truncated to %u", size);
                }

        } catch (TCLAP::ArgException &e) {
                LOG_ERR("Error: %s for arg %d",
                                e.error().c_str(),
                                e.argId().c_str());
                exit(1);
        }

}

int main(int argc, char * argv[])
{

        parseArgs(argc, argv);

        try {
                rina::initialize("INFO", "");

                if (listen) {
                        // Server mode
                        Server s(difName, serverApn, serverApi, interval);

                        s.run();
                } else {
                        // Client mode
                        Client c(difName, clientApn, clientApi, serverApn,
                                        serverApi, registration, distributionType,
                                        size, count, duration, rate, qoscube);

                        c.run();
                }
        } catch (Exception& e) {
                LOG_ERR("%s", e.what());
                return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
}
