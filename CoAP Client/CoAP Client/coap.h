#pragma once
#include <map>
#include <iostream>
#include <vector>
#include <bitset>
#include <string>
#include <algorithm>
#include "coap_header_codes.h"

using Bytes = std::vector<unsigned char>;

class Coap {
public:
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

	std::map<uint8_t, std::string> optionNameLookup {
		{1, "If-Match"},
		{3, "Uri-Host"},
		{4, "ETag"},
		{5, "If-None-Match"},
		{7, "Uri-Port"},
		{8, "Location-Path"},
		{11, "Uri-Path"},
		{12, "Content-Format"},
		{14, "Max-Age"},
		{15, "Uri-Query"},
		{17, "Accept"},
		{20, "Location-Query"},
		{28, "Size2"},
		{35, "Proxy-Uri"},
		{39, "Proxy-Scheme"},
		{60, "Size1"},
	};

	const unsigned char MASK_VERSION = 0b11000000;
	const unsigned char MASK_TYPE = 0b00110000;
	const unsigned char MASK_TOKEN_LENGTH = 0b00001111;
	const unsigned char MASK_DELTA = 0b11110000;
	const unsigned char MASK_OPTION_LENGTH = 0b00001111;
	const unsigned short MASK_MESSAGE_ID_1 = 0b0000000011111111;
	const unsigned short MASK_MESSAGE_ID_2 = 0b1111111100000000;

	const unsigned char SHIFT_VERSION = 6;
	const unsigned char SHIFT_TYPE = 4;
	const unsigned char SHIFT_MESSAGE_ID_2 = 8;
	const unsigned char SHIFT_OPTION_DELTA = 4;
	const unsigned char SHIFT_BYTE = 8;

	const unsigned short HEADER_DELIMITER = 0xFF;

	struct Option {
		enum OptionValueType {
			STRING_TYPE, INT_TYPE
		};

		OptionType type;
		std::string stringValue;
		uint32_t intValue;
		uint16_t length;
		std::string name;

		OptionValueType valueType;
	};

	struct CoapMessage {
		uint8_t version;
		Type type;
		Code code;
		std::vector<Option> options;
		uint8_t tokenLength;
		uint32_t token;
		uint16_t messageID;
		std::string payload;

		friend std::ostream& operator<<(std::ostream& os, const CoapMessage& msg) {
			os << "\n-----------------------------\n";
			os << "         COAP MESSAGE        \n";
			os << "-----------------------------\n\n";

			os << "Version: " + std::to_string(msg.version) + "\n";
			os << "Type: " + std::to_string(msg.type) + "\n";
			os << "TokenLength: " + std::to_string(msg.tokenLength) + "\n";
			os << "Code: " + std::to_string(msg.code) + "\n";
			os << "MessageID: " + std::to_string(msg.messageID) + "\n";
			os << "\n";

			for (size_t i = 0; i < msg.options.size(); ++i) {
				os << "Option #" << (i + 1) << ": " << msg.options[i].name << "\n";
				os << "  Length: " + std::to_string(msg.options[i].length) + "\n";
				if (msg.options[i].valueType == Option::OptionValueType::STRING_TYPE) {
					os << "  Value: " + msg.options[i].stringValue + "\n";
				}
				else {
					os << "  Value: " + std::to_string(msg.options[i].intValue) + "\n";
				}
				os << "\n";
			}
			os << "Payload: ";
			os << msg.payload << "\n";
			os << "\n-----------------------------\n";

			return os;
		};
	};

	Bytes encodeCoapMessage(CoapMessage& coapMessage);
	void encodeHeader(Bytes& data, CoapMessage& coapMessage);
	void encodeMessageID(Bytes& data, CoapMessage& coapMessage);
	void encodeOptions(Bytes& data, CoapMessage& coapMessage);

	CoapMessage decodeCoapMessage(Bytes& data);
	std::vector<Option> decodeOptions(Bytes& data, int index);

	uint8_t getVersion() const;
	uint16_t getMessageID() const;
	void incrementMessageID();

private:
	const uint8_t version = 1;
	uint16_t messageID = rand();
};