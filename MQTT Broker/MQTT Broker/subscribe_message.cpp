#include "subscribe_message.h"

void SubscribeMessage::setPacketIdentifier(const uint16_t packetIdentifier) {
	this->packetIdentifier = packetIdentifier;
}

void SubscribeMessage::setTopics(const std::vector<Topic>& topics) {
	this->topics = topics;
}

uint16_t SubscribeMessage::getPacketIdentifier() const {
	return this->packetIdentifier;
}

std::vector<Topic> SubscribeMessage::getTopics() const {
	return this->topics;
}
