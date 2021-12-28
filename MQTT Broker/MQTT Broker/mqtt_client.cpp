#include "mqtt_client.h"

MqttClient::MqttClient() {}

MqttClient::~MqttClient() {}

MqttClient::MqttClient(Socket& socket) : socket(socket) {}


void MqttClient::send(Bytes data) {
	this->socket.send(data);
}

std::pair<Bytes, short> MqttClient::recv() {
	std::pair<Bytes, short> data = this->socket.recv();
	return data;
}

void MqttClient::close() {	
	closesocket(this->socket.getSOCKET());
}

void MqttClient::setClientId(std::string clientId) {
	this->clientId = clientId;
}

std::string MqttClient::getClientId() const {
	return this->clientId;
}

Socket MqttClient::getSocket() const {
	return this->socket;
}

bool operator==(MqttClient& lhs, MqttClient& rhs) {
	return lhs == rhs;
}
