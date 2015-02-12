/*
 * Traffic generator application
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

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <string>

class Application
{
        public:
                Application(const std::string& difName_,
                                const std::string& appName_,
                                const std::string& appInstance_) :
                        difName(difName_),
                        appName(appName_),
                        appInstance(appInstance_) {}

                static const uint maxBufferSize;

        protected:
                void applicationRegister();

                std::string difName;
                std::string appName;
                std::string appInstance;

};
#endif
