#pragma once
#include <iostream>
#include "mqtt_message.h"

class ConnectMessage : public MqttMessage {
public:
	void setMsb(const uint8_t);
	void setLsb(const uint8_t);
	void setProtocolName(const std::string);

	uint8_t getMsb() const;
	uint8_t getLsb() const;
	std::string getProtocolName() const;

private:
	uint8_t msb;
	uint8_t lsb;
	std::string protocolName;
};

