#pragma once
#include <iostream>

enum Type : uint8_t {
	Reserved = 0,
	CONNECT = 1,
	CONNACK = 2,
	PUBLISH = 3,
	PUBACK = 4,
	PUBREC = 5,
	PUBREL = 6,
	PUBCOMP = 7,
	SUBSCRIBE = 8,
	SUBACK = 9,
	UNSUBSCRIBE = 10,
	UNSUBACK = 11,
	PINGREQ = 12,
	PINGRESP = 13,
	DISCONNECT = 14,
	Reserved2 = 15,
};

const uint8_t MASK_TYPE = 0b11110000;
const uint8_t MASK_DUP = 0b00001000;
const uint8_t MASK_QOS = 0b00000110;
const uint8_t MASK_RETAIN = 0b00000001;

const short SHIFT_TYPE = 4;
const short SHIFT_DUP = 3;
const short SHIFT_QOS = 2;
