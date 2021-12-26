#include "subscribe_ack_message.h"
void SubscribeAckMessage::setPacketIdentifier(const uint16_t packetIdentifier) {
	this->packetIdentifier = packetIdentifier;
}

void SubscribeAckMessage::setReturnCodes(const std::vector<int>& returnCodes) {
	this->returnCodes = returnCodes;
}

uint16_t SubscribeAckMessage::getPacketIdentifier() const {
	return this->packetIdentifier;
}

std::vector<int> SubscribeAckMessage::getReturnCodes() const {
	return this->returnCodes;
}
