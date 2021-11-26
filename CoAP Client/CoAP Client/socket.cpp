#pragma once
#include "socket.h"

Socket::~Socket() {
	closesocket(out);
	WSACleanup();
}

const char* Socket::dnsLookup(const char* uri, const int port) {
	addrinfo* result = NULL;
	addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	DWORD dwRetval = getaddrinfo(uri, std::to_string(port).c_str(), &hints, &result);
	
	if (dwRetval != 0) {
		std::cout << "DNS Lookup failed\n";
		WSACleanup();
		return NULL;
	}

	char address[64];
	if (result->ai_family == AF_INET) {
		sockaddr_in* sockaddr_ipv4 = (sockaddr_in*)result->ai_addr;
		inet_ntop(AF_INET, &sockaddr_ipv4->sin_addr, address, sizeof(address));
	} else if (result->ai_family == AF_INET6) {
		sockaddr_in6* sockaddr_ipv6 = (sockaddr_in6*)result->ai_addr;
		inet_ntop(result->ai_family, &sockaddr_ipv6->sin6_addr, address, sizeof(address));
	}

	freeaddrinfo(result);
	return address;
}

Socket::Socket(const char* uri, const int port) {
	WSADATA data;
	int wsOk = WSAStartup(MAKEWORD(2, 2), &data);
	if (wsOk != 0) {
		std::cout << "Can't start winsock: " << wsOk << "\n";
	}

	const char* address = dnsLookup(uri, port);

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	inet_pton(AF_INET, address, &server.sin_addr);
	out = socket(AF_INET, SOCK_DGRAM, 0);
}

std::vector<unsigned char> Socket::send(std::vector<unsigned char> msg) {
	int sendOk = sendto(out, (char*)msg.data(), msg.size(), 0, (sockaddr*)& server, sizeof(server));

	if (sendOk == SOCKET_ERROR) {
		std::cout << "Socket error: " << WSAGetLastError << "\n";
	}

	std::vector<unsigned char> response(100);
	int a = recv(out, (char*)response.data(), response.size(), 0);
	return response;
}