#include "evwrapper.h"

#include <mutex>
#include "event.h"
#include "linuxthread.h"

using namespace std;


double toDouble(const timespec_t &ts)
{
	return ((double)ts.tv_nsec / (double)BILLION) + (double)ts.tv_sec;
}


void operator*=(timespec_t &lhs, float f)
{
	if (lhs.tv_nsec == 0) {
		lhs.tv_sec--;
		lhs.tv_nsec = (BILLION-1);
	}
	lhs.tv_sec = (time_t)((double)lhs.tv_sec * (double)f);
	lhs.tv_nsec = (long)((double)lhs.tv_nsec * (double)f);
}


bool operator!=(timespec_t &lhs, long i)
{
	return lhs.tv_sec != 0 && lhs.tv_nsec != 0;
}


class LibEvMain_p
{
public:
	LibEvMain_p() {
		que_io.set(cmd.getFd(), ev::READ);
		que_io.set<LibEvMain_p, &LibEvMain_p::que_cb> (this);
		que_io.start();
		thr = nullptr;
	}
	~LibEvMain_p() {
		delete thr;
	}

	inline void start() {
		if (!thr) thr = new LinuxThread(this, &LibEvMain_p::loop);
	}
	inline void pauseEv() {
		mu.lock();
		cmd.raise();
		sched_yield();
	}
	inline void resumeEv() {
		mu.unlock();
	}
	inline void loop(void) {
		m_loop.run(0);
	}

private:
	LinuxThread *thr;
	mutex mu;
	Event cmd;
	ev::default_loop m_loop;
	ev::io que_io;
	
	void que_cb(ev::io &w, int revents) {
		cmd.end();
		mu.lock();
		sched_yield();
		mu.unlock();
	}
};


void LibEvMain::loop()
{
	priv->loop();
}


void LibEvMain::start()
{
	priv->start();
}


void LibEvMain::pause()
{
	priv->pauseEv();
}


void LibEvMain::resume()
{
	priv->resumeEv();
}


LibEvMain* LibEvMain::self = nullptr;


LibEvMain *LibEvMain::instance()
{
	if (!LibEvMain::self)
		LibEvMain::self = new LibEvMain();
	return LibEvMain::self;
}


LibEvMain::LibEvMain()
{
	priv = new LibEvMain_p();
}


LibEvMain::~LibEvMain()
{
	delete priv;
	LibEvMain::self = nullptr;
}


#include "ev.c"