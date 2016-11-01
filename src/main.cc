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

#define RINA_PREFIX	"traffic-generator"
#include <librina/logs.h>

#include "tclap/CmdLine.h"

#include "config.h"
#include "client.h"
#include "server.h"

/* TODO: add support for test scripts */

int main(int argc, char * argv[])
{
	static bool		  listen;
	static bool		  registration;
	static bool		  busy;
	static unsigned long long count;
	static unsigned int	  size;
	static unsigned int	  rate;
	static unsigned int	  duration;
	static unsigned int	  interval;
	static std::string	  server_apn;
	static std::string	  server_api;
	static std::string	  client_apn;
	static std::string	  client_api;
	static std::string	  dif_name;
	static std::string	  distribution_type;
	static std::string	  qos_cube;
	static std::string        csv_path;
	static double		  poisson_mean;

	try {
		TCLAP::CmdLine cmd("traffic-generator", ' ', PACKAGE_VERSION);
		TCLAP::SwitchArg listenArg(
			"l",
			"listen",
			"Run in server (consumer) mode, default = client.",
			false);
		TCLAP::ValueArg<std::string> difArg(
			"d",
			"dif",
			"The name of the DIF to use, empty for any DIF, "
			"default = empty (any DIF).",
			false,
			"",
			"string");
		TCLAP::ValueArg<std::string> serverApnArg(
			"",
			"server-apn",
			"Application process name for the server, "
			"default = traffic.generator.server.",
			false,
			"traffic.generator.server",
			"string");
		TCLAP::ValueArg<std::string> serverApiArg(
			"",
			"server-api",
			"Application process instance for the server, "
			"default = 1.",
			false,
			"1",
			"string");
		TCLAP::ValueArg<std::string> clientApnArg(
			"",
			"client-apn",
			"Application process name for the client, "
			"default = traffic.generator.client.",
			false,
			"traffic.generator.client",
			"string");
		TCLAP::ValueArg<std::string> clientApiArg(
			"",
			"client-api",
			"Application process instance for the client, "
			"default = 1.",
			false,
			"1",
			"string");
		TCLAP::SwitchArg registrationArg(
			"r",
			"register",
			"Register the application with the DIF, "
			"default = false.",
			false);
		TCLAP::ValueArg<unsigned int> sizeArg(
			"s",
			"size",
			"Size of the SDUs to send (bytes), default = 500.",
			false,
			500,
			"unsigned integer");
		TCLAP::ValueArg<unsigned long long> countArg(
			"c",
			"count",
			"Number of SDUs to send, 0 = unlimited, "
			"default = unlimited.",
			false,
			0,
			"unsigned integer");
		TCLAP::ValueArg<unsigned int> durationArg(
			"",
			"duration",
			"Duration of the test (seconds), 0 = unlimited, "
			"default = 60 s IF count is unlimited.",
			false,
			0,
			"unsigned integer");
		TCLAP::ValueArg<unsigned int> rateArg(
			"",
			"rate",
			"Bitrate to send the SDUs, in kb/s, 0 = no limit, "
			"default = no limit.",
			false,
			0,
			"unsigned integer");
		TCLAP::ValueArg<std::string> qoscubeArg(
			"",
			"qoscube",
			"Specify the qos cube to use for flow allocation, "
			"default = unreliable.",
			false,
			"unreliable",
			"string");
		TCLAP::ValueArg<std::string> distributionArg(
			"",
 			"distribution",
			"Distribution, currently supports Constant Bitrate, "
			"Constant Bitrate with Catchup (Warning, this is very "
			"sensitive to clock drift) and poisson distributed "
			"traffic: CBR, CBRC, poisson, default = CBR.",
			false,
			"CBR",
			"string");
		TCLAP::ValueArg<double> poissonMeanArg(
			"",
			"poissonmean",
			"The mean value for the poisson distribution used to "
			"generate interarrival times, default is 1.0.",
			false,
			1,
			"double");
		TCLAP::ValueArg<unsigned int> intervalArg(
			"",
			"interval",
			"report statistics every x ms (server), "
			"default = 1000.",
			false,
			1000,
			"unsigned integer");
		TCLAP::SwitchArg sleepArg(
			"",
			"sleep",
			"sleep instead of busywait between sending SDUs, "
			"default = false.",
			false);
		TCLAP::ValueArg<std::string> csvPathArg(
			"o",
			"output-path",
			"Write csv files per client to the specified directory, "
			"default = no csv output.",
			false,
			"",
			"string");

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
		cmd.add(csvPathArg);
		cmd.parse(argc, argv);

		listen		  = listenArg.getValue();
		count		  = countArg.getValue();
		registration	  = registrationArg.getValue();
		size		  = sizeArg.getValue();
		server_apn	  = serverApnArg.getValue();
		server_api	  = serverApiArg.getValue();
		client_apn	  = clientApnArg.getValue();
		client_api	  = clientApiArg.getValue();
		dif_name	  = difArg.getValue();
		rate		  = rateArg.getValue();
		duration	  = durationArg.getValue();
		qos_cube	  = qoscubeArg.getValue();
		distribution_type = distributionArg.getValue();
		interval	  = intervalArg.getValue();
		busy		  = !sleepArg.getValue();
		poisson_mean	  = poissonMeanArg.getValue();
		csv_path          = csvPathArg.getValue();

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
			s.set_interval(interval);
			s.set_output_path(csv_path);
			s.register_ap(dif_name);
			s.run();
		} else {
			// Client mode
			client c(client_apn, client_api);
			/* FIXME: "" means any DIF, should be cleaned up */
			if (registration)
				c.register_ap(dif_name);
			int fd = c.request_flow(server_apn,
						server_api,
						qos_cube);
			if (distribution_type == "CBR" ||
			    distribution_type == "cbr")
				c.single_cbr_test(size,
						  count,
						  duration*1000,
						  rate,
						  busy,
						  fd);
			if (distribution_type == "CBRC" ||
			    distribution_type == "cbrc")
				c.single_cbrc_test(size,
						   count,
						   duration*1000,
						   rate,
						   busy,
						   fd);
			if (distribution_type == "poisson")
				c.single_poisson_test(size,
						      count,
						      duration*1000,
						      rate,
						      busy,
						      poisson_mean,
						      fd);
		}
	} catch (rina::Exception& e) {
		LOG_ERR("%s", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
