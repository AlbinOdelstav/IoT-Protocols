#include <iostream>
#include <vector>
#include <bitset>
#include <sstream>
#include <algorithm>
#include <string>
#include <time.h>

#include "coap.h"
#include "socket.h"

Code getCode(int input) {
	switch (input) {
	case 1:
		return Code::Empty;
	case 2:
		return Code::Get;
	case 3:
		return Code::Post;
	case 4:
		return Code::Put;
	case 5:
		return Code::Delete;
	}
}

Type getType(int input) {
	switch (input) {
	case 1:
		return Type::CON;
	case 2:
		return Type::NON;
	case 3:
		return Type::ACK;
	case 4:
		return Type::RST;
	}
}

ContentFormatType getFormat(int input) {
	switch (input) {
	case 1:
		return ContentFormatType::textPlainCharsetUtf8;
	case 2:
		return ContentFormatType::applicationLinkFormat;
	case 3:
		return ContentFormatType::applicationXml;

	case 4:
		return ContentFormatType::applicationOctetStream;
	case 5:
		return ContentFormatType::applicationExi;
	case 6:
		return ContentFormatType::applicationJson;
	case 7:
		return ContentFormatType::applicationCbor;
	}
}

int main(int argc, char *argv[]) {
	Socket socket("coap.me", 5683);

	srand(time(0));
	Coap coap;

	while (true) {
		std::string input;

		std::string payload;
		Code code;
		Type type;
		uint8_t contentFormat = 0;
		std::vector<Coap::Option> options;

		std::cout << "\nEnter path: /";
		std::getline(std::cin, input);

		Coap::Option pathOption {
			OptionType::UriPath,
			input,
			0,
			pathOption.stringValue.length(),
			coap.optionNameLookup[OptionType::UriPath],
			Coap::Option::OptionValueType::STRING_TYPE,
		};

		options.push_back(pathOption);

		std::cout << "\nEnter type:\n\n";
		std::cout << "1. Confirmable\n";
		std::cout << "2. Non-confirmable\n";
		std::cout << "3. Acknowledgement\n";
		std::cout << "4. Reset\n";
		std::cin >> input;
		type = getType(std::stoi(input));
		std::cin.ignore();

		std::cout << "\nEnter method:\n\n";
		std::cout << "1. EMPTY\n";
		std::cout << "2. GET\n";
		std::cout << "3. POST\n";
		std::cout << "4. PUT\n";
		std::cout << "5. DELETE\n";
		std::cout << "0. Exit\n\n: ";
		std::cin >> input;
		code = getCode(std::stoi(input));
		if (input == "0") return 0;
		std::cin.ignore();

		std::cout << "\nEnter format:\n\n";
		std::cout << "1. text/plain; charset=utf8\n";
		std::cout << "2. application/link-format\n";
		std::cout << "3. application/xml\n";
		std::cout << "4. application/octet-stream\n";
		std::cout << "5. application/exi\n";
		std::cout << "6. application/json\n";
		std::cout << "7. application/cbor\n";
		std::cout << "0. No format\n\n: ";

		std::cin >> input;
		if (input != "0") {
		contentFormat = getFormat(std::stoi(input));

		}
		std::cin.ignore();

		std::cout << "\nEnter payload: ";
		std::getline(std::cin, payload);

		if (contentFormat!= 0) {
			const int length = contentFormat == ContentFormatType::textPlainCharsetUtf8 ? 0 : 1;

			Coap::Option contentFormatOption {
				OptionType::ContentFormat,
				"",
				contentFormat,
				length,
				coap.optionNameLookup[OptionType::ContentFormat],
				pathOption.valueType = Coap::Option::OptionValueType::INT_TYPE
			};

			options.push_back(contentFormatOption);
		}

		Coap::CoapMessage coapMessage {
			coap.getVersion(),
			type,
			code,
			options,
			0,
			0,
			coap.getMessageID(),
			payload
		};

		for (auto option : coapMessage.options) {
			std::cout << option.stringValue << "\n";
			std::cout << option.intValue << "\n";
		}

		std::vector<unsigned char> binaryCoapMessage = coap.encodeCoapMessage(coapMessage);
		std::vector<unsigned char> response = socket.send(binaryCoapMessage);
		Coap::CoapMessage responseDecoded = coap.decodeCoapMessage(response);
		coap.incrementMessageID();

		std::cout << responseDecoded << "\n";
	}
}