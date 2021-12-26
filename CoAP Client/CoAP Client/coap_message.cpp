#pragma once
#include "coap_message.h"

CoapMessage::CoapMessage() {
	messageID = rand() % 10000;
}

CoapMessage::CoapMessage(uint8_t version, Type type, Code code, uint16_t messageID,
	uint32_t token, std::vector<Option> options, std::string payload)
	: version(version), type(type), tokenLength(0), code(code), messageID(messageID), 
	token(token), options(options), payload(payload) {
	messageID = rand() % 10000;
}

CoapMessage::~CoapMessage() {
}

uint32_t CoapMessage::getVersion() const {
	return this->version;
}

Type CoapMessage::getType() const {
	return this->type;
}

Code CoapMessage::getCode() const {
	return this->code;
}

std::vector<Option> CoapMessage::getOptions() const {
	return this->options;
}

uint8_t CoapMessage::getTokenLength() const {
	return this->tokenLength;
}

uint32_t CoapMessage::getToken() const {
	return this->token;
}

uint16_t CoapMessage::getMessageID() const {
	return this->messageID;
}

std::string CoapMessage::getPayload() const {
	return this->payload;
}

void CoapMessage::setVersion(uint32_t version) {
	this->version = version;
}

void CoapMessage::setType(Type type) {
	this->type = type;
}

void CoapMessage::setCode(Code& code) {
	this->code = code;
}

void CoapMessage::addOption(Option& option) {
	this->options.push_back(option);
}
void CoapMessage::setOptions(std::vector<Option>& options) {
	this->options = options;
}

void CoapMessage::setTokenLength(uint8_t tokenLength) {
	this->tokenLength = tokenLength;
}

void CoapMessage::setToken(uint32_t token) {
	this->token = token;
}

void CoapMessage::setMessageID(uint16_t messageID) {
	this->messageID = messageID;
}

void CoapMessage::setPayload(std::string payload) {
	this->payload = payload;
}

Option::Option(OptionType optionType, OptionValue value, std::string name, uint16_t length)
	: optionType(optionType), value(value), name(name), length(length) {}

OptionType Option::getOptionType() const {
	return optionType;
}

std::string Option::getString() const {
	return value.getString();
}

uint32_t Option::getInt() const {
	return value.getInt();
}

OptionValueType Option::getValueType() const {
	return value.getType();
}

uint16_t Option::getLength() const{
	return length;
}

std::string Option::getName() const {
	return name;
}

void Option::setOptionType(OptionType optionType) {
	this->optionType = optionType;
}

void Option::setString(std::string value) {
	this->value.setValue(value);
}

void Option::setInt(uint32_t value) {
	this->value.setValue(value);
}

void Option::setValueType(OptionValueType valueType) {
	this->value.setValueType(valueType);
}

void Option::setLength(uint16_t length) {
	this->length = length;
}

void Option::setName(std::string name) {
	this->name = name;
}

std::ostream& operator<<(std::ostream& os, const CoapMessage& msg) {
	os << "\n-----------------------------\n";
	os << "         COAP MESSAGE        \n";
	os << "-----------------------------\n\n";

	os << "Version: " + std::to_string(msg.version) + "\n";
	os << "Type: " + std::to_string(msg.type) + "\n";
	os << "TokenLength: " + std::to_string(msg.tokenLength) + "\n";
	os << "Code: " + std::to_string(msg.code) + "\n";
	os << "MessageID: " + std::to_string(msg.messageID) + "\n";
	os << "\n";

	for (size_t i = 0; i < msg.getOptions().size(); ++i) {
		os << "Option #" << (i + 1) << ": " << msg.getOptions()[i].getName() << "\n";
		os << "  Length: " + std::to_string(msg.getOptions()[i].getLength()) + "\n";
		if (msg.getOptions()[i].getValueType() == STRING_TYPE) {
			os << "  Value: " + msg.getOptions()[i].getString() + "\n";
		}
		else {
			os << "  Value: " + std::to_string(msg.getOptions()[i].getInt()) + "\n";
		}
		os << "\n";
	}
	os << "Payload: ";
	os << msg.payload << "\n";
	os << "\n-----------------------------\n";

	return os;
}
