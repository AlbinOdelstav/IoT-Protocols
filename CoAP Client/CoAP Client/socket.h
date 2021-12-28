#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windns.h>
#include <string>
#include <vector>

#pragma comment (lib, "ws2_32.lib")

class Socket {
public:
	~Socket();
	Socket(const char* uri, const int port);
	std::vector<unsigned char> send(std::vector<unsigned char> msg);
	const char* dnsLookup(const char* uri, const int port);
private:
	sockaddr_in server;
	SOCKET out;
};