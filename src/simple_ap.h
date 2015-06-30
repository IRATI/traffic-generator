/*
 * Base class for simple RINA applications
 *
 *   Addy Bombeke          <addy.bombeke@ugent.be>
 *   Dimitri Staessens     <dimitri.staessens@intec.ugent.be>
 *   Douwe De Bock         <douwe.debock@ugent.be>
 *   Francesco Salvestrini <f.salvestrini@nextworks.it>
 *
 * This source code has been released under the GEANT outward license.
 * Refer to the accompanying LICENSE file for further information
 */

#ifndef SIMPLE_AP_HPP
#define SIMPLE_AP_HPP

#include <string>
#include <vector>
#include <map>

using namespace std;

/* simple application process (AP) */

class simple_ap {
public:

/* construction */

        simple_ap(const string& apn) :
                name (apn),
                instance ("1") {}
        simple_ap(const string& apn,
                const string& api) :
                name (apn),
                instance (api) {}

/* destruction */
        virtual ~simple_ap();

/* registering and unregistering with a DIF */
/* note "register" is a keyword */
        void register_ap();
        void register_ap(const string& dif_name);
        void register_ap(const vector<string>& dif_names);
        void unregister_ap();
        void unregister_ap(const std::string& dif_name);
        void unregister_ap(const vector<string>& dif_names);

/* requesting and releasing N-1-flows */
        int request_flow(const std::string& apn,
                         const std::string& api,
                         const std::string& qos_cube);

        int request_flow(const std::string& apn,
                         const std::string& api,
                         const std::string& qos_cube,
                         const std::string& dif_name);

        int release_flow(const int port_id);
        void release_all_flows();

protected:
        std::string name;       /* Application Process Name */
        /* FIXME: should be merged with AP Namespace */
        std::string instance;   /* Application Process Instance */
        std::vector<string> reg_difs; /*DIFs this AP is reg'd with */
        std::map<int,string> my_flows; /* flows this AP has allocated */

private:
};

#endif // SIMPLE_AP
