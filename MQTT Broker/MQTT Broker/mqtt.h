#pragma once
#include <iostream>

class mqtt {
public:

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
		Reserved = 15,

	};

	mqtt();
	~mqtt();
};

enum Code : uint8_t {
	Empty = 0x0,
	Get = 0x1,
	Post = 0x2,
	Put = 0x3,
	Delete = 0x4,
	Created = 0x41,
	Deleted = 0x42,
	Valid = 0x43,
	Changed = 0x44,
	Content = 0x45,
	Continue = 0x5F,
	BadRequest = 0x80,
	Unauthorized = 0x81,
	BadOption = 0x82,
	Forbidden = 0x83,
	NotFound = 0x84,
	MethodNotAllowed = 0x85,
	NotAccaptable = 0x86,
	RequestEntityIncomplete = 0x88,
	PreconditionFailed = 0x8C,
	RequestEntityTooLarge = 0x8D,
	UnsupportedContentFormat = 0x8F,
	InternalServerError = 0xA0,
	NotImplemented = 0xA1,
	BadGateway = 0xA2,
	ServiceUnavailable = 0xA3,
	GateweayTimeout = 0xA4,
	ProxyingNotSupported = 0xA5,
};