/*
 * Traffic generator main
 *
 *   Dimitri Staessens <dimitri.staessens@intec.ugent.be>
 *   Douwe De Bock <douwe.debock@ugent.be>
 *
 * This source code has been released under the GEANT outward license.
 * Refer to the accompanying LICENSE file for further information
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

/* TODO: add support for test scripts */

int main(int argc, char * argv[])
{
        static bool               listen;
        static bool               registration;
        static bool               busy;
        static unsigned long long count;
        static unsigned int       size;
        static unsigned int       rate;
        static unsigned int       duration;
        static unsigned int       interval;
        static std::string        server_apn;
        static std::string        server_api;
        static std::string        client_apn;
        static std::string        client_api;
        static std::string        dif_name;
        static std::string        distribution_type;
        static std::string        qos_cube;
        static double             poisson_mean;

        try {
                TCLAP::CmdLine cmd("traffic-generator", ' ', PACKAGE_VERSION);
                TCLAP::SwitchArg listenArg(
                        "l",
                        "listen",
                        "Run in server (consumer) mode",
                        false);
                TCLAP::ValueArg<std::string> difArg(
                        "d",
                        "dif",
                        "The name of the DIF to use (empty means 'any DIF')",
                        false,
                        "",
                        "string");
                TCLAP::ValueArg<std::string> serverApnArg(
                        "",
                        "server-apn",
                        "Application process name for the server",
                        false,
                        "traffic.generator.server",
                        "string");
                TCLAP::ValueArg<std::string> serverApiArg(
                        "",
                        "server-api",
                        "Application process instance for the server",
                        false,
                        "1",
                        "string");
                TCLAP::ValueArg<std::string> clientApnArg(
                        "",
                        "client-apn",
                        "Application process name for the client",
                        false,
                        "traffic.generator.client",
                        "string");
                TCLAP::ValueArg<std::string> clientApiArg(
                        "",
                        "client-api",
                        "Application process instance for the client",
                        false,
                        "1",
                        "string");
                TCLAP::SwitchArg registrationArg(
                        "r",
                        "register",
                        "Register the application with the DIF",
                        false);
                TCLAP::ValueArg<unsigned int> sizeArg(
                        "s",
                        "size",
                        "Size of the SDUs to send (bytes)",
                        false,
                        500,
                        "unsigned integer");
                TCLAP::ValueArg<unsigned long long> countArg(
                        "c",
                        "count",
                        "Number of SDUs to send, 0 = unlimited",
                        false,
                        0,
                        "unsigned integer");
                TCLAP::ValueArg<unsigned int> durationArg(
                        "",
                        "duration",
                        "Duration of the test (seconds), 0 = unlimited",
                        false,
                        0,
                        "unsigned integer");
                TCLAP::ValueArg<unsigned int> rateArg(
                        "",
                        "rate",
                        "Bitrate to send the SDUs, in kb/s, 0 = no limit",
                        false,
                        0,
                        "unsigned integer");
                TCLAP::ValueArg<std::string> qoscubeArg(
                        "",
                        "qoscube",
                        "Specify the qos cube to use for flow allocation",
                        false,
                        "unreliable",
                        "string");
                TCLAP::ValueArg<std::string> distributionArg(
                        "",
                        "distribution",
                        "Distribution type: CBR, poisson",
                        false,
                        "CBR",
                        "string");
		TCLAP::ValueArg<double> poissonMeanArg(
                        "",
                        "poissonmean",
                        "The mean value for the poisson distribution "
                        "used to generate interarrival times, "
                        "default is 1.",
                        false,
                        1,
                        "double");
                TCLAP::ValueArg<unsigned int> intervalArg(
                        "",
                        "interval",
                        "report statistics every x ms (server)",
                        false,
                        1000,
                        "unsigned integer");
		TCLAP::SwitchArg sleepArg("",
                                "sleep",
                                "sleep instead of busywait between sending SDUs",
                                false);

		cmd.add(sleepArg);
                cmd.add(registrationArg);
                cmd.add(serverApnArg);
                cmd.add(serverApiArg);
                cmd.add(clientApnArg);
                cmd.add(clientApiArg);
                cmd.add(difArg);
                cmd.add(qoscubeArg);
		cmd.add(poissonMeanArg);
                cmd.add(distributionArg);
                cmd.add(sizeArg);
                cmd.add(rateArg);
                cmd.add(durationArg);
                cmd.add(countArg);
                cmd.add(intervalArg);
                cmd.add(listenArg);

                cmd.parse(argc, argv);

                listen            = listenArg.getValue();
                count             = countArg.getValue();
                registration      = registrationArg.getValue();
                size              = sizeArg.getValue();
                server_apn        = serverApnArg.getValue();
                server_api        = serverApiArg.getValue();
                client_apn        = clientApnArg.getValue();
                client_api        = clientApiArg.getValue();
                dif_name          = difArg.getValue();
                rate              = rateArg.getValue();
                duration          = durationArg.getValue();
                qos_cube          = qoscubeArg.getValue();
                distribution_type = distributionArg.getValue();
                interval          = intervalArg.getValue();
		busy              = !sleepArg.getValue();
		poisson_mean      = poissonMeanArg.getValue();

        } catch (TCLAP::ArgException &e) {
                LOG_ERR("Error: %s for arg %d",
                                e.error().c_str(),
                                e.argId().c_str());
                exit(1);
        }
        if (!count && !duration)
                duration = 60; //run a 60 second test
        try {
                rina::initialize("INFO", "");

                if (listen) {
                        // Server mode
                        server s(server_apn, server_api);
                        s.configure(interval);
                        s.register_ap(dif_name);
                        s.run();
                } else {
                        // Client mode
                        client c(client_apn, client_api);
                        /* FIXME: "" means any DIF, should be cleaned up */
                        if (registration)
                                c.register_ap(dif_name);
                        int port_id = c.request_flow(server_apn, server_api, qos_cube);
                        if (distribution_type == "CBR" || distribution_type == "cbr")
                                c.single_cbr_test(size,
                                                count,
                                                duration,
                                                rate,
                                                busy,
                                                port_id);
                        else if (distribution_type == "poisson")
                                c.single_poisson_test( size,
                                                       count,
                                                       duration,
                                                       rate,
                                                       busy,
                                                       poisson_mean,
                                                       port_id);
                }
        } catch (rina::Exception& e) {
                LOG_ERR("%s", e.what());
                return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
}
