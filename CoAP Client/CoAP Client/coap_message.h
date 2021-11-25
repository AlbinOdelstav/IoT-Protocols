#pragma once
#include <iostream>
#include <ostream>
#include <vector>
#include "coap_header_codes.h"
#include "option_value.h"

struct Option {
public:
	Option(OptionType optionType, OptionValue value, uint16_t length);
	OptionType getOptionType() const;
	std::string getString() const;
	uint32_t getInt() const;
	OptionValueType getValueType() const;
	uint16_t getLength() const;

	void setOptionType(OptionType optionType);
	void setString(std::string value);
	void setInt(uint32_t value);
	void setValueType(OptionValueType valueType);
	void setLength(uint16_t length);

private:
	OptionType optionType;
	OptionValue value;
	uint16_t length;
};

class CoapMessage {
public:
	CoapMessage();
	CoapMessage(uint8_t version, Type type, Code code, uint16_t messageID, uint32_t token, std::vector<Option> options, std::string payload);
	~CoapMessage();

	uint32_t getVersion() const;
	Type getType() const;
	Code getCode() const;
	std::vector<Option> getOptions() const;
	uint8_t getTokenLength() const;
	uint32_t getToken() const;
	uint16_t getMessageID() const;
	std::string getPayload() const;

	void setVersion(uint32_t version);
	void setType(Type type);
	void setCode(Code& code);

	void addOption(Option& option);
	void setOptions(std::vector<Option>& options);

	void setTokenLength(uint8_t tokenLength);
	void setToken(uint32_t token);
	void setMessageID(uint16_t messageID);
	void setPayload(std::string payload);

	friend std::ostream& operator<<(std::ostream& os, const CoapMessage& msg);

private:
	uint8_t version;
	Type type;
	Code code;

	std::vector<Option> options;
	
	uint8_t tokenLength;
	uint32_t token;
	uint16_t messageID;
	std::string payload;
};