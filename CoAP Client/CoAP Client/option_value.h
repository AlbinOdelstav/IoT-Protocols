#pragma once
#include <iostream>
#include <string>

enum OptionValueType {
	STRING_TYPE, INT_TYPE
};

struct OptionValue {

public:
	OptionValue();
	OptionValue(OptionValueType valueType);
	OptionValue(std::string value);
	OptionValue(uint32_t value);

	void setValue(std::string value);
	void setValue(int value);
	void setValueType(OptionValueType valueType);

	std::string getString() const;
	uint32_t getInt() const;
	OptionValueType getType() const;

	OptionValue& operator=(std::string value);
	OptionValue& operator=(uint8_t value);

private:
	std::string stringValue;
	uint32_t intValue;
	OptionValueType valueType;
};