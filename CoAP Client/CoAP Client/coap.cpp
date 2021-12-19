#pragma once
#include "coap.h"

std::vector<unsigned char> Coap::encodeCoapMessage(CoapMessage& coapMessage) {
	std::vector<unsigned char> data;
	coapMessage.setMessageID(this->messageID);
	encodeHeader(data, coapMessage);
	
	if (coapMessage.getPayload().empty()) {
		return data;
	}
	
	data.push_back(HEADER_DELIMITER);
	for (char byte : coapMessage.getPayload()) {
		data.push_back(byte);
	}
	return data;
}

std::vector<Option> Coap::decodeOptions(std::vector<unsigned char>& data, int index) {
	std::vector<Option> options;
	uint8_t previousType = 0;

	while (index < data.size()) {
		uint8_t delta = (data[index] & MASK_DELTA) >> SHIFT_OPTION_DELTA;
		uint8_t optionLength = data[index] & MASK_OPTION_LENGTH;
		OptionType optionType = (OptionType)(previousType + delta);
		previousType = optionType;
		index++;

		OptionValue value(STRING_TYPE);
		switch (optionType) {

		// String
		case OptionType::UriHost: {
		case OptionType::LocationPath:
		case OptionType::UriPath:
		case OptionType::UriQuery:
		case OptionType::LocationQuery:
		case OptionType::ProxyUri:
		case OptionType::ProxyScheme:
			if (optionLength != 0) {
				std::string stringValue = std::string(data.begin() + index, data.begin() + (index + optionLength));
				value.setValue(stringValue);
			}
			break;
		}

		// Opaque (?)
		case OptionType::IfMatch:
		case OptionType::ETag:
			if (optionLength != 0) {
				std::string stringValue = std::string(data.begin() + index, data.begin() + (index + optionLength));
				value.setValue(stringValue);
			}
			break;

		// Uint
		case OptionType::UriPort:
		case OptionType::ContentFormat:
		case OptionType::MaxAge:
		case OptionType::Accept:
		case OptionType::Size2:
		case OptionType::Size1: {
			uint32_t intValue = 0;
			for (size_t shift = 0; shift < optionLength; ++shift) {
				intValue = intValue | (data[index] << shift * SHIFT_BYTE);
				index++;
			}
			value.setValue(intValue);
			break;
		}

		// Empty
		case OptionType::IfNoneMatch:

			break;
		}

		std::string name = optionNameLookup[optionType];
		options.push_back(Option(optionType, value, name, optionLength));
		index = (index + optionLength);
	}
	return options;
}

CoapMessage Coap::decodeCoapMessage(std::vector<unsigned char>& data) {
	CoapMessage coapMessage;
	auto it = std::find(data.begin(), data.end(), HEADER_DELIMITER);
	auto begin = data.begin();

	if (it != data.end()) {
		std::vector<unsigned char> header(data.begin(), it);
		std::vector<unsigned char> payloadData(++it, data.end());

		uint8_t version = (header[0] & MASK_VERSION) >> SHIFT_VERSION;
		Type type = (Type)((header[0] & MASK_TYPE) >> SHIFT_TYPE);
		uint8_t tokenLength = header[0] & MASK_TOKEN_LENGTH;
		Code code = (Code)header[1];
		uint16_t messageID = header[2] << SHIFT_MESSAGE_ID_2;
		messageID = messageID | header[3];

		coapMessage.setVersion(version);
		coapMessage.setType(type);
		coapMessage.setTokenLength(tokenLength);
		coapMessage.setCode(code);
		coapMessage.setMessageID(messageID);

		uint32_t token = 0;
		size_t tokenStartIndex = 4;
		for (size_t tokenIndex = tokenStartIndex; tokenIndex < tokenStartIndex + tokenLength; ++tokenIndex) {
			token = token | (header[tokenIndex] << SHIFT_BYTE);
		}

		coapMessage.setToken(token);

		size_t optionsStartIndex = tokenStartIndex + tokenLength;
		std::vector<Option> options = decodeOptions(header, optionsStartIndex);
		coapMessage.setOptions(options);
		coapMessage.setPayload(std::string(payloadData.begin(), payloadData.end()));
	}
	return coapMessage;
}

void Coap::encodeOptions(std::vector<unsigned char>& data, CoapMessage& coapMessage) {
	uint8_t previousType = 0;
	for (auto option : coapMessage.getOptions()) {
		unsigned char binaryOptionHeader = 0;
		uint8_t delta = option.getOptionType() - previousType;
		binaryOptionHeader = delta << SHIFT_OPTION_DELTA;
		binaryOptionHeader = binaryOptionHeader | option.getLength();
		data.push_back(binaryOptionHeader);
		previousType = option.getOptionType();

		if (option.getValueType() == OptionValueType::STRING_TYPE) {
			for (unsigned char c : option.getString()) {
				data.push_back(c);
			}
		}
		else {
			if (option.getLength() > 0) {
				data.push_back(option.getInt());
			}
		}
	}
}

void Coap::encodeMessageID(std::vector<unsigned char>& data, CoapMessage& coapMessage) {
	unsigned char dataID = coapMessage.getMessageID() & MASK_MESSAGE_ID_1;
	unsigned char dataID2 = ((coapMessage.getMessageID() & MASK_MESSAGE_ID_2) >> SHIFT_MESSAGE_ID_2);
	data.push_back(dataID);
	data.push_back(dataID2);
}

void Coap::encodeHeader(std::vector<unsigned char>& data, CoapMessage& coapMessage) {
	unsigned char byte = 0;
	byte = byte | (coapMessage.getVersion() << SHIFT_VERSION);
	byte = byte | (coapMessage.getType() << SHIFT_TYPE);
	byte = byte | coapMessage.getTokenLength();
	data.push_back(byte);
	data.push_back(coapMessage.getCode());

	encodeMessageID(data, coapMessage);
	encodeOptions(data, coapMessage);
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
