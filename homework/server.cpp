#include <iostream>
#include <stdbool.h>
#include <stdio.h>
#include <iomanip>

#include "notification.h"
#include "tcp.h"
#include "calculator.hpp"

using epoll_threadpool::EventManager;
using epoll_threadpool::IOBuffer;
using epoll_threadpool::Notification;
using epoll_threadpool::TcpListenSocket;
using epoll_threadpool::TcpSocket;

using namespace std;
using namespace std::tr1;
using namespace std::tr1::placeholders;

class server
{
public:
	server()
	{
		em.start(1000); // 1000: maximum of connecting socket
		pipe(fds);
	}

	std::tr1::shared_ptr<TcpListenSocket> createListenSocket(EventManager *em, int port)
	{
		std::tr1::shared_ptr<TcpListenSocket> s;
		while (!s)
		{
			s = TcpListenSocket::create(em, port);
		}
		return s;
	}

	void createTcpSocket()
	{
		s = createListenSocket(&em, 5007);
		if (s != NULL)
		{
			cout << "server is running on 5007 port" << endl;
		}
	}

	void acceptCallback()
	{
		s->setAcceptCallback(std::tr1::bind(&server::acceptHandler, this, &n, &c, std::tr1::placeholders::_1));
	}

private:
	void acceptHandler(Notification *n, std::tr1::shared_ptr<TcpSocket> *ps, std::tr1::shared_ptr<TcpSocket> s);
	void receiveChecker(Notification *n, IOBuffer *data);
	void disconnectChecker(Notification *n);

	EventManager em;
	Notification n;
	std::tr1::shared_ptr<TcpSocket> c;
	std::tr1::shared_ptr<TcpListenSocket> s;

	char buf[1024];
	int fds[2], num;
};

void server::acceptHandler(Notification *n, std::tr1::shared_ptr<TcpSocket> *ps, std::tr1::shared_ptr<TcpSocket> s)
{
	*ps = s;
	s->setReceiveCallback(std::tr1::bind(&server::receiveChecker, this, n, std::tr1::placeholders::_1));
	s->setDisconnectCallback(std::tr1::bind(&server::disconnectChecker, this, n));
	s->start();

	num = read(fds[0], buf, sizeof(buf));
	if (num == -1)
	{
		perror("read");
	}
	else
	{
		buf[num] = '\0';
		if (buf == string("error"))
		{
			s->disconnect();
			cout << "error in expression" << endl;
		}
		else
		{
			s->write(new IOBuffer(buf, strlen(buf)));
			cout << "server sent message: " << buf << endl;

			while (1)
			{
				n->wait(); // wait for n->signal()
				if (s->isDisconnected())
				{
					s->disconnect();
					break;
				}
			}
		}
	}
}

void server::receiveChecker(Notification *n, IOBuffer *data)
{
	uint64_t size = data->size();
	const char *str = static_cast<const char *>(data->pulldown(size));

	stringstream ss;
	ss << fixed;

	string result;

	cout << "server received message: " << string(str, size) << endl;
	string expression_string = string(str, size);

	try
	{
		// 64-bit arithmetic
		ss << calculator::eval<int64_t>(expression_string);
		ss >> result;
		cout << "result: " << result << endl;
		write(fds[1], result.c_str(), result.size());
	}
	catch (calculator::error &e)
	{
		cerr << e.what() << endl;
		write(fds[1], "error", 5);
	}
	data->consume(size);
}

void server::disconnectChecker(Notification *n)
{
	n->signal(); // release n->wait()
	n->unsignal();
}

int main()
{
	server s;

	s.createTcpSocket();
	s.acceptCallback();

	while (1)
		;

	return 0;
}
