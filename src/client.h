/*
 * Traffic generator
 *
 *   Douwe De Bock <douwe.debock@ugent.be>
 *
 * This source code has been released under the GEANT outward license.
 * Refer to the accompanying LICENSE file for further information
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
                                const std::string& distributionType_,
                                unsigned int size_,
                                unsigned long long count_,
                                unsigned int duration_,
                                unsigned int rate_,
                                const std::string& qoscube_) :
                        Application(difName_, apn, api),
                        difName(difName_),
                        serverName(serverApn),
                        serverInstance(serverApi),
                        registerClient(registration),
                        distributionType(distributionType_),
                        sduSize(size_),
                        count(count_),
                        duration(duration_),
                        rate(rate_),
                        qoscube(qoscube_) {
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
                std::string distributionType;
                unsigned int sduSize;
                unsigned long long count;
                unsigned int duration;
                unsigned int rate;
                std::string qoscube;

                void busyWait(struct timespec &start, double deadline);
                unsigned int secondsElapsed(struct timespec &start);
};
#endif//CLIENT_HPP
