#include "mqtt_client.h"

MqttClient::MqttClient() {}

MqttClient::~MqttClient() {}

MqttClient::MqttClient(Socket& socket) : socket(socket) {}


void MqttClient::send(std::vector<unsigned char> msg) {
	this->socket.send(msg);
}

std::vector<unsigned char> MqttClient::recv() {
	auto data = this->socket.recv();
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
