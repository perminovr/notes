#include "evmainloop.h"
#include "event.h"

#include <thread>
#include <mutex>

#include "ev.c"

// #include <mqueue.h>
	// mqd_t queue;
		// mq_attr attr;
        // attr.mq_flags = O_NONBLOCK;
        // attr.mq_maxmsg = 1;
        // attr.mq_msgsize = 1; // todo
        // attr.mq_curmsgs = 0;
		// queue = mq_open("/evmainloopqueue", O_CREAT | O_RDWR | O_NONBLOCK, 0644, &attr);


using namespace ev;
using namespace std;


class EvMainLoop_p
{
public:
	EvMainLoop_p() {
		que_io.set(cmd.getFd(), ev::READ);
		que_io.set<EvMainLoop_p, &EvMainLoop_p::que_cb> (this);
		que_io.start();
		thr = new thread(&EvMainLoop_p::loop, this);
	}
	~EvMainLoop_p() {
		delete thr;
	}

	inline void pauseEv() {
		mu.lock();
		cmd.raise();
		sched_yield();
	}
	inline void resumeEv() {
		mu.unlock();
	}
private:
	thread *thr;
	mutex mu;
	Event cmd;
	default_loop m_loop;
	io que_io;
	void que_cb(ev::io &w, int revents) {
		cmd.end();
		mu.lock();
		sched_yield();
		mu.unlock();
	}
	void loop(void) {
		m_loop.run(0);
	}
};


void EvMainLoop::pause()
{
	priv->pauseEv();
}


void EvMainLoop::resume()
{
	priv->resumeEv();
}


EvMainLoop* EvMainLoop::self = nullptr;


EvMainLoop *EvMainLoop::instance()
{
	if (!EvMainLoop::self)
		EvMainLoop::self = new EvMainLoop();
	return EvMainLoop::self;
}


EvMainLoop::EvMainLoop()
{
	priv = new EvMainLoop_p();
}


EvMainLoop::~EvMainLoop()
{
	delete priv;
}