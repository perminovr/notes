
#include <unistd.h>
#include <iostream>
#include <chrono>

#include "evmainloop.h"

using namespace std;
using namespace std::chrono;

struct myclass
{
	myclass() {
		cnt = 0;
	}
  void io_cb (ev::io &w, int revents) { 
	  cout << "worked" << endl;
	  usleep(1000);
  }
  int cnt;
  void timer_cb (ev::timer &w, int revents) { 
	  milliseconds ms = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
	  cout << ms.count() << " timer_cb " << cnt++ << endl;
  }
};

int main()
{
	EvMainLoop *ev = EvMainLoop::instance();

	myclass obj;

	ev->pause();


	ev::timer et;
	et.set <myclass, &myclass::timer_cb> (&obj);
	et.set(0.0001, 0.0001);
	et.start();

	ev::io iow;
	iow.set(1, ev::WRITE);
	iow.set <myclass, &myclass::io_cb> (&obj);
	// iow.start();


	ev->resume();

	for(;;) {
		usleep(2000);

		// ev->pause();
		// cout << "on pause" << endl;
		// usleep(100000);
		// cout << "resume" << endl;
		// ev->resume();


	}
	return 0;
}