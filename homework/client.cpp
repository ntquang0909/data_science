#include <iostream>

#include "notification.h"
#include "tcp.h"

using epoll_threadpool::EventManager;
using epoll_threadpool::IOBuffer;
using epoll_threadpool::Notification;
using epoll_threadpool::TcpSocket;
using epoll_threadpool::TcpListenSocket;

using namespace std;
using namespace std::tr1;
using namespace std::tr1::placeholders;

class client
{
public:
	client()
	{
		em.start(1); // 1: maximum of connecting socket
	}

	void connectTcpSocket()
	{
		c = TcpSocket::connect(&em, "127.0.0.1", 5007);
		if(c != NULL)
		{ 
			cout << "client connected to server on localhost address and 5007 port" << endl;
		}
	}

	void receiveCallback()
	{
		c->setReceiveCallback(std::tr1::bind(&client::receiveChecker, this, &n, std::tr1::placeholders::_1));
		c->setDisconnectCallback(std::tr1::bind(&client::disconnectChecker, this, &n));
		c->start();
	}

	void writeData()
	{
		string lim = "0123456789+-*/()";
		string str;

		cout << "please enter an expression: ";
		getline(cin, str);

		size_t found = str.find_first_not_of(lim);
		if (found != string::npos)
		{
			cout << "expression is wrong format" << endl;
		}
		else
		{
			data = str.c_str();
			c->write(new IOBuffer(data, strlen(data)));
			cout << "client sent message: " << data << endl;
		}
	}

	void wait()
	{
		n.wait();
	}

	void disconnect()
	{
		c->disconnect();
	}

private:
	void receiveChecker(Notification *n, IOBuffer *data);
	void disconnectChecker(Notification *n);

	EventManager em;
	Notification n;
	std::tr1::shared_ptr<TcpSocket> c;

	const char *data;
};

void client::receiveChecker(Notification *n, IOBuffer *data)
{
	uint64_t size = data->size();
	const char *str = static_cast<const char *>(data->pulldown(size));

	cout << "client received message: " << string(str, size) << endl;
	data->consume(size);
}

void client::disconnectChecker(Notification *n)
{
	n->signal();
}

int main()
{
	client c;

	c.connectTcpSocket();
	c.receiveCallback();
	c.writeData();
	c.wait();
	c.disconnect();

	return 0;
}
