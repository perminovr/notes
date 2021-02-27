#ifndef EVMAINLOOP_H
#define EVMAINLOOP_H

#define EV_STANDALONE 1
#define EV_USE_EPOLL 1
#define EV_USE_POLL 0
#define EV_USE_SELECT 0
#define EV_USE_IOURING 0
#define EV_MULTIPLICITY 0
#define EV_USE_EVENTFD 1
#include "ev++.h"

class EvMainLoop_p;

class EvMainLoop
{
public:
	static EvMainLoop *instance();

	void pause();
	void resume();

private:
	EvMainLoop();
	~EvMainLoop();

	static EvMainLoop *self;

	EvMainLoop_p *priv;
	friend class EvMainLoop_p;
};

#endif // EVMAINLOOP_H