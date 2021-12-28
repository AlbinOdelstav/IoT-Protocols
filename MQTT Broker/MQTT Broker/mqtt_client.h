#pragma once
#include <iostream>
#include <vector>
#include "socket.h"

class MqttClient {
public:
	MqttClient();
	~MqttClient();
	MqttClient(Socket& socket);
	void send(std::vector<unsigned char>);
	std::vector<unsigned char> recv();
	void close();
	Socket getSocket() const;

	void setClientId(std::string);
	std::string getClientId() const;

	friend bool operator== (MqttClient& lhs, MqttClient& rhs);
private:
	Socket socket;
	std::string clientId;
};