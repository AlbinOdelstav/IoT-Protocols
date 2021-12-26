#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "mqtt_message.h"

class SubscribeMessage : public MqttMessage {
public:
	void setPacketIdentifier(const uint16_t);
	void setTopics(const std::vector<Topic>&);

	uint16_t getPacketIdentifier() const;
	std::vector<Topic> getTopics() const;

private:
	uint16_t packetIdentifier;
	std::vector<Topic> topics;
};
