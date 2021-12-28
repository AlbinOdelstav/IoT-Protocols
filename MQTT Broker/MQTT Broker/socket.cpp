#pragma once
#include "socket.h"

Socket::~Socket() {}

Socket::Socket(SOCKET socket) : socket(socket) {}

Socket::Socket() {}

const int Socket::create(const int port) {
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
		std::cout << "Can't start winsock: " << WSAGetLastError << "\n";
		WSACleanup();
		return -1;
	}

	socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (socket == INVALID_SOCKET) {
		std::cout << "Socket function failed: " << WSAGetLastError << "\n";
		WSACleanup();
		return -1;
	}

	sockIn.sin_family = AF_INET;
	sockIn.sin_port = htons(port);
	sockIn.sin_addr.s_addr = INADDR_ANY;

	/*
	BOOL bOptVal = FALSE;
	if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char*)& bOptVal, sizeof(bOptVal)) < 0) {
		perror("setsockopt(SO_REUSEADDR) failed");
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	*/
	if (bind(socket, (const sockaddr*)& sockIn, sizeof(sockIn)) < 0) {
		std::cout << "Failed to bind" << WSAGetLastError << "\n";
		WSACleanup();
		return -1;
	}

	if (::listen(socket, SOMAXCONN)) {
		std::cout << "Failed to listen: " << WSAGetLastError << "\n";
		WSACleanup();
		return -1;
	}
	return 0;
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
	}
	else if (result->ai_family == AF_INET6) {
		sockaddr_in6* sockaddr_ipv6 = (sockaddr_in6*)result->ai_addr;
		inet_ntop(result->ai_family, &sockaddr_ipv6->sin6_addr, address, sizeof(address));
	}

	freeaddrinfo(result);
	return address;
}

SOCKET Socket::getSOCKET() const {
	return this->socket;
}

void Socket::send(std::vector<unsigned char>& msg) {
	if (sendto(socket, (char*)msg.data(), msg.size(), 0, (sockaddr*)& sockIn, sizeof(sockIn)) == SOCKET_ERROR) {
		std::cout << "Socket error: " << WSAGetLastError << "\n";
	}
}

std::vector<unsigned char> Socket::recv() {
	std::vector<unsigned char> response(100);
	if (::recv(socket, (char*)response.data(), response.size(), 0) < 0) {
		std::cout << "Recv failed: " << WSAGetLastError << "\n";
		closesocket(socket);

		// Disconnect
		response[0] = 0b11100000;
		response[1] = 0b00000000;
	}
	return response;
}

Socket Socket::listen() {
	std::cout << "listening\n";
	SOCKET incomingSocket = accept(socket, NULL, NULL);

	if (incomingSocket == INVALID_SOCKET) {
		closesocket(incomingSocket);
		return Socket(INVALID_SOCKET);
	}

	std::cout << "accepted\n";
	return Socket(incomingSocket);
}