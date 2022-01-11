#include "coap.h"

std::vector<Coap::Option> Coap::decodeOptions(Bytes& data, int index) {
	std::vector<Option> options;
	uint8_t previousType = 0;

	while (index < data.size()) {
		Option option;
		uint8_t delta = (data[index] & MASK_DELTA) >> SHIFT_OPTION_DELTA;
		option.length = data[index] & MASK_OPTION_LENGTH;
		option.type = (OptionType)(previousType + delta);
		previousType = option.type;
		index++;

		switch (option.type) {

			// String
			case OptionType::UriHost: {
			case OptionType::LocationPath:
			case OptionType::UriPath:
			case OptionType::UriQuery:
			case OptionType::LocationQuery:
			case OptionType::ProxyUri:
			case OptionType::ProxyScheme:
			case OptionType::IfMatch:
			case OptionType::ETag:
				if (option.length != 0) {
					option.valueType = Option::OptionValueType::STRING_TYPE;
					option.stringValue = std::string(data.begin() + index, data.begin() + (index + option.length));
				}
				break;
			}

			// Uint
			case OptionType::UriPort:
			case OptionType::ContentFormat:
			case OptionType::MaxAge:
			case OptionType::Accept:
			case OptionType::Size2:
			case OptionType::Size1: {
				uint32_t intValue = 0;
				for (size_t shift = 0; shift < option.length; ++shift) {
					intValue = intValue | (data[index] << shift * SHIFT_BYTE);
					index++;
				}
				option.valueType = Option::OptionValueType::INT_TYPE;
				option.intValue = intValue;
				break;
			}
		}

		std::string name = optionNameLookup[option.type];
		index = (index + option.length);
		options.push_back(option);
	}
	return options;
}

Coap::CoapMessage Coap::decodeCoapMessage(Bytes& data) {
	CoapMessage coapMessage;
	auto it = std::find(data.begin(), data.end(), HEADER_DELIMITER);
	auto begin = data.begin();

	if (it != data.end()) {
		Bytes header(data.begin(), it);
		Bytes payloadData(++it, data.end());

		coapMessage.version = (header[0] & MASK_VERSION) >> SHIFT_VERSION;
		coapMessage.type = (Type)((header[0] & MASK_TYPE) >> SHIFT_TYPE);
		coapMessage.tokenLength = header[0] & MASK_TOKEN_LENGTH;
		coapMessage.code = (Code)header[1];

		uint16_t messageID = header[2] << SHIFT_MESSAGE_ID_2;
		messageID = messageID | header[3];
		coapMessage.messageID = messageID;

		uint32_t token = 0;
		size_t tokenStartIndex = 4;
		for (size_t tokenIndex = tokenStartIndex; tokenIndex < tokenStartIndex + coapMessage.tokenLength; ++tokenIndex) {
			token = token | (header[tokenIndex] << SHIFT_BYTE);
		}

		coapMessage.token = token;

		size_t optionsStartIndex = tokenStartIndex + coapMessage.tokenLength;
		std::vector<Option> options = decodeOptions(header, optionsStartIndex);
		coapMessage.options = options;
		coapMessage.payload = std::string(payloadData.begin(), payloadData.end());
	}
	return coapMessage;
}

void Coap::encodeMessageID(Bytes& data, CoapMessage& coapMessage) {
	unsigned char dataID = coapMessage.messageID & MASK_MESSAGE_ID_1;
	unsigned char dataID2 = ((coapMessage.messageID & MASK_MESSAGE_ID_2) >> SHIFT_MESSAGE_ID_2);
	data.push_back(dataID);
	data.push_back(dataID2);
}

void Coap::encodeOptions(Bytes& data, CoapMessage& coapMessage) {
	uint8_t previousType = 0;
	for (auto option : coapMessage.options) {
		unsigned char binaryOptionHeader = 0;
		uint8_t delta = option.type - previousType;
		binaryOptionHeader = delta << SHIFT_OPTION_DELTA;
		binaryOptionHeader = binaryOptionHeader | option.length;
		data.push_back(binaryOptionHeader);
		previousType = option.type;

		if (option.valueType == Option::OptionValueType::STRING_TYPE) {
			for (unsigned char c : option.stringValue) {
				data.push_back(c);
			}
		}
		else {
			if (option.length > 0) {
				data.push_back(option.intValue);
			}
		}
	}
}

void Coap::encodeHeader(Bytes& data, CoapMessage& coapMessage) {
	unsigned char byte = 0;
	byte = byte | (coapMessage.version << SHIFT_VERSION);
	byte = byte | (coapMessage.type << SHIFT_TYPE);
	byte = byte | coapMessage.tokenLength;
	data.push_back(byte);
	data.push_back(coapMessage.code);

	encodeMessageID(data, coapMessage);
	encodeOptions(data, coapMessage);
}

Bytes Coap::encodeCoapMessage(CoapMessage& coapMessage) {
	Bytes data;
	coapMessage.messageID = this->messageID;
	encodeHeader(data, coapMessage);

	if (coapMessage.payload.empty()) {
		return data;
	}

	data.push_back(HEADER_DELIMITER);
	for (char byte : coapMessage.payload) {
		data.push_back(byte);
	}
	return data;
}

uint8_t Coap::getVersion() const {
	return this->version;
}

uint16_t Coap::getMessageID() const {
	return this->messageID;
}

void Coap::incrementMessageID() {
	this->messageID++;
}
