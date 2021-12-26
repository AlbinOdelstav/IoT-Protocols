#pragma once
#include "mqtt_message.h"
#include <vector>

class SubscribeAckMessage : public MqttMessage {
public:
	void setPacketIdentifier(const uint16_t);
	void setReturnCodes(const std::vector<int>&);

	uint16_t getPacketIdentifier() const;
	std::vector<int> getReturnCodes() const;

private:
	uint16_t packetIdentifier;
	std::vector<int> returnCodes;
};
