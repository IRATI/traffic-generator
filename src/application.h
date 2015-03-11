/*
 * Traffic generator
 *
 *   Addy Bombeke  <addy.bombeke@ugent.be>
 *   Douwe De Bock <douwe.debock@ugent.be>
 *   Francesco Salvestrini <f.salvestrini@nextworks.it>
 *
 * This source code has been released under the GEANT outward license.
 * Refer to the accompanying LICENSE file for further information
 */

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <string>

class Application {
 public:
 Application(const std::string& difName_,
             const std::string& appName_,
             const std::string& appInstance_) :
        difName(difName_),
                appName(appName_),
                appInstance(appInstance_) {}

        static const unsigned int maxBufferSize;

 protected:
        void applicationRegister();

        std::string difName;
        std::string appName;
        std::string appInstance;

        static unsigned int msElapsed(const struct timespec &start,
                                      const struct timespec &end) {
                return ((end.tv_sec - start.tv_sec) * 1000000000
                        + (end.tv_nsec - start.tv_nsec)) / 1000000;
        }

};

#endif
