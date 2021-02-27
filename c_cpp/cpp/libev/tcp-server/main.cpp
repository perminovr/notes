#include "evwrapper.h"

#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <list>

using namespace std;

typedef std::vector<uint8_t> ByteArray;

#define setFdNonBlock(fg) {			\
	int flags = fcntl(fd, F_GETFL);	\
	flags |= O_NONBLOCK;			\
	fcntl(fd, F_SETFL, flags);		\
}

class TestServer;

class TestClient {
public:
	ev::io w;
	int fd;
	TestServer *server;
	TestClient(int cfd) {
		fd = cfd;
		setFdNonBlock(fd);
		w.set(fd, ev::READ);
		w.set<TestClient, &TestClient::watcherCb> (this);
		w.start();
	}
	void watcherCb(ev::io &w, int revents) {
		if (revents & ev::READ) {
			ByteArray ret;
			uint8_t buf[1024];
			int res = ::recv(this->fd, buf, 1024, 0);
			if (res <= 0) {
				w.stop();
				::close(fd);
				server->clients.remove(this);
				return;
			} else {
				ret.insert(ret.end(), &(buf[0]), &(buf[res]));
			}
			::send(this->fd, buf, res, 0);
			string msg((const char *)ret.data(), ret.size());
			cout << msg << endl;
		}
	}
};

class TestServer {
public:
	ev::io w;
	int fd;
	list<TestClient*> clients;
	TestServer(std::string ip, uint16_t port) {
		fd = socket(AF_INET, SOCK_STREAM, 0);
		setFdNonBlock(fd);
		struct sockaddr_in addr = {0};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = inet_addr(ip.c_str());
		if (bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
			cout << "err bind" << endl;
			::abort();
		}
		::listen(fd, SOMAXCONN);

		w.set(fd, ev::READ);
		w.set<TestServer, &TestServer::acceptCb> (this);
		w.start();
	}
	void acceptCb(ev::io &w, int revents) {
		if (revents & ev::READ) {
			struct sockaddr addr;
			socklen_t addrlen = sizeof(struct sockaddr);
			int cfd = ::accept(this->fd, &addr, &addrlen);
			if (cfd > 0) {
				TestClient *client = new TestClient(cfd);
				client->server = this;
				clients.push_back(client);
			}
		}
	}
};


int main()
{
	TestServer server("127.0.0.1", 11112);

	LibEvMain *libEv = LibEvMain::instance();
	libEv->loop();

	return 0;
}
