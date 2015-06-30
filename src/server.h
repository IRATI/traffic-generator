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

#ifndef SERVER_HPP
#define SERVER_HPP

#include <librina/librina.h>
#include <time.h>
#include <signal.h>

#include "simple_ap.h"

class server: public simple_ap
{
public:
server(const std::string& apn,
       const std::string& api) :
                simple_ap(apn, api),
                stat_interval(0) {}

        void run();
        void configure(unsigned int stat_interval);
        /* void set_log(string& log_fn); */

protected:
        /* not used yet */
        typedef enum {
                S_SERVER_REGISTER,  /* registering server with DIF */
                S_SERVER_NEGOTIATE, /* negotiating test parameters */
                S_SERVER_TEST,      /* performing tests */
                S_SERVER_IDLE       /* waiting for client */
        } server_state_t;

private:

/* internal state */

        unsigned int stat_interval ; /* interval to report stats (ms) */
        /* std::string log_fn; logfile name */

/* internal functions */

        /* respond to a new flow */
        void handle_flow(int port_id);
};

#endif
