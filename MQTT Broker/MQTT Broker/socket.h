#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windns.h>
#include <windows.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib, "ws2_32.lib")

class Socket {
public:
	~Socket();
	Socket(SOCKET socket);
	Socket();

	const int create(const int port);
	Socket listen();
	void send(std::vector<unsigned char>&);
	std::vector<unsigned char> recv();
	const char* dnsLookup(const char*, const int);

	SOCKET getSOCKET() const;

private:
	sockaddr_in sockIn;
	SOCKET socket;
};