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

Socket MqttClient::getSocket() const {
	return this->socket;
}
