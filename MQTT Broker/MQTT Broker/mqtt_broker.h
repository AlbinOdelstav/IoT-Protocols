#pragma once
#include <iostream>
#include <thread>
#include <vector>>

#include "mqtt_client.h"
#include "socket.h"
#include "mqtt_message.h"
#include "subscribe_message.h"
#include "connect_message.h"
#include "connect_ack_message.h"
#include "subscribe_ack_message.h"
#include "publish_message.h"
#include <map>
#include <bitset>

using Subscription = std::pair<MqttClient, int>;

class MqttBroker {
public:
	void start(const unsigned int port);
	void handleClient(MqttClient);
	void handleConnection(MqttClient&, ConnectMessage);
	void handleSubscription(MqttClient, SubscribeMessage);
	// void handlePublish(MqttClient, PublishMessage);
	void handlePublish(MqttClient&, Bytes&);
	void handlePing(MqttClient);
	Type peekType(const Byte&);
	uint8_t peekMessageLength(const Bytes&);

	unsigned char encodeMqttMessage(MqttMessage&);
	Bytes encodeConnectAck(ConnectAckMessage&);
	Bytes encodeSubscribeAck(SubscribeAckMessage&);

	void decodeFixed(MqttMessage&, Bytes&);
	ConnectMessage decodeConnect(Bytes&);
	SubscribeMessage decodeSubscribe(Bytes&);
	PublishMessage decodePublish(Bytes&);
	std::map<std::string, std::vector<Subscription>> subscriptions;
	
	MqttBroker();
	~MqttBroker();
};