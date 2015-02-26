/*
 * Traffic generator
 *
 * Douwe De Bock <douwe.debock@ugent.be>
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

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <librina/librina.h>

#include "application.h"

class Client: public Application {
        public:
                Client(const std::string& difName_,
                                const std::string& apn,
                                const std::string& api,
                                const std::string& serverApn,
                                const std::string& serverApi,
                                bool registration,
                                const std::string& testType_,
                                unsigned int size_,
                                unsigned long long count_,
                                unsigned int duration_,
                                unsigned int rate_,
                                bool reliable_) :
                        Application(difName_, apn, api),
                        difName(difName_),
                        serverName(serverApn),
                        serverInstance(serverApi),
                        registerClient(registration),
                        testType(testType_),
                        sduSize(size_),
                        count(count_),
                        duration(duration_),
                        rate(rate_),
                        reliable(reliable_) {
                                if (!duration == !count)
                                        throw rina::IPCException("not a valid count and duration combination!");
                        }

                void run();
        protected:
                rina::Flow * createFlow();
                void setup(rina::Flow * flow);
                void constantBitRate(rina::Flow * flow);
                void poissonDistribution(rina::Flow * flow);
                void receiveServerStats(rina::Flow * flow);
                void destroyFlow(rina::Flow * flow);

        private:
                std::string difName;
                std::string serverName;
                std::string serverInstance;
                bool registerClient;
                std::string testType;
                unsigned int sduSize;
                unsigned long long count;
                unsigned int duration;
                unsigned int rate;
                bool reliable;

                void busyWait(struct timespec &start, double deadline);
                unsigned int secondsElapsed(struct timespec &start);
};
#endif//CLIENT_HPP
