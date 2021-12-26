#include "publish_message.h"

void PublishMessage::setPacketIdentifier(const uint16_t packetIdentifier) {
	this->packetIdentifier = packetIdentifier;
}

void PublishMessage::setTopic(const std::string& topic) {
	this->topic = topic;
}

void PublishMessage::setPayload(const std::string& payload) {
	this->payload = payload;
}

uint16_t PublishMessage::getPacketIdentifier() const {
	return this->packetIdentifier;
}

std::string PublishMessage::getTopic() const {
	return this->topic;
}

std::string PublishMessage::getPayload() const {
	return this->payload;
}
