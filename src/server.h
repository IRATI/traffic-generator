/*
 * Traffic generator
 *
 *   Addy Bombeke <addy.bombeke@ugent.be>
 *   Douwe De Bock <douwe.debock@ugent.be>
 *
 * This source code has been released under the GEANT outward license.
 * Refer to the accompanying LICENSE file for further information
 */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <librina/librina.h>
#include <time.h>
#include <signal.h>

#include "application.h"

class Server: public Application
{
public:
        Server(const std::string& difName,
               const std::string& appName,
               const std::string& appInstance,
               unsigned int interval_) :
                       Application(difName, appName, appInstance),
                       interval(interval_) {}

                void run();

protected:

private:
        void startReceive(rina::Flow * flow);
        static void timesUp(sigval_t val);
        unsigned int interval;
};

#endif
