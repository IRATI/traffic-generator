/*
 * Traffic generator
 *
 *   Dimitri Staessens <dimitri.staessens@intec.ugent.be>
 *   Douwe De Bock     <douwe.debock@ugent.be>
 *   Sander Vrijders   <sander.vrijders@intec.ugent.be>
 *
 * This source code has been released under the GEANT outward license.
 * Refer to the accompanying LICENSE file for further information
 */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <librina/librina.h>

#include "simple_ap.h"

class client: public simple_ap {
public:
/* constructor */
client(const std::string& apn,
       const std::string& api) :
	simple_ap(apn, api) {}

/* member functions */

	void single_cbr_test(unsigned int size,
			     unsigned long long count,
			     unsigned int duration, /* ms */
			     unsigned int rate,
			     bool busy,
			     int fd);
	void single_cbrc_test(unsigned int size,
			     unsigned long long count,
			     unsigned int duration, /* ms */
			     unsigned int rate,
			     bool busy,
			     int fd);

	void single_poisson_test(unsigned int size,
				 unsigned long long count,
				 unsigned int duration, /* ms */
				 unsigned int rate,
				 bool busy,
				 double poisson_mean,
				 int fd);

protected:

	int negotiate_test(int fd);

	void cbr_flow(int fd);
	void poisson_flow(int fd);

	void receiveServerStats(int fd);
private:
	int negotiate_test (long long count, int duration, int sdu_size, int fd);
	void busy_wait_until (const struct timespec &deadline);
	void sleep_until(const struct timespec &deadline);
};
#endif	// CLIENT_HPP
