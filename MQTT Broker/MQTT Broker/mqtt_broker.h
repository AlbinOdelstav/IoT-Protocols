#pragma once
#include <iostream>
#include <thread>

#include "mqtt_client.h"
#include "socket.h"
#include "mqtt_message.h"

class MqttBroker {
public:
	void start(const unsigned int port);
	int handleClient(MqttClient);
	void handleSubscription(MqttClient, MqttMessage);
	MqttMessage decode(std::vector<unsigned char>);
	std::vector<unsigned char> encode(MqttMessage&);
	MqttBroker();
	~MqttBroker();
};