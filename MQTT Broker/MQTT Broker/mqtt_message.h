#pragma once
#include <iostream>
#include <vector>
#include "mqtt_header_codes.h"

using Topic = std::pair<std::string, int>;
using Bytes = std::vector<unsigned char>;
using Byte = unsigned char;

class MqttMessage {
public:
	MqttMessage();
	~MqttMessage();

	virtual void setType(const Type type);
	virtual void setDup(const bool);
	virtual void setRetain(const bool);
	virtual void setQos(const uint8_t);

	virtual Type getType() const;
	virtual bool getDup() const;
	virtual bool getRetain() const;
	virtual uint8_t getQos() const;

	friend std::ostream& operator<<(std::ostream& os, const MqttMessage& msg);

private:
	Type type;
	bool dup;
	uint8_t qos;
	bool retain;
	unsigned short errorCode;
};