/*
 * Traffic generator
 *
 *   Addy Bombeke          <addy.bombeke@ugent.be>
 *   Dimitri Staessens     <dimitri.staessens@intec.ugent.be>
 *   Douwe De Bock         <douwe.debock@ugent.be>
 *   Francesco Salvestrini <f.salvestrini@nextworks.it>
 *
 * This source code has been released under the GEANT outward license.
 * Refer to the accompanying LICENSE file for further information
 */

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#define BILLION         1000000000L
#define MILLION            1000000L

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

        /* adds intvl to time and stores it in res*/
	/* TODO: move to a different sourcefile? */
	/* TODO: should probably check validity/uniqueness of ptrs */
	void addtime(const struct timespec * t,
		     const struct timespec * intv,
		     struct timespec       * res)
	{
	        long nanos = t->tv_nsec + intv->tv_nsec;

		res->tv_sec = t->tv_sec + intv->tv_sec;
		while (nanos > BILLION) {
		        nanos -= BILLION;
			res->tv_sec++;
		}
		res->tv_nsec=nanos;
	}

	void subtime(const struct timespec * t,
		     const struct timespec * intv,
		     struct timespec       * res)
	{
	        long nanos = t->tv_nsec - intv->tv_nsec;

		res->tv_sec = t->tv_sec - intv->tv_sec;
		while (nanos < 0) {
		        nanos += BILLION;
			res->tv_sec--;
		}
		res->tv_nsec=nanos;
	}
	
	long usElapsed(const struct timespec &start,
		      const struct timespec &end)
	{
	        struct timespec res = {0,0};
	        subtime (&end, &start, &res);
		return (res.tv_sec*BILLION+res.tv_nsec)/1000L;
	}
	
	long msElapsed (const struct timespec &start,
			const struct timespec &end)
	{
	        return usElapsed(start, end)/1000;
	}
};

#endif
