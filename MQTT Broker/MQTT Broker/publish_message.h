#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "mqtt_message.h"

class PublishMessage : public MqttMessage {
public:
	void setPacketIdentifier(const uint16_t);
	void setTopic(const std::string&);
	void setPayload(const std::string&);

	uint16_t getPacketIdentifier() const;
	std::string getTopic() const;
	std::string getPayload() const;

private:
	uint16_t packetIdentifier;
	std::string topic;
	std::string payload;
};

