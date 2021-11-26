#pragma once
#include "option_value.h"

OptionValue::OptionValue() : stringValue(""), intValue(0), valueType(INT_TYPE) {}

OptionValue::OptionValue(OptionValueType valueType)
	: stringValue(), intValue(0), valueType(valueType) {}

OptionValue::OptionValue(std::string value) 
	: stringValue(value), intValue(0), valueType(OptionValueType::STRING_TYPE) {}

OptionValue::OptionValue(uint32_t value) 
	: stringValue(), intValue(value), valueType(OptionValueType::INT_TYPE) {}

void OptionValue::setValue(std::string value) {
	this->setValueType(OptionValueType::STRING_TYPE);
	this->stringValue = value;
}
void OptionValue::setValue(int value) {
	this->setValueType(OptionValueType::INT_TYPE);
	this->intValue = value;
}

void OptionValue::setValueType(OptionValueType valueType) {
	this->valueType = valueType;
}

std::string OptionValue::getString() const {
	return this->stringValue;
}

uint32_t OptionValue::getInt() const {
	return this->intValue;
}

OptionValueType OptionValue::getType() const {
	return this->valueType;
}

OptionValue& OptionValue::operator=(std::string value) {
	this->setValue(value);
}

OptionValue& OptionValue::operator=(uint8_t value){
	this->setValue(value);
}
