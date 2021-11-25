#pragma once
#include <map>
#include <iostream>
#include <vector>
#include <bitset>
#include <string>
#include <algorithm>
#include "coap_message.h"

class Coap {

public:

	// ??
	std::map<uint8_t, std::string> codeLookup {
		{0x1, "GET"},
		{0x2, "POST"},
		{0x3, "PUT"},
		{0x4, "DELETE"},
		{0x41, "Created"},
		{0x42, "Deleted"},
		{0x43, "Valid"},
		{0x44, "Changed"},
		{0x45, "Content"},
		{0x5F, "Continue"},
		{0x80, "Bad Request"},
		{0x81, "Unauthorized"},
		{0x82, "Bad Option"},
		{0x83, "Forbidden"},
		{0x84, "Not Found"},
		{0x85, "Method Not Allowed"},
		{0x86, "Not Acceptable"},
		{0x88, "Request Entity Incomplete"},
		{0x8C, "Precondition Failed"},
		{0x8D, "Request Entity Too Large"},
		{0x8F, "Unsupported Content-Format"},
		{0xA0, "Internal Server Error"},
		{0xA1, "Not Implemented"},
		{0xA2, "Bad Gateway"},
		{0xA3, "Service Unavailable"},
		{0xA4, "Gateway Timeout"},
		{0xA5, "Proxying Not Supported"}
	};

	unsigned short MASK_VERSION = 0b11000000;
	unsigned short MASK_TYPE = 0b00110000;
	unsigned short MASK_TOKEN_LENGTH = 0b00001111;
	unsigned short MASK_DELTA = 0b11110000;
	unsigned short MASK_OPTION_LENGTH = 0b00001111;
	unsigned short MASK_MESSAGE_ID_1 = 0b0000000011111111;
	unsigned short MASK_MESSAGE_ID_2 = 0b1111111100000000;

	unsigned short SHIFT_VERSION = 6;
	unsigned short SHIFT_TYPE = 4;
	unsigned short SHIFT_MESSAGE_ID_2 = 8;
	unsigned short SHIFT_OPTION_DELTA = 4;
	unsigned short SHIFT_BYTE = 8;

	std::vector<unsigned char> encodeCoapMessage(CoapMessage& coapMessage);
	unsigned char generateFirstByteInHeader(CoapMessage& coapMessage);
	
	CoapMessage decodeCoapMessage(std::vector<unsigned char>& data);
	std::vector<Option> decodeOptions(std::vector<unsigned char>::iterator& it, const std::vector<unsigned char>::iterator& end);
	std::vector<Option> decodeOptions(std::vector<unsigned char>& data, int index);

	uint8_t getVersion() const;
	uint16_t getMessageID() const;
	std::vector<unsigned char> getBinaryData() const;

	void incrementMessageID();

private:
	const uint8_t version = 1;
	uint16_t messageID = rand();
	void setHeader(CoapMessage& coapMessage);

	std::vector<unsigned char> binaryMessage;
};