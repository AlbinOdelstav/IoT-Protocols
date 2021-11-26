#pragma once
#include <map>

enum Type : uint8_t {
	CON = 0,
	NON = 1,
	ACK = 2,
	RST = 3,
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

enum OptionType : uint8_t {
	IfMatch = 1,
	UriHost = 3,
	ETag = 4,
	IfNoneMatch = 5,
	UriPort = 7,
	LocationPath = 8,
	UriPath = 11,
	ContentFormat = 12,
	MaxAge = 14,
	UriQuery = 15,
	Accept = 17,
	LocationQuery = 20,
	Size2 = 28,
	ProxyUri = 35,
	ProxyScheme = 39,
	Size1 = 60,
};

enum ContentFormatType : uint8_t {
	textPlainCharsetUtf8 = 0,
	applicationLinkFormat = 40,
	applicationXml = 41,
	applicationOctetStream = 42,
	applicationExi = 47,
	applicationJson = 50,
	applicationCbor = 60,
};