#include "coap.h"

std::vector<unsigned char> Coap::encodeCoapMessage(CoapMessage& coapMessage) {
	binaryMessage.clear();
	this->messageID++;

	coapMessage.setMessageID(this->messageID);

	setHeader(coapMessage);

	
	if (coapMessage.getPayload().empty()) return binaryMessage;

	binaryMessage.push_back(0xFF);

	for (char byte : coapMessage.getPayload()) {
		binaryMessage.push_back(byte);
	}

	return binaryMessage;
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
			//	if (optionLength != 0) {
			//		uint32_t intValue = std::stoi(std::string(data.begin() + index, data.begin() + (index + optionLength)));
			//		value.setValue(intValue);
			//	}

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
		options.push_back(Option(optionType, value, optionLength));
		index = (index + optionLength);
	}
	return options;
}

CoapMessage Coap::decodeCoapMessage(std::vector<unsigned char>& data) {
	CoapMessage coapMessage;
	auto it = std::find(data.begin(), data.end(), 0xFF);
	auto begin = data.begin();
	//	coapMessage = decodeHeader(begin, it, data);

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

/*
CoapMessage Coap::decodeHeader(std::vector<unsigned char>::iterator& it, const std::vector<unsigned char>::iterator& end, std::vector<unsigned char>& data) {
	return CoapMessage();
}

CoapMessage Coap::decodeCoapMessage(std::vector<unsigned char>& data) {
	CoapMessage coapMessage;
	auto it = std::find(data.begin(), data.end(), 0xFF);
	auto begin = data.begin();
//	coapMessage = decodeHeader(begin, it, data);

	if (it != data.end()) {
		std::vector<unsigned char> header(data.begin(), it);
		std::vector<unsigned char> payloadData(++it, data.end());

		uint8_t version = (header[0] & 0b11000000) >> SHIFT_VERSION;
		Type type = (Type) ((header[0] & 0b00110000) >> SHIFT_TYPE);
		uint8_t tokenLength = header[0] & 0b00001111;
		Code code = (Code) header[1];
		uint16_t messageID = header[2] << SHIFT_MESSAGE_ID_SECOND;
		messageID = messageID | header[3];

		coapMessage.setVersion(version);
		coapMessage.setType(type);
		coapMessage.setTokenLength(tokenLength);
		coapMessage.setCode(code);
		coapMessage.setMessageID(messageID);

		uint32_t token = 0;
		auto tokenStartIndex = header.begin() + 4;
		for (auto it = tokenStartIndex; it < tokenStartIndex + tokenLength; ++it) {
			token = token | (*it << SHIFT_BYTE);
		}

		coapMessage.setToken(token);

		it = tokenStartIndex + tokenLength;

		std::vector<Option> options = decodeOptions(it, header.end());
		coapMessage.setOptions(options);
		coapMessage.setPayload(std::string(payloadData.begin(), payloadData.end()));
	}
	return coapMessage;
}
*/

std::vector<Option> Coap::decodeOptions(std::vector<unsigned char>::iterator& it, const std::vector<unsigned char>::iterator& end) {
	std::vector<Option> options;
	OptionType previousType;
	while (it != end) {

		OptionType optionType;
		OptionValue value(STRING_TYPE);

		uint8_t delta = (*it & 0b11110000) >> SHIFT_OPTION_DELTA;
		uint8_t optionLength = *it & 0b00001111;

		it++;

		if (!options.empty()) {
			optionType = (OptionType)(previousType + delta);
		}
		else {
			optionType = (OptionType)delta;
		}
		previousType = optionType;

		switch (optionType) {

			// String
		case OptionType::UriHost: {
		case OptionType::LocationPath:
		case OptionType::UriPath:
		case OptionType::UriQuery:
		case OptionType::LocationQuery:
		case OptionType::ProxyUri:
		case OptionType::ProxyScheme:
			value.setValueType(OptionValueType::STRING_TYPE);
			optionLength == 0 ? value.setValue(0) : value.setValue(std::string(it, it + optionLength));
			break;
		}

		// Opaque (?)
		case OptionType::IfMatch:
		case OptionType::ETag:
			value.setValueType(OptionValueType::STRING_TYPE);
			optionLength == 0 ? value.setValue(0) : value.setValue(std::string(it, it + optionLength));
			break;

		// Uint
		case OptionType::UriPort:
		case OptionType::ContentFormat:
		case OptionType::MaxAge:
		case OptionType::Accept:
		case OptionType::Size2:
		case OptionType::Size1:
			value.setValueType(OptionValueType::INT_TYPE);
			optionLength == 0 ? value.setValue(0) : value.setValue(std::stoi(std::string(it, it + optionLength)));

			break;

		// Empty
		case OptionType::IfNoneMatch:

			break;
		}
		options.push_back(Option(optionType, value, optionLength));
		it = (it + optionLength);
	}
	return options;
}

unsigned char Coap::generateFirstByteInHeader(CoapMessage& coapMessage) {
	unsigned char byte = 0;
	byte = byte | (coapMessage.getVersion() << SHIFT_VERSION);
	byte = byte | (coapMessage.getType() << SHIFT_TYPE);
	byte = byte | coapMessage.getTokenLength();
	return byte;
}

void Coap::setHeader(CoapMessage& coapMessage) {

	unsigned char byte = generateFirstByteInHeader(coapMessage);

	binaryMessage.push_back(byte);
	binaryMessage.push_back(coapMessage.getCode());
	
	std::cout << messageID << "\n";
	unsigned char binaryMessageID = coapMessage.getMessageID() & MASK_MESSAGE_ID_1;
	unsigned char binaryMessageID2 = ((coapMessage.getMessageID() & MASK_MESSAGE_ID_2) >> SHIFT_MESSAGE_ID_2);
	binaryMessage.push_back(binaryMessageID);
	binaryMessage.push_back(binaryMessageID2);


	/*
			Value:	opaque | string | empty | uint |

			12 = content-format => uint => mediaType
			*/

	uint8_t previousType = 0;
	for (auto option: coapMessage.getOptions()) {

		unsigned char binaryOptionHeader = 0;
		uint8_t delta = option.getOptionType() - previousType;

		binaryOptionHeader = delta << SHIFT_OPTION_DELTA;
		std::cout << std::bitset<8>(binaryOptionHeader) << "\n";

		binaryOptionHeader = binaryOptionHeader | option.getLength();
		std::cout << std::bitset<8>(binaryOptionHeader) << "\n";

		binaryMessage.push_back(binaryOptionHeader);
		previousType = option.getOptionType();
		

		if (option.getValueType() == OptionValueType::STRING_TYPE) {
			for (unsigned char c : option.getString()) {
				binaryMessage.push_back(c);
			}
		}
		else {
			if (option.getLength() > 0) {
				binaryMessage.push_back(option.getInt());
			}
		}
	}
}

uint8_t Coap::getVersion() const {
	return this->version;
}

uint16_t Coap::getMessageID() const {
	return this->messageID;
}

std::vector<unsigned char> Coap::getBinaryData() const {
	return this->binaryMessage;
}

void Coap::incrementMessageID() {
	this->messageID++;
}
