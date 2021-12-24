#include "mqtt_message.h"

MqttMessage::MqttMessage() {}

MqttMessage::~MqttMessage() {}

std::ostream& operator<<(std::ostream& os, const MqttMessage& msg) {
	os << msg.type << "\n";
	os << msg.dup << "\n";
	os << msg.retain << "\n";
	return os;
}

void MqttMessage::setType(const Type type) {
	this->type = type;
}

void MqttMessage::setDup(const bool dup) {
	this->dup = dup;
}

void MqttMessage::setRetain(const bool retain) {
	this->retain = retain;
}

void MqttMessage::setQos(const uint8_t qos) {
	this->qos = qos;
}

Type MqttMessage::getType() const {
	return this->type;
}

bool MqttMessage::getDup() const {
	return this->dup;
}

bool MqttMessage::getRetain() const {
	return this->retain;
}

uint8_t MqttMessage::getQos() const {
	return this->qos;
}
