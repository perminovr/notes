#ifndef EVWRAPPER_H
#define EVWRAPPER_H

#define EV_USE_FLOOR 1
#define EV_USE_MONOTONIC 1
#define EV_USE_NANOSLEEP 1

#define EV_STANDALONE 1
#define EV_USE_EPOLL 0
#define EV_USE_POLL 1
#define EV_USE_SELECT 0
#define EV_USE_IOURING 0
#define EV_MULTIPLICITY 0
#define EV_USE_EVENTFD 1
#include "ev++.h"

extern "C" {
	typedef struct timespec timespec_t;
}

#define BILLION 	1000000000L	
extern double toDouble(const timespec_t &ts);
extern void operator*=(timespec_t &lhs, float f);
extern bool operator!=(timespec_t &lhs, long i);

typedef ev::timer Timer;

#define timerStart(timer, ts, class, self, cb) \
	timer->set<class, &class::cb> ( ((class*)self) ); \
	timer->set(toDouble(ts)); \
	timer->start();


class LibEvMain_p;

class LibEvMain
{
public:
	static LibEvMain *instance();

	void start();

	void pause();
	void resume();

	void loop();

private:
	LibEvMain();
	~LibEvMain();

	static LibEvMain *self;

	LibEvMain_p *priv;
	friend class LibEvMain_p;
};

#endif // EVWRAPPER_H
