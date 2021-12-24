#pragma once
#include <iostream>
#include "mqtt_header_codes.h"

class MqttMessage {
public:
	MqttMessage();
	~MqttMessage();

	void setType(const Type type);
	void setDup(const bool);
	void setRetain(const bool);
	void setQos(const uint8_t);

	Type getType() const;
	bool getDup() const;
	bool getRetain() const;
	uint8_t getQos() const;

	friend std::ostream& operator<<(std::ostream& os, const MqttMessage& msg);

private:
	Type type;
	bool dup;
	uint8_t qos;
	bool retain;
};