/*
 * Traffic generator
 *
 * Addy Bombeke <addy.bombeke@ugent.be>
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
