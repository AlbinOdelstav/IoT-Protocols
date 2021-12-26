#include "connect_message.h"

void ConnectMessage::setMsb(const uint8_t msb) {
	this->msb = msb;
}

void ConnectMessage::setLsb(const uint8_t lsb) {
	this->lsb = lsb;
}

void ConnectMessage::setProtocolName(const std::string protocolName) {
	this->protocolName = protocolName;
}


uint8_t ConnectMessage::getMsb() const {
	return this->msb;
}

uint8_t ConnectMessage::getLsb() const {
	return this->lsb;
}

std::string ConnectMessage::getProtocolName() const {
	return this->protocolName;
}
